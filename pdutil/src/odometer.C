// odometer.C

#include "util/h/odometer.h"

odometer::odometer(vector<unsigned> &iDigitRanges) :
        digitRanges(iDigitRanges), currValue(digitRanges.size()) {
   for (unsigned digitlcv=0; digitlcv < numDigits(); digitlcv++)
      currValue[digitlcv] = 0;
}

bool odometer::done() const {
   for (unsigned digitlcv=0; digitlcv < numDigits(); digitlcv++) {
      assert(currValue[digitlcv] < digitRanges[digitlcv]);
      if (currValue[digitlcv] != digitRanges[digitlcv]-1)
         return false;
   }

   return true;
}

void odometer::add1() {
   assert(!done());
   currValue[0]++;

   for (unsigned digitlcv=0; digitlcv < numDigits(); digitlcv++) {
      if (currValue[digitlcv] >= digitRanges[digitlcv]) {
         // we have overflowed this digit...carry some over to the next
         unsigned digitVal = currValue[digitlcv];
         currValue[digitlcv] = digitVal % digitRanges[digitlcv];
         currValue[digitlcv+1] += digitVal / digitRanges[digitlcv];
      }
   }
}
