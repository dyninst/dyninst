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

// whereAxis.C
// Ariel Tamches

// A where axis corresponds to _exactly_ one Paradyn abstraction.

/* $Id: whereAxis.C,v 1.27 2002/12/20 07:50:05 jaw Exp $ */

#include <stdlib.h> // exit()

#include "minmax.h"

#ifndef PARADYN
#include "DMinclude.h"
#else
#include "paradyn/src/DMthread/DMinclude.h"
#endif

#include "whereAxis.h"
#include "whereAxisTcl.h"

#ifndef PARADYN
// only the where axis test program needs this
#include "whereAxisMisc.h"
#endif

#include "tkTools.h"
#include "callGraphs.h"

//modified by wxd
extern callGraphs *theCallGraphPrograms;

Tk_Font whereAxis::theRootItemFontStruct = NULL;
Tk_Font whereAxis::theListboxItemFontStruct = NULL;
Tk_3DBorder  whereAxis::rootItemTk3DBorder = NULL;
GC           whereAxis::rootItemTextGC = NULL;
GC           whereAxis::rootRetiredItemTextGC = NULL;
Tk_3DBorder  whereAxis::listboxItem3DBorder = NULL;
GC           whereAxis::listboxItemGC = NULL;
GC           whereAxis::listboxRetiredItemGC = NULL;
GC           whereAxis::listboxRayGC = NULL;
GC           whereAxis::nonListboxRayGC = NULL;
int          whereAxis::listboxBorderPix = 3;
int          whereAxis::listboxScrollBarWidth = 16;


void whereAxis::initializeStaticsIfNeeded() {
   if (theRootItemFontStruct == NULL)
      // somewhat kludgy
      theRootItemFontStruct = consts.rootTextFontStruct;

   if (theListboxItemFontStruct == NULL)
      // somewhat kludgy
      theListboxItemFontStruct = consts.listboxFontStruct;

   if (rootItemTk3DBorder == NULL)
      // somewhat kludgy
      rootItemTk3DBorder = consts.rootNodeBorder;

   if (rootItemTextGC == NULL)
      // somewhat kludgy
      rootItemTextGC = consts.rootItemTextGC;

   if (rootRetiredItemTextGC == NULL)
      // somewhat kludgy
      rootRetiredItemTextGC = consts.rootRetiredTextGC;

   if (listboxItem3DBorder == NULL)
      // somewhat kludgy
      listboxItem3DBorder = consts.listboxBorder; // ???

   if (listboxItemGC == NULL)
      listboxItemGC = consts.listboxTextGC;

   if(listboxRetiredItemGC == NULL)
      listboxRetiredItemGC = consts.listboxRetiredTextGC;

   if (listboxRayGC == NULL)
      listboxRayGC = consts.listboxRayGC;

   if (nonListboxRayGC == NULL)
      nonListboxRayGC = consts.subchildRayGC;
}

whereAxis::whereAxis(Tcl_Interp *in_interp, Tk_Window theTkWindow,
		     const string &root_str,
		     const string &iHorizSBName,
		     const string &iVertSBName,
		     const string &iNavigateMenuName) :
	     consts(in_interp, theTkWindow),
	     hash(&whereAxis::hashFunc),
	     horizSBName(iHorizSBName),
	     vertSBName(iVertSBName),
	     navigateMenuName(iNavigateMenuName) {
   initializeStaticsIfNeeded();

   const resourceHandle rootResHandle = 0;

   whereAxisRootNode tempRootNode(rootResHandle, root_str);
   rootPtr = new where4tree<whereAxisRootNode>(tempRootNode);
   assert(rootPtr);

   hash[rootResHandle] = rootPtr;

   beginSearchFromPtr = NULL;

   interp = in_interp;
      
   horizScrollBarOffset = vertScrollBarOffset = 0;
      
   rethink_nominal_centerx();

   nonSliderButtonCurrentlyPressed = false;
   nonSliderCurrentSubtree = NULL;
   slider_currently_dragging_subtree = NULL;
}

void whereAxis::rethink_nominal_centerx() {
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

void whereAxis::resizeScrollbars() {
   string commandStr = string("resize1Scrollbar ") + horizSBName + " " +
                       string(rootPtr->entire_width(consts)) + " " +
		       string(Tk_Width(consts.theTkWindow));
   myTclEval(interp, commandStr);

   commandStr = string("resize1Scrollbar ") + vertSBName + " " +
                string(rootPtr->entire_height(consts)) + " " +
		string(Tk_Height(consts.theTkWindow));
   myTclEval(interp, commandStr);
}

bool whereAxis::set_scrollbars(int absolute_x, int relative_x,
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
	// TODO - implement warping behavior (?)
#endif // !defined(i386_unknown_nt4_0)

   return anyChanges;
}

whereNodeGraphicalPath<whereAxisRootNode> whereAxis::point2path(int x, int y) const {
   const int overallWindowBorderPix = 0;
   const int root_centerx = nominal_centerx + horizScrollBarOffset;
      // relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   const int root_topy = overallWindowBorderPix + vertScrollBarOffset;
      // relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   return whereNodeGraphicalPath<whereAxisRootNode>(x, y, consts, rootPtr,
						    root_centerx, root_topy);
}

