#include "AdvancedSubtitleRenderer.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QPainter>
#include <QFontDatabase>
#include <QDebug>
#include <cmath>
#include <algorithm>

QString AdvancedSubtitleRenderer::resolveKhmerFont(const QString& requested)
{
    QFontDatabase db;
    QStringList allFamilies = db.families();

    auto familyMatch = [&allFamilies](const QString& name) -> QString {
        for (const QString& family : allFamilies) {
            if (family.compare(name, Qt::CaseInsensitive) == 0) {
                return family;
            }
        }
        return {};
    };

    auto supportsKhmer = [&db](const QString& family) {
        return db.writingSystems(family).contains(QFontDatabase::Khmer);
    };
    
    // 1. Keep the requested font only when it advertises Khmer shaping/glyph coverage.
    const QString requestedFamily = familyMatch(requested);
    if (!requestedFamily.isEmpty() && supportsKhmer(requestedFamily)) {
        return requestedFamily;
    }
    
    // 2. Deep query of fonts that strictly advertise Khmer OpenType support
    QStringList khmerFonts = db.families(QFontDatabase::Khmer);
    if (!khmerFonts.isEmpty()) {
        QStringList preferred = {
            "Khmer UI",
            "Khmer OS System",
            "Khmer OS",
            "DaunPenh",
            "MoolBoran",
            "Nokora",
            "Hanuman",
            "Leelawadee UI"
        };
        for (const QString& pref : preferred) {
            if (khmerFonts.contains(pref, Qt::CaseInsensitive)) {
                return pref;
            }
        }
        return khmerFonts.first();
    }
    
    // 3. Fallbacks for environments where QFontDatabase fails to categorize them properly
    QStringList fallbacks = {"Khmer UI", "Khmer OS System", "Khmer OS", "DaunPenh", "MoolBoran", "Leelawadee UI"};
    for (const QString& f : fallbacks) {
        if (allFamilies.contains(f, Qt::CaseInsensitive)) {
            return f;
        }
    }
    
    // 4. Absolute fallback
    return QStringLiteral("Arial");
}

QString AdvancedSubtitleRenderer::generate(const QList<SubtitleItem>& subtitles,
                                         const QString& requestedFont,
                                         int fontSize,
                                         int width,
                                         int height,
                                         const QString& outputDir,
                                         int chunkIndex,
                                         double chunkStart,
                                         double chunkEnd,
                                         QStringList& generatedFiles,
                                         double verticalPosition)
{
    QDir dir(outputDir);
    
    QString emptyImgName = QStringLiteral("chunk_%1_empty.png").arg(chunkIndex);
    QString emptyImgPath = dir.absoluteFilePath(emptyImgName);
    QImage emptyImg(width, height, QImage::Format_ARGB32_Premultiplied);
    emptyImg.fill(Qt::transparent);
    emptyImg.save(emptyImgPath);
    generatedFiles.append(emptyImgPath);

    QString concatFileName = QStringLiteral("chunk_%1_subs_concat.txt").arg(chunkIndex);
    QString concatFilePath = dir.absoluteFilePath(concatFileName);
    generatedFiles.append(concatFilePath);

    QFile concatFile(concatFilePath);
    if (!concatFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream out(&concatFile);
    out.setEncoding(QStringConverter::Utf8);

    double currentTime = chunkStart;
    
    // Ensure we use a deeply supported OpenType font for Khmer
    QString resolvedFontName = resolveKhmerFont(requestedFont);
    
    QFont font(resolvedFontName);
    int pixelSize = static_cast<int>((height / 540.0) * fontSize);
    font.setPixelSize(pixelSize);
    font.setBold(true);
    // Force best possible rendering strategies for complex ligatures
    font.setStyleStrategy(QFont::PreferAntialias);

    int subIndex = 0;
    QString lastImgName = emptyImgName;

    for (const SubtitleItem& clip : subtitles) {
        double start = std::max(chunkStart, clip.startSeconds);
        double end = std::min(chunkEnd, clip.startSeconds + clip.durationSeconds);
        double dur = end - start;
        
        if (dur <= 0.001) continue;

        if (start > currentTime) {
            double gap = start - currentTime;
            out << "file '" << emptyImgName << "'\n";
            // Ensure dot is used as decimal separator
            out << "duration " << QString::number(gap, 'f', 4) << "\n";
            currentTime = start;
            lastImgName = emptyImgName;
        }

        QString subImgName = QStringLiteral("chunk_%1_sub_%2.png").arg(chunkIndex).arg(subIndex++);
        QString subImgPath = dir.absoluteFilePath(subImgName);
        
        QImage img(width, height, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        
        {
            // IMPORTANT: Scoped QPainter to strictly enforce RAII flush of the raster buffer.
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::TextAntialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.setFont(font);
            const int horizontalMargin = std::max(32, width / 30);
            const int maxTextWidth = std::max(1, width - horizontalMargin * 2);
            const int textFlags = Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap;
            const QRect measureRect(horizontalMargin, 0, maxTextWidth, height);
            const QRect textBounds = p.boundingRect(measureRect, textFlags, clip.text);
            const int textHeight = std::max(textBounds.height(), pixelSize + 8);
            int startY = 0;
            if (verticalPosition >= 0.0 && verticalPosition <= 1.0) {
                startY = std::max(0, std::min(height - textHeight, static_cast<int>(height * verticalPosition)));
            } else {
                const int bottomMargin = std::max(24, static_cast<int>((height / 1080.0) * 80.0));
                startY = std::max(0, height - textHeight - bottomMargin);
            }
            const QRect targetRect(horizontalMargin, startY, maxTextWidth, textHeight);            const int strokeWidth = std::max(2, pixelSize / 12);
            // Optimize iteration step for large stroke widths to prevent UI freezing
            const int step = std::max(1, strokeWidth / 4);
            p.setPen(Qt::black);
            for (int dx = -strokeWidth; dx <= strokeWidth; dx += step) {
                for (int dy = -strokeWidth; dy <= strokeWidth; dy += step) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }
                    if (dx * dx + dy * dy > strokeWidth * strokeWidth) {
                        continue;
                    }
                    p.drawText(targetRect.translated(dx, dy), textFlags, clip.text);
                }
            }

            p.setPen(Qt::white);
            p.drawText(targetRect, textFlags, clip.text);
        } // Raster buffer guaranteed flushed here!
        
        img.save(subImgPath);
        generatedFiles.append(subImgPath);
        
        out << "file '" << subImgName << "'\n";
        out << "duration " << QString::number(dur, 'f', 4) << "\n";
        currentTime = end;
        lastImgName = subImgName;
    }

    if (currentTime < chunkEnd) {
        double gap = chunkEnd - currentTime;
        out << "file '" << emptyImgName << "'\n";
        out << "duration " << QString::number(gap, 'f', 4) << "\n";
        lastImgName = emptyImgName;
    }

    out << "file '" << lastImgName << "'\n";

    concatFile.close();
    return concatFilePath;
}
