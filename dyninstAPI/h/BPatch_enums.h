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

#ifndef _BPatch_enums_h_
#define _BPatch_enums_h_

/*
 * Used to specify whether a snippet should be installed before other snippets
 * that have previously been inserted at the same point, or after.
 */
typedef enum {
    BPatch_firstSnippet,
    BPatch_lastSnippet
} BPatch_snippetOrder;

/*
 * Used with BPatch_function::findPoint to specify which of the possible
 * instrumentation points within a procedure should be returned.
 */
typedef enum eBPatch_procedureLocation {
    BPatch_locEntry,
    BPatch_locExit,
    BPatch_locSubroutine,
    BPatch_locLongJump,
    BPatch_locAllLocations,
    BPatch_locInstruction,
    BPatch_locUnknownLocation,
    BPatch_locSourceBlockEntry,		// not yet used
    BPatch_locSourceBlockExit,		// not yet used
    BPatch_locSourceLoopEntry,		// not yet used
    BPatch_locSourceLoopExit,		// not yet used
    BPatch_locBasicBlockEntry,
    BPatch_locBasicBlockExit,		// not yet used
    BPatch_locSourceLoop,		// not yet used
    BPatch_locLoopEntry,	
    BPatch_locLoopExit,
    BPatch_locLoopStartIter,
    BPatch_locLoopEndIter,
    BPatch_locVarInitStart,		// not yet used
    BPatch_locVarInitEnd,		// not yet used
    BPatch_locStatement		// not yet used
} BPatch_procedureLocation;

/*
 * Used to specify whether a snippet is to be called before the instructions
 * at the point where it is inserted, or after.
 */
typedef enum {
    BPatch_callBefore,
    BPatch_callAfter,
    BPatch_callUnset
} BPatch_callWhen;

/* VG (09/07/01) Created */

typedef enum BPatch_opCode {
  BPatch_opLoad,
  BPatch_opStore,
  BPatch_opPrefetch
} BPatch_opCode;

// instrumentation locations for BPatch_paramExpr's
typedef enum {
    BPatch_ploc_guess,
    BPatch_ploc_call,
    BPatch_ploc_entry
} BPatch_ploc;

// program analysis styles
typedef enum {
    BPatch_heuristicMode,
    BPatch_normalMode,
    BPatch_exploratoryMode,
    BPatch_defensiveMode
} BPatch_hybridMode;


#endif /* _BPatch_enums_h_ */
