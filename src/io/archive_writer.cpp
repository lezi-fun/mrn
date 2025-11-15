#include "io/archive_writer.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <stdexcept>

#include "io/file_io.h"
#include "utils/logger.h"

namespace mrn {

ArchiveWriter::ArchiveWriter(const std::string& filename, const CompressionPipeline& pipeline)
    : pipeline_(pipeline),
      threadPool_(std::make_unique<ThreadPool>()) {
    archiveStream_.open(filename, std::ios::binary | std::ios::trunc);
    if (!archiveStream_) {
        throw std::runtime_error("Failed to open archive: " + filename);
    }
    // 设置创建时间
    header_.creationTime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    writeHeader();
}

ArchiveWriter::~ArchiveWriter() {
    if (archiveStream_.is_open()) {
        finalize();
    }
}

bool ArchiveWriter::addFile(const std::string& filepath,
                            const std::string& archivePath,
                            const CompressionOptions& options) {
    auto data = FileIO::readFile(filepath);
    FileCompressionResult result;
    result.originalPath = filepath;
    result.archivePath = archivePath;
    result.result.compressedData = data;
    result.result.uncompressedSize = data.size();
    result.result.isCompressed = false;
    return addCompressedFile(result);
}

bool ArchiveWriter::addDirectory(const std::string& dirpath,
                                 const CompressionOptions& options) {
    (void)dirpath;
    (void)options;
    return true;
}

bool ArchiveWriter::addCompressedFile(const FileCompressionResult& result) {
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    FileEntryHeader entry{};
    std::strncpy(entry.filename, result.archivePath.c_str(), sizeof(entry.filename) - 1);
    entry.uncompressedSize = result.result.uncompressedSize;
    entry.compressedSize = result.result.compressedData.size();
    entry.fileOffset = currentOffset_;
    entry.compressionLevel = 0;
    entry.permissions = result.filePermissions;
    entry.checksum = result.checksum;
    if (result.result.isCompressed) {
        entry.flags |= MRN_FILE_FLAG_COMPRESSED;
    }

    archiveStream_.seekp(static_cast<std::streamoff>(currentOffset_));
    archiveStream_.write(reinterpret_cast<const char*>(result.result.compressedData.data()),
                         static_cast<std::streamsize>(result.result.compressedData.size()));

    currentOffset_ += result.result.compressedData.size();
    fileEntries_.push_back(entry);
    header_.fileCount++;
    header_.totalUncompressedSize += entry.uncompressedSize;
    header_.totalCompressedSize += entry.compressedSize;
    return true;
}

void ArchiveWriter::processFileBatch(const std::vector<std::string>& fileBatch,
                                     const CompressionOptions& options) {
    (void)fileBatch;
    (void)options;
    // 这个方法可以在未来用于批量处理文件
}

bool ArchiveWriter::finalize() {
    archiveStream_.seekp(0);
    writeHeader();

    const auto entriesOffset = currentOffset_;
    archiveStream_.seekp(static_cast<std::streamoff>(entriesOffset), std::ios::beg);
    for (const auto& entry : fileEntries_) {
        archiveStream_.write(reinterpret_cast<const char*>(&entry), sizeof(FileEntryHeader));
    }

    archiveStream_.close();
    Logger::instance().log(Logger::Level::Info, "Archive finalized with " + std::to_string(header_.fileCount) + " files.");
    return true;
}

void ArchiveWriter::writeHeader() {
    archiveStream_.write(reinterpret_cast<const char*>(&header_), sizeof(MRNArchiveHeader));
}

} // namespace mrn
