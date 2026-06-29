#include "GeglOperationEngine.h"
#include <QRunnable>
#include <QDebug>
#include "ColorAdjustmentOperation.h"
#include "TransformDeformOperation.h"
#include "GenerativeFillOperation.h"

class OperationTask : public QRunnable
{
public:
    OperationTask(QString name, QImage input, QVariantMap params, QString type, std::function<void(QString, QImage, bool)> callback)
        : m_name(std::move(name)), m_input(std::move(input)), m_params(std::move(params)), m_type(std::move(type)), m_callback(std::move(callback)) {}

    void run() override {
        QImage result = m_input;
        bool success = false;

        if (m_type == "ColorAdjustment") {
            success = ColorAdjustmentOperation::apply(m_name, result, m_params);
        } else if (m_type == "TransformDeform") {
            success = TransformDeformOperation::apply(m_name, result, m_params);
        } else if (m_type == "GenerativeFill") {
            success = GenerativeFillOperation::apply(m_name, result, m_params);
        }

        if (m_callback) {
            m_callback(m_name, result, success);
        }
    }

private:
    QString m_name;
    QImage m_input;
    QVariantMap m_params;
    QString m_type;
    std::function<void(QString, QImage, bool)> m_callback;
};

GeglOperationEngine::GeglOperationEngine(QObject *parent)
    : QObject(parent)
{
    m_threadPool.setMaxThreadCount(QThread::idealThreadCount());
}

GeglOperationEngine::~GeglOperationEngine() = default;

void GeglOperationEngine::runAsyncOperation(const QString& operationName, const QImage& inputImage, const QVariantMap& params, const QString& operationType)
{
    qDebug() << "GeglOperationEngine running async operation:" << operationName << "Type:" << operationType;
    auto* task = new OperationTask(operationName, inputImage, params, operationType, [this](QString name, QImage res, bool success) {
        QMetaObject::invokeMethod(this, [this, name, res, success]() {
            emit operationCompleted(name, res, success);
        }, Qt::QueuedConnection);
    });
    task->setAutoDelete(true);
    m_threadPool.start(task);
}
