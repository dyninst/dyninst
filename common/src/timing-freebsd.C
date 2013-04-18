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

#include <sys/types.h>
#include <sys/sysctl.h>
#include "common/src/timing.h"

static const char *PROC_FREQ_MIB = "dev.cpu.0.freq";

/*
 * On multi(core|processor) machines, this function assumes that all processors
 * have the same frequency and uses the frequency of the first processor to
 * determine the number of cycles per second
 */
double calcCyclesPerSecond_sys() {
    uint32_t data;
    size_t dataSize = sizeof(uint32_t);

    if( sysctlbyname(PROC_FREQ_MIB, &data, &dataSize, NULL, 0) == -1 ) {
        return cpsMethodNotAvailable;
    }

    return data*1e6;
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
