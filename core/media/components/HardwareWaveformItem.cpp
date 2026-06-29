#include "core/media/components/HardwareWaveformItem.h"
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include <QMutex>
#include <QHash>
#include <QPointer>
#include <cmath>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

static QMutex s_cacheMutex;
static QHash<QString, QVector<float>> s_peakCache;

HardwareWaveformItem::HardwareWaveformItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    connect(this, &HardwareWaveformItem::peaksReady, this, &HardwareWaveformItem::onPeaksReady, Qt::QueuedConnection);
}

HardwareWaveformItem::~HardwareWaveformItem() = default;

QString HardwareWaveformItem::filePath() const { return m_filePath; }
void HardwareWaveformItem::setFilePath(const QString &filePath) {
    if (m_filePath == filePath) return;
    m_filePath = filePath;
    emit filePathChanged();
    fetchPeaksAsync();
}

double HardwareWaveformItem::sourceDuration() const { return m_sourceDuration; }
void HardwareWaveformItem::setSourceDuration(double sourceDuration) {
    if (qFuzzyCompare(m_sourceDuration, sourceDuration)) return;
    m_sourceDuration = sourceDuration;
    emit sourceDurationChanged();
    update();
}

double HardwareWaveformItem::pixelsPerSecond() const { return m_pixelsPerSecond; }
void HardwareWaveformItem::setPixelsPerSecond(double pixelsPerSecond) {
    if (qFuzzyCompare(m_pixelsPerSecond, pixelsPerSecond)) return;
    m_pixelsPerSecond = pixelsPerSecond;
    emit pixelsPerSecondChanged();
    update();
}

QColor HardwareWaveformItem::color() const { return m_color; }
void HardwareWaveformItem::setColor(const QColor &color) {
    if (m_color == color) return;
    m_color = color;
    emit colorChanged();
    update();
}

void HardwareWaveformItem::onPeaksReady(const QVector<float> &peaks) {
    m_peaks = peaks;
    m_loading = false;
    update();
}

