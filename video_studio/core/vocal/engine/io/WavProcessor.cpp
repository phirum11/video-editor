#include "WavProcessor.h"
#include <QFile>
#include <QDebug>
#include <cstring>
#include <algorithm>

namespace vocal_advance {

WavProcessor::WavProcessor() {
    memset(&m_header, 0, sizeof(WavHeader));
}

WavProcessor::~WavProcessor() {
    close();
}

bool WavProcessor::openForWrite(const QString& filePath, uint32_t sampleRate, uint16_t channels) {
    close();

    m_outFile = new QFile(filePath);
    if (!m_outFile->open(QIODevice::WriteOnly)) {
        qWarning() << "WavProcessor: Failed to open file for streaming write" << filePath;
        delete m_outFile;
        m_outFile = nullptr;
        return false;
    }

    int bitsPerSample = 16;
    int bytesPerSample = bitsPerSample / 8;

    memcpy(m_header.riff, "RIFF", 4);
    m_header.overallSize = 0; // Placeholder, updated on close
    memcpy(m_header.wave, "WAVE", 4);
    memcpy(m_header.fmtChunkMarker, "fmt ", 4);
    m_header.lengthOfFmt = 16;
    m_header.formatType = 1; // PCM
    m_header.channels = channels;
    m_header.sampleRate = sampleRate;
    m_header.byteRate = sampleRate * channels * bytesPerSample;
    m_header.blockAlign = channels * bytesPerSample;
    m_header.bitsPerSample = bitsPerSample;
    memcpy(m_header.dataChunkHeader, "data", 4);
    m_header.dataSize = 0; // Placeholder, updated on close

    m_dataBytesWritten = 0;

    // Write placeholder header
    m_outFile->write(reinterpret_cast<const char*>(&m_header), sizeof(WavHeader));

    return true;
}

bool WavProcessor::writeChunk(const std::vector<float>& leftChannel, const std::vector<float>& rightChannel) {
    if (!m_outFile || !m_outFile->isOpen()) return false;

    int numSamples = leftChannel.size();
    if (numSamples == 0) return true;

    int channels = m_header.channels;
    std::vector<int16_t> outBuffer(numSamples * channels);
    for (int i = 0; i < numSamples; ++i) {
        float l = std::clamp(leftChannel[i], -1.0f, 1.0f);
        outBuffer[i * channels] = static_cast<int16_t>(l * 32767.0f);
        
        if (channels == 2) {
            float r = (i < rightChannel.size()) ? std::clamp(rightChannel[i], -1.0f, 1.0f) : l;
            outBuffer[i * channels + 1] = static_cast<int16_t>(r * 32767.0f);
        }
    }

    qint64 bytesWritten = m_outFile->write(reinterpret_cast<const char*>(outBuffer.data()), outBuffer.size() * sizeof(int16_t));
    if (bytesWritten > 0) {
        m_dataBytesWritten += bytesWritten;
    }

    return bytesWritten == static_cast<qint64>(outBuffer.size() * sizeof(int16_t));
}

void WavProcessor::close() {
    if (m_outFile && m_outFile->isOpen()) {
        // Seek back to beginning and update RIFF headers with final size
        m_header.dataSize = m_dataBytesWritten;
        m_header.overallSize = m_dataBytesWritten + sizeof(WavHeader) - 8;
        
        m_outFile->seek(0);
        m_outFile->write(reinterpret_cast<const char*>(&m_header), sizeof(WavHeader));
        
        m_outFile->close();
    }
    
    if (m_outFile) {
        delete m_outFile;
        m_outFile = nullptr;
    }
}

// ... legacy read/write implementation below ...

bool WavProcessor::read(const QString& filePath, std::vector<float>& leftChannel, std::vector<float>& rightChannel) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (file.read(reinterpret_cast<char*>(&m_header), sizeof(WavHeader)) != sizeof(WavHeader)) return false;
    if (strncmp(m_header.riff, "RIFF", 4) != 0 || strncmp(m_header.wave, "WAVE", 4) != 0) return false;
    if (m_header.formatType != 1) return false;

    while (strncmp(m_header.dataChunkHeader, "data", 4) != 0) {
        file.seek(file.pos() + m_header.dataSize);
        if (file.read(m_header.dataChunkHeader, 4) != 4) return false;
        if (file.read(reinterpret_cast<char*>(&m_header.dataSize), 4) != 4) return false;
    }

    int bytesPerSample = m_header.bitsPerSample / 8;
    int numSamples = m_header.dataSize / (m_header.channels * bytesPerSample);

    leftChannel.resize(numSamples);
    if (m_header.channels >= 2) rightChannel.resize(numSamples);
    else rightChannel.clear();

    QByteArray data = file.read(m_header.dataSize);
    const char* rawData = data.constData();
    for (int i = 0; i < numSamples; ++i) {
        for (int c = 0; c < m_header.channels; ++c) {
            float sampleValue = 0.0f;
            int offset = (i * m_header.channels + c) * bytesPerSample;
            if (offset + bytesPerSample > data.size()) break;

            if (m_header.bitsPerSample == 16) {
                sampleValue = (*reinterpret_cast<const int16_t*>(rawData + offset)) / 32768.0f;
            } else if (m_header.bitsPerSample == 24) {
                int32_t val = (static_cast<uint8_t>(rawData[offset]) | 
                              (static_cast<uint8_t>(rawData[offset+1]) << 8) | 
                              (static_cast<int8_t>(rawData[offset+2]) << 16));
                sampleValue = val / 8388608.0f;
            } else if (m_header.bitsPerSample == 32) {
                sampleValue = (*reinterpret_cast<const int32_t*>(rawData + offset)) / 2147483648.0f;
            }

            if (c == 0) leftChannel[i] = sampleValue;
            else if (c == 1) rightChannel[i] = sampleValue;
        }
    }
    return true;
}

bool WavProcessor::write(const QString& filePath, const std::vector<float>& leftChannel, const std::vector<float>& rightChannel, uint32_t sampleRate) {
    if (!openForWrite(filePath, sampleRate, rightChannel.empty() ? 1 : 2)) return false;
    bool success = writeChunk(leftChannel, rightChannel);
    close();
    return success;
}

} // namespace vocal_advance
