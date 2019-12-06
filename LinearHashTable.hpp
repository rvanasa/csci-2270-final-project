#ifndef LINEAR_HASH_TABLE_H
#define LINEAR_HASH_TABLE_H

#include "HashTable.hpp"

#include <utility>

using std::pair;

/**
 * A hash table which resolves collisions via linear probing.
 *
 * @tparam U is the type of element stored in the table
 * @tparam H is the hash function given the element and capacity
 */
template<class U, unsigned H(U, unsigned)>
class LinearHashTable : public HashTable<U, H> {
protected:
    int tableSize;
    pair<bool, U> *table;

public:
    explicit LinearHashTable(unsigned size) {
        tableSize = size;
        table = new pair<bool, U>[size];
    }

    ~LinearHashTable() {
        delete[] table;
    }

    unsigned capacity() const override {
        return tableSize;
    }

    void resize(int size) {
        unsigned prevSize = tableSize;
        pair<bool, U> *prevTable = table;
        tableSize = size;
        table = new pair<bool, U>[tableSize];
        for (int i = 0; i < prevSize; i++) {
            if (prevTable[i].first && !insert(prevTable[i].second)) {
                cout << "Warning: could not re-insert value into resized table" << endl;
            }
        }
        delete[] prevTable;
    }

    bool contains(U item) const override {
        auto h = hash(item);
        for (unsigned i = 0; i < capacity(); i++) {
            if (table[h].first && table[h].second == item) {
                return true;
            }
            h = (h + 1) % capacity();
        }
        return false;
    }

    bool insert(U item) override {
        auto h = hash(item);
        for (unsigned i = 0; i < capacity(); i++) {
            if (!table[h].first) {
                table[h].first = true;
                table[h].second = item;
                return true;
            } else if (table[h].second == item) {
                return false;
            }
            h = (h + 1) % capacity();
        }
        resize(capacity() * 2);
        return insert(item);
    }

    bool remove(U item) override {
        auto h = hash(item);
        for (unsigned i = 0; i < capacity(); i++) {
            if (table[h].first) {
                if (table[h].second == item) {
                    table[h].first = false;
                    return true;
                }
            }
            h = (h + 1) % capacity();
        }
        return false;
    }
};

#endif