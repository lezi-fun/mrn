#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

namespace mrn {

class ProgressTracker {
public:
    using Callback = std::function<void(uint64_t current, uint64_t total)>;

    explicit ProgressTracker(uint64_t total = 0);

    void setTotal(uint64_t total);
    void increment(uint64_t value = 1);
    void onProgress(Callback cb);

private:
    std::atomic<uint64_t> current_;
    std::atomic<uint64_t> total_;
    Callback callback_;
};

} // namespace mrn
