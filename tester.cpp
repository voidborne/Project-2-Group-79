// tester.cpp
// runs test functions

#include "tester.h"
#include "QuadProbing.h"
#include <iostream>
#include <cstdlib>

// couldnt include HashTable from project2 without pulling in everything
// so linear probing tests arent here, only quad
std::vector<LapRecord> buildFakeRecords(int howMany)
{
    std::vector<LapRecord> fakeList;
    fakeList.reserve(howMany);
    for (int i = 0; i < howMany; i++)
    {
        LapRecord fakeOne;
        fakeOne.raceId = i + 1;
        fakeOne.driverId = i + 1;
        fakeOne.lap = i + 1;
        fakeOne.milliseconds = 1000 + i;
        fakeOne.isOccupied = false;
        fakeList.push_back(fakeOne);
    }
    return fakeList;
}

static void printResult(std::string testName, bool passed)
{
    if (passed)
        std::cout << "  [PASS] " << testName << std::endl;
    else
        std::cout << "  [FAIL] " << testName << std::endl;
}

bool checkInsertAndSearchWork()
{
    int smallTableSize = 31;
    QuadHashTable testTable(smallTableSize);
    LapRecord oneRecord;
    oneRecord.raceId = 841;
    oneRecord.driverId = 3;
    oneRecord.lap = 12;
    oneRecord.milliseconds = 95432;
    oneRecord.isOccupied = false;
    testTable.putInTable(oneRecord);
    int foundTime = testTable.lookUpLap(841, 3, 12);
    bool foundCorrectly = (foundTime == 95432);
    int shouldBeMissing = testTable.lookUpLap(999, 9, 99);
    bool missingIsCorrect = (shouldBeMissing == -1);
    
    return foundCorrectly && missingIsCorrect;
}

bool checkCollisionCountingWorks()
{
    int tinySize = 7;
    QuadHashTable tinyTable(tinySize);
    std::vector<LapRecord> testNumInsert = buildFakeRecords(100);
    for (const auto& rec : testNumInsert)
        tinyTable.putInTable(rec);
    
        return (tinyTable.getCollisionTotal() > 0);
}

bool checkBothTablesAgreeOnSearches(const std::vector<LapRecord>& someRealData)
{
    if (someRealData.empty())
    {
        std::cout << "  (skipped - no real data loaded yet)" << std::endl;
        
        return true;
    }

    int testTableSize = (int)(someRealData.size() / 0.7);
    QuadHashTable quadTestTable(testTableSize);
    int sampleCount = (int)someRealData.size();
    if (sampleCount > 500) sampleCount = 500;
    for (int i = 0; i < sampleCount; i++)
        quadTestTable.putInTable(someRealData[i]);
    int failedLookups = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        const LapRecord& rec = someRealData[i];
        int result = quadTestTable.lookUpLap(rec.raceId, rec.driverId, rec.lap);
        if (result != rec.milliseconds)
            failedLookups++;
    }
    if (failedLookups > 0)
        std::cout << "  (" << failedLookups << " lookups came back wrong)" << std::endl;

    return (failedLookups == 0);
}

void runAllTests(const std::vector<LapRecord>& someRealData)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "         Testing          " << std::endl;
    std::cout << "========================================" << std::endl;
    bool r1 = checkInsertAndSearchWork();
    printResult("insert one record and find it again", r1);
    bool r2 = checkCollisionCountingWorks();
    printResult("collision counter goes up on collision", r2);
    bool r3 = checkBothTablesAgreeOnSearches(someRealData);
    printResult("quad table finds everything it inserted (500 record sample)", r3);
    int passCount = (int)r1 + (int)r2 + (int)r3;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << passCount << "/3 tests passed" << std::endl;
    std::cout << "========================================\n" << std::endl;
}
// tester.cpp
