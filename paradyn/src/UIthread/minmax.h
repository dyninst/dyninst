// minmax.h

/* $Log: minmax.h,v $
/* Revision 1.2  1995/09/20 01:16:35  tamches
/* added ipmin and ipmax
/*
 * Revision 1.1  1995/07/17  04:53:21  tamches
 * First version of new where axis
 *
 */
#ifndef _MINMAX_H_
#define _MINMAX_H_

template <class T>
T min(const T item1, const T item2);

template <class T>
T max(const T item1, const T item2);

// "ip" stands for in-place.
// item = min(item, otherItem):
template <class T>
void ipmin(T &item, const T otherItem);

template <class T>
void ipmax(T &item, const T otherItem);

#endif
