#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "core/compressor.h"
#include "core/config.h"
#include "utils/logger.h"

using namespace mrn;

struct CommandLineOptions {
    enum Operation { COMPRESS, DECOMPRESS, LIST, TEST };

    Operation operation = COMPRESS;
    std::vector<std::string> inputPaths;
    std::string outputPath;
    int threadCount = 1;
    std::string preset = "auto";
    std::string algorithm = "moverun";
    int compressionLevel = 6;
    bool verbose = false;
    bool overwrite = false;
    bool preservePaths = true;
};

CommandLineOptions parseArguments(int argc, char** argv) {
    CommandLineOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--compress") {
            opts.operation = CommandLineOptions::COMPRESS;
        } else if (arg == "-d" || arg == "--decompress") {
            opts.operation = CommandLineOptions::DECOMPRESS;
        } else if (arg == "-l" || arg == "--list") {
            opts.operation = CommandLineOptions::LIST;
        } else if (arg == "-t" || arg == "--test") {
            opts.operation = CommandLineOptions::TEST;
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            opts.outputPath = argv[++i];
        } else if ((arg == "-j" || arg == "--threads") && i + 1 < argc) {
            opts.threadCount = std::stoi(argv[++i]);
        } else if (arg == "--preset" && i + 1 < argc) {
            opts.preset = argv[++i];
        } else if (arg == "--algorithm" && i + 1 < argc) {
            opts.algorithm = argv[++i];
        } else if ((arg == "-v" || arg == "--verbose")) {
            opts.verbose = true;
        } else if (arg == "--overwrite") {
            opts.overwrite = true;
        } else if (arg == "--preserve-paths") {
            opts.preservePaths = true;
        } else if (arg == "--no-preserve-paths") {
            opts.preservePaths = false;
        } else if (arg.front() == '-') {
            throw std::runtime_error("Unknown option: " + arg);
        } else {
            opts.inputPaths.push_back(arg);
        }
    }
    return opts;
}

int main(int argc, char** argv) {
    try {
        auto options = parseArguments(argc, argv);
        Logger::instance().setVerbose(options.verbose);

        ConfigurationManager configMgr;
        ModularCompressor compressor(options.threadCount > 0 ? options.threadCount : 0);
        
        CompressionPipeline pipeline;
        CompressionOptions compOptions;
        
        // 使用预设系统
        if (options.preset == "auto" && !options.inputPaths.empty()) {
            CompressionPreset preset = configMgr.detectBestPreset(options.inputPaths.front());
            pipeline = preset.pipeline;
            compOptions = preset.options;
        } else if (options.preset != "auto") {
            CompressionPreset preset = configMgr.getPreset(options.preset);
            pipeline = preset.pipeline;
            compOptions = preset.options;
        } else {
            pipeline.mainAlgorithm = options.algorithm;
            compOptions.compressionLevel = options.compressionLevel;
        }
        
        compOptions.verbose = options.verbose;
        compOptions.overwrite = options.overwrite;

        switch (options.operation) {
            case CommandLineOptions::COMPRESS:
                if (options.inputPaths.empty()) {
                    throw std::runtime_error("No input path specified");
                }
                if (options.outputPath.empty()) {
                    throw std::runtime_error("No output path specified");
                }
                {
                    std::filesystem::path inputPath(options.inputPaths.front());
                    
                    // 如果是auto预设且是单文件，根据文件类型重新检测预设
                    if (options.preset == "auto" && std::filesystem::is_regular_file(inputPath)) {
                        CompressionPreset preset = configMgr.detectBestPreset(options.inputPaths.front());
                        pipeline = preset.pipeline;
                        compOptions = preset.options;
                        compOptions.verbose = options.verbose;
                        compOptions.overwrite = options.overwrite;
                    }
                    
                    if (std::filesystem::is_regular_file(inputPath)) {
                        // 单文件压缩
                        compressor.compressFile(options.inputPaths.front(), options.outputPath,
                                                pipeline, compOptions);
                    } else if (std::filesystem::is_directory(inputPath)) {
                        // 目录压缩
                        compressor.compressDirectory(options.inputPaths.front(), options.outputPath,
                                                    pipeline, compOptions);
                    } else {
                        throw std::runtime_error("Input path is neither a file nor a directory: " + options.inputPaths.front());
                    }
                }
                break;
            case CommandLineOptions::DECOMPRESS:
                if (options.inputPaths.empty()) {
                    throw std::runtime_error("No archive specified");
                }
                if (options.outputPath.empty()) {
                    throw std::runtime_error("No output directory specified");
                }
                compressor.decompress(options.inputPaths.front(), options.outputPath);
                break;
            case CommandLineOptions::LIST:
                if (options.inputPaths.empty()) {
                    throw std::runtime_error("No archive specified");
                }
                compressor.listArchive(options.inputPaths.front());
                break;
            case CommandLineOptions::TEST:
                if (options.inputPaths.empty()) {
                    throw std::runtime_error("No archive specified");
                }
                {
                    bool result = compressor.testArchive(options.inputPaths.front());
                    return result ? 0 : 1;
                }
                break;
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
