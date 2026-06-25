#include "core/effects/controllers/EffectController.h"
#include "core/playback/engines/PlaybackEngine.h"

#include <QDir>
#include <QUrl>

#include <algorithm>
#include <cmath>

namespace {

bool differs(double value, double defaultValue)
{
    return std::abs(value - defaultValue) > 0.0001;
}

bool differs(const QString& value, const QString& defaultValue)
{
    return value.compare(defaultValue, Qt::CaseInsensitive) != 0;
}

void mergeEffectLayer(ClipEffects& base, const ClipEffects& layer)
{
    const ClipEffects defaults;

    if (differs(layer.transform.scale, defaults.transform.scale)) {
        base.transform.scale = layer.transform.scale;
    }
    if (differs(layer.transform.posX, defaults.transform.posX)) {
        base.transform.posX = layer.transform.posX;
    }
    if (differs(layer.transform.posY, defaults.transform.posY)) {
        base.transform.posY = layer.transform.posY;
    }
    if (differs(layer.transform.rotation, defaults.transform.rotation)) {
        base.transform.rotation = layer.transform.rotation;
    }
    if (differs(layer.transform.anchorX, defaults.transform.anchorX)) {
        base.transform.anchorX = layer.transform.anchorX;
    }
    if (differs(layer.transform.anchorY, defaults.transform.anchorY)) {
        base.transform.anchorY = layer.transform.anchorY;
    }
    if (differs(layer.transform.opacity, defaults.transform.opacity)) {
        base.transform.opacity = layer.transform.opacity;
    }
    if (differs(layer.transform.blendMode, defaults.transform.blendMode)) {
        base.transform.blendMode = layer.transform.blendMode;
    }

    base.color.brightness = std::clamp(base.color.brightness + layer.color.brightness, -100.0, 100.0);
    if (differs(layer.color.contrast, defaults.color.contrast)) {
        base.color.contrast = std::clamp(base.color.contrast * (layer.color.contrast / 100.0), 0.0, 300.0);
    }
    if (differs(layer.color.saturation, defaults.color.saturation)) {
        base.color.saturation = std::clamp(base.color.saturation * (layer.color.saturation / 100.0), 0.0, 300.0);
    }

    if (layer.blur.radius > 0.0) {
        base.blur = layer.blur;
    }

    if (differs(layer.stylize.styleName, defaults.stylize.styleName)) {
        base.stylize = layer.stylize;
    }

    if (differs(layer.audio.volume, defaults.audio.volume)) {
        base.audio.volume = std::clamp(base.audio.volume * (layer.audio.volume / 100.0), 0.0, 300.0);
    }
    if (differs(layer.audio.pan, defaults.audio.pan)) {
        base.audio.pan = std::clamp(base.audio.pan + layer.audio.pan, -100.0, 100.0);
    }

    if (layer.chromaKey.enabled) {
        base.chromaKey = layer.chromaKey;
    }
}

} // namespace

EffectController::EffectController(QObject *parent) : QObject(parent),
    m_transform(new TransformEffect(this)),
    m_color(new ColorEffect(this)),
    m_blur(new BlurEffect(this)),
    m_stylize(new StylizeEffect(this)),
    m_audio(new AudioEffect(this)),
    m_chromaKey(new ChromaKeyEffect(this))
{
    connect(m_transform, &TransformEffect::modified, this, &EffectController::onEffectModified);
    connect(m_color, &ColorEffect::modified, this, &EffectController::onEffectModified);
    connect(m_blur, &BlurEffect::modified, this, &EffectController::onEffectModified);
    connect(m_stylize, &StylizeEffect::modified, this, &EffectController::onEffectModified);
    connect(m_audio, &AudioEffect::modified, this, &EffectController::onEffectModified);
    connect(m_chromaKey, &ChromaKeyEffect::modified, this, &EffectController::onEffectModified);
}

EffectController::~EffectController()
{
}

