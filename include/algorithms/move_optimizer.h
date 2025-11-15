#pragma once

#include <string>
#include <vector>

namespace mrn {

struct MoveOptimizerResult {
    std::vector<uint8_t> data;
};

class MoveOptimizer {
public:
    MoveOptimizerResult optimize(const std::vector<uint8_t>& input,
                                 const std::string& mode) const;
};

} // namespace mrn
