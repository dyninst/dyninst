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

#ifndef _StackModExpr_h_
#define _StackModExpr_h_

#include "BPatch_snippet.h"

class BPatch_function;

class BPatch_stackInsertExpr : public BPatch_snippet {
    public: 
        // Creates a stack shift (insertion) of size
        BPatch_stackInsertExpr(int size);
};

class BPatch_stackRemoveExpr : public BPatch_snippet {
    public: 
        // Creates a stack shift (removal) of size
        BPatch_stackRemoveExpr(int size);
};

class BPatch_stackMoveExpr : public BPatch_snippet {
    public:
        // Generates no new code, but triggers relocation and sensitivty analysis
        BPatch_stackMoveExpr();
};

class BPatch_canaryExpr : public BPatch_snippet {
    public:
        // Creates the placement of a per-thread canary value on the stack 
        BPatch_canaryExpr();
};

class BPatch_canaryCheckExpr : public BPatch_snippet {
    public:
        // Checks the integrity of a per-thread canary value on the stack
        //  failureFunc
        //      The function called if the canary check at function exit fails
        //  canaryAfterPrologue
        //      Indicates if the canary is inserted at entry (false)
        //      or after the function prologue (true)
        //  canaryHeight
        //     The height of the canary relative to the stack height at which
        //     the canary is being referenced; in the common case, this is 0.
        //     If the canary is inserted after the prologue, there may not be
        //     an instruction whose stack height is the intended canary location,
        //     in this case, canaryHeight must be set to the difference
        BPatch_canaryCheckExpr(BPatch_function* failureFunc, bool canaryAfterPrologue, long canaryHeight);
};

class BPatch_stackRandomizeExpr : public BPatch_snippet {
    public:
        // Generates no new code, but triggers relocation and sensitivty analysis
        BPatch_stackRandomizeExpr();
};

#endif