void EffectController::setTimelineController(TimelineController* controller)
{
    if (m_timelineController == controller)
        return;

    if (m_timelineController) {
        disconnect(m_timelineController, &TimelineController::selectedClipIndicesChanged, this, &EffectController::onSelectedClipIndexChanged);
    }

    m_timelineController = controller;

    if (m_timelineController) {
        m_clipModel = qobject_cast<TimelineClipModel*>(m_timelineController->clipModel());
        connect(m_timelineController, &TimelineController::selectedClipIndicesChanged, this, &EffectController::onSelectedClipIndexChanged);
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

    int row = m_timelineController->selectedClipIndices().isEmpty() ? -1 : m_timelineController->selectedClipIndices().first().toInt();
    loadClipEffects(row);
}

void EffectController::onEffectModified()
{
    if (m_isUpdatingFromModel)
        return;

    const ClipEffects effects = currentEffects();
    if (m_timelineController && m_clipModel) {
        int row = m_timelineController->selectedClipIndices().isEmpty() ? -1 : m_timelineController->selectedClipIndices().first().toInt();
        if (row >= 0) {
            saveClipEffects(row);
        }
    }

    if (m_playbackEngine) {
        if (m_timelineController && m_clipModel) {
            refreshPlaybackEffects(m_playbackEngine->position(), -1);
        } else {
            m_playbackEngine->setClipEffects(effects);
        }
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
    m_chromaKey->loadData(effects.chromaKey);

    if (m_playbackEngine) {
        if (m_timelineController && m_clipModel) {
            refreshPlaybackEffects(m_playbackEngine->position(), -1);
        } else {
            m_playbackEngine->setClipEffects(effects);
        }
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
    effects.chromaKey = m_chromaKey->toData();
    return effects;
}

ClipEffects EffectController::presetEffects(const QString& presetName) const
{
    ClipEffects effects;

    if (presetName.contains("Blur", Qt::CaseInsensitive)) {
        effects.blur.radius = 50.0;
        effects.blur.blurType = "Gaussian";
    } else if (presetName.contains("4 Shots", Qt::CaseInsensitive)
               || presetName.contains("5-pic Squad", Qt::CaseInsensitive)) {
        effects.stylize.styleName = "Multi-Shot Grid";
        effects.stylize.intensity = 80.0;
    } else if (presetName.contains("12 Grapes", Qt::CaseInsensitive)) {
        effects.color.saturation = 150.0;
        effects.color.brightness = 10.0;
        effects.color.contrast = 120.0;
    } else if (presetName.contains("Abyss", Qt::CaseInsensitive)) {
        effects.color.brightness = -16.0;
        effects.color.contrast = 135.0;
        effects.color.saturation = 70.0;
        effects.blur.radius = 6.0;
        effects.blur.isRegionEnabled = false;
    } else if (presetName.contains("Aggressive", Qt::CaseInsensitive)
               || presetName.contains("Error", Qt::CaseInsensitive)) {
        effects.stylize.styleName = "Glitch";
        effects.stylize.intensity = 75.0;
        effects.color.contrast = 130.0;
    } else if (presetName.contains("Carbon", Qt::CaseInsensitive)) {
        effects.color.brightness = -8.0;
        effects.color.contrast = 135.0;
        effects.color.saturation = 25.0;
        effects.stylize.styleName = "Carbon";
        effects.stylize.intensity = 65.0;
    } else {
        effects.stylize.styleName = presetName;
        effects.stylize.intensity = 75.0;
    }

    return effects;
}

int EffectController::findBaseVideoRowAt(double timelineSeconds) const
{
    if (!m_timelineController || !m_clipModel || !std::isfinite(timelineSeconds)) {
        return -1;
    }

    for (int row = 0; row < m_timelineController->clipCount(); ++row) {
        const TimelineClip clip = m_clipModel->clipAt(row);
        if (clip.isEffect || !clip.hasVideo || !m_timelineController->clipContains(row, timelineSeconds)) {
            continue;
        }

        const int trackIndex = clip.trackIndex >= 100 ? clip.trackIndex - 100 : clip.trackIndex;
        if (!m_timelineController->isTrackHidden(true, trackIndex)) {
            return row;
        }
    }

    return -1;
}

ClipEffects EffectController::combinedEffectsAt(double timelineSeconds, int baseClipRow) const
{
    ClipEffects effects;
    if (!m_timelineController || !m_clipModel || !std::isfinite(timelineSeconds)) {
        return effects;
    }

    const int baseRow = baseClipRow >= 0 ? baseClipRow : findBaseVideoRowAt(timelineSeconds);
    if (baseRow >= 0 && baseRow < m_clipModel->rowCount()) {
        const TimelineClip baseClip = m_clipModel->clipAt(baseRow);
        if (!baseClip.isEffect) {
            effects = m_clipModel->clipEffectsAt(baseRow);
        }
    }

    QVector<int> activeEffectRows;
    for (int row = 0; row < m_timelineController->clipCount(); ++row) {
        const TimelineClip clip = m_clipModel->clipAt(row);
        if (!clip.isEffect || !m_timelineController->clipContains(row, timelineSeconds)) {
            continue;
        }

        const int trackIndex = clip.trackIndex >= 100 ? clip.trackIndex - 100 : clip.trackIndex;
        if (!m_timelineController->isTrackHidden(true, trackIndex)) {
            activeEffectRows.append(row);
        }
    }

    std::sort(activeEffectRows.begin(), activeEffectRows.end(), [this](int left, int right) {
        const TimelineClip leftClip = m_clipModel->clipAt(left);
        const TimelineClip rightClip = m_clipModel->clipAt(right);
        if (leftClip.trackIndex != rightClip.trackIndex) {
            return leftClip.trackIndex > rightClip.trackIndex;
        }
        return left < right;
    });

    for (int row : activeEffectRows) {
        mergeEffectLayer(effects, m_clipModel->clipEffectsAt(row));
    }

    return effects;
}

QString EffectController::normalizeLocalPath(const QString& path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        const QUrl url(trimmed);
        if (url.isLocalFile()) {
            return QDir::toNativeSeparators(url.toLocalFile());
        }
    }
    return QDir::toNativeSeparators(trimmed);
}

void EffectController::saveClipEffects(int row)
{
    if (row >= 0 && m_clipModel) {
        m_clipModel->updateClipEffects(row, currentEffects());
    }
}

int EffectController::addPresetToTimeline(const QString& presetName,
                                          const QString& thumbnailPath,
                                          double startSeconds,
                                          int trackIndex,
                                          double durationSeconds)
{
    if (!m_timelineController) {
        return -1;
    }

    const double safeStart = std::isfinite(startSeconds) && startSeconds >= 0.0
        ? startSeconds
        : m_timelineController->endTimeSeconds();
    double safeDuration = std::isfinite(durationSeconds) && durationSeconds > 0.0
        ? durationSeconds
        : 5.0;

    const int baseRow = findBaseVideoRowAt(safeStart);
    if ((!std::isfinite(durationSeconds) || durationSeconds <= 0.0) && baseRow >= 0) {
        safeDuration = std::max(0.25, m_timelineController->clipEndSeconds(baseRow) - safeStart);
    }

    const int row = m_timelineController->addEffectClip(
        presetName,
        normalizeLocalPath(thumbnailPath),
        presetEffects(presetName),
        safeStart,
        safeDuration,
        trackIndex
    );
    

    if (row >= 0) {
        loadClipEffects(row);
        refreshPlaybackEffects(safeStart, baseRow);
    }

    return row;
}

void EffectController::refreshPlaybackEffects(double timelineSeconds, int baseClipRow)
{
    if (!m_playbackEngine) {
        return;
    }

    if (!m_timelineController || !m_clipModel) {
        m_playbackEngine->setClipEffects(currentEffects());
        return;
    }

    m_playbackEngine->setClipEffects(combinedEffectsAt(timelineSeconds, baseClipRow));
}

void EffectController::resetAll()
{
    m_isUpdatingFromModel = true;
    
    m_transform->loadData(TransformEffectData());
    m_color->loadData(ColorEffectData());
    m_blur->loadData(BlurEffectData());
    m_stylize->loadData(StylizeEffectData());
    m_audio->loadData(AudioEffectData());
    m_chromaKey->loadData(ChromaKeyEffectData());
    
    m_isUpdatingFromModel = false;
    
    // Save to model
    const ClipEffects effects = currentEffects();
    if (m_timelineController && m_clipModel) {
        int row = m_timelineController->selectedClipIndices().isEmpty() ? -1 : m_timelineController->selectedClipIndices().first().toInt();
        if (row >= 0) {
            saveClipEffects(row);
        }
    }

    if (m_playbackEngine) {
        if (m_timelineController && m_clipModel) {
            refreshPlaybackEffects(m_playbackEngine->position(), -1);
        } else {
            m_playbackEngine->setClipEffects(effects);
        }
    }
}

void EffectController::applyPreset(const QString& presetName)
{
    m_isUpdatingFromModel = true;

    const ClipEffects preset = presetEffects(presetName);

    m_transform->loadData(preset.transform);
    m_color->loadData(preset.color);
    m_blur->loadData(preset.blur);
    m_stylize->loadData(preset.stylize);
    m_audio->loadData(preset.audio);
    m_chromaKey->loadData(preset.chromaKey);

    m_isUpdatingFromModel = false;
    
    // Save to model
    const ClipEffects effects = currentEffects();
    if (m_timelineController && m_clipModel) {
        int row = m_timelineController->selectedClipIndices().isEmpty() ? -1 : m_timelineController->selectedClipIndices().first().toInt();
        if (row >= 0) {
            saveClipEffects(row);
        }
    }

    if (m_playbackEngine) {
        if (m_timelineController && m_clipModel) {
            refreshPlaybackEffects(m_playbackEngine->position(), -1);
        } else {
            m_playbackEngine->setClipEffects(effects);
        }
    }
}
