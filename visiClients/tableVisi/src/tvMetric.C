/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// tvMetric.C
// Ariel Tamches

/*
 * $Log: tvMetric.C,v $
 * Revision 1.8  2004/03/23 01:12:49  eli
 * Updated copyright string
 *
 * Revision 1.7  2003/07/15 22:48:07  schendel
 * rename string to pdstring
 *
 * Revision 1.6  2002/05/13 19:54:09  mjbrim
 * update string class to eliminate implicit number conversions
 * and replace all use of string_of with c_str  - - - - - - - - - - - - - -
 * change implicit number conversions to explicit conversions,
 * change all use of string_of to c_str
 *
 * Revision 1.5  1999/03/13 15:24:07  pcroth
 * Added support for building under Windows NT
 *
 * Revision 1.4  1996/08/16 21:37:07  tamches
 * updated copyright for release 1.1
 *
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
		   const pdstring &iName, const pdstring &iUnitsName,
		   Tk_Font nameFont, Tk_Font unitsNameFont,
		   Tk_Font valuesFont,
		   unsigned iNumSigFigs) :
		      name(iName), unitsName(iUnitsName) {
   visiLibId = iVisiLibId;
   namePixWidth = Tk_TextWidth(nameFont, name.c_str(), name.length());
   unitsPixWidth = Tk_TextWidth(unitsNameFont, iUnitsName.c_str(),
			      iUnitsName.length());
   changeNumSigFigs(iNumSigFigs, valuesFont); // updates valuesPixWidth, too
}

void tvMetric::changeNumSigFigs(unsigned newNumSigFigs, Tk_Font valuesFont) {
   numSigFigs = newNumSigFigs;
   
   // need to recalculate valuesPixWidth

   // for values, include a decimal point and numSigFigs characters.
   valuesPixWidth = Tk_TextWidth(valuesFont, ".", 1);
   valuesPixWidth += numSigFigs * Tk_TextWidth(valuesFont, "0", 1); // a typical digit
   valuesPixWidth += Tk_TextWidth(valuesFont, "0", 1);
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
