#pragma once

#include <QString>
#include <vector>
#include <cstdint>

class QFile;

namespace vocal_advance {

class WavProcessor {
public:
    struct WavHeader {
        char riff[4];
        uint32_t overallSize;
        char wave[4];
        char fmtChunkMarker[4];
        uint32_t lengthOfFmt;
        uint16_t formatType;
        uint16_t channels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
        char dataChunkHeader[4];
        uint32_t dataSize;
    };

    WavProcessor();
    ~WavProcessor();

    // Legacy full read/write
    bool read(const QString& filePath, std::vector<float>& leftChannel, std::vector<float>& rightChannel);
    bool write(const QString& filePath, const std::vector<float>& leftChannel, const std::vector<float>& rightChannel, uint32_t sampleRate);

    // Streaming API
    bool openForWrite(const QString& filePath, uint32_t sampleRate = 44100, uint16_t channels = 2);
    bool writeChunk(const std::vector<float>& leftChannel, const std::vector<float>& rightChannel);
    void close();

private:
    WavHeader m_header;
    QFile* m_outFile = nullptr;
    uint32_t m_dataBytesWritten = 0;
};

} // namespace vocal_advance
