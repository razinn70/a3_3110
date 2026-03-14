# CIS*3110 A3 — Testing Guide

---

## Key Facts (from actual expected outputs)

- VM_SIZE = 4096B, RAM_SIZE = 1024B, default PAGE_SIZE = 128B
- 32 virtual pages (VPN 0–31), 8 frames (frame 0–7)
- TLB_SIZE = 4 entries
- `paging` outputs **lowercase** hex (`0x003a`)
- `paging_tlb_fifo` and `paging_tlb` output **UPPERCASE** hex (`0x003A`)
- Output format uses **tabs**, not spaces, between fields
- Percent signs included in rates: `100.00%`, `0.00%`
- No invalid address handling — all test inputs contain valid addresses (VPN 0–31)

---

## CLI Syntax (correct)

```sh
./paging          [-p page_size] <input_file>
./paging_tlb_fifo [-p page_size] <input_file>
./paging_tlb      [-p page_size] -s LRU <input_file>
./paging_tlb      [-p page_size] -s MIN <input_file>
```

`paging_tlb` does **not** support FIFO via `-s`. Use `paging_tlb_fifo` for FIFO.

---

## Exact Output Format

### paging

```
VM_SIZE	4096B
RAM_SIZE	1024B
PAGE_SIZE	128B
NUM_VPAGES	32
NUM_FRAMES	8
PAGE_POLICY	FIFO

     0  [vaddr] 0x0000	-x->	[paddr] 0x0000

Total accesses : 8
Page faults    : 8
Fault rate     : 100.00%
```

### paging_tlb_fifo / paging_tlb

Adds two header lines and three footer lines (uppercase hex):

```
VM_SIZE	4096B
RAM_SIZE	1024B
PAGE_SIZE	128B
NUM_VPAGES	32
NUM_FRAMES	8
PAGE_POLICY	FIFO
TLB_SIZE	4
TLB_POLICY	FIFO

     0  [vaddr] 0x0000	-x->	[paddr] 0x0000

Total accesses : 8
Page faults    : 8
Fault rate     : 100.00%
TLB hits       : 0
TLB misses     : 8
TLB hit rate   : 0.00%
```

Arrow meanings:
- `-x->` page fault (not in RAM)
- `--->` page hit, TLB miss (in RAM, not in TLB)
- `===>` TLB hit (found in TLB)

Access counter is 6-wide right-justified with two trailing spaces before `[vaddr]`.

---

## Running the Provided Test Cases

```sh
make

# input1 (8 accesses, all faults)
./paging inputs/input1.txt
./paging_tlb_fifo inputs/input1.txt
./paging_tlb -s LRU inputs/input1.txt
./paging_tlb -s MIN inputs/input1.txt

# input2 (8 accesses, all faults)
./paging inputs/input2.txt
./paging_tlb_fifo inputs/input2.txt
./paging_tlb -s LRU inputs/input2.txt
./paging_tlb -s MIN inputs/input2.txt

# input3 (10 accesses, 9 faults — includes a page hit and policy divergence)
./paging inputs/input3.txt
./paging_tlb_fifo inputs/input3.txt
./paging_tlb -s LRU inputs/input3.txt
./paging_tlb -s MIN inputs/input3.txt
```

---

## Diff Against Expected Outputs

On Linux (autograder):
```sh
diff <(./paging inputs/input3.txt) outputs/output3_paging.txt
diff <(./paging_tlb_fifo inputs/input3.txt) outputs/output3_paging_tlb_fifo.txt
diff <(./paging_tlb -s LRU inputs/input3.txt) outputs/output3_paging_tlb_LRU.txt
diff <(./paging_tlb -s MIN inputs/input3.txt) outputs/output3_paging_tlb_MIN.txt
```

On Windows (MinGW/bash) — strip CRLF from both sides:
```sh
diff <(./paging inputs/input3.txt | tr -d '\r') <(cat outputs/output3_paging.txt | tr -d '\r')
```

Run all 12 at once:
```sh
pass=0; fail=0
for i in 1 2 3; do
    for prog in paging paging_tlb_fifo; do
        if diff -q <(./A3_starter/src/$prog A3_starter/inputs/input$i.txt 2>/dev/null | tr -d '\r') \
                   <(tr -d '\r' < A3_starter/outputs/output${i}_${prog}.txt) > /dev/null 2>&1; then
            echo "PASS  input$i  $prog"; ((pass++))
        else
            echo "FAIL  input$i  $prog"; ((fail++))
        fi
    done
    for s in LRU MIN; do
        if diff -q <(./A3_starter/src/paging_tlb -s $s A3_starter/inputs/input$i.txt 2>/dev/null | tr -d '\r') \
                   <(tr -d '\r' < A3_starter/outputs/output${i}_paging_tlb_${s}.txt) > /dev/null 2>&1; then
            echo "PASS  input$i  paging_tlb -s $s"; ((pass++))
        else
            echo "FAIL  input$i  paging_tlb -s $s"; ((fail++))
        fi
    done
done
echo ""
echo "Results: $pass passed, $fail failed"
```

