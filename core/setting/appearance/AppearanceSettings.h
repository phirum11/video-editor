#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class AppearanceSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(int colorTheme READ colorTheme WRITE setColorTheme NOTIFY colorThemeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(int uiScale READ uiScale WRITE setUiScale NOTIFY uiScaleChanged)
    Q_PROPERTY(int interfaceFont READ interfaceFont WRITE setInterfaceFont NOTIFY interfaceFontChanged)
    Q_PROPERTY(int monospaceFont READ monospaceFont WRITE setMonospaceFont NOTIFY monospaceFontChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

public:
    explicit AppearanceSettings(QObject* parent = nullptr);

    int colorTheme() const;
    void setColorTheme(int theme);

    QString accentColor() const;
    void setAccentColor(const QString& color);

    int uiScale() const;
    void setUiScale(int scale);

    int interfaceFont() const;
    void setInterfaceFont(int font);

    int monospaceFont() const;
    void setMonospaceFont(int font);

    int fontSize() const;
    void setFontSize(int size);

signals:
    void colorThemeChanged();
    void accentColorChanged();
    void uiScaleChanged();
    void interfaceFontChanged();
    void monospaceFontChanged();
    void fontSizeChanged();

private:
    int m_colorTheme;
    QString m_accentColor;
    int m_uiScale;
    int m_interfaceFont;
    int m_monospaceFont;
    int m_fontSize;
};
