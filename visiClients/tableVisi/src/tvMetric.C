// tvMetric.C
// Ariel Tamches

/*
 * $Log: tvMetric.C,v $
 * Revision 1.3  1995/12/22 22:39:12  tamches
 * added visiLibId
 *
 * Revision 1.2  1995/12/19 00:53:34  tamches
 * new args to tvMetric; it calls changeNumSigFigs
 * changeNumSigFigs written to update valuesPixWidth & numSigFigs
 * getColPixWidth updated correspondingly
 *
 * Revision 1.1  1995/11/04 00:45:59  tamches
 * First version of new table visi
 *
 */

#include "minmax.h"
#include "tvMetric.h"

tvMetric::tvMetric(unsigned iVisiLibId,
		   const string &iName, const string &iUnitsName,
		   XFontStruct *nameFontStruct, XFontStruct *unitsNameFontStruct,
		   XFontStruct *valuesFontStruct,
		   unsigned iNumSigFigs) :
		      name(iName), unitsName(iUnitsName) {
   visiLibId = iVisiLibId;
   namePixWidth = XTextWidth(nameFontStruct, name.string_of(), name.length());
   unitsPixWidth = XTextWidth(unitsNameFontStruct, iUnitsName.string_of(),
			      iUnitsName.length());
   changeNumSigFigs(iNumSigFigs, valuesFontStruct); // updates valuesPixWidth, too
}

void tvMetric::changeNumSigFigs(unsigned newNumSigFigs, XFontStruct *valuesFontStruct) {
   numSigFigs = newNumSigFigs;
   
   // need to recalculate valuesPixWidth

   // for values, include a decimal point and numSigFigs characters.
   valuesPixWidth = XTextWidth(valuesFontStruct, ".", 1);
   valuesPixWidth += numSigFigs * XTextWidth(valuesFontStruct, "0", 1); // a typical digit
   valuesPixWidth += XTextWidth(valuesFontStruct, "0", 1);
      // ... and a possible leading 0 not counted against the num of sig figs
}

unsigned tvMetric::getColPixWidth() const {
   unsigned result = max(getNamePixWidth(), getUnitsPixWidth());

   // now we must consider sig figs: the values themselves might be wider
   // than "result" so far, which includes the labelling.
   result = max(result, valuesPixWidth);

   // Now add padding:
   const unsigned horiz_padding_each_side = 3;
   result += 2*horiz_padding_each_side;

   return result;
}
