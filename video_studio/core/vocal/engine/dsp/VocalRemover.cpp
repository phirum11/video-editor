#include "VocalRemover.h"
#include "WavProcessor.h"
#include "MediaAudioDecoder.h"
#include "Fft.h"
#include "AiVocalModel.h"
#include <QDebug>
#include <cmath>
#include <algorithm>
#include <deque>
#include <vector>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace vocal_advance {

// ... STFT utilities omitted for brevity but remain the same ...

static inline float sigmoid(float x, float center, float steepness) {
    return 1.0f / (1.0f + std::exp(-steepness * (x - center)));
}

static inline float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static float computeCenterScore(const Fft::Complex& L, const Fft::Complex& R) {
    float magL = std::abs(L);
    float magR = std::abs(R);
    float maxMag = std::max(magL, magR);
    if (maxMag < 1e-10f) return 0.0f;
    float minMag = std::min(magL, magR);
    float magSimilarity = minMag / maxMag;
    float dotReal = L.real() * R.real() + L.imag() * R.imag();
    float phaseSimilarity = dotReal / (magL * magR + 1e-10f);
    phaseSimilarity = std::clamp(phaseSimilarity, -1.0f, 1.0f);
    float phaseWeight = std::max(0.0f, phaseSimilarity);
    return magSimilarity * phaseWeight * phaseWeight;
}

static float getVoiceFrequencyWeight(float freq) {
    if (freq < 100.0f) return 0.0f;
    if (freq < 200.0f) return 0.0f;
    if (freq < 300.0f) return smoothstep(200.0f, 300.0f, freq) * 0.25f;
    if (freq < 900.0f) return 0.65f;
    if (freq < 2500.0f) return 1.0f;
    if (freq < 3500.0f) return 0.85f;
    if (freq < 6000.0f) return smoothstep(6000.0f, 3500.0f, freq) * 0.55f;
    if (freq < 8000.0f) return smoothstep(8000.0f, 6000.0f, freq) * 0.2f;
    return 0.0f;
}

static float computeSpectralFlux(const std::vector<float>& currentMag, 
                                  const std::vector<float>& prevMag,
                                  int startBin, int endBin) {
    float flux = 0.0f;
    float energy = 0.0f;
    for (int i = startBin; i < endBin; ++i) {
        float diff = currentMag[i] - prevMag[i];
        if (diff > 0.0f) flux += diff * diff;
        energy += currentMag[i] * currentMag[i];
    }
    if (energy < 1e-10f) return 0.0f;
    return flux / energy;
}

static float computeSpectralSmoothness(const std::vector<float>& mag, int bin, int halfSpectrum) {
    if (bin <= 1 || bin >= halfSpectrum - 2) return 0.5f;
    float center = mag[bin];
    float avg = (mag[bin-2] + mag[bin-1] + mag[bin+1] + mag[bin+2]) * 0.25f;
    if (avg < 1e-10f && center < 1e-10f) return 0.5f;
    float ratio = std::min(center, avg) / (std::max(center, avg) + 1e-10f);
    return ratio;
}


VocalRemover::VocalRemover() {}
VocalRemover::~VocalRemover() {}

