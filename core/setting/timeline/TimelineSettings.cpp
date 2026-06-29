#include "core/setting/timeline/TimelineSettings.h"

TimelineSettings::TimelineSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Timeline");
    m_snapToTimeline = settings.value("snapToTimeline", true).toBool();
    m_rippleEditByDefault = settings.value("rippleEditByDefault", false).toBool();
    m_autoSelectClipsUnderPlayhead = settings.value("autoSelectClipsUnderPlayhead", true).toBool();
    m_defaultTrackHeight = settings.value("defaultTrackHeight", "Medium").toString();
    m_frameRate = settings.value("frameRate", "24 fps").toString();
    m_resolution = settings.value("resolution", "1920 × 1080").toString();
    m_pixelAspectRatio = settings.value("pixelAspectRatio", "Square Pixels (1.0)").toString();
    settings.endGroup();
}

bool TimelineSettings::snapToTimeline() const { return m_snapToTimeline; }
void TimelineSettings::setSnapToTimeline(bool snap) {
    if (m_snapToTimeline != snap) {
        m_snapToTimeline = snap;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("snapToTimeline", snap);
        settings.endGroup();
        emit snapToTimelineChanged();
    }
}

bool TimelineSettings::rippleEditByDefault() const { return m_rippleEditByDefault; }
void TimelineSettings::setRippleEditByDefault(bool ripple) {
    if (m_rippleEditByDefault != ripple) {
        m_rippleEditByDefault = ripple;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("rippleEditByDefault", ripple);
        settings.endGroup();
        emit rippleEditByDefaultChanged();
    }
}

bool TimelineSettings::autoSelectClipsUnderPlayhead() const { return m_autoSelectClipsUnderPlayhead; }
void TimelineSettings::setAutoSelectClipsUnderPlayhead(bool autoSelect) {
    if (m_autoSelectClipsUnderPlayhead != autoSelect) {
        m_autoSelectClipsUnderPlayhead = autoSelect;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("autoSelectClipsUnderPlayhead", autoSelect);
        settings.endGroup();
        emit autoSelectClipsUnderPlayheadChanged();
    }
}

QString TimelineSettings::defaultTrackHeight() const { return m_defaultTrackHeight; }
void TimelineSettings::setDefaultTrackHeight(const QString& height) {
    if (m_defaultTrackHeight != height) {
        m_defaultTrackHeight = height;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("defaultTrackHeight", height);
        settings.endGroup();
        emit defaultTrackHeightChanged();
    }
}

QString TimelineSettings::frameRate() const { return m_frameRate; }
void TimelineSettings::setFrameRate(const QString& rate) {
    if (m_frameRate != rate) {
        m_frameRate = rate;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("frameRate", rate);
        settings.endGroup();
        emit frameRateChanged();
    }
}

QString TimelineSettings::resolution() const { return m_resolution; }
void TimelineSettings::setResolution(const QString& res) {
    if (m_resolution != res) {
        m_resolution = res;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("resolution", res);
        settings.endGroup();
        emit resolutionChanged();
    }
}

QString TimelineSettings::pixelAspectRatio() const { return m_pixelAspectRatio; }
void TimelineSettings::setPixelAspectRatio(const QString& ratio) {
    if (m_pixelAspectRatio != ratio) {
        m_pixelAspectRatio = ratio;
        QSettings settings;
        settings.beginGroup("Timeline");
        settings.setValue("pixelAspectRatio", ratio);
        settings.endGroup();
        emit pixelAspectRatioChanged();
    }
}
