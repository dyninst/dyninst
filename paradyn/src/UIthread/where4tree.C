// where4tree.C
// Ariel Tamches

/* $Log: where4tree.C,v $
/* Revision 1.10  1995/10/17 22:15:12  tamches
/* The templated class has changed from a unique-id class to a
/* full root-node class.
/*
 * Revision 1.9  1995/09/20 01:24:13  tamches
 * Major cleanification; too many things to enumerate.  no path items
 * have negative values.  No more uses of graphical paths.
 *
 * Revision 1.8  1995/09/08  19:51:19  krisna
 * stupid way to avoid the for-scope problem
 *
 * Revision 1.7  1995/08/07 00:02:01  tamches
 * added selectUnSelectFromFullPathName
 *
 * Revision 1.6  1995/08/04  19:17:11  tamches
 * More intelligent where axis resorting by added a
 * numChildrenAddedSinceLastSort field to every node.
 *
 * Changed to Vector::sort(), which uses libc's qsort().
 *
 * Revision 1.5  1995/08/01  23:16:23  tamches
 * Used Tk_3DBorderGC() (newly available tk4.0 routine) for clipping (when scrolling
 * a listbox) instead of peeking into the Border structure.
 *
 * Revision 1.4  1995/07/27  23:27:48  tamches
 * Crash upon sorting huge CMF application mysteriously
 * goes away when quicksort is altered slightly to remove
 * tail recursion.
 *
 * Revision 1.3  1995/07/24  21:35:00  tamches
 * Added sorting.
 * Removed member function addChildren()
 *
 * Revision 1.2  1995/07/18  03:41:22  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:59:03  tamches
 * First version of the new where axis
 *
 */

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
   for (unsigned i=0; i < numChildren; i++)
      if (getChildTree(i)->rethinkListboxAfterResize(tc))
         anyChildrenHaveChanged = true;

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
      return 0;

   const int listboxDataLeft = listboxLeft + tc.listboxBorderPix +
                               tc.listboxScrollBarWidth;
   const int listboxDataTop  = listboxTop + tc.listboxBorderPix;

   // Under certain conditions, we must resort to one whole draw_listbox()
   // instead of scrolling and doing a partial draw_listbox().  These conditions
   // are (1) the entire 'visible' listbox isn't entirely visible on the screen
   // [i.e a piece of it is scrolled off the screen], or (2) the scroll amount is
   // so high that the XCopyArea() would simply result in a nop due to clipping.
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

   if (redraw_all) {
      draw_listbox(tc.theTkWindow, tc, Tk_WindowId(tc.theTkWindow),
		   listboxLeft, listboxTop, -1, -1);
      return true;
   }

   // Set clipping s.t. the copy stays within the bounds of the listbox data area.

   const int listboxActualDataPixWidth = listboxPixWidth - 2*tc.listboxBorderPix -
                                         tc.listboxScrollBarWidth;
   XRectangle clipRect = {listboxDataLeft, listboxDataTop,
			  listboxActualDataPixWidth, listboxActualDataPixHeight};

   XSetClipRectangles(tc.display, tc.erasingGC,
		      0, 0,
		      &clipRect, 1, YXBanded);

   XCopyArea(tc.display,
             Tk_WindowId(tc.theTkWindow), // source drawable
	     Tk_WindowId(tc.theTkWindow), // dest drawable
	     tc.erasingGC, // not many fields of this GC are used; this should do fine
	     listboxDataLeft, // source leftx
	     listboxDataTop,  // source topy
	     listboxActualDataPixWidth,  // width of area to copy
	     listboxActualDataPixHeight, // height of area to copy
	     listboxDataLeft,              // dest leftx
	     listboxDataTop - actualDeltaY // dest topy
	     );

   // undo the clipping change:
   XSetClipMask(tc.display, tc.erasingGC, None);

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

   // Uses values of nameTextWidthIfInListbox to cache away terribly expensive
   // calls to XTextWidth() [can you imagine calling it 1,000 times?  With rbi's
   // CM-Fortran programs, listboxes can easily get that big.]

   // Step 1: width of text items (loop thru non-explicitly-expanded children)
   int maxItemWidth = 0;
   unsigned numItemsInlistbox = 0; // used for height calculation, later on.
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
         continue;

      numItemsInlistbox++;

      ipmax(maxItemWidth, theChildren[childlcv].theTree->theNodeData.getWidthAsListboxItem());
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
   const int itemHeight = tc.listboxVertPadAboveItem +
                          tc.listboxFontStruct->ascent +
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
					int currBaseLine) const {
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

   const int itemBoxWidth = backgroundWidth - tc.listboxBorderPix * 2;
   const int itemBoxHeight = tc.listboxVertPadAboveItem +
                             tc.listboxFontStruct->ascent +
                             tc.listboxVertPadAfterItemBaseline;

   const int textLeft = itemBoxLeft + tc.listboxHorizPadBeforeText;
   int currBaseline = currItemBoxTop +
                      tc.listboxVertPadAboveItem + 
                      tc.listboxFontStruct->ascent - 1;

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
			      
   XSetClipRectangles(tc.display, lightGC, 0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, darkGC, 0, 0, &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, flatGC, 0, 0, &clipRect, 1, YXBanded);

   XSetClipRectangles(tc.display, tc.listboxTriangleGC, 0, 0, &clipRect, 1, YXBanded);

   NODEDATA::prepareForDrawingListboxItems(tc.theTkWindow, clipRect);
                      
   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
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

   result += tc.vertPixParent2ChildTop;

   return result + vert_pix_everything_below_root();
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

   int result = listboxPixWidth;
   if (listboxPixWidth > 0)
      result += tc.horizPixlistbox2FirstExpandedChild;

   for (unsigned childlcv=0; childlcv < childIndex; childlcv++)
      if (allChildrenExpanded || theChildren[childlcv].isExplicitlyExpanded) {
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
   // Additionally, each descent (and this node) must have updated "listboxPixWidth"

   if (theChildren.size() == 0)
      return 0; // what children?

   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
      // no listbox --> everyone's either implicitly or explicitly expanded!

   int result = 0;
   int numExpandedChildren = 0;
   const unsigned numChildren = theChildren.size();

   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++) {
      const where4tree *theChild = getChildTree(childlcv);
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
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
   const int maxWindowY = Tk_Height(theTkWindow) - 1;
   if (topy > maxWindowY)
      return; // everything is too far down

   if (!listboxOnly)
      theNodeData.drawAsRoot(theTkWindow, theDrawable,
			     middlex, topy);

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

//   const unsigned borderPix = 0; // used to be 3

//   const int windowMinX = borderPix;
//   const int windowMaxX = Tk_Width(theTkWindow) - 1 - borderPix;

   // Draw listbox:
   if (listboxPixWidth > 0) {
      assert(existsANonExplicitlyExpandedChild());
      assert(listboxFullPixHeight > 0);
      assert(listboxActualPixHeight > 0);

      // ray from bottom of root (rayOriginX, rayOriginY) to (centerx, topy) of listbox:
      if (!listboxOnly)
         XDrawLine(tc.display, theDrawable, tc.listboxRayGC,
                   rayOriginX, rayOriginY,  // (left, top)
                   currentXpix + listboxPixWidth / 2, // destx
                   childtopypix - 1 // desty
                   );

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
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
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

         if (subtree_centerx < maximus)
            XDrawLine(tc.display, theDrawable, tc.subchildRayGC,
                      rayOriginX, rayOriginY,
                      subtree_centerx, childtopypix-1);
         else {
            // Here's the hairy case, discussed above.
            double slope = (childtopypix-1) - rayOriginY;
            slope /= (subtree_centerx - rayOriginX); // any divide by zero possibility?

//            const int rayDestinationX = windowMaxX;
            const int rayDestinationX = maximus-1;

            const int rayDestinationY = (int)((double)rayOriginY + slope*(rayDestinationX - rayOriginX));

            XDrawLine(tc.display, theDrawable, tc.subchildRayGC,
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

   // do not expand a leaf node
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
   // expand _all_ non-leaf children.  Returns true iff any changes...

   bool result = false;
   bool anythingLeftInListbox = false;

   for (unsigned childlcv=0; childlcv < theChildren.size(); childlcv++) {
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

   // We _definitely_ need a listbox now...
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
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++)
      childrenWidthIfAllExpanded += getChildTree(childlcv)->entire_width(tc) + 
                                    tc.horizPixBetweenChildren;

   assert(numChildren > 0);
   childrenWidthIfAllExpanded -= tc.horizPixBetweenChildren;
   // last child has no space after it (note that numChildren>0, so this is safe)

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

   // Check expanded children (both implicitly- and explicitly- expanded ones)
   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv = 0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         const where4tree *theChild = getChildTree(childlcv);
         const int thisChildEntireWidth = theChild->entire_width(tc);

         if (xpix >= currentX && xpix <= currentX + thisChildEntireWidth - 1)
            // Judging by x-coords, point can only fall w/in this subtree
            return childlcv;

         currentX += thisChildEntireWidth + tc.horizPixBetweenChildren;
         if (xpix < currentX)
            return -2;
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

   const int itemHeight = tc.listboxVertPadAboveItem +
                          tc.listboxFontStruct->ascent +
                          tc.listboxVertPadAfterItemBaseline;
   int theRelativeItem = localy / itemHeight;
   assert(theRelativeItem >= 0);

   // If we could find expanded child #i in constant time, then this operation
   // could be made much faster.

   const unsigned numChildren = theChildren.size();
   for (unsigned childlcv=0; childlcv < numChildren; childlcv++) {
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

   bool match = testRootNode && str.prefix_of(getRootName());
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

   unsigned numChildren = theChildren.size();
   for (unsigned i=0; i < numChildren; i++) {
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

   for (unsigned j=0; j < numChildren; j++) {
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

   // Unhighlight the node    [WHY????????]
   theChildren[lastItem].theTree->getNodeData().unhighlight();
      // doesn't redraw

   return explicitlyExpandSubchild(tc, lastItem, false); // false --> don't force
}

template <class NODEDATA>
bool where4tree<NODEDATA>::expandEntirePath(const where4TreeConstants &tc,
					    const whereNodePosRawPath &thePath,
					    unsigned index) {
   // Save as path2lbItemExpand, except we won't expand a listbox item at the
   // end of the path -- we just want to "make the entire path visible".
   // Returns true iff any changes were made

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

      (void)getChildTree(childindex)->expandEntirePath(tc, thePath, index+1);
         // recurse

      assert(explicitlyExpandSubchild(tc, childindex, false)); // false --> don't force
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
   const unsigned pathSize = thePath.getSize();
   if (pathIndex == pathSize-1) {
      // We (i.e., "this") are the _parent_ of the node to un-expand.

      const unsigned childIndex = thePath.getLastItem();

      // Unhighlight the child (doesn't redraw):
      getChildTree(childIndex)->unhighlight();

      // and un-expand (rethinks listbox size and other layout vrbles)
      return explicitlyUnexpandSubchild(tc, childIndex);
   }
   else {
      // recurse...then, if changes were made to a descendant, rethink our
      // child params...
      unsigned thisItem = thePath[pathIndex];
      const bool result = theChildren[thisItem].theTree->path2lbItemUnexpand
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

   if (pathIndex < thePath.getSize()) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're expanding all of its children!], then rethink our child size params...
      unsigned childnum = thePath[pathIndex];
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

   if (pathIndex < thePath.getSize()) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're un-expanding all of its children!], then rethink our child size params...
      unsigned thisItem = thePath[pathIndex];

      const bool result = theChildren[thisItem].theTree->
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
   // theChildren [] is initialized to empty vector
   
   removeListbox();
   
   allExpandedChildrenWidthAsDrawn  = 0;
   allExpandedChildrenHeightAsDrawn = 0;
      // since all that exists of this subtree is the root, so far.
 
   numChildrenAddedSinceLastSort = 0;
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

   if (rethinkLayoutNow)
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
   rethink_listbox_dimensions(tc); // expensive; cost is O(numchildren)

   rethink_all_expanded_children_dimensions(tc);

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
      theChildren.sort(childstruct::cmpfunc); // Vector::sort()

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

   if (name[0] != '/') {
      // name does _not_ begin with a slash --> we are expected to
      // match the first part of "name".  And, if there is nothing
      // after the first part, we have a match.  Otherwise, continue...

      const char *firstSlash = strchr(name, '/');
      if (firstSlash == NULL) {
         // There are no slashes.  That means we are at the end.
         // Either name==root-node-name or we return false

         const bool result = (0==strcmp(name, getRootName().string_of()));
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
         if (strstr(name, getRootName().string_of()) != name)
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
vector<NODEDATA *> where4tree<NODEDATA>::getSelections() const {
   // NOTE: Things would be faster if this function returned void and
   // takes in a reference to a vector<NODEDATA*> which is appended to
   // in-place...

   vector<NODEDATA *> result; // initially empty
      
   if (getNodeData().getHighlighted())
      result += &getNodeData();

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
