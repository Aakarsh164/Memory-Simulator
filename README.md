Memory Management Simulator

Quickstart

Build:

Windows (MSYS/MinGW) or WSL:

```
make
```

Run:

```
bin/memsim
```

Commands (interactive):
- `init memory <bytes>` — initialize physical memory size
- `set allocator <first_fit|best_fit|worst_fit>`
- `malloc <bytes>` — allocate memory
- `free <id|0xaddr>` — free block by id or address
- `dump memory` — show blocks
- `stats` — show statistics
- `exit` — quit

Files of interest:
- `src/allocator` — allocator implementation
- `src/memory` — physical memory stub
- `docs/design.md` — design notes
