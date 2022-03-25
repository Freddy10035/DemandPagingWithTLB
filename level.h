#ifndef LEVEL
#define LEVEL

#include <stdlib.h>
#include "Map.h"
#include <math.h>

class PageTable;    // defines PageTable for compiler

class Level
{
public:
    Level();    // default constructor
    Level(int, PageTable*);     // overloaded constructor
    Level** nextLevel;          // double pointer enables arr of Level* ptrs
    Map* mapPtr;                // single pointer so arr of Map objects
    unsigned int currDepth;     // depth of this Level. Referenced in main
    PageTable* pTable;          // pointer to PageTable object that contains the levels and info about masks and levels
    void setNextLevel();        // assigns nextLevel to arr of Level* ptrs
    void setNextLevelNull();    // instantiates each element of nextLevel to NULL
    void setMapPtr();           // assigns mapPtr to arr of Map objects
};




#endif