#include "utils/logger.h"

namespace mrn {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!verbose_ && level == Level::Debug) {
        return;
    }

    switch (level) {
        case Level::Debug:
            std::cout << "[DEBUG] ";
            break;
        case Level::Info:
            std::cout << "[INFO] ";
            break;
        case Level::Warn:
            std::cout << "[WARN] ";
            break;
        case Level::Error:
            std::cout << "[ERROR] ";
            break;
    }
    std::cout << message << std::endl;
}

void Logger::setVerbose(bool verbose) {
    verbose_ = verbose;
}

} // namespace mrn
