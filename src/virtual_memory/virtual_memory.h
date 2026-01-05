#pragma once
#include <cstddef>
#include <unordered_map>

struct PageTableEntry { bool valid; size_t frame; size_t lastAccess; };

class VirtualMemory {
public:
    VirtualMemory();
    void init(size_t virt_size, size_t page_size, size_t phys_size);
    size_t translate(size_t vaddr); // returns phys addr, 0 on failure
    bool isInitialized() const { return pageSize > 0; }
    void stats();

private:
    size_t virtSize{0}, pageSize{0}, physSize{0};
    size_t numPages{0}, numFrames{0};
    std::unordered_map<size_t, PageTableEntry> pt; // vpn -> pte
    size_t pageFaults{0}, pageHits{0};
    size_t nextFrame{0};
    size_t accessCounter{0};
};
