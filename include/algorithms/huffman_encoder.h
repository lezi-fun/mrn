#pragma once

#include <cstdint>
#include <map>
#include <queue>
#include <vector>

namespace mrn {

struct HuffmanNode {
    uint8_t symbol = 0;
    uint64_t frequency = 0;
    HuffmanNode* left = nullptr;
    HuffmanNode* right = nullptr;
    
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

class HuffmanEncoder {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decode(const std::vector<uint8_t>& data) const;

private:
    struct NodeCompare {
        bool operator()(const HuffmanNode* a, const HuffmanNode* b) {
            return a->frequency > b->frequency;
        }
    };
    
    HuffmanNode* buildTree(const std::map<uint8_t, uint64_t>& frequencies) const;
    void buildCodeTable(HuffmanNode* root, std::map<uint8_t, std::vector<bool>>& codes,
                       std::vector<bool> code = {}) const;
    void deleteTree(HuffmanNode* node) const;
};

} // namespace mrn
