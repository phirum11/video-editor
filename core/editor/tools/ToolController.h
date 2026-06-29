#pragma once

#include <QObject>
#include <QPointF>
#include <memory>
#include "ToolOptionsModel.h"
#include "../models/EditorLayerModel.h"

class ToolController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ToolOptionsModel* options READ options CONSTANT)

public:
    explicit ToolController(QObject *parent = nullptr);
    ~ToolController() override;

    ToolOptionsModel* options() const { return m_options; }

    Q_INVOKABLE void handlePointerDown(const QPointF& pos, EditorLayerModel* activeLayer);
    Q_INVOKABLE void handlePointerMove(const QPointF& pos, EditorLayerModel* activeLayer);
    Q_INVOKABLE void handlePointerUp(const QPointF& pos, EditorLayerModel* activeLayer);

signals:
    void strokeStarted();
    void strokeFinished();

private:
    void applyPaint(const QPointF& pos, EditorLayerModel* layer);

    ToolOptionsModel* m_options;
    bool m_isPainting = false;
    QPointF m_lastPos;
};
