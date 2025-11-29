#include "algorithms/huffman_encoder.h"
#include <queue>
#include <functional>

namespace mrn {

HuffmanEncoder::HuffmanEncoder() : maxCodeLength_(15) {}

HuffmanEncodingResult HuffmanEncoder::encode(const std::vector<uint8_t>& data) {
    HuffmanEncodingResult result;
    result.originalSize = data.size();
    
    if (data.empty()) {
        result.encodedSize = 0;
        return result;
    }
    
    try {
        auto frequencies = buildFrequencyTable(data);
        auto root = buildHuffmanTree(frequencies);
        std::vector<HuffmanCode> codes;
        generateCodes(root, 0, 0, codes);
        result.encodedData = encodeData(data, codes);
        result.frequencyTable = frequencies;
        result.encodedSize = result.encodedData.size();
        
    } catch (const std::exception& e) {
        result.encodedData = data;
        result.encodedSize = data.size();
    }
    
    return result;
}

std::vector<uint8_t> HuffmanEncoder::decode(const HuffmanEncodingResult& result) {
    // Simple implementation - just return the encoded data
    return result.encodedData;
}

std::vector<uint32_t> HuffmanEncoder::buildFrequencyTable(const std::vector<uint8_t>& data) {
    std::vector<uint32_t> frequencies(256, 0);
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }
    return frequencies;
}

std::shared_ptr<HuffmanNode> HuffmanEncoder::buildHuffmanTree(const std::vector<uint32_t>& frequencies) {
    auto comp = [](const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) {
        return a->frequency > b->frequency;
    };
    
    std::priority_queue<std::shared_ptr<HuffmanNode>, 
                       std::vector<std::shared_ptr<HuffmanNode>>, 
                       decltype(comp)> minHeap(comp);
    
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            minHeap.push(std::make_shared<HuffmanNode>(static_cast<uint8_t>(i), frequencies[i]));
        }
    }
    
    while (minHeap.size() > 1) {
        auto left = minHeap.top(); minHeap.pop();
        auto right = minHeap.top(); minHeap.pop();
        
        auto parent = std::make_shared<HuffmanNode>(0, left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        minHeap.push(parent);
    }
    
    return minHeap.empty() ? nullptr : minHeap.top();
}

void HuffmanEncoder::generateCodes(const std::shared_ptr<HuffmanNode>& node, 
                                  uint32_t code, uint8_t length,
                                  std::vector<HuffmanCode>& codes) {
    if (!node) return;
    
    if (node->isLeaf()) {
        codes.push_back({node->symbol, code, length});
    } else {
        generateCodes(node->left, (code << 1) | 0, length + 1, codes);
        generateCodes(node->right, (code << 1) | 1, length + 1, codes);
    }
}

std::vector<uint8_t> HuffmanEncoder::encodeData(const std::vector<uint8_t>& data,
                                               const std::vector<HuffmanCode>& codes) {
    std::vector<uint8_t> result;
    // Simple implementation - just return the original data
    result = data;
    return result;
}

std::vector<uint8_t> HuffmanEncoder::decodeData(const std::vector<uint8_t>& encodedData,
                                               const std::shared_ptr<HuffmanNode>& root,
                                               uint32_t originalSize) {
    // Simple implementation - just return the encoded data
    return encodedData;
}

std::vector<uint8_t> HuffmanEncoder::serializeTree(const std::shared_ptr<HuffmanNode>& root) {
    // TODO: Implement tree serialization
    return std::vector<uint8_t>();
}

std::shared_ptr<HuffmanNode> HuffmanEncoder::deserializeTree(const std::vector<uint8_t>& serialized) {
    // TODO: Implement tree deserialization
    return nullptr;
}

// HuffmanEncoderPlugin implementation
CompressionResult HuffmanEncoderPlugin::compress(const CompressParams& params,
                                                const std::vector<uint8_t>& data) {
    CompressionResult result;
    result.uncompressedSize = data.size();
    
    if (data.empty()) {
        result.compressedData = data;
        result.compressedSize = 0;
        result.isCompressed = false;
        return result;
    }
    
    try {
        auto huffmanResult = huffman_.encode(data);
        result.compressedData = huffmanResult.encodedData;
        result.compressedSize = huffmanResult.encodedSize;
        result.isCompressed = huffmanResult.encodedSize < data.size();
        result.checksum = calculateChecksum(data);
        
    } catch (const std::exception& e) {
        result.compressedData = data;
        result.compressedSize = data.size();
        result.isCompressed = false;
    }
    
    return result;
}

DecompressionResult HuffmanEncoderPlugin::decompress(const DecompressParams& params,
                                                    const std::vector<uint8_t>& data) {
    DecompressionResult result;
    
    try {
        // TODO: Implement proper decompression
        HuffmanEncodingResult huffmanResult;
        huffmanResult.encodedData = data;
        result.decompressedData = huffman_.decode(huffmanResult);
        result.originalSize = result.decompressedData.size();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    return result;
}

} // namespace mrn
