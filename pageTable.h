#ifndef PAGETABLE
#define PAGETABLE

#include "level.h"
#include "tlb.h"
#include "tracereader.h"

#define MEMORY_SPACE_SIZE 32





class PageTable
{
public:
    // constructor
    PageTable(unsigned int, unsigned int*, int);

    // ptr to root level
    Level* rootLevel;

    // bit arrays and entryCountArr
    unsigned int* maskArr;
    unsigned int* shiftArr;
    unsigned int* entryCountArr;
    unsigned int* bitsInLevel;
    unsigned int offsetMask;        // to append onto PFN
    unsigned int offsetShift;

    // pageTable information
    unsigned int levelCount;
    unsigned int addressCount;
    unsigned int numBytesSize;
    unsigned int frameCount;
    unsigned int vpnNumBits;
    unsigned int pageSizeBytes;
    unsigned int currFrameNum;      // for the pageInsert function to know what frameNum to use

    // hit counts
    unsigned int countPageTableHits;
    unsigned int countTlbHits;

    // set array, mask and shift methods
    void setMaskArr();
    void shiftMaskArr();      // helper fuction for setMaskArr
    void setShiftArr();
    void setEntryCountArr();
    void setOffsetMask(unsigned int vpnNumBits);
    void setOffsetShift(unsigned int vpnNumBits);

    // calculation methods
    unsigned int getOffsetOfAddress(unsigned int virtAddr);
    unsigned int virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift);
    unsigned int appendOffset(unsigned int frameNum, unsigned int virtualAddress);

    // page walk methods
    void pageInsert(Level* lvlPtr, unsigned int virtualAddress);
    Map* pageLookup(Level* lvlPtr, unsigned int virtualAddress);

};




#endif