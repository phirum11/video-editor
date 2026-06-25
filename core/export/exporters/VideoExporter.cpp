#include "core/export/exporters/VideoExporter.h"
#include "core/export/encoders/HardwareDetector.h"
#include "core/export/renderers/AdvancedSubtitleRenderer.h"

#include "ui/timeline_view/controllers/TimelineController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

struct CodecPlan
{
    QString videoEncoder;
    QString audioEncoder;
    QString muxer;
    QString extension;
    QStringList videoArgs;
    QStringList audioArgs;
};

QString number(double value)
{
    return QString::number(value, 'f', 3);
}

QString expressionNumber(double value)
{
    return QString::number(value, 'f', 6);
}

QString sanitizeExtension(QString value)
{
    value = value.trimmed().toLower();
    if (value.startsWith(QLatin1Char('.'))) {
        value.remove(0, 1);
    }
    return value;
}

int roundedDelayMs(double seconds)
{
    return std::max(0, static_cast<int>(std::llround(seconds * 1000.0)));
}

bool isFinitePositive(double value)
{
    return std::isfinite(value) && value > 0.0;
}

double unitValue(double value, double fallback = 0.0)
{
    if (!std::isfinite(value)) {
        return fallback;
    }
    return std::clamp(value, 0.0, 1.0);
}

BlurEffectData normalizedBlur(BlurEffectData blur)
{
    blur.radius = std::clamp(std::isfinite(blur.radius) ? blur.radius : 0.0, 0.0, 100.0);
    blur.regionX = unitValue(blur.regionX, 0.25);
    blur.regionY = unitValue(blur.regionY, 0.25);
    blur.regionWidth = std::clamp(std::isfinite(blur.regionWidth) ? blur.regionWidth : 0.5, 0.001, 1.0);
    blur.regionHeight = std::clamp(std::isfinite(blur.regionHeight) ? blur.regionHeight : 0.5, 0.001, 1.0);
    if (blur.regionX + blur.regionWidth > 1.0) {
        blur.regionX = 1.0 - blur.regionWidth;
    }
    if (blur.regionY + blur.regionHeight > 1.0) {
        blur.regionY = 1.0 - blur.regionHeight;
    }
    return blur;
}

QString gaussianBlurFilter(double radius)
{
    const double sigma = std::max(0.35, radius / 3.0);
    return QStringLiteral("gblur=sigma=%1:steps=2").arg(number(sigma));
}

bool hasVisibleColorAdjustment(const ClipEffects& effects)
{
    return std::abs(effects.color.brightness) > 0.0001
        || std::abs(effects.color.contrast - 100.0) > 0.0001
        || std::abs(effects.color.saturation - 100.0) > 0.0001;
}

QString ffmpegEnableBetween(double startSeconds, double endSeconds)
{
    return QStringLiteral("enable='between(t\\,%1\\,%2)'")
        .arg(expressionNumber(startSeconds))
        .arg(expressionNumber(endSeconds));
}

QString appendBlurFilters(QStringList& filters,
                          const QString& inputLabel,
                          const ClipEffects& effects,
                          int labelIndex)
{
    const BlurEffectData blur = normalizedBlur(effects.blur);
    if (blur.radius <= 0.0) {
        return inputLabel;
    }

    const QString blurFilter = gaussianBlurFilter(blur.radius);
    const QString outputLabel = QStringLiteral("v%1_blur").arg(labelIndex);
    if (!blur.isRegionEnabled) {
        filters << QStringLiteral("[%1]%2,format=rgba[%3]")
                       .arg(inputLabel, blurFilter, outputLabel);
        return outputLabel;
    }

    const QString baseLabel = QStringLiteral("v%1_blur_base").arg(labelIndex);
    const QString cropLabel = QStringLiteral("v%1_blur_crop").arg(labelIndex);
    const QString patchLabel = QStringLiteral("v%1_blur_patch").arg(labelIndex);
    filters << QStringLiteral("[%1]split=2[%2][%3]")
                   .arg(inputLabel, baseLabel, cropLabel);
    filters << QStringLiteral("[%1]crop=w=max(1\\,iw*%2):h=max(1\\,ih*%3):x=min(iw-out_w\\,max(0\\,iw*%4)):y=min(ih-out_h\\,max(0\\,ih*%5)),%6,format=rgba[%7]")
                   .arg(cropLabel,
                        expressionNumber(blur.regionWidth),
                        expressionNumber(blur.regionHeight),
                        expressionNumber(blur.regionX),
                        expressionNumber(blur.regionY),
                        blurFilter,
                        patchLabel);
    filters << QStringLiteral("[%1][%2]overlay=x=min(main_w-overlay_w\\,max(0\\,main_w*%3)):y=min(main_h-overlay_h\\,max(0\\,main_h*%4)):shortest=0:eof_action=pass:format=auto[%5]")
                   .arg(baseLabel,
                        patchLabel,
                        expressionNumber(blur.regionX),
                        expressionNumber(blur.regionY),
                        outputLabel);
    return outputLabel;
}

QString firstNonEmpty(const QVariantMap& map, std::initializer_list<const char*> keys, const QString& fallback)
{
    for (const char* key : keys) {
        const QString value = map.value(QString::fromLatin1(key)).toString().trimmed();
        if (!value.isEmpty()) {
            return value;
        }
    }
    return fallback;
}

} // namespace



VideoExporter::VideoExporter(QObject *parent)
    : QObject(parent)
{
}

VideoExporter::~VideoExporter()
{
    if (!m_process) {
        return;
    }

    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    delete m_process;
}

