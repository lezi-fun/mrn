#include "io/file_io.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace mrn {

std::vector<uint8_t> FileIO::readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    const auto size = std::filesystem::file_size(path);
    std::vector<uint8_t> buffer(size);
    input.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
    return buffer;
}

void FileIO::writeFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Failed to write file: " + path);
    }
    output.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

} // namespace mrn
