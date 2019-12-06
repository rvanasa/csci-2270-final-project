#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "Container.hpp"

/**
 * A superclass for containers utilizing a hash function.
 *
 * @tparam U is the type of element stored in the table
 * @tparam H is the hash function given the element and capacity
 */
template<class U, unsigned H(U, unsigned)>
class HashTable : public Container<U> {
protected:
    // Compute a hash from the output of `H` given the element and capacity, modulo table capacity
    unsigned hash(U item) const {
        return H(item, capacity()) % capacity();
    }
};

#endif