void VideoExporter::startExport(QObject* timelineControllerObj,
                                const QString& outputPath,
                                bool exportVideo,
                                bool exportAudio)
{
    QVariantMap settings;
    settings.insert(QStringLiteral("exportVideo"), exportVideo);
    settings.insert(QStringLiteral("exportAudio"), exportAudio);
    settings.insert(QStringLiteral("resolution"), QStringLiteral("1080P"));
    settings.insert(QStringLiteral("bitrate"), QStringLiteral("Higher"));
    settings.insert(QStringLiteral("codec"), QStringLiteral("H.264"));
    settings.insert(QStringLiteral("format"), QStringLiteral("MP4"));
    settings.insert(QStringLiteral("frameRate"), QStringLiteral("60 fps"));
    settings.insert(QStringLiteral("audioFormat"), QStringLiteral("AAC"));
    startExportWithSettings(timelineControllerObj, outputPath, settings);
}

void VideoExporter::startExportWithSettings(QObject* timelineControllerObj,
                                            const QString& outputPath,
                                            const QVariantMap& settingsMap)
{
    if (m_isExporting) {
        emit exportFinished(false, QStringLiteral("An export is already in progress."));
        return;
    }

    const ExportSettings settings = parseSettings(settingsMap,
                                                  settingsMap.value(QStringLiteral("exportVideo"), true).toBool(),
                                                  settingsMap.value(QStringLiteral("exportAudio"), true).toBool());
    if (!settings.exportVideo && !settings.exportAudio) {
        emit exportFinished(false, QStringLiteral("Enable video, audio, or both before exporting."));
        return;
    }

    QString errorMessage;
    const QList<ClipSpec> clips = collectClips(timelineControllerObj, &errorMessage);
    if (!errorMessage.isEmpty()) {
        emit exportFinished(false, errorMessage);
        return;
    }

    const bool hasVideoClip = std::any_of(clips.cbegin(), clips.cend(), [](const ClipSpec& clip) {
        return clip.hasVideo;
    });
    const bool hasAudioClip = std::any_of(clips.cbegin(), clips.cend(), [](const ClipSpec& clip) {
        return clip.hasAudio;
    });
    if (settings.exportVideo && !hasVideoClip && !hasAudioClip) {
        emit exportFinished(false, QStringLiteral("The timeline has no exportable media."));
        return;
    }
    if (!settings.exportVideo && settings.exportAudio && !hasAudioClip) {
        emit exportFinished(false, QStringLiteral("The timeline has no audio clips to export."));
        return;
    }

    double totalDuration = 0.0;
    for (const ClipSpec& clip : clips) {
        totalDuration = std::max(totalDuration, clip.startSeconds + clip.durationSeconds);
    }
    if (!isFinitePositive(totalDuration)) {
        emit exportFinished(false, QStringLiteral("The timeline duration is invalid."));
        return;
    }

    const QString ffmpegPath = findFfmpegExecutable();
    if (ffmpegPath.isEmpty()) {
        emit exportFinished(false, QStringLiteral("FFmpeg was not found. Expected ffmpeg.exe beside the app or in third_party/ffmpeg/bin."));
        return;
    }

    const QString actualOutputPath = outputPathWithExtension(outputPath, settings);
    if (actualOutputPath.trimmed().isEmpty()) {
        emit exportFinished(false, QStringLiteral("Choose a valid export path."));
        return;
    }

    const QFileInfo outputInfo(actualOutputPath);
    QDir outputDir = outputInfo.absoluteDir();
    if (!outputDir.exists() && !outputDir.mkpath(QStringLiteral("."))) {
        emit exportFinished(false, QStringLiteral("Could not create the export folder: %1").arg(outputDir.absolutePath()));
        return;
    }

    m_currentClips = clips;
    m_currentSettings = settings;
    m_totalDurationSeconds = totalDuration;
    m_finalOutputPath = actualOutputPath;



    m_chunkDurationSeconds = 60.0;
    m_totalChunks = std::max(1, static_cast<int>(std::ceil(m_totalDurationSeconds / m_chunkDurationSeconds)));
    m_currentChunkIndex = 0;
    m_chunkFiles.clear();
    m_isConcatenating = false;

    m_cancelRequested = false;
    m_isExporting = true;
    emit exportingChanged();
    setProgress(0.0);
    emit exportStarted();

    exportNextChunk();
}

void VideoExporter::cancelExport()
{
    if (!m_process || !m_isExporting) {
        return;
    }

    m_cancelRequested = true;
    m_process->terminate();
    if (!m_process->waitForFinished(1200)) {
        m_process->kill();
    }
}

bool VideoExporter::isExporting() const
{
    return m_isExporting;
}

double VideoExporter::progress() const
{
    return m_progress;
}

void VideoExporter::onProcessReadyReadStandardOutput()
{
    if (!m_process) {
        return;
    }

    const QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const int separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) {
            continue;
        }

        const QString key = line.left(separator).trimmed();
        const QString value = line.mid(separator + 1).trimmed();
        if (key == QLatin1String("out_time_ms") || key == QLatin1String("out_time_us")) {
            bool ok = false;
            const qint64 microseconds = value.toLongLong(&ok);
            if (ok && m_totalDurationSeconds > 0.0) {
                double chunkProgress = microseconds / 1000000.0;
                double totalProgress = (m_currentChunkIndex * m_chunkDurationSeconds + chunkProgress) / m_totalDurationSeconds;
                setProgress(std::min(0.999, totalProgress));
            }
        } else if (key == QLatin1String("progress") && value == QLatin1String("end")) {
            double totalProgress = ((m_currentChunkIndex + 1) * m_chunkDurationSeconds) / m_totalDurationSeconds;
            setProgress(std::min(0.999, totalProgress));
        }
    }
}

