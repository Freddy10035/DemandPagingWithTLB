#ifndef TLB
#define TLB

#include <map>
#include <deque>
#include "math.h"
#define MEMORY_SPACE_SIZE 32
#define MAX_QUEUE_SIZE 10


class tlb
{
public:
    // constructor
    tlb(int vpnNumBits, int capacity);

    // cache mapping of vpn 2 pfn
    std::map<unsigned int /*vpn*/, unsigned int /*pfn*/> vpn2pfn;

    // queue of most recently accessed pages. Used to determine which mapping to remove from cache
    std::deque<unsigned int> recentPagesQueue;    // always be size 10

    // cache information
    int capacity;   // capacity of cache
    unsigned int vpnMask;       // bit mask for masking off cpn

    // setter method
    void setVpnMask(int vpnNumBits);

    // cache methods
    bool usingTlb();
    bool hasMapping(unsigned int vpn);
    void insertMapping(unsigned int vpn, unsigned int frameNum);

    // queue methods
    void updateQueue(unsigned int recentVpn);
    bool queueContains(unsigned int vpn);
    void eraseVpnFromQueue(unsigned int vpn);



};

#endif