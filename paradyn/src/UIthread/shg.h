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

// shg.h
// new search history graph user interface, along the lines
// of the new where axis user interface
// Ariel Tamches

/* $Id: shg.h,v 1.27 2002/12/20 07:50:05 jaw Exp $ */

#ifndef _SHG_H_
#define _SHG_H_

#ifndef PARADYN
#include "Dictionary.h"
#else
#include "common/h/Dictionary.h"
#endif

#ifdef PARADYN
#include "performanceConsultant.thread.h" // for struct shg_node_info
   // shg test program doesn't touch this stuff
#endif

#include "where4tree.h"
#include "graphicalPath.h"

#include "shgConsts.h"
#include "shgRootNode.h" // contrast with rootNode.h, the where axis root node class

class shg {
 private:
   // These are needed by shgRootNode:
   static Tk_Font theRootItemFontStruct, theRootItemShadowFontStruct;
   static Tk_Font theListboxItemFontStruct, theListboxItemShadowFontStruct;

   static pdvector<Tk_3DBorder> rootItemTk3DBordersByStyle;
      // array[shgRootNode::evaluationState]
   static pdvector<Tk_3DBorder> listboxItemTk3DBordersByStyle;
      // array[shgRootNode::evaluationState]
   static GC rootItemInactiveTextGC, rootItemActiveTextGC,
             rootItemInactiveShadowTextGC, rootItemActiveShadowTextGC;
   static GC listboxInactiveItemGC, listboxActiveItemGC,
             listboxInactiveShadowItemGC, listboxActiveShadowItemGC;
   static GC whyRefinementRayGC;
   static GC whereRefinementRayGC;
   static GC listboxRayGC;

   // this appears to be the WRONG class for the following vrbles:
   static int listboxBorderPix; // 3
   static int listboxScrollBarWidth; // 16

   // These ugly variables keep track of a button press (and possible hold-down)
   // in scrollbar-up/down or pageup/pagedown region.  They are not used for
   // press (and possible hold-down) on the scrollbar slider
   bool nonSliderButtonCurrentlyPressed; // init to false
   whereNodeGraphicalPath<shgRootNode>::pathEndsIn nonSliderButtonPressRegion;
   Tk_TimerToken buttonAutoRepeatToken;
   where4tree<shgRootNode> *nonSliderCurrentSubtree;
   int nonSliderSubtreeCenter; // WARNING: what if an expansion takes place during a drag (can happen with shg, tho not where axis)!
   int nonSliderSubtreeTop; // WARNING: what if an expansion takes place during a drag (can happen with shg, tho not where axis)!

   // Analagous to above; used only for scrollbar slider
   simpSeq<unsigned> slider_scrollbar_path;
      // why don't we keep track of left/top, etc.?  Because in the SHG, things
      // can expand at any time --- even when one is sliding a scrollbar.  Hence,
      // screen locations can change.  Hence it is safest to go with this slower approach.
   int slider_initial_yclick;
   int slider_initial_scrollbar_slider_top;
      // WARNING: what if an expansion takes place during a drag (can happen
      // with shg, tho not where axis)!
   where4tree<shgRootNode> *slider_currently_dragging_subtree;
      // rechecked at each use

   where4tree<shgRootNode> *rootPtr;
   dictionary_hash<unsigned, where4tree<shgRootNode> *> hash;
      // associative array: shg-node-id --> its corresponding data node
   dictionary_hash<where4tree<shgRootNode> *, where4tree<shgRootNode> *> hash2;
      // associative array: shg-node --> its parent
   dictionary_hash<unsigned, pdvector< where4tree<shgRootNode>* > > shadowNodeHash;
      // associative array: shg-node-id --> list of shadow nodes
      // An entry exists in this dictionary _only_ if shadow node(s) exist

   where4TreeConstants consts; // yuck
   shgConsts theShgConsts;
   int thePhaseId; // new

   Tcl_Interp *interp;

   string horizSBName; // tk window name
   string vertSBName; // tk window name

   int nominal_centerx; // actual centerx = nominal_centerx + horizScrollBarOffset
   int horizScrollBarOffset; // always <= 0
   int vertScrollBarOffset; // always <= 0

   string currItemLabelName; // tk window name
   whereNodeGraphicalPath<shgRootNode> lastItemUnderMousePath;
   int lastItemUnderMouseX, lastItemUnderMouseY;

   // values of "tunable constants" saying which node types should be hidden:
   bool showTrueNodes;
   bool showFalseNodes;
   bool showUnknownNodes;
   bool showNeverSeenNodes;
   bool showActiveNodes;
   bool showInactiveNodes;
   bool showShadowNodes;

