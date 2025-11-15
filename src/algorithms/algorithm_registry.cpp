#include "core/plugin_manager.h"
#include "algorithms/move_optimizer.h"
#include "algorithms/lz77_compressor.h"
#include "algorithms/huffman_encoder.h"

namespace mrn {

// 自动注册宏
#define REGISTER_ALGORITHM(ClassName) \
    namespace { \
        struct ClassName##Registrar { \
            ClassName##Registrar() { \
                PluginManager::getInstance().registerAlgorithm( \
                    ClassName::getStaticName(), \
                    std::make_unique<ClassName>() \
                ); \
            } \
        }; \
        static ClassName##Registrar ClassName##_registrar; \
    }

class MoveRunCompressor : public ICompressionAlgorithm {
public:
    static std::string getStaticName() { return "moverun"; }

    std::string getName() const override { return "MoveRun Compressor"; }
    std::string getVersion() const override { return "2.0"; }
    uint32_t getAlgorithmId() const override { return 0x4D52; }

    CompressionResult compress(const CompressParams& params,
                               const std::vector<uint8_t>& data) override {
        auto moved = moveOptimizer_.optimize(data, params.mode);
        auto lzBlock = lz77_.compress(moved.data);
        CompressionResult result;
        result.compressedData = huffman_.encode(lzBlock.buffer);
        result.uncompressedSize = data.size();
        result.isCompressed = lzBlock.isCompressed;
        return result;
    }

    DecompressionResult decompress(const DecompressParams& params,
                                   const std::vector<uint8_t>& data) override {
        auto decoded = huffman_.decode(data);
        auto decompressed = lz77_.decompress(decoded, params.expectedSize, params.dataIsCompressed);
        DecompressionResult result;
        result.decompressedData = decompressed;
        return result;
    }

    AlgorithmCapabilities getCapabilities() const override {
        AlgorithmCapabilities caps;
        caps.supportsMultithreading = true;
        caps.supportsStreaming = false;
        caps.maxWindowSize = 1 << 20;
        return caps;
    }

    void configure(const AlgorithmConfig& config) override {
        (void)config;
    }

private:
    MoveOptimizer moveOptimizer_;
    LZ77Compressor lz77_;
    HuffmanEncoder huffman_;
};

// 注册MoveRun算法
REGISTER_ALGORITHM(MoveRunCompressor);

} // namespace mrn
