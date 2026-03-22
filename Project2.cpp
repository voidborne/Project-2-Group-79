#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;


struct LapRecord 
{
    int raceId;
    int driverId;
    int lap;
    int milliseconds;
    bool isOccupied = false;

    // using comp key to make it easier to hash (turns ids into single index)
    
    long long getCompositeKey() const 
    {
        return (long long)raceId * 1000000 + (long long)driverId * 1000 + lap;
    }
};

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
        getline(ss, temp, ','); r.milliseconds = stoi(temp);

        data.push_back(r);
    }
    file.close();
    return data;
}

// 5. ui and benchmark
void runBenchmark(const vector<LapRecord>& rawData, double loadFactor) 
{
    int tableSize = (int)(rawData.size() / loadFactor);
    HashTable lpTable(tableSize);

    cout << "\nLoad Factor: " << loadFactor << " (Capacity: " << tableSize << ")" << endl;

    auto start = chrono::high_resolution_clock::now();
    for (const auto& record : rawData) 
    {
        lpTable.insertLinear(record);
    }
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Linear Probing - Collisions: " << lpTable.getCollisions() << endl;
    cout << "Linear Probing - Insertion Time: " << duration.count() << "ms" << endl;
    cout << "------------------------------------------" << endl;
}

int main() 
{
    vector<LapRecord> rawData;
    int choice;

    while (true) 
    {
        cout << "\n==========================================" << endl;
        cout << "    F1 Anomaly Detection" << endl;
        cout << "==========================================" << endl;
        cout << "1. Load F1 Dataset (lap_times.csv)" << endl;
        cout << "2. Run Benchmarks (0.5 to 0.9 Load)" << endl;
        cout << "3. Check Lap for Anomaly (Manual Search)" << endl;
        cout << "4. Exit" << endl;
        cout << "Selection: ";
        cin >> choice;

        if (choice == 1) 
        {
            cout << "Loading data..." << endl;
            rawData = parseCSV("lap_times.csv");
            cout << "Successfully loaded " << rawData.size() << " records." << endl;
        } 
        else if (choice == 2) 
        {
            if (rawData.empty()) 
            { 
                cout << "Load data first." << endl; continue; 
            }

            double factors[] = {0.5, 0.6, 0.7, 0.8, 0.9};
            for (double f : factors) 
            {
                runBenchmark(rawData, f);
            }
        }
        else if (choice == 3) 
        {
            
            int r, d, l;
            cout << "Enter Race ID, Driver ID, and Lap Number: ";
            cin >> r >> d >> l;
            // search logic here
        }
        else if (choice == 4) break;
    }
    return 0;
}