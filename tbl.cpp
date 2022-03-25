#include "tlb.h"
#include <stack>


/**
 * @brief - constructor sets the capacity and the vpnMask
 * @param vpnNumBits - number of bits in vpn
 * @param capacity - capacity of the cache given by cFlag
 */
tlb::tlb(int vpnNumBits, int capacity)
{
    this->capacity = capacity;
    setVpnMask(vpnNumBits);
}


/**
 * @brief - returns true if capacity of TLB != 0
 */
bool tlb::usingTlb()
{
    return this->capacity != 0;
}


/**
 * @brief - sets vpnMask based on how many bits are the vpn
 * @param vpnNumBits - number of bits in vpn
 */
void tlb::setVpnMask(int vpnNumBits)
{
    this->vpnMask = 0;
    for (int i = 0; i <= vpnNumBits; i++) {
        vpnMask += pow(2, MEMORY_SPACE_SIZE - i);
    }
}


/**
 * @brief - returns true if cache has mapping for given vpn
 * @param vpn - search for mapping of this vpn
 */
bool tlb::hasMapping(unsigned int vpn)
{
    if (vpn2pfn.find(vpn) != vpn2pfn.end()) {
        return true;
    }
    return false;
}


/**
 * @brief - inserts mapping of this vpn to the given pfn. Handles if cache is
 * AT CAPACITY
 * @param vpn - vpn to map
 * @param frameNum - pfn to map vpn to
 */
void tlb::insertMapping(unsigned int vpn, unsigned int frameNum)
{
    // if tlb at capacity need to erase the least recent used address
    if (vpn2pfn.size() >= capacity) {
        vpn2pfn.erase(recentPagesQueue.front());     // erase least recently used
        recentPagesQueue.pop_front();       // erase this from queue as well
    }

    vpn2pfn[vpn] = frameNum;
}

/**
 * @brief - checks if the recentPages Queue contains the given vpn
 * @param vpn - pageNumber to check for
 */
bool tlb::queueContains(unsigned int vpn)
{
    for (int i = 0; i < recentPagesQueue.size(); i++) {
        if (recentPagesQueue[i] == vpn) return true;
    }
    return false;
}


/**
 * @brief - find vpn in queue and erase it from the queue
 * @param vpn - vpn to erase
 */
void tlb::eraseVpnFromQueue(unsigned int vpn)
{
    for (int i = 0; i < recentPagesQueue.size(); i++) {
        if (recentPagesQueue[i] == vpn) {
            recentPagesQueue.erase(recentPagesQueue.begin() + i);
        }
    }
}


/**
 * @brief - update queue with new most recent Page access. Handles if queue already contains vpn.
 * Also handles if queue is at max size of 10
 * @param recentVpn - vpn to add to recentQueue
 */
void tlb::updateQueue(unsigned int recentVpn)
{
    if (queueContains(recentVpn)) {     // if queue contains vpn update vpn to most recent
        eraseVpnFromQueue(recentVpn);
    }

    recentPagesQueue.push_back(recentVpn);

    // if queue is at size, remove the oldest page accessed
    if (recentPagesQueue.size() > MAX_QUEUE_SIZE) {
        recentPagesQueue.pop_front();
    }
}