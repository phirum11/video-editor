#pragma once
#include <QObject>

class ClipActions : public QObject {
    Q_OBJECT
public:
    explicit ClipActions(QObject* parent = nullptr);
    void registerActions();
};
