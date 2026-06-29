#include "GeglOperationManager.h"
#include <QDebug>

GeglOperationManager::GeglOperationManager(QObject *parent)
    : QObject(parent)
{
    registerDefaultOperations();
}

GeglOperationManager::~GeglOperationManager() = default;

QStringList GeglOperationManager::availableOperations() const
{
    QStringList list;
    for (const auto& op : m_operations) {
        list << op.name;
    }
    return list;
}

QVariantMap GeglOperationManager::getOperationDetails(const QString& name) const
{
    for (const auto& op : m_operations) {
        if (op.name == name) {
            QVariantMap map;
            map["name"] = op.name;
            map["category"] = op.category;
            map["description"] = op.description;
            map["defaultParameters"] = op.defaultParameters;
            return map;
        }
    }
    return QVariantMap();
}

GeglGraphNode* GeglOperationManager::createNode(const QString& operationName, QObject* parent)
{
    for (const auto& op : m_operations) {
        if (op.name == operationName) {
            auto* node = new GeglGraphNode(parent);
            node->setOperation(operationName);
            for (auto it = op.defaultParameters.begin(); it != op.defaultParameters.end(); ++it) {
                node->setProperty(it.key(), it.value());
            }
            return node;
        }
    }
    qWarning() << "Requested unknown GEGL operation:" << operationName;
    return nullptr;
}

void GeglOperationManager::registerDefaultOperations()
{
    m_operations.push_back({"gegl:gaussian-blur", "Blur", "Perform Gaussian blur on the image buffer", {{"std-dev-x", 1.5}, {"std-dev-y", 1.5}}});
    m_operations.push_back({"gegl:color-balance", "Color", "Adjust color balance in highlights, midtones, and shadows", {{"cyan-red", 0.0}, {"magenta-green", 0.0}, {"yellow-blue", 0.0}}});
    m_operations.push_back({"gegl:invert", "Color", "Invert image colors", {}});
    m_operations.push_back({"gegl:dropshadow", "Light & Shadow", "Add a drop shadow to the layer buffer", {{"opacity", 0.5}, {"x", 10.0}, {"y", 10.0}, {"blur-radius", 5.0}}});
    emit catalogChanged();
}
