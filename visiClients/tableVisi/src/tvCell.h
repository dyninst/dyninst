// tvCell.h
// Ariel Tamches

/*
 * $Log: tvCell.h,v $
 * Revision 1.2  1995/12/22 22:38:30  tamches
 * operator<, operator>, operator== are new; chose a rule that an invalid
 * cell is less than any valid cell, and all invalid cells are equal.
 *
 * Revision 1.1  1995/11/04 00:47:21  tamches
 * First version of new table visi
 *
 */

#ifndef _TV_CELL_H_
#define _TV_CELL_H_

#include <assert.h>

class tvCell {
 private:
   double data;
   bool validData;

 public:
   tvCell() {invalidate();}
   tvCell(double iData) {setValidData(iData);}
   tvCell(const tvCell &src) {
      if (validData = src.validData) // yes, single = on purpose
         data = src.data;
   }
  ~tvCell() {}

   bool operator<(const tvCell &src) {
      // let's work out a sorting order for valid v. invalid data right now:
      // let's say that valid data is always "greater than" any invalid data,
      // and that all invalid data are equal.
      if (validData)
         if (src.validData)
            // yea, a sane comparison is possible
            return data < src.data;
         else
            // we are sane but "src" is nuts.  valid data is always > invalid data, so:
            return false;
      else
         if (src.validData)
            // we are nuts; "src" is sane.  valid data > invalid data, so:
            return true;
         else
            // everyone's nuts; all invalid data are equal, so:
            return false;
   }
   bool operator==(const tvCell &src) {
      if (validData)
         if (src.validData)
            return data == src.data;
         else
            // valid data > invalid data, so:
            return false;
      else
         if (src.validData)
            // valid data > invalid data, so:
            return false;
         else
            // invalid data are equal, so:
            return true;
   }
   bool operator>(const tvCell &src) {
      if (validData)
         if (src.validData)
            return data > src.data;
         else
            return true; // valid data > invalid data
      else
         if (src.validData)
            return false; // valid data > invalid data
         else
            return false; // invalid == invalid
   }

   tvCell &operator=(const tvCell &src) {
      if (validData = src.validData) // yes, single = on purpose
         data = src.data;
      return *this;
   }

   bool isValid() const {return validData;}
   double getData() const {assert(validData); return data;}

   void invalidate() {validData = false;}
   void setValidData(double newData) {
      data = newData;
      validData = true;
   }
};

#endif
