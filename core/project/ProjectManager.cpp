#include "ProjectManager.h"
#include "ui/media_pool/MediaPoolController.h"
#include "ui/timeline_view/controllers/TimelineController.h"
#include "core/subtitle/SubtitleController.h"
#include "core/timeline/models/TimelineClipModel.h"
#include "core/effects/models/EffectData.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent)
{
}

QString ProjectManager::normalizePath(const QString& path) const
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

static QJsonObject serializeEffects(const ClipEffects& effects) {
    QJsonObject obj;

    QJsonObject transformObj;
    transformObj["scale"] = effects.transform.scale;
    transformObj["posX"] = effects.transform.posX;
    transformObj["posY"] = effects.transform.posY;
    transformObj["rotation"] = effects.transform.rotation;
    transformObj["anchorX"] = effects.transform.anchorX;
    transformObj["anchorY"] = effects.transform.anchorY;
    transformObj["opacity"] = effects.transform.opacity;
    transformObj["blendMode"] = effects.transform.blendMode;
    obj["transform"] = transformObj;

    QJsonObject colorObj;
    colorObj["brightness"] = effects.color.brightness;
    colorObj["contrast"] = effects.color.contrast;
    colorObj["saturation"] = effects.color.saturation;
    obj["color"] = colorObj;

    QJsonObject blurObj;
    blurObj["radius"] = effects.blur.radius;
    blurObj["blurType"] = effects.blur.blurType;
    blurObj["isRegionEnabled"] = effects.blur.isRegionEnabled;
    blurObj["regionX"] = effects.blur.regionX;
    blurObj["regionY"] = effects.blur.regionY;
    blurObj["regionWidth"] = effects.blur.regionWidth;
    blurObj["regionHeight"] = effects.blur.regionHeight;
    obj["blur"] = blurObj;

    QJsonObject stylizeObj;
    stylizeObj["styleName"] = effects.stylize.styleName;
    stylizeObj["intensity"] = effects.stylize.intensity;
    obj["stylize"] = stylizeObj;

    QJsonObject audioObj;
    audioObj["volume"] = effects.audio.volume;
    audioObj["pan"] = effects.audio.pan;
    obj["audio"] = audioObj;

    QJsonObject chromaKeyObj;
    chromaKeyObj["enabled"] = effects.chromaKey.enabled;
    chromaKeyObj["color"] = effects.chromaKey.color;
    chromaKeyObj["variance"] = effects.chromaKey.variance;
    chromaKeyObj["softness"] = effects.chromaKey.softness;
    chromaKeyObj["spillSuppress"] = effects.chromaKey.spillSuppress;
    obj["chromaKey"] = chromaKeyObj;

    return obj;
}

