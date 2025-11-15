#include "algorithms/huffman_encoder.h"

#include <bitset>
#include <cstring>
#include <iostream>

namespace mrn {

std::vector<uint8_t> HuffmanEncoder::encode(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        return {};
    }
    
    // 如果数据太小，直接返回（Huffman对小数据可能反而增大）
    if (data.size() < 16) {
        std::vector<uint8_t> result;
        result.push_back(0); // 标记：未压缩
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }
    
    // 统计频率
    std::map<uint8_t, uint64_t> frequencies;
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }
    
    // 如果只有一种符号，直接返回
    if (frequencies.size() == 1) {
        std::vector<uint8_t> result;
        result.push_back(0); // 标记：未压缩
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }
    
    // 构建Huffman树
    HuffmanNode* root = buildTree(frequencies);
    
    // 构建编码表
    std::map<uint8_t, std::vector<bool>> codes;
    buildCodeTable(root, codes);
    
    // 编码数据
    std::vector<bool> bits;
    for (uint8_t byte : data) {
        const auto& code = codes[byte];
        bits.insert(bits.end(), code.begin(), code.end());
    }
    
    // 转换为字节
    std::vector<uint8_t> result;
    result.push_back(1); // 标记：已压缩
    
    // 写入频率表大小
    result.push_back(static_cast<uint8_t>(frequencies.size()));
    
    // 写入频率表
    for (const auto& [symbol, freq] : frequencies) {
        result.push_back(symbol);
        uint64_t f = freq;
        for (int i = 0; i < 8; ++i) {
            result.push_back(static_cast<uint8_t>(f & 0xFF));
            f >>= 8;
        }
    }
    
    // 写入编码后的数据长度（位）
    uint32_t bitCount = static_cast<uint32_t>(bits.size());
    for (int i = 0; i < 4; ++i) {
        result.push_back(static_cast<uint8_t>(bitCount & 0xFF));
        bitCount >>= 8;
    }
    
    // 写入编码后的数据
    uint8_t currentByte = 0;
    int bitPos = 0;
    for (bool bit : bits) {
        if (bit) {
            currentByte |= (1 << bitPos);
        }
        bitPos++;
        if (bitPos == 8) {
            result.push_back(currentByte);
            currentByte = 0;
            bitPos = 0;
        }
    }
    if (bitPos > 0) {
        result.push_back(currentByte);
    }
    
    deleteTree(root);
    
    // 如果压缩后反而更大，返回原始数据
    if (result.size() >= data.size()) {
        std::vector<uint8_t> original;
        original.push_back(0);
        original.insert(original.end(), data.begin(), data.end());
        return original;
    }
    
    return result;
}

std::vector<uint8_t> HuffmanEncoder::decode(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        return {};
    }
    
    size_t pos = 0;
    uint8_t flag = data[pos++];
    
    if (flag == 0) {
        // 未压缩数据
        std::vector<uint8_t> result;
        result.insert(result.end(), data.begin() + pos, data.end());
        return result;
    }
    
    // 读取频率表大小
    uint8_t freqCount = data[pos++];
    
    // 读取频率表
    std::map<uint8_t, uint64_t> frequencies;
    for (uint8_t i = 0; i < freqCount && pos < data.size(); ++i) {
        uint8_t symbol = data[pos++];
        uint64_t freq = 0;
        for (int j = 0; j < 8 && pos < data.size(); ++j) {
            freq |= (static_cast<uint64_t>(data[pos++]) << (j * 8));
        }
        frequencies[symbol] = freq;
    }
    
    // 读取位长度
    uint32_t bitCount = 0;
    for (int i = 0; i < 4 && pos < data.size(); ++i) {
        bitCount |= (static_cast<uint32_t>(data[pos++]) << (i * 8));
    }
    
    // 读取编码数据
    std::vector<bool> bits;
    for (uint32_t i = 0; i < bitCount && pos < data.size(); ++i) {
        uint8_t byte = data[pos++];
        for (int j = 0; j < 8 && (i * 8 + j) < bitCount; ++j) {
            bits.push_back((byte >> j) & 1);
        }
    }
    
    // 重建Huffman树
    HuffmanNode* root = buildTree(frequencies);
    
    // 解码
    std::vector<uint8_t> result;
    HuffmanNode* current = root;
    for (bool bit : bits) {
        if (bit) {
            current = current->right;
        } else {
            current = current->left;
        }
        
        if (current->isLeaf()) {
            result.push_back(current->symbol);
            current = root;
        }
    }
    
    deleteTree(root);
    return result;
}

HuffmanNode* HuffmanEncoder::buildTree(const std::map<uint8_t, uint64_t>& frequencies) const {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeCompare> pq;
    
    // 创建叶子节点
    for (const auto& [symbol, freq] : frequencies) {
        auto* node = new HuffmanNode;
        node->symbol = symbol;
        node->frequency = freq;
        pq.push(node);
    }
    
    // 构建树
    while (pq.size() > 1) {
        auto* left = pq.top();
        pq.pop();
        auto* right = pq.top();
        pq.pop();
        
        auto* parent = new HuffmanNode;
        parent->frequency = left->frequency + right->frequency;
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    
    return pq.empty() ? nullptr : pq.top();
}

void HuffmanEncoder::buildCodeTable(HuffmanNode* root, std::map<uint8_t, std::vector<bool>>& codes,
                                   std::vector<bool> code) const {
    if (root == nullptr) {
        return;
    }
    
    if (root->isLeaf()) {
        codes[root->symbol] = code;
        return;
    }
    
    std::vector<bool> leftCode = code;
    leftCode.push_back(false);
    buildCodeTable(root->left, codes, leftCode);
    
    std::vector<bool> rightCode = code;
    rightCode.push_back(true);
    buildCodeTable(root->right, codes, rightCode);
}

void HuffmanEncoder::deleteTree(HuffmanNode* node) const {
    if (node == nullptr) {
        return;
    }
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

} // namespace mrn
