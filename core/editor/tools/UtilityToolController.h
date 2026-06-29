#pragma once

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QString>
#include <QVariantMap>

class UtilityToolController : public QObject
{
    Q_OBJECT

public:
    explicit UtilityToolController(QObject *parent = nullptr);
    ~UtilityToolController() override;

    Q_INVOKABLE QColor pickColorAt(const QPointF& pos, const QImage& buffer) const;
    Q_INVOKABLE qreal measureDistance(const QPointF& p1, const QPointF& p2) const;
    Q_INVOKABLE qreal measureAngle(const QPointF& p1, const QPointF& p2) const;
    Q_INVOKABLE QVariantMap addGuide(const QString& orientation, qreal position);
    Q_INVOKABLE QVariantMap addSamplePoint(const QPointF& pos);

signals:
    void guideAdded(const QString& orientation, qreal position);
    void samplePointAdded(const QPointF& pos);
};
