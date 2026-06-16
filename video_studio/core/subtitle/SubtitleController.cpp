#include "SubtitleController.h"
#include "SrtParser.h"
#include "ui/timeline_view/controllers/TimelineController.h"
#include <QDebug>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

SubtitleController::SubtitleController(QObject* parent)
    : QObject(parent)
{
    m_font = QFont("Khmer UI", 48, QFont::Bold);
    m_color = QColor(Qt::white);
    m_verticalPosition = -1.0;
}

QFont SubtitleController::font() const
{
    return m_font;
}

void SubtitleController::setFont(const QFont& font)
{
    if (m_font != font) {
        m_font = font;
        emit fontChanged();
    }
}

QColor SubtitleController::color() const
{
    return m_color;
}

void SubtitleController::setColor(const QColor& color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
    }
}

double SubtitleController::verticalPosition() const
{
    return m_verticalPosition;
}

void SubtitleController::setVerticalPosition(double pos)
{
    if (qFuzzyCompare(m_verticalPosition, pos))
        return;
    m_verticalPosition = pos;
    emit verticalPositionChanged();
}

int SubtitleController::addSrtToTimeline(const QString& filePath, TimelineController* timeline)
{
    return addSrtToTimelineAt(filePath, timeline, 0.0, 1);
}

int SubtitleController::addSrtToTimelineAt(const QString& filePath,
                                           TimelineController* timeline,
                                           double startOffsetSeconds,
                                           int trackIndex)
{
    if (!timeline) {
        qWarning() << "SubtitleController: no TimelineController provided";
        return 0;
    }

    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        qWarning() << "SubtitleController: SRT file not found:" << filePath;
        return 0;
    }

    const QVector<SubtitleEntry> entries = SrtParser::parse(filePath);
    if (entries.isEmpty()) {
        qWarning() << "SubtitleController: no subtitle entries found in" << filePath;
        return 0;
    }

    qDebug() << "SubtitleController: parsed" << entries.size() << "subtitle entries from" << filePath;

    int addedCount = 0;
    const int subtitleTrack = std::clamp(trackIndex, 0, 5);
    const double safeOffset = std::isfinite(startOffsetSeconds) && startOffsetSeconds >= 0.0
        ? startOffsetSeconds
        : 0.0;

    for (const SubtitleEntry& entry : entries) {
        const double duration = entry.endSeconds - entry.startSeconds;
        if (duration <= 0.01) {
            continue; // Skip entries with invalid duration
        }

        const int row = timeline->addSubtitleClip(
            entry.text,
            filePath,
            safeOffset + entry.startSeconds,
            duration,
            subtitleTrack
        );

        if (row >= 0) {
            ++addedCount;
        }
    }

    qDebug() << "SubtitleController: added" << addedCount << "subtitle clips to timeline";
    return addedCount;
}

QString SubtitleController::getSubtitleAtTime(TimelineController* timeline, double timeSeconds) const
{
    if (!timeline) return QString();

    QAbstractListModel* model = timeline->clipModel();
    if (!model) return QString();

    int count = timeline->clipCount();
    for (int i = 0; i < count; ++i) {
        QVariantMap clipData = timeline->clipAt(i);
        bool hasVideo = clipData.value("hasVideo").toBool();
        bool hasAudio = clipData.value("hasAudio").toBool();
        // Subtitles have neither video nor audio in our current model
        if (!hasVideo && !hasAudio) {
            double start = clipData.value("startSeconds").toDouble();
            double duration = clipData.value("durationSeconds").toDouble();
            
            if (timeSeconds >= start && timeSeconds < start + duration) {
                return clipData.value("clipName").toString();
            }
        }
    }
    
    return QString();
}