void whereAxis::nonSliderButtonRelease(ClientData cd, XEvent *) {
   whereAxis *pthis = (whereAxis *)cd;

   pthis->nonSliderButtonCurrentlyPressed = false;
   Tk_DeleteTimerHandler(pthis->buttonAutoRepeatToken);
}

void whereAxis::nonSliderButtonAutoRepeatCallback(ClientData cd) {
   // If the mouse button has been released, do NOT re-invoke the timer.
   whereAxis *pthis = (whereAxis *)cd;
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
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarUpArrow:
         deltaYpix = -4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarDownArrow:
         deltaYpix = 4;
         repeatIntervalMillisecs = 10;
         break;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPageup:
         deltaYpix = -listboxActualDataPix;
         repeatIntervalMillisecs = 250; // not so fast
         break;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPagedown:
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

void whereAxis::
processNonSliderButtonPress(whereNodeGraphicalPath<whereAxisRootNode> &thePath) {
   nonSliderButtonCurrentlyPressed = true;
   nonSliderButtonPressRegion = thePath.whatDoesPathEndIn();
   nonSliderCurrentSubtree = thePath.getLastPathNode(rootPtr);
   nonSliderSubtreeCenter = thePath.get_endpath_centerx();
   nonSliderSubtreeTop = thePath.get_endpath_topy();

   Tk_CreateEventHandler(consts.theTkWindow, ButtonReleaseMask,
			 nonSliderButtonRelease, this);

   nonSliderButtonAutoRepeatCallback(this);
}

void whereAxis::sliderMouseMotion(ClientData cd, XEvent *eventPtr) {
   assert(eventPtr->type == MotionNotify);
   whereAxis *pthis = (whereAxis *)cd;

   const int y = eventPtr->xmotion.y;
   const int amount_moved = y - pthis->slider_initial_yclick;
      // may be negative, of course.
   const int newScrollBarSliderTopPix = pthis->slider_initial_scrollbar_slider_top +
                                        amount_moved;

   assert(pthis->slider_currently_dragging_subtree != NULL);
   (void)pthis->slider_currently_dragging_subtree->
	   rigListboxScrollbarSliderTopPix(pthis->consts, pthis->slider_scrollbar_left,
					   pthis->slider_scrollbar_top,
					   pthis->slider_scrollbar_bottom,
					   newScrollBarSliderTopPix,
					   true // redraw now
					   );
}

void whereAxis::sliderButtonRelease(ClientData cd, XEvent *eventPtr) {
   assert(eventPtr->type == ButtonRelease);
   whereAxis *pthis = (whereAxis *)cd;
      
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

void whereAxis::addItem(const string &newName,
			resourceHandle parentUniqueId,
			resourceHandle newNodeUniqueId,
			bool rethinkGraphicsNow,
			bool resortNow) {
   whereAxisRootNode tempRootNode(newNodeUniqueId, newName);
   where4tree<whereAxisRootNode> *newNode = new where4tree<whereAxisRootNode>(tempRootNode);
   assert(newNode);

   assert(hash.defines(parentUniqueId));
   where4tree<whereAxisRootNode> *parentPtr = hash[parentUniqueId];
   assert(parentPtr != NULL);

   parentPtr->addChild(newNode, false, // not explicitly expanded
		       consts,
		       rethinkGraphicsNow,
		       resortNow);
   newNode->setParent(parentPtr);

   // keep access to the new node if it is a hierarchy root,
   // and make sure that it is kept in order of insertion
   // (this order must match the ordering of the ordering of
   // creation of the resource hierarchy roots, since the PC
   // relies on this ordering)
   if( parentPtr == rootPtr )
   {
        hierarchyRoots += newNode;
   }

   assert(!hash.defines(newNodeUniqueId));
   hash[newNodeUniqueId] = newNode;
   assert(hash.defines(newNodeUniqueId));
}

void whereAxis::retireItem(resourceHandle uniqueId) {
   assert(hash.defines(uniqueId));
   where4tree<whereAxisRootNode> *itemPtr = hash[uniqueId];
   assert(itemPtr != NULL);
   itemPtr->getNodeData().mark_as_retired();
}

#ifndef PARADYN
// only the where axis test program uses this stuff:
int whereAxis::readTree(ifstream &is,
			resourceHandle parentUniqueId,
			resourceHandle nextUniqueIdToUse) {
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

      whereAxisRootNode tempRootNode(nextUniqueIdToUse, rootString);
      where4tree<whereAxisRootNode> *newParentNode = new where4tree<whereAxisRootNode> (tempRootNode);

      assert(newParentNode);

      assert(nextUniqueIdToUse==0);
      assert(!hash.defines(nextUniqueIdToUse));
      hash[nextUniqueIdToUse] = newParentNode;
      assert(hash.defines(nextUniqueIdToUse));

      rootPtr = newParentNode;
   }
   else {
      this->addItem(rootString, parentUniqueId, nextUniqueIdToUse, false, false);
      // don't redraw; don't resort
   }
   

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
   where4tree<whereAxisRootNode> *theParentNode = hash[parentUniqueId];

   theParentNode->doneAddingChildren(consts, true); // true --> resort

   // eat the closing )
   is >> c;
   if (c != ')') {
      cerr << "expected ) to close tree, but found " << c << endl;
      return 0;
   }

   return result;
}

// only the where axis test program gets this routine
whereAxis::whereAxis(ifstream &infile, Tcl_Interp *in_interp,
		     Tk_Window theTkWindow,
		     const char *iHorizSBName, const char *iVertSBName,
		     const char *iNavigateMenuName) :
                              consts(in_interp, theTkWindow),
			      hash(hashFunc),
                              horizSBName(iHorizSBName),
                              vertSBName(iVertSBName),
                              navigateMenuName(iNavigateMenuName) {
   initializeStaticsIfNeeded();

   interp = in_interp;
   horizScrollBarOffset = vertScrollBarOffset = 0;

   resourceHandle rootNodeUniqueId = 0;
   const int result = readTree(infile, rootNodeUniqueId, rootNodeUniqueId);
   assert(result > 0);

   beginSearchFromPtr = NULL;  

   rethink_nominal_centerx();
}
#endif

void whereAxis::draw(bool doubleBuffer,
		     bool xsynchronize // are we debugging?
		     ) const {
   Drawable theDrawable = (doubleBuffer && !xsynchronize) ? consts.offscreenPixmap :
                                                            Tk_WindowId(consts.theTkWindow);

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

   const int overallWindowBorderPix = 0;

   if (rootPtr)
      rootPtr->draw(consts.theTkWindow, consts, theDrawable,
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
                Tk_WindowId(consts.theTkWindow), // dest pixmap
                consts.erasingGC, // ?????
                0, 0, // source x,y pix
                Tk_Width(consts.theTkWindow),
                Tk_Height(consts.theTkWindow),
                0, 0 // dest x,y offset pix
                );
}

void whereAxis::resize(bool currentlyDisplayedAbstraction) {
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

void whereAxis::map_from_callgraph(resourceHandle select_handle,bool ishighlight)
{
	unsigned i=0;
	
   	where4tree<whereAxisRootNode> *select_node = hash[select_handle];
	assert(select_node != NULL);

	bool inList = false;
        whereNodePosRawPath theClickPath;

	where4tree<whereAxisRootNode> *cur_ptr = select_node;
	while (cur_ptr != rootPtr)
	{
		where4tree<whereAxisRootNode> *cur_parent = cur_ptr->getParent();
		for (unsigned i=0;i < cur_parent->getNumChildren(); i++)
			if (cur_parent->getChildTree(i) == cur_ptr)
			{
				theClickPath.append(i);
				break;
			}
		cur_ptr = cur_parent;
	}

        whereNodePosRawPath tempClickPath;
	unsigned path_size = theClickPath.getSize();
	for (i=0; i < path_size; i++)
		tempClickPath.append(theClickPath[path_size-1-i]);

	lastClickPath = tempClickPath;
	
	cur_ptr = rootPtr;
   	for (i=0; i < lastClickPath.getSize()-1; i++)
	{
		bool allChildrenExpanded = (cur_ptr->getListboxPixWidth() == 0);
		
		if (!allChildrenExpanded && !cur_ptr->getChildIsExpandedFlag(lastClickPath[i]))
		{
			inList = true;
			break;
		}

      		cur_ptr = cur_ptr->getChildTree(lastClickPath[i]);
	}

	if (inList == false)
      		(void)softScrollToEndOfPath(lastClickPath);
   	else {
      		bool aflag;
      		aflag = (forciblyScrollToEndOfPath(lastClickPath));
      		// rethinks nominal centerx, resizes scrollbars, etc.
      		assert(aflag);
   	}

   	const int overallWindowBorderPix = 0;
	const int root_centerx = nominal_centerx + horizScrollBarOffset;
      	// relative (not absolute) coord.  note: horizScrollBarOffset <= 0
   	const int root_topy = overallWindowBorderPix + vertScrollBarOffset;
      	// relative (not absolute) coord.  note: vertScrollBarOffset <= 0

   	whereNodeGraphicalPath<whereAxisRootNode> thePath(lastClickPath, consts, rootPtr,root_centerx, root_topy);

	switch (thePath.whatDoesPathEndIn()) {
      		case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode:{
         		lastClickPath = thePath.getPath();
         		rethinkNavigateMenu();

         		// Now redraw the node in question...(its highlightedness changed)
         		where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);
        	 	ptr->set_highlight(ishighlight);
         		ptr->getNodeData().drawAsRoot(consts.theTkWindow,
				       Tk_WindowId(consts.theTkWindow),
				       thePath.get_endpath_centerx(),
				       thePath.get_endpath_topy());
			break;
      		}
      		case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem:{
         		lastClickPath = thePath.getPath();
         		rethinkNavigateMenu();

		         // Now we have to redraw the item in question...(its highlightedness changed)
         		thePath.getLastPathNode(rootPtr)->set_highlight(ishighlight);

         		thePath.getParentOfLastPathNode(rootPtr)->draw(consts.theTkWindow,
				consts, Tk_WindowId(consts.theTkWindow),
				thePath.get_endpath_centerx(),
				thePath.get_endpath_topy(),
				false, // not root only
				true // listbox only
				);
         		break;
		}
		default: break;
	}

	initiateWhereAxisRedraw(interp,true);
}
void whereAxis::processSingleClick(int x, int y) {
   whereNodeGraphicalPath<whereAxisRootNode> thePath=point2path(x, y);


   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<whereAxisRootNode>::Nothing:
//         cout << "single-click in nothing at (" << x << "," << y << ")" << endl;
         return;
      case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode: {
//         cout << "click on an non-listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();
         rethinkNavigateMenu();

         // Now redraw the node in question...(its highlightedness changed)
         where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);
         if(ptr->getNodeData().is_retired())
            return;

         ptr->toggle_highlight();

         ptr->getNodeData().drawAsRoot(consts.theTkWindow,
                                       Tk_WindowId(consts.theTkWindow),
                                       thePath.get_endpath_centerx(),
                                       thePath.get_endpath_topy());

	 bool ishighlight=ptr->isHighlighted();
	 map_to_CallGraph(ptr->getNodeData().getUniqueId(),ishighlight);
         return;
      }
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem:{
//         cout << "click on a listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();
         rethinkNavigateMenu();

         // Now we have to redraw the item in question...(its highlightedness changed)
         where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);
         if(ptr->getNodeData().is_retired())
            return;

         ptr->toggle_highlight();
         
         thePath.getParentOfLastPathNode(rootPtr)->draw(consts.theTkWindow,
                               consts, Tk_WindowId(consts.theTkWindow),
                               thePath.get_endpath_centerx(), 
                               thePath.get_endpath_topy(),
                               false, // not root only
                               true // listbox only
                               );
         
	 bool ishighlight=ptr->isHighlighted();
	 map_to_CallGraph(ptr->getNodeData().getUniqueId(),ishighlight);

         return;
	}
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPagedown:
         processNonSliderButtonPress(thePath);
         return;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarSlider: {
//         cout << "looks like a click in a listbox scrollbar slider" << endl;

         where4tree<whereAxisRootNode> *parentPtr = thePath.getLastPathNode(rootPtr);

         slider_initial_yclick = y;
         slider_currently_dragging_subtree = parentPtr;

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

         slider_scrollbar_left = thePath.get_endpath_centerx() -
	                         parentPtr->horiz_pix_everything_below_root(consts) / 2;
         slider_scrollbar_top = thePath.get_endpath_topy() +
                                parentPtr->getNodeData().getHeightAsRoot() +
				consts.vertPixParent2ChildTop;
         slider_scrollbar_bottom = slider_scrollbar_top +
                                   parentPtr->getListboxActualPixHeight() - 1;

//         cout << "slider click was on subtree whose root name is "
//              << parentPtr->getRootName() << endl;

         Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       sliderButtonRelease,
			       this);
	 Tk_CreateEventHandler(consts.theTkWindow,
			       PointerMotionMask,
			       sliderMouseMotion,
			       this);
         break;
      }
      default:
         assert(false);
   }
}

