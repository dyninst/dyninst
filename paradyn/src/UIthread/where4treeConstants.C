// where4treeConstants.C
// Ariel Tamches

/* $Log: where4treeConstants.C,v $
/* Revision 1.1  1995/07/17 04:59:06  tamches
/* First version of the new where axis
/*
 */

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>
#include "where4treeConstants.h"

void tclpanic(Tcl_Interp *interp, const char *msg) {
   cout << "msg: " << interp->result << endl;
   cout.flush();
   exit(5);
}

where4TreeConstants::where4TreeConstants(Tcl_Interp *interp,
					 Tk_Window theWindow) {
   assert(theWindow != NULL);

   display = Tk_Display(theWindow);

   this->theTkWindow = theWindow;
   masterWindow = Tk_WindowId(theWindow);

   offscreenPixmap = XCreatePixmap(display, masterWindow,
				   1, // dummy width (for now)
				   1, // dummy height (for now)
				   Tk_Depth(theWindow));
            
   listboxHeightWhereSBappears = Tk_Height(theTkWindow) * 8 / 10; // 80%

   rootTextFontStruct = XLoadQueryFont(display, "*-Helvetica-*-r-*-14-*");
   if (NULL == rootTextFontStruct) {
      cerr << "Cannot find Helvetica 14 font!" << endl;
      exit(5);
   }

   grayColor = Tk_GetColor(interp, theWindow, Tk_GetUid("gray"));
   if (grayColor == NULL)
      tclpanic(interp, "could not allocate gray color");

   pinkColor = Tk_GetColor(interp, theWindow, Tk_GetUid("pink"));
   if (pinkColor == NULL)
      tclpanic(interp, "could not allocate pink color");

   blackColor = Tk_GetColor(interp, theWindow, Tk_GetUid("black"));
   if (blackColor == NULL)
      tclpanic(interp, "could not allocate black color");

   cornflowerBlueColor = Tk_GetColor(interp, theWindow, Tk_GetUid("cornflowerBlue"));
   if (cornflowerBlueColor == NULL)
      tclpanic(interp, "could not allocate cornflower blue color");


   XGCValues values;

   // Erasing:
   values.foreground = grayColor->pixel;
   erasingGC = XCreateGC(display, masterWindow,
			 GCForeground,
			 &values);

   // Root Border
   rootNodeBorder = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("pink"));
   if (rootNodeBorder == NULL) {
      cerr << interp->result << endl;
      exit(5);
   }

   // Root Text
   values.foreground = blackColor->pixel;
   values.font = rootTextFontStruct->fid;
   rootItemTextGC = XCreateGC(display, masterWindow,
			      GCForeground | GCFont,
			      &values);

   values.foreground = pinkColor->pixel;
   values.font = rootTextFontStruct->fid;
   highlightedRootItemTextGC = XCreateGC(display, masterWindow,
					 GCForeground | GCFont,
					 &values);

   // Root Rectangle
   values.foreground = pinkColor->pixel;
   values.background = blackColor->pixel;
   values.line_width = 2;
   values.join_style = JoinRound;
   rootItemRectGC = XCreateGC(display, masterWindow,
			      GCForeground | GCBackground | GCLineWidth | GCJoinStyle,
			      &values);

   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.line_width = 2;
   values.join_style = JoinRound;
   highlightedRootItemRectGC = XCreateGC(display, masterWindow,
					 GCForeground | GCBackground | GCLineWidth | GCJoinStyle,
					 &values);

   // Master listbox Ray
   values.foreground = cornflowerBlueColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   listboxRayGC = XCreateGC(display, masterWindow,
				  GCForeground | GCBackground | GCLineWidth | GCCapStyle,
				  &values);

   // Child Ray				  
   //values.foreground = greenColor->pixel;
   values.foreground = pinkColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   subchildRayGC = XCreateGC(display, masterWindow,
			     GCForeground | GCBackground | GCLineWidth | GCCapStyle,
			     &values);

   // Master listbox Font				  
   listboxFontStruct = XLoadQueryFont(display, "*-Helvetica-*-r-*-12-*");
   if (NULL == rootTextFontStruct) {
      cerr << "Cannot find Helvetica 12 font!" << endl;
      exit(5);
   }

   // Master listbox Borders
   listboxBorder = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("cornflowerBlue"));
   if (rootNodeBorder == NULL) {
      cerr << interp->result << endl;
      exit(5);
   }

   listboxScrollbarBorderNormal = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("gray"));
   if (listboxScrollbarBorderNormal == NULL) {
      cerr << interp->result << endl;
      exit(5);
   }

   listboxScrollbarBorderDragging = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("gray"));
   if (listboxScrollbarBorderDragging == NULL) {
      cerr << interp->result << endl;
      exit(5);
   }

   // Master listbox Rect
   values.foreground = blackColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 1;
   values.join_style = JoinRound;
   listboxRectangleGC = XCreateGC(display, masterWindow,
					GCForeground | GCBackground | GCLineWidth | GCJoinStyle,
					&values);

   // Master listbox Background
   values.foreground = cornflowerBlueColor->pixel;
   values.font = listboxFontStruct->fid; // for drawing highlighted listbox items
   listboxBackgroundGC = XCreateGC(display, masterWindow,
					 GCForeground | GCFont,
					 &values);

   // Master listbox Text
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.font = listboxFontStruct->fid;
   listboxTextGC = XCreateGC(display, masterWindow,
				   GCForeground | GCBackground | GCFont,
				   &values);

   // Master listbox Triangle
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.join_style = JoinMiter;
   values.fill_style = FillSolid;

   listboxTriangleGC = XCreateGC(display, masterWindow,
				       GCForeground | GCBackground | GCJoinStyle | GCFillStyle,
				       &values);

   // Other listbox integers
   listboxHorizPadBeforeText = XTextWidth(listboxFontStruct,
						"8", 1);
   listboxHorizPadBeforeTriangle = 3 * listboxHorizPadBeforeText / 4;
   listboxTriangleWidth = listboxHorizPadBeforeText;
   listboxTriangleHeight = listboxFontStruct->ascent;
   listboxHorizPadAfterTriangle = listboxHorizPadBeforeTriangle;

   int thePad = listboxFontStruct->ascent / 4;
   if (listboxFontStruct->descent + 1 > thePad)
      thePad = listboxFontStruct->descent + 1;

   listboxVertPadAboveItem = thePad;
   listboxVertPadAfterItemBaseline = thePad;

   // spacing integers:
   horizPixBetweenChildren = 10;
   vertPixParent2ChildTop = 10;
   horizPixlistbox2FirstExpandedChild = 15;
}

where4TreeConstants::~where4TreeConstants() {
   XFreeFont(display, rootTextFontStruct);
   XFreeFont(display, listboxFontStruct);

   XFreeGC(display, erasingGC);
   XFreeGC(display, rootItemTextGC);
   XFreeGC(display, highlightedRootItemTextGC);
   XFreeGC(display, rootItemRectGC);
   XFreeGC(display, highlightedRootItemRectGC);

   XFreeGC(display, listboxRayGC);
   XFreeGC(display, subchildRayGC);

   XFreeGC(display, listboxRectangleGC);
   XFreeGC(display, listboxTextGC);
   XFreeGC(display, listboxTriangleGC);
   XFreeGC(display, listboxBackgroundGC);

   Tk_FreeColor(grayColor);
   Tk_FreeColor(pinkColor);
   Tk_FreeColor(blackColor);
   Tk_FreeColor(cornflowerBlueColor);

   Tk_Free3DBorder(rootNodeBorder);
   Tk_Free3DBorder(listboxBorder);

   XFreePixmap(display, offscreenPixmap);
}
