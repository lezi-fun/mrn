#pragma once

#include <cstdint>

namespace mrn {

#pragma pack(push, 1)
struct MRNArchiveHeader {
    char magic[3] = {'M', 'R', 'N'};
    uint8_t version = 2;
    uint16_t flags = 0;
    uint64_t creationTime = 0;
    uint32_t fileCount = 0;
    uint64_t totalUncompressedSize = 0;
    uint64_t totalCompressedSize = 0;
    uint8_t compressionPipelineId = 0;
    char reserved[16] = {0};
};

struct FileEntryHeader {
    char filename[256] = {0};
    uint64_t uncompressedSize = 0;
    uint64_t compressedSize = 0;
    uint64_t fileOffset = 0;
    uint32_t checksum = 0;
    uint16_t permissions = 0;
    uint8_t compressionLevel = 0;
    uint8_t flags = 0;
};
#pragma pack(pop)

constexpr uint8_t MRN_FILE_FLAG_COMPRESSED = 0x01;

bool validateHeader(const MRNArchiveHeader& header);

} // namespace mrn
