// tvCell.h
// Ariel Tamches

/*
 * $Log: tvCell.h,v $
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
