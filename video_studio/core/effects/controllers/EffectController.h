#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "core/effects/models/TransformEffect.h"
#include "core/effects/models/ColorEffect.h"
#include "core/effects/models/BlurEffect.h"
#include "core/effects/models/StylizeEffect.h"
#include "core/effects/models/AudioEffect.h"
#include "core/effects/models/ChromaKeyEffect.h"

#include "ui/timeline_view/controllers/TimelineController.h"
#include "core/timeline/models/TimelineClipModel.h"

#include "core/playback/engines/PlaybackEngine.h"

class EffectController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(PlaybackEngine* playbackEngine READ playbackEngine WRITE setPlaybackEngine NOTIFY playbackEngineChanged)
    Q_PROPERTY(TransformEffect* transform READ transform CONSTANT)
    Q_PROPERTY(ColorEffect* color READ color CONSTANT)
    Q_PROPERTY(BlurEffect* blur READ blur CONSTANT)
    Q_PROPERTY(StylizeEffect* stylize READ stylize CONSTANT)
    Q_PROPERTY(AudioEffect* audio READ audio CONSTANT)
    Q_PROPERTY(ChromaKeyEffect* chromaKey READ chromaKey CONSTANT)
    
    Q_PROPERTY(TimelineController* timelineController READ timelineController WRITE setTimelineController NOTIFY timelineControllerChanged)

public:
    explicit EffectController(QObject *parent = nullptr);
    ~EffectController() override;

    TransformEffect* transform() const { return m_transform; }
    ColorEffect* color() const { return m_color; }
    BlurEffect* blur() const { return m_blur; }
    StylizeEffect* stylize() const { return m_stylize; }
    AudioEffect* audio() const { return m_audio; }
    ChromaKeyEffect* chromaKey() const { return m_chromaKey; }

    TimelineController* timelineController() const { return m_timelineController; }
    void setTimelineController(TimelineController* controller);

    PlaybackEngine* playbackEngine() const { return m_playbackEngine; }
    void setPlaybackEngine(PlaybackEngine* engine);

    Q_INVOKABLE void resetAll();

signals:
    void timelineControllerChanged();
    void playbackEngineChanged();

private slots:
    void onSelectedClipIndexChanged();
    void onEffectModified();

private:
    ClipEffects currentEffects() const;
    void loadClipEffects(int row);
    void saveClipEffects(int row);

    TransformEffect* m_transform;
    ColorEffect* m_color;
    BlurEffect* m_blur;
    StylizeEffect* m_stylize;
    AudioEffect* m_audio;
    ChromaKeyEffect* m_chromaKey;

    TimelineController* m_timelineController = nullptr;
    TimelineClipModel* m_clipModel = nullptr;
    PlaybackEngine* m_playbackEngine = nullptr;

    bool m_isUpdatingFromModel = false;
};

