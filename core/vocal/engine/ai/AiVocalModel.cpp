#include "AiVocalModel.h"
#include <QDebug>
#include <QFile>
#include <algorithm>

#ifdef _WIN32
#include <sal.h>
#ifndef _Maybenull_
#define _Maybenull_
#endif
#ifndef _In_
#define _In_
#endif
#include <dml_provider_factory.h>
#endif

namespace vocal_advance {

AiVocalModel::AiVocalModel() {
    try {
        m_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "AiVocalModel");
        m_memoryInfo = std::make_unique<Ort::MemoryInfo>(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
    } catch (const Ort::Exception& e) {
        qWarning() << "AiVocalModel: Failed to initialize ONNX Environment:" << e.what();
    }
}

AiVocalModel::~AiVocalModel() {
}

bool AiVocalModel::load(const QString& modelPath) {
    if (!m_env) return false;

    if (!QFile::exists(modelPath)) {
        qWarning() << "AiVocalModel: Model file not found:" << modelPath;
        return false;
    }

    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(4);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

#ifdef _WIN32
        try {
            Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, 0));
            qDebug() << "AiVocalModel: DirectML GPU Execution Provider enabled.";
        } catch (const Ort::Exception& e) {
            qWarning() << "AiVocalModel: Failed to enable DirectML (GPU). Falling back to CPU. Error:" << e.what();
        }
#endif

        // Convert QString to wstring for Windows ONNX Runtime
        std::wstring wModelPath = modelPath.toStdWString();
        
        m_session = std::make_unique<Ort::Session>(*m_env, wModelPath.c_str(), sessionOptions);

        Ort::AllocatorWithDefaultOptions allocator;

        // Get input node names
        m_numInputNodes = m_session->GetInputCount();
        m_inputNodeNames.clear();
        m_allocatedInputNames.clear();
        for (size_t i = 0; i < m_numInputNodes; i++) {
            Ort::AllocatedStringPtr namePtr = m_session->GetInputNameAllocated(i, allocator);
            m_inputNodeNames.push_back(namePtr.get());
            m_allocatedInputNames.push_back(std::move(namePtr));
        }

        // Get output node names
        m_numOutputNodes = m_session->GetOutputCount();
        m_outputNodeNames.clear();
        m_allocatedOutputNames.clear();
        for (size_t i = 0; i < m_numOutputNodes; i++) {
            Ort::AllocatedStringPtr namePtr = m_session->GetOutputNameAllocated(i, allocator);
            m_outputNodeNames.push_back(namePtr.get());
            m_allocatedOutputNames.push_back(std::move(namePtr));
        }

        m_isLoaded = true;
        qDebug() << "AiVocalModel: Successfully loaded ONNX model:" << modelPath;
        return true;

    } catch (const Ort::Exception& e) {
        qWarning() << "AiVocalModel: Failed to load model:" << e.what();
        m_isLoaded = false;
        return false;
    }
}

bool AiVocalModel::processChunk(const std::vector<float>& inputLeft, 
                                const std::vector<float>& inputRight,
                                std::vector<float>& outputLeft,
                                std::vector<float>& outputRight,
                                bool extractVocals) 
{
    if (!m_isLoaded || !m_session) return false;
    if (inputLeft.empty() || inputLeft.size() != inputRight.size()) return false;

    size_t numSamples = inputLeft.size();

    // Prepare input tensor: [batch_size=1, channels=2, samples=numSamples]
    std::vector<int64_t> inputDims = { 1, 2, static_cast<int64_t>(numSamples) };
    
    // Flatten interleaved audio into planar (L L L ... R R R ...) as expected by most AI models
    std::vector<float> inputTensorValues(numSamples * 2);
    for (size_t i = 0; i < numSamples; ++i) {
        inputTensorValues[i] = inputLeft[i];
        inputTensorValues[numSamples + i] = inputRight[i];
    }

    try {
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            *m_memoryInfo, 
            inputTensorValues.data(), 
            inputTensorValues.size(), 
            inputDims.data(), 
            inputDims.size()
        );

        // Run inference
        auto outputTensors = m_session->Run(
            Ort::RunOptions{nullptr}, 
            m_inputNodeNames.data(), 
            &inputTensor, 
            1, 
            m_outputNodeNames.data(), 
            m_numOutputNodes
        );

        if (outputTensors.empty()) return false;

        // HT-Demucs outputs: [batch=1, stems=4, channels=2, samples=numSamples]
        // Stems order: 0=drums, 1=bass, 2=other, 3=vocals
        const float* outData = outputTensors[0].GetTensorMutableData<float>();
        
        outputLeft.resize(numSamples, 0.0f);
        outputRight.resize(numSamples, 0.0f);
        
        size_t channelOffset = numSamples;
        size_t stemOffset = 2 * numSamples;
        
        if (extractVocals) {
            // Vocal stem is index 3
            size_t vocalBase = 3 * stemOffset;
            for (size_t i = 0; i < numSamples; ++i) {
                outputLeft[i] = outData[vocalBase + i];
                outputRight[i] = outData[vocalBase + channelOffset + i];
            }
        } else {
            // Instrumental is sum of drums(0) + bass(1) + other(2)
            for (size_t s = 0; s < 3; ++s) {
                size_t base = s * stemOffset;
                for (size_t i = 0; i < numSamples; ++i) {
                    outputLeft[i] += outData[base + i];
                    outputRight[i] += outData[base + channelOffset + i];
                }
            }
        }
        
        return true;

    } catch (const Ort::Exception& e) {
        qWarning() << "AiVocalModel: Inference error:" << e.what();
        return false;
    }
}

bool AiVocalModel::isLoaded() const {
    return m_isLoaded;
}

} // namespace vocal_advance
