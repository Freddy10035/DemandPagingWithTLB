#include "pageTable.h"

/**
 * @brief - constructor zero initializes count and size fields.
 * Then, initializes some fields based on the args passed to the constructor.
 * Next initializes the arrays and sets the arrays and masks to some values using helper methods.
 * Initializes rootLevel ptr and increments numBytesSize
 * @param numLevels - will be assigned to levelCount. Simply the number of levels in the pageTree
 * @param bitsInLevel - array with number of bits in each level
 * @param vpnNumBits - number of bits in vpn
 */
PageTable::PageTable(unsigned int numLevels, unsigned int bitsInLevel[], int vpnNumBits)
{
    // zero initialize
    this->addressCount = 0;
    this->frameCount = 0;
    this->numBytesSize = 0;
    this->countTlbHits = 0;
    this->countPageTableHits = 0;
    this->currFrameNum = 0;

    // initialize from constructor args
    this->vpnNumBits = vpnNumBits;
    this->levelCount = numLevels;
    this->pageSizeBytes = (unsigned int)pow(2, (MEMORY_SPACE_SIZE - vpnNumBits));     // 2^(bits in offset)

    // initialize arrays
    this->entryCountArr = new unsigned int[numLevels];
    this->maskArr = new unsigned int[numLevels];
    this->shiftArr = new unsigned int[numLevels];
    this->bitsInLevel = bitsInLevel;

    // set arrays and masks and shifts
    setOffsetMask(vpnNumBits);
    setOffsetShift(vpnNumBits);
    setEntryCountArr();
    setMaskArr();
    setShiftArr();

    // initialize rootLevel ptr
    this->rootLevel = new Level(0, this);        // 'this' is pointer to this PageTable

    this->numBytesSize += sizeof(Level);         // incrementing numBytesSize by size of Level * number of possible Levels in nextLevelArr
}


/**
 * @brief - sets the entryCountArr as 2^(bitsInLevel). This is the total number of possible entries
 * for the nextLevel or mapPtr arrays.
 */
void PageTable::setEntryCountArr()
{
    for (int i = 0; i < levelCount; i++) {
        this->entryCountArr[i] = pow(2, bitsInLevel[i]);
    }
}


/**
 * @brief - sets the maskArr by adding 2^(MemorySpaceSize - j) where j goes from 0 to number
 * of bits in level. Then this calls shiftMaskArr to shift the values over to the correct locations for the mask.
 */
void PageTable::setMaskArr()
{
    unsigned int mask;
    for (int i = 0; i < levelCount; i++) {
        mask = 0;
        for (int j = 0; j <= bitsInLevel[i]; j++) {
            mask += (unsigned int)pow(2, (MEMORY_SPACE_SIZE - j));
        }
        this->maskArr[i] = mask;
    }
    shiftMaskArr();
}


/**
 * @brief - helper function for setMaskArr. Bit shifts the elements of maskArr to the appropiate spots based on their
 * position in the virtual address.
 */
void PageTable::shiftMaskArr()
{
    // shift mask arr
    int shiftAmmount = 0;
    for (int i = 1; i < levelCount; i++) {       // starts at 1 bc first mask doesn't need to shift
        shiftAmmount += bitsInLevel[i - 1];
        this->maskArr[i] = this->maskArr[i] >> shiftAmmount;
    }
}


/**
 * @brief - sets the shiftArr. ShiftArr values will be used for shifting levels of the VPN when they are
 * extracted from the virtual address
 */
void PageTable::setShiftArr()
{
    int shift = MEMORY_SPACE_SIZE;
    for (int i = 0; i < levelCount; i++) {
        shift = shift - bitsInLevel[i];
        this->shiftArr[i] = shift;
    }
}


/**
 * @brief - sets the offsetMask using the MEMORY_SPACE_SIZE - vpnNumBits
 */
void PageTable::setOffsetMask(unsigned int vpnNumBits)
{
    this->offsetMask = 0;
    unsigned int numBitsOffset = MEMORY_SPACE_SIZE - vpnNumBits;

    for (int i = 0; i < numBitsOffset; i++) {
        this->offsetMask += pow(2, i);
    }
}


/**
 * @brief - set offSetShift. This is used to shift the frameNumber in order to calculate the physicalAddress
 */
void PageTable::setOffsetShift(unsigned int vpnNumBits)
{
    this->offsetShift = MEMORY_SPACE_SIZE - vpnNumBits;
}


