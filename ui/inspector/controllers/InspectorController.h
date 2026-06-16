#ifndef INSPECTORCONTROLLER_H
#define INSPECTORCONTROLLER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>

class InspectorController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(float currentBrightness READ currentBrightness WRITE setCurrentBrightness NOTIFY currentBrightnessChanged)

public:
    explicit InspectorController(QObject *parent = nullptr);

    float currentBrightness() const;
    void setCurrentBrightness(float brightness);

signals:
    void currentBrightnessChanged();

private:
    float m_currentBrightness = 1.0f;
};

#endif // INSPECTORCONTROLLER_H
