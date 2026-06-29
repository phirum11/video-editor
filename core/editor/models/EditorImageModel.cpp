#include "EditorImageModel.h"

EditorImageModel::EditorImageModel(QObject *parent)
    : QObject(parent)
{
}

EditorImageModel::~EditorImageModel() = default;

void EditorImageModel::setTitle(const QString& title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void EditorImageModel::setWidth(int width)
{
    if (m_width != width) {
        m_width = width;
        emit dimensionsChanged();
    }
}

void EditorImageModel::setHeight(int height)
{
    if (m_height != height) {
        m_height = height;
        emit dimensionsChanged();
    }
}

void EditorImageModel::addLayer(EditorLayerModel* layer)
{
    if (layer) {
        m_layers.push_back(std::shared_ptr<EditorLayerModel>(layer, [](EditorLayerModel*){}));
        emit layersModified();
    }
}

EditorLayerModel* EditorImageModel::layerAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_layers.size())) {
        return m_layers[index].get();
    }
    return nullptr;
}

void EditorImageModel::removeLayer(int index)
{
    if (index >= 0 && index < static_cast<int>(m_layers.size())) {
        m_layers.erase(m_layers.begin() + index);
        emit layersModified();
    }
}

int EditorImageModel::layerCount() const
{
    return static_cast<int>(m_layers.size());
}
