#pragma once

#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "core/archive_format.h"
#include "core/compressor.h"
#include "utils/thread_pool.h"

namespace mrn {

class ArchiveWriter {
public:
    ArchiveWriter(const std::string& filename, const CompressionPipeline& pipeline);
    ~ArchiveWriter();

    bool addFile(const std::string& filepath,
                 const std::string& archivePath,
                 const CompressionOptions& options);

    bool addDirectory(const std::string& dirpath,
                      const CompressionOptions& options);

    bool addCompressedFile(const FileCompressionResult& result);

    bool finalize();

    // 多线程文件处理
    void processFileBatch(const std::vector<std::string>& fileBatch,
                         const CompressionOptions& options);

private:
    std::ofstream archiveStream_;
    MRNArchiveHeader header_{};
    std::vector<FileEntryHeader> fileEntries_;
    CompressionPipeline pipeline_;
    uint64_t currentOffset_ = sizeof(MRNArchiveHeader);
    std::unique_ptr<ThreadPool> threadPool_;
    std::mutex writeMutex_;

    void writeHeader();
};

} // namespace mrn
