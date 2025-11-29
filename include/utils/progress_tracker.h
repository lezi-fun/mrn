#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <atomic>
#include <mutex>
#include <vector>
#include <map>
#include <memory>

namespace mrn {

// 进度回调函数类型
using ProgressCallback = std::function<void(uint64_t current, uint64_t total, const std::string& message)>;

class ProgressTracker {
public:
    ProgressTracker();
    ~ProgressTracker() = default;
    
    // 设置总进度
    void setTotal(uint64_t total);
    
    // 更新进度
    void update(uint64_t increment, const std::string& message = "");
    void setCurrent(uint64_t current, const std::string& message = "");
    
    // 获取当前进度
    uint64_t getCurrent() const { return current_; }
    uint64_t getTotal() const { return total_; }
    double getPercentage() const;
    
    // 设置回调函数
    void setCallback(const ProgressCallback& callback);
    
    // 重置进度
    void reset();
    
    // 检查是否完成
    bool isComplete() const { return current_ >= total_; }
    
    // 获取当前消息
    std::string getMessage() const;
    
    // 进度阶段管理
    void startPhase(const std::string& phaseName, uint64_t phaseTotal);
    void endPhase();
    
private:
    std::atomic<uint64_t> current_;
    std::atomic<uint64_t> total_;
    mutable std::mutex messageMutex_;
    std::string currentMessage_;
    ProgressCallback callback_;
    
    struct Phase {
        std::string name;
        uint64_t total;
        uint64_t completed;
    };
    
    std::vector<Phase> phaseStack_;
    
    // 通知回调
    void notifyCallback();
};

// 全局进度管理器
class GlobalProgressManager {
public:
    static GlobalProgressManager& getInstance();
    
    // 创建新的进度跟踪器
    std::shared_ptr<ProgressTracker> createTracker(const std::string& name);
    
    // 获取进度跟踪器
    std::shared_ptr<ProgressTracker> getTracker(const std::string& name);
    
    // 移除进度跟踪器
    void removeTracker(const std::string& name);
    
    // 设置全局回调
    void setGlobalCallback(const ProgressCallback& callback);
    
    // 获取所有跟踪器状态
    struct TrackerStatus {
        std::string name;
        double percentage;
        std::string message;
        bool isComplete;
    };
    
    std::vector<TrackerStatus> getAllStatus() const;
    
private:
    GlobalProgressManager() = default;
    ~GlobalProgressManager() = default;
    
    mutable std::mutex trackersMutex_;
    std::map<std::string, std::shared_ptr<ProgressTracker>> trackers_;
    ProgressCallback globalCallback_;
};

} // namespace mrn
