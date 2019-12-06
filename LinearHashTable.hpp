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
        for (unsigned i = 0; i < prevSize; i++) {
            if (prevTable[i].first && !insert(prevTable[i].second)) {
                cout << "Warning: could not re-insert value into resized table" << endl;
            }
        }
        delete[] prevTable;
    }

    bool contains(U item) const override {
        // Cycle through table starting at the hash index
        auto h = this->hash(item);
        for (unsigned i = 0; i < this->capacity(); i++) {
            if (table[h].first && table[h].second == item) {
                // Found element
                return true;
            }
            h = (h + 1) % this->capacity();
        }
        return false;
    }

    bool insert(U item) override {
        // Cycle through table starting at the hash index
        auto h = this->hash(item);
        for (unsigned i = 0; i < this->capacity(); i++) {
            if (!table[h].first) {
                // Fill empty space
                table[h].first = true;
                table[h].second = item;
                return true;
            } else if (table[h].second == item) {
                // Cancel if the element already exists in the table
                return false;
            }
            h = (h + 1) % this->capacity();
        }
        // Double table capacity and re-attempt to insert
        resize(this->capacity());
        return insert(item);
    }

    bool remove(U item) override {
        // Cycle through table starting at the hash index
        auto h = this->hash(item);
        for (unsigned i = 0; i < this->capacity(); i++) {
            if (table[h].first && table[h].second == item) {
                // Remove element by disabling the existence flag in the pair
                table[h].first = false;
                return true;
            }
            h = (h + 1) % this->capacity();
        }
        return false;
    }
};

#endif