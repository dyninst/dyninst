// shg.C
// new search history graph user interface, along the lines
// of the new where axis user interface
// Ariel Tamches

/* $Log: shg.C,v $
/* Revision 1.10  1996/02/07 19:08:23  tamches
/* addNode, configNode, and addEdge now take in "isCurrShg" flag, which
/* is in turn passed to rethink_entire_layout
/*
 * Revision 1.9  1996/02/02 18:44:16  tamches
 * Displaying extra information about an shg node has changed from a mouse-move
 * to a middle-click
 *
 * Revision 1.8  1996/02/02 01:08:31  karavan
 * changes to support new PC/UI interface
 *
 * Revision 1.7  1996/01/23 07:02:20  tamches
 * added shadow node features
 *
 * Revision 1.6  1996/01/18 16:24:17  hollings
 * Added extra {}
 *
 * Revision 1.5  1996/01/11  23:41:38  tamches
 * there are now 2 kinds of true nodes; so tests changed accordingly.
 * Also brought the shg test program back to life
 *
 * Revision 1.4  1996/01/09 01:05:34  tamches
 * added thePhaseId member variable
 * added call to perfConsult->getNodeInfo to gather lots of juicy
 * information when displaying it in the shg status line at the bottom
 * of the window (which by the way, has been extended to 4 lines when in
 * developer mode)
 *
 * Revision 1.3  1995/11/19 04:21:50  tamches
 * added an #include of <assert.h> which had been missing, causing
 * problems on RS/6000 compiles.
 *
 * Revision 1.2  1995/11/06 19:28:03  tamches
 * slider mouse motion bug fixes
 *
 * Revision 1.1  1995/10/17 22:07:08  tamches
 * First version of "new search history graph".
 *
 */

#include <assert.h>
#include "tkTools.h"
#include "shg.h"

#ifdef PARADYN
// the shg test program doesn't need this:
#include "performanceConsultant.thread.h" // for struct shg_node_info
#include "performanceConsultant.thread.CLNT.h" // for class performanceConsultantUser
#endif

// Define static member vrbles:
XFontStruct *shg::theRootItemFontStruct, *shg::theRootItemShadowFontStruct;
XFontStruct *shg::theListboxItemFontStruct, *shg::theListboxItemShadowFontStruct;

vector<Tk_3DBorder> shg::rootItemTk3DBordersByStyle; // init to empty vector
vector<Tk_3DBorder> shg::listboxItemTk3DBordersByStyle; // inits to empty vector

GC shg::rootItemInactiveTextGC, shg::rootItemActiveTextGC;
GC shg::rootItemInactiveShadowTextGC, shg::rootItemActiveShadowTextGC;
  
GC shg::listboxInactiveItemGC, shg::listboxActiveItemGC;
GC shg::listboxInactiveShadowItemGC, shg::listboxActiveShadowItemGC;

int shg::listboxBorderPix = 3;
int shg::listboxScrollBarWidth = 16;

void shg::initializeStaticsIfNeeded() {
   if (rootItemTk3DBordersByStyle.size() > 0)
      return;

   theRootItemFontStruct = consts.rootTextFontStruct;
   theRootItemShadowFontStruct = theShgConsts.rootItemItalicFontStruct;

   theListboxItemFontStruct = consts.listboxFontStruct;
   theListboxItemShadowFontStruct = theShgConsts.listboxItemItalicFontStruct;

   rootItemTk3DBordersByStyle = theShgConsts.rootItemTk3DBordersByStyle;
   listboxItemTk3DBordersByStyle = theShgConsts.listboxItemTk3DBordersByStyle;

   rootItemInactiveTextGC = theShgConsts.rootItemInactiveTextGC;
   rootItemActiveTextGC = theShgConsts.rootItemActiveTextGC;
   rootItemInactiveShadowTextGC = theShgConsts.rootItemInactiveShadowTextGC;
   rootItemActiveShadowTextGC = theShgConsts.rootItemActiveShadowTextGC;

   listboxInactiveItemGC = theShgConsts.listboxItemInactiveTextGC;
   listboxActiveItemGC = theShgConsts.listboxItemActiveTextGC;
   listboxInactiveShadowItemGC = theShgConsts.listboxItemInactiveShadowTextGC;
   listboxActiveShadowItemGC = theShgConsts.listboxItemActiveShadowTextGC;
}