void VideoExporter::onProcessReadyReadStandardError()
{
    if (!m_process) {
        return;
    }

    const QString output = QString::fromUtf8(m_process->readAllStandardError());
    if (!output.trimmed().isEmpty()) {
        m_lastErrorOutput += output;
        if (m_lastErrorOutput.size() > 8000) {
            m_lastErrorOutput = m_lastErrorOutput.right(8000);
        }
    }

    static const QRegularExpression timeRegex(QStringLiteral(R"(time=(\d{2}):(\d{2}):(\d{2}(?:\.\d+)?))"));
    QRegularExpressionMatchIterator matches = timeRegex.globalMatch(output);
    while (matches.hasNext()) {
        const QRegularExpressionMatch match = matches.next();
        const double currentTime = match.captured(1).toDouble() * 3600.0
            + match.captured(2).toDouble() * 60.0
            + match.captured(3).toDouble();
        if (m_totalDurationSeconds > 0.0 && !m_isConcatenating) {
            double totalProgress = (m_currentChunkIndex * m_chunkDurationSeconds + currentTime) / m_totalDurationSeconds;
            setProgress(std::min(0.999, totalProgress));
        }
    }
}

void VideoExporter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_cancelRequested) {
        cleanupChunks();
        finishExport(false, QStringLiteral("Export cancelled."));
        return;
    }

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (m_isConcatenating) {
            setProgress(1.0);
            cleanupChunks();
            finishExport(true, QStringLiteral("Export completed successfully."));
        } else {
            m_currentChunkIndex++;
            if (m_currentChunkIndex < m_totalChunks) {
                exportNextChunk();
            } else {
                startConcatenation();
            }
        }
        return;
    }

    QString detail = m_lastErrorOutput.trimmed();
    if (detail.length() > 700) {
        detail = detail.right(700);
    }
    cleanupChunks();
    finishExport(false,
                 detail.isEmpty()
                     ? QStringLiteral("Export failed with exit code %1.").arg(exitCode)
                     : QStringLiteral("Export failed with exit code %1: %2").arg(exitCode).arg(detail));
}

void VideoExporter::onProcessErrorOccurred(QProcess::ProcessError error)
{
    if (m_cancelRequested) {
        return;
    }

    finishExport(false, QStringLiteral("FFmpeg process error %1: %2").arg(error).arg(m_process ? m_process->errorString() : QString()));
}

void VideoExporter::exportNextChunk()
{
    if (m_cancelRequested) return;

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, &QProcess::readyReadStandardOutput, this, &VideoExporter::onProcessReadyReadStandardOutput);
        connect(m_process, &QProcess::readyReadStandardError, this, &VideoExporter::onProcessReadyReadStandardError);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &VideoExporter::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &VideoExporter::onProcessErrorOccurred);
    }

    double chunkStart = m_currentChunkIndex * m_chunkDurationSeconds;
    double chunkEnd = std::min(m_totalDurationSeconds, chunkStart + m_chunkDurationSeconds);

    QString chunkFileName = QStringLiteral("chunk_%1.mp4").arg(m_currentChunkIndex, 4, 10, QLatin1Char('0'));
    QFileInfo outputInfo(m_finalOutputPath);
    QDir outputDir = outputInfo.absoluteDir();
    QString chunkPath = outputDir.absoluteFilePath(chunkFileName);
    m_chunkFiles.append(chunkPath);

    QString chunkSubsPath;
    if (m_currentSettings.exportVideo) {
        QList<AdvancedSubtitleRenderer::SubtitleItem> subtitleClips;
        qDebug() << "[SUBTITLE-DEBUG] Total clips:" << m_currentClips.size()
                 << "chunkStart:" << chunkStart << "chunkEnd:" << chunkEnd;
        for (const ClipSpec& clip : m_currentClips) {
            if (!clip.isEffect && !clip.hasVideo && !clip.hasAudio && !clip.name.isEmpty()) {
                if (clip.startSeconds < chunkEnd && clip.startSeconds + clip.durationSeconds > chunkStart) {
                    AdvancedSubtitleRenderer::SubtitleItem sub;
                    sub.text = clip.name;
                    sub.startSeconds = clip.startSeconds;
                    sub.durationSeconds = clip.durationSeconds;
                    subtitleClips.push_back(sub);
                }
            }
        }
        qDebug() << "[SUBTITLE-DEBUG] Subtitle clips found:" << subtitleClips.size();
        if (!subtitleClips.isEmpty()) {
            qDebug() << "[SUBTITLE-DEBUG] Calling generate with font:" << m_currentSettings.subtitleFont
                     << "size:" << m_currentSettings.subtitleFontSize
                     << "width:" << m_currentSettings.width << "height:" << m_currentSettings.height;
            chunkSubsPath = AdvancedSubtitleRenderer::generate(
                subtitleClips,
                m_currentSettings.subtitleFont,
                m_currentSettings.subtitleFontSize,
                m_currentSettings.width,
                m_currentSettings.height,
                outputDir.absolutePath(),
                m_currentChunkIndex,
                chunkStart,
                chunkEnd,
                m_chunkSubsFiles,
                m_currentSettings.subtitleVerticalPosition
            );
            qDebug() << "[SUBTITLE-DEBUG] Generated concat path:" << chunkSubsPath;
        } else {
            qDebug() << "[SUBTITLE-DEBUG] NO subtitle clips found! Skipping generation.";
        }
    }

    QStringList arguments;
    try {
        arguments = buildFfmpegArguments(m_currentClips, m_currentSettings, chunkPath, chunkStart, chunkEnd, chunkSubsPath);
        qDebug() << "[SUBTITLE-DEBUG] FFmpeg command:" << arguments.join(" ");
    } catch (const std::exception& ex) {
        cleanupChunks();
        finishExport(false, QString::fromUtf8(ex.what()));
        return;
    }

    m_lastErrorOutput.clear();
    const QString ffmpegPath = findFfmpegExecutable();
    m_process->start(ffmpegPath, arguments);
}

