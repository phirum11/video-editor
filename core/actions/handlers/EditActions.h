#pragma once
#include <QObject>

class EditActions : public QObject {
    Q_OBJECT
public:
    explicit EditActions(QObject* parent = nullptr);
    void registerActions();
};
