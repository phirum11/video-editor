#include "MediaAudioDecoder.h"
#include <QDebug>
#include <QDir>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}

namespace vocal_advance {

MediaAudioDecoder::MediaAudioDecoder() {
}

MediaAudioDecoder::~MediaAudioDecoder() {
    close();
}

bool MediaAudioDecoder::open(const QString& filePath, int outputSampleRate) {
    close();

    const QByteArray pathBytes = QDir::toNativeSeparators(filePath).toUtf8();
    
    if (avformat_open_input(&m_formatContext, pathBytes.constData(), nullptr, nullptr) < 0) {
        qWarning() << "MediaAudioDecoder: Failed to open input file" << filePath;
        return false;
    }

    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        qWarning() << "MediaAudioDecoder: Failed to find stream info";
        close();
        return false;
    }

    m_audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
            break;
        }
    }

    if (m_audioStreamIndex == -1) {
        qWarning() << "MediaAudioDecoder: No audio stream found";
        close();
        return false;
    }

    AVStream* audioStream = m_formatContext->streams[m_audioStreamIndex];
    
    if (audioStream->duration != AV_NOPTS_VALUE) {
        m_durationSec = audioStream->duration * av_q2d(audioStream->time_base);
    } else {
        m_durationSec = m_formatContext->duration / (double)AV_TIME_BASE;
    }

    const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (!codec) {
        qWarning() << "MediaAudioDecoder: Codec not found";
        close();
        return false;
    }

    m_codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(m_codecContext, audioStream->codecpar);
    
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        qWarning() << "MediaAudioDecoder: Failed to open codec";
        close();
        return false;
    }

    AVChannelLayout outLayout;
    av_channel_layout_default(&outLayout, 2); // Stereo
    
    swr_alloc_set_opts2(&m_swr,
                        &outLayout, AV_SAMPLE_FMT_FLT, outputSampleRate,
                        &m_codecContext->ch_layout, m_codecContext->sample_fmt, m_codecContext->sample_rate,
                        0, nullptr);
    if (!m_swr || swr_init(m_swr) < 0) {
        qWarning() << "MediaAudioDecoder: Failed to initialize resampler";
        av_channel_layout_uninit(&outLayout);
        close();
        return false;
    }
    av_channel_layout_uninit(&outLayout);

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();
    
    m_eof = false;
    m_flushed = false;
    m_currentSec = 0.0;

    return true;
}

bool MediaAudioDecoder::readChunk(std::vector<float>& leftChannel, std::vector<float>& rightChannel) {
    if (!m_formatContext || !m_codecContext) return false;

    // We loop until we get at least SOME samples or hit EOF
    int samplesRead = 0;
    
    while (!m_eof || !m_flushed) {
        if (!m_eof) {
            int ret = av_read_frame(m_formatContext, m_packet);
            if (ret < 0) {
                m_eof = true;
                // Enter flush mode
            } else {
                if (m_packet->stream_index == m_audioStreamIndex) {
                    avcodec_send_packet(m_codecContext, m_packet);
                }
                av_packet_unref(m_packet);
            }
        } else if (!m_flushed) {
            avcodec_send_packet(m_codecContext, nullptr); // Send flush packet
        }

        while (true) {
            int ret = avcodec_receive_frame(m_codecContext, m_frame);
            if (ret == AVERROR(EAGAIN)) {
                break; // Need more input packets
            } else if (ret == AVERROR_EOF) {
                m_flushed = true;
                break;
            } else if (ret < 0) {
                return false; // Error
            }

            if (m_frame->pts != AV_NOPTS_VALUE) {
                AVStream* st = m_formatContext->streams[m_audioStreamIndex];
                m_currentSec = m_frame->pts * av_q2d(st->time_base);
            }

            uint8_t* outputBuffer = nullptr;
            int outSamples = swr_get_out_samples(m_swr, m_frame->nb_samples);
            av_samples_alloc(&outputBuffer, nullptr, 2, outSamples, AV_SAMPLE_FMT_FLT, 0);
            
            int convertedSamples = swr_convert(m_swr, &outputBuffer, outSamples, (const uint8_t**)m_frame->data, m_frame->nb_samples);
            
            if (convertedSamples > 0) {
                float* floatData = reinterpret_cast<float*>(outputBuffer);
                size_t startIdx = leftChannel.size();
                leftChannel.resize(startIdx + convertedSamples);
                rightChannel.resize(startIdx + convertedSamples);
                
                for (int i = 0; i < convertedSamples; i++) {
                    leftChannel[startIdx + i] = floatData[i * 2];
                    rightChannel[startIdx + i] = floatData[i * 2 + 1];
                }
                samplesRead += convertedSamples;
            }
            av_freep(&outputBuffer);
        }
        
        if (samplesRead > 0) {
            return true;
        }
        
        if (m_flushed) {
            // Final flush of resampler
            uint8_t* outputBuffer = nullptr;
            int outSamples = swr_get_out_samples(m_swr, 0);
            if (outSamples > 0) {
                av_samples_alloc(&outputBuffer, nullptr, 2, outSamples, AV_SAMPLE_FMT_FLT, 0);
                int convertedSamples = swr_convert(m_swr, &outputBuffer, outSamples, nullptr, 0);
                if (convertedSamples > 0) {
                    float* floatData = reinterpret_cast<float*>(outputBuffer);
                    size_t startIdx = leftChannel.size();
                    leftChannel.resize(startIdx + convertedSamples);
                    rightChannel.resize(startIdx + convertedSamples);
                    for (int i = 0; i < convertedSamples; i++) {
                        leftChannel[startIdx + i] = floatData[i * 2];
                        rightChannel[startIdx + i] = floatData[i * 2 + 1];
                    }
                    samplesRead += convertedSamples;
                }
                av_freep(&outputBuffer);
            }
            return samplesRead > 0;
        }
    }
    
    return false;
}

double MediaAudioDecoder::getDurationSeconds() const {
    return m_durationSec;
}

double MediaAudioDecoder::getCurrentSeconds() const {
    return m_currentSec;
}

void MediaAudioDecoder::close() {
    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    if (m_packet) {
        av_packet_free(&m_packet);
        m_packet = nullptr;
    }
    if (m_swr) {
        swr_free(&m_swr);
        m_swr = nullptr;
    }
    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
        m_codecContext = nullptr;
    }
    if (m_formatContext) {
        avformat_close_input(&m_formatContext);
        m_formatContext = nullptr;
    }
}

} // namespace vocal_advance
