// QuadProbing.h
// quadratic probing header

#pragma once
#include <vector>
#include "shared_types.h"

class QuadHashTable
{
private:
    std::vector<LapRecord> tableSlots;
    int totalSlots;
    int itemsInserted;
    long long timesWeCollided;
    int figureOutStartingSlot(long long compositeKey);

public:
    QuadHashTable(int howManySlots);
    void putInTable(const LapRecord& incomingRecord);
    int lookUpLap(int raceNum, int driverNum, int lapNum);
    long long getCollisionTotal();
    int getItemCount();
};
// QuadProbing.h
