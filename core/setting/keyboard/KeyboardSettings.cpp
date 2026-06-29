#include "core/setting/keyboard/KeyboardSettings.h"

KeyboardSettings::KeyboardSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Keyboard");
    m_keyboardPreset = settings.value("keyboardPreset", "Video Studio Pro (Default)").toString();
    settings.endGroup();
}

QString KeyboardSettings::keyboardPreset() const { return m_keyboardPreset; }
void KeyboardSettings::setKeyboardPreset(const QString& preset) {
    if (m_keyboardPreset != preset) {
        m_keyboardPreset = preset;
        QSettings settings;
        settings.beginGroup("Keyboard");
        settings.setValue("keyboardPreset", preset);
        settings.endGroup();
        emit keyboardPresetChanged();
    }
}
