// Global imports
#include <iostream>

using std::cout;
using std::endl;

// Project-level imports
#include "BalancedTree.hpp"
#include "SinglyLinkedList.hpp"
#include "VectorList.hpp"
#include "BucketHashTable.hpp"
#include "LinearHashTable.hpp"
#include "CuckooTable.hpp"

// Standard library imports
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

const unsigned TABLE_SIZE = 10009;
const unsigned BATCH_SIZE = 100;

// h(x); modulo is implicitly applied by hash tables
inline unsigned hash1(int item, unsigned size) {
    return item;
}

// h'(x);
inline unsigned hash2(int item, unsigned size) {
    return item / size;
}

// h*(x); added for 3-table cuckoo hashing
inline unsigned hash3(int item, unsigned size) {
    return (item | (3 << 10)); // NOLINT(hicpp-signed-bitwise)
}

// h**(x); added for 4-table cuckoo hashing
inline unsigned hash4(int item, unsigned size) {
    return (item | (3 << 8)); // NOLINT(hicpp-signed-bitwise)
}

// Conditional hashing based on table index
inline unsigned multiHash(unsigned n, int item, unsigned size) {
    switch (n + 1) {
        case 1:
            return hash1(item, size);
        case 2:
            return hash2(item, size);
        case 3:
            return hash3(item, size);
        case 4:
            return hash4(item, size);
    }
    throw runtime_error("Unexpected hash function index");
}

// Load dataset from a given file name
vector<int> loadData(const string &filename) {
    vector<int> vec;
    ifstream input(filename);
    if (!input.is_open()) {
        throw runtime_error("Data file not found; check your current working directory");
    }

    string line, token;
    while (getline(input, line)) {
        stringstream ss(line);
        while (getline(ss, token, ',')) {
            vec.push_back(stoi(token));
        }
    }

    input.close();
    return vec;
}

// Global output stream for recording data
ofstream output; // NOLINT(cert-err58-cpp)
string outputDirectory;

