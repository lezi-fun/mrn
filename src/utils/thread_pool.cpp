#include "utils/thread_pool.h"
#include <stdexcept>

namespace mrn {

ThreadPool::ThreadPool(size_t numThreads) 
    : stop_(false), pendingTasks_(0) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

size_t ThreadPool::pendingTasks() const {
    return pendingTasks_;
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    condition_.wait(lock, [this]() { return tasks_.empty() && pendingTasks_ == 0; });
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();
    
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        task();
        pendingTasks_--;
    }
}

} // namespace mrn
