// where4tree.C
// Ariel Tamches

/* $Log: where4tree.C,v $
/* Revision 1.6  1995/08/04 19:17:11  tamches
/* More intelligent where axis resorting by added a
/* numChildrenAddedSinceLastSort field to every node.
/*
/* Changed to Vector::sort(), which uses libc's qsort().
/*
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

#ifdef PARADYN
extern "C" {
void thr_check_stack(); // libthread
}
#endif


#include "minmax.h"
#include "where4tree.h"

// Define static member variables:
// "sorry, not implemented: static data member templates" (g++ 2.6.3), so
// we resort to this hack:

// template<class USERNODEDATA>
// int where4tree<USERNODEDATA>::masterlistboxBorderPix = 3;

// template<class USERNODEDATA>
// int where4tree<USERNODEDATA>::masterlistboxScrollBarWidth = 16;

extern int listboxBorderPix;
extern int listboxScrollBarWidth;


/* *****************************************************************************
 *
 * The listbox -- contains the title of each unexpanded subtree, with a
 *    triangle justified with the right box border.
 *    listbox may have a scrollbar on its left side.
 *
 * *****************************************************************************
 */
template <class USERNODEDATA>
void where4tree<USERNODEDATA>::rethink_listbox_dimensions(const where4TreeConstants &tc) {
   // Recalculates listboxPixWidth, listboxFullPixHeight,
   // listboxActualPixHeight, and theScrollbar.

   // Expensive.  Call only when needed (subtree expansion/unexpansion).
   // Doesn't assume that a listbox _should_ be up; we'll cheerfully set the
   // dimensions to indicate no-listbox-is-up if appropriate.

   // Uses values of rootNameWidthIfInlistbox to cache away terribly expensive
   // calls to XTextWidth() [can you imagine calling it 1,000 times?  With rbi's
   // CM-Fortran programs, listboxes can easily get that big.]

   // Step 1: width of text items (loop thru non-explicitly-expanded children)
   int maxItemWidth = 0;
   int numItemsInlistbox = 0; // used for height calculation, later on.
   for (int childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
         continue;

      numItemsInlistbox++;

      maxItemWidth = max(maxItemWidth, theChildren[childlcv].nameTextWidthIfInListbox);
   }

   if (maxItemWidth == 0) {
      // No listbox is up.
      assert(numItemsInlistbox == 0);
      removeListbox();
      return;
   }

   assert(numItemsInlistbox > 0);

   // set preliminary width (we may add a scrollbar momentarily...)
   listboxPixWidth = 2*listboxBorderPix +
                     tc.listboxHorizPadBeforeText +
                     maxItemWidth +
                     tc.listboxHorizPadBeforeTriangle +
                     tc.listboxTriangleWidth +
                     tc.listboxHorizPadAfterTriangle;
                           
   // Step 2: full listbox height
   const int itemHeight = tc.listboxVertPadAboveItem +
                          tc.listboxFontStruct->ascent +
                          tc.listboxVertPadAfterItemBaseline;
   listboxFullPixHeight = 2*listboxBorderPix +
                                numItemsInlistbox * itemHeight;
   
   // Step 3: Is a scrollbar needed?
   if (listboxFullPixHeight >= tc.listboxHeightWhereSBappears) {
      listboxPixWidth += listboxScrollBarWidth;
      theScrollbar.validate();
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;

      theScrollbar.updateForNewBounds(listboxActualPixHeight-
                                      2*listboxBorderPix,
                                         // actual available data pixels
                                      listboxFullPixHeight-
                                      2*listboxBorderPix
                                         // total data pixels
                                      );
   }
   else {
      // No SB needed; entire listbox is visible
      theScrollbar.invalidate();
      listboxActualPixHeight = listboxFullPixHeight;
   }
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::drawTriangle(const where4TreeConstants &tc,
                                            int theDrawable,
                                            const int triangleEndX,
                                            const int currBaseLine) const {
   // cost is O(XFillPolygon())
   // This is actually a fairly generic triangle-drawing routine.
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
                CoordModeOrigin
                );
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::draw_listbox(const where4TreeConstants &tc,
                                            int theDrawable,
                                            const int left, const int top,
                                            const int datapart_relative_starty,
                                            const int datapart_relative_height) const {
   // Assumes a listbox exists.
   // Given (left, top) of listbox area, listbox (incl. scrollbar, if applicable)
   // is drawn.
   // listboxPixWidth, listboxFullPixHeight, listboxActualPixHeight,
   // theScrollbar _must_ be up-to-date.

   assert(listboxPixWidth > 0);
   assert(listboxFullPixHeight > 0);
   assert(listboxActualPixHeight > 0);
   assert(existsANonExplicitlyExpandedChild());

   // First, some quick & dirty macroscopic clipping:
   if (left > Tk_Width(tc.theTkWindow)-1) return; // everything's too far right
   if (left + listboxPixWidth - 1 < 0) return; // everything's too far left
   if (top > Tk_Height(tc.theTkWindow)-1) return; // everything's too far down
   if (top + listboxActualPixHeight - 1 < 0) return; // everything's too high

   int datapart_starty = top + listboxBorderPix;
   int datapart_finishy = datapart_starty + listboxActualPixHeight -
                          2*listboxBorderPix-1;
   
   if (datapart_relative_starty != -1 && datapart_relative_height != -1) {
      datapart_starty += datapart_relative_starty;
      datapart_finishy = datapart_starty + datapart_relative_height - 1;
   }

   datapart_starty = max(datapart_starty, top + listboxBorderPix);
   datapart_finishy = min(datapart_finishy, top + listboxActualPixHeight - 1 - listboxBorderPix);

   const bool existsScrollBar = theScrollbar.isValid();
   if (existsScrollBar)
      theScrollbar.draw(tc,
                        theDrawable,
                        left, top,
                        top+listboxActualPixHeight-1,
                        listboxActualPixHeight-2*listboxBorderPix,
                           // viewable data pix
                        listboxFullPixHeight-2*listboxBorderPix
                           // total data pix
                        );

   // Background:
   const int backgroundWidth = listboxPixWidth -
                               (existsScrollBar ? listboxScrollBarWidth : 0);
      // everything except scrollbar

   const int backgroundLeft = left +
                              (existsScrollBar ? listboxScrollBarWidth : 0);

   // No need to Fill a rectangle; Draw will do just fine.
   // Why?  Because individual items are drawn with a Fill.
   Tk_Draw3DRectangle(tc.theTkWindow, theDrawable,
                      tc.listboxBorder,
                      backgroundLeft, top,
                      backgroundWidth, listboxActualPixHeight,
                      listboxBorderPix,
                      TK_RELIEF_RIDGE
                      );

   const int itemBoxLeft = backgroundLeft + listboxBorderPix;
   int currItemBoxTop = top - (existsScrollBar ? theScrollbar.getPixFirst() : 0)
                      + listboxBorderPix;

   const int itemBoxWidth = backgroundWidth - listboxBorderPix * 2;
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
   // the rectangle corresponding to the body of this listbox (just inside
   // the 3d border)
   XRectangle clipRect={itemBoxLeft,
                        datapart_starty,
                        itemBoxWidth,
                        datapart_finishy - datapart_starty + 1
                        };

   GC lightGC = Tk_3DBorderGC(tc.theTkWindow, tc.listboxBorder, TK_3D_LIGHT_GC);
   GC darkGC = Tk_3DBorderGC(tc.theTkWindow, tc.listboxBorder, TK_3D_DARK_GC);
   GC flatGC = Tk_3DBorderGC(tc.theTkWindow, tc.listboxBorder, TK_3D_FLAT_GC);
			      
   XSetClipRectangles(tc.display, lightGC,
                      0, 0, // clip origin
                      &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, darkGC,
                      0, 0, // clip origin
                      &clipRect, 1, YXBanded);
   XSetClipRectangles(tc.display, flatGC,
                      0, 0, // clip origin
                      &clipRect, 1, YXBanded);

   XSetClipRectangles(tc.display, tc.listboxTextGC,
                      0, 0, // clip origin
                      &clipRect, 1, YXBanded);
                      
   XSetClipRectangles(tc.display, tc.listboxTriangleGC,
                      0, 0, // clip origin
                      &clipRect, 1, YXBanded);
                      
   for (int childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].isExplicitlyExpanded)
         continue;

      assert(!theChildren[childlcv].isExplicitlyExpanded);

      // Some macroscopic clipping:
      if (currItemBoxTop > datapart_finishy)
         break; // this item is too far down; hence, remaining items will be too.

      if (currItemBoxTop + itemBoxHeight - 1 < datapart_starty) {
         currBaseline += itemBoxHeight;
         currItemBoxTop += itemBoxHeight;

         continue; // this item is too high
      }

      // Draw this child's name
      const string &theString = theChildren[childlcv].theTree->getRootName();
      const char *name = theString.string_of();
      const int len = theString.length();

      // Change the background
      Tk_Fill3DRectangle(tc.theTkWindow, theDrawable,
                         tc.listboxBorder,
                         itemBoxLeft,
                         currItemBoxTop,
                         itemBoxWidth,
                         itemBoxHeight,
                         1, // looks much better than the usual value of 3
                            // (2 also looks pretty good)
                         theChildren[childlcv].theTree->theRootNode.getHighlighted() ?
                            highlightedItemRelief : normalItemRelief
                         );

      XDrawString(tc.display, theDrawable,
                  tc.listboxTextGC,
                  textLeft, currBaseline,
                  name, len);

      // Draw triangle only if this is not a leaf node
      if (theChildren[childlcv].theTree->theChildren.size() > 0) {
         const int triangleEndX = left + listboxPixWidth -
                                  tc.listboxHorizPadAfterTriangle;
         drawTriangle(tc, theDrawable, triangleEndX, currBaseline);
      }

      currBaseline   += itemBoxHeight;
      currItemBoxTop += itemBoxHeight;

      if (currItemBoxTop > Tk_Height(tc.theTkWindow)-1)
         // quick & dirty clipping
         break;
   }

   // Undo the XSetClipRectangles():
   XSetClipMask(tc.display, flatGC, None);
   XSetClipMask(tc.display, darkGC, None);
   XSetClipMask(tc.display, lightGC, None);

   XSetClipMask(tc.display, tc.listboxTextGC, None);
   XSetClipMask(tc.display, tc.listboxTriangleGC, None);
}


