// odometer.C

#include "util/h/odometer.h"

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
   assert(!done());
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
