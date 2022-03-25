#include "pageTable.h"

// default constructor needed for nextLevel[]
Level::Level()
{
    currDepth = 0;
    pTable = NULL;
}

// constructor sets depth, pTable, nextLevel and mapPtr
Level::Level(int depth, PageTable* tablePtr)
{
    currDepth = depth;
    pTable = tablePtr;
    setNextLevel();
    setNextLevelNull();     // set zeroeth levels netLevel[] to all nulls
    mapPtr = nullptr;       // initialize to nullptr to avoid segFault.
}

// assigns nextLevel to an array of Level*
void Level::setNextLevel()
{
    // size of nextLevel = num possible levels at the currDepth
    nextLevel = new Level * [pTable->entryCountArr[currDepth]];
}

// ensures that all elements of nextLevel are set to Null
void Level::setNextLevelNull()
{
    for (int i = 0; i < pTable->entryCountArr[currDepth]; i++) {
        nextLevel[i] = nullptr;
    }
}

// use new operator to assign mapPtr a Map arr
void Level::setMapPtr()
{
    // size of mapPtr = num possible levels based on numBits in level
    this->mapPtr = new Map[pTable->entryCountArr[currDepth]];
}