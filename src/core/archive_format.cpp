#include "core/archive_format.h"
#include <ctime>

namespace mrn {

// This file contains the implementation of archive format structures
// Most functionality is already implemented inline in the header
// Additional utility functions can be added here if needed

uint64_t getCurrentTimestamp() {
    return static_cast<uint64_t>(std::time(nullptr));
}

} // namespace mrn
