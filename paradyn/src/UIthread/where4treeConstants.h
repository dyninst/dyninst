// where4treeConstants.h
// Ariel Tamches

/* $Log: where4treeConstants.h,v $
/* Revision 1.1  1995/07/17 04:59:05  tamches
/* First version of the new where axis
/*
 */

#ifndef _WHERE4TREECONSTANTS_H_
#define _WHERE4TREECONSTANTS_H_

#include <iostream.h>

extern "C" {
   #include <X11/Xlib.h>
}

#include "tclclean.h"
#include "tkclean.h"

struct where4TreeConstants {
   Display *display;
   Tk_Window theTkWindow;
   Drawable masterWindow;
   Pixmap   offscreenPixmap;

   // Erasing:
   GC erasingGC;

   // Root Item:   
   XFontStruct *rootTextFontStruct;
   Tk_3DBorder rootNodeBorder;
   GC rootItemTextGC, highlightedRootItemTextGC;
   GC rootItemRectGC, highlightedRootItemRectGC;

   // Rays:
   GC listboxRayGC;
   GC subchildRayGC;

   // Master listbox:
   int listboxHeightWhereSBappears;
      // varies as the window gets resize -- let's say for now that it's
      // always something like 80% of the height of the master canvas window.
   XFontStruct *listboxFontStruct; // if there is to be a title, then there'd be another one of these...
   Tk_3DBorder listboxBorder;
   Tk_3DBorder listboxScrollbarBorderNormal;
   Tk_3DBorder listboxScrollbarBorderDragging;
   GC listboxRectangleGC, listboxTextGC, listboxTriangleGC;
   GC listboxBackgroundGC;
   int listboxHorizPadBeforeText, listboxHorizPadBeforeTriangle;
   int listboxTriangleWidth, listboxTriangleHeight;
   int listboxHorizPadAfterTriangle;
   int listboxVertPadAboveItem, listboxVertPadAfterItemBaseline;

   // Other Spacing issues:
   int horizPixBetweenChildren;
   int vertPixParent2ChildTop; // about 8 pixels
   int horizPixlistbox2FirstExpandedChild; // between

   // Some XColors allocated with Tk_GetColor() [deallocate w/ Tk_FreeColor()]:
   XColor *grayColor, *pinkColor, *blackColor, *cornflowerBlueColor;

   where4TreeConstants(Tcl_Interp *, Tk_Window theWindow);
  ~where4TreeConstants();  // needed to free some resources
  
   void resize() {
      XFreePixmap(display, offscreenPixmap);
      offscreenPixmap = XCreatePixmap(display, masterWindow,
				      Tk_Width(theTkWindow),
				      Tk_Height(theTkWindow),
				      Tk_Depth(theTkWindow));

      listboxHeightWhereSBappears = Tk_Height(theTkWindow) * 8 / 10;
            // 80%
   }
};

#endif
