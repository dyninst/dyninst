/*
 * Copyright (c) 1996 Barton P. Miller
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

// where4tree.C
// Ariel Tamches

/* $Id: where4tree.C,v 1.24 2002/12/20 07:50:05 jaw Exp $ */

/* ******************************************************************
 *
 * Notes on scrollbars and drawing speed and tk/tcl vs. C:
 *
 * 1. Drawing must be fast because some where axes (although not
 *    the default one) are time-critical since they're rapidly
 *    changing.  (e.g. the SHG of Perf. Cons)
 *
 * 2. Tk scrollbars (in listboxes) are not OK since they're slow
 *    (unless I am mistaken) and the SHG uses them heavily.
 *
 * 3. As always, when we need faster speed, we trade off storage space.
 *    For example, we should be able to eliminate horiz_offset_to_expanded_child()
 *    by adding another integer item to childstruct representing that offset.
 *
 *    We might also store an array (indexed by explicitly expanded child)
 *    holding indexes into theChildren.  This would make walking
 *    through explicitly expanded children O(num-explicitly-expanded-children)
 *    instead of O(numchildren). [note that we don't bother with implicitly
 *    expanded children because they're easy to detect -- if no listbox then
 *    _every_ child is implicitly expanded]
 * 
 * ******************************************************************
 */

#include "minmax.h"
#include "where4tree.h"


/* *****************************************************************************
 *
 * The listbox -- contains the title of each unexpanded subtree, with a
 *    triangle justified with the right box border.
 *    listbox may have a scrollbar on its left side.
 *
 * *****************************************************************************
 */

template <class NODEDATA>
void where4tree<NODEDATA>::removeListbox() {
   listboxPixWidth = listboxFullPixHeight = listboxActualPixHeight = 0;
   theScrollbar.invalidate();
}

template <class NODEDATA>
bool where4tree<NODEDATA>::rethinkListboxAfterResize1(const where4TreeConstants &tc) {
   // Using listboxFullPixHeight and tc.listboxHeightWhereSBappears
   // (and assuming the latter has changed), rethink scrollbar aspects of listbox.
   // Returns true iff any changes.

   if (theChildren.size() == 0)
      return false; // no children --> no listbox needs rethinking

   if (listboxPixWidth == 0)
      return false; // no listbox needs rethinking

   if (listboxFullPixHeight < tc.listboxHeightWhereSBappears)
      if (!theScrollbar.isValid())
         // no SB was up, and no SB is needed now.  The following line
         // might even be unnecessary (but it doesn't hurt):
         listboxActualPixHeight = listboxFullPixHeight;
      else {
         // An SB was up, but need not be up any longer.  The listbox width decreases
         // by the width of a scrollbar, and the listbox actual height is set to
         // listbox full height.
         theScrollbar.invalidate();
         listboxPixWidth -= tc.listboxScrollBarWidth;
         listboxActualPixHeight = listboxFullPixHeight;
      }
   else if (!theScrollbar.isValid()) {
      // no SB was up, but one is needed now
      theScrollbar.validate();
      listboxPixWidth += tc.listboxScrollBarWidth;
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;
   }
   else {
      // An SB was up, and still needs to be up.
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;
      theScrollbar.updateForNewBounds(listboxActualPixHeight-2*tc.listboxBorderPix,
				      listboxFullPixHeight-2*tc.listboxBorderPix);
   }

   return true; // changes were made
}

template <class NODEDATA>
bool where4tree<NODEDATA>::rethinkListboxAfterResize(const where4TreeConstants &tc) {
   // Assuming tc.listboxHeightWhereSBappears has changed (presumably due to a window
   // resize), rethink listbox parameters.
   // IMPORTANT: After subtree root A has a child B whose listbox dimensions have
   //            changed, A's allExpandedChildrenWidthAsDrawn and/or
   //            allExpandedChildrenHeightAsDrawn need updating.
   // Returns true iff any changes were made to us (the subtree root) or any of
   // our descendants (in which case _our_ parent should rethink its
   // allExpandedChildrenWidth/HeightAsDrawn).

   bool anyChildrenHaveChanged = false;

   const unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++) {
      if (getChildTree(i)->rethinkListboxAfterResize(tc))
         anyChildrenHaveChanged = true;
   }

   if (anyChildrenHaveChanged)
      // Fairly expensive routines: O(numchildren); could be optimized to
      // O(num-expanded-children).
      rethink_all_expanded_children_dimensions(tc);

   const bool weHaveChanged = rethinkListboxAfterResize1(tc);
   return anyChildrenHaveChanged || weHaveChanged;
}

template <class NODEDATA>
int where4tree<NODEDATA>::scroll_listbox(const where4TreeConstants &tc,
					 int deltaYpix) {
   // doesn't redraw
   // returns actual scroll amount (perhaps different from deltaYpix due to pinnning)
   assert(theScrollbar.isValid());

   const int oldPixFirst = theScrollbar.getPixFirst();

   const int listboxTotalDataPix = listboxFullPixHeight - 2*tc.listboxBorderPix;
   const int listboxActualDataPixHeight = listboxActualPixHeight - 2*tc.listboxBorderPix;
 
   int newPixFirst = oldPixFirst + deltaYpix;
   ipmax(newPixFirst, 0);
   ipmin(newPixFirst, listboxTotalDataPix - listboxActualDataPixHeight);
   assert(newPixFirst >= 0);
      // <0 implies actualHeight > totalHeight, implying no scrollbar should
      // have been up to begin with!

   if (newPixFirst == oldPixFirst)
      return 0;

   const int actualDeltaY = newPixFirst - oldPixFirst;
   assert(actualDeltaY != 0);

   theScrollbar.setPixFirst(newPixFirst);

   return actualDeltaY;
}

template <class NODEDATA>
bool where4tree<NODEDATA>::scroll_listbox(const where4TreeConstants &tc,
					  int listboxLeft,
					  int listboxTop,
					  int deltaYpix) {
   // returns true iff any changes made.  Redraws.
   const int actualDeltaY = scroll_listbox(tc, deltaYpix);
   if (actualDeltaY == 0)
      return false;

   const int listboxDataLeft = listboxLeft + tc.listboxBorderPix +
                               tc.listboxScrollBarWidth;
   const int listboxDataTop  = listboxTop + tc.listboxBorderPix;

   // Under certain conditions, we must resort to one whole draw_listbox() instead
   // of scrolling and doing a partial draw_listbox().  These conditions are:
   // (1) the entire 'visible' listbox isn't entirely visible on the screen
   //     [i.e a piece of it is scrolled off the screen], or
   // (2) the scroll amount is so high that the XCopyArea() would simply result
   //     in a nop due to clipping.
   const int absDeltaY = (actualDeltaY >= 0) ? actualDeltaY : -actualDeltaY;
   assert(absDeltaY >= 0);

   bool redraw_all = false;

   const int listboxActualDataPixHeight = listboxActualPixHeight -
                                          2 * tc.listboxBorderPix;
   if (absDeltaY > listboxActualDataPixHeight)
      // case (2)
      redraw_all = true;
   else if (listboxLeft < 0)
      // case (1)
      redraw_all = true;
   else if ((listboxLeft + listboxPixWidth - 1) > Tk_Width(tc.theTkWindow) - 1)
      // case (1)
      redraw_all = true;
   else if (listboxTop < 0)
      // case (1)
      redraw_all = true;
   else if ((listboxTop + listboxActualPixHeight - 1) > Tk_Height(tc.theTkWindow) - 1)
      // case (1)
      redraw_all = true;
   else if (tc.obscured)
      // a cheap hack to get around the fact that I can't get tk to recognize
      // GraphicsExpose events; here, we conservatively assume that any time the
      // window is obscured even a little, we're gonna have to redraw the whole listbox
      // due to a graphics-expose.
      redraw_all = true;

   if (redraw_all) {
      draw_listbox(tc.theTkWindow, tc, Tk_WindowId(tc.theTkWindow),
		   listboxLeft, listboxTop, -1, -1);
      return true;
   }

   // Set clipping s.t. the copy stays within the bounds of the listbox data area.

   const int listboxActualDataPixWidth = listboxPixWidth - 2*tc.listboxBorderPix -
                                         tc.listboxScrollBarWidth;

   // This code is new.  By differentiating up from down scrolling,
   // we can do a cheaper XCopyArea instead of an expensive XCopyArea plus
   // a clip and an un-clip.
   if (actualDeltaY < 0) {
      // the up arrow was pressed; we scroll stuff downwards
      XCopyArea(tc.display,
		Tk_WindowId(tc.theTkWindow), // src drawable
		Tk_WindowId(tc.theTkWindow), // dest drawable
		tc.listboxCopyAreaGC,
		listboxDataLeft, // src left
		listboxDataTop,  // src top
		listboxActualDataPixWidth, // width of stuff to copy
		listboxActualDataPixHeight + actualDeltaY, // height of stuff to copy
		listboxDataLeft, // dest left
		listboxDataTop - actualDeltaY
		);
   }
   else {
      // the down arrow was pressed; we scroll stuff upwards
      XCopyArea(tc.display,
		Tk_WindowId(tc.theTkWindow), // src drawable
		Tk_WindowId(tc.theTkWindow), // dest drawable
		tc.listboxCopyAreaGC,
		listboxDataLeft, // src left
		listboxDataTop + actualDeltaY, // src top (note: will be below the data top)
		listboxActualDataPixWidth, // width of stuff to copy
		listboxActualDataPixHeight - actualDeltaY, // height of stuff to copy
		listboxDataLeft, // dest left
		listboxDataTop // dest top
		);
   }

   int redrawLbStart; // data y pixel coord relative to top
   if (actualDeltaY < 0)
      // scroll upwards (e.g. click on up-arrow).  Contents move downwards;
      // hole (to redraw) opens up at the top.
      redrawLbStart = 0;
   else
      // scroll downwards (e.g. click on down-arrow).  Contents move upwards;
      // hole (to redraw) opens up at the bottom.
      redrawLbStart = (listboxActualDataPixHeight - 1) - actualDeltaY + 1;

   draw_listbox(tc.theTkWindow, tc, Tk_WindowId(tc.theTkWindow),
		listboxLeft, listboxTop,
		redrawLbStart, absDeltaY);

   return true;
}

