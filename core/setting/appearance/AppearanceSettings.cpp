#include "core/setting/appearance/AppearanceSettings.h"

AppearanceSettings::AppearanceSettings(QObject* parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup("Appearance");
    m_colorTheme = settings.value("colorTheme", 0).toInt();
    m_accentColor = settings.value("accentColor", "#58a8d8").toString();
    m_uiScale = settings.value("uiScale", 1).toInt();
    m_interfaceFont = settings.value("interfaceFont", 0).toInt();
    m_monospaceFont = settings.value("monospaceFont", 0).toInt();
    m_fontSize = settings.value("fontSize", 2).toInt();
    settings.endGroup();
}

int AppearanceSettings::colorTheme() const { return m_colorTheme; }
void AppearanceSettings::setColorTheme(int theme) {
    if (m_colorTheme != theme) {
        m_colorTheme = theme;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("colorTheme", theme);
        settings.endGroup();
        emit colorThemeChanged();
    }
}

QString AppearanceSettings::accentColor() const { return m_accentColor; }
void AppearanceSettings::setAccentColor(const QString& color) {
    if (m_accentColor != color) {
        m_accentColor = color;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("accentColor", color);
        settings.endGroup();
        emit accentColorChanged();
    }
}

int AppearanceSettings::uiScale() const { return m_uiScale; }
void AppearanceSettings::setUiScale(int scale) {
    if (m_uiScale != scale) {
        m_uiScale = scale;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("uiScale", scale);
        settings.endGroup();
        emit uiScaleChanged();
    }
}

int AppearanceSettings::interfaceFont() const { return m_interfaceFont; }
void AppearanceSettings::setInterfaceFont(int font) {
    if (m_interfaceFont != font) {
        m_interfaceFont = font;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("interfaceFont", font);
        settings.endGroup();
        emit interfaceFontChanged();
    }
}

int AppearanceSettings::monospaceFont() const { return m_monospaceFont; }
void AppearanceSettings::setMonospaceFont(int font) {
    if (m_monospaceFont != font) {
        m_monospaceFont = font;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("monospaceFont", font);
        settings.endGroup();
        emit monospaceFontChanged();
    }
}

int AppearanceSettings::fontSize() const { return m_fontSize; }
void AppearanceSettings::setFontSize(int size) {
    if (m_fontSize != size) {
        m_fontSize = size;
        QSettings settings;
        settings.beginGroup("Appearance");
        settings.setValue("fontSize", size);
        settings.endGroup();
        emit fontSizeChanged();
    }
}
