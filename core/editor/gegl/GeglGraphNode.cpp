#include "GeglGraphNode.h"
#include <QDebug>

GeglGraphNode::GeglGraphNode(QObject *parent)
    : QObject(parent)
{
}

GeglGraphNode::~GeglGraphNode() = default;

void GeglGraphNode::setOperation(const QString& op)
{
    if (m_operation != op) {
        m_operation = op;
        emit operationChanged();
    }
}

void GeglGraphNode::setProperty(const QString& name, const QVariant& value)
{
    m_properties[name] = value;
    emit propertiesModified();
}

QVariant GeglGraphNode::propertyValue(const QString& name) const
{
    return m_properties.value(name);
}

void GeglGraphNode::connectTo(GeglGraphNode* targetNode)
{
    if (targetNode) {
        m_connectedTargets.push_back(targetNode);
    }
}

QImage GeglGraphNode::process(const QImage& inputImage)
{
    qDebug() << "GeglGraphNode processing operation:" << m_operation;
    // Perform simulated non-destructive image operation
    QImage result = inputImage;
    for (auto* target : m_connectedTargets) {
        if (target) {
            result = target->process(result);
        }
    }
    return result;
}