template <class NODEDATA>
void where4tree<NODEDATA>::rethink_listbox_dimensions(const where4TreeConstants &tc) {
   // Recalculates listboxPixWidth, listboxFullPixHeight, listboxActualPixHeight,
   // and theScrollbar.

   // Expensive.  Call only when needed (subtree expansion/unexpansion).
   // Doesn't assume that a listbox _should_ be up; we'll cheerfully set the
   // dimensions to indicate no-listbox-is-up if appropriate.

   // Uses values of getWidthAsListboxItem() to cache away terribly expensive
   // calls to XTextWidth() [can you imagine calling it 1,000 times?  With rbi's
   // CM-Fortran programs, listboxes can easily get that big.]

   // Step 1: width of text items (loop thru non-explicitly-expanded children)
   int maxItemWidth = 0;
   unsigned numItemsInlistbox = 0; // used for height calculation, later on.
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
         continue;

      where4tree<NODEDATA> *childPtr = getChildTree(childlcv);
      if (!childPtr->anything2Draw)
         continue;

      numItemsInlistbox++;

      ipmax(maxItemWidth, childPtr->theNodeData.getWidthAsListboxItem());
   }

   if (maxItemWidth == 0) {
      // No listbox should be up.
      assert(numItemsInlistbox == 0);
      removeListbox();
      return;
   }

   assert(numItemsInlistbox > 0);

   // set preliminary width (we may add a scrollbar momentarily...)
   listboxPixWidth = 2*tc.listboxBorderPix +
                     tc.listboxHorizPadBeforeText +
                     maxItemWidth +
                     tc.listboxHorizPadBeforeTriangle +
                     tc.listboxTriangleWidth +
                     tc.listboxHorizPadAfterTriangle;
                           
   // Step 2: full listbox height
   Tk_FontMetrics listboxFontMetrics;
   Tk_GetFontMetrics(tc.listboxFontStruct, &listboxFontMetrics);
   const int itemHeight = tc.listboxVertPadAboveItem +
                          listboxFontMetrics.ascent +
                          tc.listboxVertPadAfterItemBaseline;
   listboxFullPixHeight = 2*tc.listboxBorderPix +
                          numItemsInlistbox * itemHeight;
   
   // Step 3: Is a scrollbar needed?
   if (listboxFullPixHeight >= tc.listboxHeightWhereSBappears) {
      listboxPixWidth += tc.listboxScrollBarWidth;
      theScrollbar.validate();
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;

      theScrollbar.updateForNewBounds(listboxActualPixHeight - 2*tc.listboxBorderPix,
                                         // actual available data pixels
                                      listboxFullPixHeight - 2*tc.listboxBorderPix
                                         // total data pixels
                                      );
   }
   else {
      // No SB needed; entire listbox is visible
      theScrollbar.invalidate();
      listboxActualPixHeight = listboxFullPixHeight;
   }
}

template <class NODEDATA>
void where4tree<NODEDATA>::drawTriangle(const where4TreeConstants &tc,
					Drawable theDrawable,
					int triangleEndX,
					int currBaseLine) {
   // cost is O(XFillPolygon())
   // This is a fairly generic triangle-drawing routine; hence, I made it static.
   const int triangleStartX = triangleEndX - tc.listboxTriangleWidth + 1;
   const int triangleTopY   = currBaseLine - tc.listboxTriangleHeight + 1;

   XPoint thePoints[4];
   thePoints[0].x = triangleStartX;
   thePoints[0].y = triangleTopY;

   thePoints[1].x = triangleStartX;
   thePoints[1].y = currBaseLine;

   thePoints[2].x = triangleEndX;
   thePoints[2].y = (triangleTopY + currBaseLine) / 2; // average of the two

   XFillPolygon(tc.display, theDrawable,
                tc.listboxTriangleGC,
                thePoints, 3,
                Convex, // no two points inside the polygon will
                        // intersect the boundaries when connected
                CoordModeOrigin);
}

