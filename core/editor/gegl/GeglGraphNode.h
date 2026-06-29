#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QImage>
#include <vector>
#include <memory>

class GeglGraphNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString operation READ operation WRITE setOperation NOTIFY operationChanged)
    Q_PROPERTY(QVariantMap properties READ properties NOTIFY propertiesModified)

public:
    explicit GeglGraphNode(QObject *parent = nullptr);
    ~GeglGraphNode() override;

    QString operation() const { return m_operation; }
    void setOperation(const QString& op);

    QVariantMap properties() const { return m_properties; }

    Q_INVOKABLE void setProperty(const QString& name, const QVariant& value);
    Q_INVOKABLE QVariant propertyValue(const QString& name) const;

    Q_INVOKABLE void connectTo(GeglGraphNode* targetNode);
    Q_INVOKABLE QImage process(const QImage& inputImage);

signals:
    void operationChanged();
    void propertiesModified();

private:
    QString m_operation;
    QVariantMap m_properties;
    std::vector<GeglGraphNode*> m_connectedTargets;
};
