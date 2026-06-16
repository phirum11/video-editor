#include "SrtParser.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

double SrtParser::parseTimestamp(const QString& ts)
{
    // Format: "HH:MM:SS,mmm"  e.g. "00:01:27,866"
    static const QRegularExpression re(
        QStringLiteral(R"((\d{2}):(\d{2}):(\d{2})[,.](\d{3}))"));

    const QRegularExpressionMatch match = re.match(ts.trimmed());
    if (!match.hasMatch()) {
        return 0.0;
    }

    const int hours   = match.captured(1).toInt();
    const int minutes = match.captured(2).toInt();
    const int seconds = match.captured(3).toInt();
    const int millis  = match.captured(4).toInt();

    return hours * 3600.0 + minutes * 60.0 + seconds + millis / 1000.0;
}

QVector<SubtitleEntry> SrtParser::parse(const QString& filePath)
{
    QVector<SubtitleEntry> entries;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return entries;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // SRT format:
    // 1
    // 00:00:01,000 --> 00:00:03,500
    // Subtitle text line 1
    // Subtitle text line 2
    // 2
    // ...

    static const QRegularExpression timestampLine(
        QStringLiteral(R"((\d{2}:\d{2}:\d{2}[,.]\d{3})\s*-->\s*(\d{2}:\d{2}:\d{2}[,.]\d{3}))"));
    static const QRegularExpression indexLine(QStringLiteral(R"(^\d+$)"));

    enum class State { ExpectIndex, ExpectTimestamp, ExpectText };
    State state = State::ExpectIndex;

    SubtitleEntry current;

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        // Handle BOM on first line
        if (entries.isEmpty() && current.index == 0 && line.startsWith(QChar(0xFEFF))) {
            line = line.mid(1);
        }

        switch (state) {
        case State::ExpectIndex: {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) {
                continue; // skip blank lines between entries
            }
            if (indexLine.match(trimmed).hasMatch()) {
                current = SubtitleEntry{};
                current.index = trimmed.toInt();
                state = State::ExpectTimestamp;
            }
            break;
        }
        case State::ExpectTimestamp: {
            const QRegularExpressionMatch match = timestampLine.match(line);
            if (match.hasMatch()) {
                current.startSeconds = parseTimestamp(match.captured(1));
                current.endSeconds   = parseTimestamp(match.captured(2));
                state = State::ExpectText;
            }
            break;
        }
        case State::ExpectText: {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) {
                // End of this subtitle entry
                if (!current.text.isEmpty()) {
                    entries.append(current);
                    state = State::ExpectIndex;
                }
                // If text is empty, it's an extra blank line before text, just ignore it.
            } else {
                if (!current.text.isEmpty()) {
                    current.text += QChar(' ');
                }
                // Strip basic HTML tags like <i>, <b>, <u>, etc.
                QString clean = trimmed;
                clean.remove(QRegularExpression(QStringLiteral(R"(</?[a-zA-Z][^>]*>)")));
                current.text += clean;
            }
            break;
        }
        }
    }

    // Don't forget last entry if file doesn't end with blank line
    if (state == State::ExpectText && !current.text.isEmpty()) {
        entries.append(current);
    }

    return entries;
}