// Helper method for formatting output file names
void replaceAll(string &str, const string &from, const string &to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

// Begin recording to a given file name
void startRecording(const string &filename) {
    string path = outputDirectory + "/" + filename + ".txt";
    replaceAll(path, " ", "_");
    replaceAll(path, "(", "");
    replaceAll(path, ")", "");
    replaceAll(path, "{", "");
    replaceAll(path, "}", "");
    replaceAll(path, ":", "");
    replaceAll(path, "'", "1");
    replaceAll(path, "*", "2");
    output.open(path);
    if (!output.good()) {
        throw runtime_error("Could not start recording");
    }
    output << "operation,load_factor,time,resize_count" << endl;
}

// Stop recording to the current file
void stopRecording() {
    output.close();
}

// Record an operation execution result
void record(const string &operation, double loadFactor, long long time, unsigned resizeCount) {
    output << operation << "," << loadFactor << "," << time << "," << resizeCount << endl;
}

// Time a specific operation and ensure correctness
template<class U, bool P(Container<U> &, U, bool), unsigned B>
void timeOperation(const vector<U> &data, vector<U> dupes, Container<U> &table, const string &label) {
    cout << "* " << label << ": ";

    // Only iterate elements up to a multiple of the provided batch size
    unsigned size = data.size();
    size -= size % B;

    vector<U> used;
    unsigned index = 0;
    long long overallTime = 0;
    unsigned batchCount = 0;
    unsigned resizeCount = 0;
    unsigned prevSize = table.capacity();
    while (index < size) {
        unsigned batchTime = 0;
        for (unsigned i = 0; i < B; i++) {
            U item = data[index];

            bool duplicate = false;
            for (U x : used) {
                if (item == x) {
                    duplicate = true;
                    break;
                }
            }
            for (unsigned j = 0; j < dupes.size(); j++) {
                // If the item is known to be a duplicate, move from `dupes` to `used`
                if (item == dupes[j]) {
                    used.push_back(dupes[j]);
                    dupes.erase(dupes.begin() + j);
                    break;
                }
            }

            // Compute the load factor based on the current index and duplicate cache
            unsigned capacity = table.capacity();
            double loadFactor = capacity ? (double) (index - used.size()) / capacity : 0;

            // Time the current operation
            auto start = high_resolution_clock::now();
            bool result = P(table, item, duplicate);
            auto stop = high_resolution_clock::now();

            // Calculate the precise execution time
            auto duration = duration_cast<nanoseconds>(stop - start);
            long long time = duration.count();

            if (!result) {
                // Notify if item was not inserted/contained/removed as logically expected
                cout << ">> unexpected (" << label << "): " << item << endl;
            }

            if (table.capacity() != prevSize) {
                // Table was resized since previous iteration
                resizeCount++;
                prevSize = table.capacity();
            }

            // Record execution details
            record(label, loadFactor, time, resizeCount);

            batchTime += (unsigned) time;
            index++;
        }
        overallTime += batchTime;
        batchCount++;
    }

    // Compute and display the average execution time per loop iteration
    auto time = overallTime / (batchCount * B);
    cout << time << " ns";
    if (resizeCount) {
        // Show the resize count only if the table has been resized
        cout << " (resizes: " << resizeCount << ")";
    }
    cout << endl;
}

// Returns true if the insertion result matches whether the item has been added before
template<class U>
bool doInsert(Container<U> &t, U item, bool duplicate) {
    return t.insert(item) != duplicate;
}

// Returns true if the item is contained in the table, regardless of duplicates
template<class U>
bool doContains(Container<U> &t, U item, bool duplicate) {
    return t.contains(item);
}

// Returns true if the deletion result matches whether the item has been removed before
template<class U>
bool doRemove(Container<U> &t, U item, bool duplicate) {
    return t.remove(item) != duplicate;
}

// Profiles a specific container
template<class U>
void profile(const vector<U> &data, const vector<U> &dupes, Container<U> &table, const string &label) {
    cout << endl;
    cout << "[" << label << "]" << endl;
    startRecording(label);
    timeOperation<U, doInsert, BATCH_SIZE>(data, dupes, table, "insert");
    timeOperation<U, doContains, BATCH_SIZE>(data, dupes, table, "contains");
    timeOperation<U, doRemove, BATCH_SIZE>(data, dupes, table, "remove");
    stopRecording();
}

// Profiles all tables which require a single hash function
template<class U, unsigned H(U, unsigned)>
void profileSingleHashFunction(const vector<U> &data, const vector<U> &dupes, const string &label) {
    {
        BucketHashTable<SinglyLinkedList<U>, U, H, TABLE_SIZE> table;
        profile(data, dupes, table, "linked list {" + label + "}");
    }
    {
        BucketHashTable<BalancedTree<U>, U, H, TABLE_SIZE> table;
        profile(data, dupes, table, "binary tree {" + label + "}");
    }
    {
        LinearHashTable<U, H> table(TABLE_SIZE);
        profile(data, dupes, table, "linear probing {" + label + "}");
    }
}

// Profiles all tables which require multiple or indexed hash functions
template<class U, unsigned H(unsigned, U, unsigned), unsigned N>
void profileMultiHashFunction(const vector<U> &data, const vector<U> &dupes, const string &label) {
    {
        CuckooTable<U, H, N> table(TABLE_SIZE);
        profile(data, dupes, table, "cuckoo hashing {" + label + "}");
    }
}

int main(int argc, char **argv) {
    // Retrieve dataset file path from first command line argument
    string inputPath = argc > 1 ? argv[1] : "data/dataSetA.csv";
    // Set global output directory from second command line argument
    outputDirectory = argc > 2 ? argv[2] : "output";

    cout << "Loading dataset: " << inputPath << endl;
    vector<int> data = loadData(inputPath);
    data.shrink_to_fit();

    cout << "Finding duplicates to verify correctness..." << endl;
    vector<int> dupes;
    for (unsigned i = 0; i < data.size(); i++) {
        for (unsigned j = 0; j < i; j++) {
            if (data[i] == data[j]) {
                bool already = false;
                for (int dupe : dupes) {
                    if (data[i] == dupe) {
                        already = true;
                        break;
                    }
                }
                if (!already) {
                    if (!dupes.empty()) {
                        cout << ", ";
                    }
                    cout << data[i];
                    dupes.push_back(data[i]);
                }
            }
        }
    }
    cout << endl;
    dupes.shrink_to_fit();

    cout << "Profiling containers..." << endl;

    // Define an alternate duplicate vector for containers which store duplicate elements
    vector<int> allowDupes;

    // Each container is scoped to automatically deallocate before the next evaluation
    {
        BalancedTree<int> tree;
        profile(data, dupes, tree, "baseline: balanced tree");
    }
    {
        SinglyLinkedList<int> list;
        profile(data, allowDupes, list, "baseline: linked list");
    }
    {
        VectorList<int> vec;
        profile(data, allowDupes, vec, "baseline: vector");
    }

    // Templates are used here for clarity and to allow additional compile-time optimizations
    profileSingleHashFunction<int, hash1>(data, dupes, "h(x)");
    profileSingleHashFunction<int, hash2>(data, dupes, "h'(x)");
    profileSingleHashFunction<int, hash3>(data, dupes, "h*(x)");

    // Using 3 hash functions improves the cuckoo table load factor from 50% to 90%
    profileMultiHashFunction<int, multiHash, 3>(data, dupes, "3");
    profileMultiHashFunction<int, multiHash, 4>(data, dupes, "4");
}
