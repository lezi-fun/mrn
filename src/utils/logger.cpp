#include "utils/logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace mrn {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : level_(LogLevel::LOG_INFO), outputStream_(&std::cout), fileStream_(nullptr), enabled_(true) {}

Logger::~Logger() {
    if (fileStream_) {
        fileStream_->close();
        delete fileStream_;
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::setOutputStream(std::ostream& stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    outputStream_ = &stream;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_) {
        fileStream_->close();
        delete fileStream_;
        fileStream_ = nullptr;
    }
    
    if (!filename.empty()) {
        fileStream_ = new std::ofstream(filename, std::ios::app);
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::LOG_DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::LOG_INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::LOG_WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::LOG_ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::LOG_FATAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!enabled_ || level < level_) {
        return;
    }
    writeLog(level, message);
}

std::string Logger::getLevelName(LogLevel level) const {
    switch (level) {
        case LogLevel::LOG_DEBUG: return "DEBUG";
        case LogLevel::LOG_INFO: return "INFO";
        case LogLevel::LOG_WARNING: return "WARNING";
        case LogLevel::LOG_ERROR: return "ERROR";
        case LogLevel::LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::writeLog(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::stringstream logLine;
    logLine << "[" << getTimestamp() << "] "
            << "[" << getLevelName(level) << "] "
            << message << std::endl;
    
    std::string logString = logLine.str();
    
    if (outputStream_) {
        *outputStream_ << logString;
        outputStream_->flush();
    }
    
    if (fileStream_ && fileStream_->is_open()) {
        *fileStream_ << logString;
        fileStream_->flush();
    }
}

// LogStream implementation
LogStream::LogStream(LogLevel level, const std::string& file, int line)
    : level_(level), file_(file), line_(line) {}

LogStream::~LogStream() {
    std::string message = stream_.str();
    if (!message.empty()) {
        Logger::getInstance().log(level_, message);
    }
}

} // namespace mrn
