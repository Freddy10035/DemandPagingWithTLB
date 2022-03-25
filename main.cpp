#include <iostream>
#include <fstream>
#include "unistd.h"
#include <stdlib.h>
#include <string.h>
#include "pageTable.h"
#include "output_mode_helpers.h"
#include "Map.h"
#include "tlb.h"
#include "main.h"
#define MEMORY_SPACE_SIZE 32
#define DEFAULT_NUM_ADDRESSES -1
#define DEFAULT_CACHE_SIZE 0
#define DEFAULT_OUTPUT_MODE (char*)"summary"

/**
 * @brief - Processes command line args. Checks that appropiate num of cmd ln args.
 * Checks if number of bits specified for levels are viable.
 * @param argc - count of cmd ln args
 * @param argv - arr of cmd ln args as char*
 * @param nFlag - int* for nFlag. Indicates number of addresses to process
 * @param cFlag - int* for cFlag. Indicates capacity for TLB
 * @param oFlag - char** for oFlag. Indicates output mode
 *
 */
void processCmdLnArgs(int argc, char* argv[], int* nFlag, int* cFlag, char** oFlag)
{
    // check that the minimum # of cmd-line args are given
    if (argc < 3)
    {
        std::cerr << "Error: Not enough command-line args given" << std::endl;
        std::cerr << "  \nExiting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt;

    // process optional flags
    // skips over if no optional flags
    while ((opt = getopt(argc, argv, "n:c:o:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            *nFlag = atoi(optarg);
            break;
        case 'c':
            *cFlag = atoi(optarg);
            // check if cFlag is valid
            if (*cFlag < 0) {
                std::cerr << "Cache capacity must be a number, greater than or equal to 0" << std::endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 'o':
            *oFlag = optarg;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    // go here if only optional cmd-line args are given but not the mandatory ones
    if (optind > (argc - 2)) {
        std::cerr << "Error:\n  Gave optional cmd line args but not mandatory ones\n";
        std::cerr << "  Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    // check bit options are viable
    int totBits = 0;
    int bitOption = 0;
    for (int i = optind + 1; i < argc; i++) {
        bitOption = atoi(argv[i]);
        totBits += bitOption;
        if (bitOption < 1) {
            std::cerr << "Level " << i - optind - 1 << " page table must be at least 1 bit" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (totBits > 28) {
            std::cerr << "Too many bits used in page tables" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

}


/**
 * @brief - Checks if tracefile exists and can be read.
 * @param argc - count of cmdln args
 * @param argv - arr of cmdln args
 * @return FILE* for tracefile
 */
FILE* readTraceFile(int argc, char* argv[])
{
    FILE* traceFile;

    // check that trace file can be opened
    char* traceFname = argv[optind];
    if ((traceFile = fopen(traceFname, "rb")) == NULL) {
        std::cerr << "Unable to open <<" << traceFname << ">>" << std::endl;
        exit(EXIT_FAILURE);
    }

    return traceFile;
}


/**
 * @brief - Conditionally calls the report functions ferom output_mode_helpers.c based on output mode chosen
 * @param pTable - pointer to the pageTable
 * @param virtAddr - virtual address value
 * @param physAddr - physical address value translated from virtAddr
 * @param frameNum - physical frame number for this address
 * @param tlbHit - true if address was already in TLB, else false
 * @param pageTableHit - true if address was already in pageTable, else false
 * @param v2p - true if virtual_to_physical mode
 * @param v2p_tlb - true if v2p_tlb_pt mode
 * @param vpn2pfn - true if vpn2pfn mode
 * @param offset - true if offset mode
 */
void report(PageTable* pTable, unsigned int virtAddr, unsigned int physAddr, unsigned int frameNum,
    bool tlbHit, bool pageTableHit, bool v2p, bool v2p_tlb, bool vpn2pfn, bool offset)
{
    if (v2p) {  // virtual2PhysicalMode
        report_virtual2physical(virtAddr, physAddr);
    }
    else if (v2p_tlb) {     // v2p_tlb_pt
        // this mode prints virt to phys translation and tlbHit and pageTableHit info
        report_v2pUsingTLB_PTwalk(virtAddr, physAddr, tlbHit, pageTableHit);
    }
    else if (vpn2pfn) {
        unsigned int pages[pTable->levelCount];
        for (int i = 0; i < pTable->levelCount; i++) {
            pages[i] = pTable->virtualAddressToPageNum(virtAddr, pTable->maskArr[i], pTable->shiftArr[i]);
        }
        // this mode shows vpn to pfn mapping
        report_pagemap(pTable->levelCount, pages, frameNum);
    }
    else if (offset) {
        // this mode shows the offset vals of each virtAddr
        hexnum(pTable->getOffsetOfAddress(virtAddr));
    }
}

/**
 * @brief - Takes in next address and calculates framenum, physAddr, and pageTableHit.
 * Checks pageTable to see if there's a hit. Inserts mapping into pageTable if not present.
 * @param trace - p2AddrTr*. Used for getting the nextAddress to process
 * @param pTable - pointer to pageTable obj. Holds info about the levels and masks
 * @param v2p - true if virtual2physical mode
 * @param v2p_tlb - true if v2p_tlb_pt mode
 * @param vpn2pfn - true if vpn2pfn mode
 * @param offset - true if offset mode
 */
void processNextAddress(p2AddrTr* trace, PageTable* pTable,
    bool v2p, bool v2p_tlb, bool vpn2pfn, bool offset)
{
    unsigned int virtAddr = 0;
    unsigned int frameNum = 0;
    unsigned int physAddr = 0;
    bool tlbHit = false;
    bool pageTableHit = true;   // default true, conditional check if not
    Map* frame;

    virtAddr = trace->addr;     // assign virtAddr a value

    frame = pTable->pageLookup(pTable->rootLevel, virtAddr);
    if (frame == nullptr) {
        // go here if PageTable MISS
        pageTableHit = false;
        pTable->pageInsert(pTable->rootLevel, virtAddr);
        frame = pTable->pageLookup(pTable->rootLevel, virtAddr);
        frameNum = frame->getFrameNum();
        pTable->frameCount++;
    }
    else {
        // go here if PageTable HIT
        frameNum = frame->getFrameNum();
        pTable->countPageTableHits++;
    }

    physAddr = pTable->appendOffset(frameNum, virtAddr);    // calculate physical address

    // call reporting functions
    report(pTable, virtAddr, physAddr, frameNum, tlbHit, pageTableHit, v2p, v2p_tlb, vpn2pfn, offset);

}

/**
 * @brief - Overloaded version that takes into account the TLB cache.
 * Takes in next address and calculates framenum, physAddr, tlbHit and pageTableHit.
 * First checks if mapping in TLB. If not, updates the TLB and checks pageTable to see if there's a hit.
 * Inserts mapping into pageTable if not present. Regardless of hit status for tlb or pageTable, updates the
 * recent address queue.
 * @param trace - p2AddrTr*. Used for getting the nextAddress to process
 * @param pTable - pointer to pageTable obj. Holds info about the levels and masks
 * @param cache - tlb* for accessing cache info and mappings
 * @param v2p - true if virtual2physical mode
 * @param v2p_tlb - true if v2p_tlb_pt mode
 * @param vpn2pfn - true if vpn2pfn mode
 * @param offset - true if offset mode
 */
void processNextAddress(p2AddrTr* trace, PageTable* pTable, tlb* cache,
    bool v2p, bool v2p_tlb, bool vpn2pfn, bool offset)
{
    unsigned int virtAddr = 0;
    unsigned int vpn = 0;
    unsigned int frameNum = 0;
    unsigned int physAddr = 0;
    bool tlbHit = false;
    bool pageTableHit = true;   // default true, conditional check if not
    Map* frame;

    virtAddr = trace->addr;     // assign virtAddr a value
    vpn = virtAddr & cache->vpnMask;
    vpn = vpn >> (MEMORY_SPACE_SIZE - pTable->vpnNumBits);

    // go here if TLB hit
    if (cache->hasMapping(vpn)) {
        frameNum = cache->vpn2pfn[vpn];
        tlbHit = true;
        pTable->countTlbHits++;
        cache->updateQueue(vpn);    // update most recently used
    }
    // go here if TLB MISS
    else {
        frame = pTable->pageLookup(pTable->rootLevel, virtAddr);
        if (frame == nullptr) {
            // go here if PageTable MISS
            pageTableHit = false;
            pTable->pageInsert(pTable->rootLevel, virtAddr);
            frame = pTable->pageLookup(pTable->rootLevel, virtAddr);
            frameNum = frame->getFrameNum();
            cache->insertMapping(vpn, frameNum);    // update cache
            pTable->frameCount++;
        }
        else {
            // go here if PageTable HIT
            frameNum = frame->getFrameNum();
            cache->insertMapping(vpn, frameNum);    // update cache
            pTable->countPageTableHits++;
        }
        cache->updateQueue(vpn);    // update most recently used
    }

    physAddr = pTable->appendOffset(frameNum, virtAddr);    // calculate physAddr

    // call reporting function
    report(pTable, virtAddr, physAddr, frameNum, tlbHit, pageTableHit, v2p, v2p_tlb, vpn2pfn, offset);

}


/**
 * @brief - called to read address from trace file. If nFlag default mode, will read all addresses.
 * Else, will read specified numAddresses from nFlag. Also will process addresses based on if usingTlb.
 * @param traceFile - FILE* for traceFile
 * @param trace - p2AddrTr*. Used for getting the nextAddress to process. Will be passed to processNextAddress
 * @param pTable - ptr to pageTable which holds info about masks and levels. Will be passed to processNextAddress
 * @param cache - tlb ptr with info about cache and recent address queue. Used to determine if tlb is being used
 * @param numAddresses - how many addresses to process based on nFlag optional cmdln arg
 * @param v2p - true if virtual2physical mode
 * @param v2p_tlb - true if v2p_tlb_pt mode
 * @param vpn2pfn - true if vpn2pfn mode
 * @param offset - true if offset mode
 */
void readAddresses(FILE* traceFile, p2AddrTr* trace, PageTable* pTable, tlb* cache, int numAddresses,
    bool v2p, bool v2p_tlb, bool vpn2pfn, bool offset)
{
    // read all virtual addresses and insert into tree if not already present
    if (numAddresses == DEFAULT_NUM_ADDRESSES) {
        while (!feof(traceFile)) {
            if (NextAddress(traceFile, trace)) {     // traceFile: File handle from fOpen
                if (cache->usingTlb()) {
                    processNextAddress(trace, pTable, cache, v2p, v2p_tlb, vpn2pfn, offset);
                }
                else {
                    processNextAddress(trace, pTable, v2p, v2p_tlb, vpn2pfn, offset);
                }
                pTable->addressCount++;
            }
        }
        // read only numAddresses number of addresses
    }
    else {
        for (int i = 0; i < numAddresses; i++) {
            if (NextAddress(traceFile, trace)) {     // traceFile: File handle from fOpen
                if (cache->usingTlb()) {
                    processNextAddress(trace, pTable, cache, v2p, v2p_tlb, vpn2pfn, offset);
                }
                else {
                    processNextAddress(trace, pTable, v2p, v2p_tlb, vpn2pfn, offset);
                }
                pTable->addressCount++;
            }
        }
    }

}

/**
 * @brief - process cmd line args. Number of levels, bits in each level and numVpnBits based on cmd line args.
 * Call readTraceFile to check if traceFile can be opened. Create pageTable and tlb objects. Conditionally readAddresses
 * based on output mode. Call reporting functions if addresses do not need to be read.
 */
int main(int argc, char** argv)
{
    int nFlag = DEFAULT_NUM_ADDRESSES;      // how many addresses to read in (default -1 = read ALL addresses)
    int cFlag = DEFAULT_CACHE_SIZE;         // cache capacity (default 0 = no TLB)
    char* oFlag = DEFAULT_OUTPUT_MODE;      // what type of output to show (default = summary)

    processCmdLnArgs(argc, argv, &nFlag, &cFlag, &oFlag);

    unsigned int numLevels = (argc - 1) - optind;   // number of levels for pageTable calculated from mandatory cmd line args
    unsigned int bitsInLevel[numLevels];            // unsigned int arr holding numBits in each level
    int vpnNumBits = 0;                             // numBits in VPN total 

    // calculate bitsInLevel and vpnNumBits
    for (int i = 0; i < numLevels; i++) {
        bitsInLevel[i] = atoi(argv[optind + i + 1]);
        vpnNumBits += bitsInLevel[i];
    }

    FILE* traceFile = readTraceFile(argc, argv);    // check if traceFile can be opened
    p2AddrTr trace;

    // instantiate PageTable and tlb objects
    PageTable pTable(numLevels, bitsInLevel, vpnNumBits);
    tlb* cache = new tlb(vpnNumBits, cFlag);

    // deal with output mode
    if (strcmp(oFlag, "bitmasks") == 0) {
        report_bitmasks(numLevels, pTable.maskArr);
    }
    else if (strcmp(oFlag, "virtual2physical") == 0) {
        readAddresses(traceFile, &trace, &pTable, cache, nFlag, true, false, false, false);
    }
    else if (strcmp(oFlag, "v2p_tlb_pt") == 0) {
        readAddresses(traceFile, &trace, &pTable, cache, nFlag, false, true, false, false);
    }
    else if (strcmp(oFlag, "vpn2pfn") == 0) {
        readAddresses(traceFile, &trace, &pTable, cache, nFlag, false, false, true, false);
    }
    else if (strcmp(oFlag, "offset") == 0) {
        readAddresses(traceFile, &trace, &pTable, cache, nFlag, false, false, false, true);
    }
    else if (strcmp(oFlag, "summary") == 0) {
        readAddresses(traceFile, &trace, &pTable, cache, nFlag, false, false, false, false);
        report_summary(pTable.pageSizeBytes, pTable.countTlbHits,
            pTable.countPageTableHits, pTable.addressCount, pTable.frameCount, pTable.numBytesSize);
    }
    else {
        std::cout << "Invalid Output Mode" << std::endl;
        exit(EXIT_FAILURE);
    }

}