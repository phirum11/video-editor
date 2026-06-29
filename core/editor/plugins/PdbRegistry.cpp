#include "PdbRegistry.h"
#include <QDebug>

PdbRegistry::PdbRegistry(QObject *parent)
    : QObject(parent)
    , m_gimpProcessor(new GimpProcessor(this))
{
    m_procedures.push_back({"gimp-image-invert", "Invert image colors", "Invert the colors of the specified image buffer."});
    m_procedures.push_back({"gimp-image-flip", "Flip image", "Flip the image horizontally or vertically."});
}

PdbRegistry::~PdbRegistry() = default;

bool PdbRegistry::executeProcedure(const QString& procedureName, const QVariantMap& arguments)
{
    qDebug() << "PdbRegistry executing procedure:" << procedureName << arguments;
    if (m_gimpProcessor) {
        // Delegate complex external processing to GimpProcessor
        m_gimpProcessor->applyGeglOperation(procedureName, arguments);
        emit procedureExecuted(procedureName, true);
        return true;
    }
    emit procedureExecuted(procedureName, false);
    return false;
}

QVariantMap PdbRegistry::getProcedureInfo(const QString& procedureName) const
{
    for (const auto& proc : m_procedures) {
        if (proc.name == procedureName) {
            QVariantMap map;
            map["name"] = proc.name;
            map["blurb"] = proc.blurb;
            map["help"] = proc.help;
            return map;
        }
    }
    return QVariantMap();
}
