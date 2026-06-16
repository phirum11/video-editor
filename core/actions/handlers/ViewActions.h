#pragma once
#include <QObject>

class ViewActions : public QObject {
    Q_OBJECT
public:
    explicit ViewActions(QObject* parent = nullptr);
    void registerActions();
};
