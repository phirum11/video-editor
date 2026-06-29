#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVector>
#include "PdbRegistry.h"

struct ProcedureDefinition {
    QString category;
    QString name;
    QString blurb;
    QString help;
    QStringList argumentTypes;
};

class PdbProcedureManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableCategories READ availableCategories CONSTANT)

public:
    explicit PdbProcedureManager(QObject *parent = nullptr);
    ~PdbProcedureManager() override;

    QStringList availableCategories() const;

    Q_INVOKABLE QStringList getProceduresInCategory(const QString& category) const;
    Q_INVOKABLE QVariantMap getProcedureDetails(const QString& procName) const;
    Q_INVOKABLE bool runProcedure(const QString& procName, const QVariantMap& args);

signals:
    void procedureInvoked(const QString& procName, bool success);

private:
    void registerDefaultProcedures();

    QVector<ProcedureDefinition> m_procedures;
    PdbRegistry* m_registry;
};
