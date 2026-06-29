#pragma once

#include <QObject>
#include <QString>
#include <QSettings>

class KeyboardSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString keyboardPreset READ keyboardPreset WRITE setKeyboardPreset NOTIFY keyboardPresetChanged)

public:
    explicit KeyboardSettings(QObject* parent = nullptr);

    QString keyboardPreset() const;
    void setKeyboardPreset(const QString& preset);

signals:
    void keyboardPresetChanged();

private:
    QString m_keyboardPreset;
};
