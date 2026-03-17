#include "Config.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

int Config::process_count = 0;
int Config::default_burst = 0;
int Config::default_priority = 0;

void Config::load(const std::string &file) {
    std::ifstream f(file);

    if (!f.is_open()) {
        throw std::runtime_error("Could not open config file!");
    }

    json data = json::parse(f);

    process_count = data["process_count"];
    default_burst = data["default_burst"];
    default_priority = data["default_priority"];
}