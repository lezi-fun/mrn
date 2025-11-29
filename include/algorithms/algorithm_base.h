#pragma once

#include "../core/plugin_interface.h"
#include <vector>
#include <string>

namespace mrn {

// 算法基类，提供一些通用功能
class AlgorithmBase : public ICompressionAlgorithm {
public:
    AlgorithmBase() = default;
    virtual ~AlgorithmBase() = default;
    
    // 默认实现
    AlgorithmCapabilities getCapabilities() const override {
        AlgorithmCapabilities caps;
        caps.supportsText = true;
        caps.supportsBinary = true;
        caps.supportsStreaming = false;
        caps.minInputSize = 1;
        caps.maxInputSize = UINT32_MAX;
        return caps;
    }
    
    void configure(const AlgorithmConfig& config) override {
        // 默认实现，子类可以重写
        config_ = config;
    }
    
protected:
    AlgorithmConfig config_;
    
    // 计算校验和
    uint32_t calculateChecksum(const std::vector<uint8_t>& data) const {
        uint32_t checksum = 0;
        for (uint8_t byte : data) {
            checksum = (checksum << 5) + checksum + byte;
        }
        return checksum;
    }
    
    // 判断是否应该跳过压缩（对于已压缩文件）
    bool shouldSkipCompression(const std::vector<uint8_t>& data, 
                              const CompressParams& params) const {
        return params.mode == CompressParams::STORE;
    }
};

// Move优化器结果
struct MoveOptimizationResult {
    std::vector<uint8_t> optimizedData;
    std::vector<uint32_t> moveOffsets;
    uint32_t originalSize;
    uint32_t optimizedSize;
    bool wasOptimized;
};

// LZ77压缩结果
struct LZ77CompressionResult {
    std::vector<uint8_t> compressedData;
    std::vector<uint32_t> literalLengths;
    std::vector<uint32_t> matchDistances;
    std::vector<uint32_t> matchLengths;
    uint32_t originalSize;
    uint32_t compressedSize;
};

// 霍夫曼编码结果
struct HuffmanEncodingResult {
    std::vector<uint8_t> encodedData;
    std::vector<uint32_t> frequencyTable;
    uint32_t originalSize;
    uint32_t encodedSize;
};

} // namespace mrn
