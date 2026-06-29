#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <QDebug>
#include <QIcon>

#include <iostream>
extern "C" {
#include <libavutil/log.h>
}
#include "crash/CrashHandler.h"
#include "core/utils/MediaImageProvider.h"
#include "core/media/providers/AudioWaveformProvider.h"
#include "core/media/components/HardwareWaveformItem.h"
#include "core/actions/managers/ActionManager.h"
#include "core/actions/managers/MenuManager.h"
#include "core/actions/handlers/FileActions.h"
#include "core/actions/handlers/EditActions.h"
#include "core/actions/handlers/ClipActions.h"
#include "core/actions/handlers/SequenceActions.h"
#include "core/actions/handlers/ViewActions.h"
#include "core/actions/handlers/ToolActions.h"
#include "core/actions/handlers/HelpActions.h"
#include "core/effect_hub/models/EffectHubModel.h"
#include "core/effects/processors/GimpProcessor.h"
#include "core/editor/EditorBackendController.h"
#include "core/setting/SettingsController.h"
#include <QQmlContext>

extern void qml_register_types_VideoStudioUI();

int main(int argc, char *argv[])
{
    // Install crash handler FIRST — before anything else can crash.
    CrashHandler::install();

    // Suppress FFmpeg info/warning logs (like MP3 duration warnings)
    av_log_set_level(AV_LOG_ERROR);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    std::cout << "Step 1: Init App\n"; std::flush(std::cout);
    QApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    app.setOrganizationName("WeHunting");
    app.setOrganizationDomain("wehunting.com");
    app.setApplicationName("Video Studio");
    app.setWindowIcon(QIcon(":/VideoStudioUI/assets/app_icon.png"));

    // Initialize Action Categories
    FileActions fileActions;
    EditActions editActions;
    ClipActions clipActions;
    SequenceActions sequenceActions;
    ViewActions viewActions;
    ToolActions toolActions;
    HelpActions helpActions;

    std::cout << "Step 2: Init Engine\n"; std::flush(std::cout);
    
    // Explicitly register the QML module to prevent MinGW from optimizing it out
    qml_register_types_VideoStudioUI();
    
    QQmlApplicationEngine engine;

    // Attach Qt Message Handler after QML Engine to ensure it doesn't get overridden
    CrashHandler::attachQtMessageHandler();

    // Expose to QML as Singletons (Context Properties do not work well with pragma ComponentBehavior: Bound)
    qmlRegisterSingletonInstance("VideoStudio.Core", 1, 0, "ActionManager", &ActionManager::instance());
    qmlRegisterSingletonInstance("VideoStudio.Core", 1, 0, "MenuManager", &MenuManager::instance());
    qmlRegisterSingletonInstance("VideoStudio.Core", 1, 0, "EditorBackend", &EditorBackendController::instance());
    qmlRegisterSingletonInstance("VideoStudio.Core", 1, 0, "SettingsController", &SettingsController::instance());

    // Register Types
    qmlRegisterType<EffectHubModel>("VideoStudio.Models", 1, 0, "EffectHubModel");
    qmlRegisterType<GimpProcessor>("VideoStudio.Models", 1, 0, "GimpProcessor");
    qmlRegisterType<EditorImageModel>("VideoStudio.Editor", 1, 0, "EditorImageModel");
    qmlRegisterType<EditorLayerModel>("VideoStudio.Editor", 1, 0, "EditorLayerModel");
    qmlRegisterType<EditorUndoStack>("VideoStudio.Editor", 1, 0, "EditorUndoStack");
    qmlRegisterType<GeglGraphNode>("VideoStudio.Editor", 1, 0, "GeglGraphNode");
    qmlRegisterType<GeglOperationManager>("VideoStudio.Editor", 1, 0, "GeglOperationManager");
    qmlRegisterType<GeglOperationEngine>("VideoStudio.Editor", 1, 0, "GeglOperationEngine");
    qmlRegisterType<PaintDynamicsEngine>("VideoStudio.Editor", 1, 0, "PaintDynamicsEngine");
    qmlRegisterType<ToolOptionsModel>("VideoStudio.Editor", 1, 0, "ToolOptionsModel");
    qmlRegisterType<ToolController>("VideoStudio.Editor", 1, 0, "ToolController");
    qmlRegisterType<SelectionToolController>("VideoStudio.Editor", 1, 0, "SelectionToolController");
    qmlRegisterType<TransformToolController>("VideoStudio.Editor", 1, 0, "TransformToolController");
    qmlRegisterType<UtilityToolController>("VideoStudio.Editor", 1, 0, "UtilityToolController");
    qmlRegisterType<XcfProjectManager>("VideoStudio.Editor", 1, 0, "XcfProjectManager");
    qmlRegisterType<XcfFileStreamParser>("VideoStudio.Editor", 1, 0, "XcfFileStreamParser");
    qmlRegisterType<PdbRegistry>("VideoStudio.Editor", 1, 0, "PdbRegistry");
    qmlRegisterType<PdbProcedureManager>("VideoStudio.Editor", 1, 0, "PdbProcedureManager");
    qmlRegisterType<PluginIpcManager>("VideoStudio.Editor", 1, 0, "PluginIpcManager");
    qmlRegisterType<DisplayViewportModel>("VideoStudio.Editor", 1, 0, "DisplayViewportModel");
    qmlRegisterType<HardwareWaveformItem>("VideoStudio.Media", 1, 0, "HardwareWaveformItem");
    
    // Register Settings Types so they can be marshalled from SettingsController to QML
    qmlRegisterUncreatableType<GeneralSettings>("VideoStudio.Settings", 1, 0, "GeneralSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<AppearanceSettings>("VideoStudio.Settings", 1, 0, "AppearanceSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<TimelineSettings>("VideoStudio.Settings", 1, 0, "TimelineSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<PlaybackSettings>("VideoStudio.Settings", 1, 0, "PlaybackSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<AudioSettings>("VideoStudio.Settings", 1, 0, "AudioSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<CacheSettings>("VideoStudio.Settings", 1, 0, "CacheSettings", "Only created by SettingsController");
    qmlRegisterUncreatableType<KeyboardSettings>("VideoStudio.Settings", 1, 0, "KeyboardSettings", "Only created by SettingsController");

    // Build the menu structure now that all actions are registered
    MenuManager::instance().buildDefaultMenu();

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            std::cerr << warning.toString().toStdString() << std::endl;
        }
    });
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() {
                         QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);
    
    engine.addImageProvider("media", new MediaImageProvider);
    engine.addImageProvider("waveform", new AudioWaveformProvider);
    
    QFile qrcFile("qrc_dump.txt");
    if (qrcFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&qrcFile);
        QDirIterator it(":", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            out << it.next() << "\n";
        }
    }

    std::cout << "Step 3: Load URL\n"; std::flush(std::cout);
    const QUrl url(QStringLiteral("qrc:/VideoStudioUI/ui/Main.qml"));
    engine.load(url);

    std::cout << "Step 4: Exec\n"; std::flush(std::cout);
    return app.exec();
}
