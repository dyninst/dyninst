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

// $Id: refCounter.h,v 1.6 2004/03/23 01:11:53 eli Exp $
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
