// minmax.C

/* $Log: minmax.C,v $
/* Revision 1.2  1995/09/20 01:16:36  tamches
/* added ipmin and ipmax
/*
 * Revision 1.1  1995/07/17  04:58:55  tamches
 * First version of the new where axis
 *
 */

#include "minmax.h"

template <class T>
T min(const T item1, const T item2) {
   if (item1 < item2) return item1;
   return item2;
}

template <class T>
T max(const T item1, const T item2) {
   if (item1 > item2) return item1;
   return item2;
}

template <class T>
void ipmin(T &item, const T otherItem) {
   if (otherItem < item) item = otherItem;
}

template <class T>
void ipmax(T &item, const T otherItem) {
   if (otherItem > item) item = otherItem;
}
