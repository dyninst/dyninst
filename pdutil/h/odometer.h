// odometer.h

#ifndef _ODOMETER_H_
#define _ODOMETER_H_

#include "util/h/Vector.h"

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
