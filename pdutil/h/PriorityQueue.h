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

// PriorityQueue.h

/* $Id: PriorityQueue.h,v 1.8 2003/07/18 15:45:05 schendel Exp $ */

// Note: in this priority queue class, the item with the _smallest_ key
//       will be the first item.

// class KEY must have the following defined:
// -- operators < and >  (very important; determines the heap ordering)
// -- the usual copy constructors and assignment (single-=) operators
//    that any well-written class will have
// -- operator<< for writing to an ostream&
//
// class DATA must have the following defined:
// -- same as above except no < or > are needed

#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include <iostream>
#include "common/h/Vector.h"
#include "common/h/std_namesp.h"

template <class KEY, class DATA>
class PriorityQueue {
 private:
   struct pair {
      KEY theKey;
      DATA theData;
      pair(const KEY &k, const DATA &d) : theKey(k), theData(d) {}
      pair() {} // required by Vector.h
      bool operator<(const pair &other) const {return theKey < other.theKey;}
      bool operator>(const pair &other) const {return theKey > other.theKey;}
      ostream &operator<<(ostream &os) const {
         os << theKey << ',' << theData;
         return os;
      }
   };

   pdvector<pair> data;

 private:
   static int cmpfunc(const void *ptr1, const void *ptr2);
      // for sorting w/in operator<<()

   void swap_items(unsigned index1, unsigned index2);

   void reheapify_with_parent(unsigned index);
      // item w/ given index may not be in proper heap order; make it so
      // in time O(log(size())) by moving the item upwards as far as necessary

   void reheapify_with_children(unsigned index);
      // item w/ given index may not be in proper heap order; make it so
      // in time O(log(size())) by moving the item downwards as far as necessary

 public:
   PriorityQueue();
   PriorityQueue(const PriorityQueue &src);
  ~PriorityQueue();

   PriorityQueue &operator=(const PriorityQueue &src);

   unsigned size() const; // returns # items in the queue
   bool empty() const; // returns true iff empty queue

   void add(const KEY &, const DATA &);

   // Peek at the first elem in the heap (bomb if heap is empty):
   const KEY  &peek_first_key()  const;
   const DATA &peek_first_data() const;
   
   void delete_first(); // bombs if empty

   ostream &operator<<(ostream &os) const; // just for debugging, nach...

   friend ostream &operator<<(ostream &os, PriorityQueue<KEY, DATA> &q);
};

#endif
