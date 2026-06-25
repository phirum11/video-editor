#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <QDebug>

#include <iostream>
extern "C" {
#include <libavutil/log.h>
}
#include "core/utils/MediaImageProvider.h"
#include "core/media/providers/AudioWaveformProvider.h"
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
#include <QQmlContext>

int main(int argc, char *argv[])
{
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

    // Initialize Action Categories
    FileActions fileActions;
    EditActions editActions;
    ClipActions clipActions;
    SequenceActions sequenceActions;
    ViewActions viewActions;
    ToolActions toolActions;
    HelpActions helpActions;

    std::cout << "Step 2: Init Engine\n"; std::flush(std::cout);
    QQmlApplicationEngine engine;

    // Expose to QML
    engine.rootContext()->setContextProperty("ActionManager", &ActionManager::instance());
    engine.rootContext()->setContextProperty("MenuManager", &MenuManager::instance());

    // Register Types
    qmlRegisterType<EffectHubModel>("VideoStudio.Models", 1, 0, "EffectHubModel");

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
