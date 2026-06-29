#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <vector>
#include <memory>
#include "GeglGraphNode.h"

struct GeglOperationInfo {
    QString name;
    QString category;
    QString description;
    QVariantMap defaultParameters;
};

class GeglOperationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableOperations READ availableOperations NOTIFY catalogChanged)

public:
    explicit GeglOperationManager(QObject *parent = nullptr);
    ~GeglOperationManager() override;

    QStringList availableOperations() const;

    Q_INVOKABLE QVariantMap getOperationDetails(const QString& name) const;
    Q_INVOKABLE GeglGraphNode* createNode(const QString& operationName, QObject* parent = nullptr);

signals:
    void catalogChanged();

private:
    void registerDefaultOperations();

    std::vector<GeglOperationInfo> m_operations;
};
