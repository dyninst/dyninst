/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BPatch_Set_h_
#define _BPatch_Set_h_

/*******************************************************/
/*		header files 			       */
/*******************************************************/

#include <assert.h>
#include <stdlib.h>
#include <set>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>
#if !defined(DO_INLINE_P)
#define DO_INLINE_P
#endif

#if !defined(DO_INLINE_F)
#define DO_INLINE_F
#endif



class BPatch_basicBlock;
class BPatch_function;
class BPatch_flowGraph;
class BPatch;

/** template struct that will be used for default compare 
  * class for BPatch_Set operations.
  */

template <class T>
struct comparison {
   bool operator() (const T&l, const T&r) const { return l < r; }
};

template<class Key, class Compare = comparison<Key> >
class BPATCH_DLL_EXPORT BPatch_Set {
   friend class BPatch_basicBlock;
   friend class BPatch_function;
   friend class BPatch_flowGraph;
   friend class BPatch;

   typedef std::set<Key, Compare> int_t;
   int_t int_set;

  public:
   typedef typename int_t::iterator iterator;
   typedef typename int_t::const_iterator const_iterator;

   typedef typename int_t::reverse_iterator reverse_iterator;
   typedef typename int_t::const_reverse_iterator const_reverse_iterator;
 
   DO_INLINE_F iterator begin() { return int_set.begin(); }
   DO_INLINE_F const_iterator begin() const { return int_set.begin(); }
   DO_INLINE_F iterator end() { return int_set.end(); }
   DO_INLINE_F const_iterator end() const { return int_set.end(); }

   DO_INLINE_F reverse_iterator rbegin() { return int_set.rbegin(); }
   DO_INLINE_F const_reverse_iterator rbegin() const { return int_set.rbegin(); }
   DO_INLINE_F reverse_iterator rend() { return int_set.rend(); }
   DO_INLINE_F const_reverse_iterator rend() const { return int_set.rend(); }

   DO_INLINE_F BPatch_Set() {}

   DO_INLINE_F BPatch_Set(int_t s) : int_set(s) {}

   /** copy constructor.
    * @param newBPatch_Set the BPatch_Set which will be copied
    */
   DO_INLINE_F BPatch_Set(const BPatch_Set<Key,Compare>& rhs){
      int_set = rhs.int_set;
   } 

   /** returns the cardinality of the tree , number of elements */
   DO_INLINE_F unsigned int size() const { return int_set.size(); }
   
   /** returns true if tree is empty */
   DO_INLINE_F bool empty() const { return int_set.empty(); }
   
   /** inserts the element in the tree 
    * @param 1 element that will be inserted
    */
   DO_INLINE_F void insert(const Key &k) { int_set.insert(k); }
   
   /** removes the element in the tree 
    * @param 1 element that will be removed  
    */
   DO_INLINE_F void remove(const Key &k) { int_set.erase(k); }
   DO_INLINE_F void erase(const Key &k) { int_set.erase(k); }
   
   /** returns true if the argument is member of the BPatch_Set
    * @param e the element that will be searched for
    */
   DO_INLINE_F bool contains(const Key &key) const { return int_set.find(key) != int_set.end(); }
   
   /** fill an buffer array with the sorted
    * elements of the BPatch_Set in ascending order according to comparison function
    * if the BPatch_Set is empty it retuns NULL, other wise it returns 
    * the input argument.
    */
   DO_INLINE_F Key* elements(Key *a) const { 
      std::copy(begin(), end(), a);
      return a;
   }

   /** Like the above, but put things in a vector.
    */
   DO_INLINE_F void elements(BPatch_Vector<Key> &v) {
      std::copy(begin(), end(), std::back_inserter(v));
   }
   
   /** returns the minimum valued member in the BPatch_Set according to the 
    * comparison function supplied. If the BPatch_Set is empty it retuns 
    * any number. Not safe to use for empty sets 
    */
   DO_INLINE_F Key minimum() const {
      if (empty()) return Key();
      return *begin();
   }
   
   /** returns the maximum valued member in the BPatch_Set according to the 
    * comparison function supplied. If the BPatch_Set is empty it retuns 
    * any number. Not safe to use for empty sets 
    */
   DO_INLINE_F Key maximum() const {
      if (empty()) return Key();
      return *(--end());
   }
   
   /** assignment operator for BPatch_Set. It replicate sthe tree 
    * structure into the new BPatch_Set.
    * @param 1 BPatch_Set that will be used in assignment
    */
   DO_INLINE_F BPatch_Set<Key,Compare>& operator= (const BPatch_Set<Key,Compare>&rhs) {
      int_set = rhs.int_set;
      return *this;
   }
   
   /** equality comparison for the BPatch_Set
    * @param 1 BPatch_Set that will be used equality check
    */
   DO_INLINE_F bool operator== (const BPatch_Set<Key,Compare>&rhs) const { 
      return int_set == rhs.int_set;
   }
   
   /** inequality comparison for the BPatch_Set
    * @param 1 BPatch_Set that will be used inequality check
    */
   DO_INLINE_F bool operator!= (const BPatch_Set<Key,Compare>&rhs) const {
      return int_set != rhs.int_set;
   }
   
   /** insertion in to the BPatch_Set 
    * @param 1 element that will be inserted 
    */
   DO_INLINE_F BPatch_Set<Key,Compare>& operator+= (const Key &k) {
      int_set.insert(k);
      return *this;
   }
   
   /** union operation with this BPatch_Set 
    * @param 1 BPatch_Set that will be used in union operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare>& operator|= (const BPatch_Set<Key,Compare> &rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_union(begin(), end(), rhs.begin(), rhs.end(),
                     std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }

   
   /** intersection operation with this BPatch_Set 
    * @param 1 BPatch_Set that will be used in intersection operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare>& operator&= (const BPatch_Set<Key,Compare>&rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_intersection(begin(), end(), rhs.begin(), rhs.end(), 
                            std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }
   
   /** difference operation with this BPatch_Set 
    * @param 1 BPatch_Set that will be used in difference operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare>& operator-= (const BPatch_Set<Key,Compare>&rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_difference(begin(), end(), rhs.begin(), rhs.end(), 
                          std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }
   
   /** union operation 
    * @param 1 BPatch_Set that will be used in union operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare> operator| (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_union(begin(), end(), rhs.begin(), rhs.end(),
                     std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
   /** intersection operation 
    * @param 1 BPatch_Set that will be used in intersection operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare> operator& (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_intersection(begin(), end(), rhs.begin(), rhs.end(),
                            std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
   /** difference operation 
    * @param 1 BPatch_Set that will be used in difference operation
    */
   DO_INLINE_F BPatch_Set<Key,Compare> operator- (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_difference(begin(), end(), rhs.begin(), rhs.end(),
                          std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
   /** removes the element in the root of the tree 
    * if the BPatch_Set is empty it return false
    * @param e refernce to the element that the value of removed
    * element will be copied.
    */
   DO_INLINE_F bool extract(Key&k) {
      if (empty()) return false;
      iterator iter = begin();
      std::advance(iter, size() / 2);
      k = *iter;
      int_set.erase(iter);
      return true;
   }
   
};


#endif /* _BPatch_Set_h_ */

