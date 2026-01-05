#include "allocator.h"
#include <iostream>
#include <limits>

Allocator::Allocator() {}

void Allocator::init(size_t total_size) {
    totalSize = total_size;
    blocks.clear();
    blocks.push_back(Block{0, 0, total_size, 0, true});
    nextId = 1;
    allocations = 0;
    failures = 0;
}

void Allocator::setStrategy(const std::string &s) {
    strategy = s;
}

int Allocator::find_block_first(size_t req) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= req) return (int)i;
    }
    return -1;
}

int Allocator::find_block_best(size_t req) {
    int best = -1;
    size_t bestSize = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= req && blocks[i].size < bestSize) {
            best = (int)i; bestSize = blocks[i].size;
        }
    }
    return best;
}

int Allocator::find_block_worst(size_t req) {
    int worst = -1;
    size_t worstSize = 0;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].free && blocks[i].size >= req && blocks[i].size > worstSize) {
            worst = (int)i; worstSize = blocks[i].size;
        }
    }
    return worst;
}

void Allocator::split_block(int idx, size_t req) {
    Block &b = blocks[idx];
    if (b.size == req) return;
    Block newb{0, b.addr + req, b.size - req, 0, true};
    b.size = req;
    blocks.insert(blocks.begin() + idx + 1, newb);
}

int Allocator::allocate(size_t req_size) {
    int idx = -1;
    if (strategy == "first_fit") idx = find_block_first(req_size);
    else if (strategy == "best_fit") idx = find_block_best(req_size);
    else if (strategy == "worst_fit") idx = find_block_worst(req_size);
    else idx = find_block_first(req_size);

    if (idx == -1) { failures++; return -1; }
    split_block(idx, req_size);
    Block &b = blocks[idx];
    b.free = false;
    b.id = nextId++;
    b.requested = req_size;
    allocations++;
    return b.id;
}

bool Allocator::freeBlockById(int id) {
    for (auto &b : blocks) {
        if (!b.free && b.id == id) {
            b.free = true;
            b.id = 0;
            coalesce();
            return true;
        }
    }
    return false;
}

bool Allocator::freeBlockByAddr(size_t addr) {
    for (auto &b : blocks) {
        if (!b.free && b.addr == addr) {
            b.free = true;
            b.id = 0;
            coalesce();
            return true;
        }
    }
    return false;
}

void Allocator::coalesce() {
    std::vector<Block> out;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (!out.empty() && out.back().free && blocks[i].free) {
            out.back().size += blocks[i].size;
        } else {
            out.push_back(blocks[i]);
        }
    }
    blocks.swap(out);
}

void Allocator::dump() {
    for (auto &b : blocks) {
        char buf[64];
        sprintf(buf, "[0x%04zX - 0x%04zX] ", b.addr, b.addr + b.size - 1);
        std::cout << buf;
        if (b.free) std::cout << "FREE\n";
        else std::cout << "USED (id=" << b.id << ") size=" << b.size << "\n";
    }
}

void Allocator::stats() {
    size_t used = 0;
    size_t freeMem = 0;
    size_t largestFree = 0;
    size_t internalFrag = 0;
    size_t allocatedCount = 0;
    for (auto &b : blocks) {
        if (b.free) { freeMem += b.size; if (b.size > largestFree) largestFree = b.size; }
        else { used += b.size; internalFrag += (b.size - b.requested); allocatedCount++; }
    }
    double externalFrag = 0.0;
    if (freeMem > 0) externalFrag = 100.0 * (1.0 - (double)largestFree / (double)freeMem);
    double util = 100.0 * (double)used / (double)totalSize;
    size_t attempts = allocations + failures;
    double successRate = attempts ? 100.0 * (double)allocations / (double)attempts : 100.0;

    std::cout << "Total memory: " << totalSize << "\n";
    std::cout << "Used memory: " << used << "\n";
    std::cout << "Free memory: " << freeMem << "\n";
    std::cout << "Internal fragmentation: " << internalFrag << " bytes\n";
    std::cout << "External fragmentation: " << externalFrag << "%\n";
    std::cout << "Utilization: " << util << "%\n";
    std::cout << "Allocation attempts: " << attempts << " successes: " << allocations << " failures: " << failures << " success rate: " << successRate << "%\n";
}
