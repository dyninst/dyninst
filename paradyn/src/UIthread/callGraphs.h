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

// $Id: callGraphs.h,v 1.6 2001/02/12 14:53:06 wxd Exp $

//callGraphs.h: the callGraphs class, whose code is taken  
//from the shgPhases class, is just used to keep track of multiple 
//call graphs. For instance, if some parallel program has more than one
//binary, we will need to display a different call graph for each. This
//feature is not yet implemented, so only one call graph may be displayed at
//any time, for now...

#ifndef _CALLGRAPHS_H_
#define _CALLGRAPHS_H_

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#else
#include "common/h/Vector.h"
#endif

#include "callGraphDisplay.h"

#include "tcl.h"
#include "tk.h"

class callGraphs {
 private:
   struct callGraphStruct {
      callGraphDisplay *theCallGraphDisplay; 
     // note: destructor must not fry (since constructor
     //       does not copy)
     string executable_name;
     string shortName;
     string fullName;
     
      // Save shg UI-type state when changing phases, so we can restore
      // it later:
      float horizSBfirst, horizSBlast;
      float vertSBfirst, vertSBlast;

       // convenience routine:
      int getProgramId() const {return theCallGraphDisplay->getProgramId();}
      
      callGraphStruct() {theCallGraphDisplay=NULL;} // needed by Vector class

      callGraphStruct(unsigned programId, resourceHandle rootId, 
		      const string &exe_name, const string &ishortName, 
		      const string &ifullName, Tcl_Interp *interp, 
		      Tk_Window theTkWindow, const string &horizSBName, 
		      const string &vertSBName) : executable_name(exe_name),
	shortName(ishortName), 
	fullName(ifullName) {
	
	callGraphDisplay *theNewCallGraphDisplay = 
	  new callGraphDisplay(programId, rootId, interp, theTkWindow,
			       executable_name, shortName, fullName,
			       horizSBName, vertSBName);
	assert(theNewCallGraphDisplay);
	
	this->theCallGraphDisplay = theNewCallGraphDisplay;
	this->horizSBfirst = this->vertSBfirst = 0.0;
	this->horizSBlast = this->vertSBlast = 1.0;
      }
      callGraphStruct(const callGraphStruct &src) : 
	executable_name(src.executable_name), shortName(src.shortName),
	  fullName(src.fullName)
       {
         theCallGraphDisplay = src.theCallGraphDisplay;
         horizSBfirst = src.horizSBfirst;
         horizSBlast = src.horizSBlast;
         vertSBfirst = src.vertSBfirst;
         vertSBlast = src.vertSBlast;
	 
       }
      void fryDag() {delete theCallGraphDisplay;}
     ~callGraphStruct() {}
     
     callGraphStruct &operator=(const callGraphStruct &src) {
       theCallGraphDisplay = src.theCallGraphDisplay; // is this right?
       executable_name = src.executable_name;
       shortName = src.shortName;
       fullName = src.fullName;
       horizSBfirst = src.horizSBfirst;
       horizSBlast = src.horizSBlast;
       vertSBfirst = src.vertSBfirst;
       vertSBlast = src.vertSBlast;
       return *this;
     }
   };
   
   vector<callGraphStruct> theCallGraphPrograms;
   unsigned currCallGraphProgramIndex;
      // NOTE: An index into the above array
   
   Tcl_Interp *interp;
   Tk_Window theTkWindow;   
   
   string menuName, horizSBName, vertSBName;
   string currItemLabelName;
   string currProgramLabelName;

   bool currInstalledAltMoveHandler;
   bool ignoreNextCallGraphAltMove;
   int callGraphAltAnchorX;
   int callGraphAltAnchorY;

   callGraphDisplay &getByID(int programID);
   callGraphStruct &getByIDLL(int programID);
   bool changeLL(unsigned newIndex);
      // returns true iff any changes

   callGraphDisplay &getCurrent();
   const callGraphDisplay &getCurrent() const;

 public:

   callGraphs(const string &iMenuName,
	      const string &iHorizSBName, const string &iVertSBName,
	      const string &iCurrItemLabelName,
	      const string &iCurrProgramLabelName,
             Tcl_Interp *iInterp, Tk_Window iTkWindow);
  ~callGraphs();

   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName() const {return vertSBName;}

   int name2id (const string &fullName) const;
      // returns -1 if not found
   const string &id2name (int id) const;

   int find(const string &str);
   // uses and updates "beginSearchFromPtr"
   // returns 0 if not found; 1 if found & no expansion needed;
   // 2 if found & some expansion is needed

   void changeNameStyle(bool fullName);
   bool changeByProgramId(int newIndex);
      // returns true iff any changes
   bool change(const string &exe_name);
      // returns true iff any changes

   bool existsCurrent() const {return currCallGraphProgramIndex < theCallGraphPrograms.size();}
   bool existsById(int programId) const;
   
   int getCurrentId() const {
      assert(existsCurrent());
      return theCallGraphPrograms[currCallGraphProgramIndex].getProgramId();
   }

   void resizeEverything();
   void rethinkLayout(int pid);
   void draw(bool doubleBuffer, bool isXsynchOn) const;

   bool resize();

   void processSingleClick(int x, int y);
   bool processDoubleClick(int x, int y);

   bool newVertScrollPosition(int argc, char **argv);
   bool newHorizScrollPosition(int argc, char **argv);

   bool altPress(int x, int y);
   void altRelease();

   bool addNewProgram(int programId, resourceHandle rootId, const string &executableName,
		      const string &shortName, const string &longName);
         
   bool addNode(int programId, resourceHandle parent,
                resourceHandle newResource, const char *shortName, 
		const char *fullName, const bool recursiveFlag,
		bool isShadowNode);

   void addToStatusDisplay(int programId, const string &msg);
      // currently, we do not append the newline character for you

   void map_from_WhereAxis(const string &module_name,const string &func_name,bool ishighlight) {
   	getCurrent().map_from_WhereAxis(module_name,func_name,ishighlight);
   }
};

#endif
