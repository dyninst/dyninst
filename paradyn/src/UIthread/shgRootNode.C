// shgRootNode.C
// Ariel Tamches

/* $Log: shgRootNode.C,v $
/* Revision 1.2  1996/01/11 23:42:40  tamches
/* there are now 6 node styles
/*
 * Revision 1.1  1995/10/17 22:08:53  tamches
 * initial version, for the new search history graph
 *
 */

#include "shg.h"
#include "shgRootNode.h"

int shgRootNode::borderPix = 3;
int shgRootNode::horizPad = 3;
int shgRootNode::vertPad = 2;

shgRootNode::shgRootNode(unsigned iId, shgRootNode::style iStyle,
			 const string &iLabel, const string &iFullInfo) :
   			    label(iLabel),
			    fullInfo(iFullInfo) {
   id = iId;
   highlighted = false;

   theStyle = iStyle;

   pixWidthAsRoot = borderPix + horizPad // static members
                  + XTextWidth(shg::getRootItemFontStruct(),
			       label.string_of(), label.length()) +
                  + horizPad + borderPix;

   pixHeightAsRoot = borderPix + vertPad +
                     shg::getRootItemFontStruct()->ascent +
                     shg::getRootItemFontStruct()->descent +
                     vertPad + borderPix;

   pixWidthAsListboxItem = XTextWidth(shg::getListboxItemFontStruct(),
				      label.string_of(), label.length());
}

bool shgRootNode::configStyle(style newStyle) {
   // returns true iff any changes.  Does not redraw.
   if (theStyle == newStyle)
      return false;

   theStyle = newStyle;
   return true;
}

void shgRootNode::drawAsRoot(Tk_Window theTkWindow,
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
		      shg::getRootItemTk3DBorder(theStyle),
		      boxLeft, root_topy,
		      pixWidthAsRoot, pixHeightAsRoot,
		      borderPix,
		      highlighted ? highlightedRelief : normalRelief);

   // Third, draw the text
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            shg::getRootItemFontStruct()->ascent - 1;

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       shg::getRootItemTextGC(),
	       textLeft, textBaseLine,
	       label.string_of(), label.length());
}

void shgRootNode::prepareForDrawingListboxItems(Tk_Window theTkWindow,
						XRectangle &listboxBounds) {
   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(),
		      0, 0, &listboxBounds, 1, YXBanded);

   for (unsigned theStyle=InactiveUnknown; theStyle <= InactiveTrue; theStyle++) {
      Tk_3DBorder thisStyleTk3DBorder = shg::getListboxItemTk3DBorder((style)theStyle);

      XSetClipRectangles(Tk_Display(theTkWindow),
			 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,  TK_3D_LIGHT_GC),
			 0, 0, &listboxBounds, 1, YXBanded);
      XSetClipRectangles(Tk_Display(theTkWindow),
			 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,  TK_3D_DARK_GC),
			 0, 0, &listboxBounds, 1, YXBanded);
      XSetClipRectangles(Tk_Display(theTkWindow),
			 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,  TK_3D_FLAT_GC),
			 0, 0, &listboxBounds, 1, YXBanded);
   }
}

void shgRootNode::doneDrawingListboxItems(Tk_Window theTkWindow) {
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(), None);

   for (unsigned theStyle=InactiveUnknown; theStyle <= InactiveTrue; theStyle++) {
      Tk_3DBorder thisStyleTk3DBorder = shg::getListboxItemTk3DBorder((style)theStyle);

      XSetClipMask(Tk_Display(theTkWindow),
		   Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_LIGHT_GC),
		   None);
      XSetClipMask(Tk_Display(theTkWindow),
		   Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_DARK_GC),
		   None);
      XSetClipMask(Tk_Display(theTkWindow),
		   Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_FLAT_GC),
		   None);
   }
}

void shgRootNode::drawAsListboxItem(Tk_Window theTkWindow, int theDrawable,
				    int boxLeft, int boxTop,
				    int boxWidth, int boxHeight,
				    int textLeft, int textBaseline) const {
   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      shg::getListboxItemTk3DBorder(theStyle),
                         // note how this is a function of style
		      boxLeft, boxTop, boxWidth, boxHeight,
		      1, // 2 is not bad looking
		      highlighted ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED);

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       shg::getListboxItemGC(),
	       textLeft, // boxLeft + tc.listboxHorizPadBeforeText
	       textBaseline, // boxTop + tc.listboxVertPadAboveItem + tc.listboxFontStruct->ascent - 1,
	       label.string_of(), label.length());
}

int shgRootNode::pointWithinAsRoot(int xpix, int ypix,
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
