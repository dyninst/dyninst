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

// $Id: callGraphRootNode.C,v 1.5 2003/07/15 22:46:07 schendel Exp $

#include <assert.h>
#include "callGraphRootNode.h"
#include "callGraphDisplay.h" // for some of its static vrbles/member functions


int callGraphRootNode::borderPix = 3;
int callGraphRootNode::horizPad = 3;
int callGraphRootNode::vertPad = 2;

callGraphRootNode::callGraphRootNode(resourceHandle iUniqueId, 
				     const pdstring &ishortName, 
				     const pdstring &ifullName, 
				     const bool recursiveFlag,
				     const bool shadowFlag) :
  shortName(ishortName), fullName(ifullName), 
  currentName(const_cast<class pdstring *>(&ishortName)) {
  
  shortNameIsDisplayed = true;
  uniqueId = iUniqueId;
  highlighted = false;
  isRecursive = recursiveFlag;
  isShadowNode = shadowFlag;
  
  Tk_Font rootItemFontStruct = 
    callGraphDisplay::getRootItemFontStruct(isShadowNode);
  pixWidthAsRoot = borderPix + horizPad +
    Tk_TextWidth(rootItemFontStruct, currentName->c_str(), 
		 currentName->length()) +
		 horizPad+borderPix;
  
  Tk_FontMetrics rootItemFontMetrics; // filled in
  Tk_GetFontMetrics(rootItemFontStruct, &rootItemFontMetrics);
  pixHeightAsRoot = borderPix + vertPad + rootItemFontMetrics.ascent +
    rootItemFontMetrics.descent + vertPad + borderPix;
  
  pixWidthAsListboxItem = 
    Tk_TextWidth(callGraphDisplay::getListboxItemFontStruct(isShadowNode),
		 currentName->c_str(), currentName->length());
}


void callGraphRootNode::showFullName(){
  if(!shortNameIsDisplayed)
    return;
  shortNameIsDisplayed = false;
  currentName = &fullName;
  pixWidthAsRoot = borderPix + horizPad +
    Tk_TextWidth(callGraphDisplay::getRootItemFontStruct(isShadowNode),
		 currentName->c_str(), currentName->length()) +
    horizPad + borderPix;
  pixWidthAsListboxItem = 
    Tk_TextWidth(callGraphDisplay::getListboxItemFontStruct(isShadowNode),
		 currentName->c_str(), currentName->length());
}

void callGraphRootNode::showShortName(){
  if(shortNameIsDisplayed)
    return;
  shortNameIsDisplayed = true;
  currentName = &shortName;
  pixWidthAsRoot = borderPix + horizPad +
    Tk_TextWidth(callGraphDisplay::getRootItemFontStruct(isShadowNode),
		 currentName->c_str(), currentName->length()) +
    horizPad + borderPix;
  pixWidthAsListboxItem = 
    Tk_TextWidth(callGraphDisplay::getListboxItemFontStruct(isShadowNode),
		 currentName->c_str(), currentName->length());
}

void callGraphRootNode::drawAsRoot(Tk_Window theTkWindow,
				   int theDrawable,//may be an offscreen pixmap
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
		      callGraphDisplay::getRootItemTk3DBorder(isRecursive),
		      boxLeft, root_topy,
		      pixWidthAsRoot, pixHeightAsRoot,
		      borderPix,
		      highlighted ? highlightedRelief : normalRelief);

   // Third, draw the text
   Tk_FontMetrics rootItemFontMetrics; // filled in
   Tk_GetFontMetrics(callGraphDisplay::getRootItemFontStruct(isShadowNode), 
		     &rootItemFontMetrics);
   
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            rootItemFontMetrics.ascent - 1;

    Tk_DrawChars(Tk_Display(theTkWindow),theDrawable,
		 callGraphDisplay::getRootItemTextGC(isShadowNode),
		 callGraphDisplay::getRootItemFontStruct(isShadowNode),
                currentName->c_str(), currentName->length(),
                textLeft, textBaseLine );

}

GC callGraphRootNode::getGCforListboxRay(const callGraphRootNode &, // parent
					 const callGraphRootNode & // 1st child
					 ) {
   // a static member function
  
   return callGraphDisplay::getGCforListboxRay();
}