shg::shg(int iPhaseId, Tcl_Interp *iInterp, Tk_Window theTkWindow,
	 const string &iHorizSBName, const string &iVertSBName,
	 const string &iCurrItemLabelName) :
	    hash(&hashFunc, 32), hash2(&hashFunc2, 32),
	    shadowNodeHash(&hashFuncShadow, 32),
	    consts(iInterp, theTkWindow),
	    theShgConsts(iInterp, theTkWindow),
	    thePhaseId(iPhaseId),
	    horizSBName(iHorizSBName),
	    vertSBName(iVertSBName),
	    currItemLabelName(iCurrItemLabelName),
	    lastItemUnderMousePath(0, 0, consts, NULL, 0, 0) // yuck
{
   initializeStaticsIfNeeded();

   interp = iInterp;
   horizScrollBarOffset = vertScrollBarOffset = 0;
   
   rootPtr = NULL;

   rethink_nominal_centerx(); // will bomb if rootPtr is undefined, so we defer this...

   lastItemUnderMouseX = lastItemUnderMouseY = -1;
}

void shg::rethink_nominal_centerx() {
   // similar to where axis
   const int horizSpaceUsedByAll = (rootPtr==NULL) ? 0 : rootPtr->entire_width(consts);

   if (horizSpaceUsedByAll <= Tk_Width(consts.theTkWindow))
      nominal_centerx = Tk_Width(consts.theTkWindow) / 2;
   else
      nominal_centerx = horizSpaceUsedByAll / 2;
}

void shg::resizeScrollbars() {
   // same as where axis
   const int entire_width = rootPtr==NULL ? 0 : rootPtr->entire_width(consts);
   const int entire_height = rootPtr==NULL ? 0 : rootPtr->entire_height(consts);

   string commandStr = string("resize1Scrollbar ") + horizSBName + " " +
                       string(entire_width) + " " +
                       string(Tk_Width(consts.theTkWindow));
   myTclEval(interp, commandStr);

   commandStr = string("resize1Scrollbar ") + vertSBName + " " +
                string(entire_height) + " " +
                string(Tk_Height(consts.theTkWindow));
   myTclEval(interp, commandStr);
}

bool shg::set_scrollbars(int absolute_x, int relative_x,
			 int absolute_y, int relative_y,
			 bool warpPointer) {
   // Sets the scrollbars s.t. (absolute_x, absolute_y) will appear
   // at window (relative) location (relative_x, relative_y).
   // May need to take into account the current scrollbar setting...

   bool anyChanges = true;
   const int entire_width = rootPtr==NULL ? 0 : rootPtr->entire_width(consts);
   const int entire_height = rootPtr==NULL ? 0 : rootPtr->entire_height(consts);

   horizScrollBarOffset = -set_scrollbar(interp, horizSBName,
					 entire_width,
                                         absolute_x, relative_x);
      // relative_x will be updated

   vertScrollBarOffset = -set_scrollbar(interp, vertSBName,
					entire_height,
                                        absolute_y, relative_y);

   if (warpPointer)
      XWarpPointer(Tk_Display(consts.theTkWindow),
                   Tk_WindowId(consts.theTkWindow), // src win
                   Tk_WindowId(consts.theTkWindow), // dest win
                   0, 0, // src x,y
                   0, 0, // src height, width
                   relative_x, relative_y
                   );

   return anyChanges;
}

whereNodeGraphicalPath<shgRootNode> shg::point2path(int x, int y) const {
   const int overallWindowBorderPix = 0; // prob should be a static vrble
   const int root_centerx = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   const int root_topy = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   return whereNodeGraphicalPath<shgRootNode>(x, y, consts, rootPtr,
					      root_centerx, root_topy);
}

bool shg::adjustHorizSBOffset(float newFirstFrac) {
   // Does not redraw; returns true iff any changes were made
   newFirstFrac = moveScrollBar(interp, horizSBName, newFirstFrac);

   int widthOfAll = rootPtr==NULL ? 0 : rootPtr->entire_width(consts);
   int oldHorizSBOffset = horizScrollBarOffset;
   horizScrollBarOffset = -(int)(newFirstFrac * widthOfAll);
      // note: always <= 0
   return (horizScrollBarOffset != oldHorizSBOffset);
}

bool shg::adjustHorizSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw; returns true iff any changes.
   const int widthOfAll = rootPtr==NULL ? 0 : rootPtr->entire_width(consts);
   float newFirstFrac = (float)(-horizScrollBarOffset + deltapix) / widthOfAll;
   return adjustHorizSBOffset(newFirstFrac);
}

bool shg::adjustHorizSBOffset() {
   // does not redraw; returns true iff any changes
   float firstFrac, lastFrac;
   getScrollBarValues(interp, horizSBName, firstFrac, lastFrac);
   return adjustHorizSBOffset(firstFrac);
}

