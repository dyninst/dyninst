// shgConsts.h
// Ariel Tamches

/* $Log: shgConsts.h,v $
/* Revision 1.1  1995/10/17 22:07:38  tamches
/* initial version for the new search history graph
/*
 */

#ifndef _SHG_CONSTS_H_
#define _SHG_CONSTS_H_

#ifdef PARADYN
#include "util/h/Vector.h"
#else
#include "Vector.h"
#endif

#include "tkclean.h"

struct shgConsts {
   vector<Tk_3DBorder> rootItemTk3DBordersByStyle;
   vector<Tk_3DBorder> listboxItemTk3DBordersByStyle;

   shgConsts(Tcl_Interp *interp, Tk_Window theTkWindow) {
      rootItemTk3DBordersByStyle.resize(4);

      // uninstrumented
//      rootItemTk3DBordersByStyle[0] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("Gray"));
      rootItemTk3DBordersByStyle[0] = Tk_Get3DBorder(interp, theTkWindow,
						     Tk_GetUid("#e9fbb57aa3c9")); // yuck --ari
      assert(rootItemTk3DBordersByStyle[0]);

      // instrumented, but no decision yet
//      rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("tan"));
      rootItemTk3DBordersByStyle[1] = Tk_Get3DBorder(interp, theTkWindow,
						     Tk_GetUid("#ffffbba5bba5")); // yuck --ari
      assert(rootItemTk3DBordersByStyle[1]);

      // instrumented, decided on true
//      rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("LightBlue"));
      rootItemTk3DBordersByStyle[2] = Tk_Get3DBorder(interp, theTkWindow,
						     Tk_GetUid("#acbff48ff6c8")); // yuck --ari
      assert(rootItemTk3DBordersByStyle[2]);

      // instrumented, decided on false
//      rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
//						     Tk_GetUid("pink"));
      rootItemTk3DBordersByStyle[3] = Tk_Get3DBorder(interp, theTkWindow,
						     Tk_GetUid("#cc85d5c2777d")); // yuck --ari
      assert(rootItemTk3DBordersByStyle[3]);


      // It seems reasonable to use the exact same colors for shg listbox items:
      listboxItemTk3DBordersByStyle = rootItemTk3DBordersByStyle;
   }

  ~shgConsts() {
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[0]);
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[1]);
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[2]);
      Tk_Free3DBorder(rootItemTk3DBordersByStyle[3]);
   }
};

#endif
