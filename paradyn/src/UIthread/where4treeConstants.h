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

// where4treeConstants.h
// Ariel Tamches

/* $Id: where4treeConstants.h,v 1.9 2002/11/25 23:52:37 schendel Exp $ */

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
   Tk_Font rootTextFontStruct;
   Tk_3DBorder rootNodeBorder;
   GC rootItemTextGC, rootRetiredTextGC;

   // Rays:
   GC listboxRayGC;
   GC subchildRayGC;

   // Listbox:
   int listboxBorderPix; // 3
   int listboxScrollBarWidth; // 16
   int listboxHeightWhereSBappears;
      // varies as the window gets resize -- let's say for now that it's
      // always something like 80% of the height of the master canvas window.
   Tk_Font listboxFontStruct;
   Tk_3DBorder listboxBorder;
   Tk_3DBorder listboxScrollbarBorderNormal;
   GC listboxTextGC, listboxTriangleGC, listboxRetiredTextGC;
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
