#include <stdio.h>
#include "output_mode_helpers.h"

/* Handle C++ namespaces, ignore if compiled in C
 * C++ usually uses this #define to declare the C++ standard.
 * It will not be defined if a C compiler is used.
 */
#ifdef __cplusplus
using namespace std;
#endif

/*
 * report_virtual2physical(src, dest)
 * Given a pair of numbers, output a line:
 *      src -> dest
 * Example usages:
 * Map between logical and physical addresses:  mapping(logical, physical)
 * Map between page number and frame: mapping(page, frame)
 */
void report_virtual2physical(uint32_t src, uint32_t dest) {
    fprintf(stdout, "%08X -> %08X\n", src, dest);
    fflush(stdout);
}

/*
 * report_v2pUsingTLB_PTwalk(src, dest, tlbhit, pthit)
 * Given a pair of addresses, hit or miss along the tlb then pagetable walk,
 * output a line:
 *      src -> dest, tlb hit
 *      src -> dest, tlb miss, pagetable hit
 *      src -> dest, tlb miss, pagetable miss
 * Example usages:
 * Report translation/mapping from logical to physical address:
 *        e.g., report_v2pUsingTLB_PTwalk(logical, physical, false, true)
 *              tlb miss, pagetable hit
 * Report translation/mapping from virtual page number to physical frame:
 *        e.g., report_v2pUsingTLB_PTwalk(page, frame, false, true)
 *              tlb miss, pagetable hit
 */
void report_v2pUsingTLB_PTwalk(uint32_t src, uint32_t dest, bool tlbhit, bool pthit) {

    fprintf(stdout, "%08X -> %08X, ", src, dest);

    if (tlbhit)
        fprintf(stdout, "tlb hit\n");
    else {
        fprintf(stdout, "tlb miss, pagetable %s\n", pthit ? "hit" : "miss");
    }

    fflush(stdout);
}

/*
 * hexnum
 * Used for writing out a number in hex, one per line
 */
void hexnum(uint32_t number) {
    printf("%08X\n", number);
    fflush(stdout);
}


/*
 * report_summary
 * Write out a mesasge that indicates summary information for the page table.
 * page_size - Number of bytes per page
 * cacheHits - Number of vpn->pfn mapping found in the TLB
 * pageTableHits - Number of times a page was mapped.
 * addresses - Number of addresses processed
 * frames_used - Number of frames allocated
 * bytes - Total number of bytes needed for page table data structure.
 *         Should include all levels, allocated arrays, etc.
 */
void report_summary(unsigned int page_size,
    unsigned int cacheHits,
    unsigned int pageTableHits,
    unsigned int addresses, unsigned int frames_used,
    unsigned int bytes) {
    unsigned int misses;
    double hit_percent;

    printf("Page size: %d bytes\n", page_size);
    /* Compute misses (page faults) and hit percentage */
    int totalhits = cacheHits + pageTableHits;
    misses = addresses - totalhits;
    hit_percent = (double)(totalhits) / (double)addresses * 100.0;
    printf("Addresses processed: %d\n", addresses);
    printf("Cache hits: %d, Page hits: %d, Total hits: %d, Misses: %d\n",
        cacheHits, pageTableHits, totalhits, misses);
    printf("Total hit percentage: %.2f%%, miss percentage: %.2f%%\n",
        hit_percent, 100 - hit_percent);
    printf("Frames allocated: %d\n", frames_used);
    printf("Bytes used:  %d\n", bytes);

    fflush(stdout);
}

/*
 * report_bitmasks
 * Write out bitmasks.
 * levels - Number of levels
 * masks - Pointer to array of bitmasks
 */
void report_bitmasks(int levels, uint32_t* masks) {
    printf("Bitmasks\n");
    for (int idx = 0; idx < levels; idx++)
        /* show mask entry and move to next */
        printf("level %d mask %08X\n", idx, masks[idx]);

    fflush(stdout);
}

/*
 * report_pagemap
 * Write out page numbers and frame number
 * levels - specified number of levels in page table
 * pages - pages[idx] is the page number associated with level idx (0 < idx < levels)
 * frame - page is mapped to specified frame
 */
void report_pagemap(int levels, uint32_t* pages, uint32_t frame) {
    /* output pages */
    for (int idx = 0; idx < levels; idx++)
        printf("%X ", pages[idx]);
    /* output frame */
    printf("-> %X\n", frame);

    fflush(stdout);
}