Expected: `Results: 12 passed, 0 failed`

---

## Worked Example: input3

`input3.txt` contains:
```
0x0aba   # VPN=21, offset=0x3a → frame 0 → paddr 0x003a  (fault)
0x0a37   # VPN=20, offset=0x37 → frame 1 → paddr 0x00b7  (fault)
0x009e   # VPN= 0, offset=0x1e → frame 2 → paddr 0x011e  (fault)
0x0c85   # VPN=25, offset=0x05 → frame 3 → paddr 0x0185  (fault)
0x0da7   # VPN=27, offset=0x27 → frame 4 → paddr 0x0227  (fault)
0x0c79   # VPN=24, offset=0x79 → frame 5 → paddr 0x02f9  (fault)
0x0999   # VPN=19, offset=0x19 → frame 6 → paddr 0x0319  (fault)
0x08c7   # VPN=17, offset=0x47 → frame 7 → paddr 0x03c7  (fault)
0x0ae9   # VPN=21 — already in frame 0   → paddr 0x0069  (page hit / TLB hit for MIN)
0x038e   # VPN= 7, offset=0x0e → RAM full → eviction      (fault)
```

**Step 9 eviction differences (VPN 7 faults, RAM full):**

| Program | Evicts | Frame | paddr |
|---------|--------|-------|-------|
| `paging` (FIFO) | VPN=21 (frame 0, loaded first) | 0 | `0x000e` |
| `paging_tlb_fifo` (FIFO) | VPN=21 (frame 0) | 0 | `0x000E` |
| `paging_tlb -s LRU` | VPN=20 (frame 1, last used at step 1) | 1 | `0x008E` |
| `paging_tlb -s MIN` | VPN=27 (frame 4, never used again) | 4 | `0x000E` |

**Step 8 TLB arrow differences:**

After 8 faults (steps 0–7), TLB holds the last 4 loaded: VPN=25, VPN=27, VPN=24, VPN=19.
At step 8 (VPN=21 again), the TLB is full and must evict.

- FIFO: evicts VPN=25 (oldest in TLB), inserts VPN=21 — but VPN=21 was not in TLB yet → `--->`
- LRU:  evicts VPN=25 (least recently used), same result → `--->`
- MIN:  VPN=27 has no future use; evicts it, inserts VPN=21 — VPN=21 is now in TLB → `===>`

This is why `output3_paging_tlb_MIN.txt` shows `===>` at step 8 and `TLB hits: 1`, while FIFO and LRU show `---> ` and `TLB hits: 0`.

---

## Address Translation Calculator

```
Given: vaddr = 0x0ABA, page_size = 128

VPN    = 0x0ABA / 128 = 21
offset = 0x0ABA % 128 = 0x3A = 58

If VPN=21 → frame 0:  paddr = 0*128 + 58 = 0x003A
If VPN=21 → frame 4:  paddr = 4*128 + 58 = 0x023A
```

VPN quick reference (page_size = 128):

| VPN | vaddr range      |
|-----|-----------------|
| 0   | 0x0000–0x007F   |
| 1   | 0x0080–0x00FF   |
| 2   | 0x0100–0x017F   |
| 7   | 0x0380–0x03FF   |
| 8   | 0x0400–0x047F   |
| 17  | 0x0880–0x08FF   |
| 19  | 0x0980–0x09FF   |
| 20  | 0x0A00–0x0A7F   |
| 21  | 0x0A80–0x0AFF   |
| 24  | 0x0C00–0x0C7F   |
| 25  | 0x0C80–0x0CFF   |
| 27  | 0x0D80–0x0DFF   |
| 31  | 0x0F80–0x0FFF   |

---

## Hand-Traceable Test Cases

### Test 1 — Basic Translation, No Eviction

8 unique pages into 8 frames, then re-access VPN=0. No eviction ever occurs.

```sh
cat > /tmp/no_eviction.txt << 'EOF'
0x0000
0x0080
0x0100
0x0180
0x0200
0x0280
0x0300
0x0380
0x0000
EOF
./paging /tmp/no_eviction.txt
```

Expected footer:
```
Total accesses : 9
Page faults    : 8
Fault rate     : 88.89%
```

Step 8 (`0x0000`) is a page hit (`--->`), not a fault.

---

### Test 2 — FIFO vs LRU Divergence

Fill 8 frames, re-access VPN=0, then bring in a 9th page. The re-access makes VPN=0 recently used, so FIFO and LRU pick different victims.

```sh
cat > /tmp/lru_vs_fifo.txt << 'EOF'
0x0000
0x0080
0x0100
0x0180
0x0200
0x0280
0x0300
0x0380
0x0000
0x0400
EOF
./paging /tmp/lru_vs_fifo.txt                  # FIFO: evicts VPN=0 → paddr 0x0000
./paging_tlb -s LRU /tmp/lru_vs_fifo.txt       # LRU:  evicts VPN=1 → paddr 0x0080
```

