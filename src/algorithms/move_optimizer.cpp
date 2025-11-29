#include "algorithms/move_optimizer.h"
#include "algorithms/lz77_compressor.h"
#include "algorithms/huffman_encoder.h"
#include <algorithm>
#include <iostream>

namespace mrn {

MoveOptimizer::MoveOptimizer() : windowSize_(8192), minRunLength_(3) {}

MoveOptimizationResult MoveOptimizer::optimize(const std::vector<uint8_t>& data, 
                                              CompressParams::Mode mode) {
    MoveOptimizationResult result;
    result.originalSize = data.size();
    result.wasOptimized = false;
    
    if (data.empty()) {
        result.optimizedData = data;
        result.optimizedSize = 0;
        return result;
    }
    
    // Apply run-length encoding first
    auto rleData = applyRunLengthEncoding(data);
    
    // Then move repeated blocks to the front
    result.optimizedData = moveRepeatedBlocks(rleData, result.moveOffsets);
    result.optimizedSize = result.optimizedData.size();
    result.wasOptimized = result.optimizedSize < result.originalSize;
    
    return result;
}

std::vector<uint8_t> MoveOptimizer::reverseOptimize(const std::vector<uint8_t>& optimizedData,
                                                   const std::vector<uint32_t>& moveOffsets) {
    // TODO: Implement reverse optimization
    return optimizedData;
}

MoveOptimizer::Match MoveOptimizer::findBestMatch(const std::vector<uint8_t>& data, 
                                                 uint32_t currentPos, 
                                                 uint32_t windowStart) {
    Match bestMatch{0, 0};
    
    uint32_t maxSearchPos = std::min(currentPos, windowStart + windowSize_);
    
    for (uint32_t i = windowStart; i < maxSearchPos; i++) {
        uint32_t matchLen = 0;
        while (currentPos + matchLen < data.size() && 
               i + matchLen < currentPos && 
               data[i + matchLen] == data[currentPos + matchLen] &&
               matchLen < minRunLength_ + 255) {
            matchLen++;
        }
        
        if (matchLen >= minRunLength_ && matchLen > bestMatch.length) {
            bestMatch.position = i;
            bestMatch.length = matchLen;
        }
    }
    
    return bestMatch;
}

std::vector<uint8_t> MoveOptimizer::applyRunLengthEncoding(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    
    for (size_t i = 0; i < data.size();) {
        uint8_t current = data[i];
        uint32_t count = 1;
        
        // Count consecutive identical bytes
        while (i + count < data.size() && data[i + count] == current && count < 255) {
            count++;
        }
        
        if (count >= minRunLength_) {
            // Use RLE encoding: [0xFF, count, byte]
            result.push_back(0xFF);
            result.push_back(static_cast<uint8_t>(count));
            result.push_back(current);
            i += count;
        } else {
            // Copy literal bytes
            result.push_back(current);
            i++;
        }
    }
    
    return result;
}

std::vector<uint8_t> MoveOptimizer::reverseRunLengthEncoding(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    
    for (size_t i = 0; i < data.size();) {
        if (data[i] == 0xFF && i + 2 < data.size()) {
            // RLE sequence: expand it
            uint32_t count = data[i + 1];
            uint8_t value = data[i + 2];
            for (uint32_t j = 0; j < count; j++) {
                result.push_back(value);
            }
            i += 3;
        } else {
            // Literal byte
            result.push_back(data[i]);
            i++;
        }
    }
    
    return result;
}

std::vector<uint8_t> MoveOptimizer::moveRepeatedBlocks(const std::vector<uint8_t>& data,
                                                      std::vector<uint32_t>& moveOffsets) {
    std::vector<uint8_t> result;
    std::vector<bool> moved(data.size(), false);
    
    // Simple implementation: just return the original data for now
    // TODO: Implement actual block moving logic
    result = data;
    
    return result;
}

// MoveRunCompressor implementation
CompressionResult MoveRunCompressor::compress(const CompressParams& params,
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
        // Apply move optimization
        auto moveResult = moveOptimizer_.optimize(data, params.mode);
        
        // Apply LZ77 compression
        // TODO: Implement LZ77 compression
        
        // Apply Huffman encoding
        // TODO: Implement Huffman encoding
        
        // For now, just return the move-optimized data
        result.compressedData = moveResult.optimizedData;
        result.compressedSize = moveResult.optimizedSize;
        result.isCompressed = moveResult.wasOptimized;
        result.checksum = calculateChecksum(data);
        
    } catch (const std::exception& e) {
        // If compression fails, store the original data
        result.compressedData = data;
        result.compressedSize = data.size();
        result.isCompressed = false;
    }
    
    return result;
}

DecompressionResult MoveRunCompressor::decompress(const DecompressParams& params,
                                                 const std::vector<uint8_t>& data) {
    DecompressionResult result;
    
    try {
        // TODO: Implement decompression
        // For now, just return the data as-is
        result.decompressedData = data;
        result.originalSize = data.size();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    return result;
}

} // namespace mrn
