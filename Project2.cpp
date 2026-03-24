// Project2.cpp
// main program file
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include "shared_types.h"
#include "QuadProbing.h"
#include "logger.h"
#include "tester.h"
#include "anomaly.h"

using namespace std;

class HashTable 
{
private:
    vector<LapRecord> table;
    int currentSize;
    int capacity;
    long long totalCollisions;

    
    int hashFunction(long long key) 
    {
        return key % capacity;
    }

public:
    HashTable(int cap) : capacity(cap), currentSize(0), totalCollisions(0) 
    {
        table.resize(capacity);
    }

    // linear probing 
    void insertLinear(const LapRecord& record) 
    {
        long long key = record.getCompositeKey();
        int index = hashFunction(key);
        int i = 0;

        while (table[(index + i) % capacity].isOccupied) 
        {
            totalCollisions++;
            i++;
            if (i >= capacity) return; // if table is full
        }

        int finalIndex = (index + i) % capacity;
        table[finalIndex] = record;
        table[finalIndex].isOccupied = true;
        currentSize++;
    }

    // 3. linear probe search
    int searchLinear(int rId, int dId, int lNum) 
    {
        LapRecord temp;
        temp.raceId = rId; temp.driverId = dId; temp.lap = lNum;
        long long key = temp.getCompositeKey();
        
        int index = hashFunction(key);
        int i = 0;

        while (table[(index + i) % capacity].isOccupied) 
        {
            if (table[(index + i) % capacity].getCompositeKey() == key) 
            {
                return table[(index + i) % capacity].milliseconds;
            }
            i++;
            if (i >= capacity) break;
        }
        return -1; // if nothing comes up
    }

    long long getCollisions()
    { 
        return totalCollisions; 
    }
};

// 4. csv parse
vector<LapRecord> parseCSV(string filename) 
{
    vector<LapRecord> data;
    ifstream file(filename);
    string line, word;

    if (!file.is_open()) 
    {
        cout << "File couldn't be opened." << endl;
        return data;
    }

    getline(file, line); // skip header

    while (getline(file, line)) 
    {
        stringstream ss(line);
        LapRecord r;
        string temp;

        getline(ss, temp, ','); r.raceId = stoi(temp);
        getline(ss, temp, ','); r.driverId = stoi(temp);
        getline(ss, temp, ','); r.lap = stoi(temp);
        getline(ss, temp, ',');
        getline(ss, temp, ',');
        getline(ss, temp, ','); r.milliseconds = stoi(temp);

        data.push_back(r);
    }
    file.close();
    return data;
}

// 5. ui and benchmark
// changed this to also do quadratic probing and return BenchmarkResult instead of just printing
BenchmarkResult runBenchmark(const vector<LapRecord>& rawData, double loadFactor)
{
    int tableSize = (int)(rawData.size() / loadFactor);

    // storeing everything in here so we can print the table later
    BenchmarkResult result;
    result.loadFactorUsed = loadFactor;
    result.tableSizeForRun = tableSize;

    cout << "\nLoad Factor: " << loadFactor << " (Capacity: " << tableSize << ")" << endl;

    // linear probing
    HashTable lpTable(tableSize);
    auto lpStart = chrono::high_resolution_clock::now();
    for (const auto& record : rawData)
        lpTable.insertLinear(record);
    auto lpEnd = chrono::high_resolution_clock::now();
    result.linearInsertTimeMs = chrono::duration_cast<chrono::milliseconds>(lpEnd - lpStart).count();
    result.linearCollisions = lpTable.getCollisions();

    int sampleSize = (int)rawData.size();
    if (sampleSize > 1000) sampleSize = 1000;

    auto lpSearchStart = chrono::high_resolution_clock::now();
    for (int i = 0; i < sampleSize; i++)
        lpTable.searchLinear(rawData[i].raceId, rawData[i].driverId, rawData[i].lap);
    auto lpSearchEnd = chrono::high_resolution_clock::now();
    result.linearAvgSearchTimeUs =
        (double)chrono::duration_cast<chrono::microseconds>(lpSearchEnd - lpSearchStart).count()
        / sampleSize;

    // quadratic probing
    QuadHashTable qpTable(tableSize);
    auto qpStart = chrono::high_resolution_clock::now();
    for (const auto& record : rawData)
        qpTable.putInTable(record);
    auto qpEnd = chrono::high_resolution_clock::now();
    result.quadInsertTimeMs = chrono::duration_cast<chrono::milliseconds>(qpEnd - qpStart).count();
    result.quadCollisions = qpTable.getCollisionTotal();
    auto qpSearchStart = chrono::high_resolution_clock::now();
    for (int i = 0; i < sampleSize; i++)
        qpTable.lookUpLap(rawData[i].raceId, rawData[i].driverId, rawData[i].lap);
    auto qpSearchEnd = chrono::high_resolution_clock::now();
    result.quadAvgSearchTimeUs =
        (double)chrono::duration_cast<chrono::microseconds>(qpSearchEnd - qpSearchStart).count()
        / sampleSize;
    cout << "  Linear  - Collisions: " << result.linearCollisions
         << "  Insert: " << result.linearInsertTimeMs << "ms"
         << "  Avg Search: " << fixed << setprecision(3) << result.linearAvgSearchTimeUs << "us" << endl;
    cout << "  Quad    - Collisions: " << result.quadCollisions
         << "  Insert: " << result.quadInsertTimeMs << "ms"
         << "  Avg Search: " << fixed << setprecision(3) << result.quadAvgSearchTimeUs << "us" << endl;
    cout << "------------------------------------------" << endl;
    return result;
}