/* ****************************************************************************
 *
 * Fast everything-below-root measurements:
 *
 * ****************************************************************************
 */
template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
horiz_pix_everything_below_root(const where4TreeConstants &tc) const {
   // simply adds up listbox width, all-expanded-children-width, and
   // padding where appropriate.  Does _no_ thinking w.r.t. who's displayed.
   // A _very_ quick routine.

   int result = listboxPixWidth; // may be 0
   result += allExpandedChildrenWidthAsDrawn; // may be 0
   if (listboxPixWidth > 0 && allExpandedChildrenWidthAsDrawn > 0)
      result += tc.horizPixlistbox2FirstExpandedChild;

   return result;
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::vert_pix_everything_below_root() const {
   // Assumes all structures are up-to-date.
   // A _very_ quick routine
   int result = listboxActualPixHeight; // may be 0
   result = max(result, allExpandedChildrenHeightAsDrawn);
  
   return result;
}


/* ****************************************************************************
 *
 * Fast entire-subtree measurements:
 *
 * ****************************************************************************
 */
template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
myEntireWidthAsDrawn(const where4TreeConstants &tc) const {
   // If this subtree were drawn right now, this routine returns
   // the number of horizontal pixels that would be consumed.

   // Assumes updated "allExpandedChildrenWidthAsDrawn",
   // explicitlyExpandedChildren[], and listboxWidth for this tree and
   // _all_ descendants.

   // A _very_ quick routine; does not thinking w.r.t. what's displayed

   if (listboxPixWidth > 0)
      assert(existsANonExplicitlyExpandedChild());

   // Consideration #1: is there a listbox?
   int result = listboxPixWidth; // may be 0
   if (result > 0) {
      // padding between listbox & first expanded child:
      if (allExpandedChildrenWidthAsDrawn > 0)
         // there is at least one expanded child (implicit or explicit)
         result += tc.horizPixlistbox2FirstExpandedChild;
   }

   // Consideration #2: children (already in "allExpandedChildrenWidthAsDrawn")
   result += allExpandedChildrenWidthAsDrawn;

   // Consideration #3: root
   result = max(result, theRootNode.getWidth());

   return result;
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
myEntireHeightAsDrawn(const where4TreeConstants &tc) const {
   // If this subtree were drawn right now, this routine returns
   // the number of vertical pixels that would be consumed.

   // Assumes updated "allExpandedChildrenHeightAsDrawn",
   // explicitlyExpandedChildren[], and listboxHeight for this tree and
   // _all_ descendants.

   // A _very_ quick routine; does not thinking w.r.t. that's displayed

   int result = theRootNode.getHeight();
   if (theChildren.size() == 0)
      return result;

   result += tc.vertPixParent2ChildTop;

   if (listboxPixWidth > 0)
      assert(existsANonExplicitlyExpandedChild());

   return result + max(listboxActualPixHeight, // may be 0
                       allExpandedChildrenHeightAsDrawn
                       );
}


/* ****************************************************************************
 *
 * Expanded Children Measurements:
 *
 * ****************************************************************************
 */
template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
horiz_offset_to_expanded_child(const where4TreeConstants &tc,
                               const int childIndex) const {
   // Gives horiz pixel offset (from the left point of the leftmost
   // item being drawn below the root, which is the leftmost part
   // of the scrollbar of the listbox IF a listbox is up; else
   // from the left point of the leftmost child)
   // to the leftmost point of a certain expanded (implicitly or explicitly) child.

   // First, assert that child #childIndex is indeed expanded
   const bool allChildrenExpanded = (listboxPixWidth == 0);
   assert(allChildrenExpanded || theChildren[childIndex].isExplicitlyExpanded);

   int result = listboxPixWidth;
   if (listboxPixWidth > 0)
      result += tc.horizPixlistbox2FirstExpandedChild;

   for (int childlcv=0; childlcv < childIndex; childlcv++)
      if (allChildrenExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         result += theChildren[childlcv].theTree->myEntireWidthAsDrawn(tc);
            // a quick routine
         result += tc.horizPixBetweenChildren;
      }

   return result;      
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
wouldbe_all_expanded_children_width(const where4TreeConstants &tc) const {
   // Call this (fairly) expensive routine only to update
   // "allExpandedChildrenWidthAsDrawn", which speeds up draw().
   // Time is O(numchildren); could be optimized to O(num-expanded-children)

   // Returns the number of horizontal pixels that would be used up, if 
   // the expanded children of this subtree were drawn right now.

   // Major assumptions:
   // 1) Each descendant has an up-to-date "allExpandedChildrenWidthAsDrawn"
   // 2) Each descendant [and this node] has an up-to-date "listboxPixWidth"
   // Don't have to assume:
   // 1) "allExpandedChildrenWidthAsDrawn" for this node being up-to-date
   // 2) "horiz_pix_allowed_this_subtree" being passed in.  In other words, no
   //    thinking about _what_ is displayed is needed; all numbers are pre-calculated
   //    and simply need adding up, period.

   if (theChildren.size() == 0)
      return 0; // what children?

   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
      // no listbox --> everyone's either implicitly or explicitly expanded!

   int result = 0;
   int numExpandedChildren = 0;
   const int numChildren = theChildren.size();

   for (int childlcv = 0; childlcv < numChildren; childlcv++) {
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         result += theChildren[childlcv].theTree->myEntireWidthAsDrawn(tc); // cheap
         numExpandedChildren++;
      }
   }

   if (numExpandedChildren > 0)
      result += (numExpandedChildren - 1) * tc.horizPixBetweenChildren;

   return result;
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
wouldbe_all_expanded_children_height(const where4TreeConstants &tc) const {
   // Call this (fairly) expensive routine only to update
   // "heightOfAllExpandedChildrenAsDrawn", which speeds up draw().

   // Returns the number of vertical pixels that would be used up, if this subtree's
   // expanded children were drawn right now.
   // A gigantic assumption made by this routine is that
   // "heightOfAllExpandedChildrenAsDrawn" and "listboxHeight" of all
   // descendants has **already** been updated before this routine is called.

   // Don't have to assume:
   // 1) "allExpandedChildrenHeightAsDrawn" for this node being up-to-date
   // 2) "vert_pix_allowed_this_subtree" being passed in.  In other words, no
   //    thinking about _what_ is displayed is needed; all numbers are pre-calculated
   //    and simply need adding up, period.

   if (theChildren.size() == 0)
      return 0; // what children?

   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
      // no listbox --> everyone's either implicitly or explicitly expanded!

   int result = 0;
   const int numChildren = theChildren.size();
   for (int childlcv = 0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded)
         result = max(result, theChildren[childlcv].theTree->myEntireHeightAsDrawn(tc));

   return result;
}


/* *******************************************************************
 *
 * draw() -- the main drawing entrypoint
 *
 * Some crude clipping (based on Tk_Width(tc.theTkWindow), Tk_Height(tc.theTkWindow))
 * is performed automatically.
 *
 * *******************************************************************
 */
template <class USERNODEDATA>
void where4tree<USERNODEDATA>::draw(const where4TreeConstants &tc,
                                    Drawable theDrawable,
                                    const int middlex, const int topy,
                                    const bool rootOnly,
                                    const bool listboxOnly) const {
   const int maxWindowY = Tk_Height(tc.theTkWindow) - 1; // anything below this won't appear
   if (topy > maxWindowY)
      return; // quick & dirty clipping

   if (!listboxOnly)
      theRootNode.draw(tc, theDrawable,
                       middlex, topy);

   if (rootOnly || theChildren.size() == 0)
      return; // no children to draw...we are done.

   // Now draw as follows:
   // a) If listboxPixWidth > 0, draw listbox (containing names of _all_
   //    non-explicitly-expanded children, with triangles next to each non-leaf
   //    child subtree.
   //    (NOTE: but some leaf nodes may be quite interesting; e.g. the leaf
   //    of a box containing source code...perhaps we need an interesting()
   //    virtual function that returns true iff I, a subtree, am interesting.
   //    Sounds silly, but useful.)
   // b) Moving ever to the right, draw expanded children (if any).  Note that
   //    if there is a listbox up, then there won't be _any_ implicitly-expanded
   //    children.  Note also that if a listbox is _not_ up, then every child
   //    is expanded (either implicitly or explicitly).

   const int horizPixEverythingBelowRoot = horiz_pix_everything_below_root(tc);
      // a very quick routine

   int rayOriginX = middlex;
   int rayOriginY = topy + theRootNode.getHeight(); // we don't subtract 1
 
   if (rayOriginY > maxWindowY)
      return; // quick & dirty clipping

   int childtopypix = rayOriginY + tc.vertPixParent2ChildTop;

   int currentXpix = middlex - horizPixEverythingBelowRoot / 2;

   const int windowMinX = 3;
   const int windowMaxX = Tk_Width(tc.theTkWindow) - 1 - 3;

   const int subStuffLeftBounds  = currentXpix;
   const int subStuffRightBounds = currentXpix + horizPixEverythingBelowRoot - 1;
   if (subStuffLeftBounds > windowMaxX)
      return; // quick & dirty (yet effective) clipping
   if (subStuffRightBounds < windowMinX)
      return; // quick & dirty (yet effective) clipping

   if (listboxPixWidth > 0) {
      assert(existsANonExplicitlyExpandedChild());
      assert(listboxFullPixHeight > 0);
      assert(listboxActualPixHeight > 0);
   }

   // First, draw the listbox
   if (listboxPixWidth > 0) {
      // ray from bottom of root to (centerx, topy) of listbox:
      if (!listboxOnly)
         XDrawLine(tc.display, theDrawable, tc.listboxRayGC,
                   rayOriginX, rayOriginY,  // (left, top)
                   currentXpix + listboxPixWidth / 2, // right
                   childtopypix - 1 // bottom
                   );

      draw_listbox(tc, theDrawable,
                   currentXpix, // left
                   childtopypix, // top
                   -1, -1
                   );

      currentXpix += listboxPixWidth +
                     tc.horizPixlistbox2FirstExpandedChild;
   }

   if (listboxOnly)
      return;

   if (currentXpix > windowMaxX)
      return;

   // Now draw expanded children, if any
   const bool allChildrenAreExpanded = (listboxPixWidth == 0);
   const int numChildren = theChildren.size();
   for (int childlcv=0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         const where4tree *theChild = theChildren[childlcv].theTree;
         const int childEntireWidthAsDrawn = theChild->myEntireWidthAsDrawn(tc);
            // not expensive

         const int subtree_centerx = currentXpix + childEntireWidthAsDrawn / 2;

         // Ray from bottom of root to (centerx,topy) of subchild:
         // Beware: X has only a 16-bit notion of x and y fields.
         //         We must pin if any value is too big.  In our case, that
         //         means don't do the XDrawLine() if the recursive child draw()
         //         is going to draw absolutely nothing due to our manual clipping.
         if (currentXpix <= windowMaxX)
            XDrawLine(tc.display, theDrawable, tc.subchildRayGC,
                      rayOriginX, rayOriginY,
                      subtree_centerx, childtopypix-1);
                              
         // Recursively draw the subtree
         theChild->draw(tc, theDrawable,
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
 * NOTE: neither routine attempts to redraw anything (is this right?  I think so.
 *       all drawing should be initiated by outside code w/ a Tk_DoWhenIdle())
 *
 * *******************************************************************
 */

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::
explicitlyExpandSubchild1Level(const where4TreeConstants &tc,
                               const int childindex) {
   // returns true iff any changes are made.

   if (theChildren[childindex].isExplicitlyExpanded)
      return false; // already expanded

   theChildren[childindex].isExplicitlyExpanded = true;

   // Note: we _don't_ rethink dimensions for the expanded subtree.  It will
   // look just at it looked that last time it was expanded.
   // But we _do_ rethink dimensions for us, the parent.  In particular,
   // the listbox dimensions can completely change; in addition, variables
   // tracking the width and height of all expanded children change.

   rethink_listbox_dimensions(tc);
      // an expensive routine, O(numchildren).  Could be optimized if the
      // listbox widths were somehow sorted, because most time is spent
      // looping through the remaining listbox entries to detemine the maximum
      // pixel width...

   rethink_all_expanded_children_dimensions(tc);

   // Note: you'll probably need to erase and redraw the _entire_ where axis
   // now, even ascendants of this subtree (why?  because changing the width
   // of this subtree changes the width of the ascendant which changes the
   // width of everything above it, which means everything needs recentering)

   return true;
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::explicitlyUnexpandSubchild(const where4TreeConstants &tc,
                                                          const int childindex) {
   // returns true iff any changes are made

   const bool allChildrenExpanded = (listboxPixWidth == 0);
   if (!allChildrenExpanded && !theChildren[childindex].isExplicitlyExpanded)
      return false; // already un-expanded!

   // invariant allChildrenExpanded or explicitlyExpandedChildren[childindex] (or both)
   theChildren[childindex].isExplicitlyExpanded = false;

   rethink_listbox_dimensions(tc);
      // an expensive routine, O(numchildren)   

   rethink_all_expanded_children_dimensions(tc);

   // note: you'll probably need to erase and redraw the _entire_ where axis
   // now, even ascendants of this subtree (why?  because changing the width
   // of this subtree changes the width of the ascendant which changes the
   // width of everything above it, which means everything needs recentering)

   return true;
}


/* *******************************************************************
 *
 * Expensive routine on window resize.  listbox may completely change.
 *
 * New feature: we need to loop through EVERY listbox (ugh) and
 * rethink whether or not it needs a scrollbar (using tc.listboxHeightWhereSBappears)
 * which _may_ change listboxActualHeight and listboxPixWidth.
 * We didn't put this functionality into rethinkAfterResize(), but it pretty
 * much needs to be called by outside code every time it calls rethinkAfterResize()
 * 
 * *******************************************************************
 */
template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::
rethinkListboxAfterResize1(const where4TreeConstants &tc) {
   // Using listboxFullPixHeight and tc.listboxHeightWhereSBappears
   // (and assuming the latter has changed), rethink scrollbar aspects of listbox.

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
         // An SB was up, but need not be up any longer.  The listbox
         // width decreases by the width of a scrollbar, and the listbox
         // actual height is set to listbox full height.
         theScrollbar.invalidate();
         listboxPixWidth -= listboxScrollBarWidth;
         listboxActualPixHeight = listboxFullPixHeight;
      }
   else if (!theScrollbar.isValid()) {
      // no SB was up, but one is needed now
      theScrollbar.validate();
      listboxPixWidth += listboxScrollBarWidth;
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;
   }
   else {
      // An SB was up, and still needs to be up.
      listboxActualPixHeight = tc.listboxHeightWhereSBappears;
      theScrollbar.updateForNewBounds(
            listboxActualPixHeight-2*listboxBorderPix,
            listboxFullPixHeight-2*listboxBorderPix);
   }

   return true; // changes were made
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::
rethinkListboxAfterResize(const where4TreeConstants &tc) {
   // Assuming tc.listboxHeightWhereSBappears has changed (presumably due to a window
   // resize), rethink listbox parameters.
   // IMPORTANT: After subtree root A has a child B whose listbox dimensions have
   //            changed, A's allExpandedChildrenWidthAsDrawn and/or
   //            allExpandedChildrenHeightAsDrawn need updating.
   // Returns true iff any changes were made to us (the subtree root) or any of
   // our descendants (in which case _our_ parent should rethink its
   // allExpandedChildrenWidth/HeightAsDrawn).

   bool anyChildrenHaveChanged = false;

   const int numChildren = theChildren.size();
   for (int i=0; i < numChildren; i++) {
      where4tree *theChild = theChildren[i].theTree;
      if (theChild->rethinkListboxAfterResize(tc))
         anyChildrenHaveChanged = true;
   }

   if (anyChildrenHaveChanged)
      // Fairly expensive routines: O(numchildren); could be optimized to
      // O(num-expanded-children).
      rethink_all_expanded_children_dimensions(tc);

   const bool weHaveChanged = rethinkListboxAfterResize1(tc);
   return anyChildrenHaveChanged || weHaveChanged;
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::rethinkAfterResize(const where4TreeConstants &tc,
                                                  int horizSpaceToWorkWithThisSubtree) {
   const int numChildren = theChildren.size();
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
   for (int childlcv=0; childlcv < numChildren; childlcv++) {
      childrenWidthIfAllExpanded +=
         theChildren[childlcv].theTree->myEntireWidthAsDrawn(tc) +
         tc.horizPixBetweenChildren;
   }

   assert(numChildren > 0);
   childrenWidthIfAllExpanded -= tc.horizPixBetweenChildren;
   // last child has no space after it (note that numChildren>0, so this is safe)

   if (existslistbox &&
       childrenWidthIfAllExpanded <= horizSpaceToWorkWithThisSubtree) {
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


/* *******************************************************************
 *
 * Paths
 *
 * *******************************************************************
 */

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::
makeGraphicalPathFromRawPath(const where4TreeConstants &tc,
                             int index,
                             whereNodeGraphicalPath &destPath,
                             whereNodeRawPath       &srcPath,
                             int rootNodeCenterX, int rootNodeTopY) {
   whereNodeGraphicalPathItem newItem(rootNodeCenterX, rootNodeTopY,
                                      srcPath[index].childnum);
   destPath.append(newItem);

   if (index == srcPath.getSize()-1)
      return; // all done

   assert(index < srcPath.getSize());

   int childnum = srcPath[index].childnum;
   assert(childnum >= 0); // negative allowed only for listbox items, which
                          // would always be at the end of a path...

   theChildren[childnum].theTree->
      makeGraphicalPathFromRawPath(tc, index + 1,
                                   destPath, srcPath,
                                   rootNodeCenterX -
                                      horiz_pix_everything_below_root(tc) / 2 +
                                      horiz_offset_to_expanded_child(tc, childnum) +
                                      theChildren[childnum].theTree->myEntireWidthAsDrawn(tc) / 2,
                                   rootNodeTopY + theRootNode.getHeight() +
                                      tc.vertPixParent2ChildTop
                                   );
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::point2ItemOneStepScrollbar(const where4TreeConstants &tc,
                                                 const int ypix,
                                                 const int scrollbarTop,
                                                 const int scrollbarHeight) const {
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
   theScrollbar.getSliderCoords(tc, scrollbarTop, scrollbarBottom,
                                listboxActualPixHeight-2*listboxBorderPix,
                                listboxFullPixHeight-2*listboxBorderPix,
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

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::
point2ItemOneStep(const where4TreeConstants &tc,
                  const int xpix, const int ypix,
                  const int root_centerx, const int root_topy) const {
   // returns -1 for a point in root, 0 thru n-1 for point w/in general
   // range of a child subtree [even if child is in a listbox],
   // -2 for a point on nothing,
   // -3 for a point in listbox scrollbar up-arrow,
   // -4 for point in listbox scrollbar down-arrow,
   // -5 for point in listbox scrollbar pageup-region,
   // -6 for point in listbox scrollbar pagedown-region,
   // -7 for point in listbox scrollbar slider.

   assert(ypix >= 0 && xpix >= 0);

   const int posRelativeToRoot = theRootNode.pointWithin(xpix, ypix,
                                                         root_centerx, root_topy);
   if (posRelativeToRoot == 2)
      return -2; // too high
   if (posRelativeToRoot == 4 || posRelativeToRoot == 5)
      return -2; // good ypix but bad xpix (directly to the left or right of root)
   if (posRelativeToRoot == 1)
      return -1; // bingo; in root node

   assert(posRelativeToRoot == 3); // below root...check listbox & expanded children

   const int childrenTop = root_topy + theRootNode.getHeight() +
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
            if (scrollBarIsUp && xpix < currentX + listboxScrollBarWidth)
               return point2ItemOneStepScrollbar(tc,
                                 ypix, // click point (note that xcoord is not needed)
                                 childrenTop, // scrollbar top
                                 listboxActualPixHeight // scrollbar height
                                                 );
                                                 
            // Wasn't a click in scrollbar.
            int listboxLocalX = xpix - currentX;
            if (scrollBarIsUp)
               listboxLocalX -= listboxScrollBarWidth;
            if (listboxLocalX < listboxBorderPix)
               return -2; // sorry, you clicked on the left border
                 
            int listboxLocalY = ypix - childrenTop;
            if (listboxLocalY < listboxBorderPix)
               return -2; // sorry, you clicked on the top border

            if (scrollBarIsUp)
               listboxLocalY += theScrollbar.getPixFirst();

            assert(listboxLocalX >= 0);
            assert(listboxLocalX < listboxPixWidth -
                                   (scrollBarIsUp ? listboxScrollBarWidth : 0));

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
   const int numChildren = theChildren.size();
   for (int childlcv = 0; childlcv < numChildren; childlcv++)
      if (allChildrenAreExpanded || theChildren[childlcv].isExplicitlyExpanded) {
         const where4tree *theChild = theChildren[childlcv].theTree;
         const int thisChildEntireWidth = theChild->myEntireWidthAsDrawn(tc);
            // not an expensive call
         if (xpix >= currentX && xpix <= currentX + thisChildEntireWidth - 1)
            // Judging by x-coords, point can only fall w/in this subtree
            return childlcv;

         currentX += thisChildEntireWidth + tc.horizPixBetweenChildren;
         if (xpix < currentX)
            return -2;
      }

   return -2;
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::point2ItemWithinListbox(const where4TreeConstants &tc,
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

   const int numChildren = theChildren.size();
   for (int childlcv=0; childlcv < numChildren; childlcv++) {
      // Since listbox exists, _all_ non-explicitly-expanded children are
      // in the listbox.
      if (!theChildren[childlcv].isExplicitlyExpanded)
         if (theRelativeItem == 0)
            return childlcv;
         else
            theRelativeItem--;
   }

   return -2; // nothing
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::point2GraphicalPath(whereNodeGraphicalPath &thePath,
                                                  const where4TreeConstants &tc,
                                                  const int xpix, const int ypix,
                                                  int root_centerxpix,
                                                  int root_topypix) {
   // Returns 1 for point in an expanded node, 2 for point in listbox data item,
   // 3 for point in listbox scrollbar up-arrow,
   // 4 for point in listbox scrollbar down-arrow,
   // 5 for point in listbox scrollbar pageup-region,
   // 6 for point in listbox scrollbar pagedown-region,
   // 7 for point in listbox scrollbar slider,
   // -1 for point in nothing.
   // The actual path is "returned" by modifying "thePath".
   // Uses point2ItemOneStep() for its main work...

   // NOTE: assumes "thePath" is initialized as usual

   whereNodeGraphicalPathItem &thisPathItem = thePath.getLastItem();
   assert(thisPathItem.childnum == theChildren.size());
   thisPathItem.root_centerxpix = root_centerxpix;
   thisPathItem.root_topypix    = root_topypix;

   const int theItem = point2ItemOneStep(tc, xpix, ypix,
                                         root_centerxpix,
                                         root_topypix);
   if (theItem == -2)
      return -1; // point in nothing

   if (theItem <= -3 && theItem >= -7)
      return -theItem; // one of 5 scrollbar positions

   if (theItem == -1)
      return 1; // point in "this" node; add nothing to "thePath" (we couldn't
                // even if we wanted to since we don't immediately know what child
                // number we are of our parent)

   assert(0 <= theItem);
   assert(theItem < theChildren.size());

   // If theItem represents a child in a listbox, then done; else, recurse.
   const bool allChildrenExpanded = (listboxPixWidth == 0);
   const bool itemIsInlistbox = !allChildrenExpanded &&
                                !theChildren[theItem].isExplicitlyExpanded;

   if (itemIsInlistbox) {
      thisPathItem.childnum = -theItem - 1;
         // negative is an indicator of a listbox item; -1 needed
         // since 0 is already reserved
      return 2; // point in listbox
   }

   // Time to recurse
   thisPathItem.childnum = theItem;

   where4tree &theChild = *(theChildren[theItem].theTree);

   thePath.append(whereNodeGraphicalPathItem(theChild.theChildren.size()));

   const int child_centerx = root_centerxpix - horiz_pix_everything_below_root(tc) / 2
                           + horiz_offset_to_expanded_child(tc, theItem)
                           + theChild.myEntireWidthAsDrawn(tc) / 2;
   const int child_topy = root_topypix + theRootNode.getHeight()
                        + tc.vertPixParent2ChildTop;

   return theChild.point2GraphicalPath(thePath, tc, xpix, ypix,
                                       child_centerx,
                                       child_topy);
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::string2Path(whereNodeRawPath &thePath,
                                          const where4TreeConstants &tc,
                                          const string &str,
                                          where4tree *beginSearchFrom,
					  bool testRootNode) {
   // returns 0 if not found.  returns 1 (and modifies "thePath") iff found,
   // and no expansions are necessary (we can scroll directly there); returns
   // 2 (and modifies "thePath") iff found but expansions of some intermediate
   // path items would be needed.
 
   // NOTE: The path returned will not contain ANY listbox childnums, except perhaps
   //       at the end.  You must check for this manually.

   // Depth first search (dive into listbox children, then expanded ones)

   // NOTE: "thePath" must start out initialized as usual...

   whereNodeRawPathItem &thisPathItem = thePath.getLastItem();
   assert(thisPathItem.childnum == theChildren.size());

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
   for (int i=0; i < numChildren; i++) {
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

         if (match) {
            thisPathItem.childnum = -i - 1; // end of path is in listbox
            return 1; // found; no exp necessary
	 }

         thisPathItem.childnum = i; // BUT, BEWARE, "i" IS IN A listbox STILL!
         where4tree *theChild = theChildren[i].theTree;
         thePath.append(whereNodeRawPathItem(theChild->theChildren.size()));
         if (theChild->string2Path(thePath, tc, str, beginSearchFrom, false) != 0)
            return 2; // found; expansion necessary.
         else
            // undo the path item we appended
            thePath.rigSize(thePath.getSize()-1);
      }
   }

   for (i=0; i < numChildren; i++) {
      const bool childIsExpanded = allChildrenExpanded ||
                                   theChildren[i].isExplicitlyExpanded;
      if (childIsExpanded) {
         thisPathItem.childnum = i;

         where4tree *theChild = theChildren[i].theTree;
         thePath.append(whereNodeRawPathItem(theChild->theChildren.size()));

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
 * listbox scrollbar clicks
 *
 * ********************************************************************
 */
template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::scroll_listbox(const where4TreeConstants &tc,
                                              const int listboxLeft,
                                              const int listboxTop,
                                              const int deltaYpix,
                                              const bool redrawNow) {
   // returns true iff any changes made.

   assert(theScrollbar.isValid());

   const int oldPixFirst = theScrollbar.getPixFirst();

   const int listboxTotalDataPix = listboxFullPixHeight - 2*listboxBorderPix;
   const int listboxActualDataPixHeight = listboxActualPixHeight - 2*listboxBorderPix;
   const int listboxActualDataPixWidth = listboxPixWidth - 2*listboxBorderPix -
                                         listboxScrollBarWidth;
 
   int newPixFirst = oldPixFirst + deltaYpix;
   newPixFirst = max(0, newPixFirst);
   newPixFirst = min(newPixFirst, listboxTotalDataPix - listboxActualDataPixHeight);
   assert(newPixFirst >= 0);
      // <0 implies actualHeight > totalHeight, implying no scrollbar should
      // have been up to begin with!

   if (newPixFirst == oldPixFirst)
      return false;

   const int actualDeltaY = newPixFirst - oldPixFirst;
   assert(actualDeltaY != 0);

   theScrollbar.setPixFirst(newPixFirst);

   if (redrawNow) {
      const int listboxDataLeft = listboxLeft + listboxBorderPix +
                                  listboxScrollBarWidth;
      const int listboxDataTop  = listboxTop + listboxBorderPix;

      // Under certain conditions, we must resort to
      // one whole draw_listbox() instead of scrolling
      // and doing a partial draw_listbox().  These conditions
      // are (1) the entire 'visible' listbox isn't entirely visible
      // on the screen [i.e a piece of it is scrolled off the screen],
      // or (2) the scroll amount is so high that the XCopyArea() would
      // simply result in a nop due to clipping.
      const int absDeltaY = (actualDeltaY >= 0) ? actualDeltaY : -actualDeltaY;
      assert(absDeltaY >= 0);
      if (absDeltaY > listboxActualDataPixHeight) {
         // this is case (2), described above.  Just draw entire listbox.
//         cout << "redrawing entire listbox since scroll amt is so high" << endl;

         draw_listbox(tc, tc.masterWindow,
                      listboxLeft, listboxTop,
                      -1, -1);
         return true;
      }
      
      if (listboxLeft < 0) {
         // this is case (1), described above.  Just draw entire listbox.
//         cout << "redrawing listbox since left < 0" << endl;

         draw_listbox(tc, tc.masterWindow,
                      listboxLeft, listboxTop,
                      -1, -1);
         return true;
      }

      if ((listboxLeft + listboxPixWidth - 1) > Tk_Width(tc.theTkWindow) - 1) {
         // this is case (1), described above.  Just draw entire listbox.
//         cout << "redrawing listbox since right (" 
//              << listboxLeft + listboxPixWidth - 1 << ") is beyond right bounds ("
//              << Tk_Width(tc.theTkWindow) - 1 << ")" << endl;

         draw_listbox(tc, tc.masterWindow,
                      listboxLeft, listboxTop,
                      -1, -1);
         return true;
      }

      if (listboxTop < 0) {
         // this is case (1), described above.  Just draw entire listbox.
//         cout << "redrawing listbox since top < 0" << endl;

         draw_listbox(tc, tc.masterWindow,
                             listboxLeft, listboxTop,
                             -1, -1);
         return true;
      }

      if ((listboxTop + listboxActualPixHeight - 1) > Tk_Height(tc.theTkWindow) - 1) {
         // this is case (1), described above.  Just draw entire listbox.
//         cout << "redrawing listbox since bottom (" << listboxTop + listboxActualPixHeight - 1 << ") is too low (" << Tk_Height(tc.theTkWindow)-1 << ")" << endl;

         draw_listbox(tc, tc.masterWindow,
                      listboxLeft, listboxTop,
                      -1, -1);
         return true;
      }


      // Set clipping to ensure that the copy stays within
      // the bounds of the listbox data area.

      XRectangle clipRect = {listboxDataLeft,
                             listboxDataTop,
                             listboxActualDataPixWidth,
                             listboxActualDataPixHeight
                             };

      XSetClipRectangles(tc.display,
                         tc.listboxBackgroundGC,
                         0, 0,
                         &clipRect, 1, YXBanded
                         );

      XCopyArea(tc.display,
                tc.masterWindow, // source drawable
                tc.masterWindow, // dest drawable
                tc.listboxBackgroundGC, // ???
                listboxDataLeft, // source leftx
                listboxDataTop,  // source topy
                listboxActualDataPixWidth,  // width of area to copy
                listboxActualDataPixHeight, // height of area to copy
                listboxDataLeft,              // dest leftx
                listboxDataTop - actualDeltaY // dest topy
                );

      // undo the clipping change:
      XSetClipMask(tc.display, tc.listboxBackgroundGC, None);

      int redrawLbStart, redrawLbHeight; // data y pixel coords relative to lb
      if (actualDeltaY < 0) {
         // scroll upwards (e.g. click on up-arrow).  Contents move downwards;
         // hole (to redraw) opens up at the top.
         redrawLbStart = 0;
         redrawLbHeight = -actualDeltaY;
      } 
      else {
         // scroll downwards (e.g. click on down-arrow).  Contents move upwards;
         // hole (to redraw) opens up at the bottom.
         redrawLbHeight = actualDeltaY;
         redrawLbStart = (listboxActualDataPixHeight - 1)
                         - actualDeltaY + 1;
      }

      draw_listbox(tc, tc.masterWindow,
                   listboxLeft, listboxTop,
                   redrawLbStart, redrawLbHeight);
   }

   return true;
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::scrollBarClick(const where4TreeConstants &tc,
                                              const int clickLocation,
                                                 // 3 --> up arrow
                                                 // 4 --> down arrow
                                                 // 5 --> pageup   region
                                                 // 6 --> pagedown region
                                              const int root_centerx,
                                              const int root_topy,
                                              const bool redrawNow) {

   const int listboxLeft = root_centerx - horiz_pix_everything_below_root(tc) / 2;
   const int listboxTop  = root_topy + theRootNode.getHeight() +
                           tc.vertPixParent2ChildTop;

   int deltaYpix=0;

   const int listboxActualDataPix = listboxActualPixHeight -
                                   2*listboxBorderPix;

   switch (clickLocation) {
      case 3: // up-arrow
         deltaYpix = -4;
         break;
      case 4: // down-arrow
         deltaYpix = 4;
         break;
      case 5: // page-up
         deltaYpix = -listboxActualDataPix;
         break;
      case 6: // page-down
         deltaYpix = listboxActualDataPix;
         break;
      default:
         cout << "where4tree<USERNODEDATA>::scrollBarClick -- unrecognized clickLocation; cannot adjust" << endl;
         assert(false);
   }

   (void)scroll_listbox(tc, listboxLeft, listboxTop, deltaYpix, redrawNow);
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::path2lbScrollBarClick(const where4TreeConstants &tc,
                                       const whereNodeGraphicalPath &thePath,
                                       const int pathlcv,
                                       const int clickLocation,
                                       const bool redrawNow) {
   assert(pathlcv < thePath.getSize());
   if (pathlcv < thePath.getSize()-1) {
      const int childnum = thePath[pathlcv].childnum;
      where4tree *theChild = theChildren[childnum].theTree;
      theChild->path2lbScrollBarClick(tc, thePath, pathlcv+1, clickLocation, redrawNow);
      return;
   }

   // We are at the last item in the path...
   assert(pathlcv == thePath.getSize()-1);

   const whereNodeGraphicalPathItem &lastItem = thePath.getConstLastItem();
   assert(lastItem.childnum == theChildren.size());

   scrollBarClick(tc, clickLocation,
                  lastItem.root_centerxpix,
                  lastItem.root_topypix,
                  redrawNow
                  );
}

template <class USERNODEDATA>
int where4tree<USERNODEDATA>::path2lbScrollBarSliderTopPix(
                                              const where4TreeConstants &tc,
                                              const whereNodeGraphicalPath &thePath,
                                              const int pathlcv) {
   assert(pathlcv < thePath.getSize());
   if (pathlcv < thePath.getSize()-1) {
      const int childnum = thePath[pathlcv].childnum;
      where4tree *theChild = theChildren[childnum].theTree;
      return theChild->path2lbScrollBarSliderTopPix(tc, thePath, pathlcv+1);
   }

   // We are at the last item in the path...
   assert(pathlcv == thePath.getSize()-1);

   const whereNodeGraphicalPathItem &lastItem = thePath.getConstLastItem();
   assert(lastItem.childnum == theChildren.size());

   const int listboxTopPix = lastItem.root_topypix + theRootNode.getHeight() +
                             tc.vertPixParent2ChildTop;

   int sliderPixTop;
   int sliderPixHeight;
   theScrollbar.getSliderCoords(tc, listboxTopPix,
                                listboxTopPix + listboxActualPixHeight - 1,
                                listboxActualPixHeight - 2*listboxBorderPix,
                                listboxFullPixHeight - 2*listboxBorderPix,
                                sliderPixTop, // filled in
                                sliderPixHeight // filled in
                                );

   return sliderPixTop;
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::
rigListboxScrollbarSliderTopPix(const where4TreeConstants &tc,
                                const int scrollBarLeft,
                                   // only needed if redrawing
                                const int scrollBarTop,
                                const int scrollBarBottom,
                                const int newScrollBarSliderTopPix,
                                const bool redrawNow) {
   // returns true iff any changes were made

   const int listboxTotalDataPix = listboxFullPixHeight -
                                   2*listboxBorderPix;

   const int oldPixFirst = theScrollbar.getPixFirst();

   int newScrollbarPixFirst = theScrollbar.pixFirstFromAbsoluteCoord(
                                     tc, scrollBarTop, scrollBarBottom,
                                     listboxTotalDataPix,
                                     newScrollBarSliderTopPix);

   return scroll_listbox(tc, scrollBarLeft, scrollBarTop,
                         newScrollbarPixFirst - oldPixFirst,
                         redrawNow);
}


/* ********************************************************************
 *
 * High-level tree actions (highlighting, expanding) made easy
 * when given paths.
 *
 * ********************************************************************
 */
template <class USERNODEDATA>
void where4tree<USERNODEDATA>::
toggleHighlightFromPath(const where4TreeConstants &tc,
                        const whereNodeGraphicalPath &thePath,
                        const bool redrawNow) {
   where4tree *subjectPtr = this;

   for (int pathlcv=0; pathlcv < thePath.getSize()-1; pathlcv++) {
      int childnum = thePath[pathlcv].childnum;
      subjectPtr = subjectPtr->theChildren[childnum].theTree;
   }

   // Now we are at the last item in thePath
   const whereNodeGraphicalPathItem &lastItem = thePath.getConstLastItem();
   if (lastItem.childnum < 0) {
      // subjectPtr is the parent of a listbox item in question, which
      // is child #abs(lastItem.childnum) of subjectPtr.
      // the -1 is needed since childnum was base -1 instead of base 0; 0
      //    was already in use for legit children
      subjectPtr->theChildren[-lastItem.childnum - 1].theTree->
                    toggle_highlight_root_only(false, // don't redraw now
                                               tc, -1, -1);
      subjectPtr->draw(tc, tc.masterWindow,
                       lastItem.root_centerxpix, lastItem.root_topypix,
                       false, // not root only
                       true // listbox only
                       );
   }
   else {
      assert(lastItem.childnum == subjectPtr->theChildren.size());
      subjectPtr->toggle_highlight_root_only(true, // redraw now
                                             tc,
                                             lastItem.root_centerxpix,
                                             lastItem.root_topypix);
   }
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::
path2pixOffset(const where4TreeConstants &tc,
               const whereNodeRawPath &thePath,
               const int pathIndex,
               const int offset_to_root_centerx,
               const int offset_to_root_topy,
               int &result_offset_centerx, int &result_offset_topy) const {
   // Finds the [centerx, topy] pixels (relative to leftx, topy of the root) of
   // the item at the end of the path.

   // WARNING: All elements along the path must be expanded.
   // WARNING: For a listbox item, we simply return the (centerx, topy) of the listbox
   //          itself, which is a mediocre solution.  (We also want the vertical
   //          offset w/in the listbox)

   if (pathIndex < thePath.getSize()-1) {
      const whereNodeRawPathItem &theItem = thePath[pathIndex];

      int offset_to_next_centerx = offset_to_root_centerx -
                                   horiz_pix_everything_below_root(tc) / 2;
      offset_to_next_centerx += horiz_offset_to_expanded_child(tc, theItem.childnum);

      const where4tree *theChild = theChildren[theItem.childnum].theTree;

      offset_to_next_centerx += theChild->myEntireWidthAsDrawn(tc) / 2;

      int offset_to_next_topy = offset_to_root_topy + theRootNode.getHeight() +
                                tc.vertPixParent2ChildTop;

      // Note: (next_centerx, nexty) give us the [centerx, topy] offset of
      //       the next child in the path...

      theChild->path2pixOffset(tc, thePath, pathIndex + 1,
                               offset_to_next_centerx, offset_to_next_topy,
                               result_offset_centerx, result_offset_topy);
      return;
   }

   // Now for the last item.  If the childnum < 0, then we are referring to
   // a listbox element (or a former one).  In that case, there's 1 more step.
   // But if the childnum >= 0, then we are referring to the end of
   // path where childnum is set to numchildren; in that case, there is
   // nothing really left to do.

   const whereNodeRawPathItem &finalItem = thePath.getConstLastItem();
   if (finalItem.childnum < 0) {
      // What to do if the last item in the path is a listbox item?
      // We'll simply return the coords of the (center,top) of the listbox.
      // Whoever called this routine may easily check that the path ended
      // in a listbox item, and take more specific actions, as desired.

      result_offset_centerx = offset_to_root_centerx -
                              horiz_pix_everything_below_root(tc)/2 +
                              listboxPixWidth / 2;
      result_offset_topy = offset_to_root_topy +
                           theRootNode.getHeight() +
                           tc.vertPixParent2ChildTop;
   }
   else {
      assert(finalItem.childnum == theChildren.size());
      result_offset_centerx = offset_to_root_centerx;
      result_offset_topy    = offset_to_root_topy;
   }
}
                                         
template <class USERNODEDATA>
int where4tree<USERNODEDATA>::path2lbItemExpand(const where4TreeConstants &tc,
                                                whereNodeRawPath &thePath,
                                                const int pathIndex) {
   // returns 0 if you tried to expand an already-expanded item,
   // returns 1 if you tried to expand a leaf listbox item,
   // returns 2 normally; plus, does the following:

   // new feature: Updates thePath to reflect the fact that the last item is no longer
   // a listbox item (iff any changes were made; i.e., if an expansion took place).
   // when doing it, we make no effort to put in correct xpix/ypix coords, we're
   // only being concerned about the correct childnum's.

   // new feature: whenever a node A is explicitly expanded, so are all of its
   //              ancestors.

   const int pathSize = thePath.getSize();
   assert(pathIndex < pathSize);

   if (pathIndex < pathSize-1) {
      // recurse...then, if changes were made to a descendant, rethink our
      // child params...
      int childnum = thePath[pathIndex].childnum;
      const int result = theChildren[childnum].theTree->
                         path2lbItemExpand(tc, thePath, pathIndex+1);
      if (result==2) {
         theChildren[childnum].isExplicitlyExpanded = true;
            // new feature

         // changes were made to a descendant of child #theItem.  We
         // need to update some of our internal variables.  Why?  Because the
         // descendant action probably changed some width/height sizes.
         rethink_all_expanded_children_dimensions(tc);
      }

      return result;
   }

   assert(pathIndex == pathSize-1);
   whereNodeRawPathItem &lastItem = thePath.getLastItem();
   if (lastItem.childnum >= 0)
      return false; // you cannot expand an already-expanded (non-listbox) item!

   // We're parent of listbox item in question: child #abs(lastItem.childnum) of us.
   // The -1 is needed since childnum was base -1 instead of base 0; 0
   //    was already in use for legit children

   const int realLastChild = -lastItem.childnum - 1; // -1 to normalize to 0

   // Unhighlight the node
   theChildren[realLastChild].theTree->theRootNode.unhighlight(false, tc, -1, -1);
      // don't redraw

   // expand --- but wait --- only if the clickee isn't a leaf node
   if (theChildren[realLastChild].theTree->theChildren.size() > 0) {
      (void)explicitlyExpandSubchild1Level(tc, realLastChild);
      // rethinks listbox dimensions (may no longer need one),
      // as well as allExpandedChildrenWidthAsDrawn and
      //            allExpandedChildrenHeightAsDrawn.
      // we always return true because a change has already
      // been made: the node has become unhighlighted.

      lastItem.childnum = realLastChild;

      whereNodeRawPathItem newLastItem(theChildren[realLastChild].theTree->theChildren.size());

      thePath.append(newLastItem);
      
      return 2;
   }
   else
      // a leaf node
      return 1;
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::expandEntirePath(const where4TreeConstants &tc,
                                                const whereNodeRawPath &thePath,
                                                const int index) {
   // Given a path with no negative childnum's [i.e., one in which we
   // determine manually whether or not an element is w/in a listbox],
   // forcibly expand out any and all listbox path elements.

   // Exception: won't expand a listbox item at the very end of the path.
   // Returns true iff any expansion(s) actually took place.

   if (index == thePath.getSize()-1) {
      if (thePath[index].childnum < 0) {
         // the last path item is in a listbox.  We won't expand it.
         return false;
      }
      else {
         assert(thePath[index].childnum == theChildren.size());
         return false;
      }
   }

   // follow the next link along the path...
   const int childindex = thePath[index].childnum;
   assert(childindex >= 0);

   const bool allChildrenExpanded = (listboxPixWidth == 0);
   if (!allChildrenExpanded && !theChildren[childindex].isExplicitlyExpanded) {
      // Aha! the next link is in a listbox.  Let's expand him.

      (void)theChildren[childindex].theTree->expandEntirePath(tc, thePath, index+1);
         // important to do this before we rethink stuff about child dimensions.
         // Hence, we do this before expansion

      assert(explicitlyExpandSubchild1Level(tc, childindex));
         // rethinks stuff, too (like allExpandedChildrenWidthAsDrawn)

      return true; // expansion(s) were made
   }
   else {
      const bool result = theChildren[thePath[index].childnum].theTree->
                          expandEntirePath(tc, thePath, index+1);

      if (result)
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::expandAllChildren(const where4TreeConstants &tc) {
   // expand _all_ children --- but wait --- only the non-leaf ones.
   // returns true iff any changes...

   bool result = false;
   bool anythingLeftInlistbox = false;

   for (int childlcv=0; childlcv < theChildren.size(); childlcv++) {
      if (theChildren[childlcv].theTree->theChildren.size() > 0) {
         theChildren[childlcv].isExplicitlyExpanded = true;
         result = true;
      }
      else
         anythingLeftInlistbox = true;
   }

   if (!result)
      return false; // no changes

   if (!anythingLeftInlistbox)
      // We _definitely_ don't need a listbox anymore...
      removeListbox();
   else
      rethink_listbox_dimensions(tc);

   rethink_all_expanded_children_dimensions(tc);
      // allExpandedChildrenWidthAsDrawn and allExpandedChildrenHeightAsDrawn

   return true; // changes were made
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::
path2lbItemScrollSoVisible(const where4TreeConstants &tc,
                           const whereNodeGraphicalPath &thePath,
                           const int pathIndex) {
   // returns true iff any changes were made

   assert(pathIndex < thePath.getSize());
   if (pathIndex == thePath.getSize()-1) {
      // Here we are, at the parent node of the listbox in question.

      if (!theScrollbar.isValid())
         // the listbox is small enough to entirely fit on the screen, so no
         // scrolling is possible or necessary.
         return false;

      assert(listboxPixWidth > 0);
      assert(existsANonExplicitlyExpandedChild());

      const int itemBoxHeight = tc.listboxVertPadAboveItem +
                                tc.listboxFontStruct->ascent +
                                tc.listboxVertPadAfterItemBaseline;

      int childnum = -thePath.getConstLastItem().childnum - 1;
      assert(childnum >= 0);

      int scrollToVertPix = 0; // relative to the listbox

      for (int i=0; i < childnum; i++) {
         // If child i is in the listbox, then add "itemBoxHeight" to "scrollToVertPix"
         if (!theChildren[i].isExplicitlyExpanded)
            scrollToVertPix += itemBoxHeight;
      }

      const int listboxLeft = thePath.getConstLastItem().root_centerxpix -
                              horiz_pix_everything_below_root(tc) / 2;
      const int listboxTop = thePath.getConstLastItem().root_topypix +
                             theRootNode.getHeight() + 
                             tc.vertPixParent2ChildTop;

      const int deltaYpix = scrollToVertPix - theScrollbar.getPixFirst();
      const bool anyChanges = scroll_listbox(tc,
                                             listboxLeft, listboxTop,
                                             deltaYpix,
                                             true // redraw now
                                             );
      return anyChanges;
   }
   else {
      const whereNodeGraphicalPathItem &thisItem = thePath[pathIndex];
      return theChildren[thisItem.childnum].theTree->
             path2lbItemScrollSoVisible(tc, thePath, pathIndex+1);
   }
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::path2ExpandAllChildren(const where4TreeConstants &tc,
                                        const whereNodeRawPath &thePath,
                                        const int pathIndex) {
   // returns true iff any changes.
   // new feature: in general, when we explicitly expand node A, we also
   //              explicitly expand all of its ancestors.  At first, this seems
   //              silly (how could we have explicitly expanded A if its ancestors
   //              were not visible to begin with?)  The key is that the ancestors
   //              may have been implicitly expanded.

   const int pathSize = thePath.getSize();
   assert(pathIndex < pathSize);

   if (pathIndex < pathSize-1) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're expanding all of its children!], then rethink our child size params...
      const whereNodeRawPathItem &thisItem = thePath.getConstItem(pathIndex);

      const bool result = theChildren[thisItem.childnum].theTree->path2ExpandAllChildren
                                                          (tc, thePath, pathIndex+1);
      if (result) {
         theChildren[thisItem.childnum].isExplicitlyExpanded = true; // new feature
         rethink_all_expanded_children_dimensions(tc);
      }

      return result;
   }

   assert(pathIndex == pathSize-1);
   const whereNodeRawPathItem &lastItem = thePath.getConstLastItem();
   assert(lastItem.childnum == theChildren.size());

   return expandAllChildren(tc);
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::path2lbItemUnexpand(const where4TreeConstants &tc,
                                                   const whereNodeRawPath &thePath,
                                                   const int pathIndex) {
   // returns true iff any changes were made
   const int pathSize = thePath.getSize();
   if (pathIndex == pathSize-2) {
      // We're at the second-to-last element, which means we're at the
      // parent of the (non-listbox) item to be expanded.
      const whereNodeRawPathItem &parentItem = thePath.getConstItem(pathIndex);
      const whereNodeRawPathItem &childItem = thePath.getConstLastItem();

      // We are un-expanding this->theChildren[parentItem.childnum].theTree
      // We assert that the child is a non-listbox item:
      if (childItem.childnum < 0)
         return false; // listbox items are already un-expanded!

      const int childIndex = parentItem.childnum;
      assert(childIndex >= 0);

      // Unhighlight the node
      theChildren[childIndex].theTree->theRootNode.unhighlight(false, tc, -1, -1);

      return this->explicitlyUnexpandSubchild(tc, childIndex);
         // rethinks listbox dimensions (will definitely be one now, which may
         // not have existed before).
         // as well as allExpandedChildrenWidthAsDrawn and
         //            allExpandedChildrenHeightAsDrawn
      
   }
   else {
      // recurse...then, if changes were made to a descendant, rethink our
      // child params...
      const whereNodeRawPathItem &thisItem = thePath.getConstItem(pathIndex);
      const bool result = theChildren[thisItem.childnum].theTree->path2lbItemUnexpand
                            (tc, thePath, pathIndex + 1);
      if (result)
         // Changes were made to a descendant of child #theItem.  We
         // need to update some of our internal variables.  Why?  Because the
         // descendant action probably changed some width/height sizes.
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }
}

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::unExpandAllChildren(const where4TreeConstants &tc) {
   // returns true iff any changes were made
   for (int childlcv=0; childlcv < theChildren.size(); childlcv++)
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

template <class USERNODEDATA>
bool where4tree<USERNODEDATA>::path2UnExpandAllChildren(const where4TreeConstants &tc,
                                          const whereNodeRawPath &thePath,
                                          const int pathIndex) {
   // returns true iff any changes were made
   const int pathSize = thePath.getSize();
   assert(pathIndex < pathSize);

   if (pathIndex < pathSize-1) {
      // recurse...then, if changes were made to a descendant [of course there were;
      // we're un-expanding all of its children!], then rethink our child size params...
      const whereNodeRawPathItem &thisItem = thePath.getConstItem(pathIndex);

      const bool result = theChildren[thisItem.childnum].theTree->
                          path2UnExpandAllChildren(tc, thePath, pathIndex+1);

      if (result)
         rethink_all_expanded_children_dimensions(tc);

      return result;
   }

   assert(pathIndex == pathSize-1);
   const whereNodeRawPathItem &lastItem = thePath.getConstLastItem();
   assert(lastItem.childnum == theChildren.size());

   return unExpandAllChildren(tc);
}


/* ******************************************************************
 *
 * Manual Highlighting
 *
 * ******************************************************************
 */

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::toggle_highlight_root_only(const bool redrawNow,
                                            const where4TreeConstants &tc,
                                            const int middlex, const int topy) {
   theRootNode.toggle_highlight(redrawNow, tc, middlex, topy);
}


/* ***************************************************************************
 *
 * Constructor, Destructor
 *
 * ***************************************************************************
 */
template <class USERNODEDATA>
void where4tree<USERNODEDATA>::removeListbox() {
   listboxPixWidth = listboxFullPixHeight
                         = listboxActualPixHeight
                         = 0;
   theScrollbar.invalidate();
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::manual_construct(const where4TreeConstants &tc) {
   // theChildren [] is initialized to empty vector

   removeListbox();

   allExpandedChildrenWidthAsDrawn  = 0;
   allExpandedChildrenHeightAsDrawn = 0;
      // since all that exists of this subtree is the root, so far.

   numChildrenAddedSinceLastSort = 0;
}

template <class USERNODEDATA>
where4tree<USERNODEDATA>::~where4tree() {
   // the children need explicit deleting
   const int numChildren = theChildren.size();
   for (int childlcv = 0; childlcv < numChildren; childlcv++) {
      where4tree *theChild = theChildren[childlcv].theTree;
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

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::addChild(where4tree *theNewChild,
					const bool explicitlyExpanded,
					const where4TreeConstants &tc,
					const bool rethinkGraphicsNow,
					bool resortNow) {
   // theNewChild _must_ have been created with C++ new -- not negotiable
   // theNewChild _must_ be fully initialized, too.
   // NOTE: In the current implementation, we always put the child into the listbox
   //       unless explicitlyExpanded is true.

   const string &childString = theNewChild->getRootName();
   const int childTextWidth = XTextWidth(tc.listboxFontStruct,
                                         childString.string_of(),
                                         childString.length());
                                         
   childstruct cs={theNewChild, explicitlyExpanded, childTextWidth};
   this->theChildren += cs;
   this->numChildrenAddedSinceLastSort++;

   if (rethinkGraphicsNow)
      if (explicitlyExpanded)
         // no need to rethink listbox dimensions, but a need to
         // rethink expanded children dimensions
         rethink_all_expanded_children_dimensions(tc);
      else
         rethink_listbox_dimensions(tc);

   if (resortNow)
      this->sortChildren();
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::doneAddingChildren(const where4TreeConstants &tc) {
   // Needed only if you call ::addChild() one or more times with
   // rethinkGraphicsNow==false
   rethink_listbox_dimensions(tc); // expensive; cost is O(numchildren)

   rethink_all_expanded_children_dimensions(tc);

   sortChildren();
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::
recursiveDoneAddingChildren(const where4TreeConstants &tc) {
   for (int i=0; i < theChildren.size(); i++) {
      where4tree *childPtr = theChildren[i].theTree;
      childPtr->recursiveDoneAddingChildren(tc);
   }

   doneAddingChildren(tc);
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::sortChildren() {
   // This is not the ideal sort.  The ideal sort is as follows:
   // 1) if only a handful of items have been added since the last
   //    sort (say, <= 10), then use insertion sort.
   // 2) if more than a handful of items have been added since the
   //    last sort, and there were only a handful of items previously
   //    in existence (say, <= 10), then just quicksort the whole thing.
   // 3) else, we have inserted more than a few items (unsorted), and more
   //    than a few items were already there (already sorted).
   //    Quicksort the newly-added items only.  Now we have two sorted
   //    subsections.  Merge those two with a mergesort's "merge".

   if (numChildrenAddedSinceLastSort > 0 && theChildren.size() > 1) {
      theChildren.sort(childstruct::cmpfunc);
   }

//   if (numChildrenAddedSinceLastSort > 0 && theChildren.size() > 1) {
//      // sorting is needed.  If the children are already almost
//      // sorted (numChildrenAddedSinceLastSort << num-children), then
//      // we sort using an insertion sort.  Else, we sort using a quicksort.
//
//      if ((numChildrenAddedSinceLastSort << 4) > theChildren.size()) {
//         // 16*numChildrenAddedSinceLastSort > num-children
//         // numChildrenAddedSinceLastSort > 1/16(num-children)
//         QuicksortChildren(0, theChildren.size()-1);
//      }
//      else
//         InsertionsortChildren(0, theChildren.size()-1);
//   }

   numChildrenAddedSinceLastSort = 0;
}

//template <class USERNODEDATA>
//void where4tree<USERNODEDATA>::InsertionsortChildren(int left, int right) {
//   for (int j=left+1; j <= right; j++) {
//      childstruct key = theChildren[j];
//
//      // Now, we insert theChildren[j] into its proper
//      // place amongst indexes (left+1) thru (j-1)
//
//      int i=j;
//      while (--i > 0 && theChildren[i] > key)
//         theChildren[i+1] = theChildren[i];
//
//      theChildren[i+1] = key;
//   }
//}

//template <class USERNODEDATA>
//int where4tree<USERNODEDATA>::partitionChildren(int left, int right) {
//   childstruct partitionAroundMe = theChildren[left];
//   
//   left--;
//   right++;
//
//   while (true) {
//      do {
//         right--;
//      } while (theChildren[right] > partitionAroundMe);
//
//      do {
//         left++;
//      } while (theChildren[left] < partitionAroundMe);
//
//      if (left < right) {
//         // swap theChildren[left] and theChildren[right]
//         childstruct temp = theChildren[left];
//         theChildren[left] = theChildren[right];
//         theChildren[right] = temp;
//      }
//      else
//         return right;
//   }
//}

//template <class USERNODEDATA>
//void where4tree<USERNODEDATA>::QuicksortChildren(int left, int right) {
//#ifdef PARADYN
//   // the where axis test program does use paradyn's libthread, hence
//   // the #ifdef...
//
//   thr_check_stack(); // threadP.h
//#endif
//   
//   while (left < right) {
//      int partitionIndex = partitionChildren(left, right);
//      assert(partitionIndex >= left);
//      assert(partitionIndex <= right);
//
//      QuicksortChildren(left, partitionIndex);
//      left = partitionIndex + 1;
//         // gets rid of tail recursion (right stays unchanged)
//   }
//}

template <class USERNODEDATA>
vector<USERNODEDATA> where4tree<USERNODEDATA>::getSelections() const {
   // NOTE: Things would be faster if this function returned void and
   // takes in a reference to a vector<USERNODEDATA> which is appended to
   // in-place...

   vector<USERNODEDATA> result; // initially empty
      
   if (theRootNode.getHighlighted())
      result += theUserNodeData;

   for (int i=0; i < theChildren.size(); i++)
      result += theChildren[i].theTree->getSelections();

   return result;
}

template <class USERNODEDATA>
void where4tree<USERNODEDATA>::recursiveClearSelections() {
   theRootNode.unhighlight();
  
   unsigned numChildren = theChildren.size();
   for (int i=0; i < numChildren; i++)
      theChildren[i].theTree->recursiveClearSelections();
}
