#include "Logger.h"
#include <ctime>
#include <iostream>
#include <string>

namespace {
std::string getTime() {
    const std::time_t now = std::time(nullptr);
    std::string timeStr = std::ctime(&now);
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    return timeStr;
}
}

void Logger::info(const std::string &msg) {
    std::cout << "[INFO] " << getTime() << " : " << msg << std::endl;
}

void Logger::warn(const std::string &msg) {
    std::cout << "[WARN] " << getTime() << " : " << msg << std::endl;
}

void Logger::error(const std::string &msg) {
    std::cout << "[ERROR] " << getTime() << " : " << msg << std::endl;
}
