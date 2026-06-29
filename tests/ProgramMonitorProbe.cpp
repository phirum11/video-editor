#include "core/playback/engines/PlaybackEngine.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QMetaObject>
#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QVariantMap>
#include <QtQml>

#include <algorithm>
#include <iostream>

namespace {

class FakeTimelineController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int clipCount READ clipCount NOTIFY timelineChanged)
    Q_PROPERTY(double timelineEndSeconds READ timelineEndSeconds NOTIFY timelineChanged)

public:
    int clipCount() const { return m_clips.size(); }

    double timelineEndSeconds() const
    {
        double end = 0.0;
        for (const QVariantMap& clip : m_clips) {
            end = std::max(end,
                           clip.value(QStringLiteral("startSeconds")).toDouble()
                               + clip.value(QStringLiteral("durationSeconds")).toDouble());
        }
        return end;
    }

    void addClip(const QVariantMap& clip)
    {
        m_clips.append(clip);
        emit timelineChanged();
    }

    Q_INVOKABLE QVariantMap clipAt(int row) const
    {
        return row >= 0 && row < m_clips.size() ? m_clips.at(row) : QVariantMap();
    }

    Q_INVOKABLE bool clipContains(int row, double seconds) const
    {
        if (row < 0 || row >= m_clips.size()) {
            return false;
        }

        const QVariantMap clip = m_clips.at(row);
        const double start = clip.value(QStringLiteral("startSeconds")).toDouble();
        const double end = start + clip.value(QStringLiteral("durationSeconds")).toDouble();
        return seconds >= start && seconds <= end;
    }

    Q_INVOKABLE bool isTrackHidden(bool, int) const { return false; }
    Q_INVOKABLE bool isTrackMuted(bool, int) const { return false; }

signals:
    void timelineChanged();
    void trackStateChanged(bool isVideo, int trackIndex);

private:
    QVector<QVariantMap> m_clips;
};

int fail(const QString& message)
{
    std::cerr << message.toStdString() << '\n';
    return 1;
}

QVariantMap makeClip(const QString& name,
                     const QString& path,
                     double duration,
                     bool hasVideo,
                     bool hasAudio,
                     int trackIndex)
{
    QVariantMap clip;
    clip.insert(QStringLiteral("clipName"), name);
    clip.insert(QStringLiteral("filePath"), path);
    clip.insert(QStringLiteral("originalFilePath"), QString());
    clip.insert(QStringLiteral("startSeconds"), 0.0);
    clip.insert(QStringLiteral("durationSeconds"), duration);
    clip.insert(QStringLiteral("trackIndex"), trackIndex);
    clip.insert(QStringLiteral("hasVideo"), hasVideo);
    clip.insert(QStringLiteral("hasAudio"), hasAudio);
    clip.insert(QStringLiteral("sourceInPoint"), 0.0);
    clip.insert(QStringLiteral("sourceDuration"), duration);
    clip.insert(QStringLiteral("isMuted"), false);
    clip.insert(QStringLiteral("isEffect"), false);
    return clip;
}

bool invokeLoadClip(QObject* root,
                    const QString& name,
                    const QString& path,
                    double duration,
                    bool hasVideo,
                    int row)
{
    return QMetaObject::invokeMethod(root,
                                     "loadClipWithOffset",
                                     Q_ARG(QVariant, name),
                                     Q_ARG(QVariant, path),
                                     Q_ARG(QVariant, duration),
                                     Q_ARG(QVariant, hasVideo),
                                     Q_ARG(QVariant, 0.0),
                                     Q_ARG(QVariant, 0.0),
                                     Q_ARG(QVariant, row),
                                     Q_ARG(QVariant, false));
}

} // namespace

int main(int argc, char* argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QGuiApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 4) {
        std::cerr << "Usage: ProgramMonitorProbe <source-root> <video-file> <audio-file>\n";
        return 2;
    }

    const QString sourceRoot = args.at(1);
    const QString videoPath = QFileInfo(args.at(2)).absoluteFilePath();
    const QString audioPath = QFileInfo(args.at(3)).absoluteFilePath();

    qmlRegisterType<PlaybackEngine>("VideoStudioUI", 1, 0, "PlaybackEngine");

    QQmlEngine engine;
    engine.addImportPath(sourceRoot + QStringLiteral("/build_mingw"));
    engine.addImportPath(sourceRoot);
    engine.addImportPath(sourceRoot + QStringLiteral("/ui"));

    QQmlComponent component(
        &engine,
        QUrl::fromLocalFile(sourceRoot + QStringLiteral("/ui/monitors/ProgramMonitor.qml"))
    );

    QObject* root = component.create();
    if (!root) {
        const QList<QQmlError> errors = component.errors();
        for (const QQmlError& error : errors) {
            std::cerr << error.toString().toStdString() << '\n';
        }
        return 1;
    }

    root->setProperty("width", 900);
    root->setProperty("height", 500);

    FakeTimelineController timeline;
    timeline.addClip(makeClip(QStringLiteral("EP002.mp4"), videoPath, 220.45, true, true, 0));
    timeline.addClip(makeClip(QStringLiteral("audio.mp3"), audioPath, 264.0, false, true, 100));
    root->setProperty("timelineCtrl", QVariant::fromValue<QObject*>(&timeline));

    if (!invokeLoadClip(root, QStringLiteral("EP002.mp4"), videoPath, 220.45, true, 0)) {
        delete root;
        return fail(QStringLiteral("Could not load video clip through ProgramMonitor"));
    }

    QCoreApplication::processEvents();

    QObject* playback = qvariant_cast<QObject*>(root->property("playbackEngine"));
    if (!playback) {
        delete root;
        return fail(QStringLiteral("ProgramMonitor did not expose playbackEngine"));
    }

    if (!root->property("hasVideo").toBool()
        || playback->property("filePath").toString() != videoPath) {
        delete root;
        return fail(QStringLiteral("Video selection did not load the main preview engine"));
    }

    if (!invokeLoadClip(root, QStringLiteral("audio.mp3"), audioPath, 264.0, false, 1)) {
        delete root;
        return fail(QStringLiteral("Could not select audio clip through ProgramMonitor"));
    }

    for (int i = 0; i < 5; ++i) {
        QCoreApplication::processEvents();
    }

    const bool audioSelected =
        root->property("hasClip").toBool()
        && !root->property("hasVideo").toBool()
        && root->property("hasAudio").toBool()
        && root->property("filePath").toString() == audioPath;

    const bool mainPreviewUnloaded =
        playback->property("filePath").toString().isEmpty()
        && !playback->property("hasVideo").toBool()
        && !playback->property("hasAudio").toBool();

    delete root;

    if (!audioSelected) {
        return fail(QStringLiteral("Audio selection did not switch ProgramMonitor to audio-only selection state"));
    }
    if (!mainPreviewUnloaded) {
        return fail(QStringLiteral("Audio selection left the main preview decoder loaded"));
    }

    std::cout << "audioSelectionState=ok\n";
    return 0;
}

#include "ProgramMonitorProbe.moc"