/* ***************************************************************** */

bool whereAxis::processCtrlClick(int x, int y,numlist &select_focus) {
   whereNodeGraphicalPath<whereAxisRootNode> thePath=point2path(x, y);


   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<whereAxisRootNode>::Nothing:
//         cout << "single-click in nothing at (" << x << "," << y << ")" << endl;
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode: {
//         cout << "click on an non-listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();

         // Now redraw the node in question...(its highlightedness changed)
         where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);
         ptr->getNodeData().drawAsRoot(consts.theTkWindow,
				       Tk_WindowId(consts.theTkWindow),
				       thePath.get_endpath_centerx(),
				       thePath.get_endpath_topy());

         select_focus = getCurFocus(thePath);
	 return true;
      }
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem:{
//         cout << "click on a listbox item; adjusting NAVIGATE menu..." << endl;
         lastClickPath = thePath.getPath();

         thePath.getParentOfLastPathNode(rootPtr)->draw(consts.theTkWindow,
							consts, Tk_WindowId(consts.theTkWindow),
							thePath.get_endpath_centerx(),
							thePath.get_endpath_topy(),
							false, // not root only
							true // listbox only
							);
         select_focus = getCurFocus(thePath);
	 return true;
	}
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPagedown:
         processNonSliderButtonPress(thePath);
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarSlider: {
//         cout << "looks like a click in a listbox scrollbar slider" << endl;

         where4tree<whereAxisRootNode> *parentPtr = thePath.getLastPathNode(rootPtr);

         slider_initial_yclick = y;
         slider_currently_dragging_subtree = parentPtr;

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

         slider_scrollbar_left = thePath.get_endpath_centerx() -
	                         parentPtr->horiz_pix_everything_below_root(consts) / 2;
         slider_scrollbar_top = thePath.get_endpath_topy() +
                                parentPtr->getNodeData().getHeightAsRoot() +
				consts.vertPixParent2ChildTop;
         slider_scrollbar_bottom = slider_scrollbar_top +
                                   parentPtr->getListboxActualPixHeight() - 1;

//         cout << "slider click was on subtree whose root name is "
//              << parentPtr->getRootName() << endl;

         Tk_CreateEventHandler(consts.theTkWindow,
			       ButtonReleaseMask,
			       sliderButtonRelease,
			       this);
	 Tk_CreateEventHandler(consts.theTkWindow,
			       PointerMotionMask,
			       sliderMouseMotion,
			       this);
         break;
      }
      default:
         assert(false);
   }
   return false;
}