void HardwareWaveformItem::fetchPeaksAsync()
{
    if (m_filePath.isEmpty()) {
        m_peaks.clear();
        update();
        return;
    }

    {
        QMutexLocker locker(&s_cacheMutex);
        if (s_peakCache.contains(m_filePath)) {
            m_peaks = s_peakCache.value(m_filePath);
            update();
            return;
        }
    }

    if (m_loading) return;
    m_loading = true;

    QString path = m_filePath;
    QPointer<HardwareWaveformItem> weakThis(this);
    
    (void)QtConcurrent::run([weakThis, path]() {
        QVector<float> peaks;
        AVFormatContext* formatContext = nullptr;
        QByteArray pathBytes = QDir::toNativeSeparators(path).toUtf8();
        
        if (avformat_open_input(&formatContext, pathBytes.constData(), nullptr, nullptr) >= 0) {
            if (avformat_find_stream_info(formatContext, nullptr) >= 0) {
                int audioStreamIndex = -1;
                for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
                    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                        audioStreamIndex = i;
                        break;
                    }
                }
                
                if (audioStreamIndex != -1) {
                    AVStream* audioStream = formatContext->streams[audioStreamIndex];
                    const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
                    if (codec) {
                        AVCodecContext* codecContext = avcodec_alloc_context3(codec);
                        avcodec_parameters_to_context(codecContext, audioStream->codecpar);
                        
                        if (avcodec_open2(codecContext, codec, nullptr) >= 0) {
                            AVChannelLayout outLayout;
                            av_channel_layout_default(&outLayout, 1);
                            SwrContext* swr = nullptr;
                            swr_alloc_set_opts2(&swr, &outLayout, AV_SAMPLE_FMT_FLT, codecContext->sample_rate,
                                                &codecContext->ch_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr);
                            swr_init(swr);

                            AVPacket* packet = av_packet_alloc();
                            AVFrame* frame = av_frame_alloc();

                            int targetPeaks = 20000;
                            qint64 totalSamples = 0;
                            if (audioStream->duration != AV_NOPTS_VALUE) {
                                totalSamples = av_rescale_q(audioStream->duration, audioStream->time_base, {1, codecContext->sample_rate});
                            } else {
                                totalSamples = (formatContext->duration * codecContext->sample_rate) / AV_TIME_BASE;
                            }
                            if (totalSamples <= 0) totalSamples = codecContext->sample_rate * 60;
                            
                            qint64 samplesPerPeak = totalSamples / targetPeaks;
                            if (samplesPerPeak <= 0) samplesPerPeak = 1;

                            qint64 currentSampleCount = 0;
                            float currentPeak = 0.0f;

                            while (av_read_frame(formatContext, packet) >= 0) {
                                if (packet->stream_index == audioStreamIndex) {
                                    if (avcodec_send_packet(codecContext, packet) == 0) {
                                        while (avcodec_receive_frame(codecContext, frame) == 0) {
                                            uint8_t* outputBuffer = nullptr;
                                            int outSamples = swr_get_out_samples(swr, frame->nb_samples);
                                            av_samples_alloc(&outputBuffer, nullptr, 1, outSamples, AV_SAMPLE_FMT_FLT, 0);
                                            int convertedSamples = swr_convert(swr, &outputBuffer, outSamples, (const uint8_t**)frame->data, frame->nb_samples);
                                            float* floatData = reinterpret_cast<float*>(outputBuffer);
                                            for (int i = 0; i < convertedSamples; i++) {
                                                float val = std::abs(floatData[i]);
                                                if (val > currentPeak) currentPeak = val;
                                                currentSampleCount++;
                                                if (currentSampleCount >= samplesPerPeak) {
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
                                if (peaks.size() >= targetPeaks) break;
                            }
                            if (currentSampleCount > 0 && peaks.size() < targetPeaks) {
                                peaks.push_back(currentPeak);
                            }

                            av_frame_free(&frame);
                            av_packet_free(&packet);
                            swr_free(&swr);
                            av_channel_layout_uninit(&outLayout);
                            avcodec_free_context(&codecContext);
                        } else {
                            avcodec_free_context(&codecContext);
                        }
                    }
                }
                avformat_close_input(&formatContext);
            } else {
                avformat_close_input(&formatContext);
            }
        }
        
        // Normalize peaks
        float maxPeak = 0.001f;
        for (float p : peaks) {
            if (p > maxPeak) maxPeak = p;
        }
        for (float &p : peaks) {
            p /= maxPeak;
        }

        QMutexLocker locker(&s_cacheMutex);
        s_peakCache.insert(path, peaks);
        locker.unlock();
        
        if (weakThis) {
            // Need to invoke on main thread safely, but QPointer handles that well enough when calling slots if we use QMetaObject::invokeMethod or rely on the signal connection. But signal emission requires a valid object.
            // Since it's QPointer, we check if valid, then emit. Wait, if it gets deleted right as we emit, it could still crash. 
            // QMetaObject::invokeMethod is safer for async.
            QMetaObject::invokeMethod(weakThis.data(), "onPeaksReady", Qt::QueuedConnection, Q_ARG(QVector<float>, peaks));
        }
    });
}

QSGNode *HardwareWaveformItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (m_peaks.isEmpty() || width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);
    if (!node) {
        node = new QSGGeometryNode();
        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);

        QSGFlatColorMaterial *material = new QSGFlatColorMaterial();
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
    }

    QSGFlatColorMaterial *material = static_cast<QSGFlatColorMaterial *>(node->material());
    material->setColor(m_color);

    // 1 pixel per bar stride (0.5px bar, 0.5px gap)
    int stride = 2; // 2px stride (1px bar, 1px gap)
    int numBars = qMax(1, static_cast<int>(width()) / stride);
    
    QSGGeometry *geometry = node->geometry();
    geometry->allocate(numBars * 6);
    QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();

    int h = height();
    int bottomY = h;

    for (int i = 0; i < numBars; ++i) {
        float x = i * stride;
        float w = 1.0f; // 1px width
        
        int startIndex = (i * m_peaks.size()) / numBars;
        int endIndex = ((i + 1) * m_peaks.size()) / numBars;
        startIndex = qBound(0, startIndex, static_cast<int>(m_peaks.size()) - 1);
        endIndex = qBound(startIndex, endIndex, static_cast<int>(m_peaks.size()) - 1);
        
        float p = 0.0f;
        for (int j = startIndex; j <= endIndex; ++j) {
            p = qMax(p, m_peaks[j]);
        }
        
        // Smooth boost for dynamic range
        p = std::pow(p, 0.6f); 
        
        float barHeight = qMax(1.0f, p * (h - 2.0f));
        float topY = bottomY - barHeight;
        
        vertices[i * 6 + 0].set(x, topY);
        vertices[i * 6 + 1].set(x, bottomY);
        vertices[i * 6 + 2].set(x + w, topY);
        vertices[i * 6 + 3].set(x, bottomY);
        vertices[i * 6 + 4].set(x + w, bottomY);
        vertices[i * 6 + 5].set(x + w, topY);
    }
    
    node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
    return node;
}
