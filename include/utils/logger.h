#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>
#include <memory>
#include <vector>

namespace mrn {

enum class LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
};

class Logger {
public:
    static Logger& getInstance();
    
    // 设置日志级别
    void setLevel(LogLevel level);
    
    // 设置输出流
    void setOutputStream(std::ostream& stream);
    
    // 设置日志文件
    void setLogFile(const std::string& filename);
    
    // 日志方法
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // 格式化日志
    void log(LogLevel level, const std::string& message);
    
    // 获取当前日志级别
    LogLevel getLevel() const { return level_; }
    
    // 启用/禁用日志
    void enable(bool enabled) { enabled_ = enabled; }
    
private:
    Logger();
    ~Logger();
    
    LogLevel level_;
    std::ostream* outputStream_;
    std::ofstream* fileStream_;
    bool enabled_;
    mutable std::mutex mutex_;
    
    // 获取级别名称
    std::string getLevelName(LogLevel level) const;
    
    // 获取当前时间戳
    std::string getTimestamp() const;
    
    // 写入日志
    void writeLog(LogLevel level, const std::string& message);
};

// 日志宏
#define MRN_LOG(level, message) \
    do { \
        std::ostringstream oss; \
        oss << message; \
        mrn::Logger::getInstance().log(level, oss.str()); \
    } while(0)

#define MRN_DEBUG(message) MRN_LOG(mrn::LogLevel::LOG_DEBUG, message)
#define MRN_INFO(message) MRN_LOG(mrn::LogLevel::LOG_INFO, message)
#define MRN_WARNING(message) MRN_LOG(mrn::LogLevel::LOG_WARNING, message)
#define MRN_ERROR(message) MRN_LOG(mrn::LogLevel::LOG_ERROR, message)
#define MRN_FATAL(message) MRN_LOG(mrn::LogLevel::LOG_FATAL, message)

// 流式日志
class LogStream {
public:
    LogStream(LogLevel level, const std::string& file, int line);
    ~LogStream();
    
    template<typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }
    
private:
    LogLevel level_;
    std::ostringstream stream_;
    std::string file_;
    int line_;
};

#define MRN_LOG_STREAM(level) \
    mrn::LogStream(level, __FILE__, __LINE__)

#define MRN_DEBUG_STREAM() MRN_LOG_STREAM(mrn::LogLevel::LOG_DEBUG)
#define MRN_INFO_STREAM() MRN_LOG_STREAM(mrn::LogLevel::LOG_INFO)
#define MRN_WARNING_STREAM() MRN_LOG_STREAM(mrn::LogLevel::LOG_WARNING)
#define MRN_ERROR_STREAM() MRN_LOG_STREAM(mrn::LogLevel::LOG_ERROR)
#define MRN_FATAL_STREAM() MRN_LOG_STREAM(mrn::LogLevel::LOG_FATAL)

} // namespace mrn
