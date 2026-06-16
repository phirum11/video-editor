#pragma once
#include <QObject>
#include <QVariantList>

class MenuManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList menuStructure READ menuStructure NOTIFY menuStructureChanged)

public:
    static MenuManager& instance();

    QVariantList menuStructure() const;
    void buildDefaultMenu();

signals:
    void menuStructureChanged();

private:
    MenuManager(QObject* parent = nullptr);
    ~MenuManager() = default;
    MenuManager(const MenuManager&) = delete;
    MenuManager& operator=(const MenuManager&) = delete;

    QVariantList m_menuStructure;
    
    QVariantMap createMenu(const QString& title, const QVariantList& items);
    QVariantMap createMenuItem(const QString& actionId);
    QVariantMap createSubMenu(const QString& title, const QVariantList& items);
    QVariantMap createSeparator();
};