void VideoExporter::startConcatenation()
{
    if (m_cancelRequested) return;

    m_isConcatenating = true;
    m_lastErrorOutput.clear();

    QFileInfo outputInfo(m_finalOutputPath);
    QDir outputDir = outputInfo.absoluteDir();
    QString listPath = outputDir.absoluteFilePath(QStringLiteral("concat.txt"));

    QFile listFile(listPath);
    if (listFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&listFile);
        for (const QString& file : m_chunkFiles) {
            out << QStringLiteral("file '") << QFileInfo(file).fileName() << QStringLiteral("'\n");
        }
        listFile.close();
    }

    QStringList arguments;
    arguments << QStringLiteral("-hide_banner") << QStringLiteral("-y")
              << QStringLiteral("-f") << QStringLiteral("concat")
              << QStringLiteral("-safe") << QStringLiteral("0")
              << QStringLiteral("-i") << listPath
              << QStringLiteral("-c") << QStringLiteral("copy")
              << m_finalOutputPath;

    const QString ffmpegPath = findFfmpegExecutable();
    m_process->start(ffmpegPath, arguments);
}

void VideoExporter::cleanupChunks()
{
    for (const QString& file : m_chunkFiles) {
        QFile::remove(file);
    }
    m_chunkFiles.clear();
    
    for (const QString& file : m_chunkSubsFiles) {
        QFile::remove(file);
    }
    m_chunkSubsFiles.clear();
    
    QFileInfo outputInfo(m_finalOutputPath);
    QDir outputDir = outputInfo.absoluteDir();
    QString listPath = outputDir.absoluteFilePath(QStringLiteral("concat.txt"));
    QFile::remove(listPath);
}

void VideoExporter::finishExport(bool success, const QString& message)
{
    if (m_process) {
        m_process->disconnect(this);
        m_process->deleteLater();
        m_process = nullptr;
    }

    if (m_isExporting) {
        m_isExporting = false;
        emit exportingChanged();
    }

    if (!success) {
        setProgress(0.0);
    }
    m_cancelRequested = false;
    emit exportFinished(success, message);
}

void VideoExporter::setProgress(double progress)
{
    const double clamped = std::clamp(progress, 0.0, 1.0);
    if (std::abs(m_progress - clamped) < 0.0005) {
        return;
    }

    m_progress = clamped;
    emit progressChanged();
    emit exportProgress(m_progress);
}

QString VideoExporter::findFfmpegExecutable() const
{
#ifdef Q_OS_WIN
    const QString executableName = QStringLiteral("ffmpeg.exe");
#else
    const QString executableName = QStringLiteral("ffmpeg");
#endif

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();
    const QStringList candidates = {
        QDir(appDir).absoluteFilePath(executableName),
        QDir(appDir).absoluteFilePath(QStringLiteral("../third_party/ffmpeg/bin/%1").arg(executableName)),
        QDir(currentDir).absoluteFilePath(QStringLiteral("third_party/ffmpeg/bin/%1").arg(executableName)),
        QStandardPaths::findExecutable(executableName)
    };

    for (const QString& candidate : candidates) {
        if (!candidate.isEmpty() && QFileInfo::exists(candidate)) {
            return QDir::toNativeSeparators(QFileInfo(candidate).absoluteFilePath());
        }
    }

    return {};
}

QString VideoExporter::normalizeLocalPath(const QString& path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        const QUrl url(trimmed);
        if (url.isLocalFile()) {
            return QDir::toNativeSeparators(url.toLocalFile());
        }
    }
    return QDir::toNativeSeparators(trimmed);
}

QString VideoExporter::outputPathWithExtension(const QString& outputPath, const ExportSettings& settings) const
{
    QString path = normalizeLocalPath(outputPath);
    if (path.trimmed().isEmpty()) {
        return {};
    }

    const QString desiredExtension = settings.exportVideo
        ? sanitizeExtension(settings.containerFormat)
        : sanitizeExtension(settings.audioFormat);
    QFileInfo info(path);
    if (info.suffix().isEmpty() && !desiredExtension.isEmpty()) {
        path += QLatin1Char('.') + desiredExtension;
    }
    return QDir::toNativeSeparators(path);
}

