#include "Config.h"
#include "json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

int Config::process_count = 0;
int Config::default_burst = 0;
int Config::default_priority = 0;

void Config::load(const std::string &file) {
    std::ifstream f(file);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open config file");
    }

    const json data = json::parse(f);
    process_count = data.value("process_count", 3);
    default_burst = data.value("default_burst", 5);
    default_priority = data.value("default_priority", 1);
}
