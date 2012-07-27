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


// $Id: refCounter.h,v 1.7 2007/05/30 19:20:03 legendre Exp $
// refCounter.h
// Ariel Tamches

#ifndef _REF_COUNTER_H_
#define _REF_COUNTER_H_

#ifdef external_templates
#pragma interface
#endif

#include <assert.h>

template <class T>
class refCounter {
 private:
   class actualData {
    private:
      mutable unsigned refCount;
      T data;
    public:
      actualData(const T &src) : data(src) {refCount=0;}
     ~actualData() {}
      void reference() const {refCount++;}
      bool dereference() const {
	 assert(refCount > 0);
	 return (--refCount == 0);
      }
      T &getData() {return data;}
      const T &getData() const {return data;}
   };
   actualData *theData;
      // allocated with new, but not necessarily by us.  _Never_ NULL.

 private:
   void reference() const {
      assert(theData);
      theData->reference();
   }
   void dereference() const {
      assert(theData);
      if (theData->dereference())
         delete theData;
   }

   // explicitly disallowed
   // (Visual C++ still requires a body, however)
   refCounter() {}

 public:
   refCounter(const T &src) {
      // examples:
      // T y; (y is initialized somehow...)
      // refCounter<T> x = y; or
      // refCounter<T> x(y);
      theData = new actualData(src);
      assert(theData);
      reference();
   }
   refCounter(const refCounter &src) {
      // This constructor is what this whole class revolves around.  It's fast.
      // examples:
      // refCounter<T> y; (y is initialized somehow...)
      // refCounter<T> x = y; or
      // refCounter<T> x(y);
      src.reference(); // just bumps up a ref count --> fast
      theData = src.theData;  // just a ptr assignment --> fast
   }
  ~refCounter() {
      dereference();
   }
   refCounter &operator=(const refCounter &src) {
      if (this == &src)
         return *this; // protect against x=x

      dereference();

      // ...and attach to the new stuff efficiently
      theData = src.theData; // just a ptr assignment --> fast
      reference();           // just bumps a ref cnt  --> fast
      return *this;
   }
   refCounter &operator=(const T &src) {
      dereference();
      theData = new actualData(src);
      reference();
      return *this;
   }
   T &getData() {
      assert(theData);
      return theData->getData();
   }
   const T &getData() const {
      assert(theData);
      return theData->getData();
   }
};

#endif
