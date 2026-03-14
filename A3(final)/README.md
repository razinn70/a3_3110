# CIS\*3110 A3 Paging and cache replacement

### Full Name: Rajin Uddin
### Student id: 1211414

## Overview

This project implements virtual memory address translation with configurable page replacement policies. Three programs are provided:

- **`paging`** — basic demand paging with FIFO page replacement, no TLB.
- **`paging_tlb_fifo`** — demand paging with a 4-entry TLB using FIFO replacement for both RAM and TLB.
- **`paging_tlb`** — demand paging with a 4-entry TLB using either LRU or MIN replacement for both RAM and TLB (selected via `-s`).

All programs simulate a 4 KB virtual address space and 1 KB physical RAM. On each memory access the virtual page number is extracted, the page table is consulted, and a physical address is produced. When a page is not in RAM a page fault occurs and a victim frame is chosen according to the active policy.

**FIFO** evicts the frame that has been in RAM the longest. **LRU** evicts the frame that was least recently accessed. **MIN** (Belady's optimal) evicts the page whose next use is furthest in the future; ties are broken by evicting the one loaded earliest (FIFO order). For MIN the entire access sequence is read up front before simulation begins.

The TLB is checked before the page table. A TLB hit (`===>`) avoids a page-table walk. A TLB miss that still finds the page in RAM is a page hit (`--->`). A TLB miss where the page is absent from RAM is a page fault (`-x->`). After any TLB miss the entry is installed in the TLB, evicting an existing entry when it is full.

## Installation

```sh
make        # builds paging, paging_tlb_fifo, paging_tlb
make clean  # removes binaries
make debug  # builds with -g -DDEBUG
```

Requires `gcc` and `make`. Tested on Linux (the autograder environment).

## Usage

```sh
./paging          [-p page_size] <input_file>
./paging_tlb_fifo [-p page_size] <input_file>
./paging_tlb      [-p page_size] -s {LRU|MIN} <input_file>
```

- `-p page_size` — page size in bytes (default: 128). Must divide evenly into 4096 and 1024.
- `-s {LRU|MIN}` — replacement policy for `paging_tlb` (required).
- `<input_file>` — text file with one hex virtual address per line (e.g. `0x0aba`).

**Output format** — header block followed by one line per access, then a statistics block:

```text
VM_SIZE    4096B
RAM_SIZE   1024B
PAGE_SIZE  128B
NUM_VPAGES 32
NUM_FRAMES 8
PAGE_POLICY FIFO

     0  [vaddr] 0x0aba  -x->  [paddr] 0x003a
     1  [vaddr] 0x0ae9  --->  [paddr] 0x0069
     2  [vaddr] 0x0ae9  ===>  [paddr] 0x0069

Total accesses : 3
Page faults    : 1
Fault rate     : 33.33%
TLB hits       : 1      (TLB programs only)
TLB misses     : 2
TLB hit rate   : 33.33%
```

Arrow meanings: `-x->` page fault, `--->` page hit (TLB miss), `===>` TLB hit.
`paging` prints lowercase hex; `paging_tlb_fifo` and `paging_tlb` print uppercase hex.

**Common usage examples:**

```bash
# Basic paging, FIFO, default page size
./paging inputs/input3.txt

# TLB with FIFO
./paging_tlb_fifo inputs/input3.txt

# TLB with LRU
./paging_tlb -s LRU inputs/input3.txt

# TLB with MIN (optimal)
./paging_tlb -s MIN inputs/input3.txt

# Custom page size (64 bytes)
./paging -p 64 inputs/input1.txt
```
