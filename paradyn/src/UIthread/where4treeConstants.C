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

// where4treeConstants.C
// Ariel Tamches

/* $Log: where4treeConstants.C,v $
/* Revision 1.6  1996/08/16 21:07:41  tamches
/* updated copyright for release 1.1
/*
 * Revision 1.5  1996/04/01 22:33:53  tamches
 * added listboxCopyAreaGC
 *
 * Revision 1.4  1995/11/06 02:42:03  tamches
 * removed tclpanic(), used the one in tkTools.h
 *
 * Revision 1.3  1995/10/17 22:16:58  tamches
 * Removed masterwindow and several unused gc's.
 *
 * Revision 1.2  1995/09/20 01:24:55  tamches
 * fixed tclpanic to properly print msg
 *
 * Revision 1.1  1995/07/17  04:59:06  tamches
 * First version of the new where axis
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <iostream.h>

#include "tkTools.h" // tclpanic
#include "where4treeConstants.h"

where4TreeConstants::where4TreeConstants(Tcl_Interp *interp,
					 Tk_Window theWindow) {
   assert(theWindow != NULL);

   display = Tk_Display(theWindow);

   theTkWindow = theWindow;
   display = Tk_Display(theTkWindow);

   offscreenPixmap = XCreatePixmap(display, Tk_WindowId(theTkWindow),
				   1, // dummy width (for now)
				   1, // dummy height (for now)
				   Tk_Depth(theWindow));

   listboxBorderPix = 3;
   listboxScrollBarWidth = 16;

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
   erasingGC = XCreateGC(display, Tk_WindowId(theTkWindow),
			 GCForeground,
			 &values);

   values.background = None;
   values.graphics_exposures = false;
   listboxCopyAreaGC = Tk_GetGC(theTkWindow, GCBackground | GCGraphicsExposures, &values);
      // graphics exposures is off; we use visibility events as a conservative temporary
      // kludge since I can't get tk to recognize GraphicsExpose events.
   

   // Root Border
   rootNodeBorder = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("pink"));
   if (rootNodeBorder == NULL) {
      cerr << interp->result << endl;
      exit(5);
   }

   // Root Text
   values.foreground = blackColor->pixel;
   values.font = rootTextFontStruct->fid;
   rootItemTextGC = XCreateGC(display, Tk_WindowId(theTkWindow),
			      GCForeground | GCFont,
			      &values);

   // Master listbox Ray
   values.foreground = cornflowerBlueColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   listboxRayGC = XCreateGC(display, Tk_WindowId(theTkWindow),
				  GCForeground | GCBackground | GCLineWidth | GCCapStyle,
				  &values);

   // Child Ray				  
   //values.foreground = greenColor->pixel;
   values.foreground = pinkColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   subchildRayGC = XCreateGC(display, Tk_WindowId(theTkWindow),
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

   // Master listbox Text
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.font = listboxFontStruct->fid;
   listboxTextGC = XCreateGC(display, Tk_WindowId(theTkWindow),
				   GCForeground | GCBackground | GCFont,
				   &values);

   // Master listbox Triangle
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.join_style = JoinMiter;
   values.fill_style = FillSolid;

   listboxTriangleGC = XCreateGC(display, Tk_WindowId(theTkWindow),
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

   Tk_FreeGC(display, listboxCopyAreaGC);
   XFreeGC(display, erasingGC);
   XFreeGC(display, rootItemTextGC);

   XFreeGC(display, listboxRayGC);
   XFreeGC(display, subchildRayGC);

   XFreeGC(display, listboxTextGC);
   XFreeGC(display, listboxTriangleGC);

   Tk_FreeColor(grayColor);
   Tk_FreeColor(pinkColor);
   Tk_FreeColor(blackColor);
   Tk_FreeColor(cornflowerBlueColor);

   Tk_Free3DBorder(rootNodeBorder);
   Tk_Free3DBorder(listboxBorder);

   XFreePixmap(display, offscreenPixmap);
}

void where4TreeConstants::resize() {
   XFreePixmap(display, offscreenPixmap);
   offscreenPixmap = XCreatePixmap(display, Tk_WindowId(theTkWindow),
				   Tk_Width(theTkWindow),
				   Tk_Height(theTkWindow),
				   Tk_Depth(theTkWindow));

   listboxHeightWhereSBappears = Tk_Height(theTkWindow) * 8 / 10;
      // 80%
}

