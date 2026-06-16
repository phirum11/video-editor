#ifndef VIDEOEXPORTER_H
#define VIDEOEXPORTER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/EffectData.h"

class VideoExporter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool exporting READ isExporting NOTIFY exportingChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    
public:
    explicit VideoExporter(QObject *parent = nullptr);
    ~VideoExporter();

    Q_INVOKABLE void startExport(QObject* timelineControllerObj, 
                                 const QString& outputPath, 
                                 bool exportVideo, 
                                 bool exportAudio);

    Q_INVOKABLE void startExportWithSettings(QObject* timelineControllerObj,
                                             const QString& outputPath,
                                             const QVariantMap& settings);
    Q_INVOKABLE void cancelExport();

    bool isExporting() const;
    double progress() const;

signals:
    void exportStarted();
    void exportProgress(double progress);
    void exportFinished(bool success, const QString& message);
    void exportingChanged();
    void progressChanged();

private slots:
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

    void exportNextChunk();
    void startConcatenation();
    void cleanupChunks();

private:
    struct ClipSpec {
        QString name;
        QString filePath;
        double startSeconds = 0.0;
        double durationSeconds = 0.0;
        int trackIndex = 0;
        int sourceIndex = 0;
        bool hasVideo = false;
        bool hasAudio = false;
        ClipEffects effects;
    };

    struct ExportSettings {
        bool exportVideo = true;
        bool exportAudio = true;
        bool isVertical = false;
        int width = 1920;
        int height = 1080;
        double frameRate = 60.0;
        QString bitrateMode = QStringLiteral("Higher");
        QString videoCodec = QStringLiteral("H.264");
        QString containerFormat = QStringLiteral("MP4");
        QString audioFormat = QStringLiteral("AAC");
        QString subtitleFont = QStringLiteral("Khmer UI");
        int subtitleFontSize = 48;
        double subtitleVerticalPosition = -1.0;
    };

    void finishExport(bool success, const QString& message);
    void setProgress(double progress);
    QString findFfmpegExecutable() const;
    QString normalizeLocalPath(const QString& path) const;
    QString outputPathWithExtension(const QString& outputPath, const ExportSettings& settings) const;
    QStringList buildFfmpegArguments(const QList<ClipSpec>& clips,
                                     const ExportSettings& settings,
                                     const QString& outputPath,
                                     double chunkStart,
                                     double chunkEnd,
                                     const QString& chunkSubsPath) const;
    ExportSettings parseSettings(const QVariantMap& settings,
                                 bool exportVideo,
                                 bool exportAudio) const;
    QList<ClipSpec> collectClips(QObject* timelineControllerObj, QString* errorMessage) const;

    QProcess* m_process = nullptr;
    double m_totalDurationSeconds = 0.0;
    double m_progress = 0.0;
    bool m_isExporting = false;
    bool m_cancelRequested = false;
    QString m_lastErrorOutput;

    int m_currentChunkIndex = 0;
    int m_totalChunks = 0;
    double m_chunkDurationSeconds = 60.0;
    QStringList m_chunkFiles;
    QStringList m_chunkSubsFiles;
    QString m_finalOutputPath;
    ExportSettings m_currentSettings;
    QList<ClipSpec> m_currentClips;
    bool m_isConcatenating = false;
};

#endif // VIDEOEXPORTER_H
