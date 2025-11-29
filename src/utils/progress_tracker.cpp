#include "utils/progress_tracker.h"
#include <cmath>

namespace mrn {

ProgressTracker::ProgressTracker() 
    : current_(0), total_(0) {}

void ProgressTracker::setTotal(uint64_t total) {
    total_ = total;
    current_ = 0;
    notifyCallback();
}

void ProgressTracker::update(uint64_t increment, const std::string& message) {
    current_ += increment;
    if (!message.empty()) {
        std::lock_guard<std::mutex> lock(messageMutex_);
        currentMessage_ = message;
    }
    notifyCallback();
}

void ProgressTracker::setCurrent(uint64_t current, const std::string& message) {
    current_ = current;
    if (!message.empty()) {
        std::lock_guard<std::mutex> lock(messageMutex_);
        currentMessage_ = message;
    }
    notifyCallback();
}

double ProgressTracker::getPercentage() const {
    if (total_ == 0) return 0.0;
    return (static_cast<double>(current_) / total_) * 100.0;
}

void ProgressTracker::setCallback(const ProgressCallback& callback) {
    callback_ = callback;
}

void ProgressTracker::reset() {
    current_ = 0;
    total_ = 0;
    {
        std::lock_guard<std::mutex> lock(messageMutex_);
        currentMessage_.clear();
    }
    phaseStack_.clear();
    notifyCallback();
}

std::string ProgressTracker::getMessage() const {
    std::lock_guard<std::mutex> lock(messageMutex_);
    return currentMessage_;
}

void ProgressTracker::startPhase(const std::string& phaseName, uint64_t phaseTotal) {
    Phase phase{phaseName, phaseTotal, 0};
    phaseStack_.push_back(phase);
}

void ProgressTracker::endPhase() {
    if (!phaseStack_.empty()) {
        phaseStack_.pop_back();
    }
}

void ProgressTracker::notifyCallback() {
    if (callback_) {
        callback_(current_, total_, getMessage());
    }
}

// GlobalProgressManager implementation
GlobalProgressManager& GlobalProgressManager::getInstance() {
    static GlobalProgressManager instance;
    return instance;
}

std::shared_ptr<ProgressTracker> GlobalProgressManager::createTracker(const std::string& name) {
    std::lock_guard<std::mutex> lock(trackersMutex_);
    auto tracker = std::make_shared<ProgressTracker>();
    trackers_[name] = tracker;
    return tracker;
}

std::shared_ptr<ProgressTracker> GlobalProgressManager::getTracker(const std::string& name) {
    std::lock_guard<std::mutex> lock(trackersMutex_);
    auto it = trackers_.find(name);
    return it != trackers_.end() ? it->second : nullptr;
}

void GlobalProgressManager::removeTracker(const std::string& name) {
    std::lock_guard<std::mutex> lock(trackersMutex_);
    trackers_.erase(name);
}

void GlobalProgressManager::setGlobalCallback(const ProgressCallback& callback) {
    globalCallback_ = callback;
}

std::vector<GlobalProgressManager::TrackerStatus> GlobalProgressManager::getAllStatus() const {
    std::vector<TrackerStatus> status;
    std::lock_guard<std::mutex> lock(trackersMutex_);
    
    for (const auto& pair : trackers_) {
        auto tracker = pair.second;
        status.push_back({
            pair.first,
            tracker->getPercentage(),
            tracker->getMessage(),
            tracker->isComplete()
        });
    }
    
    return status;
}

} // namespace mrn