// prints the comparison table from all the benchmark runs
// took me awhile to get the setw stuff right so the columns line up
void printResultsTable(const vector<BenchmarkResult>& allResults)
{
    cout << "\n========================================================================================================================" << endl;
    cout << "                                      BENCHMARK RESULTS - Linear vs Quadratic Probing" << endl;
    cout << "========================================================================================================================" << endl;

    // header row
    cout << left
         << setw(8)  << "Load"
         << setw(12) << "Capacity"
         << setw(20) << "Linear Collisions"
         << setw(19) << "Linear Insert(ms)"
         << setw(22) << "Linear AvgSearch(us)"
         << setw(20) << "Quad Collisions"
         << setw(18) << "Quad Insert(ms)"
         << setw(20) << "Quad AvgSearch(us)"
         << endl;
    cout << "------------------------------------------------------------------------------------------------------------------------" << endl;

    // a row for each load factor and aligned with the header
    for (const auto& r : allResults)
    {
        cout << left  << setw(8)  << fixed << setprecision(1) << r.loadFactorUsed
             << right << setw(10) << r.tableSizeForRun
             << right << setw(18) << r.linearCollisions
             << right << setw(17) << r.linearInsertTimeMs
             << right << setw(20) << setprecision(3) << r.linearAvgSearchTimeUs
             << right << setw(18) << r.quadCollisions
             << right << setw(16) << r.quadInsertTimeMs
             << right << setw(20) << r.quadAvgSearchTimeUs
             << endl;
    }
    cout << "========================================================================================================================\n" << endl;
}