bool VocalRemover::process(const QString& inputWavPath, 
                           const QString& outputWavPath, 
                           const Config& config,
                           ProgressCallback onProgress) 
{
    if (config.useAiModel && !config.modelPath.isEmpty()) {
        qDebug() << "VocalRemover: Starting ONNX AI STREAMING process. Input:" << inputWavPath;
        
        AiVocalModel aiModel;
        if (aiModel.load(config.modelPath)) {
            // Process using AI
            MediaAudioDecoder decoder;
            const float sampleRate = 44100.0f;
            if (!decoder.open(inputWavPath, static_cast<int>(sampleRate))) return false;
            
            WavProcessor wav;
            if (!wav.openForWrite(outputWavPath, static_cast<uint32_t>(sampleRate), 2)) return false;

            bool decoding = true;
            double totalSec = decoder.getDurationSeconds();
            
            // HT-Demucs expects exactly 343980 samples per inference (approx 7.8s at 44.1kHz)
            const int chunkSize = 343980; 
            
            std::deque<float> inLeftBuffer;
            std::deque<float> inRightBuffer;

            while (decoding || inLeftBuffer.size() > 0) {
                // Refill buffer
                if (decoding && inLeftBuffer.size() < chunkSize) {
                    std::vector<float> chunkL, chunkR;
                    decoding = decoder.readChunk(chunkL, chunkR);
                    if (!chunkL.empty()) {
                        inLeftBuffer.insert(inLeftBuffer.end(), chunkL.begin(), chunkL.end());
                        inRightBuffer.insert(inRightBuffer.end(), chunkR.begin(), chunkR.end());
                    }
                }

                // If we have enough for a chunk, or we are at the end and have remainder
                if (inLeftBuffer.size() >= chunkSize || (!decoding && inLeftBuffer.size() > 0)) {
                    int currentChunkSize = std::min(static_cast<int>(inLeftBuffer.size()), chunkSize);
                    
                    std::vector<float> processL(chunkSize, 0.0f);
                    std::vector<float> processR(chunkSize, 0.0f);
                    
                    for (int i = 0; i < currentChunkSize; ++i) {
                        processL[i] = inLeftBuffer.front();
                        processR[i] = inRightBuffer.front();
                        inLeftBuffer.pop_front();
                        inRightBuffer.pop_front();
                    }

                    std::vector<float> outL;
                    std::vector<float> outR;
                    
                    bool extractVocals = (config.isolationType == 2);
                    
                    if (aiModel.processChunk(processL, processR, outL, outR, extractVocals)) {
                        // Trim the padded zeros if this was the last chunk
                        if (currentChunkSize < chunkSize) {
                            outL.resize(currentChunkSize);
                            outR.resize(currentChunkSize);
                        }
                        wav.writeChunk(outL, outR);
                    } else {
                        qWarning() << "AI Model inference failed on chunk. Writing silence.";
                        std::vector<float> zeros(currentChunkSize, 0.0f);
                        wav.writeChunk(zeros, zeros);
                    }

                    if (onProgress && totalSec > 0.0) {
                        double current = decoder.getCurrentSeconds();
                        int progress = static_cast<int>((current * 100.0) / totalSec);
                        progress = std::clamp(progress, 0, 99);
                        onProgress(progress);
                    }
                }
            }
            
            wav.close();
            decoder.close();
            if (onProgress) onProgress(100);
            qDebug() << "VocalRemover: ONNX AI processing complete.";
            return true;
        } else {
            qWarning() << "Failed to load ONNX model. Falling back to Advanced STFT DSP.";
        }
    }

    qDebug() << "VocalRemover: Starting ADVANCED voice-focused STFT. Input:" << inputWavPath;
    return processSTFT(inputWavPath, outputWavPath, config, onProgress);
}

