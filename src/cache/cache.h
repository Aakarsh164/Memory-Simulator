#pragma once
#include <cstddef>
#include <vector>
#include <deque>

enum class Replacement { FIFO, LRU };
enum class CacheAccessLevel { L1_HIT, L2_HIT, MEMORY };

class CacheLevel {
public:
    CacheLevel();
    void init(size_t cache_size, size_t block_size, size_t assoc, Replacement r);
    bool access(size_t addr); // returns hit
    CacheAccessLevel accessWithLevel(size_t addr, CacheLevel* nextLevel = nullptr);
    size_t getAccessLatency(CacheAccessLevel level) const;
    void stats();
    bool isInitialized() const { return cacheSize > 0; }

private:
    size_t cacheSize{0};
    size_t blockSize{1};
    size_t associativity{1};
    Replacement policy{Replacement::FIFO};

    size_t sets{0};
    struct Line { size_t tag; size_t time; };
    std::vector<std::deque<Line>> setsVec;
    size_t accesses{0}, hits{0};
    size_t totalLatency{0};
};
