// shgPhases.C
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Log: shgPhases.C,v $
/* Revision 1.18  1996/04/24 21:58:07  tamches
/* We now do a yview -pickplace end on the "status window" area when changing
/* phases.
/*
 * Revision 1.17  1996/04/16 18:37:36  karavan
 * fine-tunification of UI-PC batching code, plus addification of some
 * Ari-like verbification commentification.
 *
 * Revision 1.16  1996/04/13 04:39:42  karavan
 * better implementation of batching for edge requests
 *
 * Revision 1.15  1996/04/09 19:25:15  karavan
 * added batch mode to cut down on shg redraw time.
 *
 * Revision 1.14  1996/04/08 19:54:56  tamches
 * fixed bug in shg message window that could leave some residue when
 * changing phases.
 *
 * Revision 1.13  1996/03/08 03:01:35  tamches
 * constructor now grabs initial hide-node flags from the actual paradyn
 * tunable constants
 *
 * Revision 1.12  1996/03/08 00:22:15  tamches
 * added support for hidden nodes
 *
 * Revision 1.11  1996/02/26 18:16:54  tamches
 * bug fix when changing phases
 *
 * Revision 1.10  1996/02/15 23:12:30  tamches
 * corrected parameters of addEdge to properly handle why vs. where
 * axis refinements
 *
 * Revision 1.9  1996/02/11 18:26:44  tamches
 * shg message window now works correctly for multiple phases
 * internal cleanup; more tk window name entities parameterized
 *
 * Revision 1.8  1996/02/07 21:51:35  tamches
 * defineNewSearch now returns bool should-redraw flag
 *
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
#include "paradyn/src/TCthread/tunableConst.h"
#include "performanceConsultant.thread.CLNT.h"
extern performanceConsultantUser *perfConsult;
#endif

shgPhases::shgPhases(const string &iMenuName,
		     const string &iHorizSBName, const string &iVertSBName,
		     const string &iCurrItemLabelName,
		     const string &iMsgTextWindowName,
		     const string &iSearchButtonName,
		     const string &iPauseButtonName,
		     const string &iCurrPhaseLabelName,
		     Tcl_Interp *iInterp, Tk_Window iTkWindow) :
		        menuName(iMenuName), horizSBName(iHorizSBName),
			vertSBName(iVertSBName),
			currItemLabelName(iCurrItemLabelName),
			msgTextWindowName(iMsgTextWindowName),
			searchButtonName(iSearchButtonName),
			pauseButtonName(iPauseButtonName),
			currPhaseLabelName(iCurrPhaseLabelName)
{
   currShgPhaseIndex = UINT_MAX;

   interp = iInterp;
   theTkWindow = iTkWindow;

   currInstalledAltMoveHandler=false;
   ignoreNextShgAltMove=false;

#ifdef PARADYN
   // grab initial values from the appropriate tunable constants
   hideTrueNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgTrueNodes").getValue();
   hideFalseNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgFalseNodes").getValue();
   hideUnknownNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgUnknownNodes").getValue();
   hideNeverSeenNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgNeverSeenNodes").getValue();
   hideActiveNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgActiveNodes").getValue();
   hideInactiveNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgInactiveNodes").getValue();
   hideShadowNodes = tunableConstantRegistry::findBoolTunableConstant("hideShgShadowNodes").getValue();
#else
   hideTrueNodes = hideFalseNodes = hideUnknownNodes = hideNeverSeenNodes = false;
   hideActiveNodes = hideInactiveNodes = hideShadowNodes = false;
#endif
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

   cerr << "shgPhases: phase id " << phaseID << " doesn't exist." << endl;
   assert(false);
   abort(); // placate compiler
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
   commandStr = currPhaseLabelName + " config -text \"" +
                theNewShgStruct.phaseName + "\"";
   myTclEval(interp, commandStr);

   // Update the message text:
   commandStr = msgTextWindowName + " delete 1.0 end";
      // this says 'delete from char 0 of line 1' (tk decrees that line 1 is the first line)
   myTclEval(interp, commandStr);

   commandStr = msgTextWindowName + " insert end \"" + theNewShgStruct.msgText + "\"";
   myTclEval(interp, commandStr);

   commandStr = msgTextWindowName + " yview -pickplace end";
   myTclEval(interp, commandStr);

   // We must resize, since newly displayed shg had been set aside (is this right?)
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

   cout << "shgPhases::changeByPhaseId(): phase id " << id << " doesn't exist" << endl;
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
		       currItemLabelName,
		       hideTrueNodes, hideFalseNodes, hideUnknownNodes,
		       hideNeverSeenNodes, hideActiveNodes, hideInactiveNodes,
		       hideShadowNodes);

   bool result = false; // so far, nothing has changed

   // possibly pull off the last phase...
   if (theShgPhases.size() > 1) // never touch the "Global Search"
      if (!theShgPhases[theShgPhases.size()-1].everSearched) {
         shgStruct &victimStruct = theShgPhases[theShgPhases.size()-1];
         
         cout << "shgPhases: throwing out never-searched phase id " << victimStruct.getPhaseId() << " " << victimStruct.phaseName << endl;
         string commandStr = menuName + " delete " + string(theShgPhases.size());
         myTclEval(interp, commandStr);

         victimStruct.fryDag();

         theShgPhases.resize(theShgPhases.size()-1);
            // destructor for shgStruct will fry the shg

         if (currShgPhaseIndex == theShgPhases.size()) {
            // uh oh.  We fried the current search.  We need to make
            // someone else the current search.  But who?  We'll just
            // make it the previous search index.  One will always exist,
            // since we'll never fry the global phase search.
            assert(theShgPhases.size() > 0);
            changeLL(theShgPhases.size()-1);
	    result = true;
	 }
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
         result = true; // indicates a redraw is called for

   return result;
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

bool shgPhases::changeHiddenNodes(bool newHideTrue, bool newHideFalse,
				  bool newHideUnknown, bool newHideNeverSeen,
				  bool newHideActive, bool newHideInactive,
				  bool newHideShadow) {
   if (hideTrueNodes == newHideTrue && hideFalseNodes == newHideFalse &&
       hideUnknownNodes == newHideUnknown && hideNeverSeenNodes == newHideNeverSeen &&
       hideActiveNodes == newHideActive && hideInactiveNodes == newHideInactive &&
       hideShadowNodes == newHideShadow)
      return false; // nothing changed

   bool anyChanges = false; // so far
   for (unsigned shgindex=0; shgindex < theShgPhases.size(); shgindex++) {
      shgStruct &theShgStruct = theShgPhases[shgindex];
      shg *theShg = theShgStruct.theShg;

      if (theShg->changeHiddenNodes(newHideTrue, newHideFalse, newHideUnknown,
				    newHideNeverSeen, newHideActive, newHideInactive,
				    newHideShadow,
				    shgindex == currShgPhaseIndex))
         anyChanges = true;
   }

   return anyChanges;
}
bool shgPhases::changeHiddenNodes(shg::changeType ct, bool hide) {
   switch (ct) {
      case shg::ct_true:
         if (hideTrueNodes == hide) return false;
	 hideTrueNodes = hide;
	 break;
       case shg::ct_false:
	 if (hideFalseNodes == hide) return false;
	 hideFalseNodes = hide;
	 break;
       case shg::ct_unknown:
	 if (hideUnknownNodes == hide) return false;
	 hideUnknownNodes = hide;
	 break;
       case shg::ct_never:
	 if (hideNeverSeenNodes == hide) return false;
	 hideNeverSeenNodes = hide;
	 break;
       case shg::ct_active:
	 if (hideActiveNodes == hide) return false;
	 hideActiveNodes = hide;
	 break;
       case shg::ct_inactive:
	 if (hideInactiveNodes == hide) return false;
	 hideInactiveNodes = hide;
	 break;
       case shg::ct_shadow:
	 if (hideShadowNodes == hide) return false;
	 hideShadowNodes = hide;
	 break;
       default:
	 assert(false);
   }

   bool anyChanges = false; // so far
   for (unsigned shgindex = 0; shgindex < theShgPhases.size(); shgindex++) {
      shgStruct &theShgStruct = theShgPhases[shgindex];
      shg *theShg = theShgStruct.theShg;
      if (theShg->changeHiddenNodes(ct, hide, currShgPhaseIndex == shgindex))
	 anyChanges = true;
   }

   return anyChanges;
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
			shgRootNode::refinement theRefinement,
			const char *label, // used only for shadow nodes, else NULL
			bool rethinkFlag) {
   // The evaluationState param decides whether to explicitly expand
   // the "to" node.  Rethinks the entire layout of the shg
   // Returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);
   theShg.addEdge(fromId, toId, theRefinement, label, isCurrShg, rethinkFlag);

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

void shgPhases::addToStatusDisplay(int phaseId, const string &iMsg) {
   if (!existsCurrent()) {
      cerr << "addToStatusDisplay: no current phase to display msg:" << endl;
      cerr << iMsg << endl;
      return;
   }

   if (!existsById(phaseId)) {
      //cerr << "addToStatusDisplay: no phase id " << phaseId << " exists to display msg:" << endl;
      //cerr << "\"" << iMsg << "\"" << endl;
      return;
   }

   const bool isCurrShg = (getCurrentId() == phaseId);
   shgStruct &theShgStruct = getByIDLL(phaseId);

   //const string msg = iMsg + "\n";
   //const string msg = iMsg;

   theShgStruct.msgText += iMsg;

   if (isCurrShg) {
      string commandStr = msgTextWindowName + " insert end {" + iMsg + "}";
      myTclEval(interp, commandStr);

      commandStr = msgTextWindowName + " yview -pickplace end";
      myTclEval(interp, commandStr);
   }
}
