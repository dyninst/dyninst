/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

/* $Id: where4treeConstants.C,v 1.17 2004/03/20 20:44:47 pcroth Exp $ */

#include <assert.h>
#include <stdlib.h>
#include <iostream>

#include "common/h/headers.h"
#include "where4treeConstants.h"
#include "tkTools.h" // tclpanic

where4TreeConstants::where4TreeConstants(Tcl_Interp *interp,
					 Tk_Window theWindow) {
   assert(theWindow != NULL);

   display = Tk_Display(theWindow);

   theTkWindow = theWindow;
   display = Tk_Display(theTkWindow);

   offscreenPixmap = Tk_GetPixmap(display, Tk_WindowId(theTkWindow),
				   1, // dummy width (for now)
				   1, // dummy height (for now)
				   Tk_Depth(theWindow));

   listboxBorderPix = 3;
   listboxScrollBarWidth = 16;

   listboxHeightWhereSBappears = Tk_Height(theTkWindow) * 8 / 10; // 80%

    Tk_Uid rootTextFontName = Tk_GetOption( theTkWindow,
                                                "listRootItemFont",
                                                "Font" );
    assert( rootTextFontName != NULL );
    rootTextFontStruct = Tk_GetFont(interp, theWindow, rootTextFontName );

   grayColor = Tk_GetColor(interp, theWindow, Tk_GetUid("gray"));
   if (grayColor == NULL)
      tclpanic(interp, "could not allocate gray color");

   XColor *darkGrayColor;
   darkGrayColor = Tk_GetColor(interp, theWindow, Tk_GetUid("DarkSlateGray"));
   if (grayColor == NULL)
      tclpanic(interp, "could not allocate gray color");

   pinkColor = Tk_GetColor(interp, theWindow, Tk_GetUid("pink"));
   if (pinkColor == NULL)
      tclpanic(interp, "could not allocate pink color");

   blackColor = Tk_GetColor(interp, theWindow, Tk_GetUid("black"));
   if (blackColor == NULL)
      tclpanic(interp, "could not allocate black color");

   cornflowerBlueColor = Tk_GetColor(interp, theWindow, Tk_GetUid("#6495ED"));
   if (cornflowerBlueColor == NULL)
      tclpanic(interp, "could not allocate cornflower blue color");


   XGCValues values;

   // Erasing:
   values.foreground = grayColor->pixel;
   erasingGC = Tk_GetGC(theTkWindow,
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
      cerr << Tcl_GetStringResult(interp) << endl;
      exit(5);
   }

   // Root Text
   values.foreground = blackColor->pixel;
   values.font = Tk_FontId(rootTextFontStruct);
   rootItemTextGC = Tk_GetGC(theTkWindow,
			      GCForeground | GCFont,
			      &values);

   // Master listbox Ray
   values.foreground = cornflowerBlueColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   listboxRayGC = Tk_GetGC(theTkWindow,
				  GCForeground | GCBackground | GCLineWidth | GCCapStyle,
				  &values);

   // Child Ray				  
   //values.foreground = greenColor->pixel;
   values.foreground = pinkColor->pixel;
   values.background = grayColor->pixel;
   values.line_width = 2;
   values.cap_style = CapButt;
   subchildRayGC = Tk_GetGC(theTkWindow,
			     GCForeground | GCBackground | GCLineWidth | GCCapStyle,
			     &values);

    // Master listbox Font                               
    Tk_Uid listboxFontName = Tk_GetOption( theTkWindow,
                                            "listItemFont",
                                            "Font" );
    assert( listboxFontName != NULL );
    listboxFontStruct = Tk_GetFont(interp, theWindow, listboxFontName );


   // Master listbox Borders
   listboxBorder = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("#6495ED"));
   if (rootNodeBorder == NULL) {
      cerr << Tcl_GetStringResult(interp) << endl;
      exit(5);
   }

   listboxScrollbarBorderNormal = Tk_Get3DBorder(interp, theWindow, Tk_GetUid("gray"));
   if (listboxScrollbarBorderNormal == NULL) {
      cerr << Tcl_GetStringResult(interp) << endl;
      exit(5);
   }

   // Master listbox Text
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.font = Tk_FontId(listboxFontStruct);
   listboxTextGC = Tk_GetGC(theTkWindow,
                            GCForeground | GCBackground | GCFont,
                            &values);

   // Master listbox Text
   values.foreground = grayColor->pixel;
   values.background = pinkColor->pixel;
   values.font = Tk_FontId(listboxFontStruct);
   listboxRetiredTextGC = Tk_GetGC(theTkWindow,
                                   GCForeground | GCBackground | GCFont,
                                   &values);

   // Master listbox Text
   values.foreground = darkGrayColor->pixel;
   values.background = pinkColor->pixel;
   values.font = Tk_FontId(listboxFontStruct);
   rootRetiredTextGC = Tk_GetGC(theTkWindow,
                                GCForeground | GCBackground | GCFont,
                                &values);

   // Master listbox Triangle
   values.foreground = blackColor->pixel;
   values.background = pinkColor->pixel;
   values.join_style = JoinMiter;
   values.fill_style = FillSolid;

   listboxTriangleGC = Tk_GetGC(theTkWindow,
				 GCForeground | GCBackground | GCJoinStyle | GCFillStyle,
				 &values);

   // Other listbox integers
   listboxHorizPadBeforeText = Tk_TextWidth(listboxFontStruct, "8", 1);
   listboxHorizPadBeforeTriangle = 3 * listboxHorizPadBeforeText / 4;
   listboxTriangleWidth = listboxHorizPadBeforeText;

   Tk_FontMetrics listboxFontMetrics;
   Tk_GetFontMetrics(listboxFontStruct, &listboxFontMetrics);
   listboxTriangleHeight = listboxFontMetrics.ascent;
   listboxHorizPadAfterTriangle = listboxHorizPadBeforeTriangle;

   int thePad = listboxFontMetrics.ascent / 4;
   if (listboxFontMetrics.descent + 1 > thePad)
      thePad = listboxFontMetrics.descent + 1;

   listboxVertPadAboveItem = thePad;
   listboxVertPadAfterItemBaseline = thePad;

   // spacing integers:
   horizPixBetweenChildren = 10;
   vertPixParent2ChildTop = 10;
   horizPixlistbox2FirstExpandedChild = 15;
}

