#include "TransformEffect.h"

TransformEffect::TransformEffect(QObject *parent) : QObject(parent)
{
}

double TransformEffect::scale() const { return m_data.scale; }
void TransformEffect::setScale(double val) { 
    if (m_data.scale != val) { 
        m_data.scale = val; 
        emit scaleChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::posX() const { return m_data.posX; }
void TransformEffect::setPosX(double val) { 
    if (m_data.posX != val) { 
        m_data.posX = val; 
        emit posXChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::posY() const { return m_data.posY; }
void TransformEffect::setPosY(double val) { 
    if (m_data.posY != val) { 
        m_data.posY = val; 
        emit posYChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::rotation() const { return m_data.rotation; }
void TransformEffect::setRotation(double val) { 
    if (m_data.rotation != val) { 
        m_data.rotation = val; 
        emit rotationChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::anchorX() const { return m_data.anchorX; }
void TransformEffect::setAnchorX(double val) { 
    if (m_data.anchorX != val) { 
        m_data.anchorX = val; 
        emit anchorXChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::anchorY() const { return m_data.anchorY; }
void TransformEffect::setAnchorY(double val) { 
    if (m_data.anchorY != val) { 
        m_data.anchorY = val; 
        emit anchorYChanged(); 
        emit modified(); 
    } 
}

double TransformEffect::opacity() const { return m_data.opacity; }
void TransformEffect::setOpacity(double val) { 
    if (m_data.opacity != val) { 
        m_data.opacity = val; 
        emit opacityChanged(); 
        emit modified(); 
    } 
}

QString TransformEffect::blendMode() const { return m_data.blendMode; }
void TransformEffect::setBlendMode(const QString& mode) { 
    if (m_data.blendMode != mode) { 
        m_data.blendMode = mode; 
        emit blendModeChanged(); 
        emit modified(); 
    } 
}

void TransformEffect::loadData(const TransformEffectData& data) {
    if (m_data.scale != data.scale) { m_data.scale = data.scale; emit scaleChanged(); }
    if (m_data.posX != data.posX) { m_data.posX = data.posX; emit posXChanged(); }
    if (m_data.posY != data.posY) { m_data.posY = data.posY; emit posYChanged(); }
    if (m_data.rotation != data.rotation) { m_data.rotation = data.rotation; emit rotationChanged(); }
    if (m_data.anchorX != data.anchorX) { m_data.anchorX = data.anchorX; emit anchorXChanged(); }
    if (m_data.anchorY != data.anchorY) { m_data.anchorY = data.anchorY; emit anchorYChanged(); }
    if (m_data.opacity != data.opacity) { m_data.opacity = data.opacity; emit opacityChanged(); }
    if (m_data.blendMode != data.blendMode) { m_data.blendMode = data.blendMode; emit blendModeChanged(); }
}

TransformEffectData TransformEffect::toData() const {
    return m_data;
}
