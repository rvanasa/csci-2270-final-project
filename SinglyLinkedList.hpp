#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include "Container.hpp"

/**
 * A minimal singly-linked list with front insertion.
 *
 * @tparam U is the type of element stored in the list
 */
template<class U>
class SinglyLinkedList : public Container<U> {
    struct node {
        U data;
        node *next;
    };

    unsigned listSize = 0;
    node *root = nullptr;

    void cleanup(node *t) {
        if (!t)return;
        cleanup(t->next);
        delete t;
    }

    bool contains(U x, node *t) const {
        if (!t) return false;
        return t->data == x || contains(x, t->next);
    }

    node *insert(U x, node *t) {
        listSize++;
        node *head = new node;
        head->data = x;
        head->next = t;
        return head;
    }

    node *remove(U x, node *t) {
        if (!t) return t;
        else if (t->data == x) {
            listSize--;
            return t->next;
        }
        t->next = remove(x, t->next);
        return t;
    }

public:
    explicit SinglyLinkedList() {
    }

    ~SinglyLinkedList() {
        cleanup(root);
    }

    bool contains(U x) const override {
        return contains(x, root);
    }

    bool insert(U x) override {
        root = insert(x, root);
        return true;
    }

    bool remove(U x) override {
        unsigned prevSize = listSize;
        root = remove(x, root);
        return listSize != prevSize;
    }
};

#endif