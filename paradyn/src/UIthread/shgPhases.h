// shgPhases.h
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.h,v $
/* Revision 1.9  1996/02/15 23:12:29  tamches
/* corrected parameters of addEdge to properly handle why vs. where
/* axis refinements
/*
 * Revision 1.8  1996/02/11 18:25:54  tamches
 * shg message window now works correctly for multiple phases
 * internal cleanup; more tk window name entities parameterized
 *
 * Revision 1.7  1996/02/07 21:51:04  tamches
 * defineNewSearch now returns bool
 *
 * Revision 1.6  1996/02/07 19:11:26  tamches
 * former globals currInstalledAltMoveHandler, ignoreNextShgAltMove,
 * shgAltAnchorX/Y added
 * getCurrent() made private
 * added draw, resize, single/middle/doubleClick, scrollPosition, and
 * altPress/Release routines.
 * activateSearch --> activateCurrSearch(); similar for pause, resume
 *
 * Revision 1.5  1996/02/02 18:50:26  tamches
 * better multiple phase support
 * currSearching, everSearched flags are new
 * shgStruct constructor is new
 * new cleaner pc->ui igen-corresponding routines: defineNewSearch,
 * activateSearch, pauseSearch, resumeSearch, addNode, addEdge,
 * configNode, addToStatusDisplay
 * removed add()
 *
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
#include "tkTools.h"

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

      // Save the text in the shg message window, so we can restore it later:
      string msgText;

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
         this->horizSBfirst = this->vertSBfirst = 0.0;
         this->horizSBlast = this->vertSBlast = 1.0;
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
	 msgText = src.msgText;
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

   string menuName, horizSBName, vertSBName;
   string currItemLabelName; // what you see when clicking on middle mouse button
   string msgTextWindowName;
   string searchButtonName, pauseButtonName;
   string currPhaseLabelName;

   bool currInstalledAltMoveHandler;
   bool ignoreNextShgAltMove;
   int shgAltAnchorX;
   int shgAltAnchorY;

   shg &getByID(int phaseID);
   shgStruct &getByIDLL(int phaseID);
   bool changeLL(unsigned newIndex);
      // returns true iff any changes

   shg &getCurrent();
   const shg &getCurrent() const;

 public:

   shgPhases(const string &iMenuName,
             const string &iHorizSBName, const string &iVertSBName,
             const string &iCurrItemLabelName,
             const string &iMsgTextWindowName,
             const string &iSearchButtonName, const string &iPauseButtonName,
             const string &iCurrPhaseLabelName,
             Tcl_Interp *iInterp, Tk_Window iTkWindow);
  ~shgPhases();

   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName() const {return vertSBName;}

   int name2id (const string &phaseName) const;
      // returns -1 if not found
   const string &id2name (int id) const;

   bool changeByPhaseId(int newIndex);
      // returns true iff any changes
   bool change(const string &phaseName);
      // returns true iff any changes

   bool existsCurrent() const {return currShgPhaseIndex < theShgPhases.size();}
   bool existsById(int phaseId) const;
   int getCurrentId() const {
      assert(existsCurrent());
      return theShgPhases[currShgPhaseIndex].getPhaseId();
   }

   void resizeEverything();

   void draw(bool doubleBuffer, bool isXsynchOn) const;

   bool resize();

   void processSingleClick(int x, int y);
   void processMiddleClick(int x, int y);
   bool processDoubleClick(int x, int y);

   bool newVertScrollPosition(int argc, char **argv);
   bool newHorizScrollPosition(int argc, char **argv);

   bool altPress(int x, int y);
   void altRelease();

   bool defineNewSearch(int phaseId, const string &phaseName);
   bool activateCurrSearch();
      // returns true iff search successfully started
   bool pauseCurrSearch();
      // returns true iff search successfully paused
   bool resumeCurrSearch();
      // returns true iff search successfully resumed

   bool addNode(int phaseId, unsigned nodeId,
                bool active,
                shgRootNode::evaluationState,
                const string &label, const string &fullInfo,
                bool rootNodeFlag);
   bool addEdge(int phaseId, unsigned fromId, unsigned toId,
                shgRootNode::refinement,
                const char *label // used only for shadow nodes, else NULL
                );
      // The evaluationState param decides whether to explicitly expand
      // the "to" node.  Rethinks the entire layout of the shg
   bool configNode(int phaseId, unsigned nodeId,
                   bool active, shgRootNode::evaluationState);

   void addToStatusDisplay(int phaseId, const string &msg);
      // currently, we do not append the newline character for you
};

#endif
