#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct MemoryEvent {
    int tick = 0;
    std::string action;
    std::string status;
    int vpn = -1;
    int frame = -1;
    std::string detail;
};

struct MemoryMetrics {
    int totalAccesses = 0;
    int totalFaults = 0;
    int totalTlbHits = 0;
    double pageFaultRate = 0.0;
    double tlbHitRate = 0.0;
    double memoryUtilization = 0.0;
};

class MemorySimulator {
public:
    MemorySimulator();

    void setPolicy(const std::string &policyName);
    void configure(int pageSize, int frameCount, int tlbSize);
    void start();
    void stop();
    void tick();

    bool isRunning() const;
    bool hasSnapshot() const;
    std::string buildJSONSnapshot() const;

private:
    struct PageTableEntry {
        int processId = 1;
        int vpn = 0;
        int frame = -1;
        bool valid = false;
        int lastAccessTick = -1;
        int loadedTick = -1;
    };

    struct FrameInfo {
        int index = 0;
        bool occupied = false;
        int processId = 1;
        int vpn = -1;
        int loadedTick = -1;
        int lastAccessTick = -1;
    };

    struct TlbEntry {
        int vpn = -1;
        int frame = -1;
        int lastUsedTick = -1;
    };

    struct Translation {
        uint32_t logicalAddress = 0;
        int vpn = 0;
        int offset = 0;
        bool tlbHit = false;
        bool pageFault = false;
        int frame = -1;
        uint32_t physicalAddress = 0;
        bool valid = false;
    };

    void resetLocked();
    void processAccessLocked(uint32_t logicalAddress);
    int findTlbHitLocked(int vpn);
    void upsertTlbLocked(int vpn, int frame);
    void removeTlbEntryLocked(int vpn);
    int findFreeFrameLocked() const;
    int selectVictimFrameLocked(int incomingVpn) const;
    int nextUseDistanceLocked(int vpn, std::size_t fromIndex) const;
    void recalcMetricsLocked();
    void pushEventLocked(const MemoryEvent &event);
    std::vector<uint32_t> buildReferenceString(int pageSize) const;

    mutable std::mutex mutex;

    std::string policy = "FIFO";
    int pageSize = 4096;
    int frameCount = 16;
    int tlbSize = 8;
    int tickCount = 0;
    bool running = false;
    bool hasStarted = false;

    std::vector<uint32_t> referenceString;
    std::size_t currentIndex = 0;

    std::unordered_map<int, PageTableEntry> pageTable;
    std::vector<FrameInfo> frames;
    std::vector<TlbEntry> tlb;
    std::deque<MemoryEvent> recentEvents;
    MemoryMetrics metrics;
    Translation lastTranslation;
};
