#include "VocalIsolator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QUuid>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QProcessEnvironment>
#include <QtConcurrent>
#include "dsp/VocalRemover.h"

#ifdef Q_OS_WIN
extern "C" __declspec(dllimport) int __stdcall CreateHardLinkW(
    const wchar_t* lpFileName,
    const wchar_t* lpExistingFileName,
    void* lpSecurityAttributes
);
#endif

namespace {

QString sanitizedStem(const QString& value)
{
    QString sanitized;
    sanitized.reserve(value.size());
    for (const QChar ch : value) {
        const ushort code = ch.unicode();
        const bool isAsciiLetter = (code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z');
        const bool isAsciiNumber = code >= '0' && code <= '9';
        sanitized.append(isAsciiLetter || isAsciiNumber || ch == QLatin1Char('-') || ch == QLatin1Char('_')
            ? ch
            : QLatin1Char('_'));
    }

    while (sanitized.contains(QStringLiteral("__"))) {
        sanitized.replace(QStringLiteral("__"), QStringLiteral("_"));
    }
    sanitized = sanitized.trimmed();
    while (sanitized.startsWith(QLatin1Char('_'))) {
        sanitized.remove(0, 1);
    }
    while (sanitized.endsWith(QLatin1Char('_'))) {
        sanitized.chop(1);
    }
    if (sanitized.isEmpty()) {
        sanitized = QStringLiteral("clip");
    }
    return sanitized.left(48);
}

QString cacheKeyForSource(const QFileInfo& sourceInfo)
{
    QString normalizedPath = sourceInfo.canonicalFilePath();
    if (normalizedPath.isEmpty()) {
        normalizedPath = sourceInfo.absoluteFilePath();
    }
#ifdef Q_OS_WIN
    normalizedPath = normalizedPath.toCaseFolded();
#endif
    const QString fingerprint = normalizedPath
        + QLatin1Char('|') + QString::number(sourceInfo.size())
        + QLatin1Char('|') + sourceInfo.lastModified().toUTC().toString(Qt::ISODateWithMs);
    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(fingerprint.toUtf8(), QCryptographicHash::Sha1).toHex().left(12)
    );
    return sanitizedStem(sourceInfo.completeBaseName()) + QLatin1Char('_') + hash;
}

void removeTempInput(const QString& tempFilePath, const QString& tempWorkDir)
{
    if (!tempFilePath.isEmpty()) {
        QFile::remove(tempFilePath);
    }
    if (!tempWorkDir.isEmpty()) {
        QDir(tempWorkDir).removeRecursively();
    }
}

} // namespace

VocalIsolator::VocalIsolator(QObject* parent) : QObject(parent)
{
}

VocalIsolator::~VocalIsolator()
{
    for (auto& data : m_processes) {
        if (!data.watcher->isFinished()) {
            data.watcher->cancel();
            data.watcher->waitForFinished();
        }
        delete data.watcher;
    }
}

