/*
 * Copyright (c) 1999 Barton P. Miller
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

// $Id: timing-aix.C,v 1.6 2003/02/04 14:59:19 bernat Exp $
#include "common/h/timing.h"
#include <stdio.h>
#ifdef USES_PMAPI
#include <pmapi.h>
#endif

double calcCyclesPerSecond_sys() {
#if defined(USES_PMAPI)
  /* We have a function pm_cycles which returns the time in a double */
  double cycles = pm_cycles();

  // A bit of a hack: I've seen pm_cycles returning abnormally low values
  // (34 cps?) on AIX 5.1 machines. If this value isn't sensible, return
  // as MethodNotAvailable
  if (cycles < 1000) {
      cerr << "WARNING: pm_cycles returned nonsensical value of " << cycles
           << ", using default method to determine CPS." << endl;
      return cpsMethodNotAvailable;
  }
  if (cycles == 0.0)
    return cpsMethodNotAvailable;
  return cycles;
#else
  return cpsMethodNotAvailable;
#endif
}

double calcCyclesPerSecondOS()
{
  double cps;
  cps = calcCyclesPerSecond_sys();
  cerr << "Return from system: " << cps << endl;
  if(cps == cpsMethodNotAvailable) {
      cerr << "Using default" << endl;
    cps = calcCyclesPerSecond_default();
  }
  cerr << "Returning " << cps << endl;
  return cps;
}

