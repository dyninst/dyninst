// shgRootNode.C
// Ariel Tamches

/* $Log: shgRootNode.C,v $
/* Revision 1.4  1996/02/15 23:13:20  tamches
/* added code to properly support why vs. where axis refinement
/*
 * Revision 1.3  1996/01/23 19:48:10  tamches
 * added shadow node features
 *
 * Revision 1.2  1996/01/11 23:42:40  tamches
 * there are now 6 node styles
 *
 * Revision 1.1  1995/10/17 22:08:53  tamches
 * initial version, for the new search history graph
 *
 */

#include "shg.h"
#include "shgRootNode.h"

int shgRootNode::borderPix = 3;
int shgRootNode::horizPad = 3;
int shgRootNode::vertPad = 2;

void shgRootNode::initialize(unsigned iId,
			     bool iActive, shgRootNode::evaluationState iEvalState,
			     shgRootNode::refinement iRefinement,
			     bool iShadow,
			     const string &iLabel, const string &iFullInfo) {
   label = iLabel;
   fullInfo = iFullInfo;

   id = iId;
   highlighted = false;

   active = iActive;
   evalState = iEvalState;

   theRefinement = iRefinement;

   shadowNode = iShadow;

   XFontStruct *rootItemFontStruct = shg::getRootItemFontStruct(shadowNode);

   pixWidthAsRoot = borderPix + horizPad // static members
                  + XTextWidth(rootItemFontStruct,
			       label.string_of(), label.length()) +
                  + horizPad + borderPix;

   pixHeightAsRoot = borderPix + vertPad +
                     rootItemFontStruct->ascent +
                     rootItemFontStruct->descent +
                     vertPad + borderPix;

   XFontStruct *listboxItemFontStruct = shg::getListboxItemFontStruct(shadowNode);
   pixWidthAsListboxItem = XTextWidth(listboxItemFontStruct,
				      label.string_of(), label.length());
}

shgRootNode::shgRootNode(unsigned iId,
			 bool iActive, shgRootNode::evaluationState iEvalState,
			 shgRootNode::refinement iRefinement,
			 bool iShadow,
			 const string &iLabel, const string &iFullInfo) {
   initialize(iId, iActive, iEvalState, iRefinement,
	      iShadow, iLabel, iFullInfo);
}

shgRootNode::shgRootNode(unsigned iId, bool iActive,
			 shgRootNode::evaluationState iEvalState,
			 bool iShadow,
			 const string &iLabel, const string &iFullInfo) {
   initialize(iId, iActive, iEvalState, ref_undefined,
	      iShadow, iLabel, iFullInfo);
}

shgRootNode::shgRootNode(const shgRootNode &src) : label(src.label),
                                                   fullInfo(src.fullInfo) {
   id = src.id;
   highlighted = src.highlighted;
   active = src.active;
   evalState = src.evalState;
   theRefinement = src.theRefinement;
   shadowNode = src.shadowNode;
   pixWidthAsRoot = src.pixWidthAsRoot;
   pixHeightAsRoot = src.pixHeightAsRoot;
   pixWidthAsListboxItem = src.pixWidthAsListboxItem;
}

bool shgRootNode::configStyle(bool newActive, evaluationState newEvalState) {
   // returns true iff any changes.  Does not redraw.
   if (active == newActive && evalState == newEvalState)
      return false;

   active = newActive;
   evalState = newEvalState;
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
		      shg::getRootItemTk3DBorder(evalState),
		      boxLeft, root_topy,
		      pixWidthAsRoot, pixHeightAsRoot,
		      borderPix,
		      highlighted ? highlightedRelief : normalRelief);

   // Third, draw the text
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            shg::getRootItemFontStruct(shadowNode)->ascent - 1;

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       shg::getRootItemTextGC(active, shadowNode),
	       textLeft, textBaseLine,
	       label.string_of(), label.length());
}

GC shgRootNode::getGCforListboxRay(const shgRootNode &,
				   const shgRootNode &firstChild) {
   // return GC to be used in an XDrawLine call from "parent" down to the
   // listbox of its children; "firstChild" is the node data for the first
   // such child.
   return shg::getGCforListboxRay(firstChild.theRefinement);
}

GC shgRootNode::getGCforNonListboxRay(const shgRootNode &,
				      const shgRootNode &child) {
   return shg::getGCforNonListboxRay(child.theRefinement);
}

void shgRootNode::prepareForDrawingListboxItems(Tk_Window theTkWindow,
						XRectangle &listboxBounds) {
   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, false), // inactive, not shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, true), // inactive, shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(true, false), // active, not shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(true, true), // active, shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   for (unsigned theStyle=es_never; theStyle <= es_false; theStyle++) {
      Tk_3DBorder thisStyleTk3DBorder = shg::getListboxItemTk3DBorder((evaluationState)theStyle);

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
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, false),
		None); // inactive, not shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, true),
		None); // inactive, shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(true, false),
		None); // active, not shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(true, true),
		None); // active, shadow

   for (unsigned theStyle=es_never; theStyle <= es_false; theStyle++) {
      Tk_3DBorder thisStyleTk3DBorder = shg::getListboxItemTk3DBorder((evaluationState)theStyle);

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
		      shg::getListboxItemTk3DBorder(evalState),
                         // note how this is a function of style
		      boxLeft, boxTop, boxWidth, boxHeight,
		      1, // 2 is not bad looking
		      highlighted ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED);

   XDrawString(Tk_Display(theTkWindow), theDrawable,
	       shg::getListboxItemGC(active, shadowNode),
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