static ClipEffects deserializeEffects(const QJsonObject& obj) {
    ClipEffects effects;

    if (obj.contains("transform")) {
        QJsonObject tObj = obj["transform"].toObject();
        effects.transform.scale = tObj.value("scale").toDouble(100.0);
        effects.transform.posX = tObj.value("posX").toDouble(960.0);
        effects.transform.posY = tObj.value("posY").toDouble(540.0);
        effects.transform.rotation = tObj.value("rotation").toDouble(0.0);
        effects.transform.anchorX = tObj.value("anchorX").toDouble(960.0);
        effects.transform.anchorY = tObj.value("anchorY").toDouble(540.0);
        effects.transform.opacity = tObj.value("opacity").toDouble(100.0);
        effects.transform.blendMode = tObj.value("blendMode").toString("Normal");
    }

    if (obj.contains("color")) {
        QJsonObject cObj = obj["color"].toObject();
        effects.color.brightness = cObj.value("brightness").toDouble(0.0);
        effects.color.contrast = cObj.value("contrast").toDouble(100.0);
        effects.color.saturation = cObj.value("saturation").toDouble(100.0);
    }

    if (obj.contains("blur")) {
        QJsonObject bObj = obj["blur"].toObject();
        effects.blur.radius = bObj.value("radius").toDouble(0.0);
        effects.blur.blurType = bObj.value("blurType").toString("Gaussian");
        effects.blur.isRegionEnabled = bObj.value("isRegionEnabled").toBool(true);
        effects.blur.regionX = bObj.value("regionX").toDouble(0.25);
        effects.blur.regionY = bObj.value("regionY").toDouble(0.25);
        effects.blur.regionWidth = bObj.value("regionWidth").toDouble(0.5);
        effects.blur.regionHeight = bObj.value("regionHeight").toDouble(0.5);
    }

    if (obj.contains("stylize")) {
        QJsonObject sObj = obj["stylize"].toObject();
        effects.stylize.styleName = sObj.value("styleName").toString("None");
        effects.stylize.intensity = sObj.value("intensity").toDouble(100.0);
    }

    if (obj.contains("audio")) {
        QJsonObject aObj = obj["audio"].toObject();
        effects.audio.volume = aObj.value("volume").toDouble(100.0);
        effects.audio.pan = aObj.value("pan").toDouble(0.0);
    }

    if (obj.contains("chromaKey")) {
        QJsonObject ckObj = obj["chromaKey"].toObject();
        effects.chromaKey.enabled = ckObj.value("enabled").toBool(false);
        effects.chromaKey.color = ckObj.value("color").toString("#00FF00");
        effects.chromaKey.variance = ckObj.value("variance").toDouble(0.25);
        effects.chromaKey.softness = ckObj.value("softness").toDouble(0.1);
        effects.chromaKey.spillSuppress = ckObj.value("spillSuppress").toBool(true);
    }

    return effects;
}