bool shg::adjustVertSBOffset(float newFirstFrac) {
   // does not redraw; returns true iff any changes.
   newFirstFrac = moveScrollBar(interp, vertSBName, newFirstFrac);

   int heightOfEverything = rootPtr==NULL ? 0 : rootPtr->entire_height(consts);
   int oldVertScrollBarOffset = vertScrollBarOffset;
   vertScrollBarOffset = -(int)(newFirstFrac * heightOfEverything);
      // note: always <= 0
   return (vertScrollBarOffset != oldVertScrollBarOffset);
}

bool shg::adjustVertSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw.  Returns true iff any changes were made.

   const int heightOfEverything = rootPtr==NULL ? 0 : rootPtr->entire_height(consts);
   float newFirst = (float)(-vertScrollBarOffset + deltapix) / heightOfEverything;
   return adjustVertSBOffset(newFirst);
}

bool shg::adjustVertSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, vertSBName, first, last);

   return adjustVertSBOffset(first);
}

void shg::draw(bool doubleBuffer, bool isXsynchOn) const {
   // same as where axis
   if (rootPtr==NULL)
      return;

   const int overallWindowBorderPix = 0;
   Drawable theDrawable = (doubleBuffer && !isXsynchOn) ? consts.offscreenPixmap :
                                                       Tk_WindowId(consts.theTkWindow);

   if (doubleBuffer || isXsynchOn)
      // clear the destination drawable before drawing onto it
      XFillRectangle(consts.display,
                     theDrawable,
                     consts.erasingGC,
                     0, // x-offset relative to drawable
                     0, // y-offset relative to drawable
                     Tk_Width(consts.theTkWindow),
                     Tk_Height(consts.theTkWindow)
                     );

   rootPtr->draw(consts.theTkWindow, consts, theDrawable,
                 nominal_centerx + horizScrollBarOffset,
                    // relative (not absolute) coord
                 overallWindowBorderPix + vertScrollBarOffset,
                    // relative (not absolute) coord
                 false, // not root only
                 false // not listbox only
                 );

   if (doubleBuffer && !isXsynchOn)
      // copy from offscreen pixmap onto the 'real' window
      XCopyArea(consts.display,
                theDrawable, // source pixmap
                Tk_WindowId(consts.theTkWindow), // dest pixmap
                consts.erasingGC, // ?????
                0, 0, // source x,y pix
                Tk_Width(consts.theTkWindow),
                Tk_Height(consts.theTkWindow),
                0, 0 // dest x,y offset pix
                );
}

