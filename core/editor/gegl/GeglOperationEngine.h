#pragma once

#include <QObject>
#include <QImage>
#include <QThreadPool>
#include <QVariantMap>
#include <QString>
#include <functional>
#include <memory>

class GeglOperationEngine : public QObject
{
    Q_OBJECT

public:
    explicit GeglOperationEngine(QObject *parent = nullptr);
    ~GeglOperationEngine() override;

    Q_INVOKABLE void runAsyncOperation(const QString& operationName, const QImage& inputImage, const QVariantMap& params, const QString& operationType);

signals:
    void operationCompleted(const QString& operationName, const QImage& resultImage, bool success);

private:
    QThreadPool m_threadPool;
};
