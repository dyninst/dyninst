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

   DO_INLINE_F BPatch_Set(const BPatch_Set<Key,Compare>& rhs){
      int_set = rhs.int_set;
   } 

   DO_INLINE_F unsigned int size() const { return int_set.size(); }
   
   DO_INLINE_F bool empty() const { return int_set.empty(); }
   
   DO_INLINE_F void insert(const Key &k) { int_set.insert(k); }
   
   DO_INLINE_F void remove(const Key &k) { int_set.erase(k); }
   DO_INLINE_F void erase(const Key &k) { int_set.erase(k); }
   
   DO_INLINE_F bool contains(const Key &key) const { return int_set.find(key) != int_set.end(); }
   
   DO_INLINE_F Key* elements(Key *a) const { 
      std::copy(begin(), end(), a);
      return a;
   }

   DO_INLINE_F void elements(BPatch_Vector<Key> &v) {
      std::copy(begin(), end(), std::back_inserter(v));
   }
   
   DO_INLINE_F Key minimum() const {
      if (empty()) return Key();
      return *begin();
   }
   
   DO_INLINE_F Key maximum() const {
      if (empty()) return Key();
      return *(--end());
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare>& operator= (const BPatch_Set<Key,Compare>&rhs) {
      int_set = rhs.int_set;
      return *this;
   }
   
   DO_INLINE_F bool operator== (const BPatch_Set<Key,Compare>&rhs) const { 
      return int_set == rhs.int_set;
   }
   
   DO_INLINE_F bool operator!= (const BPatch_Set<Key,Compare>&rhs) const {
      return int_set != rhs.int_set;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare>& operator+= (const Key &k) {
      int_set.insert(k);
      return *this;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare>& operator|= (const BPatch_Set<Key,Compare> &rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_union(begin(), end(), rhs.begin(), rhs.end(),
                     std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }

   
   DO_INLINE_F BPatch_Set<Key,Compare>& operator&= (const BPatch_Set<Key,Compare>&rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_intersection(begin(), end(), rhs.begin(), rhs.end(), 
                            std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare>& operator-= (const BPatch_Set<Key,Compare>&rhs) {
      Compare comp;
      std::set<Key, Compare> tmp;
      std::set_difference(begin(), end(), rhs.begin(), rhs.end(), 
                          std::inserter(tmp, tmp.begin()), comp);
      int_set.swap(tmp);
      return *this;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare> operator| (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_union(begin(), end(), rhs.begin(), rhs.end(),
                     std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare> operator& (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_intersection(begin(), end(), rhs.begin(), rhs.end(),
                            std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
   DO_INLINE_F BPatch_Set<Key,Compare> operator- (const BPatch_Set<Key,Compare>&rhs) const {
      Compare comp;
      BPatch_Set<Key, Compare> tmp;
      std::set_difference(begin(), end(), rhs.begin(), rhs.end(),
                          std::inserter(tmp.int_set, tmp.int_set.begin()), comp);
      return tmp;
   }
   
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

