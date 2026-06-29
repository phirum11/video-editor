#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class TimelineSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool snapToTimeline READ snapToTimeline WRITE setSnapToTimeline NOTIFY snapToTimelineChanged)
    Q_PROPERTY(bool rippleEditByDefault READ rippleEditByDefault WRITE setRippleEditByDefault NOTIFY rippleEditByDefaultChanged)
    Q_PROPERTY(bool autoSelectClipsUnderPlayhead READ autoSelectClipsUnderPlayhead WRITE setAutoSelectClipsUnderPlayhead NOTIFY autoSelectClipsUnderPlayheadChanged)
    Q_PROPERTY(QString defaultTrackHeight READ defaultTrackHeight WRITE setDefaultTrackHeight NOTIFY defaultTrackHeightChanged)
    Q_PROPERTY(QString frameRate READ frameRate WRITE setFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(QString resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(QString pixelAspectRatio READ pixelAspectRatio WRITE setPixelAspectRatio NOTIFY pixelAspectRatioChanged)

public:
    explicit TimelineSettings(QObject* parent = nullptr);

    bool snapToTimeline() const;
    void setSnapToTimeline(bool snap);

    bool rippleEditByDefault() const;
    void setRippleEditByDefault(bool ripple);

    bool autoSelectClipsUnderPlayhead() const;
    void setAutoSelectClipsUnderPlayhead(bool autoSelect);

    QString defaultTrackHeight() const;
    void setDefaultTrackHeight(const QString& height);

    QString frameRate() const;
    void setFrameRate(const QString& rate);

    QString resolution() const;
    void setResolution(const QString& res);

    QString pixelAspectRatio() const;
    void setPixelAspectRatio(const QString& ratio);

signals:
    void snapToTimelineChanged();
    void rippleEditByDefaultChanged();
    void autoSelectClipsUnderPlayheadChanged();
    void defaultTrackHeightChanged();
    void frameRateChanged();
    void resolutionChanged();
    void pixelAspectRatioChanged();

private:
    bool m_snapToTimeline;
    bool m_rippleEditByDefault;
    bool m_autoSelectClipsUnderPlayhead;
    QString m_defaultTrackHeight;
    QString m_frameRate;
    QString m_resolution;
    QString m_pixelAspectRatio;
};
