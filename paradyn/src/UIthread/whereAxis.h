// whereAxis.h
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Log: whereAxis.h,v $
/* Revision 1.11  1996/04/01 22:34:14  tamches
/* added makeVisibility* member functions
/*
 * Revision 1.10  1996/02/15 23:14:53  tamches
 * added code relating to the new line-GC indirection feature of where4tree
 *
 * Revision 1.9  1996/01/11 04:43:45  tamches
 * added necessary Whole Program kludge for getSelections
 *
 * Revision 1.8  1995/12/09 04:07:58  tamches
 * added const qualifier to hashFunc
 *
 * Revision 1.7  1995/11/06 19:28:44  tamches
 * dictionary_hash --> dictionary_lite
 *
 * Revision 1.6  1995/10/17 22:18:46  tamches
 * whereAxis is no longer a templated type; it used where4tree
 * with a template of whereAxisRootNode.
 * Added back static members which used to be in whereAxisMisc.
 *
 * Revision 1.5  1995/09/20 01:27:10  tamches
 * constness removed from many prototypes; other changes to correspond
 * with whereAxis.C
 *
 * Revision 1.4  1995/08/07  00:02:52  tamches
 * Added selectUnSelectFromFullPathName
 *
 * Revision 1.3  1995/07/24  21:36:03  tamches
 * removed addChildToRoot() member function.
 * Some changes related to newly implemented where4tree sorting.
 *
 * Revision 1.2  1995/07/18  03:41:24  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:07  tamches
 * First version of the new where axis
 *
 */

#ifndef _WHERE_AXIS_H_
#define _WHERE_AXIS_H_

#include <fstream.h>
#include "where4treeConstants.h"
#include "where4tree.h"
#include "rootNode.h"
#include "graphicalPath.h"

#ifndef PARADYN
#include "DictionaryLite.h"
#else
#include "util/h/DictionaryLite.h"
#endif

// Note: whereAxis is no longer a templated type.
// It utilized where4tree<> with a template of rootNode, which
// itself constains the resourceHandle that we used to template.

class whereAxis {
 private:
   // these static members are needed by whereAxisRootNode (rootNode.h)
   static XFontStruct *theRootItemFontStruct;
   static XFontStruct *theListboxItemFontStruct;
   static Tk_3DBorder rootItemTk3DBorder;
   static GC rootItemTextGC;
   static Tk_3DBorder listboxItem3DBorder;
   static GC listboxItemGC;
   static GC listboxRayGC;
   static GC nonListboxRayGC;

   // this appears to be the WRONG class for the following vrbles:
   static int listboxBorderPix; // 3
   static int listboxScrollBarWidth; // 16

   // These ugly variables keep track of a button press (and possible hold-down)
   // in scrollbar-up/down or pageup/pagedown region.  They are not used for
   // press (and possible hold-down) on the scrollbar slider
   bool nonSliderButtonCurrentlyPressed; // init to false
   whereNodeGraphicalPath<whereAxisRootNode>::pathEndsIn nonSliderButtonPressRegion;
   Tk_TimerToken buttonAutoRepeatToken;
   where4tree<whereAxisRootNode> *nonSliderCurrentSubtree;
   int nonSliderSubtreeCenter;
   int nonSliderSubtreeTop;

   // Analagous to above; used only for scrollbar slider
   int slider_scrollbar_left, slider_scrollbar_top, slider_scrollbar_bottom;
   int slider_initial_yclick, slider_initial_scrollbar_slider_top;
   where4tree<whereAxisRootNode> *slider_currently_dragging_subtree;

   void initializeStaticsIfNeeded();

   where4TreeConstants consts;
      // Each where axis has its own set of constants, so different axis may,
      // for example, have different color configurations.
   where4tree<whereAxisRootNode> *rootPtr;

   dictionary_lite<resourceHandle, where4tree<whereAxisRootNode> *> hash;
      // associative array: resource unique id --> its corresponding node

   string horizSBName; // e.g. ".nontop.main.bottsb"
   string vertSBName;  // e.g. ".nontop.main.leftsb"
   string navigateMenuName;

   whereNodePosRawPath lastClickPath;
      // used in the navigate menu
   where4tree<whereAxisRootNode> *beginSearchFromPtr;
      // if NULL, then begin from the top.  Otherwise,
      // find() treats ignores found items until this one is reached.

   Tcl_Interp *interp;

   bool obscured;
      // true if the underlying window is partially or fully obscured.
      // Currently, used to know when to properly redraw after scrolling
      // a listbox, since I'm having trouble getting tk to recognize
      // GraphicsExpose events...sigh

   int nominal_centerx; // actual centerx = nominal_centerx + horizScrollBarOffset
   int horizScrollBarOffset; // always <= 0

   // why isn't there a nominal_topy?  Because it's always at the top of the window
   // (0 or 3)
   int vertScrollBarOffset; // always <= 0

   void resizeScrollbars();

   bool set_scrollbars(int absolute_x, int relative_x,
		       int absolute_y, int relative_y,
		       bool warpPointer);
      // returns true iff any sb changes were made
      // Moves the cursor if warpPointer is true.

   whereNodeGraphicalPath<whereAxisRootNode> point2path(int x, int y) const;

   static void nonSliderButtonRelease(ClientData cd, XEvent *);
   static void nonSliderButtonAutoRepeatCallback(ClientData cd);

   void processNonSliderButtonPress(whereNodeGraphicalPath<whereAxisRootNode> &thePath);

