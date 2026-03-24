// anomaly.cpp
// handles the driver performance index calculations and related checks
#include "anomaly.h"
#include <iostream>
#include <iomanip>
#include <unordered_map>

std::unordered_map<int, double> buildDriverAverages(const std::vector<LapRecord>& allRecords)
{
    // first pass: go through and total lap times + count laps per driver
    std::unordered_map<int, long long> lapTimeSum;
    std::unordered_map<int, int> lapCount;
    for (const auto& rec : allRecords)
    {
        lapTimeSum[rec.driverId] += rec.milliseconds;
        lapCount[rec.driverId]++;
    }

    // second pass: go through and get the average from total and count
    std::unordered_map<int, double> driverAvgs;
    for (const auto& entry : lapTimeSum)
    {
        int driverId = entry.first;
        double avgMs = (double)entry.second / lapCount[driverId];
        driverAvgs[driverId] = avgMs;
    }
    return driverAvgs;
}

AnomalyResult checkLapAnomaly (
    int lapTimeMs, 
    int driverId, 
    const std::unordered_map<int,
    double>& driverAvgs
) {
    AnomalyResult result;
    result.isFlagged = false;
    result.pctDeviation = 0.0;
    result.label = "NORMAL";

    // if no average exists for this driver just leave it as normal
    if (driverAvgs.find(driverId) == driverAvgs.end())
    {
        result.label = "NO DATA";
        return result;
    }
    double avgMs = driverAvgs.at(driverId);

    // difference from avg (positive slower, negative faster)
    result.pctDeviation = (lapTimeMs - avgMs) / avgMs;
    if (result.pctDeviation >= INCIDENT_THRESHOLD)
    {
        result.isFlagged = true;
        result.label = "** INCIDENT **";   
    }
    else if (result.pctDeviation >= SLOW_LAP_THRESHOLD)
    {
        result.isFlagged = true;
        result.label = "SLOW LAP";      
    }
    else if (result.pctDeviation <= -FAST_LAP_THRESHOLD)
    {
        result.isFlagged = true;
        result.label = "FAST LAP";
    }
    return result;
}

std::vector<LapRecord> findFlaggedLapsForDriver (
    int driverId,
    const std::vector<LapRecord>& allRecords,
    const std::unordered_map<int, double>& driverAvgs
) {
    std::vector<LapRecord> flaggedOnes;
    for (const auto& rec : allRecords)
    {
        if (rec.driverId != driverId)
            continue;
        AnomalyResult check = checkLapAnomaly (
            rec.milliseconds, 
            rec.driverId, 
            driverAvgs);
        if (check.isFlagged)
            flaggedOnes.push_back(rec);
    }
    return flaggedOnes;
}

void printAnomalyInfo (
    const AnomalyResult& result, 
    int lapTimeMs, 
    double driverAvgMs
) {
    double pctDisplay = result.pctDeviation * 100.0;
    std::cout << "  Driver avg:   " << std::fixed << std::setprecision(0) << driverAvgMs << " ms" << std::endl;
    std::cout << "  This lap:     " << lapTimeMs << " ms" << std::endl;
    std::cout << "  Deviation:    " << std::showpos << std::setprecision(1) << pctDisplay << "%" << std::noshowpos << std::endl;
    std::cout << "  Status:       " << result.label << std::endl;
}
// anomaly.cpp
