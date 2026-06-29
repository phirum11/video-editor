#pragma once

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QPolygonF>
#include <QVariantMap>
#include <QString>

class SelectionToolController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(bool feather READ feather WRITE setFeather NOTIFY featherChanged)
    Q_PROPERTY(qreal featherRadius READ featherRadius WRITE setFeatherRadius NOTIFY featherRadiusChanged)

public:
    explicit SelectionToolController(QObject *parent = nullptr);
    ~SelectionToolController() override;

    QString selectionMode() const { return m_selectionMode; }
    void setSelectionMode(const QString& mode);

    bool feather() const { return m_feather; }
    void setFeather(bool feather);

    qreal featherRadius() const { return m_featherRadius; }
    void setFeatherRadius(qreal radius);

    Q_INVOKABLE void startSelection(const QString& toolType, const QPointF& pos);
    Q_INVOKABLE void updateSelection(const QString& toolType, const QPointF& pos);
    Q_INVOKABLE void endSelection(const QString& toolType, const QPointF& pos);

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void selectNone();
    Q_INVOKABLE void selectInvert();

signals:
    void selectionModeChanged();
    void featherChanged();
    void featherRadiusChanged();
    void selectionUpdated(const QPolygonF& polygon);

private:
    QString m_selectionMode = "Replace"; // Replace, Add, Subtract, Intersect
    bool m_feather = false;
    qreal m_featherRadius = 10.0;

    QPointF m_startPos;
    QPolygonF m_currentPolygon;
};
