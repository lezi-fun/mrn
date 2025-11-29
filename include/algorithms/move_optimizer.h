#pragma once

#include "algorithm_base.h"
#include <vector>
#include <cstdint>

namespace mrn {

class MoveOptimizer {
public:
    MoveOptimizer();
    ~MoveOptimizer() = default;
    
    // Move优化算法
    MoveOptimizationResult optimize(const std::vector<uint8_t>& data, 
                                   CompressParams::Mode mode);
    
    // 反转优化
    std::vector<uint8_t> reverseOptimize(const std::vector<uint8_t>& optimizedData,
                                        const std::vector<uint32_t>& moveOffsets);
    
    // 配置优化参数
    void setWindowSize(uint32_t windowSize) { windowSize_ = windowSize; }
    void setMinRunLength(uint32_t minRun) { minRunLength_ = minRun; }
    
private:
    uint32_t windowSize_ = 8192;     // 滑动窗口大小
    uint32_t minRunLength_ = 3;      // 最小运行长度
    
    // 查找最长重复序列
    struct Match {
        uint32_t position;
        uint32_t length;
    };
    
    Match findBestMatch(const std::vector<uint8_t>& data, 
                       uint32_t currentPos, 
                       uint32_t windowStart);
    
    // 检测运行长度编码机会
    std::vector<uint8_t> applyRunLengthEncoding(const std::vector<uint8_t>& data);
    std::vector<uint8_t> reverseRunLengthEncoding(const std::vector<uint8_t>& data);
    
    // 移动重复块到前面
    std::vector<uint8_t> moveRepeatedBlocks(const std::vector<uint8_t>& data,
                                           std::vector<uint32_t>& moveOffsets);
};

// MoveRun压缩算法实现
class MoveRunCompressor : public AlgorithmBase {
public:
    static std::string getStaticName() { return "moverun"; }
    
    std::string getName() const override { return "MoveRun Compressor"; }
    std::string getVersion() const override { return "2.0"; }
    uint32_t getAlgorithmId() const override { return 0x4D52; } // "MR"
    
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
    MoveOptimizer moveOptimizer_;
    class LZ77Compressor* lz77_;
    class HuffmanEncoder* huffman_;
};

} // namespace mrn
