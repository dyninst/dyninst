// PriorityQueue.h

/*
 * $Log: PriorityQueue.h,v $
 * Revision 1.1  1995/12/08 04:55:29  tamches
 * first version of priority queue class
 *
 */

#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include "util/h/Vector.h"

template <class KEY, class DATA>
// KEY must have operator<, operator>, etc. defined
// DATA can be arbitrary
class PriorityQueue {
 private:
   struct pair {
      KEY theKey;
      DATA theData;
      pair(const KEY &k, const DATA &d) : theKey(k), theData(d) {}
      pair() {} // required by Vector.h
   };

   vector<pair> data;

 private:
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
};

#endif
