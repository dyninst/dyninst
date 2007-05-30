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


// $Id: timing-irix.C,v 1.7 2007/05/30 19:20:31 legendre Exp $

#include <invent.h>
#include <stdio.h>
#include <assert.h>
#include "common/h/timing.h"


double calcCyclesPerSecond_invent()
{
  if (setinvent() == -1) {
    return cpsMethodNotAvailable;
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
      return cpsMethodNotAvailable;
    }
  }
  endinvent();

  double cps = static_cast<double>(raw) * 1000000.0; // convert MHz to Hz
  return cps;
}

double calcCyclesPerSecondOS()
{
  double cps;
  cps = calcCyclesPerSecond_invent();
  if(cps == cpsMethodNotAvailable) {
     return 0;
  }
  return cps;
}

