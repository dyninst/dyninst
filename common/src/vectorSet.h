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

// vectorSet.h
// A container class for a set of objects providing just a few operations
// (but doing so _very_ efficiently, in space and time):
// 1) add an item
// 2) remove an arbitrary item from the set and return its contents
// 3) peek at individual items in the set by their index (0 thru size-1)
// 4) remove an item by its index

#ifndef _VECTOR_SET_H_
#define _VECTOR_SET_H_

#ifdef external_templates
#pragma interface
#endif

#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("../src/vectorSet.C")
#endif


#include "common/src/Vector.h"

template <class T>
class DLLEXPORT vectorSet {
 private:
   pdvector<T> data;

 public:
   vectorSet() {} // empty set
   vectorSet(const vectorSet &src) : data(src.data) {}
  ~vectorSet() {}

   vectorSet &operator=(const vectorSet &src) {
      data = src.data;
      return *this;
   }

   bool empty() const {return data.size() == 0;}
   unsigned size() const {return data.size();}

   const T &operator[](unsigned index) const {
      return data[index];
   }

   T &operator[](unsigned index) {
      return data[index];
   }

   T removeByIndex(unsigned index);
   T removeOne() {
      return removeByIndex(0);
   }

   vectorSet &operator+=(const T &item) {
      data.push_back(item);
      return *this;
   }
};


#endif

