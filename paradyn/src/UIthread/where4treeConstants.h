// where4treeConstants.h
// Ariel Tamches

/* $Log: where4treeConstants.h,v $
/* Revision 1.3  1996/04/01 22:33:34  tamches
/* Added X visibility event features
/*
 * Revision 1.2  1995/10/17 22:16:26  tamches
 * Removed masterWindow.  Removed rootItemTextGC, rootItemRectGC,
 * and their highlighted variants.  Removed listboxScrollbarBorderDragging.
 * Removed several listbox gc's.
 *
 * Revision 1.1  1995/07/17 04:59:05  tamches
 * First version of the new where axis
 *
 */

#ifndef _WHERE4TREECONSTANTS_H_
#define _WHERE4TREECONSTANTS_H_

#include <iostream.h>

#include "tcl.h"
#include "tk.h"

struct where4TreeConstants {
   Display *display;
   Tk_Window theTkWindow;
   Pixmap   offscreenPixmap;

   // Erasing:
   GC erasingGC;

   // Root Item:   
   XFontStruct *rootTextFontStruct;
   Tk_3DBorder rootNodeBorder;
   GC rootItemTextGC;

   // Rays:
   GC listboxRayGC;
   GC subchildRayGC;

   // Listbox:
   int listboxBorderPix; // 3
   int listboxScrollBarWidth; // 16
   int listboxHeightWhereSBappears;
      // varies as the window gets resize -- let's say for now that it's
      // always something like 80% of the height of the master canvas window.
   XFontStruct *listboxFontStruct;
   Tk_3DBorder listboxBorder;
   Tk_3DBorder listboxScrollbarBorderNormal;
   GC listboxTextGC, listboxTriangleGC;
   int listboxHorizPadBeforeText, listboxHorizPadBeforeTriangle;
   int listboxTriangleWidth, listboxTriangleHeight;
   int listboxHorizPadAfterTriangle;
   int listboxVertPadAboveItem, listboxVertPadAfterItemBaseline;
   GC listboxCopyAreaGC;

   // Other Spacing issues:
   int horizPixBetweenChildren;
   int vertPixParent2ChildTop; // about 8 pixels
   int horizPixlistbox2FirstExpandedChild; // between

   // Some XColors allocated with Tk_GetColor() [deallocate w/ Tk_FreeColor()]:
   XColor *grayColor, *pinkColor, *blackColor, *cornflowerBlueColor;

   bool obscured;
      // true iff the window is partially or fully obscured.

   where4TreeConstants(Tcl_Interp *, Tk_Window theWindow);
  ~where4TreeConstants();  // needed to free some resources
  
   void resize();

   void makeVisibilityUnobscured() {obscured = false;}
   void makeVisibilityPartiallyObscured() {obscured = true;}
   void makeVisibilityFullyObscured() {obscured = true;}
};

#endif
