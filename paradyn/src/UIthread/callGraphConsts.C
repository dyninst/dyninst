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

// $Id: callGraphConsts.C,v 1.4 2000/08/11 16:32:13 pcroth Exp $


#include "callGraphConsts.h"

callGraphConsts::callGraphConsts(Tcl_Interp *interp, Tk_Window theTkWindow) {
   display = Tk_Display(theTkWindow); // needed in destructor

   textColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("black"));
   assert(textColor);
   // Root Item FontStruct's:
   rootItemFontStruct = Tk_GetFont(interp, theTkWindow,
				   "*-Helvetica-*-r-*-14-*");
   rootItemItalicFontStruct = Tk_GetFont(interp, theTkWindow, 
					 "*-Helvetica-*-o-*-14-*");
   // Root Item Text GCs:
   XGCValues values;
   values.foreground = textColor->pixel;
   values.font = Tk_FontId(rootItemFontStruct);
   rootItemTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(rootItemTextGC);

   values.font = Tk_FontId(rootItemItalicFontStruct);
   values.foreground = textColor->pixel;
   rootItemShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					 &values);
   assert(rootItemShadowTextGC);

   // Listbox FontStruct's:
   listboxItemFontStruct = Tk_GetFont(interp, theTkWindow, 
				      "*-Helvetica-*-r-*-12-*");
   listboxItemItalicFontStruct = 
     Tk_GetFont(interp, theTkWindow, "*-Helvetica-*-o-*-12-*");
   // Listbox Item Text GCs:
   values.foreground = textColor->pixel;
   values.font = Tk_FontId(listboxItemFontStruct);
   listboxItemGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemGC);

   values.font = Tk_FontId(listboxItemItalicFontStruct);
   values.foreground = textColor->pixel;
   listboxItemShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					       &values);
   assert(listboxItemShadowTextGC);

   

   rootItemTk3DBordersByStyle.resize(2);
   //Color for non-recursive nodes
   rootItemTk3DBordersByStyle[0] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("gray"));
   assert(rootItemTk3DBordersByStyle[0]);

   //Color for recursive nodes
   rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("#60c0a0")); 
                                                            //green
   assert(rootItemTk3DBordersByStyle[1]);
   
   listboxItemTk3DBordersByStyle = rootItemTk3DBordersByStyle;
   
   // 3D borders for listbox:
   // It seems reasonable to use the exact same colors for shg listbox items:
   //listboxScrollbarBorder = rootItemBorder;
}

callGraphConsts::~callGraphConsts() {
  Tk_FreeFont(rootItemFontStruct);
  Tk_FreeFont(rootItemItalicFontStruct);
  Tk_FreeFont(listboxItemFontStruct);
  Tk_FreeFont(listboxItemItalicFontStruct);

  Tk_FreeColor(textColor);

   Tk_FreeGC(display, rootItemTextGC);
   Tk_FreeGC(display, listboxItemGC);
   Tk_Free3DBorder(rootItemTk3DBordersByStyle[0]);
   Tk_Free3DBorder(rootItemTk3DBordersByStyle[1]);
   
   // Note that we intentionally don't free up anything in
   // listboxItemTk3DBordersByStyle, since it was always just a shadow copy
   // of the contents of rootItemTk3DBordersByStyle
}




