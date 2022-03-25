#ifndef MAP
#define MAP

class Map
{
public:
    Map();  // default constructor
    void setFrameNum(int frameNumber);  // sets frameNum of Map
    void setValid();    // sets valid = true
    bool isValid();     // returns valid used in PageTable::paegLookup()
    unsigned int getFrameNum();       // returns frameNum
private:
    unsigned int frameNum;      // physical frameNum mapped to vpn
    bool valid;     // true if Map has been initialized and given a frameNum


};

#endif