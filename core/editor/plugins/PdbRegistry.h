#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <vector>
#include "../../effects/processors/GimpProcessor.h"

struct PdbProcedure {
    QString name;
    QString blurb;
    QString help;
};

class PdbRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GimpProcessor* gimpProcessor READ gimpProcessor CONSTANT)

public:
    explicit PdbRegistry(QObject *parent = nullptr);
    ~PdbRegistry() override;

    GimpProcessor* gimpProcessor() const { return m_gimpProcessor; }

    Q_INVOKABLE bool executeProcedure(const QString& procedureName, const QVariantMap& arguments);
    Q_INVOKABLE QVariantMap getProcedureInfo(const QString& procedureName) const;

signals:
    void procedureExecuted(const QString& procedureName, bool success);

private:
    GimpProcessor* m_gimpProcessor;
    std::vector<PdbProcedure> m_procedures;
};
