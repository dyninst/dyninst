// shgConsts.C

/* $Log: shgConsts.C,v $
/* Revision 1.3  1996/02/02 18:44:57  tamches
/* shg color change: unknown is green more more readability than tan
/*
 * Revision 1.2  1996/01/23 07:04:09  tamches
 * added shadow node features.
 * moved code here from the .h file
 *
 * Revision 1.1  1995/10/17 22:07:39  tamches
 * initial version for the new search history graph
 *
 */

#include "shgConsts.h"

shgConsts::shgConsts(Tcl_Interp *interp, Tk_Window theTkWindow) {
   display = Tk_Display(theTkWindow); // needed in destructor

   inactiveTextColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("black"));
   assert(inactiveTextColor);
   activeTextColor = Tk_GetColor(interp, theTkWindow, Tk_GetUid("ivory"));
   assert(activeTextColor);

   // Root Item FontStruct's:
   rootItemFontStruct = Tk_GetFontStruct(interp, theTkWindow,
					 Tk_GetUid("*-Helvetica-*-r-*-14-*"));
   assert(rootItemFontStruct);

   rootItemItalicFontStruct = Tk_GetFontStruct(interp, theTkWindow,
					       Tk_GetUid("*-Helvetica-*-o-*-14-*"));
   assert(rootItemItalicFontStruct);

   // Root Item Text GCs:
   XGCValues values;
   values.foreground = activeTextColor->pixel;
   values.font = rootItemFontStruct->fid;
   rootItemActiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(rootItemActiveTextGC);

   values.foreground = inactiveTextColor->pixel;
   rootItemInactiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(rootItemInactiveTextGC);

   values.font = rootItemItalicFontStruct->fid;
   values.foreground = activeTextColor->pixel;
   rootItemActiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					 &values);
   assert(rootItemActiveShadowTextGC);

   values.foreground = inactiveTextColor->pixel;
   rootItemInactiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					   &values);
   assert(rootItemInactiveShadowTextGC);

   // Listbox FontStruct's:   
   listboxItemFontStruct = Tk_GetFontStruct(interp, theTkWindow,
					    Tk_GetUid("*-Helvetica-*-r-*-12-*"));
   assert(listboxItemFontStruct);

   listboxItemItalicFontStruct = Tk_GetFontStruct(interp, theTkWindow,
						  Tk_GetUid("*-Helvetica-*-o-*-12-*"));
   assert(listboxItemItalicFontStruct);

   // Listbox Item Text GCs:
   values.foreground = activeTextColor->pixel;
   values.font = listboxItemFontStruct->fid;
   listboxItemActiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemActiveTextGC);

   values.foreground = inactiveTextColor->pixel;
   listboxItemInactiveTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont, &values);
   assert(listboxItemInactiveTextGC);

   values.font = listboxItemItalicFontStruct->fid;
   values.foreground = activeTextColor->pixel;
   listboxItemActiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					       &values);
   assert(listboxItemActiveShadowTextGC);

   values.foreground = inactiveTextColor->pixel;
   listboxItemInactiveShadowTextGC = Tk_GetGC(theTkWindow, GCForeground | GCFont,
					      &values);
   assert(listboxItemActiveShadowTextGC);

   // 3D Borders for root item:
   rootItemTk3DBordersByStyle.resize(4);
      // indexed, in effect, by shgRootNode::evaluationState

   // Never before evaluated:
   rootItemTk3DBordersByStyle[0] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("gray"));
   assert(rootItemTk3DBordersByStyle[0]);

   // Unknown:
   rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("#e9fbb57aa3c9"));
						     Tk_GetUid("#60c0a0")); //green
   assert(rootItemTk3DBordersByStyle[1]);

   // True:
   rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("cornflowerblue"));
   assert(rootItemTk3DBordersByStyle[2]);

   // False:
   rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("pink"));
   assert(rootItemTk3DBordersByStyle[3]);

   // 3D borders for listbox:
   // It seems reasonable to use the exact same colors for shg listbox items:
   listboxItemTk3DBordersByStyle = rootItemTk3DBordersByStyle;
      // indexed, in effect, by shgRootNode::evaluationState & an active flag
}

shgConsts::~shgConsts() {
   Tk_FreeFontStruct(rootItemFontStruct);
   Tk_FreeFontStruct(rootItemItalicFontStruct);
   Tk_FreeFontStruct(listboxItemFontStruct);
   Tk_FreeFontStruct(listboxItemItalicFontStruct);
 
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

   for (unsigned stylelcv=0; stylelcv < rootItemTk3DBordersByStyle.size(); stylelcv++)
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[stylelcv]);

   // Note that we intentionally don't free up anything in
   // listboxItemTk3DBordersByStyle, since it was always just a shadow copy
   // of the contents of rootItemTk3DBordersByStyle
}
