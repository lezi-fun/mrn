#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <map>

namespace mrn {

// 压缩参数结构
struct CompressParams {
    enum Mode {
        FAST = 0,
        STANDARD = 1,
        MAXIMUM = 2,
        STORE = 3  // 仅存储，不压缩
    };
    
    Mode mode = STANDARD;
    int compressionLevel = 6;
    bool useMoveOptimization = true;
    uint32_t dictionarySize = 32768;
    uint32_t maxMatchLength = 258;
};

struct DecompressParams {
    bool verifyChecksum = true;
};

// 压缩结果
struct CompressionResult {
    std::vector<uint8_t> compressedData;
    uint64_t uncompressedSize = 0;
    uint64_t compressedSize = 0;
    bool isCompressed = false;
    uint32_t checksum = 0;
};

struct DecompressionResult {
    std::vector<uint8_t> decompressedData;
    uint64_t originalSize = 0;
    bool success = false;
    std::string errorMessage;
};

// 算法能力描述
struct AlgorithmCapabilities {
    bool supportsText = true;
    bool supportsBinary = true;
    bool supportsStreaming = false;
    uint32_t minInputSize = 1;
    uint32_t maxInputSize = UINT32_MAX;
    std::vector<std::string> supportedFileTypes;
};

// 算法配置
struct AlgorithmConfig {
    std::map<std::string, std::string> parameters;
};

// 压缩算法插件接口
class ICompressionAlgorithm {
public:
    virtual ~ICompressionAlgorithm() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual uint32_t getAlgorithmId() const = 0;
    
    // 压缩接口
    virtual CompressionResult compress(const CompressParams& params, 
                                      const std::vector<uint8_t>& data) = 0;
    
    // 解压接口  
    virtual DecompressionResult decompress(const DecompressParams& params,
                                          const std::vector<uint8_t>& data) = 0;
    
    // 算法能力描述
    virtual AlgorithmCapabilities getCapabilities() const = 0;
    
    // 配置接口
    virtual void configure(const AlgorithmConfig& config) = 0;
};

// 预处理器插件接口
class IPreprocessor {
public:
    virtual ~IPreprocessor() = default;
    virtual std::string getName() const = 0;
    virtual std::vector<uint8_t> process(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> inverseProcess(const std::vector<uint8_t>& data) = 0;
};

} // namespace mrn
