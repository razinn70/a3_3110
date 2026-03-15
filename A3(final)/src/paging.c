#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define VM_SIZE  (4 * 1024)   // 4KB virtual memory
#define RAM_SIZE (1 * 1024)   // 1KB RAM

int page_table[VM_SIZE];    // vpn -> frame number (-1 if not mapped)
int frame_owner[RAM_SIZE];  // frame -> vpn currently stored (-1 if empty)

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
    for (int i = 0; i < RAM_SIZE; i++) frame_owner[i] = -1;

    printf("VM_SIZE\t%uB\n", VM_SIZE);
    printf("RAM_SIZE\t%uB\n", RAM_SIZE);
    printf("PAGE_SIZE\t%uB\n", page_size);
    printf("NUM_VPAGES\t%u\n", num_vpages);
    printf("NUM_FRAMES\t%u\n", num_frames);
    printf("PAGE_POLICY\tFIFO\n\n");

    FILE *fp = stdin;
    if (input_file) {
        fp = fopen(input_file, "r");
        if (!fp) { perror(input_file); return EXIT_FAILURE; }
    }

    unsigned long faults = 0, accesses = 0;
    unsigned long line_num = 0;
    unsigned int frames_used = 0;
    unsigned int fifo_next = 0;  // index of next frame to evict

    char line[64];
    while (fgets(line, sizeof(line), fp)) {
        // skip blank lines
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\0') continue;

        unsigned int vaddr = (unsigned int)strtoul(p, NULL, 16);
        unsigned int vpn   = vaddr / page_size;
        unsigned int offset = vaddr % page_size;

        if (vpn >= num_vpages) {
            printf("%6lu  [vaddr] 0x%04x\tXXXX\tInvalid vaddr\n", line_num, vaddr);
            line_num++;
            continue;
        }

        int frame;
        const char *arrow;

        if (page_table[vpn] != -1) {
            // page hit
            frame = page_table[vpn];
            arrow = "--->";
        } else {
            // page fault
            faults++;
            arrow = "-x->";

            if (frames_used < num_frames) {
                frame = (int)frames_used++;
            } else if (num_frames > 0) {
                // FIFO eviction
                frame = (int)fifo_next;
                int old_vpn = frame_owner[frame];
                if (old_vpn >= 0) page_table[old_vpn] = -1;
                fifo_next = (fifo_next + 1) % num_frames;
            } else {
                frame = 0;
                int old_vpn = frame_owner[0];
                if (old_vpn >= 0) page_table[old_vpn] = -1;
            }

            page_table[vpn] = frame;
            frame_owner[frame] = (int)vpn;
        }

        unsigned int paddr = (unsigned int)frame * page_size + offset;
        printf("%6lu  [vaddr] 0x%04x\t%s\t[paddr] 0x%04x\n",
               line_num, vaddr, arrow, paddr);
        accesses++;
        line_num++;
    }

    if (input_file) fclose(fp);

    printf("\nTotal accesses : %lu\n", accesses);
    printf("Page faults    : %lu\n", faults);
    if (accesses > 0) {
        printf("Fault rate     : %.2f%%\n", 100.0 * faults / accesses);
    } else {
        printf("Fault rate     : N/A (no accesses)\n");
    }

    return EXIT_SUCCESS;
}
