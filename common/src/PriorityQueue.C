// PriorityQueue.C

/*
 * $Log: PriorityQueue.C,v $
 * Revision 1.1  1995/12/08 04:55:41  tamches
 * first version of priority queue class
 *
 */

#include <assert.h>
#include "util/h/PriorityQueue.h"

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
   PriorityQueue<KEY, DATA>::pair p(k, d);
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
   assert(!empty());
   return data[0].theKey;
}

template <class KEY, class DATA>
const DATA &PriorityQueue<KEY, DATA>::peek_first_data() const {
   assert(!empty());
   return data[0].theData;
}

template <class KEY, class DATA>
void PriorityQueue<KEY, DATA>::delete_first() {
   assert(!empty());

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
   PriorityQueue<KEY, DATA>::pair temp = data[index1];
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

