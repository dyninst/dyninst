// simpSeq.C
// Ariel Tamches

/* $Log: simpSeq.C,v $
/* Revision 1.1  1995/07/17 04:59:00  tamches
/* First version of the new where axis
/*
 */

#include <assert.h>
#include "simpSeq.h"

template <class T>
simpSeq<T>::simpSeq() {
   numitems = 0;
}

template <class T>
simpSeq<T>::~simpSeq() {}

template <class T>
int simpSeq<T>::getSize() const {
   return numitems;
}

template <class T>
T &simpSeq<T>::getItem(const int index) {
   assert(0 <= index);
   assert(0 < numitems);
   return data[index];
}

template <class T>
T &simpSeq<T>::getLastItem() {
   assert(numitems > 0);
   return data[numitems-1];
}

template <class T>
const T &simpSeq<T>::getConstItem(const int index) const {
   assert(0 <= index);
   assert(0 < numitems);
   return data[index];
}

template <class T>
const T &simpSeq<T>::getConstLastItem() const {
   assert(numitems > 0);
   return data[numitems-1];
}

template <class T>
const T *simpSeq<T>::getEntireSeqQuick() const {
   // you'll need to call getSize() to properly make use of the results
   return data;
}

template <class T>
void simpSeq<T>::append(const T &newItem) {
   assert(0 <= numitems);
   assert(numitems < 20);

   data[numitems++] = newItem; // makes a copy of the item
}

template <class T>
void simpSeq<T>::replaceItem(const int index, const T &newItem) {
   assert(0 <= index);
   assert(index < numitems);
   data[index] = newItem;
}
