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

// $Id: inst-linux.C,v 1.16 2008/06/19 22:13:42 jaw Exp $

#ifndef NULL
#define NULL 0
#endif

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/frame.h"

//
// All costs are based on Measurements on a SPARC station 10/40.
// They haven't actually been updated for Linux yet.
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

    // Values (in cycles) benchmarked on a Pentium III 700MHz
    // Level 2 - Software Level
    //primitiveCosts["DYNINSTstartWallTimer"] = 719;
    //primitiveCosts["DYNINSTstopWallTimer"] = 737;
    primitiveCosts["DYNINSTstartProcessTimer"] = 587;
    primitiveCosts["DYNINSTstopProcessTimer"] = 607;

    /* Level 1 - Hardware Level
    // Implementation still needs to be added to handle start/stop
    // timer costs for multiple levels
    */
    primitiveCosts["DYNINSTstartWallTimer"] = 145;
    primitiveCosts["DYNINSTstopWallTimer"] = 163;
   
    //primitiveCosts["DYNINSTstartProcessTimer"] = 195;
    //primitiveCosts["DYNINSTstopProcessTimer"] = 207;

    // These happen async of the rest of the system.
    // 133.86 usecs * 67Mhz
    primitiveCosts["DYNINSTalarmExpire"] = 8968;
    primitiveCosts["DYNINSTsampleValues"] = 29;
    // 6.41 usecs * 67Mhz
    primitiveCosts["DYNINSTreportTimer"] = 429;
    // 89.85 usecs * 67Mhz
    primitiveCosts["DYNINSTreportCounter"] = 6019;
    primitiveCosts["DYNINSTreportCost"] = 167;
    primitiveCosts["DYNINSTreportNewTags"] = 40; 
}