template <class NODEDATA>
void where4tree<NODEDATA>::draw_listbox(Tk_Window theTkWindow,
					const where4TreeConstants &tc,
					Drawable theDrawable, // could be offscreen buff
					int left, int top,
					int datapart_relative_starty,
					int datapart_relative_height) const {
   // Assumes a listbox exists.
   // Given (left, top) of listbox area, draw it (incl. sb, if applicable)
   // listboxPixWidth, listboxFullPixHeight, listboxActualPixHeight,
   // theScrollbar _must_ be up-to-date.

   assert(listboxPixWidth > 0);
   assert(listboxFullPixHeight > 0);
   assert(listboxActualPixHeight > 0);
   assert(existsANonExplicitlyExpandedChild());

   // First, some quick & dirty macroscopic clipping:
   if (left > Tk_Width(theTkWindow)-1) return; // everything's too far right
   if (left + listboxPixWidth - 1 < 0) return; // everything's too far left
   if (top > Tk_Height(theTkWindow)-1) return; // everything's too far down
   if (top + listboxActualPixHeight - 1 < 0) return; // everything's too high

   int datapart_starty = top + tc.listboxBorderPix;
   int datapart_finishy = datapart_starty + listboxActualPixHeight -
                          2*tc.listboxBorderPix-1;
   
   if (datapart_relative_starty != -1 && datapart_relative_height != -1) {
      datapart_starty += datapart_relative_starty;
      datapart_finishy = datapart_starty + datapart_relative_height - 1;
   }

   ipmax(datapart_starty, top + tc.listboxBorderPix);
   ipmin(datapart_finishy, top + listboxActualPixHeight - 1 - tc.listboxBorderPix);

   const bool existsScrollBar = theScrollbar.isValid();
   if (existsScrollBar)
      theScrollbar.draw(theTkWindow, tc.listboxScrollbarBorderNormal, theDrawable,
                        left, top,
                        top+listboxActualPixHeight-1,
                        listboxActualPixHeight-2*tc.listboxBorderPix,
                           // viewable data pix
                        listboxFullPixHeight-2*tc.listboxBorderPix
                           // total data pix
                        );

   // Background (total area of lb, excluding sb)
   const int backgroundWidth = listboxPixWidth -
                               (existsScrollBar ? tc.listboxScrollBarWidth : 0);

   const int backgroundLeft = left + (existsScrollBar ? tc.listboxScrollBarWidth : 0);

   // No need to Fill a rectangle; Draw will do just fine.
   // Why?  Because each individual item is drawn with a Fill.
   Tk_Draw3DRectangle(theTkWindow, theDrawable,
                      tc.listboxBorder,
                      backgroundLeft, top,
                      backgroundWidth, listboxActualPixHeight,
                      tc.listboxBorderPix,
                      TK_RELIEF_RIDGE
                      );

   const int itemBoxLeft = backgroundLeft + tc.listboxBorderPix;
   int currItemBoxTop = top - (existsScrollBar ? theScrollbar.getPixFirst() : 0)
                            + tc.listboxBorderPix;

   Tk_FontMetrics listboxFontMetrics;
   Tk_GetFontMetrics(tc.listboxFontStruct, &listboxFontMetrics);
   
   const int itemBoxWidth = backgroundWidth - tc.listboxBorderPix * 2;
   const int itemBoxHeight = tc.listboxVertPadAboveItem +
                             listboxFontMetrics.ascent +
                             tc.listboxVertPadAfterItemBaseline;

   const int textLeft = itemBoxLeft + tc.listboxHorizPadBeforeText;
   int currBaseline = currItemBoxTop +
                      tc.listboxVertPadAboveItem + 
                      listboxFontMetrics.ascent - 1;

   const int normalItemRelief = TK_RELIEF_RAISED;
   const int highlightedItemRelief = TK_RELIEF_SUNKEN;

   // XSetClipRectangles() of this GC [but there are several of them!] to
   // the rectangle corresponding to the body of this listbox (just inside 3d border)
   XRectangle clipRect={itemBoxLeft,
                        datapart_starty,
                        itemBoxWidth,
                        datapart_finishy - datapart_starty + 1};

   GC lightGC = Tk_3DBorderGC(theTkWindow, tc.listboxBorder, TK_3D_LIGHT_GC);
   GC darkGC = Tk_3DBorderGC(theTkWindow, tc.listboxBorder, TK_3D_DARK_GC);
   GC flatGC = Tk_3DBorderGC(theTkWindow, tc.listboxBorder, TK_3D_FLAT_GC);

#if !defined(i386_unknown_nt4_0)
   XSetClipRectangles(tc.display, lightGC, 0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, darkGC, 0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, flatGC, 0, 0, &clipRect, 1, YXBanded);

   XSetClipRectangles(tc.display, tc.listboxTriangleGC, 0, 0, &clipRect, 1, YXBanded);
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement clipping support (?)
#endif // !defined(i386_unknown_nt4_0)

   NODEDATA::prepareForDrawingListboxItems(tc.theTkWindow, clipRect);
                      
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
         continue;

      const where4tree<NODEDATA> *childPtr = getChildTree(childlcv);
      if (!childPtr->anything2Draw)
         continue;

      if (currItemBoxTop > datapart_finishy)
         break; // this item is too far down; hence, remaining items will be too.

      if (currItemBoxTop + itemBoxHeight - 1 < datapart_starty) {
         currBaseline += itemBoxHeight;
         currItemBoxTop += itemBoxHeight;

         continue; // this item is too high
      }

      // Change the background
      Tk_Fill3DRectangle(theTkWindow, theDrawable,
                         tc.listboxBorder,
                         itemBoxLeft,
                         currItemBoxTop,
                         itemBoxWidth,
                         itemBoxHeight,
                         1, // looks much better than the usual value of 3
                            // (2 also looks pretty good)
                         theChildren[childlcv].theTree->getNodeData().getHighlighted() ?
                            highlightedItemRelief : normalItemRelief
                         );

      NODEDATA &theChildNodeData = theChildren[childlcv].theTree->theNodeData;
      theChildNodeData.drawAsListboxItem(theTkWindow, theDrawable,
					 itemBoxLeft, currItemBoxTop,
					 itemBoxWidth, itemBoxHeight,
					 textLeft, currBaseline);
					    
      // Draw triangle iff this is non-leaf node
      if (theChildren[childlcv].theTree->getNumChildren() > 0) {
         const int triangleEndX = left + listboxPixWidth -
                                  tc.listboxHorizPadAfterTriangle;
         drawTriangle(tc, theDrawable, triangleEndX, currBaseline);
      }

      currBaseline   += itemBoxHeight;
      currItemBoxTop += itemBoxHeight;

      if (currItemBoxTop > Tk_Height(theTkWindow)-1)
         break; // everything else would be too far down
   }

   NODEDATA::doneDrawingListboxItems(tc.theTkWindow);

   // Undo the XSetClipRectangles():
   XSetClipMask(tc.display, flatGC, None);
   XSetClipMask(tc.display, darkGC, None);
   XSetClipMask(tc.display, lightGC, None);

   XSetClipMask(tc.display, tc.listboxTriangleGC, None);
}

template <class NODEDATA>
bool where4tree<NODEDATA>::
rigListboxScrollbarSliderTopPix(const where4TreeConstants &tc,
                                int scrollBarLeft,
                                   // only needed if redrawing
                                int scrollBarTop,
                                int scrollBarBottom,
                                int newScrollBarSliderTopPix,
                                bool redrawNow) {
   // returns true iff any changes were made

   const int listboxTotalDataPix = listboxFullPixHeight -
                                   2*tc.listboxBorderPix;

   const int oldPixFirst = theScrollbar.getPixFirst();

   int newScrollbarPixFirst = theScrollbar.pixFirstFromAbsoluteCoord(
                                     scrollBarTop, scrollBarBottom,
                                     listboxTotalDataPix,
                                     newScrollBarSliderTopPix);

   if (redrawNow)
      return scroll_listbox(tc, scrollBarLeft, scrollBarTop,
			    newScrollbarPixFirst - oldPixFirst);
   else
      return scroll_listbox(tc, newScrollbarPixFirst - oldPixFirst);
}


/* ****************************************************************************
 *
 * The following routines are O(1) because they access private variables
 * which have cached layout parameters.  The flipside is that such variables must
 * be up-to-date.
 *
 * ****************************************************************************
 */
template <class NODEDATA>
int where4tree<NODEDATA>::
horiz_pix_everything_below_root(const where4TreeConstants &tc) const {
   // simply adds up listbox width, all-expanded-children-width, and
   // padding where appropriate.

   int result = listboxPixWidth; // may be 0
   result += allExpandedChildrenWidthAsDrawn; // may be 0
   if (listboxPixWidth > 0 && allExpandedChildrenWidthAsDrawn > 0)
      result += tc.horizPixlistbox2FirstExpandedChild;

   return result;
}

template <class NODEDATA>
int where4tree<NODEDATA>::vert_pix_everything_below_root() const {
   return max(listboxActualPixHeight, allExpandedChildrenHeightAsDrawn);
}

template <class NODEDATA>
int where4tree<NODEDATA>::entire_width(const where4TreeConstants &tc) const {
   return max(horiz_pix_everything_below_root(tc), getNodeData().getWidthAsRoot());
}

template <class NODEDATA>
int where4tree<NODEDATA>::entire_height(const where4TreeConstants &tc) const {
   int result = theNodeData.getHeightAsRoot();

   if (theChildren.size() == 0)
      return result;

   int stuff_below_root = vert_pix_everything_below_root();
      // may be 0 if all children have anything2Draw == false

   if (stuff_below_root > 0) {
      result += tc.vertPixParent2ChildTop;
      result += stuff_below_root;
   }

   return result;
}


/* ****************************************************************************
 *
 * Expanded Children Measurements:
 *
 * ****************************************************************************
 */
template <class NODEDATA>
int where4tree<NODEDATA>::horiz_offset_to_expanded_child(const where4TreeConstants &tc,
							 unsigned childIndex) const {
   // Gives horiz pixel offset (from the left point of the leftmost item being drawn
   // below the root, which is the leftmost part of the scrollbar of the listbox IF a
   // listbox is up; else from the left point of the leftmost child) to the leftmost
   // point of a certain expanded (implicitly or explicitly) child.

   // First, assert that child #childIndex is indeed expanded
   const bool allChildrenExpanded = (listboxPixWidth == 0);
   assert(allChildrenExpanded || theChildren[childIndex].isExplicitlyExpanded);

   // Similarly, assert that the child isn't hidden:
   assert(getChildTree(childIndex)->anything2Draw);

   int result = listboxPixWidth;
   if (listboxPixWidth > 0)
      result += tc.horizPixlistbox2FirstExpandedChild;

   for (unsigned childlcv=0; childlcv < childIndex; childlcv++)
      if (allChildrenExpanded || theChildren[childlcv].isExplicitlyExpanded)
	 if (getChildTree(childlcv)->anything2Draw) {
            result += theChildren[childlcv].theTree->entire_width(tc);
               // a quick routine
            result += tc.horizPixBetweenChildren;
         }

   return result;      
}

