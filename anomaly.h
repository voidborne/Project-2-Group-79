// anomaly.h
#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "shared_types.h"

// different anomoly levels
const double SLOW_LAP_THRESHOLD      = 0.05;
const double INCIDENT_THRESHOLD      = 0.15;
const double FAST_LAP_THRESHOLD      = 0.03;

struct AnomalyResult
{
    bool   isFlagged;
    double pctDeviation;
    std::string label;
};

// creates driver avg lap time map, should only be called once after data load
std::unordered_map<int, double> buildDriverAverages(
    const std::vector<LapRecord>& allRecords
);

//compare lap time to avg, gives label and percent diff
AnomalyResult checkLapAnomaly(
    int lapTimeMs, int driverId, 
    const std::unordered_map<int, 
    double>& driverAvgs
);

// go through records for a driver and return only flaged laps
std::vector<LapRecord> findFlaggedLapsForDriver(
    int driverId,
    const std::vector<LapRecord>& allRecords,
    const std::unordered_map<int, double>& driverAvgs
);

// print out the info
void printAnomalyInfo(
    const AnomalyResult& result, 
    int lapTimeMs, 
    double driverAvgMs
);
// anomaly.h
