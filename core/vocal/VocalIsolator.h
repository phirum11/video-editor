#pragma once
#include <QObject>
#include <QProcess>
#include <QString>
#include <QMap>
#include <QFutureWatcher>
#include <QFuture>

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
    void onProcessFinished(int clipIndex);
private:
    struct ProcessData {
        QFutureWatcher<bool>* watcher;
        QString sourceFilePath;
        int isolationType;
        QString outputDir;
        QString tempFilePath;
        QString tempWorkDir;
        QString cacheKey;
    };
    QMap<int, ProcessData> m_processes;
};
