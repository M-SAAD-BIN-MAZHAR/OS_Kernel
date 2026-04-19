#include "MemorySimulator.h"
#include "../utils/json.hpp"
#include <algorithm>
#include <limits>

using json = nlohmann::json;

namespace {
constexpr int kMaxMemoryEvents = 80;
}

MemorySimulator::MemorySimulator() {
    std::lock_guard<std::mutex> lock(mutex);
    resetLocked();
}

void MemorySimulator::setPolicy(const std::string &policyName) {
    std::lock_guard<std::mutex> lock(mutex);
    if (policyName == "FIFO" || policyName == "LRU" || policyName == "Optimal") {
        policy = policyName;
    }
}

void MemorySimulator::configure(int newPageSize, int newFrameCount, int newTlbSize) {
    std::lock_guard<std::mutex> lock(mutex);
    pageSize = std::max(256, newPageSize);
    frameCount = std::max(1, newFrameCount);
    tlbSize = std::max(1, newTlbSize);
    resetLocked();
}

void MemorySimulator::start() {
    std::lock_guard<std::mutex> lock(mutex);
    resetLocked();
    running = true;
    hasStarted = true;
    pushEventLocked({tickCount, "simulation", "started", -1, -1, "Memory simulation started"});
}

void MemorySimulator::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    running = false;
}

void MemorySimulator::tick() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!running) {
        return;
    }

    if (currentIndex >= referenceString.size()) {
        running = false;
        pushEventLocked({tickCount, "simulation", "complete", -1, -1, "Reference string completed"});
        return;
    }

    ++tickCount;
    processAccessLocked(referenceString[currentIndex]);
    ++currentIndex;

    if (currentIndex >= referenceString.size()) {
        running = false;
        pushEventLocked({tickCount, "simulation", "complete", -1, -1, "Reference string completed"});
    }
}

bool MemorySimulator::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex);
    return running;
}

bool MemorySimulator::hasSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex);
    return hasStarted;
}

std::string MemorySimulator::buildJSONSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex);

    json root;
    root["type"] = "memory";
    root["policy"] = policy;
    root["tick"] = tickCount;
    root["running"] = running;

    root["currentAccess"] = {
        {"accessIndex", static_cast<int>(currentIndex)},
        {"logicalAddress", lastTranslation.logicalAddress},
    };

    root["translation"] = {
        {"logicalAddress", lastTranslation.logicalAddress},
        {"vpn", lastTranslation.vpn},
        {"offset", lastTranslation.offset},
        {"tlbHit", lastTranslation.tlbHit},
        {"pageFault", lastTranslation.pageFault},
        {"frame", lastTranslation.frame},
        {"physicalAddress", lastTranslation.physicalAddress},
        {"valid", lastTranslation.valid},
    };

    json framesJson = json::array();
    for (const auto &frame : frames) {
        framesJson.push_back({
            {"index", frame.index},
            {"occupied", frame.occupied},
            {"processId", frame.processId},
            {"vpn", frame.vpn},
            {"loadedTick", frame.loadedTick},
            {"lastAccessTick", frame.lastAccessTick},
        });
    }
    root["frames"] = framesJson;

    std::vector<PageTableEntry> entries;
    entries.reserve(pageTable.size());
    for (const auto &[vpn, entry] : pageTable) {
        (void)vpn;
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const PageTableEntry &a, const PageTableEntry &b) { return a.vpn < b.vpn; });

    json pageTableJson = json::array();
    for (const auto &entry : entries) {
        pageTableJson.push_back({
            {"processId", entry.processId},
            {"vpn", entry.vpn},
            {"frame", entry.frame},
            {"valid", entry.valid},
            {"loadedTick", entry.loadedTick},
            {"lastAccessTick", entry.lastAccessTick},
        });
    }
    root["pageTable"] = pageTableJson;

    json eventsJson = json::array();
    for (const auto &event : recentEvents) {
        eventsJson.push_back({
            {"tick", event.tick},
            {"action", event.action},
            {"status", event.status},
            {"vpn", event.vpn},
            {"frame", event.frame},
            {"detail", event.detail},
        });
    }
    root["events"] = eventsJson;

    root["metrics"] = {
        {"pageFaultRate", metrics.pageFaultRate},
        {"tlbHitRate", metrics.tlbHitRate},
        {"memoryUtilization", metrics.memoryUtilization},
        {"totalAccesses", metrics.totalAccesses},
        {"totalFaults", metrics.totalFaults},
        {"totalTlbHits", metrics.totalTlbHits},
    };

    return root.dump();
}

