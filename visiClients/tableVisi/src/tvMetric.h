// tvMetric.h
// Ariel Tamches

/*
 * $Log: tvMetric.h,v $
 * Revision 1.3  1995/12/19 00:52:49  tamches
 * added numSigFigs and valuesPixWidth member variables.
 * Constructor takes in 2 new params, accordingly.
 * changeUnitsName was cleaned up (it should not have been const)
 *
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
   unsigned numSigFigs;
   unsigned valuesPixWidth; // a direct function of numSigFigsBeingDisplayed

 public:
   tvMetric() {} // needed by class Vector (nuts)
   tvMetric(const string &iName, const string &iUnitsName,
	    XFontStruct *nameFontStruct,
	    XFontStruct *unitsNameFontStruct,
	    XFontStruct *valuesFontStruct,
	    unsigned numSigFigs);
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
   void changeUnitsName(const string &newunitsname) {
      unitsName = newunitsname;
   }

   void changeNumSigFigs(unsigned newSigFigs, XFontStruct *valuesFontStruct);
   
   unsigned getNamePixWidth() const {return namePixWidth;}
   unsigned getUnitsPixWidth() const {return unitsPixWidth;}
   unsigned getColPixWidth() const; // width of whole column (incl. horiz padding)
};

#endif
