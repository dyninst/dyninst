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

// $Id: makenan.C,v 1.7 1998/08/16 23:21:32 wylie Exp $

#include "util/h/makenan.h"
#include "util/h/headers.h"

float f_paradyn_nan = 0.0;
bool nan_created = false;
bool matherr_flag = false;

float make_Nan() {
    if(!nan_created){
    matherr_flag = true;
    double temp = -3.0;
    f_paradyn_nan = (float)sqrt(temp);
    matherr_flag = false;
    nan_created = true;
    }
    bool aflag;
    aflag=(isnan(f_paradyn_nan));
    assert(aflag);
    return f_paradyn_nan;
}

#if defined(i386_unknown_nt4_0)
int matherr(struct _exception *x) {
  if ((x->type == _DOMAIN) && !P_strcmp(x->name, "sqrt")) {
      if (matherr_flag)
          return(1);
  }
  return(0);
}
#else

int matherr(struct MATH_EXCEPTION_STRUCT *x) {
  if ((x->type == DOMAIN) && !P_strcmp(x->name, "sqrt")) {
      if (matherr_flag)
	    return(1);
  }
  return(0);
}
#endif

