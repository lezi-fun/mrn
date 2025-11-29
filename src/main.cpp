#include <iostream>
#include <string>
#include <vector>
#include "core/compressor.h"
#include "core/plugin_manager.h"
#include "io/file_io.h"
#include "utils/logger.h"

using namespace mrn;

void printUsage() {
    std::cout << "MRN Compression Tool v2.0" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  mrn -c <input> -o <output> [options]" << std::endl;
    std::cout << "  mrn -d <archive> -o <output> [options]" << std::endl;
    std::cout << "  mrn -l <archive>" << std::endl;
    std::cout << "  mrn -t <archive>" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --compress     Compress files/directories" << std::endl;
    std::cout << "  -d, --decompress   Decompress archive" << std::endl;
    std::cout << "  -l, --list         List archive contents" << std::endl;
    std::cout << "  -t, --test         Test archive integrity" << std::endl;
    std::cout << "  -o, --output       Output file/directory" << std::endl;
    std::cout << "  -j, --threads      Number of threads (default: 1)" << std::endl;
    std::cout << "  -p, --preset       Compression preset (auto, text, binary, maximum, fast, store)" << std::endl;
    std::cout << "  -a, --algorithm    Compression algorithm (moverun, lz77, huffman)" << std::endl;
    std::cout << "  -v, --verbose      Verbose output" << std::endl;
    std::cout << "  --overwrite        Overwrite existing files" << std::endl;
    std::cout << "  -h, --help         Show this help" << std::endl;
}

void listAvailableAlgorithms() {
    auto& pm = PluginManager::getInstance();
    auto algorithms = pm.getAvailableAlgorithms();
    
    std::cout << "Available compression algorithms:" << std::endl;
    for (const auto& algo : algorithms) {
        if (auto algorithm = pm.getAlgorithm(algo)) {
            std::cout << "  " << algo << " - " << algorithm->getName() 
                      << " v" << algorithm->getVersion() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    // Initialize logger
    Logger::getInstance().setLevel(LogLevel::LOG_INFO);
    
    // Parse command line arguments
    std::vector<std::string> args(argv + 1, argv + argc);
    std::string operation;
    std::string input;
    std::string output;
    int threads = 1;
    std::string preset = "auto";
    bool verbose = false;
    bool overwrite = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            printUsage();
            return 0;
        } else if (args[i] == "-c" || args[i] == "--compress") {
            operation = "compress";
            if (i + 1 < args.size()) {
                input = args[++i];
            }
        } else if (args[i] == "-d" || args[i] == "--decompress") {
            operation = "decompress";
            if (i + 1 < args.size()) {
                input = args[++i];
            }
        } else if (args[i] == "-l" || args[i] == "--list") {
            operation = "list";
            if (i + 1 < args.size()) {
                input = args[++i];
            }
        } else if (args[i] == "-t" || args[i] == "--test") {
            operation = "test";
            if (i + 1 < args.size()) {
                input = args[++i];
            }
        } else if (args[i] == "-o" || args[i] == "--output") {
            if (i + 1 < args.size()) {
                output = args[++i];
            }
        } else if (args[i] == "-j" || args[i] == "--threads") {
            if (i + 1 < args.size()) {
                threads = std::stoi(args[++i]);
            }
        } else if (args[i] == "-p" || args[i] == "--preset") {
            if (i + 1 < args.size()) {
                preset = args[++i];
            }
        } else if (args[i] == "-v" || args[i] == "--verbose") {
            verbose = true;
            Logger::getInstance().setLevel(LogLevel::LOG_DEBUG);
        } else if (args[i] == "--overwrite") {
            overwrite = true;
        } else if (args[i] == "--list-algorithms") {
            listAvailableAlgorithms();
            return 0;
        } else if (input.empty()) {
            input = args[i];
        }
    }

    try {
        if (operation == "compress") {
            if (input.empty()) {
                std::cerr << "Error: Input must be specified for compression" << std::endl;
                return 1;
            }
            
            // Generate default output filename if not provided
            if (output.empty()) {
                if (FileIO::isDirectory(input)) {
                    // For directories: dirname -> dirname.mrn
                    std::string dirname = FileIO::getFilename(input);
                    if (dirname.empty()) {
                        // Handle case where input ends with path separator
                        dirname = FileIO::getFilename(input.substr(0, input.length() - 1));
                    }
                    output = input + ".mrn";
                } else {
                    // For files: filename.ext -> filename.ext.mrn
                    output = input + ".mrn";
                }
                MRN_INFO("Using default output: " << output);
            }

            ModularCompressor compressor;
            CompressionOptions options;
            options.threadCount = threads;
            options.verbose = verbose;
            options.overwrite = overwrite;

            CompressionPipeline pipeline;
            if (preset == "text") {
                pipeline = ModularCompressor::createTextPreset().pipeline;
            } else if (preset == "binary") {
                pipeline = ModularCompressor::createBinaryPreset().pipeline;
            } else if (preset == "maximum") {
                pipeline = ModularCompressor::createMaximumPreset().pipeline;
            } else if (preset == "fast") {
                pipeline = ModularCompressor::createFastPreset().pipeline;
            } else if (preset == "store") {
                pipeline = ModularCompressor::createStorePreset().pipeline;
            } else {
                // auto preset - will be determined per file
                pipeline.mainAlgorithm = "moverun";
            }

            MRN_INFO("Starting compression: " << input << " -> " << output);
            OverallCompressionResult result;
            if (FileIO::isDirectory(input)) {
                result = compressor.compressDirectory(input, output, pipeline, options);
            } else {
                result = compressor.compressFile(input, output, pipeline, options);
            }
            
            if (result.success) {
                MRN_INFO("Compression completed successfully");
                MRN_INFO("Files: " << result.fileCount);
                MRN_INFO("Uncompressed size: " << result.totalUncompressedSize << " bytes");
                MRN_INFO("Compressed size: " << result.totalCompressedSize << " bytes");
                MRN_INFO("Compression ratio: " << (result.totalCompressedSize * 100.0 / result.totalUncompressedSize) << "%");
            } else {
                MRN_ERROR("Compression failed: " << result.errorMessage);
                return 1;
            }

        } else if (operation == "decompress") {
            if (input.empty()) {
                std::cerr << "Error: Input archive must be specified for decompression" << std::endl;
                return 1;
            }
            
            // Generate default output directory if not provided
            if (output.empty()) {
                // Remove .mrn extension if present
                if (input.length() > 5 && input.substr(input.length() - 5) == ".mrn") {
                    output = input.substr(0, input.length() - 5);
                } else {
                    // If no .mrn extension, add _extracted
                    output = input + "_extracted";
                }
                MRN_INFO("Using default output: " << output);
            }

            ModularCompressor compressor;
            MRN_INFO("Starting decompression: " << input << " -> " << output);
            auto result = compressor.decompress(input, output);
            
            if (result.success) {
                MRN_INFO("Decompression completed successfully");
                MRN_INFO("Files extracted: " << result.fileCount);
            } else {
                MRN_ERROR("Decompression failed: " << result.errorMessage);
                return 1;
            }

        } else if (operation == "list") {
            // TODO: Implement archive listing
            std::cout << "Archive listing not yet implemented" << std::endl;
            return 1;

        } else if (operation == "test") {
            // TODO: Implement archive testing
            std::cout << "Archive testing not yet implemented" << std::endl;
            return 1;

        } else {
            std::cerr << "Error: Unknown operation or missing arguments" << std::endl;
            printUsage();
            return 1;
        }

    } catch (const std::exception& e) {
        MRN_ERROR("Exception: " << e.what());
        return 1;
    }

    return 0;
}
