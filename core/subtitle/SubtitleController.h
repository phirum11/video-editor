#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include <QFont>
#include <QColor>
#include <QFont>
#include <QColor>
#include "ui/timeline_view/controllers/TimelineController.h"

class SubtitleController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(double verticalPosition READ verticalPosition WRITE setVerticalPosition NOTIFY verticalPositionChanged)

public:
    explicit SubtitleController(QObject* parent = nullptr);

    Q_INVOKABLE int addSrtToTimeline(const QString& filePath, TimelineController* timeline);
    Q_INVOKABLE int addSrtToTimelineAt(const QString& filePath,
                                       TimelineController* timeline,
                                       double startOffsetSeconds,
                                       int trackIndex);
    Q_INVOKABLE QString getSubtitleAtTime(TimelineController* timeline, double timeSeconds) const;

    QFont font() const;
    void setFont(const QFont& font);

    QColor color() const;
    void setColor(const QColor& color);

    double verticalPosition() const;
    void setVerticalPosition(double pos);

signals:
    void fontChanged();
    void colorChanged();
    void verticalPositionChanged();

private:
    QFont m_font;
    QColor m_color;
    double m_verticalPosition = -1.0;
};