template <class NODEDATA>
int where4tree<NODEDATA>::
wouldbe_all_expanded_children_width(const where4TreeConstants &tc) const {
   // Call this (fairly) expensive routine only to update
   // "allExpandedChildrenWidthAsDrawn", which speeds up draw().
   // Time is O(numchildren); could be optimized to O(num-expanded-children)

   // Returns the number of horizontal pixels that would be used up, if 
   // the expanded children of this subtree were drawn right now.

   // Don't have to assume "allExpandedChildrenWidthAsDrawn" for this node being
   // up-to-date (but that vrble _must_ be up-to-date for all descendants of this node).
   // Additionally, each descendant (and this node) must have updated "listboxPixWidth"

   if (theChildren.size() == 0)
      return 0; // what children?

   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
      // no listbox --> everyone's either implicitly or explicitly expanded!
      // (But this doesn't include hidden nodes)

   int result = 0;
   int numExpandedChildren = 0;
   const unsigned numChildren = theChildren.size();

   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++) {
      const where4tree *theChild = getChildTree(childlcv);
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded)
         if (theChild->anything2Draw) {
            result += theChild->entire_width(tc); // cheap
            numExpandedChildren++;
         }
   }

   if (numExpandedChildren > 0)
      result += (numExpandedChildren - 1) * tc.horizPixBetweenChildren;

   return result;
}

template <class NODEDATA>
int where4tree<NODEDATA>::
wouldbe_all_expanded_children_height(const where4TreeConstants &tc) const {
   // Call this (fairly) expensive routine only to update
   // "heightOfAllExpandedChildrenAsDrawn", which speeds up draw().

   // Returns the number of vertical pixels that would be used up, if this subtree's
   // expanded children were drawn right now.  The children of this node must
   // have vrbles up to date s.t. entire_height() returns the right value for them.
   // But don't have to assume "allExpandedChildrenHeightAsDrawn" for this node being
   // up-to-date.

   if (theChildren.size() == 0)
      return 0; // what children?

   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
      // no listbox --> everyone's either implicitly or explicitly expanded!

   int result = 0;
   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded)
         if (getChildTree(childlcv)->anything2Draw)
            ipmax(result, theChildren[childlcv].theTree->entire_height(tc));

   return result;
}


template <class NODEDATA>
void where4tree<NODEDATA>::draw(Tk_Window theTkWindow,
				const where4TreeConstants &tc,
				Drawable theDrawable,
				int middlex, int topy,
				   // describes position of root node
				bool rootOnly, bool listboxOnly) const {
   if (!anything2Draw)
      return;

   const int maxWindowY = Tk_Height(theTkWindow) - 1;
   if (topy > maxWindowY)
      return; // everything is too far down

   if (!listboxOnly)
      theNodeData.drawAsRoot(theTkWindow, theDrawable, middlex, topy);

   if (rootOnly || theChildren.size() == 0)
      return; // no children to draw...

   const int horizPixEverythingBelowRoot = horiz_pix_everything_below_root(tc);
      // a very quick routine

   int rayOriginX = middlex;
   int rayOriginY = topy + theNodeData.getHeightAsRoot(); // don't subtract 1
 
   if (rayOriginY > maxWindowY)
      return; // everything else would be too far down

   int childtopypix = rayOriginY + tc.vertPixParent2ChildTop;

   int currentXpix = middlex - horizPixEverythingBelowRoot / 2;

   // Draw listbox:
   if (listboxPixWidth > 0) {
      assert(existsANonExplicitlyExpandedChild());
      assert(listboxFullPixHeight > 0);
      assert(listboxActualPixHeight > 0);

      // ray from bottom of root (rayOriginX, rayOriginY) to (centerx, topy) of listbox:
      if (!listboxOnly) {
	 GC theGC = NODEDATA::getGCforListboxRay(theNodeData,
						 getChildTree(0)->theNodeData);
         XDrawLine(tc.display, theDrawable, theGC,
                   rayOriginX, rayOriginY,  // (left, top)
                   currentXpix + listboxPixWidth / 2, // destx
                   childtopypix - 1 // desty
                   );
      }

      draw_listbox(tc.theTkWindow, tc, theDrawable,
                   currentXpix, // left
                   childtopypix, // top
                   -1, -1);

      currentXpix += listboxPixWidth + tc.horizPixlistbox2FirstExpandedChild;
   }

   if (listboxOnly)
      return;

   // Now draw expanded children, if any
   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++) {
      if (!allChildrenAreExpanded && !theChildren[childlcv].isExplicitlyExpanded)
         continue; // don't draw this child since it's in the listbox

      if (!getChildTree(childlcv)->anything2Draw)
         continue;

      const where4tree *theChild = theChildren[childlcv].theTree;
      const int childEntireWidthAsDrawn = theChild->entire_width(tc);
         // not expensive

      const int subtree_centerx = currentXpix + childEntireWidthAsDrawn / 2;

      // Ray from (rayOriginX, rayOriginY) to (centerx,topy) of subchild:
      // Beware: X has only a 16-bit notion of x and y fields; we must pin if
      //         any value is too far right.  At first I thought the way to go was to
      //         skip the XDrawLine() if the recursive child's draw() won't draw
      //         anything due to clipping.  But this is too harsh; just because a
      //         child subtree is not visible does NOT mean the connecting line
      //         would be completely invisible!
      //         So how to draw just the necessary portion of the connecting line;
      //         and, how to give it the right "downward angle"?
      // This is our solution:  We draw the connecting line only to the right edge
      // of the screen.  In other words, we draw the connecting line only to
      // x=windowMaxX (a known value) and y=(unknown value; need to determine).
      // Let (x1,y1) be the (known) would-be destination (which we
      // can't draw directly to because of this 16-bit problem).  Then, the
      // line's slope is known: (y1-y0)/(x1-x0).  Using the slope value, we can
      // calculate the unknown y.  The equation is m=(y-y0)/(rightEdge-x0).
      // y is the only unknown; isolate to get y=y0 + m(rightEdge-x0).
      // Draw ray from (rayOriginX, rayOriginY) to (windowMaxX, y); done.

      const int maximus = 32768;

      GC lineGC = NODEDATA::getGCforNonListboxRay(getNodeData(),
						  theChild->getNodeData());

      if (subtree_centerx < maximus)
         XDrawLine(tc.display, theDrawable, lineGC,
		   rayOriginX, rayOriginY,
		   subtree_centerx, childtopypix-1);
      else {
	 // Here's the hairy case, discussed above.
	 double slope = (childtopypix-1) - rayOriginY;
	 slope /= (subtree_centerx - rayOriginX); // any divide by zero possibility?

//            const int rayDestinationX = windowMaxX;
	 const int rayDestinationX = maximus-1;

	 const int rayDestinationY = (int)((double)rayOriginY + slope*(rayDestinationX - rayOriginX));

	 XDrawLine(tc.display, theDrawable, lineGC,
		   rayOriginX, rayOriginY,
		   rayDestinationX, rayDestinationY);
      }

      // Recursively draw the subtree
      theChild->draw(tc.theTkWindow, tc, theDrawable,
		     subtree_centerx, childtopypix,
		     false, // not root only
		     false // not listbox only
		     );
 
      currentXpix += childEntireWidthAsDrawn + tc.horizPixBetweenChildren;
   }
}


/* *******************************************************************
 *
 * Node explicit expansion / un-expansion
 *
 * note: after expansion / un-expansion, you'll probably need to redraw the _entire_
 *       where axis---for for our ascendants.  Why?  Because changing our width changes
 *       the total width of our ascendants (and so on), which means everything needs
 *       recentering.
 *
 * *******************************************************************
 */

