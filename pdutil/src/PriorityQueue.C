/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// PriorityQueue.C

/*
 * $Log: PriorityQueue.C,v $
 * Revision 1.8  2004/05/17 00:50:57  pcroth
 * Fixed syntax for gcc 3.4.0.
 *
 * Revision 1.7  2004/03/23 01:12:41  eli
 * Updated copyright string
 *
 * Revision 1.6  2002/12/20 07:50:08  jaw
 * This commit fully changes the class name of "vector" to "pdvector".
 *
 * A nice upshot is the removal of a bunch of code previously under the flag
 * USE_STL_VECTOR, which is no longer necessary in many cases where a
 * functional difference between common/h/Vector.h and stl::vector was
 * causing a crash.
 *
 * Generally speaking, Dyninst and Paradyn now use pdvector exclusively.
 * This commit DOES NOT cover the USE_STL_VECTOR flag, which will now
 * substitute stl::vector for BPatch_Vector only.  This is currently, to
 * the best of my knowledge, only used by DPCL.  This will be updated and
 * tested in a future commit.
 *
 * The purpose of this, again, is to create a further semantic difference
 * between two functionally different classes (which both have the same
 * [nearly] interface).
 *
 * Revision 1.5  2000/07/28 17:22:35  pcroth
 * Updated #includes to reflect util library split
 *
 * Revision 1.4  1996/11/26 16:09:33  naim
 * Fixing asserts - naim
 *
 * Revision 1.3  1996/08/16 21:31:46  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.2  1995/12/08 21:30:40  tamches
 * added printing (for debugging purposes)
 *
 * Revision 1.1  1995/12/08 04:55:41  tamches
 * first version of priority queue class
 *
 */

#include <assert.h>
#include "pdutil/h/PriorityQueue.h"

template <class KEY, class DATA>
PriorityQueue<KEY, DATA>::PriorityQueue() {
   // data is left an empty array
}

template <class KEY, class DATA>
PriorityQueue<KEY, DATA>::PriorityQueue(const PriorityQueue<KEY, DATA> &src) :
                      data(src.data) {
}

template <class KEY, class DATA>
PriorityQueue<KEY, DATA>::~PriorityQueue() {
}

template <class KEY, class DATA>
PriorityQueue<KEY, DATA> &
PriorityQueue<KEY, DATA>::operator=(const PriorityQueue<KEY, DATA> &src) {
   data = src.data;
   return *this;
}

/* **************************************************************** */

template <class KEY, class DATA>
unsigned PriorityQueue<KEY, DATA>::size() const {
   return data.size();
}

template <class KEY, class DATA>
bool PriorityQueue<KEY, DATA>::empty() const {
   return data.size()==0;
}

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::add(const KEY &k, const DATA &d) {
   TYPENAME PriorityQueue<KEY, DATA>::pair p(k, d);
   data += p;

   // our situation is as follows:
   // the heap is OK except perhaps for the last (newly added) item,
   // which may not be in proper heap order.
   assert(size() > 0);
   const unsigned index = size() - 1;
   reheapify_with_parent(index);
}

template <class KEY, class DATA>
const KEY &PriorityQueue<KEY, DATA>::peek_first_key() const {
   bool aflag;
   aflag=!empty();
   assert(aflag);
   return data[0].theKey;
}

template <class KEY, class DATA>
const DATA &PriorityQueue<KEY, DATA>::peek_first_data() const {
   bool aflag;
   aflag=!empty();
   assert(aflag);
   return data[0].theData;
}

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::delete_first() {
   bool aflag;
   aflag=!empty();
   assert(aflag);

   if (size()==1) {
      // the queue will now be empty
      data.resize(0);
      return;
   }

   // swap first item with last item; fry last item (used to be first);
   // reheapify first item (used to be last)
   assert(size() > 1);

   swap_items(0, size()-1);

   data.resize(size()-1);

   // our situation: the top item in the heap may be out of heap
   // order w.r.t. its children
   reheapify_with_children(0);
}

/* **************************************************************************** */

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::swap_items(unsigned index1, unsigned index2) {
   TYPENAME PriorityQueue<KEY, DATA>::pair temp = data[index1];
   data[index1] = data[index2];
   data[index2] = temp;
}

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::reheapify_with_parent(unsigned index) {
   // item w/ given index may not be in proper heap order; make it so
   // in time O(log(size())) by moving the item upwards as far as necessary

   while (index > 0) {
      unsigned parentIndex = (index-1) / 2;

      if (data[index].theKey < data[parentIndex].theKey) {
         // out of order; swap
         swap_items(parentIndex, index);

         index = parentIndex;
      }
      else
         break;
   }
}

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::reheapify_with_children(unsigned index) {
   // item w/ given index may not be in proper heap order; make it so
   // in time O(log(size())) by moving the item downwards as far as necessary

   while (true) {
      // does at least 1 child exist?
      unsigned child1index = 2*index + 1;
      unsigned child2index = 2*index + 2;

      if (child1index >= size())
         break; // neither child is in range

      // invariant: at least one child (child1index) is in range
      unsigned smallerChildIndex = child1index;

      if (child2index < size())
         // the second child exists; perhaps its key is the smaller of the 2 children
         if (data[child2index].theKey < data[child1index].theKey)
            smallerChildIndex = child2index;

      if (data[smallerChildIndex].theKey < data[index].theKey) {
         swap_items(index, smallerChildIndex);

         index = smallerChildIndex;
      }
      else
         break;
   }
}

template <class KEY, class DATA>
int PriorityQueue<KEY, DATA>::cmpfunc(const void *ptr1, const void *ptr2) {
   const TYPENAME PriorityQueue<KEY, DATA>::pair &pair1 = 
        *(const TYPENAME PriorityQueue<KEY, DATA>::pair *)ptr1;
   const TYPENAME PriorityQueue<KEY, DATA>::pair &pair2 = 
        *(const TYPENAME PriorityQueue<KEY, DATA>::pair *)ptr2;

   if (pair1 < pair2)
      return -1;
   else if (pair1 > pair2)
      return 1;
   else
      return 0;
}

template <class KEY, class DATA>
ostream &PriorityQueue<KEY, DATA>::operator<<(ostream &os) const {
   // we need to walk the priority queue (in key order), printing each item.
   // However, this is surprisingly hard.  We end up needing to make a copy
   // of the array and sorting it...

   pdvector<TYPENAME PriorityQueue<KEY,DATA>::pair> copy = data;
   copy.sort(cmpfunc);

   for (unsigned i=0; i < copy.size(); i++) {
      const TYPENAME PriorityQueue<KEY, DATA>::pair &p = copy[i];

      os << "[" << i << "]: ";
      p.operator<<(os);
      os << endl;
   }

   return os;
}

template <class KEY, class DATA>
ostream &operator<<(ostream &os, PriorityQueue<KEY, DATA> &q) {
   return q.operator<<(os);
}
