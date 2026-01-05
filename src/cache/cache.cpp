#include "cache.h"
#include <iostream>

CacheLevel::CacheLevel() {}

void CacheLevel::init(size_t cache_size, size_t block_size, size_t assoc, Replacement r) {
    cacheSize = cache_size; blockSize = block_size; associativity = assoc; policy = r;
    sets = (cacheSize / blockSize) / associativity;
    if (sets == 0) sets = 1;
    setsVec.clear(); setsVec.resize(sets);
    accesses = hits = totalLatency = 0;
}

bool CacheLevel::access(size_t addr) {
    if (!isInitialized()) return false;
    if (sets == 0) return false;
    
    accesses++;
    size_t block = addr / blockSize;
    size_t set = block % sets;
    size_t tag = block / sets;
    auto &dq = setsVec[set];
    for (size_t i = 0; i < dq.size(); ++i) {
        if (dq[i].tag == tag) {
            hits++;
            if (policy == Replacement::LRU) {
                Line ln = dq[i];
                ln.time = accesses;
                dq.erase(dq.begin() + i);
                dq.push_back(ln);
            }
            return true;
        }
    }
    // miss
    if (dq.size() < associativity) dq.push_back(Line{tag, accesses});
    else {
        if (policy == Replacement::FIFO) {
            dq.pop_front(); dq.push_back(Line{tag, accesses});
        } else { // LRU
            size_t idx = 0; size_t minT = dq[0].time;
            for (size_t i = 1; i < dq.size(); ++i) if (dq[i].time < minT) { minT = dq[i].time; idx = i; }
            dq.erase(dq.begin() + idx);
            dq.push_back(Line{tag, accesses});
        }
    }
    return false;
}

CacheAccessLevel CacheLevel::accessWithLevel(size_t addr, CacheLevel* nextLevel) {
    if (!isInitialized()) return CacheAccessLevel::MEMORY;
    
    bool hit = access(addr);
    if (hit) {
        totalLatency += 1;
        return CacheAccessLevel::L1_HIT;  // This cache hit it
    }
    
    // Miss in this level, check next level
    if (nextLevel && nextLevel->isInitialized()) {
        CacheAccessLevel nextResult = nextLevel->accessWithLevel(addr, nullptr);
        // nextResult will be what L2 returned (which includes its own latency)
        if (nextResult == CacheAccessLevel::L1_HIT) {  // L2 hit
            totalLatency += 5; // L2 hit from perspective of L1
            return CacheAccessLevel::L2_HIT;
        } else if (nextResult == CacheAccessLevel::MEMORY) {
            // L2 also missed, went to memory
            totalLatency += 100;
            return CacheAccessLevel::MEMORY;
        }
    }
    
    // No next level or uninitialized, memory access
    totalLatency += 100;
    return CacheAccessLevel::MEMORY;
}

size_t CacheLevel::getAccessLatency(CacheAccessLevel level) const {
    switch (level) {
        case CacheAccessLevel::L1_HIT: return 1;
        case CacheAccessLevel::L2_HIT: return 5;
        case CacheAccessLevel::MEMORY: return 100;
        default: return 100;
    }
}

void CacheLevel::stats() {
    double hitRatio = accesses > 0 ? (double)hits / accesses : 0.0;
    double avgLatency = accesses > 0 ? (double)totalLatency / accesses : 0.0;
    std::cout << "Cache accesses=" << accesses << " hits=" << hits << " hit_ratio=" << hitRatio 
              << " total_latency=" << totalLatency << " avg_latency=" << avgLatency << "\n";
}
