#include <cstring>
#include "arblist.h"

template<class T> ArbList<T>::AllIterator::AllIterator(): parent(NULL) {}

template<class T> ArbList<T>::AllIterator::~AllIterator() {
    setParent(NULL);
}

template<class T> void ArbList<T>::AllIterator::setParent(ArbList<T>* par) {
    if(parent == par) return;

    if(parent != NULL) {
        auto pos = std::find(parent->allIterators.begin(), parent->allIterators.end(), this);
        if(pos != parent->allIterators.end())
            parent->allIterators.erase(pos);
    }

    parent = par;

    if(parent != NULL)
        parent->allIterators.push_back(this);
}

template<class T> ArbList<T>::ArbList(int sz): size(sz) {
    size = size<1 ? 1 : (size>67108864 ? 67108864 : size); // limit size; technically, true working max is 46340, but 67108864 should ever be more than sufficient

    items = new Item*[size];
    memset(items,0,size * sizeof(Item*));
}

template<class T> ArbList<T>::~ArbList() {
    clear();

    for(auto iter: allIterators)
        iter->parent = NULL;

    delete[] items;
}

template<class T> void ArbList<T>::set(int ind,T itm) {
    Item **ptr = &items[(ind % size + size) % size],
         *current = *ptr;

    while(current != NULL) {
        if(current->ind == ind) {
            current->item = itm;
            return;
        }
        ptr = &current->next;
        current = *ptr;
    }
    *ptr = new Item;
    current = *ptr;
    current->ind = ind;
    current->item = itm;
    current->next = NULL;
}

template<class T> T ArbList<T>::get(int ind) {
    Item *current = items[(ind % size + size) % size];

    while(current != NULL) {
        if(current->ind == ind)
            return current->item;
        current = current->next;
    }

    return T();
}

template<class T> bool ArbList<T>::has(int ind) {
    Item *current = items[(ind % size + size) % size];

    while(current != NULL) {
        if(current->ind == ind)
            return true;
        current = current->next;
    }

    return false;
}

template<class T> void ArbList<T>::remove(int ind) {
    Item **ptr = &items[(ind % size + size) % size],
         *current = *ptr;

    while(current != NULL) {
        if(current->ind == ind) {
            for(auto iter: allIterators) {
                if(iter->item == current)
                    iter->item = current->next;
            }
            *ptr = current->next;
            delete current;
            break;
        }
        ptr = &current->next;
        current = *ptr;
    }
}

template<class T> void ArbList<T>::clear() {
    for(auto iter: allIterators) {
        iter->item = NULL;
    }

    for(int i=0; i<size; i++) { // destroy each item-set
        Item *curitem = items[i];
        items[i] = NULL;

        while(curitem != NULL) { // destroy each item reference in a cell
            Item *pitem = curitem;
            curitem = curitem->next;
            delete pitem;
        }
    }
}

template<class T> void ArbList<T>::iterateAllBegin(AllIterator& iter) {
    iter.index = -1;
    iter.item = NULL;
    iter.setParent(this);
}

template<class T> T ArbList<T>::iterateAllNext(AllIterator& iter) {
    while(1) {
        if(iter.item == NULL) {
            iter.index++;
            if(iter.index >= size)
                return T();

            iter.item = items[iter.index];
            continue;
        }
        T& cur = iter.item->item;
        iter.item = iter.item->next;
        return cur;
    }
}
