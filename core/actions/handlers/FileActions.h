#pragma once
#include <QObject>

class FileActions : public QObject {
    Q_OBJECT
public:
    explicit FileActions(QObject* parent = nullptr);
    void registerActions();
};
