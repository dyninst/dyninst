// shg.h
// new search history graph user interface, along the lines
// of the new where axis user interface
// Ariel Tamches

/* $Log: shg.h,v $
/* Revision 1.13  1996/04/16 18:37:33  karavan
/* fine-tunification of UI-PC batching code, plus addification of some
/* Ari-like verbification commentification.
/*
 * Revision 1.12  1996/04/13 04:39:49  karavan
 * better implementation of batching for edge requests
 *
 * Revision 1.11  1996/04/09 19:25:13  karavan
 * added batch mode to cut down on shg redraw time.
 *
 * Revision 1.10  1996/03/10 23:20:51  hollings
 * Mad sure all assert statements were in { } blocks.  odd compiler problem
 * for AIX or UMD.
 *
 * Revision 1.9  1996/03/08  00:21:20  tamches
 * added support for hidden nodes
 *
 * Revision 1.8  1996/02/15 23:10:01  tamches
 * added proper support for why vs. where axis refinement
 *
 * Revision 1.7  1996/02/11 18:23:57  tamches
 * removed addToStatusDisplay
 *
 * Revision 1.6  1996/02/07 19:07:46  tamches
 * rethink_entire_layout, addNode, configNode, and addEdge now
 * take in "isCurrShg" flag
 *
 * Revision 1.5  1996/02/02 18:43:33  tamches
 * Displaying extra information about a node has moved from a mousemove
 * to a middle-click
 *
 * Revision 1.4  1996/01/23 07:01:03  tamches
 * added shadow node features
 *
 * Revision 1.3  1996/01/09 01:04:01  tamches
 * added thePhaseId member variable
 *
 * Revision 1.2  1995/11/06 19:27:47  tamches
 * slider bug fixes
 * dictionary_hash --> dictionary_lite
 *
 * Revision 1.1  1995/10/17 22:07:07  tamches
 * First version of "new search history graph".
 *
 */

#ifndef _SHG_H_
#define _SHG_H_

#ifndef PARADYN
#include "DictionaryLite.h"
#else
#include "util/h/DictionaryLite.h"
#endif

#include "where4tree.h"
#include "graphicalPath.h"

#include "shgConsts.h"
#include "shgRootNode.h" // contrast with rootNode.h, the where axis root node class

class shg {
 private:
   // These are needed by shgRootNode:
   static XFontStruct *theRootItemFontStruct, *theRootItemShadowFontStruct;
   static XFontStruct *theListboxItemFontStruct, *theListboxItemShadowFontStruct;
   static vector<Tk_3DBorder> rootItemTk3DBordersByStyle;
      // array[shgRootNode::evaluationState]
   static vector<Tk_3DBorder> listboxItemTk3DBordersByStyle;
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
   dictionary_lite<unsigned, where4tree<shgRootNode> *> hash;
      // associative array: shg-node-id --> its corresponding data node
   dictionary_lite<where4tree<shgRootNode> *, where4tree<shgRootNode> *> hash2;
      // associative array: shg-node --> its parent
   dictionary_lite<unsigned, vector< where4tree<shgRootNode>* > > shadowNodeHash;
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
   bool hideTrueNodes, hideFalseNodes, hideUnknownNodes, hideNeverSeenNodes;
   bool hideActiveNodes, hideInactiveNodes;
   bool hideShadowNodes;

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

   static void sliderMouseMotion(ClientData cd, XEvent *eventPtr);
   static void sliderButtonRelease(ClientData cd, XEvent *eventPtr);
 
   void rethink_entire_layout(bool isCurrShg) {
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
   static XFontStruct *getRootItemFontStruct(bool shadow) {
      if (shadow)
         return theRootItemShadowFontStruct;
      else
         return theRootItemFontStruct;
   }
   static XFontStruct *getListboxItemFontStruct(bool shadow) {
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
      else {
         assert("unknown refinement" && false);
      }
   }
   static GC getGCforNonListboxRay(shgRootNode::refinement theRefinement) {
      if (theRefinement == shgRootNode::ref_undefined) {
         assert("undefined refinement" && false);
      } else if (theRefinement == shgRootNode::ref_why)
         return whyRefinementRayGC;
      else if (theRefinement == shgRootNode::ref_where)
         return whereRefinementRayGC;
      else {
         assert("unknown refinement" && false);
      }
   }

   shg(int phaseId,
       Tcl_Interp *interp, Tk_Window theTkWindow,
       const string &iHorizSBName, const string &iVertSBName,
       const string &iCurrItemLabelName,
       bool iHideTrue, bool iHideFalse, bool iHideUnknown, bool iHideNever,
       bool iHaveActive, bool iHideInactive, bool iHideShadow);
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
   bool changeHiddenNodes(bool newHideTrue, bool newHideFalse, bool newHideUnknown,
                          bool newHideNeverSeen, bool newHideActive,
                          bool newHideInactive, bool newHideShadow,
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
   bool configNode(unsigned id, bool active, shgRootNode::evaluationState,
                   bool isCurrShg);
      // returns true iff any changes.  Does not redraw.
      // Note: a change from "tentatively-true" to
      // (anything else) will un-expand the node, leading to a massive layout
      // rethinkification.  Other changes are more simple -- simply changing the color
      // of a node.
   void addEdge(unsigned fromId, unsigned toId,
                shgRootNode::refinement, // why vs. where refinement.
                const char *label, // only used for shadow nodes; else NULL
                bool isCurrShg, 
		bool rethinkFlag); // if false, avoids rethinkification
      // The evaluationState param decides whether to explicitly expand
      // the "to" node.  Rethinks the entire layout of the shg
};

#endif
