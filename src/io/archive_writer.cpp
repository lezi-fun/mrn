#include "io/archive_writer.h"
#include "io/file_io.h"
#include "io/directory_scanner.h"
#include "utils/thread_pool.h"
#include "utils/logger.h"
#include <ctime>

namespace mrn {

ArchiveWriter::ArchiveWriter(const std::string& filename, 
                             const CompressionPipeline& pipeline)
    : pipeline_(pipeline), currentOffset_(0) {
    archiveStream_.open(filename, std::ios::binary | std::ios::out);
    if (archiveStream_.is_open()) {
        initializeHeader();
        currentOffset_ = sizeof(MRNArchiveHeader);
    }
}

ArchiveWriter::~ArchiveWriter() {
    if (archiveStream_.is_open()) {
        archiveStream_.close();
    }
}

bool ArchiveWriter::addFile(const std::string& filepath, 
                           const std::string& archivePath,
                           const CompressionOptions& options) {
    if (!archiveStream_.is_open()) {
        return false;
    }
    
    FileEntryHeader entry;
    entry.setFilename(archivePath);
    entry.uncompressedSize = FileIO::getFileSize(filepath);
    entry.permissions = FileIO::getFilePermissions(filepath);
    entry.compressionLevel = options.compressionLevel;
    entry.fileOffset = currentOffset_;
    
    if (!writeFileData(filepath, archivePath, options, entry)) {
        return false;
    }
    
    fileEntries_.push_back(entry);
    header_.fileCount++;
    header_.totalUncompressedSize += entry.uncompressedSize;
    header_.totalCompressedSize += entry.compressedSize;
    
    return true;
}

bool ArchiveWriter::addDirectory(const std::string& dirpath,
                                const CompressionOptions& options) {
    DirectoryScanner scanner;
    ScanOptions scanOptions;
    scanOptions.recursive = true;
    
    auto files = scanner.scanDirectory(dirpath, scanOptions);
    bool success = true;
    
    for (const auto& file : files) {
        if (!file.isDirectory) {
            std::string archivePath = file.relativePath;
            if (!addFile(file.path, archivePath, options)) {
                success = false;
            }
        }
    }
    
    return success;
}

bool ArchiveWriter::addCompressedFile(const FileCompressionResult& result,
                                     const std::string& archivePath) {
    if (!archiveStream_.is_open()) {
        return false;
    }
    
    FileEntryHeader entry;
    entry.setFilename(archivePath);
    entry.uncompressedSize = result.result.uncompressedSize;
    entry.compressedSize = result.result.compressedData.size();
    entry.permissions = 0644; // Default permissions
    entry.compressionLevel = 0;
    entry.fileOffset = currentOffset_;
    entry.checksum = result.result.checksum;
    
    // Write compressed data
    archiveStream_.write(reinterpret_cast<const char*>(result.result.compressedData.data()),
                        result.result.compressedData.size());
    if (!archiveStream_.good()) {
        return false;
    }
    
    fileEntries_.push_back(entry);
    header_.fileCount++;
    header_.totalUncompressedSize += entry.uncompressedSize;
    header_.totalCompressedSize += entry.compressedSize;
    currentOffset_ += entry.compressedSize;
    
    return true;
}

bool ArchiveWriter::finalize() {
    if (!archiveStream_.is_open()) {
        return false;
    }
    
    // Write file table
    uint64_t fileTableOffset = currentOffset_;
    if (!writeFileTable()) {
        return false;
    }
    
    // Update header with file table offset
    header_.flags |= 0x01; // Mark that file table exists
    
    // Seek to beginning and write updated header
    archiveStream_.seekp(0);
    if (!writeHeader()) {
        return false;
    }
    
    // Update stats
    stats_.totalFiles = header_.fileCount;
    stats_.totalUncompressedSize = header_.totalUncompressedSize;
    stats_.totalCompressedSize = header_.totalCompressedSize;
    stats_.compressionRatio = header_.totalUncompressedSize > 0 
        ? (1.0 - static_cast<double>(header_.totalCompressedSize) / header_.totalUncompressedSize) * 100.0
        : 0.0;
    
    archiveStream_.close();
    return true;
}

void ArchiveWriter::initializeHeader() {
    header_.creationTime = getCurrentTimestamp();
    header_.fileCount = 0;
    header_.totalUncompressedSize = 0;
    header_.totalCompressedSize = 0;
    header_.compressionPipelineId = 1; // Default pipeline ID
    header_.flags = 0;
}

bool ArchiveWriter::writeHeader() {
    archiveStream_.write(reinterpret_cast<const char*>(&header_), sizeof(header_));
    return archiveStream_.good();
}

bool ArchiveWriter::writeFileTable() {
    // Write number of file entries
    uint32_t numEntries = fileEntries_.size();
    archiveStream_.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));
    
    // Write each file entry
    for (const auto& entry : fileEntries_) {
        archiveStream_.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    }
    
    return archiveStream_.good();
}

bool ArchiveWriter::writeFileData(const std::string& filepath,
                                 const std::string& archivePath,
                                 const CompressionOptions& options,
                                 FileEntryHeader& entry) {
    auto data = FileIO::readFile(filepath);
    if (data.empty()) {
        return false;
    }
    
    entry.checksum = calculateFileChecksum(data);
    
    // For now, just write the raw data
    // TODO: Implement actual compression
    archiveStream_.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!archiveStream_.good()) {
        return false;
    }
    
    entry.compressedSize = data.size();
    currentOffset_ += entry.compressedSize;
    
    return true;
}

FileCompressionResult ArchiveWriter::compressFile(const std::string& filepath,
                                                 const CompressionOptions& options) {
    FileCompressionResult result;
    result.filename = filepath;
    
    // TODO: Implement actual compression
    auto data = FileIO::readFile(filepath);
    result.result.compressedData = data;
    result.result.uncompressedSize = data.size();
    result.result.compressedSize = data.size();
    result.result.isCompressed = false;
    result.result.checksum = calculateFileChecksum(data);
    result.success = true;
    
    return result;
}

void ArchiveWriter::processFileBatch(const std::vector<std::string>& fileBatch,
                                    const CompressionOptions& options) {
    // TODO: Implement batch processing
}

uint32_t ArchiveWriter::calculateFileChecksum(const std::vector<uint8_t>& data) const {
    uint32_t checksum = 0;
    for (uint8_t byte : data) {
        checksum = (checksum << 5) + checksum + byte;
    }
    return checksum;
}

uint64_t ArchiveWriter::getCurrentTimestamp() const {
    return static_cast<uint64_t>(std::time(nullptr));
}

} // namespace mrn
