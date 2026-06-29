#pragma once

#include <QObject>
#include "core/setting/general/GeneralSettings.h"
#include "core/setting/appearance/AppearanceSettings.h"
#include "core/setting/timeline/TimelineSettings.h"
#include "core/setting/playback/PlaybackSettings.h"
#include "core/setting/audio/AudioSettings.h"
#include "core/setting/cache/CacheSettings.h"
#include "core/setting/keyboard/KeyboardSettings.h"

class SettingsController : public QObject {
    Q_OBJECT
    Q_PROPERTY(GeneralSettings* general READ general CONSTANT)
    Q_PROPERTY(AppearanceSettings* appearance READ appearance CONSTANT)
    Q_PROPERTY(TimelineSettings* timeline READ timeline CONSTANT)
    Q_PROPERTY(PlaybackSettings* playback READ playback CONSTANT)
    Q_PROPERTY(AudioSettings* audio READ audio CONSTANT)
    Q_PROPERTY(CacheSettings* cache READ cache CONSTANT)
    Q_PROPERTY(KeyboardSettings* keyboard READ keyboard CONSTANT)

public:
    static SettingsController& instance();

    GeneralSettings* general() const;
    AppearanceSettings* appearance() const;
    TimelineSettings* timeline() const;
    PlaybackSettings* playback() const;
    AudioSettings* audio() const;
    CacheSettings* cache() const;
    KeyboardSettings* keyboard() const;

private:
    explicit SettingsController(QObject* parent = nullptr);
    ~SettingsController() override = default;
    SettingsController(const SettingsController&) = delete;
    SettingsController& operator=(const SettingsController&) = delete;

    GeneralSettings* m_general;
    AppearanceSettings* m_appearance;
    TimelineSettings* m_timeline;
    PlaybackSettings* m_playback;
    AudioSettings* m_audio;
    CacheSettings* m_cache;
    KeyboardSettings* m_keyboard;
};
