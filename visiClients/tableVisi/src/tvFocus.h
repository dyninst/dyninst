// tvFocus.h
// Ariel Tamches

/*
 * $Log: tvFocus.h,v $
 * Revision 1.2  1995/11/08 21:48:12  tamches
 * moved implementation of constructor to .C file
 *
 * Revision 1.1  1995/11/04 00:46:39  tamches
 * First version of new table visi
 *
 */

#ifndef _TV_FOCUS_H_
#define _TV_FOCUS_H_

#include "String.h"
#include "tkclean.h"

class tvFocus {
 private:
   string longName, shortName;
   unsigned longNamePixWidth, shortNamePixWidth;

 public:
   tvFocus(){} // needed by class Vector (nuts)
   tvFocus(const string &iLongName, XFontStruct *nameFontStruct);

   tvFocus(const tvFocus &src) : longName(src.longName), shortName(src.shortName) {
      longNamePixWidth = src.longNamePixWidth;
      shortNamePixWidth = src.shortNamePixWidth;
   }
  ~tvFocus() {}

   bool less_than(const tvFocus &other, bool useLongName) {
      if (useLongName)
         return (longName < other.longName);
      else
         return (shortName < other.shortName);
   }

   bool greater_than(const tvFocus &other, bool useLongName) {
      if (useLongName)
         return (longName > other.longName);
      else
         return (shortName > other.shortName);
   }

   const string &getLongName() const {return longName;}
   const string &getShortName() const {return shortName;}
   unsigned getLongNamePixWidth() const {return longNamePixWidth;}
   unsigned getShortNamePixWidth() const {return shortNamePixWidth;}
};

#endif