void VocalIsolator::requestIsolation(int clipIndex, const QString& sourceFilePath, int isolationType)
{
    if (isolationType != 1 && isolationType != 2) {
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }

    if (m_processes.contains(clipIndex)) {
        qDebug() << "Already processing clip index" << clipIndex;
        return;
    }

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists("vocals") && !dir.mkpath("vocals")) {
        qWarning() << "Failed to create vocal isolation output directory:" << dir.absoluteFilePath("vocals");
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }
    QString outputDir = dir.absoluteFilePath("vocals");

    QFileInfo sourceInfo(sourceFilePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        qWarning() << "Missing source file for vocal isolation:" << sourceFilePath;
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }

    QString cacheKey = cacheKeyForSource(sourceInfo);
    QString uvrDir = QDir(outputDir).absoluteFilePath("uvr/" + cacheKey);
    
    QString vocalsPath = QDir(uvrDir).absoluteFilePath("vocals.wav");
    QString noVocalsPath = QDir(uvrDir).absoluteFilePath("no_vocals.wav");

    QString resultFilePath;
    if (isolationType == 1) {
        resultFilePath = noVocalsPath;
    } else if (isolationType == 2) {
        resultFilePath = vocalsPath;
    }

    if (QFile::exists(vocalsPath) && QFile::exists(noVocalsPath)) {
        qDebug() << "Isolated files already exist for" << sourceFilePath;
        emit isolationFinished(clipIndex, isolationType, resultFilePath, true);
        return;
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString uuidBase = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString tempWorkDir = QDir(tempDir).absoluteFilePath("we_hunting_vocals/" + uuidBase);
    if (!QDir().mkpath(tempWorkDir)) {
        qWarning() << "Failed to create temp directory for vocal isolation:" << tempWorkDir;
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }
    const QString suffix = sourceInfo.suffix().isEmpty() ? QStringLiteral("media") : sourceInfo.suffix();
    QString tempFilePath = QDir(tempWorkDir).absoluteFilePath(cacheKey + "." + suffix);
    
    bool linkSuccess = false;
#ifdef Q_OS_WIN
    if (CreateHardLinkW((const wchar_t*)tempFilePath.utf16(), (const wchar_t*)sourceFilePath.utf16(), nullptr)) {
        linkSuccess = true;
    }
#endif
    if (!linkSuccess) {
        if (!QFile::copy(sourceFilePath, tempFilePath)) {
            qWarning() << "Failed to create temp file for vocal isolation:" << tempFilePath;
            QDir(tempWorkDir).removeRecursively();
            emit isolationFinished(clipIndex, isolationType, "", false);
            return;
        }
    }

    QDir().mkpath(uvrDir);

    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    
    ProcessData pd;
    pd.watcher = watcher;
    pd.sourceFilePath = sourceFilePath;
    pd.isolationType = isolationType;
    pd.outputDir = outputDir;
    pd.tempFilePath = tempFilePath;
    pd.tempWorkDir = tempWorkDir;
    pd.cacheKey = cacheKey;
    m_processes[clipIndex] = pd;

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, clipIndex]() {
        onProcessFinished(clipIndex);
    });

    QFuture<bool> future = QtConcurrent::run([this, tempFilePath, resultFilePath, isolationType, clipIndex]() -> bool {
        vocal_advance::VocalRemover remover;
        vocal_advance::VocalRemover::Config config;
        config.isolationType = isolationType;
        config.useAiModel = true;
        
        // Find the absolute path of the downloaded Demucs ONNX model
        // We know it's in c:\we_hunting\video_studio\build_mingw\models\htdemucs_ft_vocals.onnx
        config.modelPath = QStringLiteral("C:/we_hunting/video_studio/build_mingw/models/htdemucs_ft_vocals.onnx");
        
        bool success = remover.process(tempFilePath, resultFilePath, config, [this, clipIndex](int progress) {
            emit progressChanged(clipIndex, progress);
        });

        return success;
    });

    watcher->setFuture(future);
}

void VocalIsolator::cancelIsolation(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) {
        return;
    }

    ProcessData pd = m_processes.take(clipIndex);
    pd.watcher->disconnect(this);
    if (!pd.watcher->isFinished()) {
        pd.watcher->cancel();
        pd.watcher->waitForFinished();
    }
    removeTempInput(pd.tempFilePath, pd.tempWorkDir);
    pd.watcher->deleteLater();
}

void VocalIsolator::onProcessFinished(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) return;
    
    ProcessData pd = m_processes.take(clipIndex);
    bool success = pd.watcher->result();
    
    if (!success) {
        qWarning() << "VocalRemover process failed.";
    }
    
    QString uvrDir = QDir(pd.outputDir).absoluteFilePath("uvr/" + pd.cacheKey);
    
    QString resultFilePath;
    if (pd.isolationType == 1) {
        resultFilePath = QDir(uvrDir).absoluteFilePath("no_vocals.wav");
    } else if (pd.isolationType == 2) {
        resultFilePath = QDir(uvrDir).absoluteFilePath("vocals.wav");
    }

    success = success && QFileInfo::exists(resultFilePath);
    if (!success) {
        qWarning() << "VocalRemover did not produce expected output:" << resultFilePath;
        QDir outputDirObj(uvrDir);
        if (outputDirObj.exists()) {
            outputDirObj.removeRecursively();
        }
    }

    emit isolationFinished(clipIndex, pd.isolationType, resultFilePath, success);
    
    removeTempInput(pd.tempFilePath, pd.tempWorkDir);
    pd.watcher->deleteLater();
}
