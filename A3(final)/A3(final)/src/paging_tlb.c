#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#define VM_SIZE      (4 * 1024)
#define RAM_SIZE     (1 * 1024)
#define MAX_ACCESSES 65536

typedef enum { POLICY_FIFO, POLICY_LRU, POLICY_MIN } Policy;

/* ---- RAM state ---- */
int           page_table[VM_SIZE];         /* vpn -> frame (-1 if not mapped) */
int           frame_owner[RAM_SIZE];       /* frame -> vpn (-1 if empty) */
unsigned long frame_loaded[RAM_SIZE];      /* step when frame was loaded (MIN tie-break) */
unsigned long frame_last_used[RAM_SIZE];   /* step of last access (LRU) */

/* ---- TLB state ---- */
typedef struct {
    int           vpn;
    int           frame;
    unsigned long loaded;    /* step loaded (MIN tie-break) */
    unsigned long last_used; /* step last accessed (LRU) */
} TLB_Entry;

TLB_Entry *tlb = NULL;
int tlb_size     = 4;
int tlb_count    = 0;
int tlb_fifo_head = 0; /* FIFO: index of oldest entry */

/* ---- Pre-loaded addresses (used by MIN) ---- */
unsigned int all_addrs[MAX_ACCESSES];
int total_accesses_loaded = 0;

/* Return step of next use of vpn after current_step, or INT_MAX if none */
static int next_use(unsigned int vpn, int current_step, unsigned int page_size)
{
    for (int j = current_step + 1; j < total_accesses_loaded; j++) {
        if (all_addrs[j] / page_size == vpn) return j;
    }
    return INT_MAX;
}

/* Return index of frame to evict from RAM */
static int evict_ram(Policy policy, int num_frames, int current_step,
                     unsigned int page_size)
{
    if (policy == POLICY_LRU) {
        int victim = 0;
        for (int i = 1; i < num_frames; i++) {
            if (frame_last_used[i] < frame_last_used[victim]) victim = i;
        }
        return victim;
    } else { /* MIN */
        int victim = 0;
        int victim_next = next_use((unsigned int)frame_owner[0], current_step, page_size);
        for (int i = 1; i < num_frames; i++) {
            int ni = next_use((unsigned int)frame_owner[i], current_step, page_size);
            /* prefer further future; on tie, evict oldest loaded (FIFO-style) */
            if (ni > victim_next ||
                (ni == victim_next && frame_loaded[i] < frame_loaded[victim])) {
                victim = i;
                victim_next = ni;
            }
        }
        return victim;
    }
}

/* Return index in tlb[] of entry to evict */
static int evict_tlb(Policy policy, int current_step, unsigned int page_size)
{
    if (policy == POLICY_LRU) {
        int victim = 0;
        for (int i = 1; i < tlb_count; i++) {
            if (tlb[i].last_used < tlb[victim].last_used) victim = i;
        }
        return victim;
    } else { /* MIN */
        int victim = 0;
        int victim_next = next_use((unsigned int)tlb[0].vpn, current_step, page_size);
        for (int i = 1; i < tlb_count; i++) {
            int ni = next_use((unsigned int)tlb[i].vpn, current_step, page_size);
            if (ni > victim_next ||
                (ni == victim_next && tlb[i].loaded < tlb[victim].loaded)) {
                victim = i;
                victim_next = ni;
            }
        }
        return victim;
    }
}

