#pragma once

#include <string>

namespace mrn {
    class Architecture {
    public:
        static std::string detect() {
#if defined(__x86_64__) || defined(_M_X64)
            return "x86_64";
#elif defined(__aarch64__) || defined(__arm64__)
            return "arm64";
#elif defined(__arm__)
            return "arm";
#elif defined(__i386__) || defined(_M_IX86)
            return "x86";
#else
            return "unknown";
#endif
        }
        
        static std::string getPlatform() {
#ifdef __linux__
            return "linux";
#elif __APPLE__
            return "macos";
#elif _WIN32
            return "windows";
#else
            return "unknown";
#endif
        }
        
        static std::string getFullDescription() {
            return getPlatform() + "-" + detect();
        }
    };
}