void shg::resize(bool currentlyDisplayedAbstraction) {
   // same as where axis
   const int newWindowWidth = Tk_Width(consts.theTkWindow);
//   const int newWindowHeight = Tk_Height(consts.theTkWindow); [not used]

   consts.resize(); // reallocates offscreen pixmap; rethinks
                    // 'listboxHeightWhereSBappears'

   // Loop through EVERY listbox (gasp) and rethink whether or not it needs
   // a scrollbar and (possibly) change listBoxActualHeight...
   // _Must_ go _after_ the consts.resize()
   if (rootPtr)
      (void)rootPtr->rethinkListboxAfterResize(consts);

   // Now the more conventional resize code:
   if (rootPtr)
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

void shg::
processNonSliderButtonPress(whereNodeGraphicalPath<shgRootNode> &thePath) {
   // same as where axis
   nonSliderButtonCurrentlyPressed = true;
   nonSliderButtonPressRegion = thePath.whatDoesPathEndIn();
   nonSliderCurrentSubtree = thePath.getLastPathNode(rootPtr);
   nonSliderSubtreeCenter = thePath.get_endpath_centerx();
   nonSliderSubtreeTop = thePath.get_endpath_topy();

   Tk_CreateEventHandler(consts.theTkWindow, ButtonReleaseMask,
                         nonSliderButtonRelease, this);

   nonSliderButtonAutoRepeatCallback(this);
}

void shg::nonSliderButtonAutoRepeatCallback(ClientData cd) {
   // same as where axis
   // If the mouse button has been released, do NOT re-invoke the timer.
   shg *pthis = (shg *)cd;
   if (!pthis->nonSliderButtonCurrentlyPressed)
      return;

   const int listboxLeft = pthis->nonSliderSubtreeCenter -
                           pthis->nonSliderCurrentSubtree->
                             horiz_pix_everything_below_root(pthis->consts) / 2;
   const int listboxTop = pthis->nonSliderSubtreeTop +
                          pthis->nonSliderCurrentSubtree->getNodeData().
                               getHeightAsRoot() +
                          pthis->consts.vertPixParent2ChildTop;

   const int listboxActualDataPix = pthis->nonSliderCurrentSubtree->getListboxActualPixHeight() - 2 * listboxBorderPix;
   int deltaYpix = 0;
   int repeatIntervalMillisecs = 100;

   switch (pthis->nonSliderButtonPressRegion) {
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarUpArrow:
         deltaYpix = -4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarDownArrow:
         deltaYpix = 4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPageup:
         deltaYpix = -listboxActualDataPix;
         repeatIntervalMillisecs = 250; // not so fast
         break;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPagedown:
         deltaYpix = listboxActualDataPix;
         repeatIntervalMillisecs = 250; // not so fast
         break;
      default:
         assert(false);
   }

   (void)pthis->nonSliderCurrentSubtree->scroll_listbox(pthis->consts,
                                                        listboxLeft, listboxTop,
                                                        deltaYpix);
   pthis->buttonAutoRepeatToken = Tk_CreateTimerHandler(repeatIntervalMillisecs,
                                                        nonSliderButtonAutoRepeatCallback,
                                                        pthis);
}

void shg::nonSliderButtonRelease(ClientData cd, XEvent *) {
   // same as where axis
   shg *pthis = (shg *)cd;

   pthis->nonSliderButtonCurrentlyPressed = false;
   Tk_DeleteTimerHandler(pthis->buttonAutoRepeatToken);
}

void shg::sliderMouseMotion(ClientData cd, XEvent *eventPtr) {
   // This is a static method.
   // Since, in an shg, things may expand at any time (even while one is sliding
   // a scrollbar), we must be very careful.  The algorithm we use here is slower than
   // that of the where axis, but is safer.  We use "slider_scrollbar_path"
   // to obtain what the where axis caches and assumes not to change.
   assert(eventPtr->type == MotionNotify);
   shg *pthis = (shg *)cd;

   const int y = eventPtr->xmotion.y;
   const int amount_moved = y - pthis->slider_initial_yclick;
      // may be negative, of course.
   const int newScrollBarSliderTopPix = pthis->slider_initial_scrollbar_slider_top +
                                        amount_moved;

   assert(pthis->slider_currently_dragging_subtree != NULL);

   const int overallWindowBorderPix = 0;
   int root_centerx = pthis->nominal_centerx + pthis->horizScrollBarOffset;
   int root_centery = overallWindowBorderPix + pthis->vertScrollBarOffset;
   whereNodeGraphicalPath<shgRootNode> thePath(pthis->slider_scrollbar_path,
					       pthis->consts, pthis->rootPtr,
					       root_centerx, root_centery);

   // a sanity check:
   if (thePath.getLastPathNode(pthis->rootPtr) !=
       pthis->slider_currently_dragging_subtree) {
      cerr << "shg: slider-dragging-subtree unexpectedly changed (ignoring)" << endl;
      return;
   }

   where4tree<shgRootNode> *ptr = pthis->slider_currently_dragging_subtree;
   assert(thePath.getLastPathNode(pthis->rootPtr) == ptr);

   int slider_scrollbar_top = thePath.get_endpath_topy() +
                                 ptr->getNodeData().getHeightAsRoot() +
			      pthis->consts.vertPixParent2ChildTop;

   int slider_scrollbar_bottom = slider_scrollbar_top +
                                 ptr->getListboxActualPixHeight() - 1;

   int slider_scrollbar_left = thePath.get_endpath_centerx() -
                               ptr->horiz_pix_everything_below_root(pthis->consts) / 2;

   (void)ptr->rigListboxScrollbarSliderTopPix(pthis->consts,
					      slider_scrollbar_left,
					      slider_scrollbar_top,
					      slider_scrollbar_bottom,
					      newScrollBarSliderTopPix,
					      true // redraw now
					      );
}

void shg::sliderButtonRelease(ClientData cd, XEvent *eventPtr) {
   assert(eventPtr->type == ButtonRelease);
   shg *pthis = (shg *)cd;

   Tk_DeleteEventHandler(pthis->consts.theTkWindow,
                         PointerMotionMask,
                         sliderMouseMotion,
                         pthis);
   Tk_DeleteEventHandler(pthis->consts.theTkWindow,
                         ButtonReleaseMask,
                         sliderButtonRelease,
                         pthis);
   pthis->slider_currently_dragging_subtree = NULL;
}


void shg::processSingleClick(int x, int y) {
   if (rootPtr == NULL)
      return;

   whereNodeGraphicalPath<shgRootNode> thePath = point2path(x, y);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<shgRootNode>::Nothing:
         return;
      case whereNodeGraphicalPath<shgRootNode>::ExpandedNode: {
         where4tree<shgRootNode> *ptr = thePath.getLastPathNode(rootPtr);
         ptr->toggle_highlight();
         ptr->getNodeData().drawAsRoot(consts.theTkWindow,
				       Tk_WindowId(consts.theTkWindow),
				       thePath.get_endpath_centerx(),
				       thePath.get_endpath_topy());
         return;
      }
      case whereNodeGraphicalPath<shgRootNode>::ListboxItem:
         thePath.getLastPathNode(rootPtr)->toggle_highlight();
         thePath.getParentOfLastPathNode(rootPtr)->
                      draw(consts.theTkWindow,
			   consts, Tk_WindowId(consts.theTkWindow),
			   thePath.get_endpath_centerx(),
			   thePath.get_endpath_topy(),
			   false, // not root only
			   true // listbox only
			   );
         return;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPagedown:
         processNonSliderButtonPress(thePath);
         return;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarSlider: {
         where4tree<shgRootNode> *parentPtr = thePath.getLastPathNode(rootPtr);

         slider_initial_yclick = y;
         slider_currently_dragging_subtree = parentPtr;
         slider_scrollbar_path = thePath.getPath();

         const int lbTop = thePath.get_endpath_topy() +
                           parentPtr->getNodeData().getHeightAsRoot() +
                           consts.vertPixParent2ChildTop;

         int dummyint;
         parentPtr->getScrollbar().getSliderCoords(lbTop,
                     lbTop + parentPtr->getListboxActualPixHeight() - 1,
                     parentPtr->getListboxActualPixHeight() - 2*listboxBorderPix,
                     parentPtr->getListboxFullPixHeight() - 2*listboxBorderPix,
                     slider_initial_scrollbar_slider_top, // filled in
                     dummyint);

         Tk_CreateEventHandler(consts.theTkWindow,
                               ButtonReleaseMask,
                               sliderButtonRelease,
                               this);
         Tk_CreateEventHandler(consts.theTkWindow,
                               PointerMotionMask,
                               sliderMouseMotion,
                               this);
         
         return;
      }
      default:
         assert(false);
   }
}

