#include "core/compressor.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "core/archive_format.h"
#include "core/config.h"
#include "core/plugin_interface.h"
#include "io/archive_writer.h"
#include "io/directory_scanner.h"
#include "io/file_io.h"
#include "utils/logger.h"

namespace mrn {

namespace {
CompressParams buildParams(const CompressionPipeline& pipeline,
                           const CompressionOptions& options) {
    CompressParams params;
    params.level = options.compressionLevel;
    auto it = pipeline.algorithmConfigs.find(pipeline.mainAlgorithm);
    if (it != pipeline.algorithmConfigs.end()) {
        params.config = it->second;
    }
    return params;
}

uint32_t calculateChecksum(const std::vector<uint8_t>& data) {
    uint32_t checksum = 0;
    for (uint8_t byte : data) {
        checksum = (checksum << 1) ^ byte;
        if (checksum & 0x80000000) {
            checksum = (checksum << 1) ^ 0x04C11DB7; // CRC32 polynomial
        }
    }
    return checksum;
}

void logCompressionStats(const std::string& label,
                         uint64_t sourceSize,
                         uint64_t compressedSize) {
    double ratio = 0.0;
    if (sourceSize > 0) {
        ratio = (1.0 - static_cast<double>(compressedSize) / static_cast<double>(sourceSize)) * 100.0;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << label << " | 原始: " << sourceSize << " bytes, 压缩后: " << compressedSize
        << " bytes, 压缩率: " << ratio << "%";
    Logger::instance().log(Logger::Level::Info, oss.str());
}
} // namespace mrn

ModularCompressor::ModularCompressor(size_t threadCount)
    : pluginManager_(PluginManager::getInstance()),
      threadPool_(std::make_unique<ThreadPool>(threadCount > 0 ? threadCount : std::thread::hardware_concurrency())),
      directoryScanner_(std::make_unique<DirectoryScanner>()) {
    defaultPipeline_.mainAlgorithm = "moverun";
}

CompressionResult ModularCompressor::compressFile(const std::string& inputFile,
                                                  const std::string& outputFile,
                                                  const CompressionPipeline& pipeline,
                                                  const CompressionOptions& options) {
    // 使用归档格式，保持与目录压缩一致
    ArchiveWriter writer(outputFile, pipeline);
    
    std::filesystem::path inputPath(inputFile);
    std::string archivePath = inputPath.filename().string();
    
    auto result = compressSingleFile(inputFile, archivePath, pipeline, options);
    writer.addCompressedFile(result);
    writer.finalize();
    
    logCompressionStats(inputFile, result.result.uncompressedSize, result.result.compressedData.size());
    
    CompressionResult aggregated;
    aggregated.compressedData = result.result.compressedData;
    aggregated.uncompressedSize = result.result.uncompressedSize;
    return aggregated;
}

CompressionResult ModularCompressor::compressDirectory(const std::string& inputDir,
                                                       const std::string& outputFile,
                                                       const CompressionPipeline& pipeline,
                                                       const CompressionOptions& options) {
    ArchiveWriter writer(outputFile, pipeline);
    auto files = directoryScanner_->scanDirectory(inputDir, options.scanOptions);

    // 收集所有文件路径
    std::vector<DirectoryScanner::FileInfo> fileList;
    for (const auto& file : files) {
        if (!file.isDirectory) {
            fileList.push_back(file);
        }
    }

    // 多线程压缩，每个文件根据类型自动选择最佳预设
    std::vector<std::future<FileCompressionResult>> futures;
    bool useAutoPreset = (options.verbose || true); // 总是为每个文件检测最佳预设
    for (const auto& file : fileList) {
        futures.push_back(threadPool_->enqueue([this, file, pipeline, options, useAutoPreset] {
            CompressionPipeline filePipeline = pipeline;
            CompressionOptions fileOptions = options;
            
            // 为每个文件检测最佳预设（智能文件类型优化）
            if (useAutoPreset) {
                ConfigurationManager configMgr;
                CompressionPreset preset = configMgr.detectBestPreset(file.path);
                filePipeline = preset.pipeline;
                fileOptions = preset.options;
                fileOptions.verbose = options.verbose;
                fileOptions.overwrite = options.overwrite;
            }
            
            return compressSingleFile(file.path, file.relativePath, filePipeline, fileOptions);
        }));
    }

    // 收集结果并写入归档
    CompressionResult aggregated;
    uint64_t totalCompressedSize = 0;
    for (size_t i = 0; i < futures.size(); ++i) {
        auto result = futures[i].get();
        writer.addCompressedFile(result);
        aggregated.compressedData.insert(aggregated.compressedData.end(),
                                         result.result.compressedData.begin(),
                                         result.result.compressedData.end());
        aggregated.uncompressedSize += result.result.uncompressedSize;
        totalCompressedSize += result.result.compressedData.size();
        logCompressionStats(result.archivePath, result.result.uncompressedSize,
                            result.result.compressedData.size());
    }
    logCompressionStats("TOTAL", aggregated.uncompressedSize, totalCompressedSize);

    writer.finalize();
    return aggregated;
}

DecompressionResult ModularCompressor::decompress(const std::string& inputFile,
                                                  const std::string& outputPath) {
    std::ifstream archive(inputFile, std::ios::binary);
    if (!archive) {
        throw std::runtime_error("Failed to open archive: " + inputFile);
    }

    MRNArchiveHeader header{};
    archive.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!archive || !validateHeader(header)) {
        throw std::runtime_error("Invalid MRN archive: " + inputFile);
    }