   void resizeScrollbars();

   bool set_scrollbars(int absolute_x, int relative_x,
		       int absolute_y, int relative_y,
		       bool warpPointer);
      // returns true iff any sb changes were made.
      // moves cursor if warpPointer is true

   whereNodeGraphicalPath<shgRootNode> point2path(int x, int y) const;

   void processNonSliderButtonPress(whereNodeGraphicalPath<shgRootNode> &);
   static void nonSliderButtonRelease(ClientData cd, XEvent *);
   static void nonSliderButtonAutoRepeatCallback(ClientData cd);
   static void removeBrackets(char *ptr);

   static void sliderMouseMotion(ClientData cd, XEvent *eventPtr);
   static void sliderButtonRelease(ClientData cd, XEvent *eventPtr);
 
   void rethink_entire_layout(bool isCurrShg) {
      // slow...
      assert(rootPtr);
      rootPtr->recursiveDoneAddingChildren(consts, false); // false --> don't resort
      rethink_nominal_centerx();

      if (isCurrShg) {
         resizeScrollbars();
         adjustHorizSBOffset();
         adjustVertSBOffset();
      }
   }

   bool state2hidden(shgRootNode::evaluationState, bool active,
                     bool shadow) const;

   bool recursiveUpdateHiddenNodes(where4tree<shgRootNode> *ptr);
   bool changeHiddenNodesBase(bool isCurrShg);
      // called by changeHiddenNodes.

 protected:
   void rethink_nominal_centerx();
   
   static unsigned hashFunc(const unsigned &id) {return id;}
      // needed by the hash table class
   static unsigned hashFunc2(where4tree<shgRootNode>* const &id) {
      return id->getNodeData().getId();
   }
   static unsigned hashFuncShadow(const unsigned &id) {return id;}

 public:

   // These routines are needed by shgRootNode:
   static Tk_Font getRootItemFontStruct(bool shadow) {
      if (shadow)
         return theRootItemShadowFontStruct;
      else
         return theRootItemFontStruct;
   }
   static Tk_Font getListboxItemFontStruct(bool shadow) {
      if (shadow)
         return theListboxItemShadowFontStruct;
      else
         return theListboxItemFontStruct;
   }
   static Tk_3DBorder getRootItemTk3DBorder(shgRootNode::evaluationState theEvalStyle) {
      unsigned styleIndex = theEvalStyle;
      return rootItemTk3DBordersByStyle[styleIndex];
   }
   static GC getRootItemTextGC(bool active, bool shadow) {
      if (active)
         if (shadow)
            return rootItemActiveShadowTextGC;
         else
            return rootItemActiveTextGC;
      else if (shadow)
         return rootItemInactiveShadowTextGC;
      else
         return rootItemInactiveTextGC;
   }
   static GC getListboxItemGC(bool active, bool shadow) {
      if (active)
         if (shadow)
            return listboxActiveShadowItemGC;
         else
            return listboxActiveItemGC;
      else if (shadow)
         return listboxInactiveShadowItemGC;
      else
         return listboxInactiveItemGC;
   }
   static Tk_3DBorder getListboxItemTk3DBorder(shgRootNode::evaluationState theStyle) {
      unsigned styleIndex = theStyle;
      return listboxItemTk3DBordersByStyle[styleIndex];
   }
   static GC getGCforListboxRay(shgRootNode::refinement theRefinement) {
      if (theRefinement == shgRootNode::ref_undefined) {
         assert("undefined refinement" && false);
      } else if (theRefinement == shgRootNode::ref_why)
         return whyRefinementRayGC;
      else if (theRefinement == shgRootNode::ref_where)
         return whereRefinementRayGC;

      assert("unknown refinement" && false); abort();
	  return 0;
   }
   static GC getGCforNonListboxRay(shgRootNode::refinement theRefinement) {
      if (theRefinement == shgRootNode::ref_undefined) {
         assert("undefined refinement" && false);
      } else if (theRefinement == shgRootNode::ref_why)
         return whyRefinementRayGC;
      else if (theRefinement == shgRootNode::ref_where)
         return whereRefinementRayGC;

      assert("unknown refinement" && false); abort();
	  return 0;
   }

   shg(int phaseId,
       Tcl_Interp *interp, Tk_Window theTkWindow,
       const string &iHorizSBName, const string &iVertSBName,
       const string &iCurrItemLabelName,
       bool iShowTrue, bool iShowFalse, bool iShowUnknown, bool iShowNever,
       bool iHaveActive, bool iShowInactive, bool iShowShadow);
  ~shg() {delete rootPtr;}

