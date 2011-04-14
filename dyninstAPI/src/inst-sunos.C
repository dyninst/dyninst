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

// $Id: inst-sunos.C,v 1.60 2008/06/19 22:13:42 jaw Exp $

#ifndef NULL
#define NULL 0
#endif

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"

//
// All costs are based on Measurements on a SPARC station 10/40.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    primitiveCosts["DYNINSTprintCost"] = 1;

    //
    // I can't find DYNINSTincrementCounter or DYNINSTdecrementCounter
    // I think they are not being used anywhere - naim
    //
    // isthmus acutal numbers from 7/3/94 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    primitiveCosts["DYNINSTdecrementCounter"] = 16;

#if defined(i386_unknown_solaris2_5)
    logLine("Solaris/x86 platform\n");
    // Updated calculation of the cost for the following procedures.
    // cost in cycles

    // Values (in cycles) benchmarked on a Pentium II 400MHz
    // Level 2 - Software Level
    primitiveCosts["DYNINSTstartWallTimer"] = 1304;
    primitiveCosts["DYNINSTstopWallTimer"] = 1321;
    primitiveCosts["DYNINSTstartProcessTimer"] = 1324;
    primitiveCosts["DYNINSTstopProcessTimer"] = 1350;

    // These happen async of the rest of the system.
    primitiveCosts["DYNINSTalarmExpire"] = 3724;
    primitiveCosts["DYNINSTsampleValues"] = 13;
    primitiveCosts["DYNINSTreportTimer"] = 1380;
    primitiveCosts["DYNINSTreportCounter"] = 1270;
    primitiveCosts["DYNINSTreportCost"] = 1350;
    primitiveCosts["DYNINSTreportNewTags"] = 837;

#elif defined(sparc_sun_solaris2_4)
    logLine("Solaris platform\n");

    // Values (in cycles) benchmarked on an UltraSparcIIi 440MHz
    // Level 2 - Software Level
    primitiveCosts["DYNINSTstartWallTimer"] = 248;
    primitiveCosts["DYNINSTstopWallTimer"] = 277;
    primitiveCosts["DYNINSTstartProcessTimer"] = 105;
    primitiveCosts["DYNINSTstopProcessTimer"] = 104;

    // These happen async of the rest of the system.
    // 148 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTalarmExpire"] = 5513;
    // 0.81 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTsampleValues"] = 30;
    // 23.08 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTreportTimer"] = 855;
    // 7.85 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTreportCounter"] = 290;
    // 4.22 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTreportCost"] = 156;
    // 1.03 usecs * 37.04 Mhz
    primitiveCosts["DYNINSTreportNewTags"] = 38;
#endif
}