bool ProjectManager::saveProject(const QString& filePath, 
                                 MediaPoolController* mediaPool, 
                                 TimelineController* timeline, 
                                 SubtitleController* subtitles)
{
    if (!mediaPool || !timeline || !subtitles) return false;

    QString normalizedPath = normalizePath(filePath);

    QJsonObject projectObj;
    projectObj["version"] = "1.0.0";

    // 1. Save Media Pool
    QJsonArray mediaArray;
    QAbstractItemModel* mediaModel = mediaPool->mediaModel();
    if (mediaModel) {
        for (int i = 0; i < mediaModel->rowCount(); ++i) {
            QModelIndex index = mediaModel->index(i, 0);
            QJsonObject mediaItem;
            mediaItem["name"] = mediaModel->data(index, Qt::UserRole + 1).toString();
            mediaItem["filePath"] = mediaModel->data(index, Qt::UserRole + 2).toString();
            mediaArray.append(mediaItem);
        }
    }
    projectObj["mediaPool"] = mediaArray;

    // 2. Save Timeline
    QJsonArray timelineArray;
    TimelineClipModel* clipModel = qobject_cast<TimelineClipModel*>(timeline->clipModel());
    if (clipModel) {
        for (int i = 0; i < clipModel->rowCount(); ++i) {
            QJsonObject clipObj = QJsonObject::fromVariantMap(clipModel->clipMapAt(i));
            clipObj["effects"] = serializeEffects(clipModel->clipEffectsAt(i));
            timelineArray.append(clipObj);
        }
    }
    projectObj["timeline"] = timelineArray;

    // 3. Save Subtitle Styles
    QJsonObject subtitleObj;
    subtitleObj["fontFamily"] = subtitles->font().family();
    subtitleObj["fontSize"] = subtitles->font().pointSize() > 0 ? subtitles->font().pointSize() : subtitles->font().pixelSize();
    subtitleObj["fontWeight"] = subtitles->font().weight();
    subtitleObj["color"] = subtitles->color().name(QColor::HexArgb);
    subtitleObj["verticalPosition"] = subtitles->verticalPosition();
    projectObj["subtitleStyle"] = subtitleObj;

    QJsonDocument doc(projectObj);
    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool ProjectManager::loadProject(const QString& filePath, 
                                 MediaPoolController* mediaPool, 
                                 TimelineController* timeline, 
                                 SubtitleController* subtitles)
{
    if (!mediaPool || !timeline || !subtitles) return false;

    QString normalizedPath = normalizePath(filePath);

    QFile file(normalizedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject projectObj = doc.object();

    // Clear current state
    QAbstractItemModel* mediaModel = mediaPool->mediaModel();
    if (mediaModel) {
        while (mediaModel->rowCount() > 0) {
            mediaPool->removeMediaAt(0);
        }
    }

    TimelineClipModel* clipModel = qobject_cast<TimelineClipModel*>(timeline->clipModel());
    if (clipModel) {
        clipModel->clear();
    }

    // 1. Load Media Pool
    QJsonArray mediaArray = projectObj["mediaPool"].toArray();
    for (int i = 0; i < mediaArray.size(); ++i) {
        QJsonObject mediaItem = mediaArray[i].toObject();
        QString path = mediaItem["filePath"].toString();
        QString name = mediaItem["name"].toString();
        
        mediaPool->importMediaPath(path);
        
        // Find the newly added item to rename it if needed
        if (mediaModel && !name.isEmpty()) {
            for (int r = 0; r < mediaModel->rowCount(); ++r) {
                QModelIndex idx = mediaModel->index(r, 0);
                if (mediaModel->data(idx, Qt::UserRole + 2).toString() == path) {
                    mediaPool->renameMediaAt(r, name);
                    break;
                }
            }
        }
    }

    // 2. Load Timeline
    QJsonArray timelineArray = projectObj["timeline"].toArray();
    if (clipModel) {
        for (int i = 0; i < timelineArray.size(); ++i) {
            QJsonObject clipObj = timelineArray[i].toObject();
            
            TimelineClip clip;
            clip.clipName = clipObj["clipName"].toString();
            clip.filePath = clipObj["filePath"].toString();
            clip.linkGroupId = clipObj["linkGroupId"].toString();
            clip.groupId = clipObj["groupId"].toString();
            clip.originalFilePath = clipObj["originalFilePath"].toString();
            clip.vocalIsolationType = clipObj["vocalIsolationType"].toInt(0);
            clip.isolationProgress = clipObj["isolationProgress"].toInt(-1);
            clip.startSeconds = clipObj["startSeconds"].toDouble(0.0);
            clip.durationSeconds = clipObj["durationSeconds"].toDouble(0.0);
            clip.sourceInPoint = clipObj["sourceInPoint"].toDouble(0.0);
            clip.sourceDuration = clipObj["sourceDuration"].toDouble(0.0);
            clip.trackIndex = clipObj["trackIndex"].toInt(2);
            clip.hasVideo = clipObj["hasVideo"].toBool(true);
            clip.hasAudio = clipObj["hasAudio"].toBool(true);
            clip.isEffect = clipObj["isEffect"].toBool(false);
            clip.isMuted = clipObj["isMuted"].toBool(false);
            
            if (clipObj.contains("effects")) {
                clip.effects = deserializeEffects(clipObj["effects"].toObject());
            }
            
            clipModel->addClip(clip);
        }
    }

    // 3. Load Subtitle Styles
    if (projectObj.contains("subtitleStyle")) {
        QJsonObject subtitleObj = projectObj["subtitleStyle"].toObject();
        
        QFont font(subtitleObj["fontFamily"].toString("Khmer UI"));
        if (subtitleObj["fontSize"].toInt() > 0) {
            font.setPointSize(subtitleObj["fontSize"].toInt());
        }
        font.setWeight(static_cast<QFont::Weight>(subtitleObj["fontWeight"].toInt(QFont::Bold)));
        subtitles->setFont(font);
        
        subtitles->setColor(QColor(subtitleObj["color"].toString("#ffffff")));
        subtitles->setVerticalPosition(subtitleObj["verticalPosition"].toDouble(-1.0));
    }

    return true;
}
