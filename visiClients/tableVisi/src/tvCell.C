// tvCell.C
// Ariel Tamches

/*
 * $Log: tvCell.C,v $
 * Revision 1.2  1995/12/29 08:17:59  tamches
 * a cleanup commit: moved body of some member functions from tvCell.h to tvCell.C
 *
 * Revision 1.1  1995/11/04 00:47:22  tamches
 * First version of new table visi
 *
 */

#include "tvCell.h"

bool tvCell::operator<(const tvCell &src) const {
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

bool tvCell::operator==(const tvCell &src) const {
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

bool tvCell::operator>(const tvCell &src) const {
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

tvCell &tvCell::operator=(const tvCell &src) {
   if (validData = src.validData) // yes, single = on purpose
      data = src.data;
   return *this;
}
