// abstractions.h
// Ariel Tamches

// Holds a collection of "whereAxis".  Each "whereAxis"
// is 1 abstraction; hence, this class maintains a set of
// abstractions.

/* $Log: abstractions.h,v $
/* Revision 1.9  1996/04/01 22:29:51  tamches
/* makeVisibility* functions added
/*
 * Revision 1.8  1996/01/23 06:59:04  tamches
 * corrected path to include of String.h
 *
 * Revision 1.7  1996/01/11 04:39:34  tamches
 * added whole program kludge for getCurrAbstractionSelections
 *
 * Revision 1.6  1995/10/17 20:51:06  tamches
 * class abstractions is no longer templated.
 * class whereAxis is no longer templated either.
 *
 * Revision 1.5  1995/09/20 01:15:47  tamches
 * minor change; some usages of int --> unsigned
 *
 * Revision 1.4  1995/08/07  00:00:51  tamches
 * Added name2index(), getAbsMenuName()
 *
 * Revision 1.3  1995/07/24  21:32:47  tamches
 * Added getTkWindow(), get*SBName(), and change(string) member
 * functions.
 *
 * Revision 1.2  1995/07/18  03:41:16  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:58:52  tamches
 * First version of the new where axis
 *
 */

#ifndef _ABSTRACTIONS_H_
#define _ABSTRACTIONS_H_

#include <limits.h>

#include "tcl.h"
#include "tk.h"

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#include "String.h"
#else
#include "util/h/Vector.h"
#include "util/h/String.h"
#endif

#include "whereAxis.h"
#include "tkTools.h" // myTclEval

class abstractions {
 private:
   struct whereAxisStruct {
      whereAxis *theWhereAxis;
      string abstractionName; // what's used in the menu

      // These values save where axis state when changing abstractions.
      // On changing abstractions, we use these stashed-away values to
      // reset the scrollbars, the navigate menu, and the text in the find box.
      float horizSBfirst, horizSBlast;
      float vertSBfirst, vertSBlast;
      // vector<string> navigateMenuItems; [not needed; each whereAxis has
      //                                    its own 'path'; just rethinkNavigateMenu()]
      string findString;
   };

   vector<whereAxisStruct> theAbstractions;
   unsigned currAbstractionIndex; // an index into the above array

   Tcl_Interp *interp;
   Tk_Window   theTkWindow;
   string absMenuName;
   string navigateMenuName;
   string horizSBName, vertSBName;
   string findName;

 public:
   Tk_Window getTkWindow() const {return theTkWindow;}
   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName() const {return vertSBName;}
   const string &getAbsMenuName() const {return absMenuName;}

   abstractions(const string &iabsMenuName, const string &iNavMenuName,
		const string &iHorizSBName, const string &iVertSBName,
		const string &iFindName,
		Tcl_Interp *iInterp, Tk_Window iTkWindow) :
               absMenuName(iabsMenuName), navigateMenuName(iNavMenuName),
	       horizSBName(iHorizSBName), vertSBName(iVertSBName),
	       findName(iFindName) {
      currAbstractionIndex = UINT_MAX;
      interp = iInterp;
      theTkWindow = iTkWindow;
   }
  ~abstractions() {
      for (unsigned i=0; i < theAbstractions.size(); i++)
         delete theAbstractions[i].theWhereAxis;
 
      currAbstractionIndex = UINT_MAX;
   }

   void add(whereAxis *theNewAbstraction,
	    const string &whereAxisName);

   whereAxis &operator[](const string &absName);

   int name2index(const string &name) const;
      // returns -1 if found found

   bool change(unsigned newindex);
   bool change(const string &newName);

   bool existsCurrent() const {
      return currAbstractionIndex < theAbstractions.size();
   }

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

   vector< vector<resourceHandle> > getCurrAbstractionSelections(bool &wholeProgram, vector<unsigned> &wholeProgramFocus) const {
      // returns a vector[num-hierarchies] of vector of selections.
      // The number of hierarchies is defined as the number of children of the
      // root node.
      return getCurrent().getSelections(wholeProgram, wholeProgramFocus);
   }

   void resizeEverything() {
      if (!existsCurrent())
         return;

      for (unsigned i=0; i < theAbstractions.size(); i++) {
         theAbstractions[i].theWhereAxis->recursiveDoneAddingChildren(false);
            // false --> don't resort
         theAbstractions[i].theWhereAxis->resize(i==currAbstractionIndex);
      }
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
};

#endif
