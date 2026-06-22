#pragma once

#include <QString>
#include <vector>
#include <memory>

#if defined(__MINGW32__) && !defined(_Frees_ptr_opt_)
#define _Frees_ptr_opt_
#endif

#include <onnxruntime_cxx_api.h>

namespace vocal_advance {

class AiVocalModel {
public:
    AiVocalModel();
    ~AiVocalModel();

    // Load the ONNX model into memory (CPU/DirectML)
    bool load(const QString& modelPath);

    // Process a chunk of interleaved stereo audio (L, R, L, R...)
    // inputChunk: The raw mixed audio
    // outVocals: The extracted vocals (if requested)
    // outInstrumental: The extracted instrumental (if requested)
    // Both output vectors will be resized appropriately
    bool processChunk(const std::vector<float>& inputLeft, 
                      const std::vector<float>& inputRight,
                      std::vector<float>& outputLeft,
                      std::vector<float>& outputRight,
                      bool extractVocals);

    // Returns true if a model is successfully loaded
    bool isLoaded() const;

private:
    std::unique_ptr<Ort::Env> m_env;
    std::unique_ptr<Ort::Session> m_session;
    std::unique_ptr<Ort::MemoryInfo> m_memoryInfo;

    bool m_isLoaded = false;
    
    // Model I/O info
    size_t m_numInputNodes = 0;
    size_t m_numOutputNodes = 0;
    std::vector<const char*> m_inputNodeNames;
    std::vector<const char*> m_outputNodeNames;
    std::vector<Ort::AllocatedStringPtr> m_allocatedInputNames;
    std::vector<Ort::AllocatedStringPtr> m_allocatedOutputNames;
};

} // namespace vocal_advance