VideoExporter::ExportSettings VideoExporter::parseSettings(const QVariantMap& settings,
                                                           bool exportVideo,
                                                           bool exportAudio) const
{
    ExportSettings parsed;
    parsed.exportVideo = exportVideo;
    parsed.exportAudio = exportAudio;
    parsed.bitrateMode = firstNonEmpty(settings, {"bitrate", "bitRate", "bitrateMode"}, parsed.bitrateMode);
    parsed.videoCodec = firstNonEmpty(settings, {"codec", "videoCodec"}, parsed.videoCodec);
    parsed.containerFormat = firstNonEmpty(settings, {"format", "container", "containerFormat"}, parsed.containerFormat);
    parsed.audioFormat = firstNonEmpty(settings, {"audioFormat", "audioCodec"}, parsed.audioFormat);

    const QString resolution = firstNonEmpty(settings, {"resolution"}, QStringLiteral("1080P")).toUpper();
    if (resolution == QLatin1String("4K") || resolution == QLatin1String("2160P")) {
        parsed.width = 3840;
        parsed.height = 2160;
    } else if (resolution == QLatin1String("720P")) {
        parsed.width = 1280;
        parsed.height = 720;
    } else if (resolution == QLatin1String("480P")) {
        parsed.width = 854;
        parsed.height = 480;
    } else if (resolution == QLatin1String("360P")) {
        parsed.width = 640;
        parsed.height = 360;
    } else {
        parsed.width = 1920;
        parsed.height = 1080;
    }

    const QString frameRateText = firstNonEmpty(settings, {"frameRate", "fps"}, QStringLiteral("60 fps"));
    bool ok = false;
    const double fps = frameRateText.split(QLatin1Char(' ')).value(0).toDouble(&ok);
    parsed.frameRate = ok && fps > 0.0 ? fps : 60.0;

    parsed.containerFormat = parsed.containerFormat.toUpper();
    parsed.audioFormat = parsed.audioFormat.toUpper();
    parsed.subtitleFont = firstNonEmpty(settings, {"subtitleFont"}, parsed.subtitleFont);
    if (settings.contains(QStringLiteral("subtitleFontSize"))) {
        parsed.subtitleFontSize = settings.value(QStringLiteral("subtitleFontSize")).toInt();
        if (parsed.subtitleFontSize <= 0) {
            parsed.subtitleFontSize = 48;
        }
    }
    
    parsed.subtitleVerticalPosition = settings.value(QStringLiteral("subtitleVerticalPosition"), -1.0).toDouble();
    parsed.isVertical = settings.value(QStringLiteral("isVertical"), false).toBool();
    
    if (parsed.isVertical) {
        std::swap(parsed.width, parsed.height);
    }
    
    return parsed;
}

QList<VideoExporter::ClipSpec> VideoExporter::collectClips(QObject* timelineControllerObj, QString* errorMessage) const
{
    if (errorMessage) {
        errorMessage->clear();
    }
    if (!timelineControllerObj) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid timeline controller.");
        }
        return {};
    }

    const auto* timeline = qobject_cast<TimelineController*>(timelineControllerObj);
    if (!timeline) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("The export backend received an incompatible timeline controller.");
        }
        return {};
    }

    QList<ClipSpec> clips;
    const int count = timeline->clipCount();
    if (count <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Timeline is empty.");
        }
        return {};
    }

    const auto* clipModel = qobject_cast<TimelineClipModel*>(timeline->clipModel());

    for (int row = 0; row < count; ++row) {
        const QVariantMap clipMap = timeline->clipAt(row);
        ClipSpec clip;
        clip.name = clipMap.value(QStringLiteral("clipName")).toString();
        clip.filePath = normalizeLocalPath(clipMap.value(QStringLiteral("filePath")).toString());
        clip.startSeconds = std::max(0.0, clipMap.value(QStringLiteral("startSeconds")).toDouble());
        clip.durationSeconds = clipMap.value(QStringLiteral("durationSeconds")).toDouble();
        clip.trackIndex = clipMap.value(QStringLiteral("trackIndex")).toInt();
        clip.sourceIndex = row;
        clip.isEffect = clipMap.value(QStringLiteral("isEffect")).toBool();

        bool isAudioTrack = clip.trackIndex >= 100;
        int logicalTrackIdx = isAudioTrack ? clip.trackIndex - 100 : clip.trackIndex;

        clip.hasVideo = clipMap.value(QStringLiteral("hasVideo")).toBool() && !clip.isEffect;
        bool isMuted = clipMap.value(QStringLiteral("isMuted")).toBool();

        if (!isAudioTrack && timeline->isTrackHidden(true, logicalTrackIdx)) {
            clip.hasVideo = false;
        }

        if (timeline->isTrackMuted(!isAudioTrack, logicalTrackIdx)) {
            isMuted = true;
        }

        clip.hasAudio = clipMap.value(QStringLiteral("hasAudio")).toBool() && !isMuted && !clip.isEffect;
        if (clipModel) {
            clip.effects = clipModel->clipEffectsAt(row);
        }

        if (!isFinitePositive(clip.durationSeconds)) {
            continue;
        }
        if (!clip.isEffect && (clip.filePath.isEmpty() || !QFileInfo::exists(clip.filePath))) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Missing source media: %1").arg(clip.filePath);
            }
            return {};
        }

        clips.push_back(clip);
    }

    if (clips.isEmpty() && errorMessage) {
        *errorMessage = QStringLiteral("Timeline has no clips with a valid duration.");
    }
    return clips;
}

