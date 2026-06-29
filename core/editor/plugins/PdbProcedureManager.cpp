#include "PdbProcedureManager.h"
#include <QDebug>

PdbProcedureManager::PdbProcedureManager(QObject *parent)
    : QObject(parent)
    , m_registry(new PdbRegistry(this))
{
    registerDefaultProcedures();
}

PdbProcedureManager::~PdbProcedureManager() = default;

QStringList PdbProcedureManager::availableCategories() const
{
    QStringList categories;
    for (const auto& proc : m_procedures) {
        if (!categories.contains(proc.category)) {
            categories.append(proc.category);
        }
    }
    return categories;
}

QStringList PdbProcedureManager::getProceduresInCategory(const QString& category) const
{
    QStringList procs;
    for (const auto& proc : m_procedures) {
        if (proc.category == category) {
            procs.append(proc.name);
        }
    }
    return procs;
}

QVariantMap PdbProcedureManager::getProcedureDetails(const QString& procName) const
{
    for (const auto& proc : m_procedures) {
        if (proc.name == procName) {
            QVariantMap map;
            map["category"] = proc.category;
            map["name"] = proc.name;
            map["blurb"] = proc.blurb;
            map["help"] = proc.help;
            map["argumentTypes"] = proc.argumentTypes;
            return map;
        }
    }
    return QVariantMap();
}

bool PdbProcedureManager::runProcedure(const QString& procName, const QVariantMap& args)
{
    qDebug() << "PdbProcedureManager running procedure:" << procName << args;
    bool success = m_registry->executeProcedure(procName, args);
    emit procedureInvoked(procName, success);
    return success;
}

void PdbProcedureManager::registerDefaultProcedures()
{
    // Registering the 25 major PDB command groups
    m_procedures.append({"Brush", "gimp-brushes-get-list", "Retrieve available brushes", "Returns a list of all brush names.", {"STRING"}});
    m_procedures.append({"Buffer", "gimp-buffer-get-width", "Get buffer width", "Returns width of the specified cut buffer.", {"STRING"}});
    m_procedures.append({"Channel", "gimp-channel-new", "Create new channel", "Creates a new selection channel.", {"INT32", "STRING", "FLOAT"}});
    m_procedures.append({"Context", "gimp-context-set-foreground", "Set active foreground color", "Sets the active foreground paint color.", {"COLOR"}});
    m_procedures.append({"Display", "gimp-display-new", "Create new display viewport", "Creates a new viewport for an image.", {"INT32"}});
    m_procedures.append({"Drawable", "gimp-drawable-invert", "Invert drawable pixels", "Inverts the color of the active drawable.", {"INT32"}});
    m_procedures.append({"Edit", "gimp-edit-copy", "Copy selection", "Copies active selection to cut buffer.", {"INT32"}});
    m_procedures.append({"File", "gimp-file-load", "Load image file", "Loads an image from filesystem.", {"STRING", "STRING"}});
    m_procedures.append({"FloatingSel", "gimp-floating-sel-anchor", "Anchor floating selection", "Anchors floating selection to its drawable.", {"INT32"}});
    m_procedures.append({"Font", "gimp-fonts-get-list", "Retrieve available fonts", "Returns a list of available system fonts.", {"STRING"}});
    m_procedures.append({"Gradient", "gimp-gradients-get-list", "Retrieve available gradients", "Returns a list of all gradient names.", {"STRING"}});
    m_procedures.append({"Image", "gimp-image-resize", "Resize canvas", "Resizes the bounding canvas of an image.", {"INT32", "INT32", "INT32"}});
    m_procedures.append({"Item", "gimp-item-set-visible", "Set item visibility", "Enables or disables item render visibility.", {"INT32", "BOOLEAN"}});
    m_procedures.append({"Layer", "gimp-layer-new", "Create new layer", "Creates a new blank image layer.", {"INT32", "STRING", "INT32", "INT32"}});
    m_procedures.append({"Palette", "gimp-palettes-get-list", "Retrieve available palettes", "Returns a list of all color palettes.", {"STRING"}});
    m_procedures.append({"Path", "gimp-path-get-points", "Get vector path points", "Returns control points for a bezier path.", {"INT32"}});
    m_procedures.append({"Text", "gimp-text-layer-set-text", "Set text layer content", "Sets the text string for a text layer.", {"INT32", "STRING"}});
    m_procedures.append({"Vector", "gimp-vectors-new", "Create new vectors object", "Creates a new empty vector path object.", {"INT32", "STRING"}});
}
