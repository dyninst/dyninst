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

// tvMetric.h
// Ariel Tamches

#ifndef _TV_METRIC_H_
#define _TV_METRIC_H_

#include "common/h/String.h"
#include "tk.h"

class tvMetric {
 private:
   unsigned visiLibId; // what unique-id has the visi-lib assigned to us?
   pdstring name;
   pdstring unitsName;
   unsigned namePixWidth;
   unsigned unitsPixWidth;
   unsigned numSigFigs;
   unsigned valuesPixWidth; // a direct function of numSigFigsBeingDisplayed

 public:
   tvMetric() {} // needed by class Vector (nuts)
   tvMetric(unsigned iVisiLibId,
	    const pdstring &iName, const pdstring &iUnitsName,
	    Tk_Font nameFont,
	    Tk_Font unitsNameFont,
	    Tk_Font valuesFont,
	    unsigned numSigFigs);
   tvMetric(const tvMetric &src) : name(src.name), unitsName(src.unitsName) {
      visiLibId = src.visiLibId;
      namePixWidth = src.namePixWidth;
      unitsPixWidth = src.unitsPixWidth;
      numSigFigs = src.numSigFigs;
      valuesPixWidth = src.valuesPixWidth;
   }
  ~tvMetric() {}

   unsigned getVisiLibId() const {return visiLibId;}

   bool operator<(const tvMetric &other) const {
      return name < other.name;
   }

   bool operator>(const tvMetric &other) const {
      return name > other.name;
   }

   const pdstring &getName() const {return name;}
   const pdstring &getUnitsName() const {return unitsName;}
   void changeUnitsName(const pdstring &newunitsname) {
      unitsName = newunitsname;
   }

   void changeNumSigFigs(unsigned newSigFigs, Tk_Font valuesFont);
   void setUnitsPixWidth(unsigned u) { unitsPixWidth = u; }
   unsigned getUnitsPixWidth() const { return unitsPixWidth; }
   unsigned getNamePixWidth() const {return namePixWidth;}
   unsigned getColPixWidth() const; // width of whole column (incl. horiz padding)
};

#endif