/**
 * @brief - helper address calculates the offset of and address using the virtualAddress and offsetMask
 */
unsigned int PageTable::getOffsetOfAddress(unsigned int virtAddr)
{
    return virtAddr & offsetMask;
}


/**
 * @brief - takes in a virtualAddress and calculates the pageNum or VPN. Returns the vpn.
 * @param virtualAddress - address to convert
 * @param mask - bitmask to be used to mask of pageNum
 * @param shift - bitshift to be used to shift the pageNum into the least significant position
 */
unsigned int PageTable::virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift)
{
    // FIXME: Needs testing
    unsigned int pageNum;
    pageNum = virtualAddress & mask;
    pageNum = pageNum >> shift;
    return pageNum;
}


/**
 * @brief - inserts a virtual address into the pageTable by calculatingthe pageNum
 * starting at the root level and working up through there.
 * Uses recursion to achieve this passing the nextLevel to pageInsert each time.
 * @param lvlPtr - Level* to the current level being worked with.
 * @param virtualAddress - the virtualAddress we are trying to add
 */
void PageTable::pageInsert(Level* lvlPtr, unsigned int virtualAddress)
{
    unsigned int mask = maskArr[lvlPtr->currDepth];
    unsigned int shift = shiftArr[lvlPtr->currDepth];
    unsigned int pageNum = virtualAddressToPageNum(virtualAddress, mask, shift);

    // go here if lvlPtr is a leaf node
    if (lvlPtr->currDepth == levelCount - 1) {
        // go here if mapPtr array hasn't been instantiated
        if (lvlPtr->mapPtr == nullptr) {
            lvlPtr->setMapPtr();    // instantiate mapPtr
            numBytesSize += sizeof(Map) * entryCountArr[lvlPtr->currDepth];
        }
        lvlPtr->mapPtr[pageNum].setFrameNum(currFrameNum);
        lvlPtr->mapPtr[pageNum].setValid();
        currFrameNum++;
    }
    // go here if lvlPtr is interior node
    else {
        // go here if pageNum at this level has already been set
        if (lvlPtr->nextLevel[pageNum] != nullptr) {
            pageInsert(lvlPtr->nextLevel[pageNum], virtualAddress);
        }
        // go here if nextLevel[pageNum] has not been set yet
        else {
            Level* newLevel = new Level(lvlPtr->currDepth + 1, this);   // newLevels depth is currDepth + 1
            lvlPtr->nextLevel[pageNum] = newLevel;
            numBytesSize += sizeof(Level) * entryCountArr[lvlPtr->currDepth];
            pageInsert(newLevel, virtualAddress);
        }
    }
}


/**
 * @brief - Uses recurrsion to check if the pageTable contains the virtualAddress. Returns null if not found
 * and a Map* to the Map object containing the mapping if found.
 * @param lvlPtr - Level* to the current level being worked with.
 * @param virtualAddress - address we are searching the pageTable for
 */
Map* PageTable::pageLookup(Level* lvlPtr, unsigned int virtualAddress)
{
    unsigned int mask = maskArr[lvlPtr->currDepth];
    unsigned int shift = shiftArr[lvlPtr->currDepth];
    unsigned int pageNum = virtualAddressToPageNum(virtualAddress, mask, shift);

    // go here if lvlPtr is a leaf node
    if (lvlPtr->currDepth == levelCount - 1) {
        // // go here if mapPtr not set
        if (lvlPtr->mapPtr == nullptr) {
            return nullptr;
        }
        // go here if map invalid
        if (!lvlPtr->mapPtr[pageNum].isValid()) {
            return nullptr;
        }
        // returns this if page hit
        return &(lvlPtr->mapPtr[pageNum]);
    }
    // go here if lvlPtr is interior node
    if (lvlPtr->nextLevel[pageNum] == nullptr) {
        return nullptr;
    }

    pageLookup(lvlPtr->nextLevel[pageNum], virtualAddress);     // recursion to next level

}


/**
 * @brief - Helper method which calculates and returns the physical address by appending the offset to the frameNumber
 * @param frameNum - pfn being used to calculate the physical Address
 * @param virtualAddress - address we are converting to physical
 */
unsigned int PageTable::appendOffset(unsigned int frameNum, unsigned int virtualAddress)
{
    unsigned int physicalAddr = frameNum << offsetShift;
    physicalAddr = physicalAddr | getOffsetOfAddress(virtualAddress);
    return physicalAddr;
}