void shg::processMiddleClick(int x, int y) {
   // obtain information about a node...or, if the press was on
   // a scrollbar, treat it as a normal left-button press
   if (rootPtr == NULL)
      return;

   whereNodeGraphicalPath<shgRootNode> thePath = point2path(x, y);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<shgRootNode>::Nothing:
         return;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPagedown:
         processNonSliderButtonPress(thePath);
         return;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarSlider: {
         where4tree<shgRootNode> *parentPtr = thePath.getLastPathNode(rootPtr);

         slider_initial_yclick = y;
         slider_currently_dragging_subtree = parentPtr;
         slider_scrollbar_path = thePath.getPath();

         const int lbTop = thePath.get_endpath_topy() +
                           parentPtr->getNodeData().getHeightAsRoot() +
                           consts.vertPixParent2ChildTop;

         int dummyint;
         parentPtr->getScrollbar().getSliderCoords(lbTop,
                     lbTop + parentPtr->getListboxActualPixHeight() - 1,
                     parentPtr->getListboxActualPixHeight() - 2*listboxBorderPix,
                     parentPtr->getListboxFullPixHeight() - 2*listboxBorderPix,
                     slider_initial_scrollbar_slider_top, // filled in
                     dummyint);

         Tk_CreateEventHandler(consts.theTkWindow,
                               ButtonReleaseMask,
                               sliderButtonRelease,
                               this);
         Tk_CreateEventHandler(consts.theTkWindow,
                               PointerMotionMask,
                               sliderMouseMotion,
                               this);
         
         return;
      }
      default: break;
   }

   string commandStr = currItemLabelName + " config -state normal";
   myTclEval(interp, commandStr);

   commandStr = currItemLabelName + " delete @0,0 end";
   myTclEval(interp, commandStr);

   // Note that we write different stuff if we are in devel mode, so
   // we need to check the tunable const:
   const shgRootNode &theNode = thePath.getLastPathNode(rootPtr)->getNodeData();

   string dataString;
         
#ifdef PARADYN
   // the shg test program doesn't have a developer mode
   extern bool inDeveloperMode;
   if (inDeveloperMode)
      dataString += string(theNode.getId()) + " ";
#endif

   dataString += theNode.getLongName();