GC callGraphRootNode::getGCforNonListboxRay(const callGraphRootNode &,// parent
					    const callGraphRootNode & // 1st child
) {
   // a static member function
   return callGraphDisplay::getGCforNonListboxRay();
}

void callGraphRootNode::prepareForDrawingListboxItems(Tk_Window theTkWindow,
						   XRectangle &listboxBounds){
  
#if !defined(i386_unknown_nt4_0)
  //not shadow node
  XSetClipRectangles (Tk_Display(theTkWindow),
		      callGraphDisplay::getListboxItemGC(false),
		      0, 0, &listboxBounds, 1, YXBanded);
  //shadow node
  XSetClipRectangles (Tk_Display(theTkWindow),
		      callGraphDisplay::getListboxItemGC(true),
		      0, 0, &listboxBounds, 1, YXBanded);
  //Clip non-recursive node colors
  Tk_3DBorder thisStyleTk3DBorder = callGraphDisplay::getListboxItemTk3DBorder(false);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow,thisStyleTk3DBorder,TK_3D_LIGHT_GC),
		     0, 0, &listboxBounds, 1, YXBanded);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,TK_3D_DARK_GC),
		     0, 0, &listboxBounds, 1, YXBanded);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,TK_3D_FLAT_GC),
		     0, 0, &listboxBounds, 1, YXBanded);
  //Clip recursive node colors
  thisStyleTk3DBorder = callGraphDisplay::getListboxItemTk3DBorder(true);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow,thisStyleTk3DBorder,TK_3D_LIGHT_GC),
		     0, 0, &listboxBounds, 1, YXBanded);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,TK_3D_DARK_GC),
		     0, 0, &listboxBounds, 1, YXBanded);
  XSetClipRectangles(Tk_Display(theTkWindow),
		 Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,TK_3D_FLAT_GC),
		 0, 0, &listboxBounds, 1, YXBanded);

#else // !defined(i386_unknown_nt4_0)
	// TODO - implement clipping support (?)
#endif // !defined(i386_unknown_nt4_0)
}

void callGraphRootNode::doneDrawingListboxItems(Tk_Window theTkWindow) {

#if !defined(i386_unknown_nt4_0)
  //for non-shadow nodes
  XSetClipMask(Tk_Display(theTkWindow), 
	       callGraphDisplay::getListboxItemGC(false), None);
  //for shadow nodes
  XSetClipMask(Tk_Display(theTkWindow), 
	       callGraphDisplay::getListboxItemGC(true), None);
  
  //For non recursive nodes
  Tk_3DBorder thisStyleTk3DBorder = callGraphDisplay::getListboxItemTk3DBorder(false);

  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_LIGHT_GC),
	       None);
  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_DARK_GC),
	       None);
  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_FLAT_GC),
	       None);
  //For recursive nodes
  thisStyleTk3DBorder = callGraphDisplay::getListboxItemTk3DBorder(true);

  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_LIGHT_GC),
	       None);
  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_DARK_GC),
	       None);
  XSetClipMask(Tk_Display(theTkWindow),
	       Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_FLAT_GC),
	       None);
#else 
  //DO we need to do anything here?
#endif
}

void callGraphRootNode::drawAsListboxItem(Tk_Window theTkWindow,
					  int theDrawable, int boxLeft,
					  int boxTop, int boxWidth,
					  int boxHeight, int textLeft,
					  int textBaseline) const{

   Tk_Fill3DRectangle(theTkWindow, theDrawable,
		      callGraphDisplay::getListboxItemTk3DBorder(isRecursive),
	       // for a shg-like class, this routine would take in a parameter
	       // and return a varying border.  But the where axis doesn't need
               // such a feature.
		      boxLeft, boxTop,
		      boxWidth, boxHeight,
		      1, // 2 also looks pretty good; 3 doesn't
		      highlighted ? TK_RELIEF_SUNKEN : TK_RELIEF_RAISED);

   Tk_DrawChars(Tk_Display(theTkWindow),theDrawable,
		callGraphDisplay::getListboxItemGC(isShadowNode),
		callGraphDisplay::getRootItemFontStruct(isShadowNode),
		currentName->c_str(), currentName->length(),
		textLeft, textBaseline );          

}


int callGraphRootNode::pointWithinAsRoot(int xpix, int ypix,
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