bool VocalRemover::processSTFT(const QString& inputWavPath, 
                               const QString& outputWavPath, 
                               const Config& config,
                               ProgressCallback onProgress)
{
    MediaAudioDecoder decoder;
    const float sampleRate = 44100.0f;
    if (!decoder.open(inputWavPath, static_cast<int>(sampleRate))) return false;
    
    WavProcessor wav;
    if (!wav.openForWrite(outputWavPath, static_cast<uint32_t>(sampleRate), 2)) return false;

    // ── STFT Parameters ──
    const int fftSize = 4096;
    const int hopSize = 1024; // 75% overlap
    const int halfSpectrum = fftSize / 2 + 1;

    std::vector<float> window(fftSize);
    for (int i = 0; i < fftSize; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
    }

    std::vector<Fft::Complex> L_fft(fftSize);
    std::vector<Fft::Complex> R_fft(fftSize);

    std::deque<float> inLeftBuffer;
    std::deque<float> inRightBuffer;

    std::deque<float> outLeftBuffer(fftSize, 0.0f);
    std::deque<float> outRightBuffer(fftSize, 0.0f);

    std::vector<float> prevMask(halfSpectrum, 0.0f);
    std::vector<float> prevMagnitude(halfSpectrum, 0.0f);
    std::vector<float> currentMagnitude(halfSpectrum, 0.0f);

    const float centerThreshold  = 0.55f;
    const float maskStrength     = 0.88f;
    const float temporalSmooth   = 0.40f;
    const float fluxThreshold    = 0.15f;
    const float smoothnessMin    = 0.25f;

    const float gain = 1.0f / 1.5f;

    bool decoding = true;
    double totalSec = decoder.getDurationSeconds();

    while (decoding || inLeftBuffer.size() >= static_cast<size_t>(fftSize)) {
        if (decoding && inLeftBuffer.size() < static_cast<size_t>(fftSize)) {
            std::vector<float> chunkL, chunkR;
            decoding = decoder.readChunk(chunkL, chunkR);
            if (!chunkL.empty()) {
                inLeftBuffer.insert(inLeftBuffer.end(), chunkL.begin(), chunkL.end());
                inRightBuffer.insert(inRightBuffer.end(), chunkR.begin(), chunkR.end());
            }
        }

        while (inLeftBuffer.size() >= static_cast<size_t>(fftSize)) {
            for (int i = 0; i < fftSize; ++i) {
                L_fft[i] = Fft::Complex(inLeftBuffer[i] * window[i], 0.0f);
                R_fft[i] = Fft::Complex(inRightBuffer[i] * window[i], 0.0f);
            }

            Fft::forward(L_fft);
            Fft::forward(R_fft);

            for (int i = 0; i < halfSpectrum; ++i) {
                float midMag = std::abs((L_fft[i] + R_fft[i]) * 0.5f);
                currentMagnitude[i] = midMag;
            }

            int voiceLowBin  = static_cast<int>(200.0f * fftSize / sampleRate);
            int voiceHighBin = static_cast<int>(6000.0f * fftSize / sampleRate);
            voiceHighBin = std::min(voiceHighBin, halfSpectrum);
            float flux = computeSpectralFlux(currentMagnitude, prevMagnitude, voiceLowBin, voiceHighBin);
            
            float transientProtection = 1.0f - smoothstep(fluxThreshold, fluxThreshold * 3.0f, flux);

            for (int i = 0; i < halfSpectrum; ++i) {
                float freq = i * sampleRate / fftSize;
                
                float freqWeight = getVoiceFrequencyWeight(freq);
                if (freqWeight < 0.01f) continue;
                
                float centerScore = computeCenterScore(L_fft[i], R_fft[i]);
                float smoothness = computeSpectralSmoothness(currentMagnitude, i, halfSpectrum);
                float voiceLikelihood = (smoothness > smoothnessMin) ? 1.0f : smoothness / smoothnessMin;
                
                float centerMask = sigmoid(centerScore, centerThreshold, 14.0f);
                float rawMask = centerMask * freqWeight * voiceLikelihood * transientProtection;
                
                float smoothedMask = temporalSmooth * prevMask[i] + (1.0f - temporalSmooth) * rawMask;
                prevMask[i] = smoothedMask;
                
                float attenuation = smoothedMask * maskStrength;
                
                Fft::Complex mid  = (L_fft[i] + R_fft[i]) * 0.5f;
                Fft::Complex side = (L_fft[i] - R_fft[i]) * 0.5f;
                
                if (config.isolationType == 1) {
                    mid *= (1.0f - attenuation);
                } else if (config.isolationType == 2) {
                    side *= (1.0f - attenuation);
                }
                
                L_fft[i] = mid + side;
                R_fft[i] = mid - side;
                
                if (i > 0 && i < fftSize / 2) {
                    int mirror = fftSize - i;
                    Fft::Complex midM  = (L_fft[mirror] + R_fft[mirror]) * 0.5f;
                    Fft::Complex sideM = (L_fft[mirror] - R_fft[mirror]) * 0.5f;
                    
                    if (config.isolationType == 1) {
                        midM *= (1.0f - attenuation);
                    } else if (config.isolationType == 2) {
                        sideM *= (1.0f - attenuation);
                    }
                    
                    L_fft[mirror] = midM + sideM;
                    R_fft[mirror] = midM - sideM;
                }
            }

            prevMagnitude = currentMagnitude;

            Fft::inverse(L_fft);
            Fft::inverse(R_fft);

            while (outLeftBuffer.size() < static_cast<size_t>(fftSize)) {
                outLeftBuffer.push_back(0.0f);
                outRightBuffer.push_back(0.0f);
            }

            for (int i = 0; i < fftSize; ++i) {
                outLeftBuffer[i] += L_fft[i].real() * window[i];
                outRightBuffer[i] += R_fft[i].real() * window[i];
            }

            for (int i = 0; i < hopSize; ++i) {
                inLeftBuffer.pop_front();
                inRightBuffer.pop_front();
            }

            std::vector<float> writeL(hopSize);
            std::vector<float> writeR(hopSize);
            for (int i = 0; i < hopSize; ++i) {
                writeL[i] = outLeftBuffer.front() * gain;
                writeR[i] = outRightBuffer.front() * gain;
                outLeftBuffer.pop_front();
                outRightBuffer.pop_front();
            }
            
            wav.writeChunk(writeL, writeR);

            if (onProgress && totalSec > 0.0) {
                double current = decoder.getCurrentSeconds();
                int progress = static_cast<int>((current * 100.0) / totalSec);
                progress = std::clamp(progress, 0, 99);
                onProgress(progress);
            }
        }
    }

    std::vector<float> flushL(outLeftBuffer.begin(), outLeftBuffer.end());
    std::vector<float> flushR(outRightBuffer.begin(), outRightBuffer.end());
    for (size_t i = 0; i < flushL.size(); ++i) {
        flushL[i] *= gain;
        flushR[i] *= gain;
    }
    wav.writeChunk(flushL, flushR);

    wav.close();
    decoder.close();

    if (onProgress) onProgress(100);

    qDebug() << "VocalRemover: ADVANCED voice-focused processing complete.";
    return true;
}

} 
