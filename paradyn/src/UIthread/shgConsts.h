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

// shgConsts.h
// Ariel Tamches

/* $Id: shgConsts.h,v 1.11 2003/05/23 07:27:57 pcroth Exp $ */

#ifndef _SHG_CONSTS_H_
#define _SHG_CONSTS_H_

#ifdef PARADYN
#include "common/h/Vector.h"
#else
#include "Vector.h"
#endif

#include "tk.h"

struct shgConsts {
   Display *display;

   Tk_Font rootItemFontStruct, rootItemItalicFontStruct;
   Tk_Font listboxItemFontStruct, listboxItemItalicFontStruct;

   XColor* inactiveTextColor;
   XColor* deferredTextColor;
   XColor* activeTextColor;
   GC rootItemActiveTextGC, rootItemInactiveTextGC,
      rootItemActiveShadowTextGC, rootItemInactiveShadowTextGC;
   GC rootItemDeferredTextGC;
   GC rootItemDeferredShadowTextGC;
   GC listboxItemActiveTextGC, listboxItemInactiveTextGC,
      listboxItemActiveShadowTextGC, listboxItemInactiveShadowTextGC;
   GC listboxItemDeferredTextGC;
   GC listboxItemDeferredShadowTextGC;

   GC whyRefinementRayGC, whereRefinementRayGC;
   XColor *whyRefinementColor, *whereRefinementColor;

   pdvector<Tk_3DBorder> rootItemTk3DBordersByStyle;
   pdvector<Tk_3DBorder> listboxItemTk3DBordersByStyle;

   shgConsts(Tcl_Interp *interp, Tk_Window theTkWindow);
  ~shgConsts();
};

#endif
