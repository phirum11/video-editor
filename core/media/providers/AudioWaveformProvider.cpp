#include "core/media/providers/AudioWaveformProvider.h"
#include <QPainter>
#include <QPainterPath>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <vector>
#include <cmath>
#include <algorithm>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

AudioWaveformProvider::AudioWaveformProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    // Max cost is usually in KB/MB, we'll allow 50 MB of cache
    m_cache.setMaxCost(50 * 1024 * 1024);
}

QImage AudioWaveformProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    // ID format: filePath?width=xxx&height=yyy
    QString decodedId = QUrl::fromPercentEncoding(id.toUtf8());
    QString filePath = decodedId;
    QSize targetSize = requestedSize.isValid() ? requestedSize : QSize(800, 100);

    // Naive query parsing
    int queryIndex = decodedId.indexOf('?');
    if (queryIndex != -1) {
        filePath = decodedId.left(queryIndex);
        
        QString query = decodedId.mid(queryIndex + 1);
        QStringList parts = query.split('&');
        int w = targetSize.width();
        int h = targetSize.height();
        for (const QString& part : parts) {
            if (part.startsWith("width=")) {
                w = part.mid(6).toInt();
            } else if (part.startsWith("height=")) {
                h = part.mid(7).toInt();
            }
        }
        targetSize = QSize(std::max(1, w), std::max(1, h));
    }

    if (size) {
        *size = targetSize;
    }

    QString cacheKey = filePath + QString("_%1x%2").arg(targetSize.width()).arg(targetSize.height());
    
    m_mutex.lock();
    if (QImage* cachedImg = m_cache.object(cacheKey)) {
        QImage result = *cachedImg;
        m_mutex.unlock();
        return result;
    }
    m_mutex.unlock();

    QImage img = generateWaveform(filePath, targetSize);
    
    m_mutex.lock();
    m_cache.insert(cacheKey, new QImage(img), img.sizeInBytes());
    m_mutex.unlock();

    return img;
}

QImage AudioWaveformProvider::generateWaveform(const QString& filePath, const QSize& targetSize)
{
    QImage img(targetSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    AVFormatContext* formatContext = nullptr;
    const QByteArray pathBytes = QDir::toNativeSeparators(filePath).toUtf8();
    
    if (avformat_open_input(&formatContext, pathBytes.constData(), nullptr, nullptr) < 0) {
        return img;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        avformat_close_input(&formatContext);
        return img;
    }

    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1) {
        avformat_close_input(&formatContext);
        return img;
    }

    AVStream* audioStream = formatContext->streams[audioStreamIndex];
    const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, audioStream->codecpar);
    
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return img;
    }

    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, 1); // mixdown to mono
    
    SwrContext* swr = nullptr;
    swr_alloc_set_opts2(&swr,
                        &outLayout, AV_SAMPLE_FMT_FLT, codecContext->sample_rate,
                        &codecContext->ch_layout, codecContext->sample_fmt, codecContext->sample_rate,
                        0, nullptr);
    swr_init(swr);

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    int barCount = targetSize.width() / 2; // e.g. 1px line, 1px gap
    if (barCount <= 0) barCount = 1;
    
    // We'll gather all peaks, then draw
    std::vector<float> peaks;
    
    // To avoid decoding massive files fully if it's too slow, we can sub-sample
    // But for accuracy we decode it all and maintain a running max
    qint64 totalSamples = 0;
    if (audioStream->duration != AV_NOPTS_VALUE) {
        totalSamples = av_rescale_q(audioStream->duration, audioStream->time_base, {1, codecContext->sample_rate});
    } else {
        totalSamples = (formatContext->duration * codecContext->sample_rate) / AV_TIME_BASE;
    }
    
    if (totalSamples <= 0) totalSamples = codecContext->sample_rate * 60; // fallback 60s
    
    qint64 samplesPerBar = totalSamples / barCount;
    if (samplesPerBar <= 0) samplesPerBar = 1;

    qint64 currentSampleCount = 0;
    float currentPeak = 0.0f;

    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == audioStreamIndex) {
            if (avcodec_send_packet(codecContext, packet) == 0) {
                while (avcodec_receive_frame(codecContext, frame) == 0) {
                    // resample to Mono Float
                    uint8_t* outputBuffer = nullptr;
                    int outSamples = swr_get_out_samples(swr, frame->nb_samples);
                    av_samples_alloc(&outputBuffer, nullptr, 1, outSamples, AV_SAMPLE_FMT_FLT, 0);
                    
                    int convertedSamples = swr_convert(swr, &outputBuffer, outSamples, (const uint8_t**)frame->data, frame->nb_samples);
                    
                    float* floatData = reinterpret_cast<float*>(outputBuffer);
                    for (int i = 0; i < convertedSamples; i++) {
                        float val = std::abs(floatData[i]);
                        if (val > currentPeak) {
                            currentPeak = val;
                        }
                        
                        currentSampleCount++;
                        if (currentSampleCount >= samplesPerBar) {
                            peaks.push_back(currentPeak);
                            currentPeak = 0.0f;
                            currentSampleCount = 0;
                        }
                    }
                    av_freep(&outputBuffer);
                }
            }
        }
        av_packet_unref(packet);
        
        // Safety abort for huge files, e.g. stop after max bars
        if (peaks.size() > static_cast<size_t>(barCount * 1.5)) {
            break;
        }
    }
    
    if (currentSampleCount > 0 && peaks.size() < static_cast<size_t>(barCount)) {
        peaks.push_back(currentPeak);
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swr);
    av_channel_layout_uninit(&outLayout);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);

    // Normalize peaks
    float maxPeak = 0.001f;
    for (float p : peaks) {
        if (p > maxPeak) maxPeak = p;
    }
    
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Teal color like CapCut
    QColor tealColor("#009090");
    QPen pen(tealColor);
    pen.setWidth(1);
    painter.setPen(pen);
    
    int h = targetSize.height();
    int bottomY = h - 1;
    
    // Draw bottom line
    painter.setPen(QColor(tealColor).darker(150));
    painter.drawLine(0, bottomY, targetSize.width(), bottomY);
    
    painter.setPen(pen);
    
    // Make sure we have exactly barCount peaks to map to width
    for (int i = 0; i < targetSize.width(); i += 2) {
        int peakIndex = (i / 2) * peaks.size() / std::max(1, barCount);
        float p = 0.0f;
        if (peakIndex >= 0 && peakIndex < static_cast<int>(peaks.size())) {
            p = peaks[peakIndex];
        }
        
        float normalized = p / maxPeak;
        // height of bar is proportional
        int barHeight = static_cast<int>(normalized * (h - 2));
        if (barHeight < 2 && p > 0.001f) barHeight = 2; // min height
        
        int y1 = bottomY;
        int y2 = bottomY - barHeight;
        
        painter.drawLine(i, y1, i, y2);
    }
    
    return img;
}
