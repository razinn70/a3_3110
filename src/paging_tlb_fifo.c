#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define VM_SIZE   (4 * 1024)
#define RAM_SIZE  (1 * 1024)
#define TLB_SIZE  4

typedef struct {
    int vpn;
    int frame;
} TLB_Entry;

int page_table[VM_SIZE];    // vpn -> frame (-1 if not mapped)
int frame_owner[RAM_SIZE];  // frame -> vpn (-1 if empty)

TLB_Entry tlb[TLB_SIZE];
int tlb_count = 0;
int tlb_fifo_head = 0;  // oldest entry (next to evict)

// Returns frame if vpn is in TLB, -1 otherwise
static int tlb_lookup(unsigned int vpn)
{
    for (int i = 0; i < tlb_count; i++) {
        if (tlb[i].vpn == (int)vpn) return tlb[i].frame;
    }
    return -1;
}

// Insert vpn/frame into TLB (FIFO eviction when full)
static void tlb_insert(unsigned int vpn, int frame)
{
    if (tlb_count < TLB_SIZE) {
        tlb[tlb_count].vpn   = (int)vpn;
        tlb[tlb_count].frame = frame;
        tlb_count++;
    } else {
        tlb[tlb_fifo_head].vpn   = (int)vpn;
        tlb[tlb_fifo_head].frame = frame;
        tlb_fifo_head = (tlb_fifo_head + 1) % TLB_SIZE;
    }
}

int main(int argc, char *argv[])
{
    unsigned int page_size = 128;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            page_size = (unsigned int)atoi(argv[++i]);
        } else {
            input_file = argv[i];
        }
    }

    unsigned int num_vpages = VM_SIZE / page_size;
    unsigned int num_frames = RAM_SIZE / page_size;

    for (unsigned int i = 0; i < num_vpages; i++) page_table[i] = -1;
    for (unsigned int i = 0; i < num_frames; i++) frame_owner[i] = -1;

    printf("VM_SIZE\t%uB\n", VM_SIZE);
    printf("RAM_SIZE\t%uB\n", RAM_SIZE);
    printf("PAGE_SIZE\t%uB\n", page_size);
    printf("NUM_VPAGES\t%u\n", num_vpages);
    printf("NUM_FRAMES\t%u\n", num_frames);
    printf("PAGE_POLICY\tFIFO\n");
    printf("TLB_SIZE\t%d\n", TLB_SIZE);
    printf("TLB_POLICY\tFIFO\n\n");

    FILE *fp = stdin;
    if (input_file) {
        fp = fopen(input_file, "r");
        if (!fp) { perror(input_file); return EXIT_FAILURE; }
    }

    unsigned long faults = 0, accesses = 0;
    unsigned long tlb_hits = 0, tlb_misses = 0;
    unsigned int frames_used = 0;
    unsigned int fifo_next = 0;

    char line[64];
    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') continue;

        unsigned int vaddr  = (unsigned int)strtoul(p, NULL, 16);
        unsigned int vpn    = vaddr / page_size;
        unsigned int offset = vaddr % page_size;

        int frame;
        const char *arrow;
        int tlb_frame = tlb_lookup(vpn);

        if (tlb_frame != -1) {
            // TLB hit
            frame = tlb_frame;
            arrow = "===>";
            tlb_hits++;
        } else {
            // TLB miss
            tlb_misses++;

            if (page_table[vpn] != -1) {
                // Page in RAM
                frame = page_table[vpn];
                arrow = "--->";
            } else {
                // Page fault
                faults++;
                arrow = "-x->";

                if (frames_used < num_frames) {
                    frame = (int)frames_used++;
                } else {
                    frame = (int)fifo_next;
                    int old_vpn = frame_owner[frame];
                    page_table[old_vpn] = -1;
                    fifo_next = (fifo_next + 1) % num_frames;
                }

                page_table[vpn] = frame;
                frame_owner[frame] = (int)vpn;
            }

            tlb_insert(vpn, frame);
        }

        unsigned int paddr = (unsigned int)frame * page_size + offset;
        printf("%6lu  [vaddr] 0x%04X\t%s\t[paddr] 0x%04X\n",
               accesses, vaddr, arrow, paddr);
        accesses++;
    }

    if (input_file) fclose(fp);

    printf("\nTotal accesses : %lu\n", accesses);
    printf("Page faults    : %lu\n", faults);
    if (accesses > 0) {
        printf("Fault rate     : %.2f%%\n", 100.0 * faults / accesses);
    } else {
        printf("Fault rate     : N/A (no accesses)\n");
    }
    printf("TLB hits       : %lu\n", tlb_hits);
    printf("TLB misses     : %lu\n", tlb_misses);
    if (accesses > 0) {
        printf("TLB hit rate   : %.2f%%\n", 100.0 * tlb_hits / accesses);
    } else {
        printf("TLB hit rate   : N/A (no accesses)\n");
    }

    return EXIT_SUCCESS;
}