    auto algorithm = pluginManager_.getAlgorithm(defaultPipeline_.mainAlgorithm);
    if (!algorithm) {
        throw std::runtime_error("Default algorithm not registered");
    }

    const auto entriesOffset =
        static_cast<std::streamoff>(sizeof(MRNArchiveHeader) + header.totalCompressedSize);

    std::filesystem::create_directories(outputPath);

    for (uint32_t i = 0; i < header.fileCount; ++i) {
        FileEntryHeader entry{};
        archive.seekg(entriesOffset + static_cast<std::streamoff>(i * sizeof(FileEntryHeader)),
                      std::ios::beg);
        archive.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!archive) {
            throw std::runtime_error("Failed to read file entry " + std::to_string(i));
        }

        std::vector<uint8_t> compressed(entry.compressedSize);
        archive.seekg(static_cast<std::streamoff>(entry.fileOffset), std::ios::beg);
        archive.read(reinterpret_cast<char*>(compressed.data()), entry.compressedSize);
        if (!archive) {
            throw std::runtime_error("Failed to read file data for entry " + std::to_string(i));
        }

        DecompressParams params;
        params.expectedSize = entry.uncompressedSize;
        params.dataIsCompressed = (entry.flags & MRN_FILE_FLAG_COMPRESSED) != 0;
        auto fileResult = algorithm->decompress(params, compressed);

        const std::filesystem::path outputFile = std::filesystem::path(outputPath) / entry.filename;
        std::filesystem::create_directories(outputFile.parent_path());
        FileIO::writeFile(outputFile.string(), fileResult.decompressedData);
        
        // 恢复文件权限
        if (entry.permissions != 0) {
            std::filesystem::permissions(outputFile, 
                static_cast<std::filesystem::perms>(entry.permissions));
        }
    }

    return {};
}

void ModularCompressor::listArchive(const std::string& inputFile) {
    std::ifstream archive(inputFile, std::ios::binary);
    if (!archive) {
        throw std::runtime_error("Failed to open archive: " + inputFile);
    }

    MRNArchiveHeader header{};
    archive.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!archive || !validateHeader(header)) {
        throw std::runtime_error("Invalid MRN archive: " + inputFile);
    }

    const auto entriesOffset =
        static_cast<std::streamoff>(sizeof(MRNArchiveHeader) + header.totalCompressedSize);

    std::cout << "MRN Archive: " << inputFile << std::endl;
    std::cout << "Version: " << static_cast<int>(header.version) << std::endl;
    std::cout << "Files: " << header.fileCount << std::endl;
    std::cout << "Total Size: " << header.totalUncompressedSize << " bytes (uncompressed)" << std::endl;
    std::cout << "Compressed Size: " << header.totalCompressedSize << " bytes" << std::endl;
    if (header.totalUncompressedSize > 0) {
        double ratio = (1.0 - static_cast<double>(header.totalCompressedSize) /
                               static_cast<double>(header.totalUncompressedSize)) * 100.0;
        std::cout << "Compression Ratio: " << std::fixed << std::setprecision(2) << ratio << "%" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Files:" << std::endl;
    std::cout << std::left << std::setw(40) << "Filename" 
              << std::right << std::setw(12) << "Original" 
              << std::setw(12) << "Compressed" 
              << std::setw(10) << "Ratio" << std::endl;
    std::cout << std::string(74, '-') << std::endl;

    for (uint32_t i = 0; i < header.fileCount; ++i) {
        FileEntryHeader entry{};
        archive.seekg(entriesOffset + static_cast<std::streamoff>(i * sizeof(FileEntryHeader)),
                      std::ios::beg);
        archive.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!archive) {
            throw std::runtime_error("Failed to read file entry " + std::to_string(i));
        }

        std::string filename(entry.filename);
        double ratio = entry.uncompressedSize > 0
                           ? (1.0 - static_cast<double>(entry.compressedSize) /
                                        static_cast<double>(entry.uncompressedSize)) * 100.0
                           : 0.0;

        std::cout << std::left << std::setw(40) << filename
                  << std::right << std::setw(12) << entry.uncompressedSize
                  << std::setw(12) << entry.compressedSize
                  << std::setw(9) << std::fixed << std::setprecision(2) << ratio << "%" << std::endl;
    }
}