template <class NODEDATA>
bool where4tree<NODEDATA>::expandUnexpand1(const where4TreeConstants &tc,
					   unsigned childindex, bool expandFlag,
					   bool rethinkFlag,
					   bool force) {
   // won't expand a leaf node unless "force" is true
   // NOTE: Expanding a child will require rethinking the all-expanded-children
   //       dimensions for ALL ancestors of the expandee.  This routine only takes
   //       care of updating those dimensions for the immediate ancestor (parent)
   //       of the expandee.  YOU MUST MANUALLY HANDLE THE REST OF THE PATH!

   if (theChildren[childindex].isExplicitlyExpanded == expandFlag)
      return false; // nothing would change

   // We don't want to hear about a child that is hidden:
   assert(getChildTree(childindex)->anything2Draw);

   // do not expand a leaf node unless "force" is true
   if (expandFlag)
      if (!force && getChildTree(childindex)->theChildren.size() == 0)
         return false;
      else if (listboxPixWidth == 0) {
         // Already implicitly expanded.  Just make it explicit.
         theChildren[childindex].isExplicitlyExpanded = expandFlag;
         return false;
      }

   theChildren[childindex].isExplicitlyExpanded = expandFlag;

   if (rethinkFlag) {
      // Note: we _don't_ rethink dimensions for the expanded subtree.  It will
      // look just at it looked that last time it was expanded.
      // But we _do_ rethink dimensions for us, the parent.  In particular,
      // the listbox dimensions can completely change; in addition, variables
      // tracking the width and height of all expanded children change.
   
      rethink_listbox_dimensions(tc);
         // an expensive routine, O(numchildren).  Could be optimized if the
         // listbox widths were somehow sorted, because most time is spent
         // looping through the remaining listbox entries to detemine max pix width...
   
      rethink_all_expanded_children_dimensions(tc);
   }

   return true;
}

template <class NODEDATA>
bool where4tree<NODEDATA>::explicitlyExpandSubchild(const where4TreeConstants &tc,
						    unsigned childindex,
						    bool force) {
   // returns true iff any changes are made.  Won't expand a leaf node.
   // NOTE: The same warning of expandUnexpand1() applies -- outside code must
   //       manually rethink_expanded_children_dimensions() for ALL ancestors of
   //       the expandee; this routine only handles the immeidate ancestor (parent).
   return expandUnexpand1(tc, childindex, true, true, force);
}

template <class NODEDATA>
bool where4tree<NODEDATA>::explicitlyUnexpandSubchild(const where4TreeConstants &tc,
						      unsigned childindex) {
   // returns true iff any changes are made
   // NOTE: The same warning of expandUnexpand1() applies -- outside code must
   //       manually rethink_expanded_children_dimensions() for ALL ancestors of
   //       the expandee; this routine only handles the immeidate ancestor (parent).

   const bool allChildrenExpanded = (listboxPixWidth == 0);
   if (!allChildrenExpanded && !theChildren[childindex].isExplicitlyExpanded)
      return false; // this child is already un-expanded

   return expandUnexpand1(tc, childindex, false, true, false);
}

template <class NODEDATA>
bool where4tree<NODEDATA>::expandAllChildren(const where4TreeConstants &tc) {
   // expand _all_ non-leaf non-hidden children.  Returns true iff any changes...

   bool result = false;
   bool anythingLeftInListbox = false;

   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (!getChildTree(childlcv)->anything2Draw)
         continue;

      if (expandUnexpand1(tc, childlcv, true, // expand
			  false, // we'll rethink lb dimensions manually, below
			  false // don't force expansion of leaf items
			  ))
         result = true;
      else
         anythingLeftInListbox = true;
   }

   if (!result)
      return false; // no changes

   if (!anythingLeftInListbox)
      removeListbox(); // don't need a listbox anymore
   else
      rethink_listbox_dimensions(tc);

   rethink_all_expanded_children_dimensions(tc);
      // allExpandedChildrenWidthAsDrawn and allExpandedChildrenHeightAsDrawn

   return true; // changes were made
}

template <class NODEDATA>
bool where4tree<NODEDATA>::unExpandAllChildren(const where4TreeConstants &tc) {
   // returns true iff any changes were made
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++)
      theChildren[childlcv].isExplicitlyExpanded = false;

   // We _definitely_ need a listbox now... (unless all children are hidden I guess)
   rethink_listbox_dimensions(tc);

   // There are no more expanded children:
   // (calling rethink_all_expanded_children_dimensions() would work OK --- since
   //  we have already properly adjusted the listbox --- but, this is quicker)
   allExpandedChildrenWidthAsDrawn  = 0;
   allExpandedChildrenHeightAsDrawn = 0;

   return true;
}

template <class NODEDATA>
void where4tree<NODEDATA>::rethinkAfterResize(const where4TreeConstants &tc,
					      int horizSpaceToWorkWithThisSubtree) {
   const unsigned numChildren = theChildren.size();
   if (numChildren == 0)
      return; // what's to rethink?  nothing!  This is a measly leaf node.

   // If there is a listbox up now, perhaps it doesn't need to stay up
   // (but only in the case that the window was made wider, of course)
   bool existslistbox = (listboxPixWidth > 0);

   if (listboxPixWidth > 0) {
      assert(listboxFullPixHeight > 0);
      assert(listboxActualPixHeight > 0);
      assert(existsANonExplicitlyExpandedChild());
   }
   
   int childrenWidthIfAllExpanded = 0;
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++) {
      where4tree<NODEDATA> *childPtr = getChildTree(childlcv);
      if (!childPtr->anything2Draw) continue;

      childrenWidthIfAllExpanded += childPtr->entire_width(tc) +
	                            tc.horizPixBetweenChildren;
   }

   assert(numChildren > 0);
   if (childrenWidthIfAllExpanded > 0) // it's possible that all nodes were hidden
      childrenWidthIfAllExpanded -= tc.horizPixBetweenChildren;
      // last child has no space after it

   if (existslistbox && childrenWidthIfAllExpanded <= horizSpaceToWorkWithThisSubtree) {
      // All the children fit; no need for a listbox anymore.
      removeListbox();

      rethink_all_expanded_children_dimensions(tc);

      assert(allExpandedChildrenWidthAsDrawn == childrenWidthIfAllExpanded);
   }
   else if (!existslistbox &&
	    childrenWidthIfAllExpanded > horizSpaceToWorkWithThisSubtree) {
      // We now need a listbox.  Explicitly expanded children
      // stay expanded; implicitly expanded children are collapsed into a listbox.

      rethink_listbox_dimensions(tc); // expensive, O(numchildren)
      rethink_all_expanded_children_dimensions(tc);
   }
}


template <class NODEDATA>
int where4tree<NODEDATA>::point2ItemOneStepScrollbar(const where4TreeConstants &tc,
						     int ypix, int scrollbarTop,
						     int scrollbarHeight) const {
   // -3 for a point in listbox scrollbar up-arrow,
   // -4 for point in listbox scrollbar down-arrow,
   // -5 for point in listbox scrollbar pageup-region,
   // -6 for point in listbox scrollbar pagedown-region,
   // -7 for point in listbox scrollbar slider.

   const int arrowHeight = theScrollbar.getArrowPixHeight();

   const int scrollbarBottom = scrollbarTop + scrollbarHeight - 1;

   // Check for top-arrow or bottom-arrow click
   if (ypix >= scrollbarTop && ypix <= scrollbarTop + arrowHeight - 1)
      return -3;

   if (ypix >= scrollbarBottom - arrowHeight + 1 && ypix <= scrollbarBottom)
      return -4;

   // Now, we need to calculate the coordinates of the slider...
   int sliderPixTop;
   int sliderPixHeight;
   theScrollbar.getSliderCoords(scrollbarTop, scrollbarBottom,
                                listboxActualPixHeight-2*tc.listboxBorderPix,
                                listboxFullPixHeight-2*tc.listboxBorderPix,
                                sliderPixTop, // filled in
                                sliderPixHeight // filled in
                                );

   if (ypix < sliderPixTop)
      return -5; // pageup-region
   if (ypix > (sliderPixTop + sliderPixHeight - 1))
      return -6; // pagedown-region

   assert(ypix >= sliderPixTop && ypix <= (sliderPixTop + sliderPixHeight - 1));
   return -7; // in slider
}

