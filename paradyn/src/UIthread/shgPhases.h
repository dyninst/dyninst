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

// shgPhases.h
// Ariel Tamches
// Analagous to "abstractions.h" for the where axis; this class
// basically manages several "shg"'s, as defined in shgPhases.h

/* $Id: shgPhases.h,v 1.25 2003/06/20 02:12:19 pcroth Exp $ */

#ifndef _SHG_PHASES_H_
#define _SHG_PHASES_H_

#ifndef PARADYN
// The test program has "correct" -I paths already set
#include "Vector.h"
#else
#include "common/h/Vector.h"
#endif

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
		const string &currItemLabelName,
		bool showTrueNodes, bool showFalseNodes, bool showUnknownNodes,
		bool showNeverSeenNodes, bool showActiveNodes, bool showInactiveNodes,
		bool showShadowNodes
		) : phaseName(iPhaseName) {
         shg *theNewShg = new shg(phaseId, interp, theTkWindow,
				  horizSBName, vertSBName,
				  currItemLabelName,
				  showTrueNodes, showFalseNodes, showUnknownNodes,
				  showNeverSeenNodes, showActiveNodes,
				  showInactiveNodes, showShadowNodes);
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

   pdvector<shgStruct> theShgPhases;
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

   // values of "tunable constants" saying which node types should be hidden:
   bool showTrueNodes;
   bool showFalseNodes;
   bool showUnknownNodes;
   bool showNeverSeenNodes;
   bool showActiveNodes;
   bool showInactiveNodes;
   bool showShadowNodes;

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

   bool newVertScrollPosition(int argc, TCLCONST char **argv);
   bool newHorizScrollPosition(int argc, TCLCONST char **argv);

   bool altPress(int x, int y);
   void altRelease();

   bool defineNewSearch(int phaseId, const string &phaseName);
   bool activateCurrSearch();
      // returns true iff search successfully started
   bool pauseCurrSearch();
      // returns true iff search successfully paused
   bool resumeCurrSearch();
      // returns true iff search successfully resumed

   bool changeHiddenNodes(bool newShowTrue, bool newShowFalse, bool newShowUnknown,
                          bool newShowNeverSeen, bool newShowActive,
                          bool newShowInactive, bool newShowShadow);
      // simply calls changeHiddenNodes() for every shg.  Returns true iff any changes.
   bool changeHiddenNodes(shg::changeType ct, bool show);
      // like above but just changes one characteristic.

   bool addNode(int phaseId, unsigned nodeId,
                bool active,
                shgRootNode::evaluationState,
                bool deferred,
                const string &label, const string &fullInfo,
                bool rootNodeFlag);
   bool addEdge(int phaseId, unsigned fromId, unsigned toId,
                shgRootNode::refinement,
                const char *label, // used only for shadow nodes, else NULL
                bool rethinkFlag); // if false avoids rethinkification
      // The evaluationState param decides whether to explicitly expand
      // the "to" node.  Rethinks the entire layout of the shg
   bool configNode(int phaseId, unsigned nodeId,
                   bool active, shgRootNode::evaluationState, bool deferred);
   bool inactivateEntireSearch(int phaseId);

   void addToStatusDisplay(int phaseId, const string &msg);
      // currently, we do not append the newline character for you

#ifdef PARADYN
   void nodeInformation(int phaseId, int nodeId, const shg_node_info &theNodeInfo);
      // in response to a middle-mouse-click...
#endif
};

#endif
