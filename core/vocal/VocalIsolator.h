#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include <QMap>
class VocalIsolator : public QObject {
    Q_OBJECT
public:
    explicit VocalIsolator(QObject* parent = nullptr);
    ~VocalIsolator() override;
    void requestIsolation(int clipIndex, const QString& sourceFilePath, int isolationType);
    void cancelIsolation(int clipIndex);
signals:
    void progressChanged(int clipIndex, int progress);
    void isolationFinished(int clipIndex, int isolationType, const QString& resultFilePath, bool success);

private slots:
    void onProcessReadyReadStandardError(int clipIndex);
    void onProcessFinished(int clipIndex, int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(int clipIndex, QProcess::ProcessError error);
private:
    struct ProcessData {
        QProcess* process;
        QString sourceFilePath;
        int isolationType;
        QString outputDir;
        QString tempFilePath;
        QString tempWorkDir;
        QString cacheKey;
        QString stderrBuffer;
    };
    QMap<int, ProcessData> m_processes;
};
