/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: inst-linux.C,v 1.15 2006/07/07 00:01:03 jaw Exp $

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
#include "dyninstAPI/src/stats.h"

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

