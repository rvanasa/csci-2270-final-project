#ifndef CUCKOO_TABLE_H
#define CUCKOO_TABLE_H

#include "Container.hpp"

#include <utility>
#include <vector>
#include <cmath>

using std::pair;
using std::vector;
using std::log2;

/**
 * An implementation of an N-table cuckoo hash table.
 *
 * @tparam U is the type of element stored in the table
 * @tparam H is the hash function given the table index, element, and capacity
 * @tparam N is the number of tables (usually 2 or 3)
 */
template<class U, unsigned H(unsigned, U, unsigned), unsigned N>
class CuckooTable : public Container<U> {
protected:
    int tableSize;
    pair<bool, U> *tables[N];

    // Compute a hash from the output of `H` given the table index and element, modulo table capacity
    unsigned hash(unsigned n, U item) const {
        return H(n, item, this->capacity()) % this->capacity();
    }

public:
    explicit CuckooTable(int size) {
        tableSize = size;
        for (unsigned n = 0; n < N; n++) {
            tables[n] = new pair<bool, U>[size];
        }
    }

    ~CuckooTable() {
        for (auto table : tables) {
            delete[] table;
        }
    }

    unsigned capacity() const override {
        return tableSize;
    }

    void resize(unsigned size) {
        unsigned prevSize = tableSize;
        tableSize = size;

        // Move previous tables to a temporary array, and allocate new tables
        pair<bool, U> *prevTables[N];
        for (unsigned n = 0; n < N; n++) {
            prevTables[n] = tables[n];
            tables[n] = new pair<bool, U>[tableSize];
        }

        // Re-insert items, and store fail cases in a vector to reduce intermediate memory consumption
        vector<U> unplaced;
        for (unsigned i = 0; i < prevSize; i++) {
            for (auto table : prevTables) {
                auto &pair = table[i];
                if (pair.first && !tryInsert(pair.second)) {
                    unplaced.push_back(pair.second);
                }
            }
        }
        // Clean up previous tables
        for (auto table : prevTables) {
            delete[] table;
        }
        for (U x : unplaced) {
            // If unable to re-insert again, display a warning
            if (!insert(x)) {
                cout << "Warning: failed to re-insert unplaced value" << endl;
            }
        }
    }

    bool contains(U item) const override {
        for (unsigned n = 0; n < N; n++) {
            auto i = hash(n, item);
            if (tables[n][i].first && tables[n][i].second == item) {
                return true;
            }
        }
        return false;
    }

    bool insert(U item) override {
        if (contains(item)) {
            return false;
        }
        if (!tryInsert(item)) {
            // Increase table capacity by a factor of 1.5
            resize(tableSize + tableSize / 2);
            return insert(item);
        }
        return true;
    }

    bool remove(U item) override {
        for (unsigned n = 0; n < N; n++) {
            auto i = hash(n, item);
            auto &pair = tables[n][i];
            if (pair.first && pair.second == item) {
                pair.first = false;
                return true;
            }
        }
        return false;
    }

private:
    bool tryInsert(U item) {
        return tryInsert(item, (unsigned)log2(tableSize) * 2 / N);
    }

    bool tryInsert(U item, unsigned remaining) {
        if (!remaining) {
            return false;
        }

        // Try to fill the first available empty slot
        for (unsigned n = 0; n < N; n++) {
            auto i = hash(n, item);
            auto &pair = tables[n][i];
            if (!pair.first) {
                pair.first = true;
                pair.second = item;
                return true;
            }
        }

        // Try to push the next item to an alternate table
        auto i = hash(0, item);
        auto &pair = tables[0][i];
        if (tryInsert(pair.second, remaining - 1)) {
            pair.second = item;
            return true;
        }

        // Notify rebuild
        return false;
    }
};

#endif