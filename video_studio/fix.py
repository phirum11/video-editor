import sys

# Fix SubtitleController.h
path_h = 'core/subtitle/SubtitleController.h'
with open(path_h, 'r', encoding='utf-8') as f:
    content_h = f.read()
if '#include <QFont>' not in content_h:
    content_h = content_h.replace('#include <QtQml/qqmlregistration.h>', '#include <QtQml/qqmlregistration.h>\n#include <QFont>\n#include <QColor>\n')
    with open(path_h, 'w', encoding='utf-8') as f:
        f.write(content_h)

# Fix VideoExporter.cpp
path_cpp = 'core/export/VideoExporter.cpp'
with open(path_cpp, 'r', encoding='utf-8') as f:
    content_cpp = f.read()

correct_block = """    m_chunkFiles.append(chunkPath);

    QString chunkSubsPath;
    if (m_currentSettings.exportVideo) {
        QList<AdvancedSubtitleRenderer::SubtitleItem> subtitleClips;
        qDebug() << "[SUBTITLE-DEBUG] Total clips:" << m_currentClips.size()
                 << "chunkStart:" << chunkStart << "chunkEnd:" << chunkEnd;
        for (const ClipSpec& clip : m_currentClips) {
            if (!clip.hasVideo && !clip.hasAudio && !clip.name.isEmpty()) {
                if (clip.startSeconds < chunkEnd && clip.startSeconds + clip.durationSeconds > chunkStart) {
                    AdvancedSubtitleRenderer::SubtitleItem sub;
                    sub.text = clip.name;
                    sub.startSeconds = clip.startSeconds;
                    sub.durationSeconds = clip.durationSeconds;
                    subtitleClips.push_back(sub);
                }
            }
        }
        qDebug() << "[SUBTITLE-DEBUG] Subtitle clips found:" << subtitleClips.size();
        if (!subtitleClips.isEmpty()) {
            qDebug() << "[SUBTITLE-DEBUG] Calling generate with font:" << m_currentSettings.subtitleFont
                     << "size:" << m_currentSettings.subtitleFontSize
                     << "width:" << m_currentSettings.width << "height:" << m_currentSettings.height;
            chunkSubsPath = AdvancedSubtitleRenderer::generate(
                subtitleClips,
                m_currentSettings.subtitleFont,
                m_currentSettings.subtitleFontSize,
                m_currentSettings.width,
                m_currentSettings.height,
                outputDir.absolutePath(),
                m_currentChunkIndex,
                chunkStart,
                chunkEnd,
                m_chunkSubsFiles
            );
            qDebug() << "[SUBTITLE-DEBUG] Generated concat path:" << chunkSubsPath;
        } else {
            qDebug() << "[SUBTITLE-DEBUG] NO subtitle clips found! Skipping generation.";
        }
    }

    QStringList arguments;
    try {
        arguments = buildFfmpegArguments(m_currentClips, m_currentSettings, chunkPath, chunkStart, chunkEnd, chunkSubsPath);
        qDebug() << "[SUBTITLE-DEBUG] FFmpeg command:" << arguments.join(" ");
    } catch (const std::exception& ex) {
        cleanupChunks();
        finishExport(false, QString::fromUtf8(ex.what()));
        return;
    }

    m_lastErrorOutput.clear();"""

old_block = """    m_chunkFiles.append(chunkPath);


    m_lastErrorOutput.clear();"""

content_cpp = content_cpp.replace(old_block, correct_block)
with open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(content_cpp)

print("Fixed!")
