#ifndef __ARBLIST_H__
#define __ARBLIST_H__

/*
    ArbList<class>:
    Arbitrary List: A 1-dimensional list of items; maps any integer to an array of any choice type

    -- Designed for mapping objects by numbers for fast identification --
*/

template<class T> class ArbList {
public:
    class AllIterator;

private:
    struct Item;

    std::vector<AllIterator*> allIterators;
public:
    class AllIterator {
        ArbList<T>* parent;
        int index;
        Item* item;

    public:
        AllIterator();
        virtual ~AllIterator();

    private:
        void setParent(ArbList<T>* par);

        friend class ArbList<T>;
    };

    ArbList(int sz=1024);
    virtual ~ArbList();

    void set(int ind,T itm);
    T get(int ind);
    bool has(int ind);
    void remove(int ind);

    void clear();

    void iterateAllBegin(AllIterator& iter);
    T iterateAllNext(AllIterator& iter);

private:
    int size;

    struct Item {
        int ind;
        T item;
        ArbList::Item* next;
    } **items;
};

#include "arblist.tpp"

#endif // __ARBLIST_H__
