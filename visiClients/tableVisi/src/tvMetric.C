// tvMetric.C
// Ariel Tamches

/*
 * $Log: tvMetric.C,v $
 * Revision 1.1  1995/11/04 00:45:59  tamches
 * First version of new table visi
 *
 */

#include "minmax.h"
#include "tvMetric.h"

tvMetric::tvMetric(const string &iName, const string &iUnitsName,
		   XFontStruct *nameFontStruct, XFontStruct *unitsNameFontStruct) :
		      name(iName), unitsName(iUnitsName) {
   namePixWidth = XTextWidth(nameFontStruct, name.string_of(), name.length());
   unitsPixWidth = XTextWidth(unitsNameFontStruct, iUnitsName.string_of(),
			      iUnitsName.length());
}

unsigned tvMetric::getColPixWidth() const {
   const int horiz_padding_each_side = 3;
   return max(getNamePixWidth(), getUnitsPixWidth()) + 2*horiz_padding_each_side;
}
