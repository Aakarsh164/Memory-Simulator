# Memory Management Simulator

A comprehensive OS memory management simulator implementing dynamic allocation strategies, multilevel cache simulation, buddy allocation, and virtual memory with page translation.

## Quick Start

### Build

Windows (MSYS/MinGW) or WSL:

```
g++ -std=c++17 -O2 -Iinclude -o bin/memsim src/main.cpp src/allocator/allocator.cpp src/buddy/buddy.cpp src/cache/cache.cpp src/memory/physical_memory.cpp src/virtual_memory/virtual_memory.cpp
```

Or simply:
```
make
```

### Run

```
bin/memsim
```

## Features Implemented

### 1. **Dynamic Memory Allocation**
- First-Fit strategy: allocate at first sufficient block
- Best-Fit strategy: allocate at smallest sufficient block
- Worst-Fit strategy: allocate at largest block
- Automatic block splitting and coalescence
- Fragmentation tracking (internal and external)

### 2. **Buddy Allocator**
- Power-of-two block sizes
- Free list management per order
- Coalescing on deallocation
- Efficient memory reuse

### 3. **Multilevel Cache Simulation**
- **L1 Cache**: Configurable size, block size, associativity, and replacement policy
- **L2 Cache**: Independent configuration with different parameters
- Set-associative lookup with tag/index/offset breakdown
- LRU (Least Recently Used) and FIFO replacement policies
- Hit/miss detection with latency tracking
- Automatic miss penalty propagation to lower levels
- Access time reporting: L1 hit = 1 cycle, L2 hit = 5 cycles, Memory = 100 cycles

### 4. **Virtual Memory**
- Virtual address translation to physical addresses
- Page table management with LRU page replacement
- Page fault handling and frame allocation
- Hit/miss statistics

## Interactive Commands

```
# Memory Initialization
init memory <size>                 # Initialize physical memory (bytes)

# Allocator Management
set allocator <strategy>           # Choose: first_fit, best_fit, worst_fit, buddy
malloc <size>                      # Allocate memory block
free <id|0xaddr>                   # Free by block ID or physical address
dump memory                        # Display current memory layout
dump buddy                         # Display buddy allocator free lists

# Cache Configuration
set cache l1 <size> <block> <assoc> <policy>  # Initialize L1 cache
set cache l2 <size> <block> <assoc> <policy>  # Initialize L2 cache
  where: size = cache size (bytes)
         block = block size (bytes)
         assoc = associativity (number of ways)
         policy = fifo | lru

# Memory Access
access <addr>                      # Perform cache access (shows L1/L2/MEMORY with latency)

# Virtual Memory
vm init <virt_size> <page_size> <phys_size>  # Initialize VM
vm access <addr>                              # Translate virtual address
vm stats                                      # Show VM statistics

# Statistics & Status
stats                              # Display all statistics
exit / quit                        # Exit simulator
```

## Example Session

```
memsim> init memory 2048
Initialized memory 2048 bytes
memsim> set allocator first_fit
Allocator set to first_fit
memsim> malloc 256
Allocated block id=1
memsim> malloc 128
Allocated block id=2
memsim> dump memory
[0x0000 - 0x00FF] USED (id=1) size=256
[0x0100 - 0x017F] USED (id=2) size=128
[0x0180 - 0x07FF] FREE
memsim> set cache l1 512 64 4 lru
Initialized L1 cache: size=512 block=64 assoc=4 policy=lru
memsim> set cache l2 1024 64 8 lru
Initialized L2 cache: size=1024 block=64 assoc=8 policy=lru
memsim> access 0x0000
Access 0x0000 -> phys=0x0 [MEMORY | 100 cycles]
memsim> access 0x0000
Access 0x0000 -> phys=0x0 [L1_HIT | 1 cycles]
memsim> stats
Total memory: 2048
Used memory: 384
Free memory: 1664
...
Cache accesses=2 hits=1 hit_ratio=0.5 total_latency=101 avg_latency=50.5
```

## Files

- `src/allocator` — Dynamic allocation with multiple strategies
- `src/buddy` — Buddy allocator implementation
- `src/cache` — Multilevel cache (L1, L2) with LRU/FIFO
- `src/memory` — Physical memory management
- `src/virtual_memory` — Page translation and VM management
- `docs/design.md` — Detailed design documentation
- `TESTING.md` — Test cases and examples
- `*.txt` — Sample input workloads and output logs

## Key Design Decisions

### Cache Architecture
- Set-associative caches with configurable associativity
- Tag = block/sets, Index = block % sets, Offset = addr % blockSize
- LRU policy maintained via timestamp on each cache line
- Multilevel hierarchy: L1 → L2 → Memory with automatic fallthrough on miss

### Fragmentation Metrics
- **Internal Fragmentation**: Sum of (allocated size - requested size) for each block
- **External Fragmentation**: 1 - (largest free block / total free)

### Memory Access Latency
- L1 hit: 1 cycle (serves immediately from cache)
- L2 hit: 5 cycles (takes 5 cycles to retrieve from L2)
- Memory: 100 cycles (main memory access)

## Testing

See [TESTING.md](TESTING.md) for comprehensive test cases and output examples.
