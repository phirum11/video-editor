#pragma once

#include <QObject>
#include <memory>
#include "models/EditorImageModel.h"
#include "models/EditorUndoStack.h"
#include "gegl/GeglOperationManager.h"
#include "gegl/GeglOperationEngine.h"
#include "tools/ToolController.h"
#include "tools/SelectionToolController.h"
#include "tools/TransformToolController.h"
#include "tools/UtilityToolController.h"
#include "paint/PaintDynamicsEngine.h"
#include "io/XcfProjectManager.h"
#include "io/XcfFileStreamParser.h"
#include "plugins/PdbRegistry.h"
#include "plugins/PdbProcedureManager.h"
#include "plugins/PluginIpcManager.h"
#include "display/DisplayViewportModel.h"

class EditorBackendController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EditorImageModel* imageModel READ imageModel CONSTANT)
    Q_PROPERTY(EditorUndoStack* undoStack READ undoStack CONSTANT)
    Q_PROPERTY(GeglOperationManager* operationManager READ operationManager CONSTANT)
    Q_PROPERTY(GeglOperationEngine* operationEngine READ operationEngine CONSTANT)
    Q_PROPERTY(ToolController* toolController READ toolController CONSTANT)
    Q_PROPERTY(SelectionToolController* selectionTool READ selectionTool CONSTANT)
    Q_PROPERTY(TransformToolController* transformTool READ transformTool CONSTANT)
    Q_PROPERTY(UtilityToolController* utilityTool READ utilityTool CONSTANT)
    Q_PROPERTY(PaintDynamicsEngine* paintDynamics READ paintDynamics CONSTANT)
    Q_PROPERTY(XcfProjectManager* projectManager READ projectManager CONSTANT)
    Q_PROPERTY(XcfFileStreamParser* xcfParser READ xcfParser CONSTANT)
    Q_PROPERTY(PdbRegistry* pdbRegistry READ pdbRegistry CONSTANT)
    Q_PROPERTY(PdbProcedureManager* pdbProcedureManager READ pdbProcedureManager CONSTANT)
    Q_PROPERTY(PluginIpcManager* pluginIpc READ pluginIpc CONSTANT)
    Q_PROPERTY(DisplayViewportModel* viewport READ viewport CONSTANT)

public:
    static EditorBackendController& instance();

    EditorImageModel* imageModel() const { return m_imageModel; }
    EditorUndoStack* undoStack() const { return m_undoStack; }
    GeglOperationManager* operationManager() const { return m_operationManager; }
    GeglOperationEngine* operationEngine() const { return m_operationEngine; }
    ToolController* toolController() const { return m_toolController; }
    SelectionToolController* selectionTool() const { return m_selectionTool; }
    TransformToolController* transformTool() const { return m_transformTool; }
    UtilityToolController* utilityTool() const { return m_utilityTool; }
    PaintDynamicsEngine* paintDynamics() const { return m_paintDynamics; }
    XcfProjectManager* projectManager() const { return m_projectManager; }
    XcfFileStreamParser* xcfParser() const { return m_xcfParser; }
    PdbRegistry* pdbRegistry() const { return m_pdbRegistry; }
    PdbProcedureManager* pdbProcedureManager() const { return m_pdbProcedureManager; }
    PluginIpcManager* pluginIpc() const { return m_pluginIpc; }
    DisplayViewportModel* viewport() const { return m_viewport; }

    Q_INVOKABLE void initializeDefaultProject();

signals:
    void projectInitialized();

private:
    explicit EditorBackendController(QObject *parent = nullptr);
    ~EditorBackendController() override;

    EditorImageModel* m_imageModel;
    EditorUndoStack* m_undoStack;
    GeglOperationManager* m_operationManager;
    GeglOperationEngine* m_operationEngine;
    ToolController* m_toolController;
    SelectionToolController* m_selectionTool;
    TransformToolController* m_transformTool;
    UtilityToolController* m_utilityTool;
    PaintDynamicsEngine* m_paintDynamics;
    XcfProjectManager* m_projectManager;
    XcfFileStreamParser* m_xcfParser;
    PdbRegistry* m_pdbRegistry;
    PdbProcedureManager* m_pdbProcedureManager;
    PluginIpcManager* m_pluginIpc;
    DisplayViewportModel* m_viewport;
};
