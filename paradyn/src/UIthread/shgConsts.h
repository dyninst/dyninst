// shgConsts.h
// Ariel Tamches

/* $Log: shgConsts.h,v $
/* Revision 1.4  1996/02/15 23:10:58  tamches
/* added code to support why vs. where axis refinement
/*
 * Revision 1.3  1996/01/23 07:03:28  tamches
 * added shadow node features
 * moved code to .C file
 *
 * Revision 1.2  1996/01/11 23:42:04  tamches
 * there are now 6 node styles instead of 4
 *
 * Revision 1.1  1995/10/17 22:07:38  tamches
 * initial version for the new search history graph
 *
 */

#ifndef _SHG_CONSTS_H_
#define _SHG_CONSTS_H_

#ifdef PARADYN
#include "util/h/Vector.h"
#else
#include "Vector.h"
#endif

#include "tk.h"

struct shgConsts {
   Display *display;

   XFontStruct *rootItemFontStruct, *rootItemItalicFontStruct;
   XFontStruct *listboxItemFontStruct, *listboxItemItalicFontStruct;

   XColor *inactiveTextColor, *activeTextColor;
   GC rootItemActiveTextGC, rootItemInactiveTextGC,
      rootItemActiveShadowTextGC, rootItemInactiveShadowTextGC;
   GC listboxItemActiveTextGC, listboxItemInactiveTextGC,
      listboxItemActiveShadowTextGC, listboxItemInactiveShadowTextGC;

   GC whyRefinementRayGC, whereRefinementRayGC;
   XColor *whyRefinementColor, *whereRefinementColor;

   vector<Tk_3DBorder> rootItemTk3DBordersByStyle;
   vector<Tk_3DBorder> listboxItemTk3DBordersByStyle;

   shgConsts(Tcl_Interp *interp, Tk_Window theTkWindow);
  ~shgConsts();
};

#endif
