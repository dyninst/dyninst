// shgPhases.C
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.C,v $
/* Revision 1.8  1996/02/07 21:51:35  tamches
/* defineNewSearch now returns bool should-redraw flag
/*
 * Revision 1.7  1996/02/07 19:12:23  tamches
 * added draw(), resize, single/middle/doubleClick, scroll-position,
 * and altPress/release routines.
 * activateSearch --> activateCurrSearch(); similar for pause/resume
 *
 * Revision 1.6  1996/02/02 18:50:27  tamches
 * better multiple phase support
 * currSearching, everSearched flags are new
 * shgStruct constructor is new
 * new cleaner pc->ui igen-corresponding routines: defineNewSearch,
 * activateSearch, pauseSearch, resumeSearch, addNode, addEdge,
 * configNode, addToStatusDisplay
 * removed add()
 *
 * Revision 1.5  1996/01/23 07:06:46  tamches
 * clarified interface to change()
 *
 * Revision 1.4  1996/01/09 01:40:26  tamches
 * added existsById
 *
 * Revision 1.3  1996/01/09 01:06:43  tamches
 * changes to reflect moving phase id to shg class
 *
 * Revision 1.2  1995/11/29 00:20:05  tamches
 * removed some warnings
 *
 * Revision 1.1  1995/10/17 22:08:36  tamches
 * initial version, for the new search history graph
 *
 */

#include <limits.h>
#include "tkTools.h" // myTclEval()
#include "shgPhases.h"

#ifdef PARADYN
#include "performanceConsultant.thread.CLNT.h"
extern performanceConsultantUser *perfConsult;
#endif

shgPhases::shgPhases(const string &iMenuName,
		     const string &iHorizSBName, const string &iVertSBName,
		     const string &iCurrItemLabelName,
		     Tcl_Interp *iInterp, Tk_Window iTkWindow) :
		        menuName(iMenuName), horizSBName(iHorizSBName),
			vertSBName(iVertSBName),
			currItemLabelName(iCurrItemLabelName)
{
   currShgPhaseIndex = UINT_MAX;

   interp = iInterp;
   theTkWindow = iTkWindow;

   currInstalledAltMoveHandler=false;
   ignoreNextShgAltMove=false;
}

shgPhases::~shgPhases() {
   for (unsigned i=0; i < theShgPhases.size(); i++)
      delete theShgPhases[i].theShg;

   currShgPhaseIndex = UINT_MAX;
}

shgPhases::shgStruct &shgPhases::getByIDLL(int phaseID) {
   for (unsigned i=0; i < theShgPhases.size(); i++) {
      if (theShgPhases[i].getPhaseId() == phaseID)
         return theShgPhases[i];
   }
   assert(false);
   abort();
}

shg &shgPhases::getByID(int phaseID) {
   return *(getByIDLL(phaseID).theShg);
}

int shgPhases::name2id(const string &phaseName) const {
   // returns -1 if not found
   for (unsigned i=0; i < theShgPhases.size(); i++)
      if (phaseName == theShgPhases[i].phaseName)
         return theShgPhases[i].getPhaseId();

   return -1;
}

const string &shgPhases::id2name(int id) const {
   for (unsigned lcv=0; lcv < theShgPhases.size(); lcv++)
      if (theShgPhases[lcv].getPhaseId() == id)
         return theShgPhases[lcv].phaseName;
   assert(false);
   abort();
}

