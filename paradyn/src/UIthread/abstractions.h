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

// abstractions.h
// Ariel Tamches

// Holds a collection of "whereAxis".  Each "whereAxis"
// is 1 abstraction; hence, this class maintains a set of
// abstractions.

/* $Id: abstractions.h,v 1.20 2003/07/15 22:46:04 schendel Exp $ */

#ifndef _ABSTRACTIONS_H_
#define _ABSTRACTIONS_H_

#include <limits.h>

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#include "String.h"
#else
#include "common/h/Vector.h"
#include "common/h/String.h"
#endif

#include "whereAxis.h"
#include "tkTools.h" // myTclEval


class abstractions {
 private:
   struct whereAxisStruct {
      whereAxis *theWhereAxis;
      pdstring abstractionName; // what's used in the menu

      // These values save where axis state when changing abstractions.
      // On changing abstractions, we use these stashed-away values to
      // reset the scrollbars, the navigate menu, and the text in the find box.
      float horizSBfirst, horizSBlast;
      float vertSBfirst, vertSBlast;
      // pdvector<pdstring> navigateMenuItems; [not needed; each whereAxis has
      //                                    its own 'path'; just rethinkNavigateMenu()]
      pdstring findString;
   };

   pdvector<whereAxisStruct> theAbstractions;
   unsigned currAbstractionIndex; // an index into the above array

   Tcl_Interp *interp;
   Tk_Window   theTkWindow;
   pdstring absMenuName;
   pdstring navigateMenuName;
   pdstring horizSBName, vertSBName;
   pdstring findName;

   bool inBatchMode;

   whereAxis &getCurrent() {
      assert(existsCurrent());
      whereAxis *result = theAbstractions[currAbstractionIndex].theWhereAxis;
      assert(result);
      
      return *result;
   }

   const whereAxis &getCurrent() const {
      assert(existsCurrent());
      whereAxis *result = theAbstractions[currAbstractionIndex].theWhereAxis;
      assert(result);
      
      return *result;
   }

 public:
   Tk_Window getTkWindow() const {return theTkWindow;}
   const pdstring &getHorizSBName() const {return horizSBName;}
   const pdstring &getVertSBName() const {return vertSBName;}
   const pdstring &getAbsMenuName() const {return absMenuName;}

   abstractions(const pdstring &iabsMenuName, const pdstring &iNavMenuName,
		const pdstring &iHorizSBName, const pdstring &iVertSBName,
		const pdstring &iFindName,
		Tcl_Interp *iInterp, Tk_Window iTkWindow) :
               absMenuName(iabsMenuName), navigateMenuName(iNavMenuName),
	       horizSBName(iHorizSBName), vertSBName(iVertSBName),
	       findName(iFindName) {
      currAbstractionIndex = UINT_MAX;
      interp = iInterp;
      theTkWindow = iTkWindow;

      inBatchMode = false;
   }
  ~abstractions() {
      for (unsigned i=0; i < theAbstractions.size(); i++)
         delete theAbstractions[i].theWhereAxis;
 
      currAbstractionIndex = UINT_MAX;
   }

   void add(whereAxis *theNewAbstraction,
	    const pdstring &whereAxisName);

   whereAxis &operator[](const pdstring &absName);

   int name2index(const pdstring &name) const;
      // returns -1 if found found

   bool change(unsigned newindex);
   bool change(const pdstring &newName);

   bool existsCurrent() const {
      return currAbstractionIndex < theAbstractions.size();
   }

   void drawCurrent(bool doubleBuffer, bool xsynchOn);
      // checks batch mode; if true, rethinks before the redraw

   pdvector< pdvector<resourceHandle> > getCurrAbstractionSelections(bool &wholeProgram, pdvector<unsigned> &wholeProgramFocus) const {
      // returns a pdvector[num-hierarchies] of vector of selections.
      // The number of hierarchies is defined as the number of children of the
      // root node.
      return getCurrent().getSelections(wholeProgram, wholeProgramFocus);
   }

   void resizeEverything(bool resort) {
      if (!existsCurrent())
         return;

      for (unsigned i=0; i < theAbstractions.size(); i++) {
         theAbstractions[i].theWhereAxis->recursiveDoneAddingChildren(resort);
         theAbstractions[i].theWhereAxis->resize(i==currAbstractionIndex);
      }
   }

   void resizeCurrent() {
      if (!existsCurrent()) return;
      getCurrent().resize(true); // true --> resize sb's since we're curr displayed abs
   }

   void makeVisibilityUnobscured() {
      for (unsigned i=0; i < theAbstractions.size(); i++)
         theAbstractions[i].theWhereAxis->makeVisibilityUnobscured();
   }
   void makeVisibilityPartiallyObscured() {
      for (unsigned i=0; i < theAbstractions.size(); i++)
         theAbstractions[i].theWhereAxis->makeVisibilityPartiallyObscured();
   }
   void makeVisibilityFullyObscured() {
      for (unsigned i=0; i < theAbstractions.size(); i++)
         theAbstractions[i].theWhereAxis->makeVisibilityFullyObscured();
   }

   void startBatchMode();
   void endBatchMode();

   bool selectUnSelectFromFullPathName(const pdstring &name, bool select);
      // returns true iff the item was found
      // pass true for the 2nd param iff you want to select it; false
      // if you want to unselect it.

   void processSingleClick(int x, int y) {
      getCurrent().processSingleClick(x,y);
   }
 
   bool processCtrlClick(int x, int y,numlist &select_focus) {
      return getCurrent().processCtrlClick(x,y,select_focus);
   }
 
   bool processDoubleClick(int x, int y) {
      return getCurrent().processDoubleClick(x,y);
   }

   bool processShiftDoubleClick(int x, int y) {
      return getCurrent().processShiftDoubleClick(x,y);
   }

   bool processCtrlDoubleClick(int x, int y) {
      return getCurrent().processCtrlDoubleClick(x,y);
   }

   int getVertSBOffset() const { return getCurrent().getVertSBOffset(); }
   int getHorizSBOffset() const { return getCurrent().getHorizSBOffset(); }
   int getTotalVertPixUsed() const { return getCurrent().getTotalVertPixUsed(); }
   int getTotalHorizPixUsed() const { return getCurrent().getTotalHorizPixUsed(); }
   int getVisibleVertPix() const { return getCurrent().getVisibleVertPix(); }
   int getVisibleHorizPix() const { return getCurrent().getVisibleHorizPix(); }

   bool adjustVertSBOffset(float newfirstfrac) { return getCurrent().adjustVertSBOffset(newfirstfrac); }
   bool adjustVertSBOffsetFromDeltaPix(int deltay) { return getCurrent().adjustVertSBOffsetFromDeltaPix(deltay); }
   bool adjustHorizSBOffset(float newfirstfrac) { return getCurrent().adjustHorizSBOffset(newfirstfrac); }
   bool adjustHorizSBOffsetFromDeltaPix(int deltax) { return getCurrent().adjustHorizSBOffsetFromDeltaPix(deltax); }

   void clearSelections() { getCurrent().clearSelections(); }
   void navigateTo(unsigned pathLen) { getCurrent().navigateTo(pathLen); }

   int find(const pdstring &str) { return getCurrent().find(str); }

   void map_from_callgraph(resourceHandle select_handle,bool ishighlight) {
      getCurrent().map_from_callgraph(select_handle,ishighlight);
   }

};

#endif
