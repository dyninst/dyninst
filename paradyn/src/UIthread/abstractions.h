// abstractions.h
// Ariel Tamches

// Holds a collection of "whereAxis".  Each "whereAxis"
// is 1 abstraction; hence, this class maintains a set of
// abstractions.

/* $Log: abstractions.h,v $
/* Revision 1.1  1995/07/17 04:58:52  tamches
/* First version of the new where axis
/*
 */

#ifndef _ABSTRACTIONS_H_
#define _ABSTRACTIONS_H_

#include <limits.h>

#include <tclclean.h>
#include <tkclean.h>

#include "String.h"

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#else
#include "util/h/Vector.h"
#endif

#include "whereAxis.h"
#include "whereAxisMisc.h" // myTclEval

template <class USERNODEDATA>
class abstractions {
 private:
   struct whereAxisStruct {
      whereAxis<USERNODEDATA> *theWhereAxis;
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
   abstractions(const string &iabsMenuName, const string &iNavMenuName,
		const string &iHorizSBName, const string &iVertSBName,
		const string &iFindName,
		Tcl_Interp *iInterp, Tk_Window iTkWindow) :
               absMenuName(iabsMenuName), navigateMenuName(iNavMenuName),
	       horizSBName(iHorizSBName), vertSBName(iVertSBName),
	       findName(iFindName) {
      currAbstractionIndex = ULONG_MAX;
      interp = iInterp;
      theTkWindow = iTkWindow;
   }
  ~abstractions() {
      for (int i=0; i < theAbstractions.size(); i++)
         delete theAbstractions[i].theWhereAxis;
 
      currAbstractionIndex = ULONG_MAX;
   }

   void add(whereAxis<USERNODEDATA> *theNewAbstraction,
	    const string &whereAxisName);

   whereAxis<USERNODEDATA> &operator[](string &absName) {
      // given an abstraction name, this routine returns the where axis
      // structure.  If no abstraction/where-axis exists with the given
      // name, however, we ADD A NEW WHERE-AXIS and return that one.
      // This routine pretty much ignores the concept of a current where axis.

      for (int i=0; i < theAbstractions.size(); i++) {
         if (absName == theAbstractions[i].abstractionName) {
            assert(theAbstractions[i].theWhereAxis);
            return *(theAbstractions[i].theWhereAxis);
         }
      }

      // cout << "abstractions[]: adding a new where axis..." << endl;
      whereAxis<USERNODEDATA> *theNewWhereAxis = new whereAxis<USERNODEDATA>
                    (interp, theTkWindow, "root",
                     horizSBName, vertSBName, navigateMenuName);
      assert(theNewWhereAxis);

      add(theNewWhereAxis, absName);
      return *theNewWhereAxis;
   }

   bool change(unsigned newindex);

   bool existsCurrent() {
      return currAbstractionIndex < theAbstractions.size();
   }

   whereAxis<USERNODEDATA> &getCurrent() {
      assert(existsCurrent);
      whereAxis<USERNODEDATA> *result = theAbstractions[currAbstractionIndex].theWhereAxis;
      assert(result);
      
      return *result;
   }

   const whereAxis<USERNODEDATA> &getCurrent() const {
      assert(existsCurrent);
      whereAxis<USERNODEDATA> *result = theAbstractions[currAbstractionIndex].theWhereAxis;
      assert(result);
      
      return *result;
   }

   vector< vector<USERNODEDATA> > getCurrAbstractionSelections() const {
      // returns a vector[num-hierarchies] of vector of selections.
      // The number of hierarchies is defined as the number of children of the
      // root node.
      return getCurrent().getSelections();
   }

   void resizeEverything() {
      if (!existsCurrent()) return;

      for (int i=0; i < theAbstractions.size(); i++) {
         theAbstractions[i].theWhereAxis->recursiveDoneAddingChildren();
         theAbstractions[i].theWhereAxis->resize(i==currAbstractionIndex);
      }
   }
};

#endif
