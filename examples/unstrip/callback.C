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

#include "util.h"
#include "callback.h"
#include "fingerprint.h"

#include "dyntypes.h"
#include "InstructionAdapter.h"
#include "CodeObject.h"
#include "CFG.h"
#include "ParseCallback.h"

#include "Instruction.h"

using namespace std;
using namespace Dyninst;

InstrCallback::InstrCallback(Address _s, Fingerprint * _f) : 
    syscallTrampStore(_s),
    fingerprint(_f) {}

/* 
 * Record system calls found during binary parsing.
 */
void InstrCallback::instruction_cb(ParseAPI::Function * f,
        ParseAPI::Block* b,
        Address addr,
        insn_details* insnDetails)
{
    InstructionAPI::Instruction::Ptr insn = insnDetails->insn->getInstruction();

    /* If we haven't found the syscallTrampStore yet, 
     * check if this is an indirect call through it */
    if (!syscallTrampStore) {
        isCallToSyscallTrampStore(insn, syscallTrampStore);
    }

    if (isSyscall(insn, syscallTrampStore)) {
        /* Store trapLoc */
        trapLoc tloc(addr, insn, NULL);
        fingerprint->addTrapInfo(f, tloc);
    }
}
