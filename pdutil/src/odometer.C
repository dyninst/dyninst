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

// odometer.C

#include "pdutil/h/odometer.h"

odometer::odometer(vector<unsigned> &iDigitRanges) :
        digitRanges(iDigitRanges), currValue(digitRanges.size()) {
   for (unsigned digitlcv=0; digitlcv < numDigits(); digitlcv++)
      currValue[digitlcv] = 0;
}

bool odometer::done() const {
   // we are done if we have overflowed
   // So, all we need to do is check the most-significant-digit for
   // an overflow
   unsigned msd_index = numDigits()-1;
   return currValue[msd_index] >= digitRanges[msd_index];
}

void odometer::add1() {
   bool aflag;
   aflag=(!done());
   assert(aflag);
   currValue[0]++;

   // now move from the least significant to most significant digit,
   // checking for overflows and carrying as necessary.
   // The only exception is the most significant digit, which we keep
   // in an overflowed state if applicable.
   for (unsigned digitlcv=0; digitlcv < numDigits()-1; digitlcv++) {
      if (currValue[digitlcv] >= digitRanges[digitlcv]) {
         // we have overflowed this digit...carry some over to the next
         unsigned digitVal = currValue[digitlcv];
         currValue[digitlcv] = digitVal % digitRanges[digitlcv];
         currValue[digitlcv+1] += digitVal / digitRanges[digitlcv];
      }
   }
}
