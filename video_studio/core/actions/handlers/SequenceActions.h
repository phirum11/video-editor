#pragma once
#include <QObject>

class SequenceActions : public QObject {
    Q_OBJECT
public:
    explicit SequenceActions(QObject* parent = nullptr);
    void registerActions();
};