bool shgPhases::changeLL(unsigned newIndex) {
   // returns true iff any changes, in which case you should redraw
   // NOTE: Like currShgPhaseIndex, the param newIndex is an index into
   //       the low-level array theShgPhases; it is not a dagid/phaseid!
   if (newIndex == currShgPhaseIndex)
      return false;

   if (existsCurrent()) {
      // Save current scrollbar values
      shgStruct &theShgStruct=theShgPhases[currShgPhaseIndex];

      string commandStr = horizSBName + " get";
      myTclEval(interp, commandStr);
      assert(2==sscanf(interp->result, "%f %f", &theShgStruct.horizSBfirst,
		       &theShgStruct.horizSBlast));

      commandStr = vertSBName + " get";
      myTclEval(interp, commandStr);
      assert(2==sscanf(interp->result, "%f %f", &theShgStruct.vertSBfirst,
		       &theShgStruct.vertSBlast));
   }

   // Set new scrollbar values:
   shgStruct &theNewShgStruct=theShgPhases[currShgPhaseIndex = newIndex];

   string commandStr = horizSBName + " set " +
                       string(theNewShgStruct.horizSBfirst) + " " +
                       string(theNewShgStruct.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set " + string(theNewShgStruct.vertSBfirst) + " " +
                string(theNewShgStruct.vertSBlast);
   myTclEval(interp, commandStr);

   // Set the Search(Resume) and Pause buttons:
   const string searchButtonName = ".shg.nontop.buttonarea.left.search";
   const string pauseButtonName  = ".shg.nontop.buttonarea.middle.pause";
   if (!theNewShgStruct.everSearched) {
      commandStr = searchButtonName + " config -text \"Search\" -state normal";
      myTclEval(interp, commandStr);

      commandStr = pauseButtonName + " config -state disabled";
      myTclEval(interp, commandStr);
   }
   else if (theNewShgStruct.currSearching) {
      commandStr = searchButtonName + " config -text \"Resume\" -state disabled";
      myTclEval(interp, commandStr);

      commandStr = pauseButtonName + " config -state normal";
      myTclEval(interp, commandStr);
   }
   else {
      // we are currently paused
      commandStr = searchButtonName + " config -text \"Resume\" -state normal";
      myTclEval(interp, commandStr);

      commandStr = pauseButtonName + " config -state disabled";
      myTclEval(interp, commandStr);
   }

   // This should update the menu:
   Tcl_SetVar(interp, "currShgPhase",
	      string(theNewShgStruct.getPhaseId()).string_of(), TCL_GLOBAL_ONLY);

   // Update the label containing the current phase name:
   commandStr = string(".shg.nontop.currphasearea.label2 config -text ") +
	           "\"" + theNewShgStruct.phaseName + "\"";
   myTclEval(interp, commandStr);

   // We must resize, since newly displayed shg had been set aside (?)
   theNewShgStruct.theShg->resize(true);
   
   return true;
}

bool shgPhases::changeByPhaseId(int id) {
   // returns true iff changes were made, in which case you should redraw
   for (unsigned i=0; i < theShgPhases.size(); i++) {
      shgStruct &theShgStruct = theShgPhases[i];
      if (theShgStruct.getPhaseId() == id)
         return changeLL(i);
   }

   return false; // could not find any phase with that name
}

bool shgPhases::change(const string &newPhaseName) {
   // returns true iff successful, in which case you should redraw
   for (unsigned i=0; i < theShgPhases.size(); i++) {
      shgStruct &theShgStruct = theShgPhases[i];
      if (theShgStruct.phaseName == newPhaseName)
         return changeLL(i);
   }

   return false; // could not find any phase with that name
}

shg &shgPhases::getCurrent() {
   assert(existsCurrent());
   assert(theShgPhases[currShgPhaseIndex].theShg);
   return *(theShgPhases[currShgPhaseIndex].theShg);
}

bool shgPhases::existsById(int id) const {
   for (unsigned lcv=0; lcv < theShgPhases.size(); lcv++) {
      const shgStruct &theStruct = theShgPhases[lcv];

      if (theStruct.getPhaseId() == id)
         return true;
   }

   return false;
}

const shg &shgPhases::getCurrent() const {
   assert(existsCurrent());
   assert(theShgPhases[currShgPhaseIndex].theShg);
   return *(theShgPhases[currShgPhaseIndex].theShg);
}

void shgPhases::resizeEverything() {
   if (!existsCurrent())
      return;

   for (unsigned i=0; i < theShgPhases.size(); i++) {
      theShgPhases[i].theShg->recursiveRethinkLayout();
      theShgPhases[i].theShg->resize(i==currShgPhaseIndex);
   }
}   

/* ************************************************************ */

void shgPhases::draw(bool doubleBuffer, bool isXsynchOn) const {
   if (existsCurrent())
      getCurrent().draw(doubleBuffer, isXsynchOn);
}

bool shgPhases::resize() {
   // returns true if a redraw is called for
   const bool ec = existsCurrent();
   if (ec)
      getCurrent().resize(true); // true --> we are curr shg
   return ec;
}

void shgPhases::processSingleClick(int x, int y) {
   if (existsCurrent())
      getCurrent().processSingleClick(x, y);
}

void shgPhases::processMiddleClick(int x, int y) {
   if (existsCurrent())
      getCurrent().processMiddleClick(x, y);
}

bool shgPhases::processDoubleClick(int x, int y) {
   // returns true if a redraw is called for
   const bool ec = existsCurrent();
   if (ec)
      getCurrent().processDoubleClick(x, y);
   return ec;
}

bool shgPhases::newVertScrollPosition(int argc, char **argv) {
   if (!existsCurrent())
      return false;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   getVertSBName(),
					   getCurrent().getVertSBOffset(), // <= 0
					   getCurrent().getTotalVertPixUsed(),
					   getCurrent().getVisibleVertPix(),
					   newFirst);
   if (anyChanges)
      anyChanges = getCurrent().adjustVertSBOffset(newFirst);

   return anyChanges;
}

bool shgPhases::newHorizScrollPosition(int argc, char **argv) {
   if (!existsCurrent())
      return false;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   getHorizSBName(),
					   getCurrent().getHorizSBOffset(), // <= 0
					   getCurrent().getTotalHorizPixUsed(),
					   getCurrent().getVisibleHorizPix(),
					   newFirst);
   if (anyChanges)
      anyChanges = getCurrent().adjustHorizSBOffset(newFirst);

   return anyChanges;
}

