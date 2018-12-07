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

#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "dyntypes.h"
#include "dyn_regs.h"
#include "InstructionAdapter.h"
#include "CodeObject.h"
#include "CFG.h"
#include "ParseCallback.h"

#include "fingerprint.h"

using namespace std;
using namespace Dyninst;

class InstrCallback : public ParseAPI::ParseCallback {
    public:
        InstrCallback(Address _s, Fingerprint * _f);

        void instruction_cb(ParseAPI::Function * f, 
                ParseAPI::Block * h,
                Address addr, 
                insn_details * insn);

    private:
        Address syscallTrampStore;
        Fingerprint * fingerprint;
};

#endif