template <class NODEDATA>
int where4tree<NODEDATA>::point2ItemOneStep(const where4TreeConstants &tc,
					    int xpix, int ypix,
					    int root_centerx, int root_topy) const {
   // returns -1 for a point in root, 0 thru n-1 for point w/in general
   // range of a child subtree [even if child is in a listbox],
   // -2 for a point on nothing,
   // -3 for a point in listbox scrollbar up-arrow,
   // -4 for point in listbox scrollbar down-arrow,
   // -5 for point in listbox scrollbar pageup-region,
   // -6 for point in listbox scrollbar pagedown-region,
   // -7 for point in listbox scrollbar slider.

   assert(ypix >= 0 && xpix >= 0);
   assert(anything2Draw);

   const int posRelativeToRoot = theNodeData.pointWithinAsRoot(xpix, ypix,
							       root_centerx, root_topy);
   if (posRelativeToRoot == 2)
      return -2; // too high
   if (posRelativeToRoot == 4 || posRelativeToRoot == 5)
      return -2; // good ypix but bad xpix (directly to the left or right of root)
   if (posRelativeToRoot == 1)
      return -1; // bingo; in root node

   assert(posRelativeToRoot == 3); // below root...check listbox & expanded children

   const int childrenTop = root_topy + getNodeData().getHeightAsRoot() +
                           tc.vertPixParent2ChildTop;
   if (ypix < childrenTop)
      return -2; // nothing

   const int horizPixEverythingBelowRoot = horiz_pix_everything_below_root(tc);
      // a quick routine

   int currentX = root_centerx - horizPixEverythingBelowRoot / 2;

   if (xpix < currentX)
      return -2; // too far left for any children
   if (xpix > currentX + horizPixEverythingBelowRoot - 1)
      return -2; // too far right for any children

   const int vertPixEverythingBelowRoot = vert_pix_everything_below_root();

   if (ypix > childrenTop + vertPixEverythingBelowRoot - 1)
      return -2; // too far down for any children

   // Check listbox:
   if (listboxPixWidth > 0) {
      const int listboxRight = currentX + listboxPixWidth - 1;
      
      if (xpix >= currentX && xpix <= listboxRight) {
         // xpix indicates that it can only be a point w/in listbox

         const int listboxBottom = childrenTop + listboxActualPixHeight - 1;

         if (ypix >= childrenTop && ypix <= listboxBottom) {
            // point is in listbox!  But scrollbar part or data part?
            const bool scrollBarIsUp = theScrollbar.isValid();
            if (scrollBarIsUp && xpix < currentX + tc.listboxScrollBarWidth)
               return point2ItemOneStepScrollbar(tc,
                                 ypix, // click point (note that xcoord is not needed)
                                 childrenTop, // scrollbar top
                                 listboxActualPixHeight // scrollbar height
                                                 );
                                                 
            // Wasn't a click in scrollbar.
            int listboxLocalX = xpix - currentX;
            if (scrollBarIsUp)
               listboxLocalX -= tc.listboxScrollBarWidth;
            if (listboxLocalX < tc.listboxBorderPix)
               return -2; // sorry, you clicked on the left border
                 
            int listboxLocalY = ypix - childrenTop;
            if (listboxLocalY < tc.listboxBorderPix)
               return -2; // sorry, you clicked on the top border

            if (scrollBarIsUp)
               listboxLocalY += theScrollbar.getPixFirst();

            assert(listboxLocalX >= 0);
            assert(listboxLocalX < listboxPixWidth -
                                   (scrollBarIsUp ? tc.listboxScrollBarWidth : 0));

            assert(listboxLocalY >= 0);
            assert(listboxLocalY < listboxFullPixHeight);

            return point2ItemWithinListbox(tc, listboxLocalX, listboxLocalY);
         }

         return -2; // correct xpix but wrong ypix for listbox
      }

      // point wasn't in listbox; we'll try the expanded children
      currentX += listboxPixWidth + tc.horizPixlistbox2FirstExpandedChild;
   }

   // Check expanded, non-hidden children (both implicitly and explicitly expanded ones)
   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++) {
      const where4tree<NODEDATA> *childPtr = getChildTree(childlcv);

      if (!childPtr->anything2Draw)
         continue;

      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         const int thisChildEntireWidth = childPtr->entire_width(tc);

         if (xpix >= currentX && xpix <= currentX + thisChildEntireWidth - 1)
            // Judging by x-coords, point can only fall w/in this subtree
            return childlcv;

         currentX += thisChildEntireWidth + tc.horizPixBetweenChildren;
         if (xpix < currentX)
            return -2;
      }
   }

   return -2;
}

template <class NODEDATA>
int where4tree<NODEDATA>::point2ItemWithinListbox(const where4TreeConstants &tc,
						  int localx, int localy) const {
   // Given point relative to the data part (not scrollbar part) of the 
   // listbox (may assert that a listbox exists for this subtree and
   // that the provided points lie within its bounds), return index of item
   // that was clicked on, or -2 for nothing.

   // called by point2ItemOneStep()

   // Important note: localy is between 0 and (_Full_PixHeight-1).
   //                 That's what I mean by relative.  Scrollbar has already
   //                 taken into account.  Borders have also been taken into account.

   assert(listboxPixWidth > 0);
   assert(listboxFullPixHeight > 0);
   assert(listboxActualPixHeight > 0);

   assert(0 <= localx);
   assert(localx < listboxPixWidth);
   assert(0 <= localy);
   assert(localy < listboxFullPixHeight);

   Tk_FontMetrics listboxFontMetrics;
   Tk_GetFontMetrics(tc.listboxFontStruct, &listboxFontMetrics);
   
   const int itemHeight = tc.listboxVertPadAboveItem +
                          listboxFontMetrics.ascent +
                          tc.listboxVertPadAfterItemBaseline;
   int theRelativeItem = localy / itemHeight;
   assert(theRelativeItem >= 0);

   // If we could find expanded, non-hidden child #i in constant time, then this
   // operation could be made much faster.

   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++) {
      if (!getChildTree(childlcv)->anything2Draw) continue;

      // listbox exists --> _all_ non-explicitly-expanded children are in the listbox.
      if (!theChildren[childlcv].isExplicitlyExpanded)
         if (theRelativeItem == 0)
            return childlcv;
         else
            theRelativeItem--;
   }

   return -2; // nothing
}

template <class NODEDATA>
int where4tree<NODEDATA>::string2Path(whereNodePosRawPath &thePath,
				      const where4TreeConstants &tc,
				      const string &str,
				      where4tree *beginSearchFrom,
				      bool testRootNode) {
   // If not found, thePath is left undefined, and 0 is returned.
   // Otherwise, modifies "thePath" and:
   //    -- returns 1 if no expansions are necessary (can scroll)
   //    -- returns 2 if expansions are necessary before scrolling

   // Depth first search (dive into listbox children, then expanded ones)

   // NOTE: "thePath" must start out initialized as usual...

   bool match = anything2Draw && testRootNode && str.prefix_of(getRootName());
   if (match) {
      // If beginSearchFrom==NULL, then we truly have a match.
      // Else, if beginSearchFrom==this, then set beginSearchFrom to NULL
      //       and continue the search as if no match were made.
      // Else, continue the search as if no match were made.

      if (beginSearchFrom==NULL)
         return 1; // found; no expansion necessary
      else if (beginSearchFrom==this)
         beginSearchFrom=NULL;
      else
         ;
   }

   // Check all children.  In order to set the path correctly, we'll need
   // to distinguish listbox from non-listbox items.
   const bool allChildrenExpanded = (listboxPixWidth == 0);

   // First, the listbox children:
   unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++) {
      if (!getChildTree(i)->anything2Draw) continue;

      const bool childIsInListbox = !allChildrenExpanded &&
                                    !theChildren[i].isExplicitlyExpanded;
      if (childIsInListbox) {
         const string &childName = theChildren[i].theTree->getRootName();
         match = str.prefix_of(childName);
         if (match)
            if (beginSearchFrom==theChildren[i].theTree) {
               match = false;
               beginSearchFrom=NULL;
            }
            else if (beginSearchFrom != NULL)
               match = false;

         thePath.append(i);

         if (match)
            return 1; // found; no exp necessary

         where4tree *theChild = getChildTree(i);

         if (theChild->string2Path(thePath, tc, str, beginSearchFrom, false) != 0)
            return 2; // found; expansion necessary.
         else
            // undo the path item we appended
            thePath.rigSize(thePath.getSize()-1);
      }
   }
 
   // Next, the non-listbox children
   for (unsigned j=0; j < numChildren; j++) {
      if (!getChildTree(j)->anything2Draw) continue;

      const bool childIsExpanded = allChildrenExpanded ||
                                   theChildren[j].isExplicitlyExpanded;
      if (childIsExpanded) {
         thePath.append(j);

         where4tree *theChild = getChildTree(j);

         int result = theChild->string2Path(thePath, tc, str, beginSearchFrom, true);
         if (result == 0)
            // undo the path item which we appended
            thePath.rigSize(thePath.getSize()-1);
         else
            return result;
      }
   }

   return 0; // not found
}

