#pragma once
#include <cstddef>
#include <vector>
#include <string>

struct Block {
    int id; // 0 means free
    size_t addr; // start address
    size_t size; // block size in bytes
    size_t requested; // requested size (for internal fragmentation)
    bool free;
};

class Allocator {
public:
    Allocator();
    void init(size_t total_size);
    void setStrategy(const std::string &s);
    int allocate(size_t req_size); // returns block id, -1 on failure
    bool freeBlockById(int id);
    bool freeBlockByAddr(size_t addr);
    void dump();
    void stats();

private:
    std::vector<Block> blocks;
    size_t totalSize{0};
    std::string strategy{"first_fit"};
    int nextId{1};

    int find_block_first(size_t req);
    int find_block_best(size_t req);
    int find_block_worst(size_t req);
    void split_block(int idx, size_t req);
    void coalesce();
    // metrics
    size_t allocations{0};
    size_t failures{0};
};
