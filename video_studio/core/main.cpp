#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <QDebug>

#include <iostream>
#include "core/utils/MediaImageProvider.h"
#include "core/media/AudioWaveformProvider.h"
#include "core/actions/ActionManager.h"
#include "core/actions/MenuManager.h"
#include "core/actions/FileActions.h"
#include "core/actions/EditActions.h"
#include "core/actions/ClipActions.h"
#include "core/actions/SequenceActions.h"
#include "core/actions/ViewActions.h"
#include "core/actions/ToolActions.h"
#include "core/actions/HelpActions.h"
#include <QQmlContext>

int main(int argc, char *argv[])
{
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