bool shgPhases::altPress(int x, int y) {
   // returns true if a redraw is called for (in which case a scroll
   // is done as well as an XWarpPointer)
   if (!existsCurrent())
      return false;

   if (currInstalledAltMoveHandler) {
      if (ignoreNextShgAltMove) {
         ignoreNextShgAltMove = false;
         return false;
      }

      int deltax = x - shgAltAnchorX;
      int deltay = y - shgAltAnchorY;

      // add some extra speedup juice as an incentive to use alt-mousemove scrolling
      deltax *= 4;
      deltay *= 4;

      getCurrent().adjustHorizSBOffsetFromDeltaPix(deltax);
      getCurrent().adjustVertSBOffsetFromDeltaPix(deltay);

      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   shgAltAnchorX, shgAltAnchorY);

      ignoreNextShgAltMove = true;
         
      return true;
   }
   else {
      shgAltAnchorX = x;
      shgAltAnchorY = y;

      currInstalledAltMoveHandler = true;
      return false;
   }
}

void shgPhases::altRelease() {
   if (currInstalledAltMoveHandler)
      currInstalledAltMoveHandler = false;
}

/* ************************************************************ */

bool shgPhases::defineNewSearch(int phaseId, const string &phaseName) {
   assert(!existsById(phaseId));

   shgStruct theStruct(phaseId, phaseName,
		       interp, theTkWindow,
		       horizSBName, vertSBName,
		       currItemLabelName);

   // possibly pull off the last phase...
   if (theShgPhases.size() > 1) // never touch the "Global Search"
      if (!theShgPhases[theShgPhases.size()-1].everSearched) {
         shgStruct &victimStruct = theShgPhases[theShgPhases.size()-1];
         
         cout << "shgPhases: throwing out never-used phase " << victimStruct.phaseName << endl;
         string commandStr = menuName + " delete " + string(theShgPhases.size());
         myTclEval(interp, commandStr);

         victimStruct.fryDag();

         theShgPhases.resize(theShgPhases.size()-1);
            // destructor for shgStruct will fry the shg
      }

   theShgPhases += theStruct;

   string commandStr = menuName + " add radiobutton -label " +
                       string::quote + phaseName + string::quote +
                       " -command " + string::quote + "shgChangePhase " +
                       string(phaseId) + string::quote +
                       " -variable currShgPhase -value " +
                       string(phaseId);
   myTclEval(interp, commandStr);

   const bool changeTo = (theShgPhases.size()==1);
   if (changeTo)
      if (change(phaseName))
         return true; // indicates a redraw is called for

   return false; // no redraw needed
}

bool shgPhases::activateCurrSearch() {
   // returns true iff successfully activated
   if (!existsCurrent())
      return false;

   int phaseId = getCurrentId();
   shgStruct &theStruct = getByIDLL(phaseId); // slow...

   assert(!theStruct.currSearching);

   // make the igen call to do the actual search:
#ifdef PARADYN
   perfConsult->activateSearch(phaseId);
#endif

   theStruct.everSearched = true;
   theStruct.currSearching = true;

   return true;
}

bool shgPhases::pauseCurrSearch() {
   // returns true iff successfully paused
   if (!existsCurrent())
      return false;

   int phaseId = getCurrentId();
   shgStruct &theStruct = getByIDLL(phaseId); // slow...

   assert(theStruct.everSearched);
   assert(theStruct.currSearching);

   // make the igen call to do the actual pausing:
#ifdef PARADYN
   perfConsult->pauseSearch(phaseId);
#endif

   theStruct.currSearching = false;

   return true;
}

bool shgPhases::resumeCurrSearch() {
   // returns true iff successfully resumed
   if (!existsCurrent())
      return false;

   int phaseId = getCurrentId();
   shgStruct &theStruct = getByIDLL(phaseId); // slow...

   assert(theStruct.everSearched);
   assert(theStruct.currSearching);

   // make the igen call to do the actual resuming:
#ifdef PARADYN
   perfConsult->activateSearch(phaseId);
#endif

   theStruct.currSearching = true;

   return true;
}

bool shgPhases::addNode(int phaseId, unsigned nodeId,
			bool active,
			shgRootNode::evaluationState es,
			const string &label, const string &fullInfo,
			bool rootNodeFlag) {
   // returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);
   theShg.addNode(nodeId, active, es, label, fullInfo, rootNodeFlag, isCurrShg);

   return isCurrShg;
}

bool shgPhases::addEdge(int phaseId, unsigned fromId, unsigned toId,
			shgRootNode::evaluationState es,
			const char *label // used only for shadow nodes, else NULL
			) {
   // The evaluationState param decides whether to explicitly expand
   // the "to" node.  Rethinks the entire layout of the shg
   // Returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);
   theShg.addEdge(fromId, toId, es, label, isCurrShg);

   return isCurrShg;
}

bool shgPhases::configNode(int phaseId, unsigned nodeId,
			   bool active, shgRootNode::evaluationState es) {
   // returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);
   theShg.configNode(nodeId, active, es, isCurrShg);

   return isCurrShg;
}

void shgPhases::addToStatusDisplay(int phaseId, const char *msg) {
   if (!existsCurrent()) {
      cerr << "addToStatusDisplay: no current phase to display msg:" << endl;
      cerr << msg << endl;
      return;
   }

   shg &theShg = getByID(phaseId);
   theShg.addToStatusDisplay(msg);
}
