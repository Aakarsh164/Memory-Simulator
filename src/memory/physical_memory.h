#pragma once
#include <cstddef>

class PhysicalMemory {
public:
    void init(size_t size);
    size_t size() const { return totalSize; }
private:
    size_t totalSize{0};
};
