// whereAxis.C
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Log: whereAxis.C,v $
/* Revision 1.6  1995/09/20 01:27:55  tamches
/* Better set_scrollbars() (mouse moves).  Use of graphicalPath, a new class.
/* Other major cleanifications to go along with where4tree changes.
/*
 * Revision 1.5  1995/08/07  00:02:53  tamches
 * Added selectUnSelectFromFullPathName
 *
 * Revision 1.4  1995/08/01  23:03:54  tamches
 * Fixed a layout bug whereby scrolling a listbox whose width was less
 * than that of the parent (pink) node would redraw the listbox incorrectly.
 *
 * Revision 1.3  1995/07/24  21:36:04  tamches
 * removed addChildToRoot() member function.
 * Some changes related to newly implemented where4tree sorting.
 *
 * Revision 1.2  1995/07/18  03:41:26  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:08  tamches
 * First version of the new where axis
 *
 */

#include <stdlib.h> // exit()

#include "minmax.h"

#ifndef PARADYN
#include "DMinclude.h"
#else
#include "paradyn/src/DMthread/DMinclude.h"
#endif

#include "graphicalPath.h"
#include "whereAxis.h"
#include "whereAxisMisc.h"

template <class USERNODEDATA>
whereAxis<USERNODEDATA>::whereAxis(Tcl_Interp *in_interp, Tk_Window theTkWindow,
				   const string &root_str,
				   const string &iHorizSBName,
				   const string &iVertSBName,
				   const string &iNavigateMenuName) :
				   consts(in_interp, theTkWindow),
				   hash(hashFunc, 32),
				   horizSBName(iHorizSBName),
				   vertSBName(iVertSBName),
				   navigateMenuName(iNavigateMenuName) {
   USERNODEDATA rootUND = 0;

   rootPtr = new where4tree<USERNODEDATA>(rootUND, root_str, consts);
   assert(rootPtr);

   hash[rootUND] = rootPtr;

   beginSearchFromPtr = NULL;

   interp = in_interp;
      
   horizScrollBarOffset = vertScrollBarOffset = 0;
      
   rethink_nominal_centerx();
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::rethink_nominal_centerx() {
   // using Tk_Width(theTkWindow) as the available screen width, and
   // using root->myEntireWidthAsDrawn(consts) as the amount of screen real
   // estate used, this routine rethinks this->nominal_centerx.

   const int horizSpaceUsedByTree = rootPtr->entire_width(consts);

   // If the entire tree fits, then set center-x to window-width / 2.
   // Otherwise, set center-x to used-width / 2;
   if (horizSpaceUsedByTree <= Tk_Width(consts.theTkWindow))
      nominal_centerx = Tk_Width(consts.theTkWindow) / 2;
   else
      nominal_centerx = horizSpaceUsedByTree / 2;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::resizeScrollbars() {
   string commandStr = string("resize1Scrollbar ") + horizSBName + " " +
                       string(rootPtr->entire_width(consts)) + " " +
		       string(Tk_Width(consts.theTkWindow));
   myTclEval(interp, commandStr);

   commandStr = string("resize1Scrollbar ") + vertSBName + " " +
                       string(rootPtr->entire_height(consts)) + " " +
		       string(Tk_Height(consts.theTkWindow));
   myTclEval(interp, commandStr);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
set_scrollbars(int absolute_x, int relative_x,
	       int absolute_y, int relative_y,
	       bool warpPointer) {
   // Sets the scrollbars s.t. (absolute_x, absolute_y) will appear
   // at window (relative) location (relative_x, relative_y).
   // May need to take into account the current scrollbar setting...

   bool anyChanges = true;

   horizScrollBarOffset = -set_scrollbar(interp, horizSBName,
					 rootPtr->entire_width(consts),
					 absolute_x, relative_x);
      // relative_x will be updated
   
   vertScrollBarOffset = -set_scrollbar(interp, vertSBName,
					rootPtr->entire_height(consts),
					absolute_y, relative_y);

   if (warpPointer) {
//      cout << "warping pointer to " << relative_x << "," << relative_y << endl;

      XWarpPointer(Tk_Display(consts.theTkWindow),
		   Tk_WindowId(consts.theTkWindow), // src win
		   Tk_WindowId(consts.theTkWindow), // dest win
		   0, 0, // src x,y
		   0, 0, // src height, width
		   relative_x, relative_y
		   );
   }

   return anyChanges;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::addItem(const string &newName,
				      USERNODEDATA parentUniqueId,
				      USERNODEDATA newNodeUniqueId,
				      bool rethinkGraphicsNow,
				      bool resortNow) {
   where4tree<USERNODEDATA> *newNode = new where4tree<USERNODEDATA>
                                       (newNodeUniqueId, newName, consts);
   assert(newNode);

   assert(hash.defines(parentUniqueId));
   where4tree<USERNODEDATA> *parentPtr = hash[parentUniqueId];
   assert(parentPtr != NULL);

   parentPtr->addChild(newNode, false, // not explicitly expanded
		       consts,
		       rethinkGraphicsNow,
		       resortNow);

   assert(!hash.defines(newNodeUniqueId));
   hash[newNodeUniqueId] = newNode;
   assert(hash.defines(newNodeUniqueId));
}

#ifndef PARADYN
// only the where axis test program uses this stuff:
template <class USERNODEDATA>
int whereAxis<USERNODEDATA>::readTree(ifstream &is,
				      USERNODEDATA parentUniqueId,
				      USERNODEDATA nextUniqueIdToUse) {
   // returns the number of new nodes added, 0 if nothing could be read
   char c;
   is >> c;
   if (!is) return false;
   if (c == ')') {
      is.putback(c);
      return 0;
   }

   const bool oneItemTree = (c != '(');
   if (oneItemTree)
      is.putback(c);

   string rootString = readItem(is);
   if (rootString.length() == 0)
      return 0; // could not read root name

   const bool thisIsWhereAxisRoot = (nextUniqueIdToUse==parentUniqueId);
   if (thisIsWhereAxisRoot) {
      // a little hack for the where axis root node, which can't be added
      // using addItem().  Not to mention we have to set "whereAxis::rootPtr"

      where4tree<USERNODEDATA> *newParentNode = new where4tree<USERNODEDATA>
	                                        (nextUniqueIdToUse, rootString, consts);
      assert(newParentNode);

      assert(nextUniqueIdToUse==0);
      assert(!hash.defines(nextUniqueIdToUse));
      hash[nextUniqueIdToUse] = newParentNode;
      assert(hash.defines(nextUniqueIdToUse));

      rootPtr = newParentNode;
   }
   else
      this->addItem(rootString, parentUniqueId, nextUniqueIdToUse, false, false);
         // don't redraw; don't resort

   if (oneItemTree)
      return 1;

   parentUniqueId = nextUniqueIdToUse++;

   // now the children (if any)
   int result = 1; // number of nodes read in so far...
   while (true) {
      int localResult = readTree(is, parentUniqueId, nextUniqueIdToUse);
      if (localResult == 0)
         break;

      result += localResult;
      nextUniqueIdToUse += localResult;
   }

   // now, do some graphical rethinking w.r.t. "result" that we had suppressed
   // up until now in the name of improved performance...
   assert(hash.defines(parentUniqueId));
   where4tree<USERNODEDATA> *theParentNode = hash[parentUniqueId];

   theParentNode->doneAddingChildren(consts); // also resorts its children...

   // eat the closing )
   is >> c;
   if (c != ')') {
      cerr << "expected ) to close tree, but found " << c << endl;
      return 0;
   }

   return result;
}

// only the where axis test program gets this routine
template <class USERNODEDATA>
whereAxis<USERNODEDATA>::whereAxis(ifstream &infile, Tcl_Interp *in_interp,
                                   Tk_Window theWindow,
                                   const char *iHorizSBName, const char *iVertSBName,
				   const char *iNavigateMenuName) :
                              consts(in_interp, theWindow),
			      hash(hashFunc, 32),
                              horizSBName(iHorizSBName),
                              vertSBName(iVertSBName),
                              navigateMenuName(iNavigateMenuName) {
   interp = in_interp;
   horizScrollBarOffset = vertScrollBarOffset = 0;

   USERNODEDATA rootNodeUniqueId = 0;
   cout << "readtree" << endl;
   const int result = readTree(infile,
			       rootNodeUniqueId,
			       rootNodeUniqueId
			       );
   cout << "readtree done" << endl;
   assert(result > 0);

   beginSearchFromPtr = NULL;  

   rethink_nominal_centerx();
}
#endif

const int overallWindowBorderPix = 0;

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::draw(bool doubleBuffer,
				   bool xsynchronize // are we debugging?
				   ) const {
   Drawable theDrawable = (doubleBuffer && !xsynchronize) ? consts.offscreenPixmap :
                                                            consts.masterWindow;

   if (doubleBuffer || xsynchronize)
      // clear the offscreen pixmap before drawing onto it
      XFillRectangle(consts.display,
                     theDrawable,
                     consts.erasingGC,
                     0, // x-offset relative to drawable
                     0, // y-offset relative to drawable
                     Tk_Width(consts.theTkWindow),
                     Tk_Height(consts.theTkWindow)
                     );

   rootPtr->draw(consts, theDrawable,
		 nominal_centerx + horizScrollBarOffset,
		    // relative (not absolute) coord
		 overallWindowBorderPix + vertScrollBarOffset,
		    // relative (not absolute) coord
		 false, // not root only
		 false // not listbox only
		 );

   if (doubleBuffer && !xsynchronize)
      // copy from offscreen pixmap onto the 'real' window
      XCopyArea(consts.display,
                theDrawable, // source pixmap
                consts.masterWindow, // dest pixmap
                consts.erasingGC, // ?????
                0, 0, // source x,y pix
                Tk_Width(consts.theTkWindow),
                Tk_Height(consts.theTkWindow),
                0, 0 // dest x,y offset pix
                );
}

/* *********************************************************** */

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::resize(int newWindowWidth,
                                     int newWindowHeight) {
   assert(newWindowWidth == Tk_Width(consts.theTkWindow));
   assert(newWindowHeight == Tk_Height(consts.theTkWindow));

   resize(true);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::resize(bool currentlyDisplayedAbstraction) {
   const int newWindowWidth = Tk_Width(consts.theTkWindow);
//   const int newWindowHeight = Tk_Height(consts.theTkWindow); [not used]

   consts.resize(); // reallocates offscreen pixmap; rethinks
                    // 'listboxHeightWhereSBappears'

   // Loop through EVERY listbox (gasp) and rethink whether or not it needs
   // a scrollbar and (possibly) change listBoxActualHeight...
   // _Must_ go _after_ the consts.resize()
   (void)rootPtr->rethinkListboxAfterResize(consts);

   // Now the more conventional resize code:
   rootPtr->rethinkAfterResize(consts, newWindowWidth);
   rethink_nominal_centerx(); // must go _after_ the rootPtr->rethinkAfterResize()

   if (currentlyDisplayedAbstraction) {
      // We have changed axis width and/or height.  Let's inform tcl, so it may
      // rethink the scrollbar ranges.
      resizeScrollbars();

      // Now, let's update our own stored horiz & vert scrollbar offset values
      adjustHorizSBOffset(); // obtain FirstPix from the actual tk scrollbar
      adjustVertSBOffset (); // obtain FirstPix from the actual tk scrollbar
   }
}

/* *********************************************************** */

extern bool nonSliderButtonCurrentlyPressed;
extern whereNodeGraphicalPath<unsigned>::pathEndsIn nonSliderButtonPressRegion;
extern Tk_TimerToken buttonAutoRepeatToken;

extern where4tree<resourceHandle> *nonSliderCurrentSubtree;
extern int nonSliderSubtreeCenter;
extern int nonSliderSubtreeTop;

extern void nonSliderCallMeOnButtonRelease(ClientData cd, XEvent *theEvent);
extern void nonSliderButtonAutoRepeatCallback(ClientData cd);

/* *********************************************************** */

extern int slider_scrollbar_left;
extern int slider_scrollbar_top;
extern int slider_scrollbar_bottom;
extern int slider_initial_yclick;
extern int slider_initial_scrollbar_slider_top;
extern where4tree<resourceHandle> *slider_currently_dragging_subtree;

extern void sliderCallMeOnMouseMotion(ClientData cd, XEvent *eventPtr);
extern void sliderCallMeOnButtonRelease(ClientData cd, XEvent *eventPtr);

/* *********************************************************** */

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::processSingleClick(int x, int y,
                                                 bool redrawNow) {
   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   whereNodeGraphicalPath<USERNODEDATA> thePath(x, y, consts, rootPtr,
						rootNodeCenterX, rootNodeTopY);
   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<USERNODEDATA>::Nothing:
//         cout << "single-click in nothing at (" << x << "," << y << ")" << endl;
         return;
      case whereNodeGraphicalPath<USERNODEDATA>::ExpandedNode: {
//         cout << "click on an non-listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();
         rethinkNavigateMenu();

         // Now redraw the node in question...(its highlightedness changed)
         where4tree<USERNODEDATA> *ptr = thePath.getLastPathNode(rootPtr);
         ptr->toggle_highlight();
         ptr->getRootNode().draw(consts, consts.masterWindow,
				 thePath.get_endpath_centerx(),
				 thePath.get_endpath_topy());
         return;
      }
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxItem:
//         cout << "click on a listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();
         rethinkNavigateMenu();

         // Now we have to redraw the item in question...(its highlightedness changed)
         thePath.getLastPathNode(rootPtr)->toggle_highlight();
         if (redrawNow)
 	    thePath.getParentOfLastPathNode(rootPtr)->draw(consts, consts.masterWindow,
							   thePath.get_endpath_centerx(),
							   thePath.get_endpath_topy(),
							   false, // not root only
							   true // listbox only
							   );
         break;
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPagedown:
         nonSliderButtonCurrentlyPressed = true;
         nonSliderButtonPressRegion = thePath.whatDoesPathEndIn();
         nonSliderCurrentSubtree = thePath.getLastPathNode(rootPtr);
         nonSliderSubtreeCenter = thePath.get_endpath_centerx();
         nonSliderSubtreeTop = thePath.get_endpath_topy();

         Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       nonSliderCallMeOnButtonRelease,
			       &consts);

         nonSliderButtonAutoRepeatCallback(&consts);
         return;
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarSlider: {
//         cout << "looks like a click in a listbox scrollbar slider" << endl;

         where4tree<USERNODEDATA> *parentPtr = thePath.getLastPathNode(rootPtr);

         slider_initial_yclick = y;
         slider_currently_dragging_subtree = parentPtr;

         const int lbTop = thePath.get_endpath_topy() + parentPtr->getRootNode().getHeight() +
			   consts.vertPixParent2ChildTop;

         int dummyint;
         parentPtr->getScrollbar().getSliderCoords(lbTop,
		     lbTop + parentPtr->getListboxActualPixHeight() - 1,
		     parentPtr->getListboxActualPixHeight() - 2*listboxBorderPix,
		     parentPtr->getListboxFullPixHeight() - 2*listboxBorderPix,
		     slider_initial_scrollbar_slider_top, // filled in
		     dummyint);

         slider_scrollbar_left = thePath.get_endpath_centerx() -
	                         parentPtr->horiz_pix_everything_below_root(consts) / 2;
         slider_scrollbar_top = thePath.get_endpath_topy() +
                                parentPtr->getRootNode().getHeight() +
				consts.vertPixParent2ChildTop;
         slider_scrollbar_bottom = slider_scrollbar_top +
                                   parentPtr->getListboxActualPixHeight() - 1;

//         cout << "slider click was on subtree whose root name is "
//              << parentPtr->getRootName() << endl;

         Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       sliderCallMeOnButtonRelease,
			       &consts);
	 Tk_CreateEventHandler(consts.theTkWindow,
			       PointerMotionMask,
			       sliderCallMeOnMouseMotion,
			       &consts);
         break;
      }
      default:
         assert(false);
   }
}

/* ***************************************************************** */

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::processShiftDoubleClick(const int x, const int y) {
   // returns true iff a complete redraw is called for

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   whereNodeGraphicalPath<USERNODEDATA> thePath(x, y, consts, rootPtr,
						rootNodeCenterX, rootNodeTopY);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<USERNODEDATA>::Nothing:
//         cout << "shift-double-click in nothing" << endl;
         return false;
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxItem:
//         cout << "shift-double-click in lb item; ignoring" << endl;
         return false;
      case whereNodeGraphicalPath<USERNODEDATA>::ExpandedNode:
         break; // some breathing room for lots of code to follow...
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPagedown:
         // in this case, do the same as a single-click
         nonSliderButtonCurrentlyPressed = true;
	 nonSliderButtonPressRegion = thePath.whatDoesPathEndIn();
	 nonSliderCurrentSubtree = thePath.getLastPathNode(rootPtr);
	 nonSliderSubtreeCenter = thePath.get_endpath_centerx();
	 nonSliderSubtreeTop = thePath.get_endpath_topy();
	 Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       nonSliderCallMeOnButtonRelease,
			       &consts);
         nonSliderButtonAutoRepeatCallback(&consts);
         return false; // no need to redraw further
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarSlider:
//         cout << "shift-double-click in a listbox scrollbar slider...doing nothing" << endl;
         return false;
      default:
         assert(false);
   }

   // How do we know whether to expand-all or un-expand all?
   // Well, if there is no listbox up, then we should _definitely_ unexpand.
   // And, if there is a listbox up containing all items (no explicitly expanded items),
   //    then we should _definitely_ expand.
   // But, if there is a listbox up with _some_ explicitly expanded items, then
   //    are we supposed to expand out the remaining ones; or un-expand the expanded
   //    ones?  It could go either way.  For now, we'll say that we should un-expand.

   bool anyChanges = false; // so far...
   where4tree<USERNODEDATA> *ptr = thePath.getLastPathNode(rootPtr);

   if (ptr->getListboxPixWidth() > 0) {
      bool noExplicitlyExpandedChildren = true; // so far...
      for (unsigned childlcv=0; childlcv < ptr->getNumChildren(); childlcv++)
         if (ptr->getChildIsExpandedFlag(childlcv)) {
            noExplicitlyExpandedChildren = false; 
            break;
         }

      if (noExplicitlyExpandedChildren)
         // There is a listbox up, and it contains ALL children.
         // So, obviously, we want to expand them all...
         anyChanges = rootPtr->path2ExpandAllChildren(consts, thePath.getPath(), 0);
      else
         // There is a listbox up, but there are also some expanded children.
         // This call (whether to expand the remaining listbox items, or whether
         // to un-expand the expanded items) could go either way; we choose the
         // latter (for now, at least)
         anyChanges = rootPtr->path2UnExpandAllChildren(consts, thePath.getPath(), 0);
   }
   else
      // No listbox is up; hence, all children are expanded.
      // So obviously, we wish to un-expand all the children.
      anyChanges = rootPtr->path2UnExpandAllChildren(consts, thePath.getPath(), 0);

   if (!anyChanges)
      return false;

   rethink_nominal_centerx();

   // We have changed axis width and/or height.  Let's inform tcl, so it may
   // rethink the scrollbar ranges.
   resizeScrollbars();

   // Now, let's update our own stored horiz & vert scrollbar offset values
   adjustHorizSBOffset(); // obtain FirstPix from the actual tk scrollbar
   adjustVertSBOffset (); // obtain FirstPix from the actual tk scrollbar

   return true;
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::processCtrlDoubleClick(int x, int y) {
   // returns true iff changes were made

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   whereNodeGraphicalPath<USERNODEDATA> thePath(x, y, consts, rootPtr,
						rootNodeCenterX, rootNodeTopY);
   if (thePath.whatDoesPathEndIn() != whereNodeGraphicalPath<USERNODEDATA>::ExpandedNode)
      return false;

//   cout << "ctrl-double-click:" << endl;

   // How do we know whether to select-all or unselect-all?
   // Well, if noone is selected, then we should _definitely_ select all.
   // And, if everyone is selected, then we should _definitely_ unselect all.
   // But, if _some_ items are selected, what should we do?  It could go either way.
   // For now, we'll say unselect.

   where4tree<USERNODEDATA> *ptr = thePath.getLastPathNode(rootPtr);
   if (ptr->getNumChildren()==0)
      return false;

   bool allChildrenSelected = true;
   bool noChildrenSelected = true;
   for (unsigned childlcv=0; childlcv < ptr->getNumChildren(); childlcv++)
      if (ptr->getChildTree(childlcv)->isHighlighted()) {
         noChildrenSelected = false;
         if (!allChildrenSelected)
            break;
      }
      else {
         allChildrenSelected = false;
         if (!noChildrenSelected)
            break;
      }

   assert(!(allChildrenSelected && noChildrenSelected));

   if (allChildrenSelected || !noChildrenSelected) {
      // unselect all children
      for (unsigned childlcv=0; childlcv < ptr->getNumChildren(); childlcv++)
         ptr->getChildTree(childlcv)->unhighlight();
      return true; // changes were made
   }
   else {
      assert(noChildrenSelected);
      for (unsigned childlcv=0; childlcv < ptr->getNumChildren(); childlcv++)
         ptr->getChildTree(childlcv)->highlight();
      return true; // changes were made
   }
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::processDoubleClick(int x, int y) {
   // returns true iff a complete redraw is called for

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord      
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord      

   bool scrollToWhenDone = false; // for now...

   whereNodeGraphicalPath<USERNODEDATA> thePath(x, y, consts, rootPtr,
						rootNodeCenterX, rootNodeTopY);
   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<USERNODEDATA>::Nothing:
//         cout << "looks like a double-click in nothing" << endl;
         return false;
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarPagedown:
         // in this case, do the same as a single-click
         nonSliderButtonCurrentlyPressed = true;
	 nonSliderButtonPressRegion = thePath.whatDoesPathEndIn();
	 nonSliderCurrentSubtree = thePath.getLastPathNode(rootPtr);
	 nonSliderSubtreeCenter = thePath.get_endpath_centerx();
	 nonSliderSubtreeTop = thePath.get_endpath_topy();
	 Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       nonSliderCallMeOnButtonRelease,
			       &consts);
         nonSliderButtonAutoRepeatCallback(&consts);
         return false; // no need to redraw further
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxScrollbarSlider:
//         cout << "double-click in a listbox scrollbar slider...doing nothing" << endl;
         return false;
      case whereNodeGraphicalPath<USERNODEDATA>::ExpandedNode:
         // double-click in a "regular" node (not in listbox): un-expand
//         cout << "double-click on a non-listbox item" << endl;

         if (thePath.getSize() == 0) {
//            cout << "double-click un-expansion on the root..ignoring" << endl;
            return false;
         }

         if (!rootPtr->path2lbItemUnexpand(consts, thePath.getPath(), 0)) {
            // probably an attempt to expand a leaf item (w/o a triangle next to it)
//            cout << "could not un-expand this subtree (a leaf?)...continuing" << endl;
            return false;
         }
         break;
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxItem: {
         // double-click in a listbox item
//         cout << "double-click on a listbox item" << endl;
         int result = rootPtr->path2lbItemExpand(consts, thePath.getPath(), 0);
         if (result == 1) {
//            cout << "could not expand listbox leaf item" << endl;
            // Just change highlightedness:
            thePath.getLastPathNode(rootPtr)->toggle_highlight(); // doesn't redraw

            thePath.getParentOfLastPathNode(rootPtr)->
	         draw(consts, consts.masterWindow,
		      thePath.get_endpath_centerx(),
		      thePath.get_endpath_topy(),
		      false, // not root only
		      true // listbox only
		      );
            return false;
         }
         else {
            assert(result==2);

            // expansion was successful...later, we'll scroll to the expanded item.
            // NOTE: rootPtr->path2lbItemExpand will have modified "thePath"
            //       for us (by changing the last item from a listbox item to
            //       an expanded one, which is the proper thing to do).

            scrollToWhenDone = true;
         }
         break;
      }
      default:
         assert(false);
   }

   // expansion or un-expansion successful

   rethink_nominal_centerx();

   // We have changed axis width and/or height.  Let's inform tcl, so it may
   // rethink the scrollbar ranges.
   resizeScrollbars();

   // Now, let's update our own stored horiz & vert scrollbar offset values
   adjustHorizSBOffset(); // obtain FirstPix from the actual tk scrollbar
   adjustVertSBOffset (); // obtain FirstPix from the actual tk scrollbar

   if (scrollToWhenDone) {
      whereNodeGraphicalPath<USERNODEDATA>
	   path_to_scroll_to(thePath.getPath(),
			consts, rootPtr,
			nominal_centerx,
                           // root centerx (abs. pos., regardless of sb [intentional])
			overallWindowBorderPix
                           // topy of root (abs. pos., regardless of sb [intentional])
			);

      int newlyExpandedElemCenterX = path_to_scroll_to.get_endpath_centerx();
      int newlyExpandedElemTopY    = path_to_scroll_to.get_endpath_topy();
      int newlyExpandedElemMiddleY = newlyExpandedElemTopY +
	                             path_to_scroll_to.getLastPathNode(rootPtr)->getRootNode().getHeight() / 2;

      (void)set_scrollbars(newlyExpandedElemCenterX, x,
			   newlyExpandedElemMiddleY, y,
			   true);
   }

   return true;
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::softScrollToPathItem(const whereNodePosRawPath &thePath,
						   unsigned index) {
   // scrolls s.t. the (centerx, topy) of path index is in middle of screen.

   whereNodePosRawPath newPath = thePath;
     // make a copy, because we're gonna hack it up a bit.
   newPath.rigSize(index);
   return softScrollToEndOfPath(newPath);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
forciblyScrollToPathItem(const whereNodePosRawPath &thePath, unsigned pathLen) {
   // Simply a stub which generates a new path and calls forciblyScrollToEndOfPath()   
   // "index" indicates the length of the new path; in particular, 0 will give
   // and empty path (the root node).

   // make a copy of "thePath", truncate it to "pathLen", and forciblyScrollToEndOfPath
   whereNodePosRawPath newPath = thePath;
   newPath.rigSize(pathLen);
   return forciblyScrollToEndOfPath(newPath);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
softScrollToEndOfPath(const whereNodePosRawPath &thePath) {
   whereNodeGraphicalPath<USERNODEDATA>
         scrollToPath(thePath, consts, rootPtr,
		      nominal_centerx,
		         // ignores scrollbar settings (an absolute coord),
		      overallWindowBorderPix
		         // absolute coord of root topy
		      );
   const int last_item_centerx = scrollToPath.get_endpath_centerx();
   const int last_item_topy    = scrollToPath.get_endpath_topy();
      // note: if the path ends in an lb item, these coords refer to the PARENT
      //       node, which is expanded.

   switch (scrollToPath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<USERNODEDATA>::ExpandedNode: {
//         cout << "soft scrolling to expanded node" << endl;
         const int last_item_middley =
            last_item_topy +
            scrollToPath.getLastPathNode(rootPtr)->getRootNode().getHeight() / 2;
         return set_scrollbars(last_item_centerx,
			       Tk_Width(consts.theTkWindow) / 2,
			       last_item_middley,
			       Tk_Height(consts.theTkWindow) / 2,
			       true);
      }
      case whereNodeGraphicalPath<USERNODEDATA>::ListboxItem: {
//         cout << "soft scrolling to lb item" << endl;

         // First, let's scroll within the listbox (no redrawing yet)
         where4tree<USERNODEDATA> *parent = scrollToPath.getParentOfLastPathNode(rootPtr);

         const unsigned itemHeight = consts.listboxVertPadAboveItem +
	                             consts.listboxFontStruct->ascent +
   	                             consts.listboxVertPadAfterItemBaseline;
   
         int scrollToVertPix = 0; // relative to listbox top
         const unsigned childnum = scrollToPath.getPath().getLastItem();
         for (unsigned childlcv=0; childlcv < childnum; childlcv++)
            if (!parent->getChildIsExpandedFlag(childlcv))
               scrollToVertPix += itemHeight;

         int destItemRelToListboxTop = scrollToVertPix;   
         if (parent->getScrollbar().isValid()) {
            (void)parent->scroll_listbox(scrollToVertPix -
					 parent->getScrollbar().getPixFirst());

            destItemRelToListboxTop -= parent->getScrollbar().getPixFirst();
         }

         if (destItemRelToListboxTop < 0)
            cout << "note: softScrollToEndOfPath() failed to scroll properly to item w/in listbox" << endl;

         return set_scrollbars(scrollToPath.get_endpath_centerx() -
			          parent->horiz_pix_everything_below_root(consts)/2 +
  			          parent->getListboxPixWidth() / 2,
   			          // listbox centerx
			       Tk_Width(consts.theTkWindow) / 2,
			       scrollToPath.get_endpath_topy() +
			          parent->getRootNode().getHeight() +
			          consts.vertPixParent2ChildTop +
			          destItemRelToListboxTop + itemHeight / 2,
			          // should be the middley of the lb item
			       Tk_Height(consts.theTkWindow) / 2,
			       true);
      }
      default:
         assert(false);
   }

   assert(false);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
forciblyScrollToEndOfPath(const whereNodePosRawPath &thePath) {
   // Forcibly expands path items as needed, then calls softScrollToEndOfPath()

   const bool anyChanges = rootPtr->expandEntirePath(consts, thePath, 0);
   if (anyChanges) {
      rethink_nominal_centerx();
      resizeScrollbars();
      adjustHorizSBOffset();
      adjustVertSBOffset();
   }

   softScrollToEndOfPath(thePath);

   return anyChanges;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::navigateTo(unsigned pathLen) {
   (void)forciblyScrollToPathItem(lastClickPath, pathLen);
}

template <class USERNODEDATA>
int whereAxis<USERNODEDATA>::find(const string &str) {
   // does a blind search for the given string.  Expands things along the
   // way if needed.  Returns 0 if not found, 1 if found & no expansions
   // were needed, 2 if found & expansion(s) _were_ needed

   // Our strategy: (1) make a path out of the string.
   //               (2) call a routine that "forcefully" scrolls to the end of that
   //                   path.  We say "forcefully" beceause this routine will
   //                   expand items along the way, if necessary.

   // Uses and alters "beginSearchFromPtr"

   whereNodePosRawPath thePath(rootPtr->getNumChildren());
   int result = rootPtr->string2Path(thePath, consts, str, beginSearchFromPtr, true);

   if (result == 0)
      if (beginSearchFromPtr == NULL)
         return 0; // not found
      else {
         // try again with beginSearchFromPtr of NULL (wrap-around search)
//         cout << "search wrap-around" << endl;
         beginSearchFromPtr = NULL;
         return find(str);
      }

   // found.  Update beginSearchFromPtr.
   where4tree<USERNODEDATA> *beginSearchFromPtr = rootPtr;
   for (unsigned i=0; i < thePath.getSize(); i++)
      beginSearchFromPtr = beginSearchFromPtr->getChildTree(thePath[i]);

   if (result==1)
      (void)softScrollToEndOfPath(thePath);
   else {
      assert(result==2);
      assert(forciblyScrollToEndOfPath(thePath));
         // rethinks nominal centerx, resizes scrollbars, etc.
   }

   return result;
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustHorizSBOffset(float newFirst) {
   // does not redraw.  Returns true iff any changes.

   // First, we need to make the change to the tk scrollbar
   newFirst = moveScrollBar(interp, horizSBName, newFirst);

   // Then, we update our C++ variables
   int widthOfEverything = rootPtr->entire_width(consts);
   int oldHorizScrollBarOffset = horizScrollBarOffset;
   horizScrollBarOffset = -(int)(newFirst * widthOfEverything);
      // yes, horizScrollBarOffset is always negative (unless it's zero)
   return (horizScrollBarOffset != oldHorizScrollBarOffset);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustHorizSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw.  Returns true iff any changes.

   const int widthOfEverything = rootPtr->entire_width(consts);
   float newFirst = (float)(-horizScrollBarOffset + deltapix) / widthOfEverything;
   return adjustHorizSBOffset(newFirst);
}

//template <class USERNODEDATA>
//bool whereAxis<USERNODEDATA>::adjustHorizSBOffsetFromDeltaPages(int deltapages) {
//   // does not redraw.  Returns true iff any changes.
//
//   // First, update the tk scrollbar
//   const int widthOfEverything = rootPtr->entire_width(consts);
//   const int widthOfVisible = Tk_Width(consts.theTkWindow);
//   float newFirst = (float)(-horizScrollBarOffset + widthOfVisible*deltapages) /
//                     widthOfEverything;
//   return adjustHorizSBOffset(newFirst);
//}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustHorizSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, horizSBName, first, last);

   return adjustHorizSBOffset(first);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustVertSBOffset(float newFirst) {
   // does not redraw.  Returns true iff any changes.
   // First, we need to make the change to the tk scrollbar
   newFirst = moveScrollBar(interp, vertSBName, newFirst);

   // Then, we update our C++ variables
   int heightOfEverything = rootPtr->entire_height(consts);
   int oldVertScrollBarOffset = vertScrollBarOffset;
   vertScrollBarOffset = -(int)(newFirst * heightOfEverything);
      // yes, vertScrollBarOffset is always negative (unless it's zero)
   return (vertScrollBarOffset != oldVertScrollBarOffset);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustVertSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw.  Returns true iff any changes were made.

   const int heightOfEverything = rootPtr->entire_height(consts);
   float newFirst = (float)(-vertScrollBarOffset + deltapix) / heightOfEverything;
   return adjustVertSBOffset(newFirst);
}

//template <class USERNODEDATA>
//bool whereAxis<USERNODEDATA>::adjustVertSBOffsetFromDeltaPages(int deltapages) {
//   // does not redraw
//
//   // First, update the tk scrollbar
//   const int heightOfEverything = rootPtr->entire_height(consts);
//   const int heightOfVisible = Tk_Height(consts.theTkWindow);
//   float newFirst = (float)(-vertScrollBarOffset + heightOfVisible*deltapages) /
//                    heightOfEverything;
//   return adjustVertSBOffset(newFirst);
//}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::adjustVertSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, vertSBName, first, last);

   return adjustVertSBOffset(first);
}

/* ************************************************************************ */

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::rethinkNavigateMenu() {
   // We loop through the items of "lastClickPath".  For each item,
   // we get the name of the root node, and append the full-path entry
   // to the navigate menu.

   where4tree<USERNODEDATA> *currTree = rootPtr;   

   // Note: in tk4.0, menu indices start at 1, not 0 (0 is reserved for tearoff)
   string commandStr = navigateMenuName + " delete 1 100";
   myTclEval(interp, commandStr);

   string theString;

   unsigned itemlcv = 0;
   while (true) {
      if (itemlcv >= 1)
         theString += "/";
      theString += currTree->getRootName();

//      cout << "adding " << theString << " to the navigate menu" << endl;

      string commandStr = navigateMenuName + " add command -label \"" + theString + "\" -command \"navigateTo " + string(itemlcv) + "\"";
      myTclEval(interp, commandStr);

      if (itemlcv >= lastClickPath.getSize())
         break;

      unsigned theItem = lastClickPath[itemlcv++];
      currTree = currTree->getChildTree(theItem);
   }
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
selectUnSelectFromFullPathName(const string &name, bool selectFlag) const {
   // returns true iff found
   const char *str = name.string_of();
   if (str == NULL)
      return false;

   return rootPtr->selectUnSelectFromFullPathName(str, selectFlag);
}

template <class USERNODEDATA>
vector< vector<USERNODEDATA> > whereAxis<USERNODEDATA>::getSelections() const {
   // returns a vector[num-hierarchies] of vector of selections.
   // The number of hierarchies is defined as the number of children of the
   // root node.
   const unsigned numHierarchies = rootPtr->getNumChildren();

   vector < vector<USERNODEDATA> > result(numHierarchies);

   for (unsigned i=0; i < numHierarchies; i++) {
      where4tree<USERNODEDATA> *hierarchyRoot = rootPtr->getChildTree(i);

      result[i] = hierarchyRoot->getSelections();
      if (result[i].size()==0) {
         // this hierarchy had no selections; therefore, choose the hierarchy's
         // root item...
         vector<USERNODEDATA> defaultHierarchy(1);
         defaultHierarchy[0] = hierarchyRoot->getUserNodeData();
         result[i] = defaultHierarchy;
      }
      else if (rootPtr->isHighlighted()) {
         // The root node was highlighted --> add this hierarchy's root item,
         // if not already done.
         USERNODEDATA hierarchyRootId = hierarchyRoot->getUserNodeData();

         bool hierarchyRootAlreadyAdded = false;
         for (unsigned j=0; j < result[i].size(); j++) {
            if (result[i][j] == hierarchyRootId) {
               hierarchyRootAlreadyAdded = true;
               break;
            }
	 }
         if (!hierarchyRootAlreadyAdded) {
//            cout << "adding hierarchy root for hierarchy #" << i << " because the root node was selected" << endl;
            result[i] += hierarchyRootId;
         }
      }
   }

   return result;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::clearSelections() {
   rootPtr->recursiveClearSelections();
}
