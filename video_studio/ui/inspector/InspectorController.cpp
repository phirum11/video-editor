#include "InspectorController.h"
#include <QDebug>

InspectorController::InspectorController(QObject *parent)
    : QObject(parent)
{
}

float InspectorController::currentBrightness() const
{
    return m_currentBrightness;
}

void InspectorController::setCurrentBrightness(float brightness)
{
    if (qFuzzyCompare(m_currentBrightness, brightness))
        return;

    m_currentBrightness = brightness;
    emit currentBrightnessChanged();
    
    // TODO: Call Core App logic here to apply brightness to the video engine
    qDebug() << "Inspector UI: Brightness changed to" << m_currentBrightness;
}
