#include "sortlist.h"

// SortList
template<class T> SortList<T>::Node::Node(float t,T v): t(t), item(v) {}

template<class T> SortList<T>::SortList(): fir(NULL), las(NULL) {
    for(int i=0;i<SL_DIVS-1;i++) divs[i]=NULL;
}
template<class T> SortList<T>::~SortList() {
    clear();
}
template<class T> typename SortList<T>::Node* SortList<T>::add(float t,T v) {
    Node* N = new Node(t,v);
    add(N);
    return N;
}
template<class T> typename SortList<T>::Node* SortList<T>::add(Node* N) {
    N->t=N->t<0?0:N->t>1?1:N->t; // clamp t
    
    int sect = (int)(N->t*(SL_DIVS-0.0001));
    Node *p=NULL,*n;
    if(sect==0) n = fir; else n = divs[sect-1]; // assign first node
    if(n==NULL) n = fir; // if div was NULL, use first instead
    while(n!=NULL && n->t<N->t) {p=n; n=n->n;} // iterate until found
    
    if(p==NULL) {N->n = fir; fir = N;} // if first, assign as first
    else {N->n = n; p->n = N;} // if not first, insert (or append)
    if(n==NULL) las = N; // if last, assign as last

    // update divs
    for(int i=sect;i<SL_DIVS-1;i++) {
        if(divs[i]==NULL || divs[i]->t<N->t) divs[i]=N; // new node makes new beginning for section
        else break; // found the next node in line; no need to continue
    }

    return N;
}
template<class T> void SortList<T>::rem(Node* o) {
    pop(o);
    delete o;
}
template<class T> typename SortList<T>::Node* SortList<T>::pop(Node* o) {
    int sect = (int)(o->t*SL_DIVS);
    Node *p=NULL,*n;
    if(sect==0) n = fir; else n = divs[sect-1]; // assign first node
    if(n==NULL) n = fir; // if div was NULL, use first instead
    while(n!=NULL && n!=o) {p=n; n=n->n;} // iterate until found

    if(n==NULL) return NULL; // not found; shouldn't happen

    if(fir==o) fir = o->n; // was first
    if(las==o) las = p; // was last
    if(p!=NULL) p->n = o->n; // close gap

    // update divs
    for(int i=sect;i<SL_DIVS-1;i++) {
        if(divs[i]==o) divs[i]=p; // change div set on old node to previous node
        else break; // found the next node in line; no need to continue
    }

    return o;
}
template<class T> void SortList<T>::iterateall(SortList<T>::func_itemiterator cb) {
    Node* n = fir;
    while(n!=NULL) {
        cb(n->t,n->item);
        n = n->n;
    }
}
template<class T> typename SortList<T>::Node* SortList<T>::nearest(float t) {
    if(fir==NULL) return NULL; // list is empty

    int sect = (int)(t*SL_DIVS);
    Node *p=NULL,*n;
    if(sect==0) n = fir; else n = divs[sect-1]; // assign first node
    if(n==NULL) n = fir; // if div was NULL, use first instead
    while(n!=NULL && n->t<t) {p=n; n=n->n;} // iterate until found

    if(p==NULL) return fir; // t is before first
    if(n==NULL) return las; // t is after last

    return t-p->t < n->t-t ? p : n; // return closest t
}
template<class T> typename SortList<T>::Node* SortList<T>::before(float t) {
    if(fir==NULL) return NULL; // list is empty

    int sect = (int)(t*SL_DIVS);
    Node *p=NULL,*n;
    if(sect==0) n = fir; else n = divs[sect-1]; // assign first node
    if(n==NULL) n = fir; // if div was NULL, use first instead
    while(n!=NULL && n->t<t) {p=n; n=n->n;} // iterate until found

    return p==NULL?NULL:p;
}
template<class T> typename SortList<T>::Node* SortList<T>::after(float t) {
    if(fir==NULL) return NULL; // list is empty

    int sect = (int)(t*SL_DIVS);
    Node *n;
    if(sect==0) n = fir; else n = divs[sect-1]; // assign first node
    if(n==NULL) n = fir; // if div was NULL, use first instead
    while(n!=NULL && n->t<t) n=n->n; // iterate until found

    return n==NULL?NULL:n;
}
template<class T> typename SortList<T>::Node* SortList<T>::first() {
    return fir==NULL?NULL:fir;
}
template<class T> typename SortList<T>::Node* SortList<T>::last() {
    return las==NULL?NULL:las;
}
template<class T> typename SortList<T>::Node* SortList<T>::find(T v) {
    Node* c = fir;
    while(c) {
        if(c->item == v) return c;
        c = c->n;
    }
    return NULL;
}
template<class T> void SortList<T>::clear() {
    Node *n = fir,*p;
    while(n!=NULL) {
        p = n;
        n = n->n;
        delete p;
    }
    for(int i=0;i<SL_DIVS-1;i++)
        divs[i]=NULL;
    fir=las=NULL;
}
template<class T> void SortList<T>::destnode(Node* o) {
    delete o;
}