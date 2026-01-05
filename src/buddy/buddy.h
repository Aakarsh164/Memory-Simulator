#pragma once
#include <cstddef>
#include <vector>
#include <unordered_map>

class BuddyAllocator {
public:
    BuddyAllocator();
    void init(size_t total_size);
    int allocate(size_t req_size); // returns id, -1 on failure
    bool freeBlockById(int id);
    void dump();
    void stats();

private:
    size_t totalSize{0};
    size_t minOrder{0};
    int nextId{1};
    std::vector<std::vector<size_t>> freeLists; // addresses per order
    std::unordered_map<int, std::pair<size_t,size_t>> allocated; // id -> (addr,size)

    int order_for_size(size_t s) const;
    void split_to_order(size_t order);
};
