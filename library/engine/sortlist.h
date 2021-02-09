#ifndef __SORTLIST_H__
#define __SORTLIST_H__

#define SL_DIVS 16

template<class T> class SortList {
public:
    typedef void (*func_itemiterator)(float t,T v);

    struct Node {
        float t;
        Node* n;
        T item;
    private:
        Node(float t,T v);
        friend class SortList;
    };
    SortList();
    virtual ~SortList();

    Node* add(float t,T v);
    Node* add(Node* o);
    void rem(Node* o);
    Node* pop(Node* o);
    void iterateall(func_itemiterator fn);
    Node* nearest(float t);
    Node* before(float t);
    Node* after(float t);
    Node* first();
    Node* last();
    Node* find(T v);
    void clear();
    void destnode(Node* o); // destroy loose node

protected:
    Node *fir, *las;
    Node* divs[SL_DIVS-1];
};

#include "sortlist.tpp"

#endif // __SORTLIST_H__