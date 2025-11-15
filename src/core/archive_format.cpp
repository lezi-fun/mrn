#include "core/archive_format.h"

namespace mrn {

bool validateHeader(const MRNArchiveHeader& header) {
    return header.magic[0] == 'M' && header.magic[1] == 'R' && header.magic[2] == 'N';
}

} // namespace mrn
