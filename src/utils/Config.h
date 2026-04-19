#pragma once

#include <string>

class Config {
public:
    static void load(const std::string &file);

    static int process_count;
    static int default_burst;
    static int default_priority;
};
