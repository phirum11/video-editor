#pragma once

#include <QString>
#include <vector>
#include <cstdint>

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;

namespace vocal_advance {

class MediaAudioDecoder {
public:
    MediaAudioDecoder();
    ~MediaAudioDecoder();

    // Open media file and initialize decoder/resampler
    bool open(const QString& filePath, int outputSampleRate = 44100);

    // Read a chunk of audio. Returns true if more data is available, false on EOF or error.
    // Appends the read samples to leftChannel and rightChannel.
    bool readChunk(std::vector<float>& leftChannel, std::vector<float>& rightChannel);

    // Get total duration in seconds (rough estimate for progress)
    double getDurationSeconds() const;

    // Get current progress in seconds
    double getCurrentSeconds() const;

    void close();

private:
    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    SwrContext* m_swr = nullptr;
    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;
    
    int m_audioStreamIndex = -1;
    bool m_eof = false;
    bool m_flushed = false;
    double m_durationSec = 0.0;
    double m_currentSec = 0.0;
};

} // namespace vocal_advance
