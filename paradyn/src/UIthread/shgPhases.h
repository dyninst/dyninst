// shgPhases.h
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.h,v $
/* Revision 1.3  1996/01/09 01:40:14  tamches
/* added existsById
/*
 * Revision 1.2  1996/01/09 01:06:17  tamches
 * changes due to moving phaseId to the shg class
 *
 * Revision 1.1  1995/10/17 22:08:35  tamches
 * initial version, for the new search history graph
 *
 */

#ifndef _SHG_PHASES_H_
#define _SHG_PHASES_H_

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#else
#include "util/h/Vector.h"
#endif

#include "tcl.h"
#include "tk.h"

#include "shg.h"

class shgPhases {
 private:
   struct shgStruct {
      shg *theShg;
      // int  phaseID; (now present in shg.h)
      string phaseName; // what's used in the menu
      
      // Save shg UI-type state when changing phases, so we can restore
      // it later:
      float horizSBfirst, horizSBlast;
      float vertSBfirst, vertSBlast;

      // convenience routine:
      int getPhaseId() const {return theShg->getPhaseId();}
   };

   vector<shgStruct> theShgPhases;
   unsigned currShgPhaseIndex;

   Tcl_Interp *interp;
   Tk_Window theTkWindow;   

   string menuName, horizSBName, vertSBName, currItemLabelName;

 public:

   shgPhases(const string &iMenuName,
             const string &iHorizSBName, const string &iVertSBName,
             const string &iCurrItemLabelName,
             Tcl_Interp *iInterp, Tk_Window iTkWindow);
  ~shgPhases();

   Tk_Window getTkWindow() {return theTkWindow;} // needed for XWarpPointer
   const string &getMenuName() const {return menuName;}
   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName() const {return vertSBName;}
   const string &getCurrItemLabelName() const {return currItemLabelName;}

   void add(shg *theNewShg, const string &theNewShgPhaseName);
   
   shg &getByID(int phaseID);

   int name2id (const string &phaseName) const;
      // returns -1 if not found
   const string &id2name (int id) const;

   bool change(unsigned newIndex);
      // returns true iff any changes
   bool change(const string &phaseName);
      // returns true iff successful

   bool existsCurrent() const {return currShgPhaseIndex < theShgPhases.size();}
   bool existsById(int phaseId) const;
   shg &getCurrent();
   const shg &getCurrent() const;
   int getCurrentId() const {
      assert(existsCurrent());
      return theShgPhases[currShgPhaseIndex].getPhaseId();
   }

   void resizeEverything();
};

#endif
