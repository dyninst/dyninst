/*
 * Copyright (c) 1996-2001 Barton P. Miller
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

// new search history graph user interface, along the lines
// of the new where axis user interface
// $Id: shg.C,v 1.32 2001/06/20 20:33:42 schendel Exp $
// Ariel Tamches

#include <assert.h>
#include "shg.h"
#if defined(i386_unknown_nt4_0)
#include <strstrea.h>
#else
#include <strstream.h>
#endif

#ifdef PARADYN
// the shg test program doesn't need this:
#include "performanceConsultant.thread.h" // for struct shg_node_info
#include "performanceConsultant.thread.CLNT.h" // for class performanceConsultantUser
#endif
#include "tkTools.h"

// Define static member vrbles:
Tk_Font shg::theRootItemFontStruct, shg::theRootItemShadowFontStruct;
Tk_Font shg::theListboxItemFontStruct, shg::theListboxItemShadowFontStruct;

vector<Tk_3DBorder> shg::rootItemTk3DBordersByStyle; // init to empty vector
vector<Tk_3DBorder> shg::listboxItemTk3DBordersByStyle; // inits to empty vector

GC shg::rootItemInactiveTextGC, shg::rootItemActiveTextGC;
GC shg::rootItemInactiveShadowTextGC, shg::rootItemActiveShadowTextGC;
  
GC shg::listboxInactiveItemGC, shg::listboxActiveItemGC;
GC shg::listboxInactiveShadowItemGC, shg::listboxActiveShadowItemGC;

GC shg::whyRefinementRayGC;
GC shg::whereRefinementRayGC;
GC shg::listboxRayGC;

int shg::listboxBorderPix = 3;
int shg::listboxScrollBarWidth = 16;

#ifdef PARADYN
extern performanceConsultantUser *perfConsult;
#endif

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

   whyRefinementRayGC = theShgConsts.whyRefinementRayGC;
   whereRefinementRayGC = theShgConsts.whereRefinementRayGC;
   listboxRayGC = consts.listboxRayGC;
}

shg::shg(int iPhaseId, Tcl_Interp *iInterp, Tk_Window theTkWindow,
	 const string &iHorizSBName, const string &iVertSBName,
	 const string &iCurrItemLabelName,
	 bool iHideTrue, bool iHideFalse, bool iHideUnknown, bool iHideNever,
	 bool iHideActive, bool iHideInactive, bool iHideShadow) :
	    hash(&hashFunc),
            hash2(&hashFunc2),
	    shadowNodeHash(&hashFuncShadow),
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

   hideTrueNodes = iHideTrue;
   hideFalseNodes = iHideFalse;
   hideUnknownNodes = iHideUnknown;
   hideNeverSeenNodes = iHideNever;
   hideActiveNodes = iHideActive;
   hideInactiveNodes = iHideInactive;
   hideShadowNodes = iHideShadow;
}

bool shg::state2hidden(shgRootNode::evaluationState es,
		       bool active, bool shadow) const {
   if (active && hideActiveNodes) return true;
   if (!active && hideInactiveNodes) return true;

   if (shadow && hideShadowNodes) return true;

   if (es==shgRootNode::es_true && hideTrueNodes) return true;
   else if (es==shgRootNode::es_false && hideFalseNodes) return true;
   else if (es==shgRootNode::es_unknown && hideUnknownNodes) return true;
   else if (es==shgRootNode::es_never && hideNeverSeenNodes) return true;

   return false;
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

#if !defined(i386_unknown_nt4_0)
   if (warpPointer)
      XWarpPointer(Tk_Display(consts.theTkWindow),
                   Tk_WindowId(consts.theTkWindow), // src win
                   Tk_WindowId(consts.theTkWindow), // dest win
                   0, 0, // src x,y
                   0, 0, // src height, width
                   relative_x, relative_y
                   );
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement warping support (?)
#endif // !defined(i386_unknown_nt4_0)

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

   if (rootPtr!=NULL)
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
   bool aflag;
   aflag=(thePath.getLastPathNode(pthis->rootPtr) == ptr);
   assert(aflag);

   // The scrollbar may no longer exist in the listbox, if one or more items
   // have been expanded from it, thus shrinking the listbox to the point where
   // a scrollbar was no longer needed.  Check for that now.
   if (!ptr->getScrollbar().isValid()) {
      //cout << "scrollbar no longer valid; ending dragging vrbles" << endl;
      XEvent hackEvent = *eventPtr;
      hackEvent.type = ButtonRelease;
      sliderButtonRelease(pthis, &hackEvent);
      return;
   }

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

   // Middle-mouse-click on a node:
   // Make an async request (to the PC) for information about this node
#ifdef PARADYN
   const shgRootNode &theNode = thePath.getLastPathNode(rootPtr)->getNodeData();
   perfConsult->requestNodeInfo(thePhaseId, theNode.getId());
#endif
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

         Tk_FontMetrics listboxFontMetrics; // filled in
         Tk_GetFontMetrics(consts.listboxFontStruct, &listboxFontMetrics);
         
         const unsigned itemHeight = consts.listboxVertPadAboveItem +
                                     listboxFontMetrics.ascent +
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

bool shg::changeHiddenNodesBase(bool isCurrShg) {
   // private routine called by the 2 implementations of changeHiddenNodes, below.
   if (rootPtr==NULL)
      return false; // nothing changed since shg has no nodes!

   // Now we need to loop thru each node in the dag, rethinking
   // whether or not it is hidden:
   if (!recursiveUpdateHiddenNodes(rootPtr))
      return false; // nothing changed.  Somewhat surprising.

   rootPtr->updateAnything2Draw(consts);
   rethink_entire_layout(isCurrShg);

   return true;
}

bool shg::changeHiddenNodes(bool newHideTrue, bool newHideFalse, bool newHideUnknown,
			    bool newHideNeverSeen, bool newHideActive,
			    bool newHideInactive, bool newHideShadow,
			    bool isCurrShg) {
   // returns true iff any changes
   if (hideTrueNodes == newHideTrue && hideFalseNodes == newHideFalse &&
       hideUnknownNodes == newHideUnknown && hideNeverSeenNodes == newHideNeverSeen &&
       hideActiveNodes == newHideActive && hideInactiveNodes == newHideInactive &&
       hideShadowNodes == newHideShadow)
      return false; // nothing changed

   hideTrueNodes = newHideTrue;
   hideFalseNodes = newHideFalse;
   hideUnknownNodes = newHideUnknown;
   hideNeverSeenNodes = newHideNeverSeen;
   hideActiveNodes = newHideActive;
   hideInactiveNodes = newHideInactive;
   hideShadowNodes = newHideShadow;

   return changeHiddenNodesBase(isCurrShg);
}
bool shg::changeHiddenNodes(shg::changeType ct, bool hide, bool isCurrShg) {
   switch (ct) {
      case shg::ct_true:
         if (hideTrueNodes == hide) return false;
	 hideTrueNodes = hide;
	 break;
       case shg::ct_false:
	 if (hideFalseNodes == hide) return false;
	 hideFalseNodes = hide;
	 break;
       case shg::ct_unknown:
	 if (hideUnknownNodes == hide) return false;
	 hideUnknownNodes = hide;
	 break;
       case shg::ct_never:
	 if (hideNeverSeenNodes == hide) return false;
	 hideNeverSeenNodes = hide;
	 break;
       case shg::ct_active:
	 if (hideActiveNodes == hide) return false;
	 hideActiveNodes = hide;
	 break;
       case shg::ct_inactive:
	 if (hideInactiveNodes == hide) return false;
	 hideInactiveNodes = hide;
	 break;
       case shg::ct_shadow:
	 if (hideShadowNodes == hide) return false;
	 hideShadowNodes = hide;
	 break;
       default:
	 assert(false);
   }

   return changeHiddenNodesBase(isCurrShg);
}

bool shg::recursiveUpdateHiddenNodes(where4tree<shgRootNode> *ptr) {
   // returns true iff any changes
   bool anyChanges = false; // so far...

   shgRootNode &theRootNode = ptr->getNodeData();
   bool isShadowNode = theRootNode.isShadowNode();
   bool curr_hidden = !theRootNode.anything2draw();

   bool this_node_should_be_hidden = state2hidden(theRootNode.getEvalState(),
						  theRootNode.isActive(),
						  isShadowNode);
   if (this_node_should_be_hidden != curr_hidden) {
      anyChanges = true;
      if (this_node_should_be_hidden)
         theRootNode.hidify();
      else
         theRootNode.unhide();
   }

   // Continue for children:
   for (unsigned childlcv=0; childlcv < ptr->getNumChildren(); childlcv++)
      if (recursiveUpdateHiddenNodes(ptr->getChildTree(childlcv)))
         anyChanges = true;

   return anyChanges;
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

   bool hidden = state2hidden(iEvalState, iActive, false);
      // false --> not a shadow node
   shgRootNode tempNewNode(id, iActive, iEvalState, false, label, fullInfo, hidden);

   where4tree<shgRootNode> *newNode = new where4tree<shgRootNode>(tempNewNode);
   assert(newNode);
 
   if (rootNodeFlag)
      rootPtr = newNode;

   assert(rootPtr);

   // Note that processing here is very different from the where axis.
   // In the where axis, you are always given a parent when inserting a node.
   // Here, we are not (intentionally); creating nodes and adding edges must be
   // kept separate, or else we couldn't create a dag (we would only be able to
   // create a tree).
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

shg::configNodeResult shg::configNode(unsigned id, bool newActive,
				      shgRootNode::evaluationState newEvalState,
				      bool isCurrShg,
				      bool rethinkIfNecessary) {
   // Does not redraw.  Possible return values:
   // noChanges, benignChanges (a node changed color but didn't expand/unexpand
   // or hide/unhide), changesInvolvingJustExpandedness, changesInvolvingHideness.

   // Note that a true value of "rethinkIfNecessary" can only lead to a return
   // value of noChanges or benignChanges, because any "higher" changes would trigger
   // the necessary rethink(s).
   //
   // In practice:
   // a return value indicating a change in just expandedness means that you should
   //    call rethink_entire_layout(isCurrShg)
   // a return value indicating a change in Hideness means that you should
   //    call rootPtr->updateAnything2Draw(consts)
   //    and  rethink_entire_layout(isCurrShg)

   // Note: a change from "tentatively-true" to (anything else) will un-expand;
   //                to "tentatively=true" from (anything else) will expand;
   //       leading to a massive layout rethinkification.
   // In addition, changing to or from a hidden state will also lead to
   //    layout rethinkification.
   // But the rethinking is always suppressed if "rethinkIfNecessary" is false.

   // Other changes are more simple -- simply changing the color of a node.
   assert(rootPtr);

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
   const bool oldActive = ptr->getNodeData().isActive();
   const bool oldHidden = state2hidden(oldEvalState, oldActive, false);
      // false --> we are not a shadow node

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
      return noChanges;

   // note: if we are changing this to "true (active or not)", we explicitly expand
   // "ptr".  Unfortunately, we need ptr's parent node for that; hence the need
   // for the hash table "hash2" (a mapping of nodes to their parents).  It's
   // somewhat of a hack; a solution eliminating the need for "hash2" would save memory.

   // Note that we make no effort to alter the expanded-ness of any shadowed nodes.
   // (Is this right?)

   bool resultExpandednessChanged = false; // so far
   bool resultHidenessChanged     = false; // so far
   bool rethink_layout = false; // so far...

   // Algorithm:
   // If a node becomes un-hidden, we unhide it first.
   // If a node becomes hidden, we hide it last.
   // In the middle come the possible expansions / un-expansions due to
   //    changing to/from es_true.

   // Algorithm Step 1: If a node becomes un-hidden
   // Note that there's no need to check our shadow children, because the
   // only trait that may lead to a hidden-ness that differs from us is that
   // they're shadow nodes, and that trait isn't gonna change.
   assert(oldHidden == !ptr->getNodeData().anything2draw());
   bool new_hidden = state2hidden(newEvalState, newActive,
				  false); // false --> not shadow node
   if (oldHidden != new_hidden && !new_hidden) {
      // The node is going to be un-hidden; let's do that first to avoid
      // an assertion failure.
      ptr->getNodeData().unhide();

      if (rethinkIfNecessary) {
         (void)rootPtr->updateAnything2Draw(consts); // result should be true
         rethink_layout = true;
            // probably a bit more than is needed...
      }
      else
	 resultHidenessChanged=true;
   }

   // Algorithm Step 2: Expansion/un-expansion due to change to/from es_true
   bool autoExpand   = (newEvalState == shgRootNode::es_true);
   bool autoUnExpand = (oldEvalState == shgRootNode::es_true);

   if (autoExpand || autoUnExpand) {
      where4tree<shgRootNode> *parentPtr = hash2[ptr];
      if (parentPtr == NULL)
         ; // either root node or a node which hasn't been addEdge'd in yet.
           // So, we skip the ceremonies (but we don't 'return' because there
           // may be more to do below w.r.t. hide-ness)
      else {
         // the following is unacceptably slow; it must be fixed by using real paths
         const unsigned numChildren = parentPtr->getNumChildren();
         for (unsigned childlcv=0; childlcv < numChildren; childlcv++)
            if (parentPtr->getChildTree(childlcv) == ptr) {
	       if (autoExpand)
                  parentPtr->explicitlyExpandSubchild(consts, childlcv, true);
	             // true --> force expansion, even if it's a leaf node
	       else
		  parentPtr->explicitlyUnexpandSubchild(consts, childlcv);
   
               if (rethinkIfNecessary)
		  rethink_layout = true;
	          // (all we really need to do is rethink all-expanded-children
   	          // dimensions for all ancestors of parentPtr)
	       else
		  resultExpandednessChanged = true;

	       break;
            } 

         assert(rethink_layout || !rethinkIfNecessary);
      }
   }
  
   if (oldHidden != new_hidden && new_hidden) {
      // The node is going to be hidden; let's do that last to avoid
      // an assertion failure.
      ptr->getNodeData().hidify();
      
      if (rethinkIfNecessary) {
	 rootPtr->updateAnything2Draw(consts); // result should be true
         rethink_layout = true;
            // probably a bit more than is needed...
      }
      else
	 resultHidenessChanged = true;
   }

   if (rethink_layout) {
      assert(rethinkIfNecessary);
      // If changes involving hideness occurred, they will have been processed,
      // since rethinkIfNecessary is true.  So right now all we have to do is
      // handle expansions that have occurred...
      rethink_entire_layout(isCurrShg);

      // ...which means there's nothing left for outside code to do but redraw:
      return benignChanges;
   }
   else if (resultHidenessChanged)
      // the biggest change; user must call rootPtr->updateAnything2Draw(consts) plus
      // rethink_entire_layout(isCurrShg)
      return changesInvolvingHideness;
   else if (resultExpandednessChanged)
      // user must call rethink_entire_layout(isCurrShg)
      return changesInvolvingJustExpandedness;
   else
      // user doesn't need to do anything before redrawing...
      return benignChanges;
}

bool shg::inactivateAll(bool isCurrShg) {
   // returns true iff any changes.  Does not redraw.
   // The usual case is just to change fg colors.  However, if we are e.g. hiding
   // inactive nodes, then a full rethinkification is a possibility.

   bool anyChanges = false; // so far...

   if (rootPtr == NULL)
      return false;

   // Loop through everything in the main hash table ("hash")
   // Assume ids start at 0.  But don't assume they aren't skip-numbered
   // (just to be safe).  If nodes are radically skip-numbered, then this
   // algorithm will be too slow; a recursive traversal replacement should
   // be no problem...
   const unsigned totalNumNodes = hash.size();
   unsigned numNodesProcessed = 0;
   unsigned nodeId = 0;

   bool needToRethinkHidden = false; // so far...
   bool needToRethinkLayout = false; // so far...

   while (numNodesProcessed < totalNumNodes) {
      if (!hash.defines(nodeId)) {
	 nodeId++;
         continue; // no progress made
      }

      // success
      where4tree<shgRootNode> *nodePtr = hash[nodeId];
      assert(hash2.defines(nodePtr));
         // may be NULL (rootPtr or not yet addEdge'd), but at least it should be there.

      // If this node has been addEdge'd (or is rootPtr), then call configNode.
      // Otherwise, configNode would just bomb, since it barfs on seeing any node
      // that hasn't been addEdge'd into the graph yet.  In that case, we configure
      // manually w/o rethinking.
      
      const shgRootNode::evaluationState oldEvalState = nodePtr->getNodeData().getEvalState();

      if (nodePtr == rootPtr || hash2[nodePtr] != NULL) {
         configNodeResult localResult = configNode(nodeId, false,
						      // false --> not active
						   oldEvalState, isCurrShg,
						   false // don't rethink
						   );
	 if (localResult != noChanges)
	    anyChanges = true;

	 if (localResult == changesInvolvingHideness) {
	    needToRethinkHidden = true;
	    needToRethinkLayout = true;
	 }
	 else if (localResult == changesInvolvingJustExpandedness)
	    needToRethinkLayout = true;
      }
      else {
	 // manual job on nodes which haven't yet been addEdge'd into the graph.
	 (void)nodePtr->getNodeData().configStyle(false, oldEvalState);
	    // false --> inactivate
	    // we ignore the return result; even if true, we don't want to redraw
      }

      nodeId++;
      numNodesProcessed++;
   }

   // Now do any deferred rethinkification, etc.:
   if (needToRethinkHidden)
      (void)rootPtr->updateAnything2Draw(consts); // result should be true

   if (needToRethinkLayout)
      rethink_entire_layout(isCurrShg);

   return anyChanges;   
}

void shg::addEdge(unsigned fromId, unsigned toId,
		  shgRootNode::refinement theRefinement,
		  const char *label, // only used for shadow nodes; else NULL
		  bool isCurrShg,
		  bool rethinkFlag) {
   // What to do about hidden nodes?
   // 1) If child is a shadow node then we must initialize the hidden flag properly
   // 2) If not a shadow node then probably nothing to do, assuming the hidden
   //    flag was properly set when the child was addNode'd.

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

      // For drawing purposes, let's correctly update the refinement field of
      // the child node, which would otherwise be left at ref_undefined.
      childPtr->getNodeData().setRefinement(theRefinement);
   }
   else {
      // We are adding a shadow node.
      assert(label != NULL);
      addingShadowNode = true;

      // copy all information from the existing "real" node...
      shgRootNode tempNewNode = childPtr->getNodeData();
      // ...and decide if it should be hidden...(if hide-shadow is in effect, the
      //    shadow node might be hidden while the real node isn't)
      if (state2hidden(tempNewNode.getEvalState(),
		       tempNewNode.isActive(),
		       true))
         tempNewNode.hidify();
      else
         tempNewNode.unhide();

      // ...and make it a shadow node:
      childPtr = new where4tree<shgRootNode> (tempNewNode.shadowify(label));
      assert(childPtr);

      vector<where4tree<shgRootNode>*> &theShadowNodes = shadowNodeHash[toId];
      theShadowNodes += childPtr;

      // Note: we do not add shadow node pointers to hash2[] (is this right?)

      // Note: we make no attempt to set the refinement field of the child shadow node.
      // It will have been copied from the "real" non-shadow node it points to, and
      // is presumably correct.  Hence no need to look at "theRefinement", except
      // perhaps to assert it's the same as that already present in the shadow node.
   }

   // Should the child node be explicitly expanded when added to its parent?
   // Here are the rules:
   // 1) If a shadow node, then no.
   // 2) Else, yes iff the child node is true
   // Note that whether the node is active or not is irrelevant.

   bool explicitlyExpandedFlag;
   if (addingShadowNode)
      explicitlyExpandedFlag = false;
   else {
      const shgRootNode &childNodeData = childPtr->getNodeData();
      shgRootNode::evaluationState theEvalState = childNodeData.getEvalState();

      explicitlyExpandedFlag = (theEvalState == shgRootNode::es_true);
   }

   parentPtr->addChild(childPtr,
		       explicitlyExpandedFlag,
		       consts,
		       false, // don't rethink graphics
		       false // don't resort
		       );

   // rethink layout of entire shg (slow...):
   if (rethinkFlag) rethink_entire_layout(isCurrShg);
}

void shg::removeBrackets(char *ptr) {
  char *oldptr;
  for(oldptr = ptr; *ptr != '\0' ; ptr++) {
    if(*ptr != '[' && *ptr != ']') {
      *oldptr = *ptr;
      oldptr++;
    } else {
      // skip the [ or ] characters, thus don't increment oldptr
    }
  }
  *oldptr = '\0';
}

#ifdef PARADYN
void shg::nodeInformation(unsigned nodeId, const shg_node_info &theNodeInfo) {
   // In response to a right-mouse-click...
   // First, delete what was in the curr-item-description widget
   string commandStr = currItemLabelName + " config -state normal";
   myTclEval(interp, commandStr);

   commandStr = currItemLabelName + " delete @0,0 end";
   myTclEval(interp, commandStr);

   // Second, fill in the multi-line description string...
   // (Note that we do extra stuff in developer mode...)

   ostrstream dataString;
   assert(hash.defines(nodeId));
   const shgRootNode &theNode = hash[nodeId]->getNodeData();
   
   // shg test program doesn't have a devel mode
   extern bool inDeveloperMode;
   if (inDeveloperMode)
     dataString << theNode.getId() << " ";

#if defined(i386_unknown_nt4_0)
    // the full name may include a pathname, which will include backslashes
    // which must be escaped to be acceptable to the Tk widget displaying the
    // name string
    string nameStr = theNode.getLongName();
    char tmpstr[2];
    tmpstr[1] = '\0';
    for( unsigned int i = 0; i < nameStr.length(); i++ )
    {
        // this approach for building up a new string seems horribly
        // ugly, but the string class provides no tokenizer support,
        // and we have no "append a character" operator.
        //
        // we would like to avoid having to create a new string object
        // just to add a character to an existing string, but it
        // appears from the current implementation that a new string
        // object would be created anyway, since the underlying reference-
        // counted object might be shared
        tmpstr[0] = nameStr[i];
        dataString << tmpstr;
        if( nameStr[i] == '\\' )
        {
	  dataString << tmpstr;
        }
    }
#else
    dataString << theNode.getLongName();
#endif // defined(i386_unknown_nt4_0)

   // The igen call isn't implemented in shg test program
   if (inDeveloperMode) {
     dataString << "\n";
     dataString << "curr concl: ";
     dataString << (theNodeInfo.currentConclusion ? "true" : "false");
     dataString << " made after time ";
     dataString << theNodeInfo.timeTrueFalse;
     dataString << "\n";
     dataString << "time from (" << theNodeInfo.startTime
		<< ") to (" << theNodeInfo.endTime;
     dataString << "); ";
     dataString << "persistent: " 
		<< (theNodeInfo.persistent ? "true" : "false");
     dataString << "; ";
     dataString << "active: " 
		<< (theNodeInfo.active ? "true" : "false");
     dataString << "\n";
     dataString << "curr value: " << theNodeInfo.currentValue;
     dataString << "; ";
     dataString << "thresh: " << theNodeInfo.lastThreshold;
     dataString << "; ";
     dataString << "hys constant: " << theNodeInfo.hysConstant;
     dataString << "; ";
     dataString << "adj value: " << theNodeInfo.adjustedValue;
     dataString << "; ";
     dataString << "estim cost: " << theNodeInfo.estimatedCost;
     dataString << ends;
   }

   commandStr = currItemLabelName + " insert end " + "\"" + dataString.str() 
                                  + "\"";
   char *noBrack_str = new char[commandStr.length()+1];
   strncpy(noBrack_str, commandStr.string_of(), commandStr.length());
   noBrack_str[commandStr.length()] = '\0';
   removeBrackets(noBrack_str);
   myTclEval(interp, noBrack_str);
   delete noBrack_str;

   commandStr = currItemLabelName + " config -state disabled";
   myTclEval(interp, commandStr);
}
#endif
