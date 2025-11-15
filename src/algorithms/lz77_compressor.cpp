#include "algorithms/lz77_compressor.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <zlib.h>

namespace mrn {

namespace {
constexpr int kCompressionLevel = Z_BEST_COMPRESSION;
}

LZ77CompressedBlock LZ77Compressor::compress(const std::vector<uint8_t>& data) const {
    LZ77CompressedBlock block;
    const uint64_t originalSize = data.size();
    if (originalSize == 0) {
        block.isCompressed = false;
        return block;
    }

    uLongf destinationSize = compressBound(static_cast<uLong>(originalSize));
    block.buffer.resize(destinationSize);

    int result = ::compress2(
        block.buffer.data(),
        &destinationSize,
        data.data(),
        static_cast<uLong>(originalSize),
        kCompressionLevel);

    if (result != Z_OK) {
        throw std::runtime_error("LZ77Compressor: compress2 failed with code " + std::to_string(result));
    }

    if (destinationSize >= originalSize) {
        block.isCompressed = false;
        block.buffer.assign(data.begin(), data.end());
        return block;
    }

    block.isCompressed = true;
    block.buffer.resize(destinationSize);
    return block;
}

std::vector<uint8_t> LZ77Compressor::decompress(const std::vector<uint8_t>& data,
                                                uint64_t expectedSize,
                                                bool isCompressed) const {
    if (!isCompressed) {
        if (data.size() != expectedSize) {
            throw std::runtime_error("LZ77Compressor: raw payload size mismatch");
        }
    return data;
}

    std::vector<uint8_t> output(expectedSize);
    if (expectedSize == 0) {
        return output;
    }

    uLongf destinationSize = static_cast<uLongf>(expectedSize);
    int result = ::uncompress(
        output.data(),
        &destinationSize,
        data.data(),
        static_cast<uLong>(data.size()));

    if (result != Z_OK || destinationSize != expectedSize) {
        throw std::runtime_error("LZ77Compressor: uncompress failed with code " + std::to_string(result));
    }

    return output;
}

} // namespace mrn
