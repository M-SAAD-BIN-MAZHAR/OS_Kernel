#pragma once

#include <string>
#include <vector>

enum class ResourceType {
    PRINTER,
    SCANNER,
    MODEM,
    PLOTTER,
    DISK,
    MEMORY_BLOCK
};

struct Resource {
    int resourceId;
    ResourceType type;
    std::string name;
    int totalInstances;
    int availableInstances;
    std::vector<int> allocatedTo;  // process IDs

    Resource() : resourceId(-1), type(ResourceType::DISK), name(""), totalInstances(0), availableInstances(0) {}
    
    Resource(int id, ResourceType t, const std::string& n, int total)
        : resourceId(id), type(t), name(n), totalInstances(total), availableInstances(total) {}
};
