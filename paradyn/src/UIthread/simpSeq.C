// simpSeq.C
// Ariel Tamches

/* $Log: simpSeq.C,v $
/* Revision 1.2  1995/09/20 01:19:16  tamches
/* int --> unsigned in a lot of places
/*
 * Revision 1.1  1995/07/17  04:59:00  tamches
 * First version of the new where axis
 *
 */

#include <assert.h>
#include "simpSeq.h"

template <class T>
unsigned simpSeq<T>::getSize() const {
   return numitems;
}

template <class T>
T &simpSeq<T>::getItem(unsigned index) {
   assert(index < numitems);
   return data[index];
}

template <class T>
const T &simpSeq<T>::getItem(unsigned index) const {
   assert(index < numitems);
   return data[index];
}

template <class T>
T &simpSeq<T>::getLastItem() {
   assert(numitems > 0);
   return data[numitems-1];
}

template <class T>
const T &simpSeq<T>::getLastItem() const {
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
   assert(numitems < 20);

   data[numitems++] = newItem; // makes a copy of the item
}

template <class T>
void simpSeq<T>::replaceItem(unsigned index, const T &newItem) {
   assert(index < numitems);
   data[index] = newItem;
}
