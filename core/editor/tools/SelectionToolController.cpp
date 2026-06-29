#include "SelectionToolController.h"
#include <QDebug>

SelectionToolController::SelectionToolController(QObject *parent)
    : QObject(parent)
{
}

SelectionToolController::~SelectionToolController() = default;

void SelectionToolController::setSelectionMode(const QString& mode)
{
    if (m_selectionMode != mode) {
        m_selectionMode = mode;
        emit selectionModeChanged();
    }
}

void SelectionToolController::setFeather(bool feather)
{
    if (m_feather != feather) {
        m_feather = feather;
        emit featherChanged();
    }
}

void SelectionToolController::setFeatherRadius(qreal radius)
{
    if (!qFuzzyCompare(m_featherRadius, radius)) {
        m_featherRadius = radius;
        emit featherRadiusChanged();
    }
}

void SelectionToolController::startSelection(const QString& toolType, const QPointF& pos)
{
    qDebug() << "SelectionToolController startSelection:" << toolType << pos;
    m_startPos = pos;
    m_currentPolygon.clear();
    m_currentPolygon.append(pos);
}

void SelectionToolController::updateSelection(const QString& toolType, const QPointF& pos)
{
    if (toolType == "Rectangle" || toolType == "Ellipse") {
        m_currentPolygon.clear();
        m_currentPolygon.append(m_startPos);
        m_currentPolygon.append(QPointF(pos.x(), m_startPos.y()));
        m_currentPolygon.append(pos);
        m_currentPolygon.append(QPointF(m_startPos.x(), pos.y()));
    } else {
        m_currentPolygon.append(pos);
    }
    emit selectionUpdated(m_currentPolygon);
}

void SelectionToolController::endSelection(const QString& toolType, const QPointF& pos)
{
    qDebug() << "SelectionToolController endSelection:" << toolType << pos;
    updateSelection(toolType, pos);
}

void SelectionToolController::selectAll()
{
    qDebug() << "SelectionToolController selectAll";
    m_currentPolygon.clear();
    // Representing full canvas bounding rect
    m_currentPolygon.append(QPointF(0, 0));
    m_currentPolygon.append(QPointF(1920, 0));
    m_currentPolygon.append(QPointF(1920, 1080));
    m_currentPolygon.append(QPointF(0, 1080));
    emit selectionUpdated(m_currentPolygon);
}

void SelectionToolController::selectNone()
{
    qDebug() << "SelectionToolController selectNone";
    m_currentPolygon.clear();
    emit selectionUpdated(m_currentPolygon);
}

void SelectionToolController::selectInvert()
{
    qDebug() << "SelectionToolController selectInvert";
    // Placeholder for selection mask inversion
}
