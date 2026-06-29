#include "EditorUndoStack.h"

EditorUndoStack::EditorUndoStack(QObject *parent)
    : QObject(parent)
{
}

EditorUndoStack::~EditorUndoStack() = default;

bool EditorUndoStack::canUndo() const
{
    return !m_undoStack.empty();
}

bool EditorUndoStack::canRedo() const
{
    return !m_redoStack.empty();
}

QString EditorUndoStack::undoText() const
{
    if (m_undoStack.empty()) return QString();
    return m_undoStack.back().description;
}

QString EditorUndoStack::redoText() const
{
    if (m_redoStack.empty()) return QString();
    return m_redoStack.back().description;
}

void EditorUndoStack::pushAction(const QString& description, std::function<void()> undoCall, std::function<void()> redoCall)
{
    m_undoStack.push_back({description, std::move(undoCall), std::move(redoCall)});
    m_redoStack.clear();
    emit canUndoChanged();
    emit canRedoChanged();
}

void EditorUndoStack::undo()
{
    if (!m_undoStack.empty()) {
        auto action = m_undoStack.back();
        m_undoStack.pop_back();
        if (action.undoCall) {
            action.undoCall();
        }
        m_redoStack.push_back(action);
        emit canUndoChanged();
        emit canRedoChanged();
    }
}

void EditorUndoStack::redo()
{
    if (!m_redoStack.empty()) {
        auto action = m_redoStack.back();
        m_redoStack.pop_back();
        if (action.redoCall) {
            action.redoCall();
        }
        m_undoStack.push_back(action);
        emit canUndoChanged();
        emit canRedoChanged();
    }
}

void EditorUndoStack::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
    emit canUndoChanged();
    emit canRedoChanged();
}