QStringList VideoExporter::buildFfmpegArguments(const QList<ClipSpec>& clips,
                                                const ExportSettings& settings,
                                                const QString& outputPath,
                                                double chunkStart,
                                                double chunkEnd,
                                                const QString& chunkSubsPath) const
{
    double chunkDuration = chunkEnd - chunkStart;

    QList<ClipSpec> videoClips;
    QList<ClipSpec> audioClips;
    QList<ClipSpec> effectClips;
    for (const ClipSpec& clip : clips) {
        if (clip.startSeconds >= chunkEnd || clip.startSeconds + clip.durationSeconds <= chunkStart) {
            continue;
        }
        if (settings.exportVideo && clip.isEffect) {
            effectClips.push_back(clip);
        } else if (settings.exportVideo && clip.hasVideo) {
            videoClips.push_back(clip);
        }
        if (settings.exportAudio && clip.hasAudio) {
            audioClips.push_back(clip);
        }
    }

    std::sort(videoClips.begin(), videoClips.end(), [](const ClipSpec& left, const ClipSpec& right) {
        if (left.trackIndex != right.trackIndex) {
            return left.trackIndex > right.trackIndex;
        }
        if (!qFuzzyCompare(left.startSeconds, right.startSeconds)) {
            return left.startSeconds < right.startSeconds;
        }
        return left.sourceIndex < right.sourceIndex;
    });

    std::sort(effectClips.begin(), effectClips.end(), [](const ClipSpec& left, const ClipSpec& right) {
        if (left.trackIndex != right.trackIndex) {
            return left.trackIndex > right.trackIndex;
        }
        if (!qFuzzyCompare(left.startSeconds, right.startSeconds)) {
            return left.startSeconds < right.startSeconds;
        }
        return left.sourceIndex < right.sourceIndex;
    });

    CodecPlan plan;
    plan.muxer = settings.exportVideo ? settings.containerFormat.toLower() : settings.audioFormat.toLower();
    plan.extension = plan.muxer;
    if (plan.muxer == QLatin1String("mp4")) {
        plan.muxer = QStringLiteral("mp4");
    } else if (plan.muxer == QLatin1String("mov")) {
        plan.muxer = QStringLiteral("mov");
    } else if (plan.muxer == QLatin1String("mkv")) {
        plan.muxer = QStringLiteral("matroska");
    } else if (plan.muxer == QLatin1String("webm")) {
        plan.muxer = QStringLiteral("webm");
    } else if (plan.muxer == QLatin1String("aac")) {
        plan.muxer = QStringLiteral("adts");
    } else if (plan.muxer == QLatin1String("wav")) {
        plan.muxer = QStringLiteral("wav");
    } else if (plan.muxer == QLatin1String("ogg")) {
        plan.muxer = QStringLiteral("ogg");
    }

    const QString bitrateKey = settings.bitrateMode.trimmed().toLower();
    const double rateMultiplier = settings.frameRate > 30.0 ? 1.25 : 1.0;
    int recommendedKbps = 8000;
    if (settings.width >= 3840) {
        recommendedKbps = 42000;
    } else if (settings.width >= 1920) {
        recommendedKbps = 12000;
    } else if (settings.width >= 1280) {
        recommendedKbps = 6500;
    } else if (settings.width >= 854) {
        recommendedKbps = 3200;
    } else {
        recommendedKbps = 1800;
    }
    recommendedKbps = static_cast<int>(recommendedKbps * rateMultiplier);
    const int targetKbps = bitrateKey.contains(QLatin1String("higher"))
        ? static_cast<int>(recommendedKbps * 1.55)
        : recommendedKbps;

    const QString codecKey = settings.videoCodec.trimmed().toUpper();
    if (settings.exportVideo) {
        auto hwEncoder = HardwareDetector::detectBestEncoder(findFfmpegExecutable());

        if (settings.containerFormat == QLatin1String("WEBM")) {
            if (codecKey.contains(QLatin1String("AV1"))) {
                plan.videoEncoder = QStringLiteral("libsvtav1");
                plan.videoArgs << QStringLiteral("-preset") << QStringLiteral("8");
            } else {
                plan.videoEncoder = QStringLiteral("libvpx-vp9");
                plan.videoArgs << QStringLiteral("-deadline") << QStringLiteral("good")
                               << QStringLiteral("-cpu-used") << QStringLiteral("4");
            }
        } else if (codecKey.contains(QLatin1String("HEVC")) || codecKey.contains(QLatin1String("H.265"))) {
            if (hwEncoder) {
                plan.videoEncoder = hwEncoder->hevcEncoder();
                plan.videoArgs.append(hwEncoder->extraArgs());
            } else {
                plan.videoEncoder = QStringLiteral("libx265");
                plan.videoArgs << QStringLiteral("-preset") << QStringLiteral("medium");
            }
            if (settings.containerFormat == QLatin1String("MP4") || settings.containerFormat == QLatin1String("MOV")) {
                plan.videoArgs << QStringLiteral("-tag:v") << QStringLiteral("hvc1");
            }
        } else if (codecKey.contains(QLatin1String("AV1"))) {
            plan.videoEncoder = QStringLiteral("libsvtav1");
            plan.videoArgs << QStringLiteral("-preset") << QStringLiteral("8");
        } else {
            if (hwEncoder) {
                plan.videoEncoder = hwEncoder->h264Encoder();
                plan.videoArgs.append(hwEncoder->extraArgs());
                plan.videoArgs << QStringLiteral("-profile:v") << QStringLiteral("high");
            } else {
                plan.videoEncoder = QStringLiteral("libx264");
                plan.videoArgs << QStringLiteral("-preset") << QStringLiteral("fast")
                               << QStringLiteral("-profile:v") << QStringLiteral("high");
            }
        }

        plan.videoArgs << QStringLiteral("-b:v") << QStringLiteral("%1k").arg(targetKbps)
                       << QStringLiteral("-maxrate") << QStringLiteral("%1k").arg(static_cast<int>(targetKbps * 1.4))
                       << QStringLiteral("-bufsize") << QStringLiteral("%1k").arg(static_cast<int>(targetKbps * 2.0))
                       << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p");
    }

    if (settings.exportAudio) {
        const QString audioKey = settings.audioFormat.trimmed().toUpper();
        if (settings.containerFormat == QLatin1String("WEBM")) {
            plan.audioEncoder = QStringLiteral("libopus");
            plan.audioArgs << QStringLiteral("-b:a") << QStringLiteral("160k");
        } else if (!settings.exportVideo && audioKey == QLatin1String("MP3")) {
            plan.audioEncoder = QStringLiteral("libmp3lame");
            plan.audioArgs << QStringLiteral("-b:a") << QStringLiteral("192k");
        } else if (!settings.exportVideo && audioKey == QLatin1String("WAV")) {
            plan.audioEncoder = QStringLiteral("pcm_s16le");
        } else if (!settings.exportVideo && audioKey == QLatin1String("FLAC")) {
            plan.audioEncoder = QStringLiteral("flac");
        } else if (!settings.exportVideo && audioKey == QLatin1String("OGG")) {
            plan.audioEncoder = QStringLiteral("libvorbis");
            plan.audioArgs << QStringLiteral("-q:a") << QStringLiteral("5");
        } else {
            plan.audioEncoder = QStringLiteral("aac");
            plan.audioArgs << QStringLiteral("-b:a") << QStringLiteral("192k");
        }
    }

    QStringList args;
    args << QStringLiteral("-hide_banner")
         << QStringLiteral("-y")
         << QStringLiteral("-nostdin")
         << QStringLiteral("-progress") << QStringLiteral("pipe:1")
         << QStringLiteral("-stats_period") << QStringLiteral("0.25");

    int nextInputIndex = 0;
    int baseInputIndex = -1;
    if (settings.exportVideo) {
        baseInputIndex = nextInputIndex++;
        args << QStringLiteral("-f") << QStringLiteral("lavfi")
             << QStringLiteral("-i")
             << QStringLiteral("color=c=black:s=%1x%2:r=%3:d=%4")
                    .arg(settings.width)
                    .arg(settings.height)
                    .arg(number(settings.frameRate))
                    .arg(number(chunkDuration));
    }

    QHash<int, int> inputIndexBySourceRow;
    for (const ClipSpec& clip : clips) {
        if (clip.startSeconds >= chunkEnd || clip.startSeconds + clip.durationSeconds <= chunkStart) {
            continue;
        }
        const bool needInput = (settings.exportVideo && clip.hasVideo) || (settings.exportAudio && clip.hasAudio);
        if (!needInput) {
            continue;
        }
        inputIndexBySourceRow.insert(clip.sourceIndex, nextInputIndex++);
        args << QStringLiteral("-i") << clip.filePath;
    }

    int subsInputIndex = -1;
    if (!chunkSubsPath.isEmpty()) {
        subsInputIndex = nextInputIndex++;
        args << QStringLiteral("-f") << QStringLiteral("concat")
             << QStringLiteral("-safe") << QStringLiteral("0")
             << QStringLiteral("-i") << chunkSubsPath;
    }

    QStringList filters;
    QString videoOutLabel;
    if (settings.exportVideo) {
        filters << QStringLiteral("[%1:v]trim=duration=%2,setpts=PTS-STARTPTS,format=rgba[base0]")
                       .arg(baseInputIndex)
                       .arg(number(chunkDuration));
        QString previous = QStringLiteral("base0");
        int overlayIndex = 0;
        for (const ClipSpec& clip : videoClips) {
            const int inputIndex = inputIndexBySourceRow.value(clip.sourceIndex, -1);
            if (inputIndex < 0) {
                continue;
            }

            double overlapStartTimeline = std::max(chunkStart, clip.startSeconds);
            double overlapEndTimeline = std::min(chunkEnd, clip.startSeconds + clip.durationSeconds);
            double overlapDuration = overlapEndTimeline - overlapStartTimeline;
            double sourceStart = overlapStartTimeline - clip.startSeconds;
            double timelineStartInChunk = overlapStartTimeline - chunkStart;

            const QString scaled = QStringLiteral("v%1_scaled").arg(overlayIndex);
            const QString prepared = QStringLiteral("v%1").arg(overlayIndex);
            const QString output = QStringLiteral("vbase%1").arg(overlayIndex + 1);
            filters << QStringLiteral("[%1:v]trim=start=%2:duration=%3,setpts=PTS-STARTPTS+%4/TB,fps=%5,scale=%6:%7:force_original_aspect_ratio=decrease,setsar=1,format=rgba[%8]")
                           .arg(inputIndex)
                           .arg(number(sourceStart))
                           .arg(number(overlapDuration))
                           .arg(number(timelineStartInChunk))
                           .arg(number(settings.frameRate))
                           .arg(settings.width)
                           .arg(settings.height)
                           .arg(scaled);

            const QString effected = appendBlurFilters(filters, scaled, clip.effects, overlayIndex);
            filters << QStringLiteral("[%1]pad=%2:%3:(ow-iw)/2:(oh-ih)/2:color=black,setsar=1,format=rgba[%4]")
                           .arg(effected)
                           .arg(settings.width)
                           .arg(settings.height)
                           .arg(prepared);
            filters << QStringLiteral("[%1][%2]overlay=shortest=0:eof_action=pass:format=auto[%3]")
                           .arg(previous)
                           .arg(prepared)
                           .arg(output);
            previous = output;
            ++overlayIndex;
        }

        int timelineEffectIndex = 0;
        for (const ClipSpec& effectClip : effectClips) {
            const double overlapStartTimeline = std::max(chunkStart, effectClip.startSeconds);
            const double overlapEndTimeline = std::min(chunkEnd, effectClip.startSeconds + effectClip.durationSeconds);
            if (overlapEndTimeline <= overlapStartTimeline) {
                continue;
            }

            const double enableStart = overlapStartTimeline - chunkStart;
            const double enableEnd = overlapEndTimeline - chunkStart;
            const QString enable = ffmpegEnableBetween(enableStart, enableEnd);

            if (hasVisibleColorAdjustment(effectClip.effects)) {
                const QString output = QStringLiteral("timeline_fx%1_color").arg(timelineEffectIndex++);
                const double brightness = std::clamp(effectClip.effects.color.brightness / 100.0, -1.0, 1.0);
                const double contrast = std::max(0.0, effectClip.effects.color.contrast / 100.0);
                const double saturation = std::max(0.0, effectClip.effects.color.saturation / 100.0);
                filters << QStringLiteral("[%1]eq=brightness=%2:contrast=%3:saturation=%4:%5[%6]")
                               .arg(previous,
                                    expressionNumber(brightness),
                                    expressionNumber(contrast),
                                    expressionNumber(saturation),
                                    enable,
                                    output);
                previous = output;
            }

            const BlurEffectData blur = normalizedBlur(effectClip.effects.blur);
            if (blur.radius > 0.0) {
                const QString output = QStringLiteral("timeline_fx%1_blur").arg(timelineEffectIndex++);
                filters << QStringLiteral("[%1]%2:%3[%4]")
                               .arg(previous,
                                    gaussianBlurFilter(blur.radius),
                                    enable,
                                    output);
                previous = output;
            }
        }

        filters << QStringLiteral("[%1]trim=duration=%2,setpts=PTS-STARTPTS,format=yuv420p[vout]")
                       .arg(previous)
                       .arg(number(chunkDuration));
        previous = QStringLiteral("vout");

        if (subsInputIndex >= 0) {
            filters << QStringLiteral("[%1:v]fps=%2[sub_fps]")
                           .arg(subsInputIndex)
                           .arg(number(settings.frameRate));
            filters << QStringLiteral("[%1][sub_fps]overlay=shortest=0:eof_action=pass:format=auto[%2]")
                           .arg(previous)
                           .arg(QStringLiteral("vout_sub"));
            previous = QStringLiteral("vout_sub");
        }
        videoOutLabel = QStringLiteral("[%1]").arg(previous);
    }

    QString audioOutLabel;
    if (settings.exportAudio && !audioClips.isEmpty()) {
        QStringList audioLabels;
        int audioIndex = 0;
        for (const ClipSpec& clip : audioClips) {
            const int inputIndex = inputIndexBySourceRow.value(clip.sourceIndex, -1);
            if (inputIndex < 0) {
                continue;
            }

            double overlapStartTimeline = std::max(chunkStart, clip.startSeconds);
            double overlapEndTimeline = std::min(chunkEnd, clip.startSeconds + clip.durationSeconds);
            double overlapDuration = overlapEndTimeline - overlapStartTimeline;
            double sourceStart = overlapStartTimeline - clip.startSeconds;
            double timelineStartInChunk = overlapStartTimeline - chunkStart;

            const QString label = QStringLiteral("a%1").arg(audioIndex);
            QString chain = QStringLiteral("[%1:a]atrim=start=%2:duration=%3,asetpts=PTS-STARTPTS,aresample=48000,aformat=sample_rates=48000:channel_layouts=stereo")
                                .arg(inputIndex)
                                .arg(number(sourceStart))
                                .arg(number(overlapDuration));
            const int delayMs = roundedDelayMs(timelineStartInChunk);
            if (delayMs > 0) {
                chain += QStringLiteral(",adelay=%1:all=1").arg(delayMs);
            }
            chain += QStringLiteral(",apad=whole_dur=%1,atrim=duration=%1[%2]")
                         .arg(number(chunkDuration))
                         .arg(label);
            filters << chain;
            audioLabels << QStringLiteral("[%1]").arg(label);
            ++audioIndex;
        }

        if (!audioLabels.isEmpty()) {
            filters << QStringLiteral("%1amix=inputs=%2:duration=longest:dropout_transition=0:normalize=0,atrim=duration=%3,asetpts=N/SR/TB[aout]")
                           .arg(audioLabels.join(QString()))
                           .arg(audioLabels.size())
                           .arg(number(chunkDuration));
            audioOutLabel = QStringLiteral("[aout]");
        }
    }

    if (!filters.isEmpty()) {
        args << QStringLiteral("-filter_complex") << filters.join(QLatin1Char(';'));
    }

    if (!videoOutLabel.isEmpty()) {
        args << QStringLiteral("-map") << videoOutLabel
             << QStringLiteral("-c:v") << plan.videoEncoder;
        args.append(plan.videoArgs);
    }
    if (!audioOutLabel.isEmpty()) {
        args << QStringLiteral("-map") << audioOutLabel
             << QStringLiteral("-c:a") << plan.audioEncoder;
        args.append(plan.audioArgs);
    }

    if (settings.exportVideo && videoOutLabel.isEmpty()) {
        throw std::runtime_error("No video stream could be built for export.");
    }
    if (!settings.exportVideo && settings.exportAudio && audioOutLabel.isEmpty()) {
        throw std::runtime_error("No audio stream could be built for export.");
    }

    if (settings.exportVideo && (settings.containerFormat == QLatin1String("MP4") || settings.containerFormat == QLatin1String("MOV"))) {
        args << QStringLiteral("-movflags") << QStringLiteral("+faststart");
    }
    args << QStringLiteral("-t") << number(chunkDuration)       
         << QStringLiteral("-f") << plan.muxer
         << outputPath;
    return args;
}
