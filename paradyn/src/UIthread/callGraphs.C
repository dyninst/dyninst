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

// $Id: callGraphs.C,v 1.8 2002/05/13 19:53:21 mjbrim Exp $

#include <limits.h>
#include "callGraphTcl.h"
#ifdef PARADYN
#include "paradyn/src/TCthread/tunableConst.h"
#include "performanceConsultant.thread.CLNT.h"
#endif
#include "callGraphs.h"
#include "tkTools.h"

callGraphs::callGraphs(const string &iMenuName,
		     const string &iHorizSBName, const string &iVertSBName,
		     const string &iCurrItemLabelName,
		     const string &iCurrProgramLabelName,
		     Tcl_Interp *iInterp, Tk_Window iTkWindow) :
		        menuName(iMenuName), horizSBName(iHorizSBName),
			vertSBName(iVertSBName),
			currItemLabelName(iCurrItemLabelName),
			currProgramLabelName(iCurrProgramLabelName)
{
   currCallGraphProgramIndex = UINT_MAX;
   interp = iInterp;
   theTkWindow = iTkWindow;
   currInstalledAltMoveHandler=false;
   ignoreNextCallGraphAltMove=false;
}

callGraphs::~callGraphs() {
   for (unsigned i=0; i < theCallGraphPrograms.size(); i++)
      delete theCallGraphPrograms[i].theCallGraphDisplay;

   currCallGraphProgramIndex = UINT_MAX;
}

void callGraphs::changeNameStyle(bool fullName){
  assert(existsCurrent());
  getCurrent().changeNameStyle(fullName);
  getCurrent().rethinkEntireLayout();
  initiateCallGraphRedraw(interp, true);
}

callGraphs::callGraphStruct &callGraphs::getByIDLL(int programID) {
  
   for (unsigned i=0; i < theCallGraphPrograms.size(); i++) {
      if (theCallGraphPrograms[i].getProgramId() == programID)
         return theCallGraphPrograms[i];
   }

   cerr << "call graph program: program id " << programID 
	<< " doesn't exist." << endl;
   assert(false);
   abort();
   return theCallGraphPrograms[0]; // placate VC++5.0 compiler
}

callGraphDisplay &callGraphs::getByID(int programID) {
   return *(getByIDLL(programID).theCallGraphDisplay);
}

int callGraphs::name2id(const string &programName) const {
   // returns -1 if not found
   for (unsigned i=0; i < theCallGraphPrograms.size(); i++)
      if (programName == theCallGraphPrograms[i].executable_name)
         return theCallGraphPrograms[i].getProgramId();
   return -1;
}

const string &callGraphs::id2name(int id) const {
   for (unsigned lcv=0; lcv < theCallGraphPrograms.size(); lcv++)
      if (theCallGraphPrograms[lcv].getProgramId() == id)
         return theCallGraphPrograms[lcv].executable_name;
   cerr << "call graph program: program id " << id
	<< " doesn't exist." << endl;
   assert(false);
   abort();
   return ""; // placate VC++5.0 compiler (somewhat)
}

int callGraphs::find(const string &str){
  assert(existsCurrent());
  return getCurrent().find(str);
}

bool callGraphs::changeLL(unsigned newIndex) {
   // returns true iff any changes, in which case you should redraw
   // NOTE: Like currShgPhaseIndex, the param newIndex is an index into
   //       the low-level array theCallGraphPrograms
   if (newIndex == currCallGraphProgramIndex)
      return false;

   if (existsCurrent()) {
      // Save current scrollbar values
      callGraphStruct &theCallGraphStruct=theCallGraphPrograms[currCallGraphProgramIndex];

      string commandStr = horizSBName + " get";
      myTclEval(interp, commandStr);
      bool aflag;
      aflag=(2==sscanf(Tcl_GetStringResult(interp),
            "%f %f",&theCallGraphStruct.horizSBfirst,
		       &theCallGraphStruct.horizSBlast));
      assert(aflag);

      commandStr = vertSBName + " get";
      myTclEval(interp, commandStr);
      aflag=(2==sscanf(Tcl_GetStringResult(interp),
          "%f %f",&theCallGraphStruct.vertSBfirst,
		       &theCallGraphStruct.vertSBlast));
      assert(aflag);
   }

   // Set new scrollbar values:
   callGraphStruct &theNewCallGraphStruct=
     theCallGraphPrograms[currCallGraphProgramIndex = newIndex];

   string commandStr = horizSBName + " set " +
                       string(theNewCallGraphStruct.horizSBfirst) + " " +
                       string(theNewCallGraphStruct.horizSBlast);
   myTclEval(interp, commandStr);

   commandStr = vertSBName + " set "+string(theNewCallGraphStruct.vertSBfirst)
     + " " + string(theNewCallGraphStruct.vertSBlast);
   myTclEval(interp, commandStr);

   // This should update the menu:
   Tcl_SetVar(interp, "currCallGraphProgram",
	      const_cast<char *>(string(theNewCallGraphStruct.getProgramId()).c_str()), TCL_GLOBAL_ONLY);

   // Update the label containing the current phase name:
   commandStr = currProgramLabelName + " config -text \"" +
                theNewCallGraphStruct.executable_name + "\"";
   myTclEval(interp, commandStr);

   theNewCallGraphStruct.theCallGraphDisplay->resize(true);
   
   return true;
}

