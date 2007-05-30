/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// $Id: timing-aix.C,v 1.10 2007/05/30 19:20:30 legendre Exp $
#include "common/h/timing.h"
#include <stdio.h>
#ifdef USES_PMAPI
#include <pmapi.h>
#endif

double calcCyclesPerSecond_sys() {
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
}

double calcCyclesPerSecondOS()
{
  double cps;
  cps = calcCyclesPerSecond_sys();
  if(cps == cpsMethodNotAvailable) {
     return 0.0f;
  }
  return cps;
}