/* ***************************************************************** */


bool whereAxis::processShiftDoubleClick(int x, int y) {
   // returns true iff a complete redraw is called for

   whereNodeGraphicalPath<whereAxisRootNode> thePath = point2path(x, y);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<whereAxisRootNode>::Nothing:
//         cout << "shift-double-click in nothing" << endl;
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem:
//         cout << "shift-double-click in lb item; ignoring" << endl;
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode:
         break; // some breathing room for lots of code to follow...
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPagedown:
         // in this case, do the same as a single-click
         processNonSliderButtonPress(thePath);
         return false; // no need to redraw further
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarSlider:
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
   where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);
   ptr->toggle_highlight(); // doesn't redraw

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

bool whereAxis::processCtrlDoubleClick(int x, int y) {
   // returns true iff changes were made

   whereNodeGraphicalPath<whereAxisRootNode> thePath = point2path(x, y);

   if (thePath.whatDoesPathEndIn() != whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode)
      return false;

//   cout << "ctrl-double-click:" << endl;

   // How do we know whether to select-all or unselect-all?
   // Well, if noone is selected, then we should _definitely_ select all.
   // And, if everyone is selected, then we should _definitely_ unselect all.
   // But, if _some_ items are selected, what should we do?  It could go either way.
   // For now, we'll say unselect.

   where4tree<whereAxisRootNode> *ptr = thePath.getLastPathNode(rootPtr);

   // change highlightedness of the root node (i.e. the double-click should undo
   // the effects of the earlier single-click, now that we know that a double-click
   // was the user's intention all along)
   ptr->toggle_highlight(); // doesn't redraw

   if (ptr->getNumChildren()==0)
      return true; // changes were made

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

bool whereAxis::processDoubleClick(int x, int y) {
   // returns true iff a complete redraw is called for

   bool scrollToWhenDone = false; // for now...

   whereNodeGraphicalPath<whereAxisRootNode> thePath = point2path(x, y);

   switch (thePath.whatDoesPathEndIn()) {
      case whereNodeGraphicalPath<whereAxisRootNode>::Nothing:
//         cout << "looks like a double-click in nothing" << endl;
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarUpArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarDownArrow:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPageup:
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarPagedown:
         // in this case, do the same as a single-click
         processNonSliderButtonPress(thePath);
         return false; // no need to redraw further
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxScrollbarSlider:
//         cout << "double-click in a listbox scrollbar slider...doing nothing" << endl;
         return false;
      case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode: {
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

         // Now let's scroll to the un-expanded item.
         rethink_nominal_centerx();
         resizeScrollbars();
         adjustHorizSBOffset();
         adjustVertSBOffset();
         softScrollToEndOfPath(thePath.getPath());

	 where4tree<whereAxisRootNode> *ptr=thePath.getLastPathNode(rootPtr);
	 bool ishighlight=ptr->isHighlighted();
	 map_to_CallGraph(ptr->getNodeData().getUniqueId(),ishighlight);
         return true;
      }
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem: {
         // double-click in a listbox item
//         cout << "double-click on a listbox item" << endl;
         // first thing's first: now that we know the user intended to do a double-click
         // all along, we should undo the effects of the single-click which came earlier.
         thePath.getLastPathNode(rootPtr)->toggle_highlight(); // doesn't redraw

         const bool anyChanges = rootPtr->path2lbItemExpand(consts,
							    thePath.getPath(), 0);

         if (!anyChanges) {
            // The only real change we made was the toggle_highlight().  This is
            // a case where we can do the redrawing ourselves (and fast).
            thePath.getParentOfLastPathNode(rootPtr)->
	         draw(consts.theTkWindow, consts, Tk_WindowId(consts.theTkWindow),
		      thePath.get_endpath_centerx(),
		      thePath.get_endpath_topy(),
		      false, // not root only
		      true // listbox only
		      );

	     where4tree<whereAxisRootNode> *ptr=thePath.getLastPathNode(rootPtr);
	     bool ishighlight=ptr->isHighlighted();
	     map_to_CallGraph(ptr->getNodeData().getUniqueId(),ishighlight);
             return false;
         }
         else {
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
      const int overallWindowBorderPix = 0;

      whereNodeGraphicalPath<whereAxisRootNode>
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
	                             path_to_scroll_to.getLastPathNode(rootPtr)->getNodeData().getHeightAsRoot() / 2;

      (void)set_scrollbars(newlyExpandedElemCenterX, x,
			   newlyExpandedElemMiddleY, y,
			   true);
   }

   return true;
}

bool whereAxis::softScrollToPathItem(const whereNodePosRawPath &thePath,
				     unsigned index) {
   // scrolls s.t. the (centerx, topy) of path index is in middle of screen.

   whereNodePosRawPath newPath = thePath;
     // make a copy, because we're gonna hack it up a bit.
   newPath.rigSize(index);
   return softScrollToEndOfPath(newPath);
}

bool whereAxis::forciblyScrollToPathItem(const whereNodePosRawPath &thePath,
					 unsigned pathLen) {
   // Simply a stub which generates a new path and calls forciblyScrollToEndOfPath()   
   // "index" indicates the length of the new path; in particular, 0 will give
   // and empty path (the root node).

   // make a copy of "thePath", truncate it to "pathLen", and forciblyScrollToEndOfPath
   whereNodePosRawPath newPath = thePath;
   newPath.rigSize(pathLen);
   return forciblyScrollToEndOfPath(newPath);
}

bool whereAxis::softScrollToEndOfPath(const whereNodePosRawPath &thePath) {
   const int overallWindowBorderPix = 0;
   whereNodeGraphicalPath<whereAxisRootNode>
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
      case whereNodeGraphicalPath<whereAxisRootNode>::ExpandedNode: {
//         cout << "soft scrolling to expanded node" << endl;
         const int last_item_middley =
            last_item_topy +
            scrollToPath.getLastPathNode(rootPtr)->getNodeData().getHeightAsRoot() / 2;
         return set_scrollbars(last_item_centerx,
			       Tk_Width(consts.theTkWindow) / 2,
			       last_item_middley,
			       Tk_Height(consts.theTkWindow) / 2,
			       true);
      }
      case whereNodeGraphicalPath<whereAxisRootNode>::ListboxItem: {
//         cout << "soft scrolling to lb item" << endl;

         // First, let's scroll within the listbox (no redrawing yet)
         where4tree<whereAxisRootNode> *parent = scrollToPath.getParentOfLastPathNode(rootPtr);

         Tk_FontMetrics lbFontMetrics; // filled in by Tk_GetFontMetrics()
         Tk_GetFontMetrics(consts.listboxFontStruct, &lbFontMetrics);
         const unsigned itemHeight = consts.listboxVertPadAboveItem +
                                     lbFontMetrics.ascent +
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

bool whereAxis::forciblyScrollToEndOfPath(const whereNodePosRawPath &thePath) {
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

void whereAxis::navigateTo(unsigned pathLen) {
   (void)forciblyScrollToPathItem(lastClickPath, pathLen);
}

int whereAxis::find(const string &str) {
   // does a blind search for the given string.  Expands things along the
   // way if needed.  Returns 0 if not found, 1 if found & no expansions
   // were needed, 2 if found & expansion(s) _were_ needed

   // Our strategy: (1) make a path out of the string.
   //               (2) call a routine that "forcefully" scrolls to the end of that
   //                   path.  We say "forcefully" beceause this routine will
   //                   expand items along the way, if necessary.

   // Uses and alters "beginSearchFromPtr"

   whereNodePosRawPath thePath;
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
//   where4tree<whereAxisRootNode> *beginSearchFromPtr = rootPtr;
   beginSearchFromPtr = rootPtr;
   for (unsigned i=0; i < thePath.getSize(); i++)
      beginSearchFromPtr = beginSearchFromPtr->getChildTree(thePath[i]);

   if (result==1)
      (void)softScrollToEndOfPath(thePath);
   else {
      assert(result==2);
      bool aflag;
      aflag = (forciblyScrollToEndOfPath(thePath));
      // rethinks nominal centerx, resizes scrollbars, etc.
      assert(aflag);
   }

   return result;
}

bool whereAxis::adjustHorizSBOffset(float newFirst) {
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

bool whereAxis::adjustHorizSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw.  Returns true iff any changes.

   const int widthOfEverything = rootPtr->entire_width(consts);
   float newFirst = (float)(-horizScrollBarOffset + deltapix) / widthOfEverything;
   return adjustHorizSBOffset(newFirst);
}

//template <class NODEDATA>
//bool whereAxis<NODEDATA>::adjustHorizSBOffsetFromDeltaPages(int deltapages) {
//   // does not redraw.  Returns true iff any changes.
//
//   // First, update the tk scrollbar
//   const int widthOfEverything = rootPtr->entire_width(consts);
//   const int widthOfVisible = Tk_Width(consts.theTkWindow);
//   float newFirst = (float)(-horizScrollBarOffset + widthOfVisible*deltapages) /
//                     widthOfEverything;
//   return adjustHorizSBOffset(newFirst);
//}

bool whereAxis::adjustHorizSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, horizSBName, first, last);

   return adjustHorizSBOffset(first);
}

bool whereAxis::adjustVertSBOffset(float newFirst) {
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

bool whereAxis::adjustVertSBOffsetFromDeltaPix(int deltapix) {
   // does not redraw.  Returns true iff any changes were made.

   const int heightOfEverything = rootPtr->entire_height(consts);
   float newFirst = (float)(-vertScrollBarOffset + deltapix) / heightOfEverything;
   return adjustVertSBOffset(newFirst);
}

//template <class NODEDATA>
//bool whereAxis<NODEDATA>::adjustVertSBOffsetFromDeltaPages(int deltapages) {
//   // does not redraw
//
//   // First, update the tk scrollbar
//   const int heightOfEverything = rootPtr->entire_height(consts);
//   const int heightOfVisible = Tk_Height(consts.theTkWindow);
//   float newFirst = (float)(-vertScrollBarOffset + heightOfVisible*deltapages) /
//                    heightOfEverything;
//   return adjustVertSBOffset(newFirst);
//}

bool whereAxis::adjustVertSBOffset() {
   // Does not redraw.  Obtains PixFirst from actual tk scrollbar.
   float first, last; // fractions (0.0 to 1.0)
   getScrollBarValues(interp, vertSBName, first, last);

   return adjustVertSBOffset(first);
}

/* ************************************************************************ */

void whereAxis::rethinkNavigateMenu() {
   // We loop through the items of "lastClickPath".  For each item,
   // we get the name of the root node, and append the full-path entry
   // to the navigate menu.

   where4tree<whereAxisRootNode> *currTree = rootPtr;   

   // Note: in tk4.0, menu indices start at 1, not 0 (0 is reserved for tearoff)
   string commandStr = navigateMenuName + " delete 1 100";
   myTclEval(interp, commandStr);

   string theString;

   unsigned itemlcv = 0;
   while (true) {
      if (itemlcv >= 1)
         theString += "/";
      theString += currTree->getNodeData().getName();

//      cout << "adding " << theString << " to the navigate menu" << endl;

      string commandStr = navigateMenuName + " add command -label {" + theString + "} " +
                          "-command {whereAxisNavigateTo " + string(itemlcv) + "}";
      myTclEval(interp, commandStr);

      if (itemlcv >= lastClickPath.getSize())
         break;

      unsigned theItem = lastClickPath[itemlcv++];
      currTree = currTree->getChildTree(theItem);
   }
}

bool whereAxis::selectUnSelectFromFullPathName(const string &name, bool selectFlag) {
   // returns true iff fountor < pdvector<resourceHandle> > res 
   const char *str = name.c_str();
   if (str == NULL)
      return false;

   return rootPtr->selectUnSelectFromFullPathName(str, selectFlag);
}

pdvector< pdvector<resourceHandle> >
whereAxis::getSelections(bool &wholeProgram,
			 pdvector<unsigned> &wholeProgramFocus) const {
   // returns a vector[num-hierarchies] of vector of selections.
   // The number of hierarchies is defined as the number of children of the
   // root node.  If "Whole Program" was selection, it isn't returned with
   // the main result; it's returned by modifying the 2 params
   const unsigned numHierarchies = rootPtr->getNumChildren();

   pdvector < pdvector<resourceHandle> > result(numHierarchies);

   bool wholeProgramImplicit = true; // so far...

   for (unsigned i=0; i < numHierarchies; i++) {
      where4tree<whereAxisRootNode> *hierarchyRoot = hierarchyRoots[i];
      pdvector <const whereAxisRootNode *> thisHierarchySelections = hierarchyRoot->getSelections();

      if (thisHierarchySelections.size()==0)
         // add hierarchy's root item
         thisHierarchySelections += &hierarchyRoot->getNodeData();
      else
         // since the hierarchy selection was not empty, we do _not_
         // want to implicitly select whole-program
         wholeProgramImplicit = false;

      result[i].resize(thisHierarchySelections.size());
      for (unsigned j=0; j < thisHierarchySelections.size(); j++)
         result[i][j] = thisHierarchySelections[j]->getUniqueId();
   }

   wholeProgram = wholeProgramImplicit || rootPtr->isHighlighted();
   if (wholeProgram) {
      // write to wholeProgramFocus:
      wholeProgramFocus.resize(numHierarchies);
      for (unsigned i=0; i < numHierarchies; i++) {
         where4tree<whereAxisRootNode> *hierarchyRoot = hierarchyRoots[i];
         const whereAxisRootNode &hierarchyRootData = hierarchyRoot->getNodeData();
         unsigned hierarchyRootUniqueId = hierarchyRootData.getUniqueId();
         wholeProgramFocus[i] = hierarchyRootUniqueId;
      }
   }

   return result;
}

numlist whereAxis::getCurFocus(whereNodeGraphicalPath<whereAxisRootNode> thePath) const {
   // returns a vector[num-hierarchies] of vector of selections.
   // The number of hierarchies is defined as the number of children of the
   // root node.  If "Whole Program" was selection, it isn't returned with
   // the main result; it's returned by modifying the 2 params

   const unsigned numHierarchies = rootPtr->getNumChildren();

   numlist result;
   result.resize(numHierarchies);

   if (thePath.getSize() == 0)
   {//wholeProgram is selected
      for (unsigned i=0; i < numHierarchies; i++) {
         where4tree<whereAxisRootNode> *hierarchyRoot = hierarchyRoots[i];
         const whereAxisRootNode &hierarchyRootData = hierarchyRoot->getNodeData();
         unsigned hierarchyRootUniqueId = hierarchyRootData.getUniqueId();
         result[i] = hierarchyRootUniqueId;
      }

      return result;
   }
   
   whereNodePosRawPath clickPath=thePath.getPath();
   string select_hierarchy_name=rootPtr->getChildTree(clickPath[0])->getNodeData().getName();

   for (unsigned i=0; i < numHierarchies; i++) {
      where4tree<whereAxisRootNode> *hierarchyRoot = hierarchyRoots[i];
      whereAxisRootNode *hierarchy_root = &hierarchyRoot->getNodeData();
      if (hierarchy_root->getName() == select_hierarchy_name) {
      	 where4tree<whereAxisRootNode> *ptr=thePath.getLastPathNode(rootPtr);

	 //only add the current ctrl-selected node of this hierarchy
	 result[i] = ptr->getNodeData().getUniqueId();
      }else {
      	 //add the root of this hierarchy into focus
	 result[i] = hierarchy_root->getUniqueId();
      }
   }

   return result;
}

void whereAxis::clearSelections() {
   rootPtr->recursiveClearSelections();
}

void whereAxis::map_to_CallGraph(resourceHandle select_handle,bool ishighlight)
{
	if (theCallGraphPrograms && theCallGraphPrograms->existsCurrent())
      		theCallGraphPrograms->map_from_WhereAxis(select_handle,ishighlight);
}
