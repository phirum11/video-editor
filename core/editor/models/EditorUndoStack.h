#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <functional>

struct EditorAction {
    QString description;
    std::function<void()> undoCall;
    std::function<void()> redoCall;
};

class EditorUndoStack : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY canUndoChanged)
    Q_PROPERTY(QString redoText READ redoText NOTIFY canRedoChanged)

public:
    explicit EditorUndoStack(QObject *parent = nullptr);
    ~EditorUndoStack() override;

    bool canUndo() const;
    bool canRedo() const;
    QString undoText() const;
    QString redoText() const;

    void pushAction(const QString& description, std::function<void()> undoCall, std::function<void()> redoCall);

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void clear();

signals:
    void canUndoChanged();
    void canRedoChanged();

private:
    std::vector<EditorAction> m_undoStack;
    std::vector<EditorAction> m_redoStack;
};
