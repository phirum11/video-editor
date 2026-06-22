#include "AIVoiceGenerator.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QUuid>
#include <QRegularExpression>
#include <QProcessEnvironment>

AIVoiceGenerator::AIVoiceGenerator(QObject* parent) : QObject(parent)
{
}

AIVoiceGenerator::~AIVoiceGenerator()
{
    for (auto& data : m_processes) {
        if (data.process->state() != QProcess::NotRunning) {
            data.process->kill();
            data.process->waitForFinished();
        }
        delete data.process;
    }
}

void AIVoiceGenerator::requestGeneration(int clipIndex, const QString& srtFilePath, const QString& language)
{
    if (m_processes.contains(clipIndex)) {
        qDebug() << "Already generating voice for clip index" << clipIndex;
        return;
    }

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists("ai_voices") && !dir.mkpath("ai_voices")) {
        qWarning() << "Failed to create AI voice output directory:" << dir.absoluteFilePath("ai_voices");
        emit generationFinished(clipIndex, QString(), false);
        return;
    }

    QString outputDir = dir.absoluteFilePath("ai_voices/" + QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!QDir().mkpath(outputDir)) {
        qWarning() << "Failed to create temp directory for AI voice:" << outputDir;
        emit generationFinished(clipIndex, QString(), false);
        return;
    }

    QProcess* process = new QProcess(this);
    
    ProcessData pd;
    pd.process = process;
    pd.srtFilePath = srtFilePath;
    pd.language = language;
    pd.outputDir = outputDir;
    pd.stderrBuffer = "";
    m_processes[clipIndex] = pd;

    connect(process, &QProcess::readyReadStandardOutput, this, [this, clipIndex]() {
        onProcessReadyReadStandardOutput(clipIndex);
    });
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
        qWarning() << "Python executable was not found for AI voice generation.";
        m_processes.remove(clipIndex);
        process->deleteLater();
        emit generationFinished(clipIndex, QString(), false);
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString runnerPath = QDir(appDir).absoluteFilePath(QStringLiteral("edge_tts_runner.py"));
    if (!QFileInfo::exists(runnerPath)) {
        qWarning() << "edge_tts_runner was not found:" << runnerPath;
        m_processes.remove(clipIndex);
        process->deleteLater();
        emit generationFinished(clipIndex, QString(), false);
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
         << "--srt" << srtFilePath
         << "--lang" << language
         << "--outdir" << outputDir
         << "--ffmpeg" << QDir(appDir).absoluteFilePath(QStringLiteral("ffmpeg"));
    
    qDebug() << "Starting edge-tts generator:" << pythonExecutable << args;
    process->start(pythonExecutable, args);
}

void AIVoiceGenerator::cancelGeneration(int clipIndex)
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
    pd.process->deleteLater();
}

void AIVoiceGenerator::onProcessReadyReadStandardOutput(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) return;
    
    QProcess* process = m_processes[clipIndex].process;
    while (process->canReadLine()) {
        QString line = QString::fromUtf8(process->readLine()).trimmed();
        if (!line.isEmpty()) {
            emit segmentGenerated(clipIndex, line);
        }
    }
}

void AIVoiceGenerator::onProcessReadyReadStandardError(int clipIndex)
{
    if (!m_processes.contains(clipIndex)) return;
    
    QProcess* process = m_processes[clipIndex].process;
    QString errOutput = QString::fromUtf8(process->readAllStandardError());
    m_processes[clipIndex].stderrBuffer.append(errOutput);
    
    static QRegularExpression re("(\\d+)%");
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

void AIVoiceGenerator::onProcessFinished(int clipIndex, int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_processes.contains(clipIndex)) return;
    
    ProcessData pd = m_processes.take(clipIndex);
    bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
    
    if (!success) {
        qWarning() << "edge-tts process failed. Exit code:" << exitCode;
        qWarning() << pd.stderrBuffer;
    }
    
    QString metadataJsonPath = QDir(pd.outputDir).absoluteFilePath("metadata.json");

    success = success && QFile::exists(metadataJsonPath);
    if (!success) {
        qWarning() << "edge-tts did not produce expected metadata.json:" << metadataJsonPath;
        metadataJsonPath = QString();
    }

    emit generationFinished(clipIndex, metadataJsonPath, success);
    pd.process->deleteLater();
}

void AIVoiceGenerator::onProcessErrorOccurred(int clipIndex, QProcess::ProcessError error)
{
    if (!m_processes.contains(clipIndex)) return;

    ProcessData pd = m_processes.take(clipIndex);
    qWarning() << "edge-tts process error:" << error << pd.process->errorString();
    emit generationFinished(clipIndex, QString(), false);
    pd.process->deleteLater();
}
