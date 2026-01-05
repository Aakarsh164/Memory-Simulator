#include "buddy.h"
#include <cmath>
#include <iostream>
#include <algorithm>

BuddyAllocator::BuddyAllocator() {}

void BuddyAllocator::init(size_t total_size) {
    // require power of two
    size_t t = 1;
    int maxOrder = 0;
    while (t < total_size) { t <<= 1; ++maxOrder; }
    totalSize = t;
    minOrder = 0; // allow 1-byte min
    freeLists.clear();
    freeLists.resize(maxOrder+1);
    // add full block at top order
    freeLists[maxOrder].push_back(0);
    allocated.clear();
    nextId = 1;
}

int BuddyAllocator::order_for_size(size_t s) const {
    size_t t = 1; int order = 0;
    while (t < s) { t <<= 1; ++order; }
    return order;
}

void BuddyAllocator::split_to_order(size_t order) {
    // find a larger block and split it down to requested order
    for (size_t o = order + 1; o < freeLists.size(); ++o) {
        if (freeLists[o].empty()) continue;
        // take one block from order o and split down
        size_t addr = freeLists[o].back(); freeLists[o].pop_back();
        size_t curAddr = addr;
        for (int k = (int)o - 1; k >= (int)order; --k) {
            size_t half = (size_t(1) << k);
            // push the upper buddy into free list of order k
            freeLists[k].push_back(curAddr + half);
            // continue splitting the lower buddy (curAddr remains)
        }
        // ensure lower buddy ends up in requested order (one entry already pushed for each upper buddy)
        return;
    }
}

int BuddyAllocator::allocate(size_t req_size) {
    if (req_size == 0) return -1;
    int order = order_for_size(req_size);
    if ((size_t)order >= freeLists.size()) return -1;
    if (freeLists[order].empty()) split_to_order(order);
    if (freeLists[order].empty()) return -1;
    size_t addr = freeLists[order].back(); freeLists[order].pop_back();
    int id = nextId++;
    allocated[id] = {addr, (size_t)(1u<<order)};
    return id;
}

bool BuddyAllocator::freeBlockById(int id) {
    auto it = allocated.find(id);
    if (it == allocated.end()) return false;
    size_t addr = it->second.first;
    size_t sz = it->second.second;
    int order = order_for_size(sz);
    // attempt to coalesce with buddy blocks
    size_t curAddr = addr;
    while (order + 1 < (int)freeLists.size()) {
        size_t buddy = curAddr ^ (size_t(1) << order);
        // search for buddy in freeLists[order]
        auto &fl = freeLists[order];
        auto fit = std::find(fl.begin(), fl.end(), buddy);
        if (fit == fl.end()) break; // no buddy free
        // remove buddy from free list
        fl.erase(fit);
        // merge
        curAddr = std::min(curAddr, buddy);
        order += 1;
    }
    freeLists[order].push_back(curAddr);
    allocated.erase(it);
    return true;
}

void BuddyAllocator::dump() {
    std::cout << "Buddy allocator dump:\n";
    for (size_t o = 0; o < freeLists.size(); ++o) {
        std::cout << "order " << o << " (size=" << (1u<<o) << "): ";
        for (auto a: freeLists[o]) std::cout << a << ",";
        std::cout << "\n";
    }
    for (auto &p: allocated) {
        std::cout << "id=" << p.first << " addr=" << p.second.first << " size=" << p.second.second << "\n";
    }
}

void BuddyAllocator::stats() {
    size_t used = 0;
    for (auto &p: allocated) used += p.second.second;
    std::cout << "Total: " << totalSize << " Used: " << used << " Allocations: " << allocated.size() << "\n";
}
