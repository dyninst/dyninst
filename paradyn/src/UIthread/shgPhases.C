/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

// $Id: shgPhases.C,v 1.34 2003/07/15 22:46:18 schendel Exp $
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

#include <limits.h>
#include "common/h/headers.h"
#include "tkTools.h" // myTclEval()
#include "shgPhases.h"

#ifdef PARADYN
#include "paradyn/src/TCthread/tunableConst.h"
#include "performanceConsultant.thread.CLNT.h"
extern performanceConsultantUser *perfConsult;
#endif

shgPhases::shgPhases(const pdstring &iMenuName,
		     const pdstring &iHorizSBName, const pdstring &iVertSBName,
		     const pdstring &iCurrItemLabelName,
		     const pdstring &iMsgTextWindowName,
		     const pdstring &iSearchButtonName,
		     const pdstring &iPauseButtonName,
		     const pdstring &iCurrPhaseLabelName,
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
   showTrueNodes = tunableConstantRegistry::findBoolTunableConstant("showShgTrueNodes").getValue();
   showFalseNodes = tunableConstantRegistry::findBoolTunableConstant("showShgFalseNodes").getValue();
   showUnknownNodes = tunableConstantRegistry::findBoolTunableConstant("showShgUnknownNodes").getValue();
   showNeverSeenNodes = tunableConstantRegistry::findBoolTunableConstant("showShgNeverSeenNodes").getValue();
   showActiveNodes = tunableConstantRegistry::findBoolTunableConstant("showShgActiveNodes").getValue();
   showInactiveNodes = tunableConstantRegistry::findBoolTunableConstant("showShgInactiveNodes").getValue();
   showShadowNodes = tunableConstantRegistry::findBoolTunableConstant("showShgShadowNodes").getValue();
#else
   showTrueNodes = false;
   showFalseNodes = false;
   showUnknownNodes = false;
   showNeverSeenNodes = false;
   showActiveNodes = false;
   showInactiveNodes = false;
   showShadowNodes = false;
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
   return theShgPhases[0];	// placate compiler
}

shg &shgPhases::getByID(int phaseID) {
   return *(getByIDLL(phaseID).theShg);
}

int shgPhases::name2id(const pdstring &phaseName) const {
   // returns -1 if not found
   for (unsigned i=0; i < theShgPhases.size(); i++)
      if (phaseName == theShgPhases[i].phaseName)
         return theShgPhases[i].getPhaseId();

   return -1;
}

const pdstring &shgPhases::id2name(int id) const {
   for (unsigned lcv=0; lcv < theShgPhases.size(); lcv++)
      if (theShgPhases[lcv].getPhaseId() == id)
         return theShgPhases[lcv].phaseName;
   assert(false);
   return NULL;	// placate compiler
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

      pdstring commandStr = horizSBName + " get";
      myTclEval(interp, commandStr);
      bool aflag;
      aflag=(2==sscanf(Tcl_GetStringResult(interp),
          "%f %f", &theShgStruct.horizSBfirst,
		       &theShgStruct.horizSBlast));
      assert(aflag);

      commandStr = vertSBName + " get";
      myTclEval(interp, commandStr);
      aflag=(2==sscanf(Tcl_GetStringResult(interp),
          "%f %f", &theShgStruct.vertSBfirst,
		       &theShgStruct.vertSBlast));
      assert(aflag);
   }

   // Set new scrollbar values:
   shgStruct &theNewShgStruct=theShgPhases[currShgPhaseIndex = newIndex];

   pdstring commandStr = horizSBName + " set " +
                       pdstring(theNewShgStruct.horizSBfirst) + " " +
                       pdstring(theNewShgStruct.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set " + pdstring(theNewShgStruct.vertSBfirst) + " " +
                pdstring(theNewShgStruct.vertSBlast);
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
	      (char*)pdstring(theNewShgStruct.getPhaseId()).c_str(), TCL_GLOBAL_ONLY);

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

