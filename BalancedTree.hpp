#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "Container.hpp"

#include <set>

/**
 * A wrapper for `std::set`, which is generally implemented as a red-black tree or similar.
 *
 * @tparam U is the type of element stored in the tree
 */
template<class U>
class BalancedTree : public Container<U> {
    std::set<U> tree;

public:
    bool contains(U x) const override {
        return tree.find(x) != tree.end();
    }

    bool insert(U x) override {
        unsigned prevSize = tree.size();
        tree.insert(x);
        return tree.size() != prevSize;
    }

    bool remove(U x) override {
        unsigned prevSize = tree.size();
        tree.erase(x);
        return tree.size() != prevSize;
    }
};

#endif