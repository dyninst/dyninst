// shgConsts.C

/* $Log: shgConsts.C,v $
/* Revision 1.2  1996/01/23 07:04:09  tamches
/* added shadow node features.
/* moved code here from the .h file
/*
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
//						     Tk_GetUid("tan"));
						     Tk_GetUid("#e9fbb57aa3c9"));
         // yuck --ari (try tan)
   assert(rootItemTk3DBordersByStyle[1]);

//      // instrumented, but no decision yet
//      rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("#ffffbba5bba5")); // yuck --ari
//      assert(rootItemTk3DBordersByStyle[1]);

   // True:
   rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("cornflowerblue"));
//      rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("#acbff48ff6c8")); // yuck --ari
   assert(rootItemTk3DBordersByStyle[2]);

   // False
   rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
						  Tk_GetUid("pink"));
//      rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("#cc85d5c2777d")); // yuck --ari
   assert(rootItemTk3DBordersByStyle[3]);

//      // instrumented, false
//      rootItemTk3DBordersByStyle[4] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("plum"));
//
//      // uninstrumented, true
//      rootItemTk3DBordersByStyle[5] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("green"));

   // 3D borders for listbox:
   // It seems reasonable to use the exact same colors for shg listbox items:
   listboxItemTk3DBordersByStyle = rootItemTk3DBordersByStyle;
      // indexed, in effect, by shgRootNode::evaluationState
}

shgConsts::~shgConsts() {
   Tk_FreeFontStruct(rootItemFontStruct);
   Tk_FreeFontStruct(rootItemItalicFontStruct);
   Tk_FreeFontStruct(listboxItemFontStruct);
   Tk_FreeFontStruct(listboxItemItalicFontStruct);
 
   Tk_FreeGC(display, rootItemActiveTextGC);
   Tk_FreeGC(display, rootItemInactiveTextGC);
   Tk_FreeGC(display, listboxItemActiveTextGC);
   Tk_FreeGC(display, listboxItemInactiveTextGC);

   Tk_FreeColor(activeTextColor);
   Tk_FreeColor(inactiveTextColor);

   Tk_Free3DBorder(rootItemTk3DBordersByStyle[0]);
   Tk_Free3DBorder(rootItemTk3DBordersByStyle[1]);
   Tk_Free3DBorder(rootItemTk3DBordersByStyle[2]);
   Tk_Free3DBorder(rootItemTk3DBordersByStyle[3]);
}
