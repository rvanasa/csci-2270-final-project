#ifndef BUCKET_HASH_TABLE_H
#define BUCKET_HASH_TABLE_H

#include "HashTable.hpp"

/**
 * A hash table containing multiple "buckets" to resolve collisions.
 *
 * @tparam T is the type of data structure for each bucket
 * @tparam U is the type of element stored in the table
 * @tparam H is the hash function given the element and capacity
 * @tparam S is the number of buckets in the table
 */
template<class T, class U, unsigned H(U, unsigned), unsigned S>
class BucketHashTable : public HashTable<U, H> {
protected:
    T table[S];

public:
    unsigned capacity() const override {
        return S;
    }

    bool contains(U item) const override {
        return table[this->hash(item)].contains(item);
    }

    bool insert(U item) override {
        if (this->contains(item)) {
            return false;
        }
        table[this->hash(item)].insert(item);
        return true;
    }

    bool remove(U item) override {
        if (!this->contains(item)) {
            return false;
        }
        table[this->hash(item)].remove(item);
        return true;
    }
};

#endif