where4TreeConstants::~where4TreeConstants() {
   Tk_FreeFont(rootTextFontStruct);
   Tk_FreeFont(listboxFontStruct);

   Tk_FreeGC(display, listboxCopyAreaGC);
   Tk_FreeGC(display, erasingGC);
   Tk_FreeGC(display, rootItemTextGC);

   Tk_FreeGC(display, listboxRayGC);
   Tk_FreeGC(display, subchildRayGC);

   Tk_FreeGC(display, listboxTextGC);
   Tk_FreeGC(display, listboxTriangleGC);

   Tk_FreeColor(grayColor);
   Tk_FreeColor(pinkColor);
   Tk_FreeColor(blackColor);
   Tk_FreeColor(cornflowerBlueColor);

   Tk_Free3DBorder(rootNodeBorder);
   Tk_Free3DBorder(listboxBorder);

   Tk_FreePixmap(display, offscreenPixmap);
}

void where4TreeConstants::resize() {
   Tk_FreePixmap(display, offscreenPixmap);
   offscreenPixmap = Tk_GetPixmap(display, Tk_WindowId(theTkWindow),
				   Tk_Width(theTkWindow),
				   Tk_Height(theTkWindow),
				   Tk_Depth(theTkWindow));

   listboxHeightWhereSBappears = Tk_Height(theTkWindow) * 8 / 10;
      // 80%
}

