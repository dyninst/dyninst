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

// shgConsts.C

/* $Id: shgConsts.C,v 1.11 2004/03/20 20:44:47 pcroth Exp $ */

#include "shgConsts.h"

shgConsts::shgConsts(Tcl_Interp *interp, Tk_Window theTkWindow) {
   display = Tk_Display(theTkWindow); // needed in destructor

   inactiveTextColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("black"));
   assert(inactiveTextColor);
   activeTextColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("ivory"));
   assert(activeTextColor);
   deferredTextColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("blue"));
   assert(deferredTextColor);

    // Root Item FontStruct's:
    Tk_Uid rootItemFontName = Tk_GetOption( theTkWindow,
                                            "listRootItemFont",
                                            "Font" );
    assert( rootItemFontName != NULL );
    rootItemFontStruct = Tk_GetFont(interp, 
                                        theTkWindow, 
                                        rootItemFontName );

    Tk_Uid rootItemItalicFontName = Tk_GetOption( theTkWindow,
                                            "listRootItemEmphFont",
                                            "Font" );
    assert( rootItemItalicFontName != NULL );
    rootItemItalicFontStruct = Tk_GetFont(interp, 
                                            theTkWindow, 
                                            rootItemItalicFontName );

   // Root Item Text GCs:
   XGCValues values;
   values.foreground = activeTextColor->pixel;
   values.font = Tk_FontId(rootItemFontStruct);
   rootItemActiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(rootItemActiveTextGC);

   values.foreground = inactiveTextColor->pixel;
   rootItemInactiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(rootItemInactiveTextGC);

    values.foreground = deferredTextColor->pixel;
    rootItemDeferredTextGC = Tk_GetGC(theTkWindow,
                                        GCForeground | GCFont,
                                        &values );
    assert( rootItemDeferredTextGC );

   values.font = Tk_FontId(rootItemItalicFontStruct);
   values.foreground = activeTextColor->pixel;
   rootItemActiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					 &values);
   assert(rootItemActiveShadowTextGC);

   values.foreground = inactiveTextColor->pixel;
   rootItemInactiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					   &values);
   assert(rootItemInactiveShadowTextGC);

   values.foreground = deferredTextColor->pixel;
   rootItemDeferredShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					   &values);
   assert(rootItemDeferredShadowTextGC);

    // Listbox FontStruct's:
    Tk_Uid listboxItemFontName = Tk_GetOption( theTkWindow,
                                                "listItemFont",
                                                "Font" );
    assert( listboxItemFontName != NULL );
    listboxItemFontStruct = Tk_GetFont( interp,
                                        theTkWindow,
                                        listboxItemFontName );

    Tk_Uid listboxItemItalicFontName = Tk_GetOption( theTkWindow,
                                                "listItemEmphFont",
                                                "Font" );
    assert( listboxItemItalicFontName != NULL );
    listboxItemItalicFontStruct = Tk_GetFont(interp, 
                                        theTkWindow,
                                        listboxItemItalicFontName );

   // Listbox Item Text GCs:
   values.foreground = activeTextColor->pixel;
   values.font = Tk_FontId(listboxItemFontStruct);
   listboxItemActiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemActiveTextGC);

   values.foreground = inactiveTextColor->pixel;
   listboxItemInactiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemInactiveTextGC);

   values.foreground = deferredTextColor->pixel;
   listboxItemDeferredTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemDeferredTextGC);

   values.font = Tk_FontId(listboxItemItalicFontStruct);
   values.foreground = activeTextColor->pixel;
   listboxItemActiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					       &values);
   assert(listboxItemActiveShadowTextGC);

   values.foreground = inactiveTextColor->pixel;
   listboxItemInactiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					      &values);
   assert(listboxItemInactiveShadowTextGC);

   values.foreground = deferredTextColor->pixel;
   listboxItemDeferredShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					      &values);
   assert(listboxItemDeferredShadowTextGC);

   // 3D Borders for root item:
   rootItemTk3DBordersByStyle.resize(4);
      // indexed, in effect, by shgRootNode::evaluationState

   // Never before evaluated:
   rootItemTk3DBordersByStyle[0] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("gray"));
   assert(rootItemTk3DBordersByStyle[0]);

   // Unknown:
   rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
						     Tk_GetUid("#60c0a0")); //green
   assert(rootItemTk3DBordersByStyle[1]);

   // True:
   rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("#6495ED"));	// cornflowerblue
   assert(rootItemTk3DBordersByStyle[2]);

   // False:
   rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("pink"));
   assert(rootItemTk3DBordersByStyle[3]);

   // 3D borders for listbox:
   // It seems reasonable to use the exact same colors for shg listbox items:
   listboxItemTk3DBordersByStyle = rootItemTk3DBordersByStyle;
      // indexed, in effect, by shgRootNode::evaluationState & an active flag

   // Refinement Styles (why axis, where axis)
   whyRefinementColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("palegoldenrod"));
   whereRefinementColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("orchid"));

   values.foreground = whyRefinementColor->pixel;
   values.line_width = 2;
   whyRefinementRayGC = Tk_GetGC(theTkWindow, GCForeground | GCLineWidth, &values);
   assert(whyRefinementRayGC);

   values.foreground = whereRefinementColor->pixel;
   values.line_width = 2;
   whereRefinementRayGC = Tk_GetGC(theTkWindow, GCForeground | GCLineWidth, &values);
   assert(whereRefinementRayGC);
}

shgConsts::~shgConsts() {
   Tk_FreeFont(rootItemFontStruct);
   Tk_FreeFont(rootItemItalicFontStruct);
   Tk_FreeFont(listboxItemFontStruct);
   Tk_FreeFont(listboxItemItalicFontStruct);
 
   Tk_FreeColor(activeTextColor);
   Tk_FreeColor(inactiveTextColor);

   Tk_FreeGC(display, rootItemActiveTextGC);
   Tk_FreeGC(display, rootItemInactiveTextGC);
   Tk_FreeGC(display, rootItemActiveShadowTextGC);
   Tk_FreeGC(display, rootItemInactiveShadowTextGC);
   Tk_FreeGC(display, listboxItemActiveTextGC);
   Tk_FreeGC(display, listboxItemInactiveTextGC);
   Tk_FreeGC(display, listboxItemActiveShadowTextGC);
   Tk_FreeGC(display, listboxItemInactiveShadowTextGC);

   Tk_FreeGC(display, whyRefinementRayGC);
   Tk_FreeGC(display, whereRefinementRayGC);

   Tk_FreeColor(whyRefinementColor);
   Tk_FreeColor(whereRefinementColor);

   for (unsigned stylelcv=0; stylelcv < rootItemTk3DBordersByStyle.size(); stylelcv++)
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[stylelcv]);

   // Note that we intentionally don't free up anything in
   // listboxItemTk3DBordersByStyle, since it was always just a shadow copy
   // of the contents of rootItemTk3DBordersByStyle
}
