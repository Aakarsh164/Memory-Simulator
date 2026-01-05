#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include "allocator/allocator.h"
#include "memory/physical_memory.h"
#include "buddy/buddy.h"
#include "cache/cache.h"
#include "virtual_memory/virtual_memory.h"

int main() {
    Allocator alloc;
    PhysicalMemory pm;
    BuddyAllocator buddy;
    CacheLevel cacheL1;
    CacheLevel cacheL2;
    VirtualMemory vm;
    enum class ActiveAlloc { SIMPLE, BUDDY } active = ActiveAlloc::SIMPLE;
    std::string line;
    std::cout << "memsim> ";
    while (std::getline(std::cin, line)) {
        if (line.empty()) { std::cout << "memsim> "; continue; }
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        if (cmd == "exit" || cmd == "quit") break;
        else if (cmd == "init") {
            std::string what; iss >> what;
            if (what == "memory") {
                size_t n; iss >> n;
                pm.init(n);
                alloc.init(n);
                buddy.init(n);
                std::cout << "Initialized memory " << n << " bytes\n";
            }
        } else if (cmd == "set") {
            std::string what; iss >> what;
            if (what == "allocator") {
                std::string s; iss >> s;
                if (s == "buddy") { active = ActiveAlloc::BUDDY; buddy.init(pm.size()); }
                else { active = ActiveAlloc::SIMPLE; alloc.setStrategy(s); }
                std::cout << "Allocator set to " << s << "\n";
            }
            else if (what == "cache") {
                std::string level; iss >> level;
                if (level == "l1") {
                    size_t csize, bsize, assoc; std::string pol;
                    iss >> csize >> bsize >> assoc >> pol;
                    Replacement rp = (pol == "lru") ? Replacement::LRU : Replacement::FIFO;
                    cacheL1.init(csize, bsize, assoc, rp);
                    std::cout << "Initialized L1 cache: size=" << csize << " block=" << bsize << " assoc=" << assoc << " policy=" << pol << "\n";
                }
                else if (level == "l2") {
                    size_t csize, bsize, assoc; std::string pol;
                    iss >> csize >> bsize >> assoc >> pol;
                    Replacement rp = (pol == "lru") ? Replacement::LRU : Replacement::FIFO;
                    cacheL2.init(csize, bsize, assoc, rp);
                    std::cout << "Initialized L2 cache: size=" << csize << " block=" << bsize << " assoc=" << assoc << " policy=" << pol << "\n";
                }
            } else if (what == "vm") {
                size_t vs, ps, ph; iss >> vs >> ps >> ph;
                vm.init(vs, ps, ph);
                std::cout << "VM initialized\n";
            }
        } else if (cmd == "malloc") {
            size_t n; iss >> n;
            if (active == ActiveAlloc::SIMPLE) {
                int id = alloc.allocate(n);
                if (id != -1) std::cout << "Allocated block id=" << id << "\n";
                else std::cout << "Allocation failed\n";
            } else {
                int id = buddy.allocate(n);
                if (id != -1) std::cout << "Allocated buddy id=" << id << "\n";
                else std::cout << "Buddy allocation failed\n";
            }
        } else if (cmd == "free") {
            std::string token; iss >> token;
            if (token.empty()) { std::cout << "Usage: free <id|addr>\n"; }
            else {
                if (token.find("0x") == 0) {
                    unsigned long addr = std::stoul(token, nullptr, 0);
                    if (active == ActiveAlloc::SIMPLE) {
                        if (alloc.freeBlockByAddr(addr)) std::cout << "Block at " << token << " freed\n";
                        else std::cout << "Free failed\n";
                    } else {
                        std::cout << "Free by addr not supported for buddy\n";
                    }
                } else {
                    int id = std::stoi(token);
                    bool ok = false;
                    if (active == ActiveAlloc::SIMPLE) ok = alloc.freeBlockById(id);
                    else ok = buddy.freeBlockById(id);
                    if (ok) std::cout << "Block " << id << " freed\n";
                    else std::cout << "Free failed\n";
                }
            }
        } else if (cmd == "dump") {
            std::string what; iss >> what;
            if (what == "memory") alloc.dump();
            else if (what == "buddy") buddy.dump();
        } else if (cmd == "stats") {
            alloc.stats();
            buddy.stats();
            cacheL1.stats();
            cacheL2.stats();
            vm.stats();
        } else if (cmd == "access") {
            std::string token; iss >> token;
            if (!cacheL1.isInitialized()) {
                std::cout << "Error: L1 cache not initialized. Use: set cache l1 <size> <block> <assoc> <fifo|lru>\n";
            } else {
                size_t addr = std::stoul(token, nullptr, 0);
                size_t phys = vm.translate(addr);
                size_t paddr = phys ? phys : addr;
                
                CacheAccessLevel level = cacheL1.accessWithLevel(paddr, &cacheL2);
                std::string levelStr;
                size_t latency = 0;
                
                switch (level) {
                    case CacheAccessLevel::L1_HIT: 
                        levelStr = "L1_HIT"; 
                        latency = 1;
                        break;
                    case CacheAccessLevel::L2_HIT: 
                        levelStr = "L2_HIT"; 
                        latency = 5;
                        break;
                    case CacheAccessLevel::MEMORY: 
                        levelStr = "MEMORY"; 
                        latency = 100;
                        break;
                }
                
                std::cout << "Access " << token << " -> phys=0x" << std::hex << paddr << std::dec 
                          << " [" << levelStr << " | " << latency << " cycles]\n";
            }
        } else if (cmd == "vm") {
            std::string subcmd; iss >> subcmd;
            if (subcmd == "init") {
                size_t vs, ps, ph; iss >> vs >> ps >> ph;
                vm.init(vs, ps, ph);
                std::cout << "VM initialized: virt=" << vs << " page=" << ps << " phys=" << ph << "\n";
            } else if (subcmd == "access") {
                std::string token; iss >> token;
                size_t addr = std::stoul(token, nullptr, 0);
                size_t phys = vm.translate(addr);
                if (phys) std::cout << "VM: vaddr=" << token << " -> paddr=" << phys << "\n";
                else std::cout << "VM page fault\n";
            } else if (subcmd == "stats") {
                vm.stats();
            }
        } else {
            std::cout << "Unknown command: " << cmd << "\n";
            std::cout << "Commands: init memory <n>, set allocator <first_fit|best_fit|worst_fit>, malloc <n>, free <id|0xaddr>, dump memory, stats, access <addr>, exit\n";
        }
        std::cout << "memsim> ";
    }
    return 0;
}