   int getPhaseId() const {return thePhaseId;}
   Tk_Window getTkWindow() {
      // a little hack needed for XWarpPointer
      return consts.theTkWindow;
   }

   void initializeStaticsIfNeeded();

   // the return values of the next 2 routines will be <= 0
   int getVertSBOffset()  const {return vertScrollBarOffset;}
   int getHorizSBOffset() const {return horizScrollBarOffset;}

   // These routines return true iff a change was made; they don't redraw:
   bool adjustHorizSBOffset(float newFirstFrac);
   bool adjustHorizSBOffsetFromDeltaPix(int deltapix);
   bool adjustHorizSBOffset(); // obtains first pix from actual tk sb

   bool adjustVertSBOffset(float newFirstFrac);
   bool adjustVertSBOffsetFromDeltaPix(int deltapix);
   bool adjustVertSBOffset(); // obtains first pix from actual tk sb

   int getTotalVertPixUsed()  const {return rootPtr->entire_height(consts);}
   int getTotalHorizPixUsed() const {return rootPtr->entire_width(consts);}

   int getVisibleVertPix()  const {return Tk_Height(consts.theTkWindow);}
   int getVisibleHorizPix() const {return Tk_Width(consts.theTkWindow);}

   void draw(bool doubleBuffer, bool isXsynchOn) const;
   void resize(bool rethinkScrollbars); // pass true iff we are currently displayed shg

   bool softScrollToEndOfPath(const whereNodePosRawPath &thePath);
      // Like the above routine, but always scrolls to the last item in the path.

   void recursiveRethinkLayout() {
      rootPtr->recursiveDoneAddingChildren(consts, false); // don't resort
   }
 
   void processSingleClick(int x, int y);
   void processMiddleClick(int x, int y);
   bool processDoubleClick(int x, int y);
//      // returns true iff a complete redraw is needed
//   void processShiftDoubleClick(int x, int y);
//   void processCtrlDoubleClick(int x, int y);

   enum changeType{ct_true, ct_false, ct_unknown, ct_never, ct_active, ct_inactive,
                   ct_shadow};
   bool changeHiddenNodes(bool newShowTrue, bool newShowFalse, bool newShowUnknown,
                          bool newShowNeverSeen, bool newShowActive,
                          bool newShowInactive, bool newShowShadow,
                          bool isCurrShg);
      // Returns true iff any changes.
   bool changeHiddenNodes(changeType, bool hide, bool isCurrShg);
      // Like above routine but just changes 1 trait

   // The following are very high-level routines; they tend to correspond
   // with shg-related igen calls in UI.I:
   void addNode(unsigned id, bool iActive, shgRootNode::evaluationState iEvalStyle,
		const string &label, const string &fullInfo,
		bool rootNodeFlag, bool isCurrShg);
      // unless we are adding the root node, this routine generally doesn't
      // require a redraw, because the new node won't (and shouldn't) show up
      // until a corresponding addEdge() call connects this new node to the rest
      // of the "graph".
   enum configNodeResult {noChanges, benignChanges, changesInvolvingJustExpandedness,
			  changesInvolvingHiddenness};
   configNodeResult configNode(unsigned id, bool active, shgRootNode::evaluationState,
			       bool isCurrShg, bool rethinkIfNecessary);
      // Does not redraw, but may rethink layout and/or hide-ness.
      // Note: a change from "tentatively-true" to
      // (anything else) will un-expand the node, leading to a massive layout
      // rethinkification.  Other changes are more simple -- simply changing the color
      // of a node.
   bool inactivateAll(bool isCurrShg);
      // returns true iff any changes were made.  Note that in the "usual" case,
      // the only thing that will happen is the fg color of nodes will change.
      // _However_, if some tunable like "hideInactiveNodes" is set, then massive
      // rethinkifications are a possibility.

   void addEdge(unsigned fromId, unsigned toId,
                shgRootNode::refinement, // why vs. where refinement.
                const char *label, // only used for shadow nodes; else NULL
                bool isCurrShg, 
		bool rethinkFlag); // if false, avoids rethinkification
      // The evaluationState param decides whether to explicitly expand
      // the "to" node.  Rethinks the entire layout of the shg

#ifdef PARADYN 
   void nodeInformation(unsigned nodeId, const shg_node_info &theNodeInfo);
      // In response to a middle-mouse-click...
      // the shg test program doesn't implement this stuff
#endif
};

#endif