#ifdef PARADYN
   // the igen call isn't implemented in the shg axis test program
   if (inDeveloperMode) {
      dataString += "\n";
      // make an igen call to the performance consultant to get more information
      // about this node:
      extern performanceConsultantUser *perfConsult;
      shg_node_info theNodeInfo;
      assert(perfConsult->getNodeInfo(thePhaseId, theNode.getId(), &theNodeInfo));
      dataString += "curr concl: ";
      dataString += theNodeInfo.currentConclusion ? "true" : "false";
      dataString += " made at time ";
      dataString += string(theNodeInfo.timeTrueFalse) + "\n";
      dataString += string("curr value: ") + string(theNodeInfo.currentValue);
      dataString += string(" estim cost: ") + string(theNodeInfo.estimatedCost) + "\n";
      dataString += string("time from ") + string(theNodeInfo.startTime) + " to " +
	                                 string(theNodeInfo.endTime) + "\n";
      dataString += string("persistent: ") + (theNodeInfo.persistent ? "true" : "false");
   }
#endif

   commandStr = currItemLabelName + " insert end " + "\"" + dataString + "\"";
   myTclEval(interp, commandStr);

   commandStr = currItemLabelName + " config -state disabled";
   myTclEval(interp, commandStr);
}

bool shg::processDoubleClick(int x, int y) {
   if (rootPtr==NULL)
      return false;

   // returns true iff a complete redraw is called for
   bool scrollToWhenDone = false;
   whereNodeGraphicalPath<shgRootNode> thePath = point2path(x, y);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<shgRootNode>::Nothing:
         return false;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarPagedown:
         // processNonSliderButtonPress(thePath); // same as single-click
         return false;
      case whereNodeGraphicalPath<shgRootNode>::ListboxScrollbarSlider:
         return false;
      case whereNodeGraphicalPath<shgRootNode>::ExpandedNode: {
         if (thePath.getSize()==0)
            return false; // ignore a double-click on the root

         // un-expand
         if (!rootPtr->path2lbItemUnexpand(consts, thePath.getPath(), 0))
            return false;

         rethink_nominal_centerx();
         resizeScrollbars();
         adjustHorizSBOffset();
         adjustVertSBOffset();
         softScrollToEndOfPath(thePath.getPath());
 
         return true;
      }
      case whereNodeGraphicalPath<shgRootNode>::ListboxItem: {
         const bool anyChanges = rootPtr->path2lbItemExpand(consts, thePath.getPath(), 0);
         if (!anyChanges) {
            // just change highlighted-ness
            thePath.getLastPathNode(rootPtr)->toggle_highlight();
            thePath.getParentOfLastPathNode(rootPtr)->
                    draw(consts.theTkWindow, consts, Tk_WindowId(consts.theTkWindow),
			 thePath.get_endpath_centerx(),
			 thePath.get_endpath_topy(),
			 false, // not root only
			 true // listbox only
			 );
            return false;
         }
         else
            // expansion was successfull
            scrollToWhenDone = true;
         break;
      }
      default:
         assert(false);
   }

   // expansion or un-expansion successful

   rethink_nominal_centerx();
   
   resizeScrollbars();

   adjustHorizSBOffset();
   adjustVertSBOffset();

   if (scrollToWhenDone) {
      const int overallWindowBorderPix = 0;
      whereNodeGraphicalPath<shgRootNode> 
           path_to_scroll_to(thePath.getPath(),
			     consts, rootPtr,
			     nominal_centerx,
			     overallWindowBorderPix
			     );
      int newlyExpandedElemCenterX = path_to_scroll_to.get_endpath_centerx();
      int newlyExpandedElemTopY    = path_to_scroll_to.get_endpath_topy();
      int newlyExpandedElemMiddleY = newlyExpandedElemTopY +
	                             path_to_scroll_to.getLastPathNode(rootPtr)->getNodeData().getHeightAsRoot() / 2;

      (void)set_scrollbars(newlyExpandedElemCenterX, x,
			   newlyExpandedElemMiddleY, y,
			   true);
   }

   return true;
}

