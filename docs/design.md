# Design Document: Memory Management Simulator

## Overview

This simulator models a complete OS memory management system with the following components:
1. **Dynamic Memory Allocator** with multiple strategies (First-Fit, Best-Fit, Worst-Fit)
2. **Buddy Allocator** for power-of-two sized allocations
3. **Multilevel Cache System** (L1 and L2 caches with set-associativity)
4. **Virtual Memory** with page translation and LRU page replacement

## Architecture

### 1. Memory Allocator (`src/allocator/`)

**Class**: `Allocator`

**Data Structure**:
```cpp
struct Block {
    int id;              // Unique block ID (0 = free)
    size_t addr;         // Start address
    size_t size;         // Allocated size
    size_t requested;    // User-requested size (for fragmentation tracking)
    bool free;           // Free/allocated flag
};
```

**Strategies**:

1. **First-Fit** (`find_block_first`):
   - Scans blocks sequentially
   - Allocates at first block with size >= requested
   - Time: O(n)
   - Result: Fast but can lead to fragmentation

2. **Best-Fit** (`find_block_best`):
   - Finds smallest block with size >= requested
   - Time: O(n)
   - Result: Minimizes fragmentation but slower

3. **Worst-Fit** (`find_block_worst`):
   - Finds largest available block
   - Time: O(n)
   - Result: Attempts to keep large free blocks

**Operations**:
- `allocate(size)`: Returns block ID or -1 on failure
- `freeBlockById(id)`: Free by ID
- `freeBlockByAddr(addr)`: Free by address
- `coalesce()`: Merge adjacent free blocks
- `split_block(idx, size)`: Split block if allocated size < block size

**Fragmentation Metrics**:
- **Internal**: Sum of (block_size - requested_size) for allocated blocks
- **External**: 1 - (largest_free / total_free)

---

### 2. Buddy Allocator (`src/buddy/`)

**Class**: `BuddyAllocator`

**Algorithm**:
- Maintains power-of-two free lists (one per order)
- Order k = block size 2^k bytes
- Total memory rounded up to nearest power of two

**Operations**:
- `allocate(size)`: 
  1. Compute order for size: order = log2(ceil(size))
  2. If free list[order] empty, recursively split larger block
  3. Return allocated block
  
- `freeBlockById(id)`:
  1. Mark block as free in appropriate order
  2. Recursively coalesce with buddy blocks

**Coalescing**:
- Buddy of block at address A with order K is at address A ^ (1 << K)
- Coalesce while buddy is free and order < max_order

---

### 3. Multilevel Cache (`src/cache/`)

**Class**: `CacheLevel`

**Parameters**:
- `cache_size`: Total cache capacity (bytes)
- `block_size`: Cache line/block size (bytes)
- `associativity`: Number of ways (set-associativity)
- `replacement_policy`: FIFO or LRU

**Derived Values**:
```
num_sets = (cache_size / block_size) / associativity
num_lines_per_set = associativity
```

**Address Breakdown**:
```
Physical Address = [TAG | INDEX | OFFSET]

offset_bits = log2(block_size)
index_bits = log2(num_sets)
tag_bits = remaining bits

offset = addr % block_size
block = addr / block_size
index = block % num_sets
tag = block / num_sets
```

**Cache Lookup**:
1. Compute offset, block, index, tag from address
2. Search all lines in set[index] for matching tag
3. On hit: Update LRU timestamp (if LRU policy), return true
4. On miss: 
   - If set not full: Insert new line
   - If set full: Evict LRU line (FIFO: remove oldest; LRU: remove least recent)
   - Return false

**Multilevel Cache Hierarchy**:
- L1 miss → check L2
- L2 miss → access memory (100 cycles)
- Latencies:
  - L1 hit: 1 cycle
  - L2 hit: 5 cycles
  - Memory: 100 cycles

---

### 4. Virtual Memory (`src/virtual_memory/`)

**Class**: `VirtualMemory`

**Data Structure**:
```cpp
struct PageTableEntry {
    bool valid;           // Page mapped to frame
    size_t frame;         // Physical frame number
    size_t lastAccess;    // LRU timestamp
};

unordered_map<size_t, PageTableEntry> page_table;  // VPN -> PTE
```

**Address Translation**:
```
Virtual Address = [VPN | OFFSET]
offset_bits = log2(page_size)

vpn = vaddr / page_size
offset = vaddr % page_size
paddr = (frame * page_size) + offset
```

**Page Faults**:
- On first access to unmapped page: Allocate frame from free frames
- If all frames used: Evict LRU page, reuse its frame

---

## Cache Configuration Examples

### Example 1: L1 Cache
```
set cache l1 32768 64 8 lru
Size: 32 KB = 32,768 bytes
Block size: 64 bytes
Associativity: 8 ways
Sets: 32768 / 64 / 8 = 64 sets
Policy: LRU
```

### Example 2: L2 Cache
```
set cache l2 262144 64 8 lru
Size: 256 KB = 262,144 bytes
Block size: 64 bytes
Associativity: 8 ways
Sets: 262144 / 64 / 8 = 512 sets
Policy: LRU
```

---

## CLI Design

**Prompt-based interactive interface**:
```
memsim> [command]
```

**Command Categories**:

1. **System Setup**
   - `init memory <size>` - Initialize memory
   - `set allocator <strategy>` - Choose allocation algorithm
   - `set cache <level> <params>` - Configure cache
   - `vm init <params>` - Initialize virtual memory

2. **Memory Operations**
   - `malloc <size>` - Allocate
   - `free <id>` - Deallocate
   - `access <addr>` - Memory access (triggers cache)

3. **Debugging**
   - `dump memory` - Show memory map
   - `dump buddy` - Show buddy free lists
   - `stats` - Show all statistics

---

## Performance Metrics

### Memory Statistics
- Total/Used/Free memory
- Internal fragmentation: Sum of wasted space in allocated blocks
- External fragmentation: Percentage (1 - largest_free/total_free)
- Allocation success rate: Successful allocations / Total attempts

### Cache Statistics
- Total accesses
- Cache hits
- Hit ratio = hits / accesses
- Total latency (cycle count)
- Average latency per access

### VM Statistics
- Page hits (translations in TLB/cache)
- Page faults (translations requiring frame allocation)

---

## Limitations & Future Work

### Current Limitations
- Single-threaded operation only
- No true TLB (page table cached in memory hierarchy)
- No instruction cache (I-cache) modeling
- No prefetching or write-back policies
- Simplified memory model (no DRAM timing)

### Potential Enhancements
- L3 cache support
- Prefetching policies
- Write-through vs. write-back cache consistency
- NUMA memory architecture
- Multi-level page tables
- Parallel simulation
- Performance visualization

---

## Testing & Validation

See `TESTING.md` for test cases demonstrating:
- All allocation strategies
- Buddy allocator correctness
- Multilevel cache hits/misses
- Virtual memory translation
- Fragmentation calculations

