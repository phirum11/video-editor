#include "ui/timeline_view/controllers/TimelineController.h"
#include "ui/timeline_view/controllers/ClipTrimController.h"
#include <QGuiApplication>
#include <QMetaObject>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <QUrl>
#include <QtQml>

#include <iostream>

namespace {
void dumpClipItems(QObject* object, int* count, QSet<QObject*>* visited)
{
    if (!object || visited->contains(object)) {
        return;
    }
    visited->insert(object);

    const QVariant clipName = object->property("clipName");
    const QVariant clipIndex = object->property("clipIndex");
    if (clipName.isValid() && clipIndex.isValid()) {
        ++(*count);
        std::cout << "clipDelegate index=" << clipIndex.toInt()
                  << " name=" << clipName.toString().toStdString()
                  << " x=" << object->property("x").toDouble()
                  << " y=" << object->property("y").toDouble()
                  << " width=" << object->property("width").toDouble()
                  << " height=" << object->property("height").toDouble()
                  << " visible=" << object->property("visible").toBool()
                  << '\n';
    }

    for (QObject* child : object->children()) {
        dumpClipItems(child, count, visited);
    }

    if (QQuickItem* item = qobject_cast<QQuickItem*>(object)) {
        for (QQuickItem* childItem : item->childItems()) {
            dumpClipItems(childItem, count, visited);
        }
    }
}

int fail(const QString& message)
{
    std::cerr << message.toStdString() << '\n';
    return 1;
}
}

int main(int argc, char* argv[])
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);

    if (app.arguments().size() < 2) {
        std::cerr << "Usage: TimelineQmlProbe <source-root>\n";
        return 2;
    }

    qmlRegisterType<TimelineController>("VideoStudioUI", 1, 0, "TimelineController");
    qmlRegisterType<ClipTrimController>("VideoStudioUI", 1, 0, "ClipTrimController");

    const QString sourceRoot = app.arguments().at(1);
    QQmlEngine engine;
    engine.addImportPath(sourceRoot + QStringLiteral("/tests/qml_probe_imports"));
    engine.addImportPath(sourceRoot);
    engine.addImportPath(sourceRoot + QStringLiteral("/ui"));
    engine.addImportPath(sourceRoot + QStringLiteral("/ui/timeline_view"));

    QQmlComponent rulerComponent(
        &engine,
        QUrl::fromLocalFile(sourceRoot + QStringLiteral("/ui/timeline_view/TimelineRuler.qml"))
    );
    QObject* ruler = rulerComponent.create();
    if (!ruler) {
        const QList<QQmlError> errors = rulerComponent.errors();
        for (const QQmlError& error : errors) {
            std::cerr << error.toString().toStdString() << '\n';
        }
        return 1;
    }
    ruler->setProperty("width", 1700.0);
    ruler->setProperty("contentWidth", 1700.0);
    ruler->setProperty("pixelsPerSecond", 1700.0 / (80.0 * 60.0));
    const double wideMajorStep = ruler->property("majorStepSeconds").toDouble();

    QVariant formattedTenMinutes;
    const bool formatted = QMetaObject::invokeMethod(
        ruler,
        "formatTime",
        Q_RETURN_ARG(QVariant, formattedTenMinutes),
        Q_ARG(QVariant, 600.0)
    );
    if (!formatted) {
        delete ruler;
        return fail(QStringLiteral("Could not invoke TimelineRuler.formatTime"));
    }

    ruler->setProperty("pixelsPerSecond", 72.0);
    const double zoomedMajorStep = ruler->property("majorStepSeconds").toDouble();

    std::cout << "wideMajorStep=" << wideMajorStep << '\n'
              << "tenMinuteLabel=" << formattedTenMinutes.toString().toStdString() << '\n'
              << "zoomedMajorStep=" << zoomedMajorStep << '\n';

    if (wideMajorStep != 600.0) {
        delete ruler;
        return fail(QStringLiteral("Wide ruler should use 10-minute major steps"));
    }
    if (formattedTenMinutes.toString() != QStringLiteral("10:00")) {
        delete ruler;
        return fail(QStringLiteral("Ten-minute ruler label should render as 10:00"));
    }
    if (zoomedMajorStep > 5.0) {
        delete ruler;
        return fail(QStringLiteral("Zoomed ruler should switch to second-level major steps"));
    }
    delete ruler;

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(sourceRoot + QStringLiteral("/ui/timeline_view/TimelineTracks.qml"))
    );

    QObject* root = component.create();
    if (!root) {
        const QList<QQmlError> errors = component.errors();
        for (const QQmlError& error : errors) {
            std::cerr << error.toString().toStdString() << '\n';
        }
        return 1;
    }

    root->setProperty("width", 1200);
    root->setProperty("height", 420);

    QVariant returnedRow;
    const bool invoked = QMetaObject::invokeMethod(
        root,
        "addMediaClip",
        Q_RETURN_ARG(QVariant, returnedRow),
        Q_ARG(QVariant, QStringLiteral("EP001.mp4")),
        Q_ARG(QVariant, QStringLiteral("C:/tmp/EP001.mp4")),
        Q_ARG(QVariant, 12.0),
        Q_ARG(QVariant, true),
        Q_ARG(QVariant, true),
        Q_ARG(QVariant, 3.2),
        Q_ARG(QVariant, -1)
    );
    if (!invoked) {
        return fail(QStringLiteral("Could not invoke addMediaClip"));
    }

    for (int i = 0; i < 5; ++i) {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    }

    TimelineController* controller = root->findChild<TimelineController*>();
    if (!controller) {
        return fail(QStringLiteral("TimelineController was not created"));
    }

    std::cout << "returnedRow=" << returnedRow.toInt() << '\n'
              << "clipCount=" << controller->clipCount() << '\n'
              << "timelineEnd=" << controller->endTimeSeconds() << '\n';

    int delegateCount = 0;
    QSet<QObject*> clipVisited;
    dumpClipItems(root, &delegateCount, &clipVisited);
    std::cout << "delegateCount=" << delegateCount << '\n';

    if (controller->clipCount() <= 0) {
        return fail(QStringLiteral("No clips were added to the model"));
    }
    if (delegateCount <= 0) {
        return fail(QStringLiteral("No timeline clip delegates were created"));
    }

    delete root;
    return 0;
}

