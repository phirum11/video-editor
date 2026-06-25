#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QProcess>

class AIVoiceGenerator : public QObject
{
    Q_OBJECT
public:
    explicit AIVoiceGenerator(QObject* parent = nullptr);
    ~AIVoiceGenerator();

    void requestGeneration(int clipIndex, const QString& srtFilePath, const QString& language);
    void cancelGeneration(int clipIndex);

signals:
    void progressChanged(int clipIndex, int progress);
    void segmentGenerated(int clipIndex, const QString& segmentJson);
    void generationFinished(int clipIndex, const QString& metadataJsonPath, bool success);

private:
    void onProcessReadyReadStandardOutput(int clipIndex);
    void onProcessReadyReadStandardError(int clipIndex);
    void onProcessFinished(int clipIndex, int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(int clipIndex, QProcess::ProcessError error);

    struct ProcessData {
        QProcess* process;
        QString srtFilePath;
        QString language;
        QString outputDir;
        QString stderrBuffer;
    };

    QMap<int, ProcessData> m_processes;
};
