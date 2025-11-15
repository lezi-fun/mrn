#include "utils/progress_tracker.h"

namespace mrn {

ProgressTracker::ProgressTracker(uint64_t total)
    : current_(0), total_(total) {}

void ProgressTracker::setTotal(uint64_t total) {
    total_.store(total, std::memory_order_relaxed);
}

void ProgressTracker::increment(uint64_t value) {
    auto current = current_.fetch_add(value, std::memory_order_relaxed) + value;
    auto total = total_.load(std::memory_order_relaxed);
    if (callback_ && total > 0) {
        callback_(current, total);
    }
}

void ProgressTracker::onProgress(Callback cb) {
    callback_ = std::move(cb);
}

} // namespace mrn
