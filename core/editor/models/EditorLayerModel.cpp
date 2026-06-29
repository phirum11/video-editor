#include "EditorLayerModel.h"

EditorLayerModel::EditorLayerModel(QObject *parent)
    : QObject(parent)
{
}

EditorLayerModel::~EditorLayerModel() = default;

void EditorLayerModel::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void EditorLayerModel::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        emit visibleChanged();
    }
}

void EditorLayerModel::setOpacity(qreal opacity)
{
    if (!qFuzzyCompare(m_opacity, opacity)) {
        m_opacity = opacity;
        emit opacityChanged();
    }
}

void EditorLayerModel::setBlendMode(const QString& mode)
{
    if (m_blendMode != mode) {
        m_blendMode = mode;
        emit blendModeChanged();
    }
}

void EditorLayerModel::setBuffer(const QImage& buffer)
{
    m_buffer = buffer;
    emit bufferModified();
}

void EditorLayerModel::setMask(const QImage& mask)
{
    m_mask = mask;
    emit maskModified();
}

void EditorLayerModel::setTransform(const QTransform& transform)
{
    m_transform = transform;
    emit transformChanged();
}
