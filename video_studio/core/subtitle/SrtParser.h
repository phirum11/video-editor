#pragma once

#include <QString>
#include <QVector>

struct SubtitleEntry {
    int index = 0;
    double startSeconds = 0.0;
    double endSeconds = 0.0;
    QString text;
};

class SrtParser {
public:
    static QVector<SubtitleEntry> parse(const QString& filePath);

private:
    static double parseTimestamp(const QString& ts);
};
