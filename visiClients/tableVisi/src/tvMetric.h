/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// tvMetric.h
// Ariel Tamches

/*
 * $Log: tvMetric.h,v $
 * Revision 1.6  1996/08/16 21:37:08  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/08/05 07:12:33  tamches
 * tkclean.h --> tk.h
 *
 * Revision 1.4  1995/12/22 22:38:56  tamches
 * added visiLibId
 *
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
#include "tk.h"

class tvMetric {
 private:
   unsigned visiLibId; // what unique-id has the visi-lib assigned to us?
   string name;
   string unitsName;
   unsigned namePixWidth;
   unsigned unitsPixWidth;
   unsigned numSigFigs;
   unsigned valuesPixWidth; // a direct function of numSigFigsBeingDisplayed

 public:
   tvMetric() {} // needed by class Vector (nuts)
   tvMetric(unsigned iVisiLibId,
	    const string &iName, const string &iUnitsName,
	    XFontStruct *nameFontStruct,
	    XFontStruct *unitsNameFontStruct,
	    XFontStruct *valuesFontStruct,
	    unsigned numSigFigs);
   tvMetric(const tvMetric &src) : name(src.name), unitsName(src.unitsName) {
      visiLibId = src.visiLibId;
      namePixWidth = src.namePixWidth;
      unitsPixWidth = src.unitsPixWidth;
   }
  ~tvMetric() {}

   unsigned getVisiLibId() const {return visiLibId;}

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