template <class NODEDATA>
int where4tree<NODEDATA>::matchChildRen(whereNodePosRawPath &thePath,
				      const string &str) {
   // If not found, thePath is left undefined, and 0 is returned.
   // Otherwise, modifies "thePath" and:
   //    -- returns 1 if no expansions are necessary (can scroll)
   //    -- returns 2 if expansions are necessary before scrolling

   // NOTE: "thePath" must start out initialized as usual...

   bool match = false;

   // Check all children.  In order to set the path correctly, we'll need
   // to distinguish listbox from non-listbox items.
   const bool allChildrenExpanded = (listboxPixWidth == 0);

   // First, the listbox children:
   unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++) {
      if (!getChildTree(i)->anything2Draw) continue;

         const string &childName = theChildren[i].theTree->getRootName();
         match = str == childName;
	 if (match)
	 {
            thePath.append(i);
            const bool childIsInListbox = !allChildrenExpanded &&
                                    !theChildren[i].isExplicitlyExpanded;
	    if (childIsInListbox)
	    	return 2;
	    else return 1;
	 }
   }
 
   return 0; // not found
}


/* ********************************************************************
 *
 * High-level tree actions (highlighting, expanding) given paths.
 *
 * ********************************************************************
 */

template <class NODEDATA>
bool where4tree<NODEDATA>::path2lbItemExpand(const where4TreeConstants &tc,
					     const whereNodePosRawPath &thePath,
					     unsigned pathIndex) {
   // returns true iff any changes.

   // new feature: whenever a node A is explicitly expanded, so are all of its
   //              ancestors.

   assert(anything2Draw);

   const unsigned pathSize = thePath.getSize();
   assert(pathIndex < pathSize);

   if (pathIndex < pathSize-1) {
      // recurse...then, if changes were made to a descendant, rethink our
      // child params...
      unsigned childnum = thePath[pathIndex];

      bool anyChanges = getChildTree(childnum)->
	                path2lbItemExpand(tc, thePath, pathIndex+1);

      if (explicitlyExpandSubchild(tc, childnum, false)) // false --> no force
         anyChanges = true;

      if (anyChanges)
         rethink_all_expanded_children_dimensions(tc);

      return anyChanges;
   }

   assert(pathIndex == pathSize-1);
   unsigned lastItem = thePath.getLastItem();

   return explicitlyExpandSubchild(tc, lastItem, false); // false --> don't force
}

template <class NODEDATA>
bool where4tree<NODEDATA>::expandEntirePath(const where4TreeConstants &tc,
					    const whereNodePosRawPath &thePath,
					    unsigned index) {
   // Save as path2lbItemExpand, except we won't expand a listbox item at the
   // end of the path -- we just want to "make the entire path visible".
   // Returns true iff any changes were made

   assert(anything2Draw);

   if (thePath.getSize() == 0)
      return false; // we never need to expand the root node
   if (index == thePath.getSize()-1)
      // we never expand the item at the end of the path.
      // Why not?  Well, if it's a listbox item, then we want to keep it there.
      // And, if it's not a listbox item, then by definition it's already expanded.
      return false;
   
   // follow the next link along the path...
   const unsigned childindex = thePath[index];

   const bool allChildrenExpanded = (listboxPixWidth == 0);
   if (!allChildrenExpanded && !theChildren[childindex].isExplicitlyExpanded) {
      // Aha! the next link is in a listbox.  Let's expand him.  BUT, we wait until
      // we finish the recursion first (necessary for correct rethinking of layout).

      assert(getChildTree(childindex)->anything2Draw);

      (void)getChildTree(childindex)->expandEntirePath(tc, thePath, index+1);
         // recurse

      bool aflag;
      aflag=(explicitlyExpandSubchild(tc, childindex, false)); 
      // false --> don't force
      assert(aflag);
         // rethinks stuff, too (like allExpandedChildrenWidthAsDrawn)

      return true; // expansion(s) were made
   }
   else {
      const bool result = getChildTree(childindex)->
	                  expandEntirePath(tc, thePath, index+1);

      if (result)
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }
}

template <class NODEDATA>
bool where4tree<NODEDATA>::path2lbItemUnexpand(const where4TreeConstants &tc,
					       const whereNodePosRawPath &thePath,
					       unsigned pathIndex) {
   // returns true iff any changes were made
   assert(anything2Draw);

   const unsigned pathSize = thePath.getSize();
   if (pathIndex == pathSize-1) {
      // We (i.e., "this") are the _parent_ of the node to un-expand.

      const unsigned childIndex = thePath.getLastItem();

      // Unhighlight the child (doesn't redraw):
      assert(getChildTree(childIndex)->anything2Draw);
      getChildTree(childIndex)->unhighlight();

      // and un-expand (rethinks listbox size and other layout vrbles)
      return explicitlyUnexpandSubchild(tc, childIndex);
   }
   else {
      // recurse...then, if changes were made to a descendant, rethink our
      // child params...
      unsigned thisItem = thePath[pathIndex];
      assert(getChildTree(thisItem)->anything2Draw);
      const bool result = getChildTree(thisItem)->path2lbItemUnexpand
                            (tc, thePath, pathIndex + 1);
      if (result)
         // Changes were made to a descendant of child #theItem.  We
         // need to update some of our internal variables.  Why?  Because the
         // descendant action probably changed some width/height sizes.
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }
}

template <class NODEDATA>
bool where4tree<NODEDATA>::path2ExpandAllChildren(const where4TreeConstants &tc,
						  const whereNodePosRawPath &thePath,
						  unsigned pathIndex) {
   // returns true iff any changes.
   // new feature: in general, when we explicitly expand node A, we also
   //              explicitly expand all of its ancestors.  At first, this seems
   //              silly (how could we have explicitly expanded A if its ancestors
   //              were not visible to begin with?)  The key is that the ancestors
   //              may have been implicitly expanded.

   assert(anything2Draw);
   if (pathIndex < thePath.getSize()) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're expanding all of its children!], then rethink our child size params...
      unsigned childnum = thePath[pathIndex];
      assert(getChildTree(childnum)->anything2Draw);
      bool result = getChildTree(childnum)->path2ExpandAllChildren
                                                (tc, thePath, pathIndex+1);

      if (explicitlyExpandSubchild(tc, childnum, false)) // don't force
         result = true;
      else {
         // we didn't end up expanding anything; however, we still need
         // to rethink our all-expanded-width, etc.
         rethink_all_expanded_children_dimensions(tc);
         result = true;
      }

      return result;
   }

   return expandAllChildren(tc);
}

template <class NODEDATA>
bool where4tree<NODEDATA>::path2UnExpandAllChildren(const where4TreeConstants &tc,
						    const whereNodePosRawPath &thePath,
						    unsigned pathIndex) {
   // returns true iff any changes were made.  Doesn't redraw

   assert(anything2Draw);
   if (pathIndex < thePath.getSize()) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're un-expanding all of its children!], then rethink our child size params...
      unsigned thisItem = thePath[pathIndex];
      assert(getChildTree(thisItem)->anything2Draw);

      const bool result = getChildTree(thisItem)->
                          path2UnExpandAllChildren(tc, thePath, pathIndex+1);

      if (result)
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }

   return unExpandAllChildren(tc);
}


/* ***************************************************************************
 *
 * Constructor, Destructor
 *
 * ***************************************************************************
 */
