// whereAxisRootNode.C
// Ariel Tamches

/* $Log: rootNode.C,v $
/* Revision 1.4  1995/10/17 22:05:08  tamches
/* Changed name from rootNode to whereAxisRootNode
/* Added pixWidthAsListboxItem and drawAsListboxItem,
/* along the lines of shgRootNode.C
/*
 * Revision 1.3  1995/09/20 01:18:03  tamches
 * minor cleanifications hardly worth mentioning
 *
 * Revision 1.2  1995/07/18  03:41:19  tamches
 * Added ctrl-double-click feature for selecting/unselecting an entire
 * subtree (nonrecursive).  Added a "clear all selections" option.
 * Selecting the root node now selects the entire program.
 *
 * Revision 1.1  1995/07/17  04:58:57  tamches
 * First version of the new where axis
 *
 */

#include <assert.h>

#include "whereAxis.h" // for some of its static vrbles / member functions
#include "rootNode.h" // change to whereAxisRootNode.h some day

int whereAxisRootNode::borderPix = 3;
int whereAxisRootNode::horizPad = 3;
int whereAxisRootNode::vertPad = 2;

whereAxisRootNode::whereAxisRootNode(resourceHandle iUniqueId, const string &initStr) :
                                  name(initStr) {
   uniqueId = iUniqueId;
   highlighted = false;

   pixWidthAsRoot = borderPix + horizPad +
                    XTextWidth(&whereAxis::getRootItemFontStruct(),
			       name.string_of(), name.length()) +
                    horizPad + borderPix;

   pixHeightAsRoot = borderPix + vertPad +
                     whereAxis::getRootItemFontStruct().ascent +
		     whereAxis::getRootItemFontStruct().descent +
                     vertPad + borderPix;

   pixWidthAsListboxItem = XTextWidth(&whereAxis::getListboxItemFontStruct(),
				      name.string_of(), name.length());
}

void whereAxisRootNode::drawAsRoot(Tk_Window theTkWindow,
				   int theDrawable, // may be an offscreen pixmap
				   int root_middlex, int root_topy) const {
   // First, some quick & dirty clipping:
   const int minWindowX = 0;
   const int maxWindowX = Tk_Width(theTkWindow) - 1;
   const int minWindowY = 0;
   const int maxWindowY = Tk_Height(theTkWindow) - 1;

   if (root_topy > maxWindowY)
      return;
   if (root_topy + pixHeightAsRoot - 1 < minWindowY)
      return;

   const int boxLeft = root_middlex - (pixWidthAsRoot / 2);

   if (boxLeft > maxWindowX)
      return;
   if (boxLeft + pixWidthAsRoot - 1 < minWindowX)
      return;

   const int normalRelief = TK_RELIEF_GROOVE;
   const int highlightedRelief = TK_RELIEF_SUNKEN;

   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      whereAxis::getRootItemTk3DBorder(),
		      boxLeft, root_topy,
		      pixWidthAsRoot, pixHeightAsRoot,
		      borderPix,
		      highlighted ? highlightedRelief : normalRelief);

   // Third, draw the text
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            whereAxis::getRootItemFontStruct().ascent - 1;

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       // tc.rootItemTextGC,
	       whereAxis::getRootItemTextGC(),
	       textLeft, textBaseLine,
	       name.string_of(), name.length());
}

void whereAxisRootNode::prepareForDrawingListboxItems(Tk_Window theTkWindow,
						      XRectangle &listboxBounds) {
   XSetClipRectangles(Tk_Display(theTkWindow), whereAxis::getListboxItemGC(),
		      0, 0, &listboxBounds, 1, YXBanded);
}

void whereAxisRootNode::doneDrawingListboxItems(Tk_Window theTkWindow) {
   XSetClipMask(Tk_Display(theTkWindow), whereAxis::getListboxItemGC(), None);
}

void whereAxisRootNode::drawAsListboxItem(Tk_Window theTkWindow,
					  int theDrawable,
					  int boxLeft, int boxTop,
					  int boxWidth, int boxHeight,
					  int textLeft, int textBaseline) const {
   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      whereAxis::getListboxItem3DBorder(),
		         // for a shg-like class, this routine would take in a parameter
		         // and return a varying border.  But the where axis doesn't need
                         // such a feature.
		      boxLeft, boxTop,
		      boxWidth, boxHeight,
		      1, // 2 also looks pretty good; 3 doesn't
		      highlighted ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED);

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       whereAxis::getListboxItemGC(),
	       textLeft, // boxLeft + tc.listboxHorizPadBeforeText
	       textBaseline, // boxTop + tc.listboxVertPadAboveItem + tc.listboxFontStruct->ascent - 1,
	       name.string_of(), name.length());
}


int whereAxisRootNode::pointWithinAsRoot(int xpix, int ypix,
					 int root_centerx, int root_topy) const {
   // return values:
   // 1 -- yes
   // 2 -- no, point is above the root
   // 3 -- no, point is below root
   // 4 -- no, point is to the left of root
   // 5 -- no, point is to the right of root
   
   assert(xpix >= 0 && ypix >= 0);
   
   if (ypix < root_topy) return 2;

   const int root_bottomy = root_topy + pixHeightAsRoot - 1;
   if (ypix > root_bottomy) return 3;

   const int root_leftx = root_centerx - pixWidthAsRoot / 2;
   if (xpix < root_leftx) return 4;

   const int root_rightx = root_leftx + pixWidthAsRoot - 1;
   if (xpix > root_rightx) return 5;

   assert(xpix >= root_leftx && xpix <= root_rightx);
   assert(ypix >= root_topy && ypix <= root_bottomy);
   return 1; // bingo
}