void MemorySimulator::resetLocked() {
    tickCount = 0;
    running = false;
    hasStarted = false;
    currentIndex = 0;
    referenceString = buildReferenceString(pageSize);
    pageTable.clear();
    frames.clear();
    frames.reserve(static_cast<std::size_t>(frameCount));
    for (int i = 0; i < frameCount; ++i) {
        FrameInfo frame;
        frame.index = i;
        frames.push_back(frame);
    }
    tlb.clear();
    recentEvents.clear();
    metrics = {};
    lastTranslation = {};
}

void MemorySimulator::processAccessLocked(uint32_t logicalAddress) {
    const int vpn = static_cast<int>(logicalAddress / static_cast<uint32_t>(pageSize));
    const int offset = static_cast<int>(logicalAddress % static_cast<uint32_t>(pageSize));

    Translation translation;
    translation.logicalAddress = logicalAddress;
    translation.vpn = vpn;
    translation.offset = offset;

    ++metrics.totalAccesses;

    int frameIndex = findTlbHitLocked(vpn);
    if (frameIndex >= 0) {
        translation.tlbHit = true;
        translation.pageFault = false;
        translation.frame = frameIndex;
        translation.physicalAddress = static_cast<uint32_t>(frameIndex * pageSize + offset);
        translation.valid = true;
        ++metrics.totalTlbHits;
        frames[frameIndex].lastAccessTick = tickCount;
        pageTable[vpn].lastAccessTick = tickCount;
        pushEventLocked({tickCount, "tlb", "hit", vpn, frameIndex, "TLB resolved translation"});
        recalcMetricsLocked();
        lastTranslation = translation;
        return;
    }

    auto pageIt = pageTable.find(vpn);
    const bool pageHit = pageIt != pageTable.end() && pageIt->second.valid;
    if (!pageHit) {
        translation.pageFault = true;
        ++metrics.totalFaults;
        int targetFrame = findFreeFrameLocked();
        if (targetFrame < 0) {
            targetFrame = selectVictimFrameLocked(vpn);
            const int victimVpn = frames[targetFrame].vpn;
            if (victimVpn >= 0) {
                pageTable[victimVpn].valid = false;
                pageTable[victimVpn].frame = -1;
                removeTlbEntryLocked(victimVpn);
                pushEventLocked({tickCount, "replace", policy, victimVpn, targetFrame, "Evicted VPN " + std::to_string(victimVpn)});
            }
        }

        FrameInfo &frame = frames[targetFrame];
        frame.occupied = true;
        frame.processId = 1;
        frame.vpn = vpn;
        frame.loadedTick = tickCount;
        frame.lastAccessTick = tickCount;

        PageTableEntry &entry = pageTable[vpn];
        entry.processId = 1;
        entry.vpn = vpn;
        entry.frame = targetFrame;
        entry.valid = true;
        entry.loadedTick = tickCount;
        entry.lastAccessTick = tickCount;

        translation.frame = targetFrame;
        pushEventLocked({tickCount, "page", "fault", vpn, targetFrame, "Loaded VPN into frame"});
    } else {
        translation.frame = pageIt->second.frame;
        frames[translation.frame].lastAccessTick = tickCount;
        pageIt->second.lastAccessTick = tickCount;
        pushEventLocked({tickCount, "page", "hit", vpn, translation.frame, "Page table resolved translation"});
    }

    upsertTlbLocked(vpn, translation.frame);
    translation.physicalAddress = static_cast<uint32_t>(translation.frame * pageSize + offset);
    translation.valid = true;
    recalcMetricsLocked();
    lastTranslation = translation;
}

int MemorySimulator::findTlbHitLocked(int vpn) {
    for (auto &entry : tlb) {
        if (entry.vpn == vpn) {
            entry.lastUsedTick = tickCount;
            return entry.frame;
        }
    }
    return -1;
}

