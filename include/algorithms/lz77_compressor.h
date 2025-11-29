#pragma once

#include "algorithm_base.h"
#include <vector>
#include <cstdint>
#include <string>

namespace mrn {

class LZ77Compressor {
public:
    LZ77Compressor();
    ~LZ77Compressor() = default;
    
    // LZ77压缩
    LZ77CompressionResult compress(const std::vector<uint8_t>& data,
                                  uint32_t windowSize = 32768,
                                  uint32_t maxMatchLength = 258);
    
    // LZ77解压
    std::vector<uint8_t> decompress(const LZ77CompressionResult& result);
    
    // 配置参数
    void setWindowSize(uint32_t size) { windowSize_ = size; }
    void setMaxMatchLength(uint32_t length) { maxMatchLength_ = length; }
    void setMinMatchLength(uint32_t length) { minMatchLength_ = length; }
    
private:
    uint32_t windowSize_ = 32768;      // 滑动窗口大小
    uint32_t maxMatchLength_ = 258;    // 最大匹配长度
    uint32_t minMatchLength_ = 3;      // 最小匹配长度
    
    // 在滑动窗口中查找最佳匹配
    struct LZ77Match {
        uint32_t distance;
        uint32_t length;
        bool found;
    };
    
    LZ77Match findBestMatch(const std::vector<uint8_t>& data,
                           uint32_t currentPos,
                           uint32_t windowStart);
    
    // 编码匹配信息
    void encodeMatch(uint32_t distance, uint32_t length, 
                    std::vector<uint8_t>& output,
                    std::vector<uint32_t>& literalLengths,
                    std::vector<uint32_t>& matchDistances,
                    std::vector<uint32_t>& matchLengths);
};

// LZ77压缩算法插件
class LZ77CompressorPlugin : public AlgorithmBase {
public:
    static std::string getStaticName() { return "lz77"; }
    
    std::string getName() const override { return "LZ77 Compressor"; }
    std::string getVersion() const override { return "1.0"; }
    uint32_t getAlgorithmId() const override { return 0x4C5A; } // "LZ"
    
    CompressionResult compress(const CompressParams& params,
                              const std::vector<uint8_t>& data) override;
    
    DecompressionResult decompress(const DecompressParams& params,
                                  const std::vector<uint8_t>& data) override;
    
    AlgorithmCapabilities getCapabilities() const override {
        AlgorithmCapabilities caps = AlgorithmBase::getCapabilities();
        caps.supportsText = true;
        caps.supportsBinary = true;
        return caps;
    }
    
private:
    LZ77Compressor lz77_;
};

} // namespace mrn
