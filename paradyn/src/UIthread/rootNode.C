/*
 * Copyright (c) 1996-1998 Barton P. Miller
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

// whereAxisRootNode.C
// Ariel Tamches

/* $Id: rootNode.C,v 1.12 2000/07/28 17:22:06 pcroth Exp $ */

#include <assert.h>

#include "common/h/headers.h"
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
      Tk_TextWidth(whereAxis::getRootItemFontStruct(),
                   name.string_of(), name.length()) +
                   horizPad + borderPix;

   Tk_FontMetrics rootItemFontMetrics; // filled in
   Tk_GetFontMetrics(whereAxis::getRootItemFontStruct(), &rootItemFontMetrics);
   pixHeightAsRoot = borderPix + vertPad +
                     rootItemFontMetrics.ascent +
                     rootItemFontMetrics.descent +
                     vertPad + borderPix;

   pixWidthAsListboxItem = Tk_TextWidth(whereAxis::getListboxItemFontStruct(),
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
   Tk_FontMetrics rootItemFontMetrics; // filled in
   Tk_GetFontMetrics(whereAxis::getRootItemFontStruct(), &rootItemFontMetrics);
   
   const int textLeft = boxLeft + borderPix + horizPad;
   const int textBaseLine = root_topy + borderPix + vertPad +
                            rootItemFontMetrics.ascent - 1;

	Tk_DrawChars(Tk_Display(theTkWindow),
		theDrawable,
		whereAxis::getRootItemTextGC(),
		whereAxis::getRootItemFontStruct(),
		name.string_of(), name.length(),
		textLeft, textBaseLine );
}

GC whereAxisRootNode::getGCforListboxRay(const whereAxisRootNode &, // parent
					 const whereAxisRootNode & // 1st child
					 ) {
   // a static member function
   return whereAxis::getGCforListboxRay();
}

GC whereAxisRootNode::getGCforNonListboxRay(const whereAxisRootNode &, // parent
					    const whereAxisRootNode &  // 1st child
					    ) {
   // a static member function
   return whereAxis::getGCforNonListboxRay();
}

void whereAxisRootNode::prepareForDrawingListboxItems(Tk_Window theTkWindow,
						      XRectangle &listboxBounds) {
#if !defined(i386_unknown_nt4_0)
  XSetClipRectangles(Tk_Display(theTkWindow), whereAxis::getListboxItemGC(),
		     0, 0, &listboxBounds, 1, YXBanded);
  Tk_3DBorder thisStyleTk3DBorder = whereAxis::getListboxItem3DBorder();
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
	// TODO - is this needed?
#endif // !defined(i386_unknown_nt4_0)
}

void whereAxisRootNode::doneDrawingListboxItems(Tk_Window theTkWindow) {
   XSetClipMask(Tk_Display(theTkWindow), whereAxis::getListboxItemGC(), None);
   Tk_3DBorder thisStyleTk3DBorder = whereAxis::getListboxItem3DBorder();
   
   XSetClipMask(Tk_Display(theTkWindow),
		Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder,TK_3D_LIGHT_GC),
		None);
   XSetClipMask(Tk_Display(theTkWindow),
		Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_DARK_GC),
		None);
   XSetClipMask(Tk_Display(theTkWindow),
		Tk_3DBorderGC(theTkWindow, thisStyleTk3DBorder, TK_3D_FLAT_GC),
		None);
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

	Tk_DrawChars(Tk_Display(theTkWindow),
		theDrawable,
		whereAxis::getListboxItemGC(),
		whereAxis::getRootItemFontStruct(),	// is this correct?
		name.string_of(), name.length(),
		textLeft, textBaseline );
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
