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

// $Id: timing-linux.C,v 1.3 2000/10/17 17:42:07 schendel Exp $
#include <stdio.h>
#include "common/h/timing.h"


// TODO: replace body with (better) platform-specific code

double calcCyclesPerSecond_sys() {
  FILE *cpuinfo_f = fopen( "/proc/cpuinfo", "r" );
  if(cpuinfo_f == NULL)  return cpsMethodNotAvailable;

  while(!feof(cpuinfo_f)) {
    char strbuf[150];
    char *res = fgets(strbuf, 148, cpuinfo_f);
    double cpumhz = 0.0;
    if(res != NULL) {
      int totassigned = sscanf(res, "cpu MHz : %lf",&cpumhz);
      if(totassigned == 1) return cpumhz*1000000.0;
    }
  }
  fclose(cpuinfo_f);
  return cpsMethodNotAvailable;
}

double calcCyclesPerSecondOS()
{
  double cps;
  cps = calcCyclesPerSecond_sys();
  if(cps == cpsMethodNotAvailable) {
    cps = calcCyclesPerSecond_default();
  }
  return cps;
}

