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

// $Id: timing-irix.C,v 1.3 1999/06/08 21:06:31 csserra Exp $

#include <invent.h>
#include <stdio.h>
#include <assert.h>


// "cyclesPerSecond" function prototype
typedef int (*cpsFunc_t)(unsigned &);


int cyclesPerSecond_invent(unsigned &cps)
{
  if (setinvent() == -1) {
    return -1;
  }

  unsigned raw = 0;
  for (inventory_t *inv = getinvent(); inv != NULL; inv = getinvent()) {
    /* only need PROCESSOR/CPUBOARD inventory entries */
    if (inv->inv_class != INV_PROCESSOR) continue;
    if (inv->inv_type != INV_CPUBOARD) continue;
    /* check for clock speed mismatch */
    if (raw == 0) raw = inv->inv_controller;
    if (inv->inv_controller != raw) {
      fprintf(stderr, "!!! non-uniform CPU speeds\n");
      endinvent();
      return -1;
    }
  }
  endinvent();

  cps = raw * 1000000; // convert MHz to Hz
  return 0;
}

extern int cyclesPerSecond_default(unsigned &);

cpsFunc_t cpsFuncs[] = 
{
  cyclesPerSecond_invent,
  cyclesPerSecond_default  // "default" should be last
};

unsigned getCyclesPerSecond()
{
  unsigned cps = 0;
  int nfuncs = sizeof(cpsFuncs) / sizeof(cpsFunc_t);
  int ret = -1;
  for (int i = 0; i < nfuncs; i++) {
    ret = (*cpsFuncs[i])(cps);
    if (ret == 0) break;
  }
  assert(ret == 0); // "default" should never fail
  return cps;
}
