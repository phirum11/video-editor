#include "ToolController.h"
#include <QPainter>
#include <QDebug>

ToolController::ToolController(QObject *parent)
    : QObject(parent)
    , m_options(new ToolOptionsModel(this))
{
}

ToolController::~ToolController() = default;

void ToolController::handlePointerDown(const QPointF& pos, EditorLayerModel* activeLayer)
{
    if (!activeLayer || !activeLayer->isVisible()) return;
    m_isPainting = true;
    m_lastPos = pos;
    applyPaint(pos, activeLayer);
    emit strokeStarted();
}

void ToolController::handlePointerMove(const QPointF& pos, EditorLayerModel* activeLayer)
{
    if (!m_isPainting || !activeLayer || !activeLayer->isVisible()) return;
    applyPaint(pos, activeLayer);
    m_lastPos = pos;
}

void ToolController::handlePointerUp(const QPointF& pos, EditorLayerModel* activeLayer)
{
    if (!m_isPainting) return;
    if (activeLayer && activeLayer->isVisible()) {
        applyPaint(pos, activeLayer);
    }
    m_isPainting = false;
    emit strokeFinished();
}

void ToolController::applyPaint(const QPointF& pos, EditorLayerModel* layer)
{
    if (!layer) return;

    QImage buffer = layer->buffer();
    if (buffer.isNull()) {
        buffer = QImage(1920, 1080, QImage::Format_ARGB32_Premultiplied);
        buffer.fill(Qt::transparent);
    }

    QPainter painter(&buffer);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_options->currentTool() == "Paintbrush") {
        QPen pen(m_options->foregroundColor(), m_options->brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawLine(m_lastPos, pos);
    } else if (m_options->currentTool() == "Eraser") {
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        QPen pen(Qt::transparent, m_options->brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        painter.drawLine(m_lastPos, pos);
    }

    painter.end();
    layer->setBuffer(buffer);
}
