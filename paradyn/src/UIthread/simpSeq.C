/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// simpSeq.C
// Ariel Tamches

/* $Log: simpSeq.C,v $
/* Revision 1.4  1996/08/16 21:07:23  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.3  1995/10/17 22:09:28  tamches
 * Added operator==
 *
 * Revision 1.2  1995/09/20 01:19:16  tamches
 * int --> unsigned in a lot of places
 *
 * Revision 1.1  1995/07/17  04:59:00  tamches
 * First version of the new where axis
 *
 */

#include <assert.h>
#include "simpSeq.h"

template <class T>
bool simpSeq<T>::operator==(const simpSeq<T> &other) const {
   if (numitems != other.numitems)
      return false;

   for (unsigned i=0; i < numitems; i++)
      if (data[i] != other.data[i])
         return false;

   return true;
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