bool shg::softScrollToEndOfPath(const whereNodePosRawPath &thePath) {
   assert(rootPtr);
   const int overallWindowBorderPix = 0;
   whereNodeGraphicalPath<shgRootNode>
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
      case whereNodeGraphicalPath<shgRootNode>::ExpandedNode: {
         const int last_item_middley =
            last_item_topy +
            scrollToPath.getLastPathNode(rootPtr)->getNodeData().getHeightAsRoot() / 2;
         return set_scrollbars(last_item_centerx,
                               Tk_Width(consts.theTkWindow) / 2,
                               last_item_middley,
                               Tk_Height(consts.theTkWindow) / 2,
                               true);
      }
      case whereNodeGraphicalPath<shgRootNode>::ListboxItem: {
         // First, let's scroll within the listbox (no redrawing yet)
         where4tree<shgRootNode> *parent = scrollToPath.getParentOfLastPathNode(rootPtr);

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
            (void)parent->scroll_listbox(consts, scrollToVertPix -
                                         parent->getScrollbar().getPixFirst());

            destItemRelToListboxTop -= parent->getScrollbar().getPixFirst();
         }
         if (destItemRelToListboxTop < 0) {
            cout << "note: softScrollToEndOfPath() failed to scroll properly to item w/in listbox" << endl;
         }

         return set_scrollbars(scrollToPath.get_endpath_centerx() -
                                  parent->horiz_pix_everything_below_root(consts)/2 +
                                  parent->getListboxPixWidth() / 2,
                                  // listbox centerx
                               Tk_Width(consts.theTkWindow) / 2,
                               scrollToPath.get_endpath_topy() +
                                  parent->getNodeData().getHeightAsRoot() +
                                  consts.vertPixParent2ChildTop +
                                  destItemRelToListboxTop + itemHeight / 2,
                                  // should be the middley of the lb item
                               Tk_Height(consts.theTkWindow) / 2,
                               true);
      }
      default: assert(false);
   }

   assert(false);
   return false; // placate compiler
}

void shg::addNode(unsigned id, bool iActive, shgRootNode::evaluationState iEvalState,
		  const string &label, const string &fullInfo,
		  bool rootNodeFlag, bool isCurrShg) {
   if (rootNodeFlag) {
      assert(hash.size() == 0);
      assert(rootPtr == NULL);
   }
   else {
      assert(hash.size() > 0);
      assert(rootPtr != NULL);
   }

   shgRootNode tempNewNode(id, iActive, iEvalState, false, label, fullInfo);
      // false --> not a shadow node
   where4tree<shgRootNode> *newNode = new where4tree<shgRootNode>(tempNewNode);
   assert(newNode);

   if (rootNodeFlag)
      rootPtr = newNode;

   assert(rootPtr);

   // Note that processing here is very different from the where axis.
   // In the where axis, you are always given a parent when inserting a node.
   // Here, we are not (which is ugly and should be eventually fixed, imho).
   // So, we put the node in the hash table but don't try to link it to any parent
   // until the addEdge...

   // the id should not already be in use
   assert(!hash.defines(id));
   hash[id] = newNode;
   assert(hash.defines(id));

   assert(!hash2.defines(newNode));
   hash2[newNode] = NULL; // no parent yet (until addEdge)
   assert(hash2.defines(newNode));

   if (rootNodeFlag)
      // only in this case do we rethink layout sizes; otherwise,
      // we wait until an edge connects to this new node.
      rethink_entire_layout(isCurrShg);
}