void MemorySimulator::upsertTlbLocked(int vpn, int frame) {
    for (auto &entry : tlb) {
        if (entry.vpn == vpn) {
            entry.frame = frame;
            entry.lastUsedTick = tickCount;
            return;
        }
    }

    if (static_cast<int>(tlb.size()) >= tlbSize) {
        auto victim = std::min_element(tlb.begin(), tlb.end(),
                                       [](const TlbEntry &a, const TlbEntry &b) { return a.lastUsedTick < b.lastUsedTick; });
        if (victim != tlb.end()) {
            *victim = {vpn, frame, tickCount};
            return;
        }
    }

    tlb.push_back({vpn, frame, tickCount});
}

void MemorySimulator::removeTlbEntryLocked(int vpn) {
    tlb.erase(std::remove_if(tlb.begin(), tlb.end(), [vpn](const TlbEntry &entry) { return entry.vpn == vpn; }), tlb.end());
}

int MemorySimulator::findFreeFrameLocked() const {
    for (const auto &frame : frames) {
        if (!frame.occupied) {
            return frame.index;
        }
    }
    return -1;
}

int MemorySimulator::selectVictimFrameLocked(int incomingVpn) const {
    if (policy == "LRU") {
        auto victim = std::min_element(frames.begin(), frames.end(),
                                       [](const FrameInfo &a, const FrameInfo &b) { return a.lastAccessTick < b.lastAccessTick; });
        return victim->index;
    }

    if (policy == "Optimal") {
        int bestFrame = frames.front().index;
        int farthestUse = -1;
        for (const auto &frame : frames) {
            const int distance = nextUseDistanceLocked(frame.vpn, currentIndex + 1);
            if (distance > farthestUse) {
                farthestUse = distance;
                bestFrame = frame.index;
            }
        }
        (void)incomingVpn;
        return bestFrame;
    }

    auto victim = std::min_element(frames.begin(), frames.end(),
                                   [](const FrameInfo &a, const FrameInfo &b) { return a.loadedTick < b.loadedTick; });
    return victim->index;
}

int MemorySimulator::nextUseDistanceLocked(int vpn, std::size_t fromIndex) const {
    for (std::size_t i = fromIndex; i < referenceString.size(); ++i) {
        const int futureVpn = static_cast<int>(referenceString[i] / static_cast<uint32_t>(pageSize));
        if (futureVpn == vpn) {
            return static_cast<int>(i - fromIndex);
        }
    }
    return std::numeric_limits<int>::max();
}

void MemorySimulator::recalcMetricsLocked() {
    if (metrics.totalAccesses > 0) {
        metrics.pageFaultRate = (static_cast<double>(metrics.totalFaults) / metrics.totalAccesses) * 100.0;
        metrics.tlbHitRate = (static_cast<double>(metrics.totalTlbHits) / metrics.totalAccesses) * 100.0;
    } else {
        metrics.pageFaultRate = 0.0;
        metrics.tlbHitRate = 0.0;
    }

    int usedFrames = 0;
    for (const auto &frame : frames) {
        if (frame.occupied) {
            ++usedFrames;
        }
    }
    metrics.memoryUtilization = frameCount > 0 ? (static_cast<double>(usedFrames) / frameCount) * 100.0 : 0.0;
}

void MemorySimulator::pushEventLocked(const MemoryEvent &event) {
    recentEvents.push_back(event);
    while (static_cast<int>(recentEvents.size()) > kMaxMemoryEvents) {
        recentEvents.pop_front();
    }
}

std::vector<uint32_t> MemorySimulator::buildReferenceString(int pageSizeValue) const {
    const std::vector<int> pages = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5, 6, 2, 1, 2, 3, 7, 3, 2, 1, 6, 5, 4, 3, 2};
    std::vector<uint32_t> accesses;
    accesses.reserve(pages.size());
    for (std::size_t i = 0; i < pages.size(); ++i) {
        const uint32_t offset = static_cast<uint32_t>((i * 97U) % static_cast<unsigned>(pageSizeValue));
        accesses.push_back(static_cast<uint32_t>(pages[i] * pageSizeValue) + offset);
    }
    return accesses;
}
