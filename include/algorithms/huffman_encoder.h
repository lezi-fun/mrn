#pragma once

#include "algorithm_base.h"
#include <vector>
#include <cstdint>
#include <queue>
#include <memory>

namespace mrn {

// 霍夫曼树节点
struct HuffmanNode {
    uint8_t symbol;
    uint32_t frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;
    
    HuffmanNode(uint8_t sym, uint32_t freq) 
        : symbol(sym), frequency(freq), left(nullptr), right(nullptr) {}
    
    bool isLeaf() const { return !left && !right; }
};

// 霍夫曼编码表
struct HuffmanCode {
    uint8_t symbol;
    uint32_t code;
    uint8_t length;
};

class HuffmanEncoder {
public:
    HuffmanEncoder();
    ~HuffmanEncoder() = default;
    
    // 霍夫曼编码
    HuffmanEncodingResult encode(const std::vector<uint8_t>& data);
    
    // 霍夫曼解码
    std::vector<uint8_t> decode(const HuffmanEncodingResult& result);
    
    // 配置参数
    void setMaxCodeLength(uint8_t length) { maxCodeLength_ = length; }
    
private:
    uint8_t maxCodeLength_ = 15;  // 最大码长
    
    // 构建频率表
    std::vector<uint32_t> buildFrequencyTable(const std::vector<uint8_t>& data);
    
    // 构建霍夫曼树
    std::shared_ptr<HuffmanNode> buildHuffmanTree(const std::vector<uint32_t>& frequencies);
    
    // 生成编码表
    void generateCodes(const std::shared_ptr<HuffmanNode>& node, 
                      uint32_t code, uint8_t length,
                      std::vector<HuffmanCode>& codes);
    
    // 编码数据
    std::vector<uint8_t> encodeData(const std::vector<uint8_t>& data,
                                   const std::vector<HuffmanCode>& codes);
    
    // 解码数据
    std::vector<uint8_t> decodeData(const std::vector<uint8_t>& encodedData,
                                   const std::shared_ptr<HuffmanNode>& root,
                                   uint32_t originalSize);
    
    // 序列化霍夫曼树
    std::vector<uint8_t> serializeTree(const std::shared_ptr<HuffmanNode>& root);
    
    // 反序列化霍夫曼树
    std::shared_ptr<HuffmanNode> deserializeTree(const std::vector<uint8_t>& serialized);
};

// 霍夫曼编码算法插件
class HuffmanEncoderPlugin : public AlgorithmBase {
public:
    static std::string getStaticName() { return "huffman"; }
    
    std::string getName() const override { return "Huffman Encoder"; }
    std::string getVersion() const override { return "1.0"; }
    uint32_t getAlgorithmId() const override { return 0x4846; } // "HF"
    
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
    HuffmanEncoder huffman_;
};

} // namespace mrn
