#pragma once
#include <QObject>

class HelpActions : public QObject {
    Q_OBJECT
public:
    explicit HelpActions(QObject* parent = nullptr);
    void registerActions();
};
