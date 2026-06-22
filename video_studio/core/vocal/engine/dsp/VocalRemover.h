#pragma once

#include <QString>
#include <functional>

namespace vocal_advance {

class VocalRemover {
public:
    VocalRemover();
    ~VocalRemover();

    // Configuration for the vocal remover
    struct Config {
        bool useAiModel = false; // Placeholder for future ONNX integration
        QString modelPath = "";
        int isolationType = 1; // 1 = Remove vocals (karaoke), 2 = Extract vocals
    };

    // Callback type for reporting progress (0-100)
    using ProgressCallback = std::function<void(int)>;

    // Process a file
    // Returns true if successful, false otherwise
    bool process(const QString& inputWavPath, 
                 const QString& outputWavPath, 
                 const Config& config,
                 ProgressCallback onProgress = nullptr);

private:
    // Advanced STFT Spectral Center Cancellation
    bool processSTFT(const QString& inputWavPath, 
                     const QString& outputWavPath, 
                     const Config& config,
                     ProgressCallback onProgress);
};
} // namespace vocal_advance
