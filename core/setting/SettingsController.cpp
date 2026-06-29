#include "core/setting/SettingsController.h"

SettingsController& SettingsController::instance() {
    static SettingsController instance;
    return instance;
}

SettingsController::SettingsController(QObject* parent) : QObject(parent) {
    m_general = new GeneralSettings(this);
    m_appearance = new AppearanceSettings(this);
    m_timeline = new TimelineSettings(this);
    m_playback = new PlaybackSettings(this);
    m_audio = new AudioSettings(this);
    m_cache = new CacheSettings(this);
    m_keyboard = new KeyboardSettings(this);
}

GeneralSettings* SettingsController::general() const { return m_general; }
AppearanceSettings* SettingsController::appearance() const { return m_appearance; }
TimelineSettings* SettingsController::timeline() const { return m_timeline; }
PlaybackSettings* SettingsController::playback() const { return m_playback; }
AudioSettings* SettingsController::audio() const { return m_audio; }
CacheSettings* SettingsController::cache() const { return m_cache; }
KeyboardSettings* SettingsController::keyboard() const { return m_keyboard; }
