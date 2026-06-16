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
        if (data.process->state() != QProcess::NotRunning) {
            data.process->kill();
            data.process->waitForFinished();
        }
        delete data.process;
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
    QString htdemucsDir = QDir(outputDir).absoluteFilePath("htdemucs/" + cacheKey);
    
    QString vocalsPath = QDir(htdemucsDir).absoluteFilePath("vocals.wav");
    QString noVocalsPath = QDir(htdemucsDir).absoluteFilePath("no_vocals.wav");

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

    QProcess* process = new QProcess(this);
    
    ProcessData pd;
    pd.process = process;
    pd.sourceFilePath = sourceFilePath;
    pd.isolationType = isolationType;
    pd.outputDir = outputDir;
    pd.tempFilePath = tempFilePath;
    pd.tempWorkDir = tempWorkDir;
    pd.cacheKey = cacheKey;
    pd.stderrBuffer = "";
    m_processes[clipIndex] = pd;

    connect(process, &QProcess::readyReadStandardError, this, [this, clipIndex]() {
        onProcessReadyReadStandardError(clipIndex);
    });
    connect(process, &QProcess::finished, this, [this, clipIndex](int exitCode, QProcess::ExitStatus exitStatus) {
        onProcessFinished(clipIndex, exitCode, exitStatus);
    });
    connect(process, &QProcess::errorOccurred, this, [this, clipIndex](QProcess::ProcessError error) {
        onProcessErrorOccurred(clipIndex, error);
    });

    const QString pythonExecutable = QStandardPaths::findExecutable(QStringLiteral("python"));
    if (pythonExecutable.isEmpty()) {
        qWarning() << "Python executable was not found for vocal isolation.";
        m_processes.remove(clipIndex);
        removeTempInput(tempFilePath, tempWorkDir);
        process->deleteLater();
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString runnerPath = QDir(appDir).absoluteFilePath(QStringLiteral("demucs_runner.py"));
    if (!QFileInfo::exists(runnerPath)) {
        qWarning() << "Demucs runner was not found:" << runnerPath;
        m_processes.remove(clipIndex);
        removeTempInput(tempFilePath, tempWorkDir);
        process->deleteLater();
        emit isolationFinished(clipIndex, isolationType, QString(), false);
        return;
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString currentPath = environment.value(QStringLiteral("PATH"));
    environment.insert(
        QStringLiteral("PATH"),
        appDir + (currentPath.isEmpty() ? QString() : QStringLiteral(";") + currentPath)
    );
    process->setProcessEnvironment(environment);
    process->setWorkingDirectory(appDir);

    QStringList args;
    args << runnerPath
         << "--two-stems" << "vocals"
         << "--segment" << "7"
         << "--jobs" << "1"
         << tempFilePath
         << "-o" << outputDir;
    
    qDebug() << "Starting demucs:" << pythonExecutable << args;
    process->start(pythonExecutable, args);
}

void VocalIsolator::cancelIsolation(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) {
        return;
    }

    ProcessData pd = m_processes.take(clipIndex);
    pd.process->disconnect(this);
    if (pd.process->state() != QProcess::NotRunning) {
        pd.process->terminate();
        if (!pd.process->waitForFinished(1500)) {
            pd.process->kill();
            pd.process->waitForFinished(1500);
        }
    }
    removeTempInput(pd.tempFilePath, pd.tempWorkDir);
    pd.process->deleteLater();
}

void VocalIsolator::onProcessReadyReadStandardError(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) return;
    
    QProcess* process = m_processes[clipIndex].process;
    QString errOutput = QString::fromUtf8(process->readAllStandardError());
    m_processes[clipIndex].stderrBuffer.append(errOutput);
    
    static QRegularExpression re("(\\d+)%");
    QRegularExpressionMatch match = re.match(errOutput);
    // Find the last match in the string
    int lastProgress = -1;
    QRegularExpressionMatchIterator i = re.globalMatch(errOutput);
    while (i.hasNext()) {
        QRegularExpressionMatch m = i.next();
        lastProgress = m.captured(1).toInt();
    }
    
    if (lastProgress >= 0) {
        emit progressChanged(clipIndex, lastProgress);
    }
}

void VocalIsolator::onProcessFinished(int clipIndex, int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_processes.contains(clipIndex)) return;
    
    ProcessData pd = m_processes.take(clipIndex);
    bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
    
    if (!success) {
        qWarning() << "demucs process failed. Exit code:" << exitCode;
        qWarning() << pd.stderrBuffer;
    }
    
    QString htdemucsDir = QDir(pd.outputDir).absoluteFilePath("htdemucs/" + pd.cacheKey);
    
    QString resultFilePath;
    if (pd.isolationType == 1) {
        resultFilePath = QDir(htdemucsDir).absoluteFilePath("no_vocals.wav");
    } else if (pd.isolationType == 2) {
        resultFilePath = QDir(htdemucsDir).absoluteFilePath("vocals.wav");
    }

    success = success && QFileInfo::exists(resultFilePath);
    if (!success) {
        qWarning() << "demucs did not produce expected output:" << resultFilePath;
        QDir outputDir(htdemucsDir);
        if (outputDir.exists()) {
            outputDir.removeRecursively();
        }
    }

    emit isolationFinished(clipIndex, pd.isolationType, resultFilePath, success);
    
    removeTempInput(pd.tempFilePath, pd.tempWorkDir);
    pd.process->deleteLater();
}

void VocalIsolator::onProcessErrorOccurred(int clipIndex, QProcess::ProcessError error)
{
    if (!m_processes.contains(clipIndex)) return;

    ProcessData pd = m_processes.take(clipIndex);
    qWarning() << "demucs process error:" << error << pd.process->errorString();
    emit isolationFinished(clipIndex, pd.isolationType, QString(), false);
    removeTempInput(pd.tempFilePath, pd.tempWorkDir);
    pd.process->deleteLater();
}
