#include "virtual_memory.h"
#include <iostream>

VirtualMemory::VirtualMemory() {}

void VirtualMemory::init(size_t virt_size, size_t page_size, size_t phys_size) {
    virtSize = virt_size; pageSize = page_size; physSize = phys_size;
    numPages = virtSize / pageSize; numFrames = physSize / pageSize;
    pt.clear(); pageFaults = 0; pageHits = 0; nextFrame = 0;
}

size_t VirtualMemory::translate(size_t vaddr) {
    if (!isInitialized()) return 0;  // Return 0 if not initialized
    
    size_t vpn = vaddr / pageSize; size_t offset = vaddr % pageSize;
    accessCounter++;
    auto it = pt.find(vpn);
    if (it == pt.end() || !it->second.valid) {
        // page fault
        pageFaults++;
        if (nextFrame >= numFrames) {
            // evict LRU
            if (pt.empty()) return 0;
            size_t victimVpn = 0; size_t minAccess = (size_t)-1;
            for (auto &p : pt) {
                if (p.second.valid && p.second.lastAccess < minAccess) {
                    minAccess = p.second.lastAccess; victimVpn = p.first;
                }
            }
            size_t victimFrame = pt[victimVpn].frame;
            pt.erase(victimVpn);
            pt[vpn] = {true, victimFrame, accessCounter};
            return victimFrame * pageSize + offset;
        } else {
            pt[vpn] = {true, nextFrame++, accessCounter};
            return pt[vpn].frame * pageSize + offset;
        }
    } else {
        pageHits++;
        it->second.lastAccess = accessCounter;
        return it->second.frame * pageSize + offset;
    }
}

void VirtualMemory::stats() {
    std::cout << "Page hits=" << pageHits << " faults=" << pageFaults << "\n";
}