   static void sliderMouseMotion(ClientData cd, XEvent *eventPtr);
   static void sliderButtonRelease(ClientData cd, XEvent *eventPtr);

 protected:
   void rethink_nominal_centerx();

   static unsigned hashFunc(const resourceHandle &uniqueId) {return uniqueId;}
      // needed for hash table class...

#ifndef PARADYN
   // only the where axis test program reads from a file
   int readTree(ifstream &,
		resourceHandle parentUniqueId,
		resourceHandle nextAvailChildId);
      // returns # of nodes read in.  Recursive.
#endif
		 
 public:
   whereAxis(Tcl_Interp *in_interp, Tk_Window theTkWindow,
	     const string &root_str,
	     const string &iHorizSBName, const string &iVertSBName,
	     const string &iNavigateMenuName);

#ifndef PARADYN
   // only the where axis test program reads from a file
   whereAxis(ifstream &infile, Tcl_Interp *interp, Tk_Window theTkWindow,
	     const char *iHorizSBName, const char *iVertSBName,
	     const char *iNavigateMenuName);
#endif

  ~whereAxis() {delete rootPtr;}

   // the return values of the next 2 routines will be <= 0
   int getVertSBOffset() const {return vertScrollBarOffset;}
   int getHorizSBOffset() const {return horizScrollBarOffset;}

   int getTotalVertPixUsed() const {return rootPtr->entire_height(consts);}
   int getTotalHorizPixUsed() const {return rootPtr->entire_width(consts);}

   int getVisibleVertPix() const {return Tk_Height(consts.theTkWindow);}
   int getVisibleHorizPix() const {return Tk_Width(consts.theTkWindow);}

   void addItem(const string &name,
		resourceHandle parentUniqueId,
		resourceHandle newNodeUniqueId,
		bool rethinkGraphicsNow,
		bool resortNow);

   void recursiveDoneAddingChildren(bool resortNow) {
      rootPtr->recursiveDoneAddingChildren(consts, resortNow);
   }

   static XFontStruct &getRootItemFontStruct() {
      assert(theRootItemFontStruct); // a static member vrble
      return *theRootItemFontStruct;
   }
   static XFontStruct &getListboxItemFontStruct() {
      assert(theListboxItemFontStruct); // a static member vrble
      return *theListboxItemFontStruct;
   }
   static Tk_3DBorder getRootItemTk3DBorder() {
      return rootItemTk3DBorder;
   }
   static GC getRootItemTextGC() {
      return rootItemTextGC;
   }
   static Tk_3DBorder getListboxItem3DBorder() {
      return listboxItem3DBorder;
   }
   static GC getListboxItemGC() {
      return listboxItemGC;
   }
   static GC getGCforListboxRay() {
      return listboxRayGC;
   }
   static GC getGCforNonListboxRay() {
      return nonListboxRayGC;
   }

   void draw(bool doubleBuffer, bool isXsynchOn) const;

   void resize(bool rethinkScrollbars);
      // should be true only if we are the currently displayed abstraction

   void makeVisibilityUnobscured() {consts.makeVisibilityUnobscured();}
   void makeVisibilityPartiallyObscured() {consts.makeVisibilityPartiallyObscured();}
   void makeVisibilityFullyObscured() {consts.makeVisibilityFullyObscured();}

   void processSingleClick(int x, int y);
   bool processDoubleClick(int x, int y);
      // returns true iff a redraw of everything is still needed
   bool processShiftDoubleClick(int x, int y);
   bool processCtrlDoubleClick (int x, int y);

   int find(const string &str);
      // uses and updates "beginSearchFromPtr"
      // returns 0 if not found; 1 if found & no expansion needed;
      // 2 if found & some expansion is needed

   bool softScrollToPathItem(const whereNodePosRawPath &thePath, unsigned index);
      // scrolls s.t. the (centerx, topy) of the path item in question is placed in the
      // middle of the screen.  Returns true iff the scrollbar settings changed.

   bool softScrollToEndOfPath(const whereNodePosRawPath &thePath);
      // Like the above routine, but always scrolls to the last item in the path.

   bool forciblyScrollToEndOfPath(const whereNodePosRawPath &thePath);
      // Like the above routine, but explicitly expands any un-expanded children
      // along the path.
   bool forciblyScrollToPathItem(const whereNodePosRawPath &thePath, unsigned pathLen);

   // Noe of these scrollbar adjustment routines redraw anything
   bool adjustHorizSBOffset(float newFirstFrac);
   bool adjustHorizSBOffsetFromDeltaPix(int deltapix);
      // needed for alt-mousemove
//   bool adjustHorizSBOffsetFromDeltaPages(int deltapages);
   bool adjustHorizSBOffset(); // Obtains FirstPix from actual tk scrollbar

   bool adjustVertSBOffset (float newFirstFrac);
   bool adjustVertSBOffsetFromDeltaPix(int deltapix);
      // needed for alt-mousemove
//   bool adjustVertSBOffsetFromDeltaPages(int deltapages);
   bool adjustVertSBOffset(); // Obtains FirstPix from actual tk scrollbar

   void rethinkNavigateMenu();
   void navigateTo(unsigned pathLen);
      // forcibly scrolls to item #pathLen of "lastClickPath"

   bool selectUnSelectFromFullPathName(const string &name, bool select);
      // returns true iff the item was found
      // pass true for the 2nd param iff you want to select it; false
      // if you want to unselect it.
   
   vector< vector<resourceHandle> > getSelections(bool &wholeProgram, vector<unsigned> &wholeProgramFocus) const;
   void clearSelections();
};

#endif