int main()
{
    vector<LapRecord> rawData;
    vector<BenchmarkResult> savedResults;
    unordered_map<int, double> driverAvgs;
    int choice;
    RunLogger appLog("run_log.txt");

    while (true)
    {
        cout << "\n==========================================" << endl;
        cout << "    F1 Anomaly Detection" << endl;
        cout << "==========================================" << endl;
        cout << "1. Load F1 Dataset (lap_times.csv)" << endl;
        cout << "2. Run Benchmarks (Linear vs Quadratic)" << endl;
        cout << "3. Search for a Specific Lap (+ Anomaly Check)" << endl;
        cout << "4. Show Results Table" << endl;
        cout << "5. Scan Driver for Flagged Laps" << endl;
        cout << "6. Run Tests" << endl;
        cout << "7. Exit" << endl;
        cout << "Selection: ";
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1)
        {
            appLog.write("Loading data...");
            rawData = parseCSV("lap_times.csv");
            cout << "Successfully loaded " << rawData.size() << " records." << endl;
            appLog.writeLabeled("Records loaded", (long long)rawData.size());
            driverAvgs = buildDriverAverages(rawData);
            cout << "Computed averages for " << driverAvgs.size() << " drivers." << endl;
        }
        else if (choice == 2)
        {
            if (rawData.empty())
            {
                cout << "Load data first." << endl; continue;
            }

            savedResults.clear();
            appLog.write("Starting benchmarks...");

            double factors[] = {0.5, 0.6, 0.7, 0.8, 0.9};
            for (double f : factors)
            {
                BenchmarkResult oneRun = runBenchmark(rawData, f);
                savedResults.push_back(oneRun);

                // log the collision numbers for each run
                appLog.writeLabeled("  load factor", f);
                appLog.writeLabeled("    linear collisions", oneRun.linearCollisions);
                appLog.writeLabeled("    quad collisions", oneRun.quadCollisions);
            }
            cout << "\nDone. Use option 4 to see the full table." << endl;
        }
        else if (choice == 3)
        {
            if (rawData.empty())
            {
                cout << "Load data first." << endl; continue;
            }

            int r, d, l;
            cout << "Enter Race ID, Driver ID, and Lap Number (3 numbers separated by spaces): ";
            if (!(cin >> r >> d >> l))
            {
                // check to see if they entered anything
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Need all 3 numbers" << endl;
                continue;
            }

            // search logic here
            int searchTableSize = (int)(rawData.size() / 0.7);
            HashTable searchLP(searchTableSize);
            QuadHashTable searchQP(searchTableSize);
            for (const auto& rec : rawData)
            {
                searchLP.insertLinear(rec);
                searchQP.putInTable(rec);
            }
            int lpTime = searchLP.searchLinear(r, d, l);
            int qpTime = searchQP.lookUpLap(r, d, l);
            if (lpTime == -1 && qpTime == -1)
            {
                cout << "Lap not found." << endl;
            }
            else
            {
                int lapTime = (lpTime != -1) ? lpTime : qpTime;
                cout << "\n  Lap time:     " << lapTime << " ms (" << fixed << setprecision(3) << lapTime / 1000.0 << " s)" << endl;
                cout << "  Found by linear: " << (lpTime != -1 ? "yes" : "no") << "  |  Found by quad: " << (qpTime != -1 ? "yes" : "no") << endl;

                // compare this lap against the driver's average.
                if (!driverAvgs.empty())
                {
                    AnomalyResult anomaly = checkLapAnomaly(lapTime, d, driverAvgs);
                    printAnomalyInfo(anomaly, lapTime, driverAvgs.at(d));
                }
                else
                {
                    cout << "  (load dataset first for anomaly info)" << endl;
                }
            }
        }
        else if (choice == 4)
        {
            if (savedResults.empty())
                cout << "Run the benchmark first with option 2." << endl;
            else
                printResultsTable(savedResults);
        }
        else if (choice == 5)
        {
            if (rawData.empty())
            {
                cout << "Load data first." << endl; continue;
            }
            int driverId;
            cout << "Enter Driver ID: ";
            if (!(cin >> driverId))
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input." << endl;
                continue;
            }

            // find all laps for this driver that are not normal
            vector<LapRecord> flaggedLaps = findFlaggedLapsForDriver(driverId, rawData, driverAvgs);
            if (flaggedLaps.empty())
            {
                cout << "There are no flagged laps for the driver " << driverId << "." << endl;
            }
            else
            {
                cout << "\nFlagged laps for driver " << driverId
                     << " (avg: " << fixed << setprecision(0)
                     << driverAvgs.at(driverId) << " ms):" << endl;
                cout << left << setw(10) << "Race"
                     << setw(8)  << "Lap"
                     << setw(14) << "Time(ms)"
                     << setw(10) << "Deviation"
                     << "Status" << endl;
                cout << "----------------------------------------------------" << endl;

                for (const auto& rec : flaggedLaps)
                {
                    AnomalyResult a = checkLapAnomaly(rec.milliseconds, rec.driverId, driverAvgs);
                    cout << left  << setw(10) << rec.raceId
                         << setw(8)  << rec.lap
                         << setw(14) << rec.milliseconds
                         << right << setw(8) << showpos << fixed << setprecision(1)
                         << (a.pctDeviation * 100.0) << "%" << noshowpos
                         << "  " << a.label << endl;
                }
                cout << "\n" << flaggedLaps.size() << " flagged laps found." << endl;
            }
        }
        else if (choice == 6)
        {
            runAllTests(rawData);
        }
        else if (choice == 7) break;
    }
    return 0;
}
// Project2.cpp
