#include "EffectController.h"
#include "core/playback/PlaybackEngine.h"

EffectController::EffectController(QObject *parent) : QObject(parent),
    m_transform(new TransformEffect(this)),
    m_color(new ColorEffect(this)),
    m_blur(new BlurEffect(this)),
    m_stylize(new StylizeEffect(this)),
    m_audio(new AudioEffect(this))
{
    connect(m_transform, &TransformEffect::modified, this, &EffectController::onEffectModified);
    connect(m_color, &ColorEffect::modified, this, &EffectController::onEffectModified);
    connect(m_blur, &BlurEffect::modified, this, &EffectController::onEffectModified);
    connect(m_stylize, &StylizeEffect::modified, this, &EffectController::onEffectModified);
    connect(m_audio, &AudioEffect::modified, this, &EffectController::onEffectModified);
}

EffectController::~EffectController()
{
}

void EffectController::setTimelineController(TimelineController* controller)
{
    if (m_timelineController == controller)
        return;

    if (m_timelineController) {
        disconnect(m_timelineController, &TimelineController::selectedClipIndexChanged, this, &EffectController::onSelectedClipIndexChanged);
    }

    m_timelineController = controller;

    if (m_timelineController) {
        m_clipModel = qobject_cast<TimelineClipModel*>(m_timelineController->clipModel());
        connect(m_timelineController, &TimelineController::selectedClipIndexChanged, this, &EffectController::onSelectedClipIndexChanged);
    } else {
        m_clipModel = nullptr;
    }

    emit timelineControllerChanged();
    
    // Initial load
    onSelectedClipIndexChanged();
}

void EffectController::setPlaybackEngine(PlaybackEngine* engine)
{
    if (m_playbackEngine == engine)
        return;
    m_playbackEngine = engine;
    if (m_playbackEngine) {
        m_playbackEngine->setClipEffects(currentEffects());
    }
    emit playbackEngineChanged();
}

void EffectController::onSelectedClipIndexChanged()
{
    if (!m_timelineController || !m_clipModel)
        return;

    int row = m_timelineController->selectedClipIndex();
    loadClipEffects(row);
}

void EffectController::onEffectModified()
{
    if (m_isUpdatingFromModel)
        return;

    const ClipEffects effects = currentEffects();
    if (m_timelineController && m_clipModel) {
        int row = m_timelineController->selectedClipIndex();
        if (row >= 0) {
            saveClipEffects(row);
        }
    }

    if (m_playbackEngine) {
        m_playbackEngine->setClipEffects(effects);
    }
}

void EffectController::loadClipEffects(int row)
{
    m_isUpdatingFromModel = true;
    
    ClipEffects effects;
    if (row >= 0 && m_clipModel) {
        effects = m_clipModel->clipEffectsAt(row);
    }

    m_transform->loadData(effects.transform);
    m_color->loadData(effects.color);
    m_blur->loadData(effects.blur);
    m_stylize->loadData(effects.stylize);
    m_audio->loadData(effects.audio);

    if (m_playbackEngine) {
        m_playbackEngine->setClipEffects(effects);
    }
    
    m_isUpdatingFromModel = false;
}

ClipEffects EffectController::currentEffects() const
{
    ClipEffects effects;
    effects.transform = m_transform->toData();
    effects.color = m_color->toData();
    effects.blur = m_blur->toData();
    effects.stylize = m_stylize->toData();
    effects.audio = m_audio->toData();
    return effects;
}

void EffectController::saveClipEffects(int row)
{
    if (row >= 0 && m_clipModel) {
        m_clipModel->updateClipEffects(row, currentEffects());
    }
}

void EffectController::resetAll()
{
    m_isUpdatingFromModel = true;
    
    m_transform->loadData(TransformEffectData());
    m_color->loadData(ColorEffectData());
    m_blur->loadData(BlurEffectData());
    m_stylize->loadData(StylizeEffectData());
    m_audio->loadData(AudioEffectData());
    
    m_isUpdatingFromModel = false;
    
    // Save to model
    const ClipEffects effects = currentEffects();
    if (m_timelineController && m_clipModel) {
        int row = m_timelineController->selectedClipIndex();
        if (row >= 0) {
            saveClipEffects(row);
        }
    }

    if (m_playbackEngine) {
        m_playbackEngine->setClipEffects(effects);
    }
}
