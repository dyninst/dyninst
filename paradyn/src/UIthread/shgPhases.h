// shgPhases.h
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.h,v $
/* Revision 1.5  1996/02/02 18:50:26  tamches
/* better multiple phase support
/* currSearching, everSearched flags are new
/* shgStruct constructor is new
/* new cleaner pc->ui igen-corresponding routines: defineNewSearch,
/* activateSearch, pauseSearch, resumeSearch, addNode, addEdge,
/* configNode, addToStatusDisplay
/* removed add()
/*
 * Revision 1.4  1996/01/23 07:04:44  tamches
 * clarified interface to change()
 *
 * Revision 1.3  1996/01/09 01:40:14  tamches
 * added existsById
 *
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
      shg *theShg; // note: destructor must not fry (since constructor
                   //       does not copy)
      // int  phaseID; (now present in shg.h) (is this right?)
      string phaseName; // what's used in the menu
      
      // Save shg UI-type state when changing phases, so we can restore
      // it later:
      float horizSBfirst, horizSBlast;
      float vertSBfirst, vertSBlast;

      // Save the activeness/disabledness of the "Search(Resume)" and "Pause" buttons:
      // Note that the "everSearched" button will distinguish "Search" from "Resume"
      bool currSearching;

      // needed to eliminate unused lame-duck phases when the time arrives...
      bool everSearched;

      // convenience routine:
      int getPhaseId() const {return theShg->getPhaseId();}

      shgStruct() {theShg=NULL;} // needed by Vector class
      shgStruct(unsigned phaseId, const string &iPhaseName,
		Tcl_Interp *interp, Tk_Window theTkWindow,
		const string &horizSBName, const string &vertSBName,
		const string &currItemLabelName) : phaseName(iPhaseName) {
         shg *theNewShg = new shg(phaseId, interp, theTkWindow,
				  horizSBName, vertSBName,
				  currItemLabelName);
         assert(theNewShg);

         this->theShg = theNewShg;
         this->horizSBfirst = this->horizSBlast = 0.0;
         this->vertSBfirst = this->vertSBlast = 0.0;
         this->currSearching = false;
         this->everSearched = false;
      }
      shgStruct(const shgStruct &src) : phaseName(src.phaseName) {
         theShg = src.theShg;
         horizSBfirst = src.horizSBfirst;
         horizSBlast = src.horizSBlast;
         vertSBfirst = src.vertSBfirst;
         vertSBlast = src.vertSBlast;
	 currSearching = src.currSearching;
	 everSearched = src.everSearched;
      }
      void fryDag() {delete theShg;}
     ~shgStruct() {}

      shgStruct &operator=(const shgStruct &src) {
         theShg = src.theShg; // is this right?
         phaseName = src.phaseName;
         horizSBfirst = src.horizSBfirst;
         horizSBlast = src.horizSBlast;
         vertSBfirst = src.vertSBfirst;
         vertSBlast = src.vertSBlast;
         currSearching = src.currSearching;
         everSearched = src.everSearched;

         return *this;
      }
   };

   vector<shgStruct> theShgPhases;
   unsigned currShgPhaseIndex;
      // NOTE: An index into the above array; not a phaseid/dagid!

   Tcl_Interp *interp;
   Tk_Window theTkWindow;   

   string menuName, horizSBName, vertSBName, currItemLabelName;

   shg &getByID(int phaseID);
   shgStruct &getByIDLL(int phaseID);
   bool changeLL(unsigned newIndex);
      // returns true iff any changes

 public:

   shgPhases(const string &iMenuName,
             const string &iHorizSBName, const string &iVertSBName,
             const string &iCurrItemLabelName,
             Tcl_Interp *iInterp, Tk_Window iTkWindow);
  ~shgPhases();

   Tk_Window getTkWindow() {return theTkWindow;} // needed for XWarpPointer
//   const string &getMenuName() const {return menuName;}
   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName() const {return vertSBName;}
//   const string &getCurrItemLabelName() const {return currItemLabelName;}

   int name2id (const string &phaseName) const;
      // returns -1 if not found
   const string &id2name (int id) const;

   bool changeByPhaseId(int newIndex);
      // returns true iff any changes
   bool change(const string &phaseName);
      // returns true iff any changes

   bool existsCurrent() const {return currShgPhaseIndex < theShgPhases.size();}
   bool existsById(int phaseId) const;
   shg &getCurrent();
   const shg &getCurrent() const;
   int getCurrentId() const {
      assert(existsCurrent());
      return theShgPhases[currShgPhaseIndex].getPhaseId();
   }

   void resizeEverything();

   void defineNewSearch(int phaseId, const string &phaseName);
   bool activateSearch(int phaseId);
      // returns true iff search successfully started
   bool pauseSearch(int phaseId);
      // returns true iff search successfully paused
   bool resumeSearch(int phaseId);
      // returns true iff search successfully resumed

   bool addNode(int phaseId, unsigned nodeId,
                bool active,
                shgRootNode::evaluationState,
                const string &label, const string &fullInfo,
                bool rootNodeFlag);
   bool addEdge(int phaseId, unsigned fromId, unsigned toId,
                shgRootNode::evaluationState,
                const char *label // used only for shadow nodes, else NULL
                );
      // The evaluationState param decides whether to explicitly expand
      // the "to" node.  Rethinks the entire layout of the shg
   bool configNode(int phaseId, unsigned nodeId,
                   bool active, shgRootNode::evaluationState);

   void addToStatusDisplay(int phaseId, const char *msg);
};

#endif