bool ModularCompressor::testArchive(const std::string& inputFile) {
    std::ifstream archive(inputFile, std::ios::binary);
    if (!archive) {
        std::cerr << "Error: Failed to open archive: " << inputFile << std::endl;
        return false;
    }

    MRNArchiveHeader header{};
    archive.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!archive || !validateHeader(header)) {
        std::cerr << "Error: Invalid MRN archive header" << std::endl;
        return false;
    }

    auto algorithm = pluginManager_.getAlgorithm(defaultPipeline_.mainAlgorithm);
    if (!algorithm) {
        std::cerr << "Error: Default algorithm not registered" << std::endl;
        return false;
    }

    const auto entriesOffset =
        static_cast<std::streamoff>(sizeof(MRNArchiveHeader) + header.totalCompressedSize);

    bool allOk = true;
    std::cout << "Testing archive: " << inputFile << std::endl;
    std::cout << "Files: " << header.fileCount << std::endl;

    for (uint32_t i = 0; i < header.fileCount; ++i) {
        FileEntryHeader entry{};
        archive.seekg(entriesOffset + static_cast<std::streamoff>(i * sizeof(FileEntryHeader)),
                      std::ios::beg);
        archive.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!archive) {
            std::cerr << "Error: Failed to read file entry " << i << std::endl;
            allOk = false;
            continue;
        }

        std::vector<uint8_t> compressed(entry.compressedSize);
        archive.seekg(static_cast<std::streamoff>(entry.fileOffset), std::ios::beg);
        archive.read(reinterpret_cast<char*>(compressed.data()), entry.compressedSize);
        if (!archive || archive.gcount() != static_cast<std::streamsize>(entry.compressedSize)) {
            std::cerr << "Error: Failed to read file data for " << entry.filename << std::endl;
            allOk = false;
            continue;
        }

        try {
            DecompressParams params;
            params.expectedSize = entry.uncompressedSize;
            params.dataIsCompressed = (entry.flags & MRN_FILE_FLAG_COMPRESSED) != 0;
            auto fileResult = algorithm->decompress(params, compressed);

            if (fileResult.decompressedData.size() != entry.uncompressedSize) {
                std::cerr << "Error: Size mismatch for " << entry.filename 
                          << " (expected " << entry.uncompressedSize 
                          << ", got " << fileResult.decompressedData.size() << ")" << std::endl;
                allOk = false;
            } else {
                std::cout << "OK: " << entry.filename << std::endl;
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: Failed to decompress " << entry.filename << ": " << ex.what() << std::endl;
            allOk = false;
        }
    }

    if (allOk) {
        std::cout << std::endl << "All files OK. Archive is valid." << std::endl;
    } else {
        std::cout << std::endl << "Some files failed. Archive may be corrupted." << std::endl;
    }

    return allOk;
}

void ModularCompressor::setDefaultPipeline(const std::string& preset) {
    ConfigurationManager configMgr;
    CompressionPreset p;
    
    if (preset == "text") {
        p = CompressionPreset::createTextPreset();
    } else if (preset == "binary") {
        p = CompressionPreset::createBinaryPreset();
    } else if (preset == "maximum") {
        p = CompressionPreset::createMaximumPreset();
    } else if (preset == "fast") {
        p = CompressionPreset::createFastPreset();
    } else {
        p = configMgr.getPreset(preset);
    }
    
    defaultPipeline_ = p.pipeline;
}

CompressionPipeline ModularCompressor::createCustomPipeline(const std::vector<std::string>& steps) {
    CompressionPipeline pipeline = defaultPipeline_;
    if (!steps.empty()) {
        pipeline.preprocessors = steps;
    }
    return pipeline;
}

FileCompressionResult ModularCompressor::compressSingleFile(const std::string& filepath,
                                                            const std::string& archivePath,
                                                            const CompressionPipeline& pipeline,
                                                            const CompressionOptions& options) {
    auto data = FileIO::readFile(filepath);
    FileCompressionResult result;
    result.originalPath = filepath;
    result.archivePath = archivePath;
    
    // 如果设置了跳过压缩（如视频、已压缩文件），直接存储
    if (options.skipCompression) {
        result.result.compressedData = data;
        result.result.uncompressedSize = data.size();
        result.result.isCompressed = false;
    } else {
        auto algorithm = pluginManager_.getAlgorithm(pipeline.mainAlgorithm);
        if (!algorithm) {
            throw std::runtime_error("Algorithm not found: " + pipeline.mainAlgorithm);
        }
        result.result = algorithm->compress(buildParams(pipeline, options), data);
        
        // 如果压缩后反而更大，使用原始数据
        if (result.result.compressedData.size() >= data.size()) {
            result.result.compressedData = data;
            result.result.isCompressed = false;
        }
    }
    
    // 计算校验和（对压缩后的数据）
    result.checksum = calculateChecksum(result.result.compressedData);
    
    // 获取文件元数据
    if (std::filesystem::exists(filepath)) {
        auto fileStatus = std::filesystem::status(filepath);
        result.filePermissions = static_cast<uint16_t>(std::filesystem::perms::mask & fileStatus.permissions());
        auto fileTime = std::filesystem::last_write_time(filepath);
        result.modifiedTime = fileTime.time_since_epoch().count();
    }
    
    return result;
}

} // namespace mrn
