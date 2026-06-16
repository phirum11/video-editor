#pragma once
#include <QObject>

class ToolActions : public QObject {
    Q_OBJECT
public:
    explicit ToolActions(QObject* parent = nullptr);
    void registerActions();
};