bool callGraphs::changeByProgramId(int id) {
   // returns true iff changes were made, in which case you should redraw
   for (unsigned i=0; i < theCallGraphPrograms.size(); i++) {
      callGraphStruct &theCallGraphStruct = theCallGraphPrograms[i];
      if (theCallGraphStruct.getProgramId() == id)
         return changeLL(i);
   }

   return false; // could not find any phase with that name
}

bool callGraphs::change(const string &exe_name) {
   // returns true iff successful, in which case you should redraw
   for (unsigned i=0; i < theCallGraphPrograms.size(); i++) {
      callGraphStruct &theCallGraphStruct = theCallGraphPrograms[i];
      if (theCallGraphStruct.executable_name == exe_name)
         return changeLL(i);
   }
   return false; // could not find any phase with that name
}

callGraphDisplay &callGraphs::getCurrent() {
  assert(existsCurrent());
  assert(theCallGraphPrograms[currCallGraphProgramIndex].theCallGraphDisplay);
  return *(theCallGraphPrograms[currCallGraphProgramIndex].theCallGraphDisplay);
}

bool callGraphs::existsById(int id) const {
   for (unsigned lcv=0; lcv < theCallGraphPrograms.size(); lcv++) {
      const callGraphStruct &theCallGraphStruct = theCallGraphPrograms[lcv];

      if (theCallGraphStruct.getProgramId() == id)
         return true;
   }

   return false;
}

const callGraphDisplay &callGraphs::getCurrent() const {
   assert(existsCurrent());
   assert(theCallGraphPrograms[currCallGraphProgramIndex].theCallGraphDisplay);
   return *(theCallGraphPrograms[currCallGraphProgramIndex].theCallGraphDisplay);
}

void callGraphs::resizeEverything() {
   if (!existsCurrent())
      return;

   for (unsigned i=0; i < theCallGraphPrograms.size(); i++) {
     theCallGraphPrograms[i].theCallGraphDisplay->resize(i==currCallGraphProgramIndex);
   }
}   

void callGraphs::draw(bool doubleBuffer, bool isXsynchOn) const {
   if (existsCurrent())
      getCurrent().draw(doubleBuffer, isXsynchOn);
}

bool callGraphs::resize() {
   // returns true if a redraw is called for
   const bool ec = existsCurrent();
   if (ec)
      getCurrent().resize(true); // true --> we are curr shg
   return ec;
}

void callGraphs::processSingleClick(int x, int y) {
   if (existsCurrent())
      getCurrent().processSingleClick(x, y);
}

bool callGraphs::processDoubleClick(int x, int y) {
   // returns true if a redraw is called for
   const bool ec = existsCurrent();
   if (ec)
      getCurrent().processDoubleClick(x, y);
   return ec;
}

bool callGraphs::newVertScrollPosition(int argc, char **argv) {
   if (!existsCurrent())
      return false;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   getVertSBName(),
					   getCurrent().getVertSBOffset(),
					   getCurrent().getTotalVertPixUsed(),
					   getCurrent().getVisibleVertPix(),
					   newFirst);
   if (anyChanges)
      anyChanges = getCurrent().adjustVertSBOffset(newFirst);

   return anyChanges;
}