bool shgPhases::change(const pdstring &newPhaseName) {
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

bool shgPhases::newVertScrollPosition(int argc, TCLCONST char **argv) {
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

bool shgPhases::newHorizScrollPosition(int argc, TCLCONST char **argv) {
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

#if !defined(i386_unknown_nt4_0)
      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   shgAltAnchorX, shgAltAnchorY);
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement warping support
#endif // !defined(i386_unknown_nt4_0)

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

bool shgPhases::defineNewSearch(int phaseId, const pdstring &phaseName) {
   assert(!existsById(phaseId));

   shgStruct theStruct(phaseId, phaseName,
		       interp, theTkWindow,
		       horizSBName, vertSBName,
		       currItemLabelName,
		       showTrueNodes, showFalseNodes, showUnknownNodes,
		       showNeverSeenNodes, showActiveNodes, showInactiveNodes,
		       showShadowNodes);

   bool result = false; // so far, nothing has changed

   // possibly pull off the last phase...
   if (theShgPhases.size() > 1) // never touch the "Global Search"
      if (!theShgPhases[theShgPhases.size()-1].everSearched) {
         shgStruct &victimStruct = theShgPhases[theShgPhases.size()-1];
         
//         cout << "shgPhases: throwing out never-searched phase id " << victimStruct.getPhaseId() << " " << victimStruct.phaseName << endl;
         pdstring commandStr = menuName + " delete " + pdstring(theShgPhases.size());
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

   pdstring commandStr = menuName + " add radiobutton -label " +
                       "\"" + phaseName + "\"" +
                       " -command " + "\"" + "shgChangePhase " +
                       pdstring(phaseId) + "\"" +
                       " -variable currShgPhase -value " +
                       pdstring(phaseId);
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

bool shgPhases::changeHiddenNodes(bool newShowTrue, bool newShowFalse,
				  bool newShowUnknown, bool newShowNeverSeen,
				  bool newShowActive, bool newShowInactive,
				  bool newShowShadow) {
   if (showTrueNodes == newShowTrue && showFalseNodes == newShowFalse &&
       showUnknownNodes == newShowUnknown && showNeverSeenNodes == newShowNeverSeen &&
       showActiveNodes == newShowActive && showInactiveNodes == newShowInactive &&
       showShadowNodes == newShowShadow)
      return false; // nothing changed

   bool anyChanges = false; // so far
   for (unsigned shgindex=0; shgindex < theShgPhases.size(); shgindex++) {
      shgStruct &theShgStruct = theShgPhases[shgindex];
      shg *theShg = theShgStruct.theShg;

      if (theShg->changeHiddenNodes(newShowTrue, newShowFalse, newShowUnknown,
				    newShowNeverSeen, newShowActive, newShowInactive,
				    newShowShadow,
				    shgindex == currShgPhaseIndex))
         anyChanges = true;
   }

   return anyChanges;
}
bool shgPhases::changeHiddenNodes(shg::changeType ct, bool show) {
   switch (ct) {
      case shg::ct_true:
         if (showTrueNodes == show) return false;
	 showTrueNodes = show;
	 break;
       case shg::ct_false:
	 if (showFalseNodes == show) return false;
	 showFalseNodes = show;
	 break;
       case shg::ct_unknown:
	 if (showUnknownNodes == show) return false;
	 showUnknownNodes = show;
	 break;
       case shg::ct_never:
	 if (showNeverSeenNodes == show) return false;
	 showNeverSeenNodes = show;
	 break;
       case shg::ct_active:
	 if (showActiveNodes == show) return false;
	 showActiveNodes = show;
	 break;
       case shg::ct_inactive:
	 if (showInactiveNodes == show) return false;
	 showInactiveNodes = show;
	 break;
       case shg::ct_shadow:
	 if (showShadowNodes == show) return false;
	 showShadowNodes = show;
	 break;
       default:
	 assert(false);
   }

   bool anyChanges = false; // so far
   for (unsigned shgindex = 0; shgindex < theShgPhases.size(); shgindex++) {
      shgStruct &theShgStruct = theShgPhases[shgindex];
      shg *theShg = theShgStruct.theShg;
      if (theShg->changeHiddenNodes(ct, show, currShgPhaseIndex == shgindex))
	 anyChanges = true;
   }

   return anyChanges;
}

bool shgPhases::addNode(int phaseId, unsigned nodeId,
			bool active,
			shgRootNode::evaluationState es,
            bool deferred,
			const pdstring &label, const pdstring &fullInfo,
			bool rootNodeFlag) {
   // returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);

   theShg.addNode(nodeId, active, es, deferred,
                    label, fullInfo, rootNodeFlag, isCurrShg);

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
			   bool active, shgRootNode::evaluationState es, bool deferred)
{
   // returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);

   const shg::configNodeResult changes =
      theShg.configNode(nodeId, active, es, deferred, isCurrShg, true);
         // true --> rethink if needed

   return isCurrShg && (changes != shg::noChanges);
}

bool shgPhases::inactivateEntireSearch(int phaseId) {
   // returns true iff a redraw should take place
   shg &theShg = getByID(phaseId);
   const bool isCurrShg = (getCurrentId() == phaseId);
   const bool anyChanges = theShg.inactivateAll(isCurrShg);

   return isCurrShg && anyChanges;
}

void shgPhases::addToStatusDisplay(int phaseId, const pdstring &iMsg) {
   // currently, we do _not_ add a \n to the message for you.
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

   pdstring Msg;
   // auto-prepend "\n" for all but first message
   if (theShgStruct.msgText.length() > 1) Msg = pdstring("\n") + iMsg;
   else Msg=iMsg;

   theShgStruct.msgText += Msg;

   if (isCurrShg) {
      pdstring commandStr = msgTextWindowName + " insert end {" + Msg + "}";
      myTclEval(interp, commandStr);

      commandStr = msgTextWindowName + " yview -pickplace end";
      myTclEval(interp, commandStr);
   }
}

#ifdef PARADYN
void shgPhases::nodeInformation(int phaseId, int nodeId,
				const shg_node_info &theNodeInfo) {
   // in response to a middle-mouse-click...
   shg &theShg = getByID(phaseId);
   theShg.nodeInformation(nodeId, theNodeInfo);
}
#endif
