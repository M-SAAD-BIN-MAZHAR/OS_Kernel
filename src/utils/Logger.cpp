#include "Logger.h"
#include <iostream>
#include <string>
using namespace std;
#include <ctime>

std::string getTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
string timeStr(dt);

// remove newline
timeStr.pop_back();

return timeStr;
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