bool shg::configNode(unsigned id, bool newActive,
		     shgRootNode::evaluationState newEvalState,
		     bool isCurrShg) {
   // returns true iff any changes.  Does not redraw.
   // Note: a change from "tentatively-true" to (anything else)
   // will un-expand the node, leading to a massive layout rethinkification.
   // Other changes are more simple -- simply changing the color of a node.
   assert(rootPtr);

   // returns true iff any changes.  Does not redraw.
   assert(hash.defines(id));
   where4tree<shgRootNode> *ptr = hash[id];
   assert(ptr);

   // just a sanity check: ptr should have a parent (will be NULL if ptr is the root)
   assert(hash2.defines(ptr));
   // added extra { } around this if, the AIX g++ compiler seems to need it
   //  jkh - 1/18/96
   if (ptr == rootPtr) {
      assert(hash2[ptr] == NULL);
   } else {
      assert(hash2[ptr] != NULL);
   } 

   const shgRootNode::evaluationState oldEvalState = ptr->getNodeData().getEvalState();

   bool anyChanges = ptr->getNodeData().configStyle(newActive, newEvalState);

   if (shadowNodeHash.defines(id)) {
      // shadow nodes exist for this id.  configStyle() them, too.
      vector< where4tree<shgRootNode>* > &shadowList = shadowNodeHash[id];
      for (unsigned i=0; i < shadowList.size(); i++) {
         where4tree<shgRootNode>* shadowNode = shadowList[i];
         if (shadowNode->getNodeData().configStyle(newActive, newEvalState))
            anyChanges = true;
      }
   }
   
   if (!anyChanges)
      return false;

   // note: if we are changing this to "true (active or not)", we explicitly expand
   // "ptr".  Unfortunately, we need ptr's parent node for that; hence the need
   // for the hash table "hash2" (a mapping of nodes to their parents).  It's
   // somewhat of a hack; a solution eliminating the need for "hash2" would save memory.

   // Note that we make no effort to alter the expanded-ness of any shadowed nodes.

   if (newEvalState == shgRootNode::es_true) {
      // if we have changed to true (whether active or not)
      where4tree<shgRootNode> *parentPtr = hash2[ptr];
      if (parentPtr == NULL)
         return false; // either root node or a node which hasn't been addEdge'd in yet

      // the following is unacceptably slow; it must be fixed by using real paths
      const unsigned numChildren = parentPtr->getNumChildren();
      for (unsigned childlcv=0; childlcv < numChildren; childlcv++)
         if (parentPtr->getChildTree(childlcv) == ptr) {
            parentPtr->explicitlyExpandSubchild(consts, childlcv, true);
               // true --> force expansion, even if it's a leaf node
 
            // now rethink all-expanded-children dimensions for all ancestors
            // of parentPtr.  For now, we are sloppy, and just rethink those traits
            // for the entier shg (!):
            rethink_entire_layout(isCurrShg);
            return true;
	 }

      assert(false);
      return true; // placate compiler
   }
   else if (oldEvalState == shgRootNode::es_true) {
      // It used to be true (active or not), but ain't anymore.
      where4tree<shgRootNode> *parentPtr = hash2[ptr];
      if (parentPtr == NULL)
         return false; // either root node or a node which hasn't been addEdge'd in yet

      const unsigned numChildren = parentPtr->getNumChildren();
      for (unsigned childlcv=0; childlcv < numChildren; childlcv++)
         if (parentPtr->getChildTree(childlcv) == ptr) {
            parentPtr->explicitlyUnexpandSubchild(consts, childlcv);

            // now rethink all-expanded-children dimensions for all ancestors
            // of parentPtr.  For now, we are sloppy, and just rethink those traits
            // for the entier shg (!):
	    rethink_entire_layout(isCurrShg);
            return true;
	 }

      assert(false);
      return true; // placate compiler
   }
   else
      return true;
}

void shg::addEdge(unsigned fromId, unsigned toId,
		  shgRootNode::evaluationState theState,
		  const char *label, // only used for shadow nodes; else NULL
		  bool isCurrShg
		  ) {
   assert(rootPtr);

   // Obviously, both nodes must already exist.
   assert(hash.defines(fromId));
   assert(hash.defines(toId));
   
   where4tree<shgRootNode> *parentPtr = hash[fromId];
   assert(parentPtr);

   where4tree<shgRootNode> *childPtr  = hash[toId];
   assert(childPtr);

   // If, before this addEdge, childPtr had no parents, then this is a "normal"
   // addEdge operation in which the child node will finally be made visible on the
   // screen.
   // Else, we are adding a shadow node.  More specifically, the new child of
   // "parentPtr" will just be made a shadow node.  We'll _create_ a separate node
   // ptr for this shadow node.  "childPtr" will be unused (more specifically, we'll
   // let it dangle and overwrite it)

   bool addingShadowNode = false;
   if (hash2[childPtr] == NULL) {
      // We are _not_ adding a shadow node.
      hash2[childPtr] = parentPtr;
      assert(label==NULL);
   }
   else {
      // We are adding a shadow node.
      assert(label != NULL);
      addingShadowNode = true;

      // copy all information from the existing "real" node...
      shgRootNode tempNewNode = childPtr->getNodeData();
      // ...and make it a shadow node:
      childPtr = new where4tree<shgRootNode> (tempNewNode.shadowify(label));
      assert(childPtr);

      vector<where4tree<shgRootNode>*> &theShadowNodes = shadowNodeHash[toId];
      theShadowNodes += childPtr;

      // Note: we do not add shadow node pointers to hash2[] (is this right?)
   }

   const bool explicitlyExpandedFlag = (theState==shgRootNode::es_true &&
					!addingShadowNode);
      // whether active or not...
   parentPtr->addChild(childPtr,
		       explicitlyExpandedFlag,
		       consts,
		       false, // don't rethink graphics
		       false // don't resort
		       );

   // rethink layout of entire shg (slow...):
   rethink_entire_layout(isCurrShg);
}

void shg::addToStatusDisplay(const string &str) {
   string commandStr = string(".shg.nontop.textarea.text insert end ") +
                       "{" + str + "}\n";
   myTclEval(interp, commandStr);

   commandStr = ".shg.nontop.textarea.text yview -pickplace end";
   myTclEval(interp, commandStr);
}
