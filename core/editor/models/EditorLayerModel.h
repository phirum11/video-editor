#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <QTransform>
#include <memory>

class EditorLayerModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QString blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)

public:
    explicit EditorLayerModel(QObject *parent = nullptr);
    ~EditorLayerModel() override;

    QString name() const { return m_name; }
    void setName(const QString& name);

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);

    QString blendMode() const { return m_blendMode; }
    void setBlendMode(const QString& mode);

    QImage buffer() const { return m_buffer; }
    void setBuffer(const QImage& buffer);

    QImage mask() const { return m_mask; }
    void setMask(const QImage& mask);

    QTransform transform() const { return m_transform; }
    void setTransform(const QTransform& transform);

signals:
    void nameChanged();
    void visibleChanged();
    void opacityChanged();
    void blendModeChanged();
    void bufferModified();
    void maskModified();
    void transformChanged();

private:
    QString m_name;
    bool m_visible = true;
    qreal m_opacity = 1.0;
    QString m_blendMode = "Normal";
    QImage m_buffer;
    QImage m_mask;
    QTransform m_transform;
};
