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

// odometer.h

#ifndef _ODOMETER_H_
#define _ODOMETER_H_

#include "common/h/Vector.h"

class odometer {
 private:
   vector<unsigned> digitRanges;
      // the number of elements in digit index <n> is digitRanges[n];
      // the values that digit index <n> can take on are from 0
      // to (digitRanges[n]-1), inclusive.

   vector<unsigned> currValue;
   // note: for both of the above arrays, index 0 represents the _least_
   //       significant digit in the odometer; index <maxindex-1> represents
   //       the _most_ significant digit in the odometer.

 public:
   odometer(vector<unsigned> &iDigitRanges);
   odometer(const odometer &) {}
  ~odometer() {}

   odometer &operator=(const odometer &src) {
      digitRanges = src.digitRanges;
      currValue = src.currValue;
      return *this;
   }

   unsigned numDigits() const {return digitRanges.size();}
   bool done() const;

   unsigned getDigit(unsigned index) const {
      // note: 0 is the _least_ significant digit in the odometer;
      //       the _most_ significant digit is <numDigits()-1>
      assert(currValue[index] < digitRanges[index]);
      return currValue[index];
   }
   unsigned operator[](unsigned index) const {
      return getDigit(index);
   }

   void add1();
   odometer &operator++() {
      // prefix: add then return
      add1();
      return *this;
   }
   odometer operator++(int) {
      // postfix: add but return old value
      odometer result = *this;
      add1();
      return result;
   }
};

#endif
