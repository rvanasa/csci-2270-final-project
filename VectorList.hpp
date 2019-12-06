#ifndef VECTOR_LIST_H
#define VECTOR_LIST_H

#include "Container.hpp"

#include <vector>
#include <algorithm>

using std::vector;
using std::find;

/**
 * A wrapper for common `std::vector` operations.
 *
 * @tparam U is the type of element stored in the vector
 */
template<class U>
class VectorList : public Container<U> {
    vector<U> vec;

public:
    unsigned capacity() const {
        return vec.capacity();
    }

    bool contains(U x) const override {
        return find(vec.begin(), vec.end(), x) != vec.end();
    }

    bool insert(U x) override {
        vec.push_back(x);
        return true;
    }

    bool remove(U x) override {
        for (unsigned i = 0; i < vec.size(); i++) {
            if (vec[i] == x) {
                vec.erase(vec.begin() + i);
                return true;
            }
        }
        return false;
    }
};

#endif