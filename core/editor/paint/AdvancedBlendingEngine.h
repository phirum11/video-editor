#pragma once

#include <QImage>
#include <QPointF>
#include <QString>
#include <QVariantMap>

class AdvancedBlendingEngine
{
public:
    static bool applyBlendAction(const QString& actionType, QImage& buffer, const QPointF& pos, const QVariantMap& options);

private:
    static void applyConvolve(QImage& buffer, const QPointF& pos, qreal size, bool sharpen);
    static void applyDodgeBurn(QImage& buffer, const QPointF& pos, qreal size, bool dodge, qreal exposure);
    static void applyHeal(QImage& buffer, const QPointF& src, const QPointF& dest, qreal size);
    static void applySmudge(QImage& buffer, const QPointF& start, const QPointF& end, qreal size, qreal rate);
    static void applySourceCore(QImage& buffer, const QPointF& pos, qreal size);
};
