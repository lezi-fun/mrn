#pragma once

#include <cstdint>
#include <vector>

namespace mrn {

struct LZ77CompressedBlock {
    std::vector<uint8_t> buffer;
    bool isCompressed = true;
};

class LZ77Compressor {
public:
    LZ77CompressedBlock compress(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data,
                                    uint64_t expectedSize,
                                    bool isCompressed) const;
};

} // namespace mrn