Step 9 line differs:
- FIFO: `     9  [vaddr] 0x0400	-x->	[paddr] 0x0000`
- LRU:  `     9  [vaddr] 0x0400	-x->	[paddr] 0x0080`

---

### Test 3 — TLB Hit

Access the same page twice. Second access is a TLB hit.

```sh
cat > /tmp/tlb_hit.txt << 'EOF'
0x0000
0x0001
0x0080
0x0081
EOF
./paging_tlb_fifo /tmp/tlb_hit.txt
```

Expected:
```
     0  [vaddr] 0x0000	-x->	[paddr] 0x0000
     1  [vaddr] 0x0001	===>	[paddr] 0x0001
     2  [vaddr] 0x0080	-x->	[paddr] 0x0080
     3  [vaddr] 0x0081	===>	[paddr] 0x0081

Total accesses : 4
Page faults    : 2
Fault rate     : 50.00%
TLB hits       : 2
TLB misses     : 2
TLB hit rate   : 50.00%
```

---

### Test 4 — TLB FIFO Eviction, Then Page Hit

5 unique VPNs fills TLB. 6th access (VPN=0 again) is a TLB miss but page hit because VPN=0 is still in RAM.

```sh
cat > /tmp/tlb_fifo_evict.txt << 'EOF'
0x0000
0x0080
0x0100
0x0180
0x0200
0x0000
EOF
./paging_tlb_fifo /tmp/tlb_fifo_evict.txt
```

Expected:
```
     0  [vaddr] 0x0000	-x->	[paddr] 0x0000
     1  [vaddr] 0x0080	-x->	[paddr] 0x0080
     2  [vaddr] 0x0100	-x->	[paddr] 0x0100
     3  [vaddr] 0x0180	-x->	[paddr] 0x0180
     4  [vaddr] 0x0200	-x->	[paddr] 0x0200
     5  [vaddr] 0x0000	--->	[paddr] 0x0000

Total accesses : 6
Page faults    : 5
Fault rate     : 83.33%
TLB hits       : 0
TLB misses     : 6
TLB hit rate   : 0.00%
```

---

### Test 5 — MIN Optimal

MIN should have equal or fewer faults than LRU on any sequence.

```sh
cat > /tmp/min_test.txt << 'EOF'
0x0000
0x0100
0x0200
0x0000
0x0300
0x0100
0x0200
0x0100
EOF
./paging_tlb -s LRU /tmp/min_test.txt
./paging_tlb -s MIN /tmp/min_test.txt
```

Compare `Page faults` line. MIN faults ≤ LRU faults.

---

## Generating Random Test Addresses

```python
# gen_addrs.py
import random

PAGE_SIZE = 128
VM_SIZE   = 4096
NUM_PAGES = VM_SIZE // PAGE_SIZE   # 32 valid VPNs (0–31)

addrs = []

# Hit all 32 valid pages
for vpn in range(NUM_PAGES):
    offset = random.randint(0, PAGE_SIZE - 1)
    addrs.append(f"0x{(vpn * PAGE_SIZE + offset):04x}")

# Add repeated accesses to exercise hit/eviction paths
for vpn in [0, 1, 2, 3, 0, 1, 4, 5]:
    offset = random.randint(0, PAGE_SIZE - 1)
    addrs.append(f"0x{(vpn * PAGE_SIZE + offset):04x}")

random.shuffle(addrs)

with open("/tmp/random.txt", "w") as f:
    f.write("\n".join(addrs) + "\n")

print(f"Generated {len(addrs)} addresses")
```

Run: `python3 gen_addrs.py`

---

## Valgrind

```sh
sudo apt install valgrind   # if needed

valgrind --leak-check=full ./paging inputs/input3.txt
valgrind --leak-check=full ./paging_tlb_fifo inputs/input3.txt
valgrind --leak-check=full ./paging_tlb -s LRU inputs/input3.txt
valgrind --leak-check=full ./paging_tlb -s MIN inputs/input3.txt
```

Look for: `no leaks are possible` and `0 errors from 0 contexts`.

---

## Pre-Push Checklist

```
[ ] make compiles with zero warnings
[ ] All 12 diff tests pass (3 inputs × 4 programs)
[ ] paging uses lowercase hex, paging_tlb_fifo/paging_tlb use uppercase
[ ] Header and footer labels match exactly (spacing, colons, % sign)
[ ] TLB invalidation: evicting a RAM frame removes its TLB entry
[ ] LRU and FIFO give different paddr on step 9 for input3
[ ] MIN gives ===> TLB hit on step 8 for input3; FIFO/LRU give --->
[ ] valgrind shows 0 errors on all four programs
[ ] README.md has your real name and student ID
[ ] git push → check A3_feedback branch
```

---

## Autograder Feedback Loop

```sh
git add src/ README.md
git commit -m "fix: describe what you fixed"
git push

# Then go to:
# gitlab.socs.uoguelph.ca/cis3110s01w26/YOUR_USERNAME/A3
# Branch: A3_feedback → read the latest timestamped report
```
