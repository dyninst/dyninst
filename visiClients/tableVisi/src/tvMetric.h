// tvMetric.h
// Ariel Tamches

/*
 * $Log: tvMetric.h,v $
 * Revision 1.2  1995/12/03 21:09:32  newhall
 * changed units labeling to match type of data being displayed
 *
 * Revision 1.1  1995/11/04  00:45:58  tamches
 * First version of new table visi
 *
 */

#ifndef _TV_METRIC_H_
#define _TV_METRIC_H_

#include "String.h"
#include "tkclean.h"

class tvMetric {
 private:
   string name;
   string unitsName;
   unsigned namePixWidth;
   unsigned unitsPixWidth;

 public:
   tvMetric() {} // needed by class Vector (nuts)
   tvMetric(const string &iName, const string &iUnitsName,
	    XFontStruct *nameFontStruct,
	    XFontStruct *unitsNameFontStruct);
   tvMetric(const tvMetric &src) : name(src.name), unitsName(src.unitsName) {
      namePixWidth = src.namePixWidth;
      unitsPixWidth = src.unitsPixWidth;
   }
  ~tvMetric() {}

   bool operator<(const tvMetric &other) const {
      return name < other.name;
   }

   bool operator>(const tvMetric &other) const {
      return name > other.name;
   }

   const string &getName() const {return name;}
   const string &getUnitsName() const {return unitsName;}
   void changeUnitsName(const string &newname) const { 
		const char *temp = newname.string_of();
		((string) unitsName) = temp; 
   }
   unsigned getNamePixWidth() const {return namePixWidth;}
   unsigned getUnitsPixWidth() const {return unitsPixWidth;}
   unsigned getColPixWidth() const; // width of whole column (incl. horiz padding)
};

#endif
