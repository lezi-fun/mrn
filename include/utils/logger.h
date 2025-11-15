#pragma once

#include <iostream>
#include <mutex>
#include <string>

namespace mrn {

class Logger {
public:
    enum class Level { Debug, Info, Warn, Error };

    static Logger& instance();

    void log(Level level, const std::string& message);
    void setVerbose(bool verbose);

private:
    Logger() = default;
    std::mutex mutex_;
    bool verbose_ = false;
};

} // namespace mrn
