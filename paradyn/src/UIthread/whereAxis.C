// whereAxis.C
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Log: whereAxis.C,v $
/* Revision 1.2  1995/07/18 03:41:26  tamches
/* Added ctrl-double-click feature for selecting/unselecting an entire
/* subtree (nonrecursive).  Added a "clear all selections" option.
/* Selecting the root node now selects the entire program.
/*
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

#include "whereAxis.h"
#include "whereAxisMisc.h"

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::rethink_nominal_centerx() {
   // using Tk_Width(theTkWindow) as the available screen width, and
   // using root->myEntireWidthAsDrawn(consts) as the amount of screen real
   // estate used, this routine rethinks this->nominal_centerx.

   // NOTE: It seems that we must rethink_nominal_centerx() every
   // time we call the tcl script informTclOfOverallSizeChange
   // Coincidence?  Probably not.  But why?
   const int horizSpaceUsedByTree = rootPtr->myEntireWidthAsDrawn(consts);

   // If the entire tree fits, then set center-x to window-width / 2.
   // Otherwise, set center-x to used-width / 2;
   if (horizSpaceUsedByTree <= Tk_Width(consts.theTkWindow))
      nominal_centerx = Tk_Width(consts.theTkWindow) / 2;
   else
      nominal_centerx = horizSpaceUsedByTree / 2;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::resizeScrollbars() {
   char buffer[100];
   (void)sprintf(buffer, "resize1Scrollbar %s %d %d",
                 horizSBName.string_of(),
                 rootPtr->myEntireWidthAsDrawn(consts), // total
                 Tk_Width(consts.theTkWindow) // visible
                 );
   myTclEval(interp, buffer);

   (void)sprintf(buffer, "resize1Scrollbar %s %d %d",
                 vertSBName.string_of(),
                 rootPtr->myEntireHeightAsDrawn(consts), // total
                 Tk_Height(consts.theTkWindow) // visible
                 );
   myTclEval(interp, buffer);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
set_scrollbars(const int absolute_x, const int relative_x,
	       const int absolute_y, const int relative_y) {
   // Sets the scrollbars s.t. (absolute_x, absolute_y) will appear
   // at window (relative) location (relative_x, relative_y).
   // May need to take into account the current scrollbar setting...

   bool anyChanges = true;

   // First, the horizontal scrollbar:
   const int newXpixFirst = absolute_x - relative_x;
   const int widthOfEverything = rootPtr->myEntireWidthAsDrawn(consts);
   float newXpixRelativeFirst = (1.0 * newXpixFirst / widthOfEverything);
   newXpixRelativeFirst = moveScrollBar(interp, horizSBName, newXpixRelativeFirst);
   horizScrollBarOffset = -(int)(newXpixRelativeFirst * widthOfEverything);

   // And now the vertical scrollbar:
   const int newYpixFirst = absolute_y - relative_y;
   const int heightOfEverything = rootPtr->myEntireHeightAsDrawn(consts);
   float newYpixRelativeFirst = (1.0 * newYpixFirst / heightOfEverything);
   newYpixRelativeFirst = moveScrollBar(interp, vertSBName, newYpixRelativeFirst);
   vertScrollBarOffset = -(int)(newYpixRelativeFirst * heightOfEverything);

   return anyChanges;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::addItem(const string &newName,
				      USERNODEDATA parentUniqueId,
				      USERNODEDATA newNodeUniqueId,
				      bool rethinkGraphicsNow) {
   where4tree<USERNODEDATA> *newNode = new where4tree<USERNODEDATA>
                                       (newNodeUniqueId, newName, consts);
   assert(newNode);

   assert(hash.defines(parentUniqueId));
   where4tree<USERNODEDATA> *parentPtr = hash[parentUniqueId];
   assert(parentPtr != NULL);

   parentPtr->addChild(newNode, false, // not explicitly expanded
		       consts,
		       rethinkGraphicsNow);

   assert(!hash.defines(newNodeUniqueId));
   hash[newNodeUniqueId] = newNode;
   assert(hash.defines(newNodeUniqueId));
}

#ifndef PARADYN
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
      this->addItem(rootString, parentUniqueId, nextUniqueIdToUse, false);

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

   theParentNode->doneAddingChildren(consts);

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
   const int result = readTree(infile,
			       rootNodeUniqueId,
			       rootNodeUniqueId
			       );
   assert(result > 0);

   beginSearchFromPtr = NULL;  

   rethink_nominal_centerx();
}
#endif

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::addChildToRoot(where4tree<USERNODEDATA> *theChild,
                                             const bool explicitlyExpanded) {
   rootPtr->addChild(theChild, explicitlyExpanded, consts);

   rethink_nominal_centerx();

   // We have changed axis width and/or height.  Let's inform tcl, so it may
   // rethink the scrollbar ranges.
   resizeScrollbars();

   // Now, let's update our own stored horiz & vert scrollbar offset values
   adjustHorizSBOffset(); // obtain FirstPix from the actual tk scrollbar
   adjustVertSBOffset (); // obtain FirstPix from the actual tk scrollbar
}

const int overallWindowBorderPix = 0;

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::draw(const bool doubleBuffer,
				   const bool xsynchronize // are we debugging?
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

   rootPtr->draw(consts,
             theDrawable,
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
void whereAxis<USERNODEDATA>::resize(const int newWindowWidth,
                                     const int newWindowHeight) {
   if (newWindowWidth != Tk_Width(consts.theTkWindow)) {
      cout << "whereAxis::resize() warning -- newWindowWidth of " << newWindowWidth << " does not match window width of " << Tk_Width(consts.theTkWindow) << endl;
   }

   if (newWindowHeight != Tk_Height(consts.theTkWindow)) {
      cout << "whereAxis::resize() warning -- newWindowHeight of " << newWindowHeight << " does not match window height of " << Tk_Height(consts.theTkWindow) << endl;
   }

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
   // Must go _after_ the consts.resize()
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
extern int nonSliderButtonPressRegion;
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
void whereAxis<USERNODEDATA>::processSingleClick(const int x, const int y,
                                                 const bool redrawNow) {
   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScollBarOffset <= 0

   whereNodeGraphicalPath theGraphicalPath(whereNodeGraphicalPathItem(rootPtr->theChildren.size()));
   const int result = rootPtr->point2GraphicalPath(theGraphicalPath, consts, x, y,
						   rootNodeCenterX,
						   rootNodeTopY);
   if (result == -1) {
      //cout << "single-click in nothing at (" << x << "," << y << ")" << endl;
   }
   else if (result == 1) {
      // cout << "click on an non-listbox item; adjusting NAVIGATE menu..." << endl;
      lastClickPath = graphical2RawPath(theGraphicalPath);
      rethinkNavigateMenu();

      // Now we have to redraw the node in question...(its highlightedness changed)
      rootPtr->toggleHighlightFromPath(consts, theGraphicalPath,
                                       true // redrawNow
                                       );
   }
   else if (result == 2) {
      // cout << "click on a listbox item; adjusting NAVIGATE menu..." << endl;
      lastClickPath = graphical2RawPath(theGraphicalPath);
      rootPtr->noNegativeChildNums(lastClickPath, 0);
      rethinkNavigateMenu();

      // Now we have to redraw the node in question...(its highlightedness changed)
      rootPtr->toggleHighlightFromPath(consts, theGraphicalPath,
                                       true // redrawNow
                                       );
   }
   else if (result >= 3 && result <= 6) {
      // 3 --> up-arrow; 4 --> down-arrow
      // 5 --> pageup;   6 --> pagedown

      nonSliderButtonCurrentlyPressed = true;
      nonSliderButtonPressRegion = result;

      nonSliderCurrentSubtree = rootPtr;
      for (int i=0; i < theGraphicalPath.getSize()-1; i++) {
         const int childnum = theGraphicalPath[i].childnum;
         nonSliderCurrentSubtree = nonSliderCurrentSubtree->theChildren[childnum].theTree;
      }
   
      nonSliderSubtreeCenter=theGraphicalPath.getConstLastItem().root_centerxpix;
      nonSliderSubtreeTop=theGraphicalPath.getConstLastItem().root_topypix;

      Tk_CreateEventHandler(consts.theTkWindow,
                            ButtonReleaseMask,
                            nonSliderCallMeOnButtonRelease,
                            &consts);

      nonSliderButtonAutoRepeatCallback(&consts);
   }
   else if (result == 7) {
      // cout << "looks like a click in a listbox scrollbar slider" << endl;
      // cout << "installing mouse-move and button-release handlers" << endl;

      slider_initial_yclick = y;
      slider_initial_scrollbar_slider_top = rootPtr->path2lbScrollBarSliderTopPix
                                            (consts, theGraphicalPath, 0);
      slider_currently_dragging_subtree=rootPtr;
      for (int i=0; i < theGraphicalPath.getSize()-1; i++) {
         const int childnum = theGraphicalPath[i].childnum;
         slider_currently_dragging_subtree = slider_currently_dragging_subtree->
                                             theChildren[childnum].theTree;
      }

      slider_scrollbar_left = theGraphicalPath.getConstLastItem().root_centerxpix -
                              slider_currently_dragging_subtree->
                                myEntireWidthAsDrawn(consts)/2;
      slider_scrollbar_top = theGraphicalPath.getConstLastItem().root_topypix +
                             slider_currently_dragging_subtree->
                               theRootNode.getHeight() +
                             consts.vertPixParent2ChildTop;
      slider_scrollbar_bottom = slider_scrollbar_top +
                                slider_currently_dragging_subtree->
                                  listboxActualPixHeight - 1;

      //cout << "slider click was on subtree whose root name is "
      //     << slider_currently_dragging_subtree->getRootName() << endl;

      Tk_CreateEventHandler(consts.theTkWindow,
                            ButtonReleaseMask,
                            sliderCallMeOnButtonRelease,
                            &consts);
      Tk_CreateEventHandler(consts.theTkWindow,
                            PointerMotionMask,
                            sliderCallMeOnMouseMotion,
                            &consts);
   }
}

/* ***************************************************************** */

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::processShiftDoubleClick(const int x, const int y) {
   // returns true iff a complete redraw is called for

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScollBarOffset <= 0

   whereNodeGraphicalPath theGraphicalPath(whereNodeGraphicalPathItem(rootPtr->theChildren.size()));
   const int result = rootPtr->point2GraphicalPath(theGraphicalPath, consts, x, y,
						   rootNodeCenterX, rootNodeTopY);

   if (result == -1) {
      //cout << "shift-double-click in nothing" << endl;
      return false;
   }
   else if (result >= 3 && result <= 6) {
      // 3 --> up-arrow; 4 --> down-arrow
      // 5 --> pageup;   6 --> pagedown

      rootPtr->path2lbScrollBarClick(consts, theGraphicalPath, 0,
                                     result, // indicates page-up
                                     true // redrawNow
                                     );
      return false; // no need to redraw further
   }
   else if (result == 7) {
      //cout << "shift-double-click in a listbox scrollbar slider...doing nothing" << endl;
      return false;
   }

   if (result != 1)
      return false;

   //cout << "shift-double-click:" << endl;

   assert (result == 1); // a "regular" node (not a listbox item) was clicked on

   // How do we know whether to expand-all or un-expand all?
   // Well, if there is no listbox up, then we should _definitely_ unexpand.
   // And, if there is a listbox up containing all items (no explicitly expanded items),
   //    then we should _definitely_ expand.
   // But, if there is a listbox up with _some_ explicitly expanded items, then
   //    are we supposed to expand out the remaining ones; or un-expand the expanded
   //    ones?  For now, we'll say that we should un-expand.

   assert(theGraphicalPath.getSize() > 0);
   
   where4tree<USERNODEDATA> *clickee = rootPtr;
   for (int lcv=0; lcv < theGraphicalPath.getSize()-1; lcv++) {
      const whereNodeGraphicalPathItem &item = theGraphicalPath[lcv];
      assert(item.childnum >= 0 && item.childnum < clickee->theChildren.size());

      clickee = clickee->theChildren[item.childnum].theTree;
   }
   assert(theGraphicalPath.getConstLastItem().childnum == clickee->theChildren.size());
   
   // Now examine "clickee" to determine whether or not a listbox is up
   bool anyChanges = false;

   whereNodeRawPath theRawPath = graphical2RawPath(theGraphicalPath);

   if (clickee->listboxPixWidth > 0) {
      bool noExplicitlyExpandedChildren = true; // so far...
      for (int childlcv=0; childlcv < clickee->theChildren.size(); childlcv++)
         if (clickee->theChildren[childlcv].isExplicitlyExpanded) {
            noExplicitlyExpandedChildren = false; 
            break;
         }

      if (noExplicitlyExpandedChildren)
         // There is a listbox up, and it contains ALL children.
         // So, obviously, we want to expand them all...
         anyChanges = rootPtr->path2ExpandAllChildren(consts, theRawPath, 0);
      else
         // There is a listbox up, but there are also some expanded children.
         // This call (whether to expand the remaining listbox items, or whether
         // to un-expand the expanded items) could go either way; we choose the
         // latter (for now, at least)
         anyChanges = rootPtr->path2UnExpandAllChildren(consts, theRawPath, 0);
   }
   else
      // No listbox is up; hence, all children are expanded.
      // So obviously, we wish to un-expand all the children.
      anyChanges = rootPtr->path2UnExpandAllChildren(consts, theRawPath, 0);

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
bool whereAxis<USERNODEDATA>::processCtrlDoubleClick(const int x, const int y) {
   // returns true iff changes were made

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScollBarOffset <= 0
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScollBarOffset <= 0

   whereNodeGraphicalPath theGraphicalPath(whereNodeGraphicalPathItem(rootPtr->theChildren.size()));
   const int result = rootPtr->point2GraphicalPath(theGraphicalPath, consts, x, y,
						   rootNodeCenterX, rootNodeTopY);

      // 1 --> node; 2--> listbox item
      // 3 --> up-arrow; 4 --> down-arrow
      // 5 --> pageup;   6 --> pagedown

   if (result != 1)
      return false;

   //cout << "ctrl-double-click:" << endl;

   // How do we know whether to select-all or unselect-all?
   // Well, if noone is selected, then we should _definitely_ select all.
   // And, if everyone is selected, then we should _definitely_ unselect all.
   // But, if _some_ items are selected, what should we do?  For now, we'll
   //    say unselect.

   assert(theGraphicalPath.getSize() > 0);
   
   where4tree<USERNODEDATA> *clickee = rootPtr;
   for (int lcv=0; lcv < theGraphicalPath.getSize()-1; lcv++) {
      const whereNodeGraphicalPathItem &item = theGraphicalPath[lcv];
      assert(item.childnum >= 0 && item.childnum < clickee->theChildren.size());

      clickee = clickee->theChildren[item.childnum].theTree;
   }
   assert(theGraphicalPath.getConstLastItem().childnum == clickee->theChildren.size());
   
   if (clickee->theChildren.size()==0)
      return false;

   bool allChildrenSelected = true;
   bool noChildrenSelected = true;
   for (int childlcv=0; childlcv < clickee->theChildren.size(); childlcv++)
      if (clickee->theChildren[childlcv].theTree->isHighlighted()) {
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
      for (int childlcv=0; childlcv < clickee->theChildren.size(); childlcv++)
         clickee->theChildren[childlcv].theTree->unhighlight();
      return true; // changes were made
   }
   else {
      assert(noChildrenSelected);
      for (int childlcv=0; childlcv < clickee->theChildren.size(); childlcv++)
         clickee->theChildren[childlcv].theTree->highlight();
      return true; // changes were made
   }
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::processDoubleClick(const int x, const int y,
                                                 const bool redrawNow) {
   // returns true iff a complete redraw is called for

   const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord      
   const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord      

   whereNodeGraphicalPath theGraphicalPath(whereNodeGraphicalPathItem(rootPtr->theChildren.size()));
   const int result = rootPtr->point2GraphicalPath(theGraphicalPath, consts, x, y,
						   rootNodeCenterX, rootNodeTopY);

   if (result == -1) {
      // cout << "looks like a double-click in nothing" << endl;
      return false;
   }
   else if (result >= 3 && result <= 6) {
      // 3 --> up-arrow; 4 --> down-arrow
      // 5 --> pageup;   6 --> pagedown

      rootPtr->path2lbScrollBarClick(consts, theGraphicalPath, 0,
                                     result, // indicates page-up
                                     true // redrawNow
                                     );
      return false; // no need to redraw further
   }
   else if (result == 7) {
      // cout << "double-click in a listbox scrollbar slider...doing nothing" << endl;
      return false;
   }

   bool scrolltoWhenDone=false;

   whereNodeRawPath theRawPath = graphical2RawPath(theGraphicalPath);

   if (result == 1) {
      // double-click in a "regular" node (not in listbox): un-expand
      // cout << "double-click on a non-listbox item" << endl;

      assert(theGraphicalPath.getSize() > 0);
      if (theGraphicalPath.getSize() == 1 && theGraphicalPath.getConstLastItem().childnum > 0) {
         assert(theGraphicalPath.getConstLastItem().childnum == rootPtr->theChildren.size());
         // cout << "double-click un-expansion on the root..ignoring" << endl;
         return false;
      }

      if (!rootPtr->path2lbItemUnexpand(consts, theRawPath, 0)) {
         // probably an attempt to expand a leaf item (w/o a triangle next to it)
         // cout << "could not un-expand this subtree (a leaf?)...continuing" << endl;
         return false;
      }
   }      
   else if (result == 2) {
      // double-click in a listbox item
      // cout << "double-click on a listbox item" << endl;
      int result = rootPtr->path2lbItemExpand(consts, theRawPath, 0);
      if (result == 0) {
         // cout << "could not expand already-expanded item" << endl;
         return false;
      }
      else if (result == 1) {
         // cout << "could not expand listbox leaf item" << endl;
         // Just change highlightedness:
         
         rootPtr->toggleHighlightFromPath(consts, theGraphicalPath, true);
         return false;
      }
      else {
         assert(result==2);

         // expansion was successful...later, we'll scroll to the expanded item.
         // NOTE: rootPtr->path2lbItemExpand will have modified "thePath"
         //       for us (by changing the last item from a listbox item to
         //       an expanded one, which is the proper thing to do).

         scrolltoWhenDone = true;
      }
   }
   else
      assert(false);

   // expansion or un-expansion successful

   rethink_nominal_centerx();

   // We have changed axis width and/or height.  Let's inform tcl, so it may
   // rethink the scrollbar ranges.
   resizeScrollbars();

   // Now, let's update our own stored horiz & vert scrollbar offset values
   adjustHorizSBOffset(); // obtain FirstPix from the actual tk scrollbar
   adjustVertSBOffset (); // obtain FirstPix from the actual tk scrollbar

   if (scrolltoWhenDone) {
      int absolute_scrollto_x, absolute_scrollto_y;
      rootPtr->path2pixOffset(consts, theRawPath, 0,
                              nominal_centerx,
                                 // absolute centerx-coord of root,
                                 // irregardless of curr scrollbar value
                              overallWindowBorderPix,
                                 // absolute topy-coord of root,
                                 // irregardless of curr scrollbar value
                              absolute_scrollto_x, // filled in
                              absolute_scrollto_y // filled in
                              );

      // cout << "You clicked at relative=(" << x << "," << y << "); absolute-scrollto=(" << absolute_scrollto_x << "," << absolute_scrollto_y << ")" << endl;

      (void)set_scrollbars(absolute_scrollto_x, x,
                           absolute_scrollto_y, y);
         // set_scrollbars() returns true iff any changes were made to the scrollbars
         // but we _always_ want to redraw, regardless of whether or not
         // the expansion caused a scrollbar bounds change
   }

   return true;
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::softScrollToPathItem(const whereNodeRawPath &thePath,
						   const int index) {
   // scrolls s.t. the (centerx, topy) of the path item in
   // question is placed in the middle of the screen.

   // Simply a stub which generates a new path and calls scrollToEndOfPath()   

   whereNodeRawPath newPath = thePath; // make a copy, because we're gonna
                                       // hack it up a bit.

   // If thePath[index] is a non-listbox item, then, in order to truncate
   // the path right there, we need to change its childNum field to
   // numChildren.  If thePath[index] is a listbox item, however, then
   // make no changes.

   where4tree<USERNODEDATA> *theTree = rootPtr;
   for (int item=0; item < index; item++) {
      int childnum = thePath[item].childnum;
      theTree = theTree->theChildren[childnum].theTree;
   }
   whereNodeRawPathItem &newItem = newPath[index];
   if (newItem.childnum < 0) {
      // a listbox item...should only happen at end of path
      assert(index == thePath.getSize()-1);
   }
   else {
      newItem.childnum = theTree->theChildren.size();
      newPath.rigSize(index+1);
   }

   // Okay, now we have "newPath" to work with.  Don't use "thePath" anymore.
   return softScrollToEndOfPath(newPath);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::forciblyScrollToPathItem(const whereNodeRawPath &thePath,
						       const int index) {
   // Simply a stub which generates a new path and calls forciblyScrollToEndOfPath()   

   whereNodeRawPath newPath = thePath; // make a copy, because we're gonna
                                       // hack it up a bit.

   // If thePath[index] is a non-listbox item, then, in order to truncate
   // the path right there, we need to change its childNum field to
   // numChildren.  If thePath[index] is a listbox item, however, then
   // make no changes.

   where4tree<USERNODEDATA> *theTree = rootPtr;
   for (int item=0; item < index; item++) {
      int childnum = thePath[item].childnum;
      theTree = theTree->theChildren[childnum].theTree;
   }
   whereNodeRawPathItem &newItem = newPath[index];
   if (newItem.childnum < 0) {
      // a listbox item...should only happen at end of path
      assert(index == thePath.getSize()-1);
   }
   else {
      newItem.childnum = theTree->theChildren.size();
      newPath.rigSize(index+1);
   }

   // Okay, now we have "newPath" to work with.  Don't use "thePath" anymore.
   return forciblyScrollToEndOfPath(newPath);
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::softScrollToEndOfPath(const whereNodeRawPath &thePath) {
   int absolute_scrollto_x, absolute_scrollto_y;

   // WARNING: the following will bomb if ANY path element corresponds to an
   //          unexpanded node!
   rootPtr->path2pixOffset(consts, thePath, 0,
                           nominal_centerx, // absolute coord of root's center-x
                           overallWindowBorderPix, // absol coord of root's top-y
                           absolute_scrollto_x, // filled in
                           absolute_scrollto_y // filled in
                           );
   bool anySBChanges = set_scrollbars(absolute_scrollto_x,
                                      Tk_Width(consts.theTkWindow) / 2,
                                      absolute_scrollto_y,
                                      Tk_Height(consts.theTkWindow) / 2
                                      );
   return anySBChanges;
}

template <class USERNODEDATA>
bool whereAxis<USERNODEDATA>::
forciblyScrollToEndOfPath(const whereNodeRawPath &theRawPath) {
   // Given a path with no negative childnum's [i.e., one in which we manually
   // determine which children are in listboxes]...

   // Strategy:
   // 1) go through the path, explicitly expanding items as necessary
   // 2) rethink nominal centerx, and anything else necessary due to the
   //    explicit expansions
   // 3) call softScrollToEndOfPath

   const bool anyChanges = rootPtr->expandEntirePath(consts, theRawPath, 0);
   if (anyChanges) {
      rethink_nominal_centerx();
      resizeScrollbars();
      adjustHorizSBOffset();
      adjustVertSBOffset();
   }

   softScrollToEndOfPath(theRawPath);

   return anyChanges;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::navigateTo(const int index) {
   (void)forciblyScrollToPathItem(lastClickPath, index);
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

   whereNodeRawPath thePath(whereNodeRawPathItem(rootPtr->theChildren.size()));
   int result = rootPtr->string2Path(thePath, consts, str, beginSearchFromPtr, true);
      // NOTE: the path returned by this routine will not contain ANY listbox
      //       childnums, except perhaps at the end.  We must check for this
      //       manually in forciblyScrollToEndOfPath().

   if (result == 0)
      if (beginSearchFromPtr == NULL)
         return 0; // not found
      else {
         // try again with beginSearchFromPtr of NULL (wrap-around search)
         beginSearchFromPtr = NULL;
         return find(str);
      }

   // found.  Update beginSearchFromPtr.
   beginSearchFromPtr = rootPtr;
   for (int i=0; i < thePath.getSize(); i++) {
      if (i == thePath.getSize()-1) {
         int childnum = thePath[i].childnum;
         if (childnum < 0) {
            childnum = -childnum - 1;
            beginSearchFromPtr = beginSearchFromPtr->theChildren[childnum].theTree;
         }
         else
            assert(childnum == beginSearchFromPtr->theChildren.size());
      }
      else {
         int childnum = thePath[i].childnum;
         beginSearchFromPtr = beginSearchFromPtr->theChildren[childnum].theTree;
      }
   }

   if (result==1)
      (void)softScrollToEndOfPath(thePath);
   else {
      assert(result==2);
      assert(forciblyScrollToEndOfPath(thePath));
         // rethinks nominal centerx, resizes scrollbars, etc.
   }

   const bool pathEndsInLBitem = thePath.getConstLastItem().childnum < 0;
   if (pathEndsInLBitem) {
      whereNodeGraphicalPath theGraphicalPath;
      const int rootNodeCenterX = nominal_centerx + horizScrollBarOffset;
         // relative (not absolute) coord.  note: horizScollBarOffset <= 0
      const int rootNodeTopY = overallWindowBorderPix + vertScrollBarOffset;
         // relative (not absolute) coord.  note: vertScollBarOffset <= 0

      rootPtr->makeGraphicalPathFromRawPath(consts,
					    0, theGraphicalPath,
					    thePath,
					    rootNodeCenterX,
					    rootNodeTopY);
					       
      (void)rootPtr->path2lbItemScrollSoVisible(consts, theGraphicalPath, 0);
   }

   return result;
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustHorizSBOffset(const float newFirst) {
   // does not redraw

   // First, we need to make the change to the tk scrollbar
   newFirst = moveScrollBar(interp, horizSBName, newFirst);

   // Then, we update our C++ variables
   int widthOfEverything = rootPtr->myEntireWidthAsDrawn(consts);
   horizScrollBarOffset = -(int)(newFirst * widthOfEverything);
      // yes, horizScrollBarOffset is always negative (unless it's zero)
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustHorizSBOffsetFromDeltaPix(const int deltapix) {
   // does not redraw

   // Make the change to the tk scrollbar
   const int widthOfEverything = rootPtr->myEntireWidthAsDrawn(consts);
   float newFirst = (float)(-horizScrollBarOffset + deltapix) / widthOfEverything;
   newFirst = moveScrollBar(interp, horizSBName, newFirst);

   // Next, update C++ internals:
   horizScrollBarOffset = -(int)(newFirst * widthOfEverything);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustHorizSBOffsetFromDeltaPages(const int deltapages) {
   // does not redraw

   // First, update the tk scrollbar
   const int widthOfEverything = rootPtr->myEntireWidthAsDrawn(consts);
   const int widthOfVisible = Tk_Width(consts.theTkWindow);
   float newFirst = (float)(-horizScrollBarOffset + widthOfVisible*deltapages) /
                     widthOfEverything;
   newFirst = moveScrollBar(interp, horizSBName, newFirst);

   // Then, update C++ internals
   horizScrollBarOffset = -(int)(newFirst * widthOfEverything);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustHorizSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, horizSBName, first, last);

   adjustHorizSBOffset(first);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustVertSBOffset(const float newFirst) {
   // does not redraw

   // First, we need to make the change to the tk scrollbar
   newFirst = moveScrollBar(interp, vertSBName, newFirst);

   // Then, we update our C++ variables
   int heightOfEverything = rootPtr->myEntireHeightAsDrawn(consts);
   vertScrollBarOffset = -(int)(newFirst * heightOfEverything);
      // yes, vertScrollBarOffset is always negative (unless it's zero)
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustVertSBOffsetFromDeltaPix(const int deltapix) {
   // does not redraw

   // Make the change to the tk scrollbar
   const int heightOfEverything = rootPtr->myEntireHeightAsDrawn(consts);
   float newFirst = (float)(-vertScrollBarOffset + deltapix) / heightOfEverything;
   newFirst = moveScrollBar(interp, vertSBName, newFirst);

   // Next, update C++ internals:
   vertScrollBarOffset = -(int)(newFirst * heightOfEverything);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustVertSBOffsetFromDeltaPages(const int deltapages) {
   // does not redraw

   // First, update the tk scrollbar
   const int heightOfEverything = rootPtr->myEntireHeightAsDrawn(consts);
   const int heightOfVisible = Tk_Height(consts.theTkWindow);
   float newFirst = (float)(-vertScrollBarOffset + heightOfVisible*deltapages) /
                    heightOfEverything;
   newFirst = moveScrollBar(interp, vertSBName, newFirst);

   // Then, update C++ internals
   vertScrollBarOffset = -(int)(newFirst * heightOfEverything);
}

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::adjustVertSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, vertSBName, first, last);

   adjustVertSBOffset(first);
}

/* ************************************************************************ */

template <class USERNODEDATA>
void whereAxis<USERNODEDATA>::rethinkNavigateMenu() {
   // We loop through the items of "lastClickPath".  For each item,
   // we get the name of the root node, and append the full-path entry
   // to the navigate menu.
   string theString; // we'll keep appending to this...

   where4tree<USERNODEDATA> *currTree = rootPtr;   

   // Note: in tk4.0, menu indices start at 1, not 0 (0 is reserved for tearoff)
   char buffer[200];
   sprintf(buffer, "%s delete 1 100", navigateMenuName.string_of());
   myTclEval(interp, buffer);

   for (int item=0; item < lastClickPath.getSize(); item++) {
      if (item > 0)
         theString += "/";
      theString += currTree->getRootName();

      sprintf(buffer, "%s add command -label \"%s\" -command \"navigateTo %d\"",
              navigateMenuName.string_of(), theString.string_of(), item);
      myTclEval(interp, buffer);

      if (item < lastClickPath.getSize()-1) {
         const whereNodeRawPathItem &theItem = lastClickPath.getConstItem(item);
         currTree = currTree->theChildren[theItem.childnum].theTree;
      }
   }
}

template <class USERNODEDATA>
vector< vector<USERNODEDATA> > whereAxis<USERNODEDATA>::getSelections() const {
   // returns a vector[num-hierarchies] of vector of selections.
   // The number of hierarchies is defined as the number of children of the
   // root node.
   const unsigned numHierarchies = rootPtr->theChildren.size();

   vector < vector<USERNODEDATA> > result(numHierarchies);

   for (int i=0; i < numHierarchies; i++) {
      where4tree<USERNODEDATA> *hierarchyRoot = rootPtr->theChildren[i].theTree;

      result[i] = hierarchyRoot->getSelections();
      if (result[i].size()==0) {
         // this hierarchy had no selections; therefore, choose the hierarchy's
         // root item...
         vector<USERNODEDATA> defaultHierarchy(1);
         defaultHierarchy[0] = hierarchyRoot->theUserNodeData;
         result[i] = defaultHierarchy;
      }
      else if (rootPtr->isHighlighted()) {
         // The root node was highlighted --> add this hierarchy's root item,
         // if not already done.
         USERNODEDATA hierarchyRootId = hierarchyRoot->theUserNodeData;

         bool hierarchyRootAlreadyAdded = false;
         for (int j=0; j < result[i].size(); j++) {
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
