#include "algorithms/move_optimizer.h"

namespace mrn {

MoveOptimizerResult MoveOptimizer::optimize(const std::vector<uint8_t>& input,
                                            const std::string& mode) const {
    (void)mode;
    MoveOptimizerResult result;
    result.data = input;
    return result;
}

} // namespace mrn
