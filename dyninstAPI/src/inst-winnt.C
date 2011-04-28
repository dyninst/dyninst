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

// $Id: inst-winnt.C,v 1.31 2008/06/19 22:13:42 jaw Exp $

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/instPoint.h"

#include "dyninstAPI/src/function.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
using namespace Dyninst::InstructionAPI;

#ifndef mips_unknown_ce2_11 //ccw 27 july 2000 : 29 mar 2001
//defined in inst-mips.C

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

    // Updated calculation of the cost for the following procedures.
    // cost in cycles
    // Values (in cycles) benchmarked on a Pentium II 450MHz
    // Level 1 - Hardware Level
    primitiveCosts["DYNINSTstartWallTimer"] = 151;
    primitiveCosts["DYNINSTstopWallTimer"] = 165;

    // Implementation still needs to be added to handle start/stop
    // timer costs for multiple levels
    /* Level 2 - Software Level
    primitiveCosts["DYNINSTstartWallTimer"] = 797;
    primitiveCosts["DYNINSTstopWallTimer"] = 807;
    */
    primitiveCosts["DYNINSTstartProcessTimer"] = 990;
    primitiveCosts["DYNINSTstopProcessTimer"] = 1017;

    // These happen async of the rest of the system.
    primitiveCosts["DYNINSTalarmExpire"] = 3724;
    primitiveCosts["DYNINSTsampleValues"] = 13;
    primitiveCosts["DYNINSTreportTimer"] = 1380;
    primitiveCosts["DYNINSTreportCounter"] = 1270;
    primitiveCosts["DYNINSTreportCost"] = 1350;
    primitiveCosts["DYNINSTreportNewTags"] = 837;
}
#endif

 

// hasBeenBound: returns false
// dynamic linking not implemented on this platform
bool process::hasBeenBound(const SymtabAPI::relocationEntry &,func_instance *&, Address ) {
    return false;
}

