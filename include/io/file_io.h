#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mrn {

class FileIO {
public:
    static std::vector<uint8_t> readFile(const std::string& path);
    static void writeFile(const std::string& path, const std::vector<uint8_t>& data);
};

} // namespace mrn
