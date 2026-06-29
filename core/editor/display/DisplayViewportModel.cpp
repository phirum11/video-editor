#include "DisplayViewportModel.h"

DisplayViewportModel::DisplayViewportModel(QObject *parent)
    : QObject(parent)
{
}

DisplayViewportModel::~DisplayViewportModel() = default;

void DisplayViewportModel::setZoomLevel(qreal zoom)
{
    if (!qFuzzyCompare(m_zoomLevel, zoom)) {
        m_zoomLevel = qBound(0.1, zoom, 32.0);
        emit zoomLevelChanged();
    }
}

void DisplayViewportModel::setPanOffset(const QPointF& offset)
{
    if (m_panOffset != offset) {
        m_panOffset = offset;
        emit panOffsetChanged();
    }
}

void DisplayViewportModel::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        emit showGridChanged();
    }
}

void DisplayViewportModel::setShowGuides(bool show)
{
    if (m_showGuides != show) {
        m_showGuides = show;
        emit showGuidesChanged();
    }
}

void DisplayViewportModel::resetViewport()
{
    setZoomLevel(1.0);
    setPanOffset(QPointF(0, 0));
}
