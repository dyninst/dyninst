/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: timing-linux.C,v 1.7 2007/05/30 19:20:32 legendre Exp $
#include <stdio.h>
#include "common/src/timing.h"


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
      if(totassigned == 1) {
         fclose(cpuinfo_f);
         return cpumhz*1000000.0;
      }
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
     return 0.0;
  }
  return cps;
}

