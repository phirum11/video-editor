#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QtQml/qqmlregistration.h>

class GimpProcessor : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)
    Q_PROPERTY(QString lastExportPath READ lastExportPath NOTIFY lastExportPathChanged)

public:
    explicit GimpProcessor(QObject *parent = nullptr);
    ~GimpProcessor() override;

    bool isProcessing() const { return m_isProcessing; }
    QString lastExportPath() const { return m_lastExportPath; }

    Q_INVOKABLE bool applyFilter(const QString& filterName, const QString& inputPath, const QString& outputPath);
    Q_INVOKABLE bool exportToGimp(const QString& imagePath);
    Q_INVOKABLE bool applyGeglOperation(const QString& operation, const QVariantMap& parameters);
    Q_INVOKABLE bool removeBackground(const QString& imagePath);

signals:
    void isProcessingChanged();
    void lastExportPathChanged();
    void processingFinished(bool success, const QString& resultPath, const QString& operation);
    void gimpEditorUpdateReceived(const QString& updatedPath);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onFileChanged(const QString& path);

private:
    void setIsProcessing(bool processing);
    void setLastExportPath(const QString& path);
    QString findGimpExecutable() const;
    QString cleanPath(const QString& path) const;

    bool m_isProcessing = false;
    QString m_lastExportPath;
    QString m_currentOperation;
    QProcess* m_process = nullptr;
    QFileSystemWatcher* m_fileWatcher = nullptr;
};
