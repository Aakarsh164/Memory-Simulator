# Memory Management Simulator - Complete Test Suite

## Quick Demo Commands

Run the binary in the terminal and type these commands:

### 1. Test Simple Allocator (First Fit)
```
init memory 1024
set allocator first_fit
malloc 100
malloc 200
malloc 150
free 1
dump memory
stats
```

### 2. Test Best Fit
```
set allocator best_fit
malloc 100
free 2
malloc 50
dump memory
stats
```

### 3. Test Buddy Allocator
```
set allocator buddy
malloc 256
malloc 128
malloc 64
free 1
dump buddy
stats
```

### 4. Test Cache with Memory Access
```
set cache l1 256 16 2 lru
access 0
access 16
access 32
access 0
stats
```

### 5. Test Virtual Memory
```
set vm 4096 256 1024
access 0x0
access 0x100
stats
```

### Build & Run

```bash
# Compile (from memory-simulator directory)
g++ -std=c++11 -O2 -Isrc src/main.cpp src/allocator/allocator.cpp src/memory/physical_memory.cpp src/buddy/buddy.cpp src/cache/cache.cpp src/virtual_memory/virtual_memory.cpp -o bin/memsim.exe

# Run
.\bin\memsim.exe
```

Type `exit` to quit.