bool callGraphs::newHorizScrollPosition(int argc, char **argv) {
   if (!existsCurrent())
      return false;

   float newFirst;
   bool anyChanges = processScrollCallback(interp, argc, argv,
					   getHorizSBName(),
					   getCurrent().getHorizSBOffset(), 
					   getCurrent().getTotalHorizPixUsed(),
					   getCurrent().getVisibleHorizPix(),
					   newFirst);
   if (anyChanges)
      anyChanges = getCurrent().adjustHorizSBOffset(newFirst);

   return anyChanges;
}

bool callGraphs::altPress(int x, int y) {
   // returns true if a redraw is called for (in which case a scroll
   // is done as well as an XWarpPointer)
   if (!existsCurrent())
      return false;

   if (currInstalledAltMoveHandler) {
      if (ignoreNextCallGraphAltMove) {
         ignoreNextCallGraphAltMove = false;
         return false;
      }

      int deltax = x - callGraphAltAnchorX;
      int deltay = y - callGraphAltAnchorY;

      // add some extra speedup juice as an incentive to use 
      // alt-mousemove scrolling
      deltax *= 4;
      deltay *= 4;

      getCurrent().adjustHorizSBOffsetFromDeltaPix(deltax);
      getCurrent().adjustVertSBOffsetFromDeltaPix(deltay);
#if !defined(i386_unknown_nt4_0)
      XWarpPointer(Tk_Display(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   Tk_WindowId(theTkWindow),
		   0, 0, 0, 0,
		   callGraphAltAnchorX, callGraphAltAnchorY);
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement warping support
#endif // !defined(i386_unknown_nt4_0)

      ignoreNextCallGraphAltMove = true;
         
      return true;
   }
   else {
      callGraphAltAnchorX = x;
      callGraphAltAnchorY = y;

      currInstalledAltMoveHandler = true;
      return false;
   }
}

void callGraphs::altRelease() {
   if (currInstalledAltMoveHandler)
      currInstalledAltMoveHandler = false;
}


//This should be called when a new call graph root node is created

bool callGraphs::addNewProgram(int programId, resourceHandle rootId, 
			       const string &executableName,
			       const string &shortName,const string &fullName)
{
  assert(!existsById(programId));
  
  callGraphStruct theStruct(programId, rootId, executableName, shortName,
			    fullName, interp, theTkWindow,
			    horizSBName, vertSBName);
  
  bool result = false; // so far, nothing has changed
  
  // possibly pull off the last phase...
  if (theCallGraphPrograms.size() > 1) // never touch the "Global Search"
    {
      callGraphStruct &victimStruct = 
	theCallGraphPrograms[theCallGraphPrograms.size()-1];
  
      string commandStr = menuName + " delete " + 
	string(theCallGraphPrograms.size());
      myTclEval(interp, commandStr);
      
      victimStruct.fryDag();
      
      theCallGraphPrograms.resize(theCallGraphPrograms.size()-1);
      // destructor for shgStruct will fry the shg
      
      if (currCallGraphProgramIndex == theCallGraphPrograms.size()) {
	// uh oh.  We fried the current CG.  We need to make
	// someone else the current search.  But who?  We'll just
	// make it the previous call graph. 
	assert(theCallGraphPrograms.size() > 0);
	changeLL(theCallGraphPrograms.size()-1);
	result = true;
      }
    }
  
  theCallGraphPrograms += theStruct;
  
  string commandStr = menuName + " add radiobutton -label " +
    "\"" + executableName + "\"" +
    " -command " + "\"" + "callGraphChangeProgram " +
    string(programId) + "\"" +
    " -variable currCallGraphProgram -value " +
    string(programId);
  myTclEval(interp, commandStr); 
  
  const bool changeTo = (theCallGraphPrograms.size()==1);
  if (changeTo)
    if (change(executableName))
      result = true; // indicates Size redraw is called for
  
    return result;
  
}

void callGraphs::rethinkLayout(int pid){
  
  callGraphDisplay &theCallGraphDisplay = getByID(pid);
  theCallGraphDisplay.rethinkEntireLayout();
}

bool callGraphs::addNode(int programId,resourceHandle parent, 
			 resourceHandle newResource,
			 const char *shortName, const char *fullName,
			 const bool recursiveFlag, bool isShadowNode)
{
   // returns true iff a redraw should take place
   callGraphDisplay &theCallGraphDisplay = getByID(programId);
   
   theCallGraphDisplay.addItem(shortName, fullName,parent, newResource, 
			       recursiveFlag,isShadowNode,false,false);
   return true;
}

