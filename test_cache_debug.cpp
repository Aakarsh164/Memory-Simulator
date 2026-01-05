#include <iostream>
#include "src/cache/cache.h"

int main() {
    CacheLevel cache;
    cache.init(512, 64, 4, Replacement::LRU);
    std::cout << "Cache initialized\n";
    std::cout << "Calling accessWithLevel\n";
    CacheAccessLevel level = cache.accessWithLevel(0, nullptr);
    std::cout << "Access returned\n";
    return 0;
}
