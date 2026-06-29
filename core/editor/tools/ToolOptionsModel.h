#pragma once

#include <QObject>
#include <QString>
#include <QColor>

class ToolOptionsModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentTool READ currentTool WRITE setCurrentTool NOTIFY currentToolChanged)
    Q_PROPERTY(qreal brushSize READ brushSize WRITE setBrushSize NOTIFY brushSizeChanged)
    Q_PROPERTY(qreal brushHardness READ brushHardness WRITE setBrushHardness NOTIFY brushHardnessChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    explicit ToolOptionsModel(QObject *parent = nullptr);
    ~ToolOptionsModel() override;

    QString currentTool() const { return m_currentTool; }
    void setCurrentTool(const QString& tool);

    qreal brushSize() const { return m_brushSize; }
    void setBrushSize(qreal size);

    qreal brushHardness() const { return m_brushHardness; }
    void setBrushHardness(qreal hardness);

    QColor foregroundColor() const { return m_foregroundColor; }
    void setForegroundColor(const QColor& color);

    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor& color);

signals:
    void currentToolChanged();
    void brushSizeChanged();
    void brushHardnessChanged();
    void foregroundColorChanged();
    void backgroundColorChanged();

private:
    QString m_currentTool = "Paintbrush";
    qreal m_brushSize = 20.0;
    qreal m_brushHardness = 0.8;
    QColor m_foregroundColor = QColor("#000000");
    QColor m_backgroundColor = QColor("#ffffff");
};
