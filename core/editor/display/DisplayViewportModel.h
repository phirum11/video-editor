#pragma once

#include <QObject>
#include <QPointF>

class DisplayViewportModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QPointF panOffset READ panOffset WRITE setPanOffset NOTIFY panOffsetChanged)
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
    Q_PROPERTY(bool showGuides READ showGuides WRITE setShowGuides NOTIFY showGuidesChanged)

public:
    explicit DisplayViewportModel(QObject *parent = nullptr);
    ~DisplayViewportModel() override;

    qreal zoomLevel() const { return m_zoomLevel; }
    void setZoomLevel(qreal zoom);

    QPointF panOffset() const { return m_panOffset; }
    void setPanOffset(const QPointF& offset);

    bool showGrid() const { return m_showGrid; }
    void setShowGrid(bool show);

    bool showGuides() const { return m_showGuides; }
    void setShowGuides(bool show);

    Q_INVOKABLE void resetViewport();

signals:
    void zoomLevelChanged();
    void panOffsetChanged();
    void showGridChanged();
    void showGuidesChanged();

private:
    qreal m_zoomLevel = 1.0;
    QPointF m_panOffset = QPointF(0, 0);
    bool m_showGrid = false;
    bool m_showGuides = true;
};
