#include "EditorBackendController.h"
#include <QDebug>

EditorBackendController& EditorBackendController::instance()
{
    static EditorBackendController s_instance;
    return s_instance;
}

EditorBackendController::EditorBackendController(QObject *parent)
    : QObject(parent)
    , m_imageModel(new EditorImageModel(this))
    , m_undoStack(new EditorUndoStack(this))
    , m_operationManager(new GeglOperationManager(this))
    , m_operationEngine(new GeglOperationEngine(this))
    , m_toolController(new ToolController(this))
    , m_selectionTool(new SelectionToolController(this))
    , m_transformTool(new TransformToolController(this))
    , m_utilityTool(new UtilityToolController(this))
    , m_paintDynamics(new PaintDynamicsEngine(this))
    , m_projectManager(new XcfProjectManager(this))
    , m_xcfParser(new XcfFileStreamParser(this))
    , m_pdbRegistry(new PdbRegistry(this))
    , m_pdbProcedureManager(new PdbProcedureManager(this))
    , m_pluginIpc(new PluginIpcManager(this))
    , m_viewport(new DisplayViewportModel(this))
{
    connect(m_toolController, &ToolController::strokeFinished, this, [this]() {
        m_undoStack->pushAction("Tool stroke", [](){}, [](){});
    });
}

EditorBackendController::~EditorBackendController() = default;

void EditorBackendController::initializeDefaultProject()
{
    qDebug() << "EditorBackendController initializing default project";
    m_imageModel->setTitle("Untitled Project");
    m_imageModel->setWidth(1920);
    m_imageModel->setHeight(1080);

    auto* defaultLayer = new EditorLayerModel(m_imageModel);
    defaultLayer->setName("Background");
    QImage bg(1920, 1080, QImage::Format_ARGB32_Premultiplied);
    bg.fill(Qt::white);
    defaultLayer->setBuffer(bg);

    m_imageModel->addLayer(defaultLayer);
    m_undoStack->clear();

    emit projectInitialized();
}
