#include "ToolOptionsModel.h"

ToolOptionsModel::ToolOptionsModel(QObject *parent)
    : QObject(parent)
{
}

ToolOptionsModel::~ToolOptionsModel() = default;

void ToolOptionsModel::setCurrentTool(const QString& tool)
{
    if (m_currentTool != tool) {
        m_currentTool = tool;
        emit currentToolChanged();
    }
}

void ToolOptionsModel::setBrushSize(qreal size)
{
    if (!qFuzzyCompare(m_brushSize, size)) {
        m_brushSize = size;
        emit brushSizeChanged();
    }
}

void ToolOptionsModel::setBrushHardness(qreal hardness)
{
    if (!qFuzzyCompare(m_brushHardness, hardness)) {
        m_brushHardness = hardness;
        emit brushHardnessChanged();
    }
}

void ToolOptionsModel::setForegroundColor(const QColor& color)
{
    if (m_foregroundColor != color) {
        m_foregroundColor = color;
        emit foregroundColorChanged();
    }
}

void ToolOptionsModel::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
    }
}
