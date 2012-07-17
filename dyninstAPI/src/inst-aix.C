/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: inst-aix.C,v 1.28 2008/09/03 06:08:44 jaw Exp $
 * inst-aix.C - AIX-specific code for paradynd.
 *
 * XXX - The following functions seem to be less than OS dependent, but I
 *   have included them here to reduce the number of files changed in the
 *   AIX port. - jkh 6/30/95.
 *	process::getProcessStatus()
 *	computePauseTimeMetric()
 */

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include <sys/ldr.h>

//
// All costs are based on Measurements on a SPARC station 10/40.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // these happen async of the rest of the system.
    primitiveCosts["DYNINSTprintCost"] = 1;

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    // nene acutal numbers from 1/26/96 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    //primitiveCosts["DYNINSTdecrementCounter"] = 16;

    logLine("IBM platform\n");
    // Updated calculation of the cost for the following procedures.

    // Values (in cycles) benchmarked on a PowerPC POWER3 375MHz
    // Level 1 - Hardware Level (on by default)
    primitiveCosts["DYNINSTstartWallTimer"] = 321;
    primitiveCosts["DYNINSTstopWallTimer"]  = 325;

    // Values (in cycles) benchmarked on a PowerPC POWER3 375MHz
    // Level 2 - Software Level
    //primitiveCosts["DYNINSTstartWallTimer"]  = 1285;
    //primitiveCosts["DYNINSTstopWallTimer"]   = 1297;
    primitiveCosts["DYNINSTstartProcessTimer"] = 1307;
    primitiveCosts["DYNINSTstopProcessTimer"]  = 1311;

    // These happen async of the rest of the system.
    // 33.74 usecs * 64 Mhz
    primitiveCosts["DYNINSTalarmExpire"] = 2159;
    // 0.38 usecs * 64 Mhz
    primitiveCosts["DYNINSTsampleValues"] = 24;
    // 11.52 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportTimer"] = 737;
    // 1.14 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportCounter"] = 72;
    // 2.07 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportCost"] = 131;
    // 0.66 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportNewTags"] = 42;
}


#if 0
void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool)
{
    assert (0);
}
#endif





