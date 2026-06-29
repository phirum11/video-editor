#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <memory>
#include "EditorLayerModel.h"

class EditorImageModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY dimensionsChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY dimensionsChanged)

public:
    explicit EditorImageModel(QObject *parent = nullptr);
    ~EditorImageModel() override;

    QString title() const { return m_title; }
    void setTitle(const QString& title);

    int width() const { return m_width; }
    void setWidth(int width);

    int height() const { return m_height; }
    void setHeight(int height);

    Q_INVOKABLE void addLayer(EditorLayerModel* layer);
    Q_INVOKABLE void removeLayer(int index);
    Q_INVOKABLE EditorLayerModel* layerAt(int index) const;
    Q_INVOKABLE int layerCount() const;

signals:
    void titleChanged();
    void dimensionsChanged();
    void layersModified();

private:
    QString m_title;
    int m_width = 1920;
    int m_height = 1080;
    std::vector<std::shared_ptr<EditorLayerModel>> m_layers;
};
