// shg.h
// new search history graph user interface, along the lines
// of the new where axis user interface
// Ariel Tamches

/* $Log: shg.h,v $
/* Revision 1.3  1996/01/09 01:04:01  tamches
/* added thePhaseId member variable
/*
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
   // these are needed by shgRootNode
   static XFontStruct *theRootItemFontStruct;
   static XFontStruct *theListboxItemFontStruct;
   static vector<Tk_3DBorder> rootItemTk3DBordersByStyle; // array[shgRootNode::style]
   static vector<Tk_3DBorder> listboxItemTk3DBordersByStyle; // array[shgRootNode::style]
   static GC rootItemTextGC;
   static GC listboxItemGC; // for drawing text

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
 
   void rethink_entire_layout() {
      // slow...
      assert(rootPtr);
      rootPtr->recursiveDoneAddingChildren(consts, false); // false --> don't resort
      rethink_nominal_centerx();
      resizeScrollbars();
      adjustHorizSBOffset();
      adjustVertSBOffset();
   }

 protected:
   void rethink_nominal_centerx();
   
   static unsigned hashFunc(const unsigned &id) {return id;}
      // needed by the hash table class
   static unsigned hashFunc2(where4tree<shgRootNode>* const &id) {
      return id->getNodeData().getId();
   }

 public:

   // these routines are needed by shgRootNode
   static XFontStruct *getRootItemFontStruct() {return theRootItemFontStruct;}
   static XFontStruct *getListboxItemFontStruct() {return theListboxItemFontStruct;}
   static Tk_3DBorder getRootItemTk3DBorder(shgRootNode::style theStyle) {
      unsigned styleIndex = theStyle;
       return rootItemTk3DBordersByStyle[styleIndex];
   }
   static GC getRootItemTextGC() {return rootItemTextGC;}
   static GC getListboxItemGC() {return listboxItemGC;}
   static Tk_3DBorder getListboxItemTk3DBorder(shgRootNode::style theStyle) {
      unsigned styleIndex = theStyle;
      return listboxItemTk3DBordersByStyle[styleIndex];
   }

   shg(int phaseId,
       Tcl_Interp *interp, Tk_Window theTkWindow,
       const string &iHorizSBName, const string &iVertSBName,
       const string &iCurrItemLabelName);
  ~shg() {delete rootPtr;}

   int getPhaseId() const {return thePhaseId;}
   Tk_Window getTkWindow() {
      // a little hack needed for XWarpPointer
      return consts.theTkWindow;
   }

   void initializeStaticsIfNeeded();

   const string &getHorizSBName() const {return horizSBName;}
   const string &getVertSBName()  const {return vertSBName;}

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
   bool processDoubleClick(int x, int y);
//      // returns true iff a complete redraw is needed
//   void processShiftDoubleClick(int x, int y);
//   void processCtrlDoubleClick(int x, int y);

   // The following are very high-level routines; they tend to correspond
   // with shg-related igen calls in UI.I:
   void addNode(unsigned id, shgRootNode::style styleid,
		const string &label, const string &fullInfo,
		bool rootNodeFlag);
      // unless we are adding the root node, this routine generally doesn't
      // require a redraw, because the new node won't (and shouldn't) show up
      // until a corresponding addEdge() call connects this new node to the rest
      // of the "graph".
   bool configNode(unsigned id, shgRootNode::style newStyleId);
      // returns true iff any changes.  Does not redraw.
      // Note: a change from "tentatively-true" to
      // (anything else) will un-expand the node, leading to a massive layout
      // rethinkification.  Other changes are more simple -- simply changing the color
      // of a node.
   void addEdge(unsigned fromId, unsigned toId, shgRootNode::style theStyle);
      // rethinks the entire layout of the shg

   void addToStatusDisplay(const string &);

   void possibleMouseMoveIntoItem(int x, int y);
};

#endif