template <class NODEDATA>
where4tree<NODEDATA>::where4tree(const NODEDATA &iNodeData) :
                                     theNodeData(iNodeData) {
   // theChildren [] is initialized to empty pdvector

   // We'll assume that the new node is a leaf node; hence, the right way to
   // initialize anything2Draw is to simply extract the value from the node-data.
   // Things can change when we actually add this child!
   anything2Draw = theNodeData.anything2draw();
   
   removeListbox();
   
   allExpandedChildrenWidthAsDrawn  = 0;
   allExpandedChildrenHeightAsDrawn = 0;
      // since all that exists of this subtree is the root, so far.
 
   numChildrenAddedSinceLastSort = 0;
}

template <class NODEDATA>
bool where4tree<NODEDATA>::updateAnything2Draw1(const where4TreeConstants &tc) {
   // called by updateAnything2Draw(), below.  Simply a non-recursive version
   // of that routine.
   bool old_anything2Draw = anything2Draw;

   anything2Draw = false; // so far...
   
   // loop thru children; if any of them have anything to draw, then we have
   // something to draw (by definition)
   for (unsigned childlcv=0; childlcv < theChildren.size() && !anything2Draw;
	childlcv++)
      if (getChildTree(childlcv)->anything2Draw)
         anything2Draw = true;

   if (!anything2Draw)
      // Our children have nothing to draw, but we the root node have one last chance:
      anything2Draw = getNodeData().anything2draw();

   if (anything2Draw == old_anything2Draw)
      return false; // nothing changed

   // Stuff changed.  Update spatial variables now!  We assume that our children have
   // already done that; so we just update our (the root node's) vrbles:
   rethink_all_expanded_children_dimensions(tc);
   rethink_listbox_dimensions(tc);
   
   return true;
}

template <class NODEDATA>
bool where4tree<NODEDATA>::updateAnything2Draw(const where4TreeConstants &tc) {
   // Outside code should call this when it has changed the NODEDATA anything2draw()
   // component of one or more nodes.  We will update "anything2Draw" for each node
   // recursively (bottom-up)

   bool anyChanges = false; // so far...

   // First, do the children recursively:
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++)
      if (getChildTree(childlcv)->updateAnything2Draw(tc))
         anyChanges = true;

   // Now, do the root node:
   if (updateAnything2Draw1(tc))
      anyChanges = true;

   return anyChanges;
}

template <class NODEDATA>
where4tree<NODEDATA>::~where4tree() {
   // the children need explicit deleting
   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++) {
      where4tree *theChild = getChildTree(childlcv);
      delete theChild;
   }
}


/* ****************************************************************************
 *
 * Adding children
 * ---
 *     important, crude rule: unless explicitlyExpanded is specified for a
 *     given new child, it will be put into a listbox.
 * ---
 *
 * ****************************************************************************
 */

template <class NODEDATA>
void where4tree<NODEDATA>::addChild(where4tree *theNewChild,
				    bool explicitlyExpanded,
				    const where4TreeConstants &tc,
				    bool rethinkLayoutNow,
				    bool resortNow) {
   // theNewChild _must_ have been created with C++ new -- not negotiable
   // theNewChild _must_ be fully initialized, too.
   // NOTE: In the current implementation, we always put the child into the listbox
   //       unless explicitlyExpanded is true.
   // NOTE: Even if you pass rethinkLayoutNow as true, we only rethink the listbox
   //       dimensions and/or the all-expanded-children dimensions, as needed.
   //       In all likelihood, you'll also need to rethink all-expanded-children
   //       dimensions for all ancestors of this node; you must do this manually.

   const childstruct cs={theNewChild, explicitlyExpanded};
   this->theChildren += cs;
   this->numChildrenAddedSinceLastSort++;

   if (rethinkLayoutNow && theNewChild->anything2Draw)
      if (explicitlyExpanded)
         // no need to rethink listbox dimensions, but a need to
         // rethink expanded children dimensions
         rethink_all_expanded_children_dimensions(tc);
      else
         rethink_listbox_dimensions(tc);

   if (resortNow)
      this->sortChildren();
}

template <class NODEDATA>
void where4tree<NODEDATA>::doneAddingChildren(const where4TreeConstants &tc,
					      bool sortNow) {
   // Needed only if you call ::addChild() one or more times with
   // rethinkGraphicsNow==false
   if (!updateAnything2Draw1(tc)) { // non-recursive; just affects root node
      // well, nothing changed w.r.t. anything2Draw, so the call didn't
      // update listbox/children dimensions.  Hence, we do the update manually:
      rethink_listbox_dimensions(tc); // this one must come first...
      rethink_all_expanded_children_dimensions(tc);
   }

   if (sortNow)
      sortChildren();
}

template <class NODEDATA>
void where4tree<NODEDATA>::
recursiveDoneAddingChildren(const where4TreeConstants &tc, bool sortNow) {
   unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++)
      getChildTree(i)->recursiveDoneAddingChildren(tc, sortNow);

   doneAddingChildren(tc, sortNow);
}

template <class NODEDATA>
void where4tree<NODEDATA>::sortChildren() {
   // Ideal sort is as follows:
   // 1) if only a handful of items have been added since the last
   //    sort (say, <= 10), then use selection sort.
   // 2) if more than a handful of items have been added since the
   //    last sort, and there were only a handful of items previously
   //    in existence (say, <= 10), then just quicksort the whole thing.
   // 3) else, we have inserted more than a few items (unsorted), and more
   //    than a few items were already there (already sorted).
   //    Quicksort the newly-added items only.  Now we have two sorted
   //    subsections.  Merge those two with a mergesort's "merge".
   //    (But how to do an in-place mergesort?)

   if (numChildrenAddedSinceLastSort > 0 && theChildren.size() > 1)
      theChildren.sort(childstruct::cmpfunc); // Pdvector::sort()

   numChildrenAddedSinceLastSort = 0;
}

template <class NODEDATA>
bool where4tree<NODEDATA>::
selectUnSelectFromFullPathName(const char *name, bool selectFlag) {
   // Use char* instead of string because the string class lacks
   // certain operations such as name++ to strip off 1 character.
   // returns true iff found

   if (name == NULL)
      return false;

   if (!anything2Draw)
      return false;

   if (name[0] != '/') {
      // name does _not_ begin with a slash --> we are expected to
      // match the first part of "name".  And, if there is nothing
      // after the first part, we have a match.  Otherwise, continue...

      const char *firstSlash = strchr(name, '/');
      if (firstSlash == NULL) {
         // There are no slashes.  That means we are at the end.
         // Either name==root-node-name or we return false

         const bool result = (0==strcmp(name, getRootName().c_str()));
         if (result)
            if (selectFlag)
               highlight();
            else
               unhighlight();
         return result;
      }
      else {
         // There is at least one slash --> hence, we are not at the end
         // of the trail.  Let's check to see that the first component
         // (upto but not including the first slash) equals getRootName().
         // If not, we've been barking down the wrong subtree, and we return false.
         // Else, advance "name" to "firstSlash" and continue the walk.

         const unsigned firstPartLen = firstSlash-name;
         if (firstPartLen != getRootName().length())
            return false;

         // okay, lengths match...now to match the actual characters
         if (strstr(name, getRootName().c_str()) != name)
            return false;

         name = firstSlash; // advance to the point where we begin
                            // searching children
      }
   }

   // Name begins with a slash --> search children
   assert(*name == '/');

   name++; // skip by the slash

   for (unsigned i=0; i < theChildren.size(); i++)
      if (theChildren[i].theTree->selectUnSelectFromFullPathName(name, selectFlag))
         return true;

   return false;
}

template <class NODEDATA>
pdvector<const NODEDATA *> where4tree<NODEDATA>::getSelections() const {
   // NOTE: Things would be faster if this function returned void and
   // takes in a reference to a pdvector<NODEDATA*> which is appended to
   // in-place...

   pdvector<const NODEDATA *> result; // initially empty
      
   if (getNodeData().getHighlighted()) {
      const NODEDATA &theNodeData = getNodeData();
      result += &theNodeData;
   }

   for (unsigned i=0; i < theChildren.size(); i++)
      result += theChildren[i].theTree->getSelections();

   return result;
}

template <class NODEDATA>
void where4tree<NODEDATA>::recursiveClearSelections() {
   theNodeData.unhighlight();
  
   unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++)
      theChildren[i].theTree->recursiveClearSelections();
}
