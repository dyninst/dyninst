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

#ifdef __XLC__
#pragma implementation("../src/vectorSet.C")
#endif


#include "common/h/Vector.h"

template <class T>
class vectorSet {
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

