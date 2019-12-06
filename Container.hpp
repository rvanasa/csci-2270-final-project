#ifndef CONTAINER_H
#define CONTAINER_H

/**
 * A common wrapper interface for container-like data structures.
 *
 * @tparam U is the type of element in this container
 */
template<class U>
class Container {
public:
    /**
     * @return the pre-allocated capacity of this data structure if relevant, otherwise 0
     */
    virtual unsigned capacity() const {
        return 0;
    }

    virtual bool contains(U) const = 0;

    virtual bool insert(U) = 0;

    virtual bool remove(U) = 0;
};

#endif