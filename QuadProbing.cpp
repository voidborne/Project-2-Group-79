// QuadProbing.cpp
// quadratic probing using (start + i*i) % tableSize to reduce clustering

#include "QuadProbing.h"

// constructor
QuadHashTable::QuadHashTable(int howManySlots)
    : totalSlots(howManySlots), itemsInserted(0), timesWeCollided(0)
{
    tableSlots.resize(totalSlots);
}

int QuadHashTable::figureOutStartingSlot(long long compositeKey)
{
    return (int)(compositeKey % totalSlots);
}

void QuadHashTable::putInTable(const LapRecord& incomingRecord)
{
    long long keyVal = incomingRecord.getCompositeKey();
    int startingSpot = figureOutStartingSlot(keyVal);
    int jumpCount = 0;

    while (jumpCount < totalSlots)
    {
        int tryingThisSpot = (startingSpot + jumpCount * jumpCount) % totalSlots;
        if (!tableSlots[tryingThisSpot].isOccupied)
        {
            tableSlots[tryingThisSpot] = incomingRecord;
            tableSlots[tryingThisSpot].isOccupied = true;
            itemsInserted++;
            return;
        }
        // collision
        timesWeCollided++;
        jumpCount++;
    }
}

int QuadHashTable::lookUpLap(int raceNum, int driverNum, int lapNum)
{
    LapRecord dummyRecord;
    dummyRecord.raceId = raceNum;
    dummyRecord.driverId = driverNum;
    dummyRecord.lap = lapNum;
    long long keyVal = dummyRecord.getCompositeKey();
    int startingSpot = figureOutStartingSlot(keyVal);
    int jumpCount = 0;

    while (jumpCount < totalSlots)
    {
        int tryingThisSpot = (startingSpot + jumpCount * jumpCount) % totalSlots;
        if (!tableSlots[tryingThisSpot].isOccupied)
            break;
        if (tableSlots[tryingThisSpot].getCompositeKey() == keyVal)
            return tableSlots[tryingThisSpot].milliseconds;
        jumpCount++;
    }
    // not found
    return -1;
}

long long QuadHashTable::getCollisionTotal()
{
    return timesWeCollided;
}

int QuadHashTable::getItemCount()
{
    return itemsInserted;
}
// QuadProbing.cpp
