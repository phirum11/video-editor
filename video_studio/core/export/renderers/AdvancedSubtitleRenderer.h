#pragma once

#include <QString>
#include <QList>
#include <QStringList>

class AdvancedSubtitleRenderer {
public:
    struct SubtitleItem {
        QString text;
        double startSeconds;
        double durationSeconds;
    };

    // Generates the sequence of subtitled images and returns the path to the concat demuxer file.
    static QString generate(const QList<SubtitleItem>& subtitles,
                            const QString& fontName,
                            int fontSize,
                            int videoWidth,
                            int videoHeight,
                            const QString& outputDir,
                            int chunkIndex,
                            double chunkStart,
                            double chunkEnd,
                            QStringList& generatedFiles,
                            double verticalPosition = -1.0);

private:
    // Deeply probes QFontDatabase to guarantee OpenType support for complex scripts like Khmer.
    static QString resolveKhmerFont(const QString& requested);
};