/* Remove TLB entry for vpn (called when its RAM frame is evicted) */
static void tlb_invalidate(int vpn)
{
    for (int i = 0; i < tlb_count; i++) {
        if (tlb[i].vpn == vpn) {
            /* compact: shift remaining entries left */
            for (int j = i; j < tlb_count - 1; j++) tlb[j] = tlb[j + 1];
            tlb_count--;
            /* keep fifo_head valid */
            if (i < tlb_fifo_head) {
                tlb_fifo_head--;
            } else if (tlb_fifo_head >= tlb_count) {
                tlb_fifo_head = 0;
            }
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    unsigned int page_size = 128;
    Policy policy = POLICY_LRU;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            page_size = (unsigned int)atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            i++;
            if      (strcmp(argv[i], "LRU")  == 0) policy = POLICY_LRU;
            else if (strcmp(argv[i], "MIN")  == 0) policy = POLICY_MIN;
            else if (strcmp(argv[i], "FIFO") == 0) policy = POLICY_FIFO;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            tlb_size = atoi(argv[++i]);
        } else {
            input_file = argv[i];
        }
    }

    tlb = (TLB_Entry *)malloc((size_t)tlb_size * sizeof(TLB_Entry));
    if (!tlb) { perror("malloc"); return EXIT_FAILURE; }
    for (int i = 0; i < tlb_size; i++) {
        tlb[i].vpn = -1; tlb[i].frame = -1;
        tlb[i].loaded = 0; tlb[i].last_used = 0;
    }

    unsigned int num_vpages = VM_SIZE / page_size;
    unsigned int num_frames = RAM_SIZE / page_size;

    for (unsigned int i = 0; i < num_vpages; i++) page_table[i] = -1;
    for (int i = 0; i < RAM_SIZE; i++) {
        frame_owner[i]     = -1;
        frame_loaded[i]    = 0;
        frame_last_used[i] = 0;
    }

    const char *pname = (policy == POLICY_LRU) ? "LRU" :
                        (policy == POLICY_MIN)  ? "MIN" : "FIFO";

    printf("VM_SIZE\t%uB\n",     VM_SIZE);
    printf("RAM_SIZE\t%uB\n",    RAM_SIZE);
    printf("PAGE_SIZE\t%uB\n",   page_size);
    printf("NUM_VPAGES\t%u\n",   num_vpages);
    printf("NUM_FRAMES\t%u\n",   num_frames);
    printf("PAGE_POLICY\t%s\n",  pname);
    printf("TLB_SIZE\t%d\n",     tlb_size);
    printf("TLB_POLICY\t%s\n\n", pname);

    FILE *fp = stdin;
    if (input_file) {
        fp = fopen(input_file, "r");
        if (!fp) { perror(input_file); free(tlb); return EXIT_FAILURE; }
    }

    char line[64];

    /* MIN requires knowing future accesses: pre-read entire file.
       Store ALL addresses (including out-of-range) so step indices align. */
    if (policy == POLICY_MIN) {
        while (fgets(line, sizeof(line), fp) &&
               total_accesses_loaded < MAX_ACCESSES) {
            char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '\n' || *p == '\0') continue;
            all_addrs[total_accesses_loaded++] =
                (unsigned int)strtoul(p, NULL, 16);
        }
        if (input_file) { fclose(fp); fp = NULL; }
    }

    unsigned long faults    = 0, accesses = 0;
    unsigned long tlb_hits  = 0, tlb_misses = 0;
    unsigned long line_num  = 0;
    unsigned int  frames_used = 0;
    unsigned int  fifo_next   = 0; /* FIFO RAM eviction pointer */
    int step = 0;

    while (1) {
        unsigned int vaddr;

        if (policy == POLICY_MIN) {
            if (step >= total_accesses_loaded) break;
            vaddr = all_addrs[step];
        } else {
            if (!fgets(line, sizeof(line), fp)) break;
            char *p = line;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '\n' || *p == '\0') continue;
            vaddr = (unsigned int)strtoul(p, NULL, 16);
        }

        unsigned int vpn    = vaddr / page_size;
        unsigned int offset = vaddr % page_size;

        if (vpn >= num_vpages) {
            printf("%6lu  [vaddr] 0x%04X\tXXXX\tInvalid vaddr\n", line_num, vaddr);
            line_num++;
            step++;
            continue;
        }

        int frame;
        const char *arrow;

        /* --- TLB lookup --- */
        int tlb_idx = -1;
        for (int i = 0; i < tlb_count; i++) {
            if (tlb[i].vpn == (int)vpn) { tlb_idx = i; break; }
        }

        if (tlb_idx != -1) {
            /* TLB hit */
            frame = tlb[tlb_idx].frame;
            tlb[tlb_idx].last_used = accesses;
            frame_last_used[frame] = accesses;
            arrow = "===>";
            tlb_hits++;
        } else {
            /* TLB miss */
            tlb_misses++;

            if (page_table[vpn] != -1) {
                /* Page in RAM */
                frame = page_table[vpn];
                arrow = "--->";
            } else {
                /* Page fault */
                faults++;
                arrow = "-x->";

                if (frames_used < num_frames) {
                    frame = (int)frames_used++;
                } else if (num_frames > 0) {
                    /* Evict a RAM frame */
                    if (policy == POLICY_LRU) {
                        frame = evict_ram(POLICY_LRU, (int)num_frames,
                                          step, page_size);
                    } else if (policy == POLICY_MIN) {
                        frame = evict_ram(POLICY_MIN, (int)num_frames,
                                          step, page_size);
                    } else {
                        frame = (int)fifo_next;
                        fifo_next = (fifo_next + 1) % num_frames;
                    }
                    int old_vpn = frame_owner[frame];
                    if (old_vpn >= 0) {
                        page_table[old_vpn] = -1;
                        tlb_invalidate(old_vpn);
                    }
                } else {
                    frame = 0;
                    int old_vpn = frame_owner[0];
                    if (old_vpn >= 0) {
                        page_table[old_vpn] = -1;
                        tlb_invalidate(old_vpn);
                    }
                }

                page_table[vpn] = frame;
                frame_owner[frame]     = (int)vpn;
                frame_loaded[frame]    = accesses;
            }

            frame_last_used[frame] = accesses;

            /* Insert into TLB */
            int tlb_slot;
            if (tlb_count < tlb_size) {
                tlb_slot = tlb_count++;
            } else {
                if (policy == POLICY_LRU) {
                    tlb_slot = evict_tlb(POLICY_LRU, step, page_size);
                } else if (policy == POLICY_MIN) {
                    tlb_slot = evict_tlb(POLICY_MIN, step, page_size);
                } else {
                    tlb_slot = tlb_fifo_head;
                    tlb_fifo_head = (tlb_fifo_head + 1) % tlb_size;
                }
            }
            tlb[tlb_slot].vpn       = (int)vpn;
            tlb[tlb_slot].frame     = frame;
            tlb[tlb_slot].loaded    = accesses;
            tlb[tlb_slot].last_used = accesses;
        }

        unsigned int paddr = (unsigned int)frame * page_size + offset;
        printf("%6lu  [vaddr] 0x%04X\t%s\t[paddr] 0x%04X\n",
               line_num, vaddr, arrow, paddr);
        accesses++;
        line_num++;
        step++;
    }

    if (fp && input_file) fclose(fp);
    free(tlb);

    printf("\nTotal accesses : %lu\n", accesses);
    printf("Page faults    : %lu\n",  faults);
    if (accesses > 0)
        printf("Fault rate     : %.2f%%\n", 100.0 * faults / accesses);
    else
        printf("Fault rate     : N/A (no accesses)\n");
    printf("TLB hits       : %lu\n",  tlb_hits);
    printf("TLB misses     : %lu\n",  tlb_misses);
    if (accesses > 0)
        printf("TLB hit rate   : %.2f%%\n", 100.0 * tlb_hits / accesses);
    else
        printf("TLB hit rate   : N/A (no accesses)\n");

    return EXIT_SUCCESS;
}
