/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// shgRootNode.C
// Ariel Tamches

/* $Id: shgRootNode.C,v 1.16 2005/01/28 18:12:04 legendre Exp $ */

#include "shg.h"
#include "shgRootNode.h"

int shgRootNode::borderPix = 3;
int shgRootNode::horizPad = 3;
int shgRootNode::vertPad = 2;

void shgRootNode::initialize(unsigned iId,
			     bool iActive, shgRootNode::evaluationState iEvalState,
			     shgRootNode::refinement iRefinement,
			     bool iShadow,
			     const pdstring &iLabel, const pdstring &iFullInfo,
			     bool iHidden) {
   hidden = iHidden;

   label = iLabel;

   const char* mfl = getenv("PARADYN_MAX_FUNCTION_LENGTH");
   mfl = mfl ? mfl : "0";
   int abbrevLength = atoi(mfl);
   if(label.length() > (unsigned)abbrevLength && (unsigned)abbrevLength != 0) {
       abbrevLabel = label.substr(0, abbrevLength / 2);
       abbrevLabel += pdstring("...");
       abbrevLabel += label.substr(label.length() - (abbrevLength / 2), label.length());
   } else {
       abbrevLabel = label;
   }

   fullInfo = iFullInfo;

   id = iId;
   highlighted = false;

   active = iActive;
   evalState = iEvalState;
   deferred = false;

   theRefinement = iRefinement;

   shadowNode = iShadow;

   Tk_Font rootItemFontStruct = shg::getRootItemFontStruct(shadowNode);

   pixWidthAsRoot = borderPix + horizPad // static members
                  + Tk_TextWidth(rootItemFontStruct,
                                 abbrevLabel.c_str(), abbrevLabel.length()) +
                  + horizPad + borderPix;

   Tk_FontMetrics rootItemFontMetrics;
   Tk_GetFontMetrics(rootItemFontStruct, &rootItemFontMetrics);
   pixHeightAsRoot = borderPix + vertPad +
                     rootItemFontMetrics.ascent +
                     rootItemFontMetrics.descent +
                     vertPad + borderPix;

   Tk_Font listboxItemFontStruct = shg::getListboxItemFontStruct(shadowNode);
   pixWidthAsListboxItem = Tk_TextWidth(listboxItemFontStruct,
                                       abbrevLabel.c_str(), abbrevLabel.length());
}

shgRootNode::shgRootNode(unsigned iId,
			 bool iActive, shgRootNode::evaluationState iEvalState,
			 shgRootNode::refinement iRefinement,
			 bool iShadow,
			 const pdstring &iLabel, const pdstring &iFullInfo,
			 bool iHidden) {
   initialize(iId, iActive, iEvalState, iRefinement,
	      iShadow, iLabel, iFullInfo, iHidden);
}

shgRootNode::shgRootNode(unsigned iId, bool iActive,
			 shgRootNode::evaluationState iEvalState,
			 bool iShadow,
			 const pdstring &iLabel, const pdstring &iFullInfo,
			 bool iHidden) {
   initialize(iId, iActive, iEvalState, ref_undefined,
	      iShadow, iLabel, iFullInfo, iHidden);
}

shgRootNode::shgRootNode(const shgRootNode &src) : label(src.label),
                                                   abbrevLabel(src.abbrevLabel),
                                                   fullInfo(src.fullInfo) {
   hidden = src.hidden;
   id = src.id;
   highlighted = src.highlighted;
   active = src.active;
   evalState = src.evalState;
   deferred = src.deferred;
   theRefinement = src.theRefinement;
   shadowNode = src.shadowNode;
   pixWidthAsRoot = src.pixWidthAsRoot;
   pixHeightAsRoot = src.pixHeightAsRoot;
   pixWidthAsListboxItem = src.pixWidthAsListboxItem;
}

shgRootNode &shgRootNode::operator=(const shgRootNode &src) {
   label = src.label;
   abbrevLabel = src.abbrevLabel;
   
   fullInfo = src.fullInfo;
   
   hidden = src.hidden;
   id = src.id;
   highlighted = src.highlighted;
   active = src.active;
   evalState = src.evalState;
   deferred = src.deferred;
   theRefinement = src.theRefinement;
   shadowNode = src.shadowNode;
   pixWidthAsRoot = src.pixWidthAsRoot;
   pixHeightAsRoot = src.pixHeightAsRoot;
   pixWidthAsListboxItem = src.pixWidthAsListboxItem;

   return *this;
}

bool shgRootNode::configStyle(bool newActive,
                                evaluationState newEvalState,
                                bool newDeferred)
{
   // returns true iff any changes.  Does not redraw.
   if ((active == newActive) && 
        (evalState == newEvalState) &&
        (deferred == newDeferred) )
    {
      return false;
    }

   active = newActive;
   evalState = newEvalState;
   deferred = newDeferred;
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
   Tk_FontMetrics rootItemFontMetrics; // filled in
   Tk_GetFontMetrics(shg::getRootItemFontStruct(shadowNode), &rootItemFontMetrics);
   
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            rootItemFontMetrics.ascent - 1;

	Tk_DrawChars(Tk_Display(theTkWindow),
		theDrawable,
		shg::getRootItemTextGC(active, shadowNode, deferred),
		shg::getRootItemFontStruct(shadowNode),
		abbrevLabel.c_str(), abbrevLabel.length(),
		textLeft, textBaseLine );
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
#if !defined(i386_unknown_nt4_0)
   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, false, false), // inactive, not shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, true, false), // inactive, shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, false, true), // deferred, not shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(false, true, true), // deferred, shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(true, false, false), // active, not shadow
		      0, 0, &listboxBounds, 1, YXBanded);

   XSetClipRectangles(Tk_Display(theTkWindow),
		      shg::getListboxItemGC(true, true, false), // active, shadow
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
#else // !defined(i386_unknown_nt4_0)
	// TODO - implement clipping support (?)
#endif // !defined(i386_unknown_nt4_0)
}

void shgRootNode::doneDrawingListboxItems(Tk_Window theTkWindow) {
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, false, false),
		None); // inactive, not shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, true, false),
		None); // inactive, shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, false, true),
		None); // deferred, not shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(false, true, true),
		None); // deferred, shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(true, false, false),
		None); // active, not shadow
   XSetClipMask(Tk_Display(theTkWindow), shg::getListboxItemGC(true, true, false),
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

	Tk_DrawChars(Tk_Display(theTkWindow),
		theDrawable,
		shg::getListboxItemGC(active, shadowNode, deferred),
		shg::getRootItemFontStruct(shadowNode),
		abbrevLabel.c_str(), abbrevLabel.length(),
		textLeft, textBaseline);
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
