// tvCell.h
// Ariel Tamches

/*
 * $Log: tvCell.h,v $
 * Revision 1.3  1995/12/29 08:17:58  tamches
 * a cleanup commit: moved body of some member functions from tvCell.h to tvCell.C
 *
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

   bool operator<(const tvCell &src) const;
   bool operator==(const tvCell &src) const;
   bool operator>(const tvCell &src) const;

   tvCell &operator=(const tvCell &src);

   bool isValid() const {return validData;}
   double getData() const {assert(validData); return data;}

   void invalidate() {validData = false;}
   void setValidData(double newData) {
      data = newData;
      validData = true;
   }
};

#endif
