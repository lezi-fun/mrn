#include "core/compressor.h"
#include "core/plugin_manager.h"
#include "io/file_io.h"
#include "io/directory_scanner.h"
#include "io/archive_writer.h"
#include "utils/thread_pool.h"
#include "utils/logger.h"

namespace mrn {

ModularCompressor::ModularCompressor() 
    : pluginManager_(PluginManager::getInstance())
    , threadPool_(std::make_unique<ThreadPool>(1)) {
    // Initialize default pipeline
    defaultPipeline_.mainAlgorithm = "moverun";
}

ModularCompressor::~ModularCompressor() = default;

OverallCompressionResult ModularCompressor::compressDirectory(const std::string& inputDir,
                                                             const std::string& outputFile,
                                                             const CompressionPipeline& pipeline,
                                                             const CompressionOptions& options) {
    OverallCompressionResult result;
    
    try {
        // Check if input directory exists
        if (!FileIO::isDirectory(inputDir)) {
            result.errorMessage = "Input directory does not exist: " + inputDir;
            return result;
        }

        // Create archive writer
        ArchiveWriter writer(outputFile, pipeline);
        if (!writer.isValid()) {
            result.errorMessage = "Failed to create archive file: " + outputFile;
            return result;
        }

        // Add directory to archive
        if (!writer.addDirectory(inputDir, options)) {
            result.errorMessage = "Failed to add directory to archive";
            return result;
        }

        // Finalize archive
        if (!writer.finalize()) {
            result.errorMessage = "Failed to finalize archive";
            return result;
        }

        auto stats = writer.getStats();
        result.success = true;
        result.totalUncompressedSize = stats.totalUncompressedSize;
        result.totalCompressedSize = stats.totalCompressedSize;
        result.fileCount = stats.totalFiles;

    } catch (const std::exception& e) {
        result.errorMessage = e.what();
    }

    return result;
}

OverallCompressionResult ModularCompressor::compressFile(const std::string& inputFile,
                                                        const std::string& outputFile, 
                                                        const CompressionPipeline& pipeline,
                                                        const CompressionOptions& options) {
    OverallCompressionResult result;
    
    try {
        // Check if input file exists
        if (!FileIO::fileExists(inputFile)) {
            result.errorMessage = "Input file does not exist: " + inputFile;
            return result;
        }

        // Create archive writer
        ArchiveWriter writer(outputFile, pipeline);
        if (!writer.isValid()) {
            result.errorMessage = "Failed to create archive file: " + outputFile;
            return result;
        }

        // Add file to archive
        if (!writer.addFile(inputFile, FileIO::getFilename(inputFile), options)) {
            result.errorMessage = "Failed to add file to archive";
            return result;
        }

        // Finalize archive
        if (!writer.finalize()) {
            result.errorMessage = "Failed to finalize archive";
            return result;
        }

        auto stats = writer.getStats();
        result.success = true;
        result.totalUncompressedSize = stats.totalUncompressedSize;
        result.totalCompressedSize = stats.totalCompressedSize;
        result.fileCount = stats.totalFiles;

    } catch (const std::exception& e) {
        result.errorMessage = e.what();
    }

    return result;
}

OverallDecompressionResult ModularCompressor::decompress(const std::string& inputFile,
                                                        const std::string& outputPath) {
    OverallDecompressionResult result;
    
    try {
        // TODO: Implement decompression
        result.errorMessage = "Decompression not yet implemented";
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
    }

    return result;
}

void ModularCompressor::setDefaultPipeline(const std::string& preset) {
    if (preset == "text") {
        defaultPipeline_ = createTextPreset().pipeline;
    } else if (preset == "binary") {
        defaultPipeline_ = createBinaryPreset().pipeline;
    } else if (preset == "maximum") {
        defaultPipeline_ = createMaximumPreset().pipeline;
    } else if (preset == "fast") {
        defaultPipeline_ = createFastPreset().pipeline;
    } else if (preset == "store") {
        defaultPipeline_ = createStorePreset().pipeline;
    } else {
        defaultPipeline_.mainAlgorithm = "moverun";
    }
}

CompressionPipeline ModularCompressor::createCustomPipeline(const std::vector<std::string>& steps) {
    CompressionPipeline pipeline;
    for (const auto& step : steps) {
        if (step == "moverun" || step == "lz77" || step == "huffman") {
            pipeline.mainAlgorithm = step;
        } else {
            pipeline.preprocessors.push_back(step);
        }
    }
    return pipeline;
}

ModularCompressor::CompressionPreset ModularCompressor::createTextPreset() {
    CompressionPreset preset;
    preset.name = "text";
    preset.pipeline.mainAlgorithm = "moverun";
    preset.options.compressionLevel = 9;
    return preset;
}

ModularCompressor::CompressionPreset ModularCompressor::createBinaryPreset() {
    CompressionPreset preset;
    preset.name = "binary";
    preset.pipeline.mainAlgorithm = "lz77";
    preset.options.compressionLevel = 6;
    return preset;
}

ModularCompressor::CompressionPreset ModularCompressor::createMaximumPreset() {
    CompressionPreset preset;
    preset.name = "maximum";
    preset.pipeline.mainAlgorithm = "moverun";
    preset.pipeline.preprocessors = {"bwt"}; // Burrows-Wheeler Transform
    preset.options.compressionLevel = 9;
    return preset;
}

ModularCompressor::CompressionPreset ModularCompressor::createFastPreset() {
    CompressionPreset preset;
    preset.name = "fast";
    preset.pipeline.mainAlgorithm = "lz77";
    preset.options.compressionLevel = 1;
    return preset;
}

ModularCompressor::CompressionPreset ModularCompressor::createStorePreset() {
    CompressionPreset preset;
    preset.name = "store";
    preset.pipeline.mainAlgorithm = "store";
    preset.options.skipCompression = true;
    return preset;
}

std::string ModularCompressor::detectFileType(const std::string& filename) {
    auto extension = FileIO::getFileExtension(filename);
    
    // Video files
    static const std::vector<std::string> videoExtensions = {
        "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp", "ogv", "ts", "mts"
    };
    
    // Audio files
    static const std::vector<std::string> audioExtensions = {
        "mp3", "aac", "ogg", "wma", "flac", "m4a", "wav", "opus", "ape"
    };
    
    // Compressed archives
    static const std::vector<std::string> archiveExtensions = {
        "zip", "gz", "bz2", "xz", "7z", "rar", "tar", "cab", "iso", "dmg"
    };
    
    // Compressed images
    static const std::vector<std::string> compressedImageExtensions = {
        "jpg", "jpeg", "png", "gif", "webp"
    };
    
    // Text files
    static const std::vector<std::string> textExtensions = {
        "txt", "md", "rst", "log", "csv", "cpp", "hpp", "c", "h", "java", "py", "js", "ts", "go", 
        "rs", "swift", "kt", "scala", "php", "rb", "pl", "sh", "bash", "xml", "html", "htm", "css", 
        "scss", "json", "yaml", "yml", "toml", "ini", "conf", "config", "properties"
    };
    
    for (const auto& ext : videoExtensions) {
        if (extension == ext) return "video";
    }
    
    for (const auto& ext : audioExtensions) {
        if (extension == ext) return "audio";
    }
    
    for (const auto& ext : archiveExtensions) {
        if (extension == ext) return "archive";
    }
    
    for (const auto& ext : compressedImageExtensions) {
        if (extension == ext) return "compressed_image";
    }
    
    for (const auto& ext : textExtensions) {
        if (extension == ext) return "text";
    }
    
    return "binary";
}

ModularCompressor::CompressionPreset ModularCompressor::detectBestPreset(const std::string& filename) {
    auto fileType = detectFileType(filename);
    
    if (fileType == "video" || fileType == "audio" || fileType == "archive" || fileType == "compressed_image") {
        return createStorePreset();
    } else if (fileType == "text") {
        return createTextPreset();
    } else {
        return createBinaryPreset();
    }
}

// Private method implementations
FileCompressionResult ModularCompressor::compressSingleFile(const std::string& filepath,
                                                           const CompressionPipeline& pipeline,
                                                           const CompressionOptions& options) {
    FileCompressionResult result;
    result.filename = filepath;
    
    try {
        auto algorithm = pluginManager_.getAlgorithm(pipeline.mainAlgorithm);
        if (!algorithm) {
            result.errorMessage = "Algorithm not found: " + pipeline.mainAlgorithm;
            return result;
        }

        auto data = FileIO::readFile(filepath);
        if (data.empty()) {
            result.errorMessage = "Failed to read file: " + filepath;
            return result;
        }

        CompressParams params;
        params.mode = static_cast<CompressParams::Mode>(options.compressionLevel / 3);
        params.compressionLevel = options.compressionLevel;

        // Apply preprocessors
        std::vector<uint8_t> processedData = data;
        for (const auto& preprocessorName : pipeline.preprocessors) {
            auto preprocessor = pluginManager_.getPreprocessor(preprocessorName);
            if (preprocessor) {
                processedData = preprocessor->process(processedData);
            }
        }

        // Check if we should skip compression for certain file types
        if (options.skipCompression || FileIO::shouldSkipCompression(filepath)) {
            result.result.compressedData = processedData;
            result.result.uncompressedSize = processedData.size();
            result.result.compressedSize = processedData.size();
            result.result.isCompressed = false;
        } else {
            result.result = algorithm->compress(params, processedData);
            
            // If compression made it worse, use original data
            if (result.result.compressedData.size() >= processedData.size()) {
                result.result.compressedData = processedData;
                result.result.compressedSize = processedData.size();
                result.result.isCompressed = false;
            }
        }

        result.success = true;

    } catch (const std::exception& e) {
        result.errorMessage = e.what();
    }

    return result;
}

void ModularCompressor::compressDirectoryParallel(const std::string& inputDir,
                                                 const std::string& outputFile,
                                                 const CompressionPipeline& pipeline,
                                                 const CompressionOptions& options) {
    // TODO: Implement parallel compression
}

std::vector<std::vector<std::string>> ModularCompressor::groupFilesByType(const std::vector<std::string>& files) {
    std::map<std::string, std::vector<std::string>> groups;
    
    for (const auto& file : files) {
        auto type = detectFileType(file);
        groups[type].push_back(file);
    }
    
    std::vector<std::vector<std::string>> result;
    for (auto& group : groups) {
        result.push_back(std::move(group.second));
    }
    
    return result;
}

CompressionPipeline ModularCompressor::selectPipelineForFileGroup(const std::string& fileType, 
                                                                 const CompressionPipeline& basePipeline) {
    if (fileType == "video" || fileType == "audio" || fileType == "archive" || fileType == "compressed_image") {
        CompressionPipeline storePipeline = basePipeline;
        storePipeline.mainAlgorithm = "store";
        return storePipeline;
    }
    return basePipeline;
}

} // namespace mrn
