#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mrn {

struct AlgorithmConfig {
    std::map<std::string, std::string> values;
};

struct AlgorithmCapabilities {
    bool supportsStreaming = false;
    bool supportsMultithreading = false;
    uint32_t maxWindowSize = 0;
};

struct CompressParams {
    AlgorithmConfig config;
    int level = 6;
    std::string mode = "default";
};

struct CompressionResult {
    std::vector<uint8_t> compressedData;
    uint64_t uncompressedSize = 0;
    bool isCompressed = true;
};

struct DecompressionResult {
    std::vector<uint8_t> decompressedData;
};

struct DecompressParams {
    AlgorithmConfig config;
    uint64_t expectedSize = 0;
    bool dataIsCompressed = true;
};

class ICompressionAlgorithm {
public:
    virtual ~ICompressionAlgorithm() = default;

    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual uint32_t getAlgorithmId() const = 0;

    virtual CompressionResult compress(const CompressParams& params,
                                       const std::vector<uint8_t>& data) = 0;

    virtual DecompressionResult decompress(const DecompressParams& params,
                                           const std::vector<uint8_t>& data) = 0;

    virtual AlgorithmCapabilities getCapabilities() const = 0;

    virtual void configure(const AlgorithmConfig& config) = 0;
};

class IPreprocessor {
public:
    virtual ~IPreprocessor() = default;
    virtual std::vector<uint8_t> process(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> inverseProcess(const std::vector<uint8_t>& data) = 0;
};

} // namespace mrn
