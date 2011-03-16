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

// $Id: replacedInstruction.C,v 1.11 2008/06/19 19:53:38 legendre Exp $

#include "multiTramp.h"
#include "process.h"
#include "instPoint.h"
#include "ast.h"

#include "registerSpace.h"

// Necessary methods

// Fork constructor...

replacedInstruction::replacedInstruction(const replacedInstruction *parRI,
                                         multiTramp *cMT,
                                         process *child) :
    relocatedCode(parRI, child),
    oldInsn_(NULL), // Fill in below...
    ast_(parRI->ast_),
    multiT_(cMT),
    addr_(parRI->addr_),
    size_(parRI->size_)
{
    oldInsn_ = new relocatedInstruction(parRI->oldInsn_,
                                        cMT,
                                        child);
};

generatedCodeObject *replacedInstruction::replaceCode(generatedCodeObject *newParent) {
    multiTramp *newMulti = dynamic_cast<multiTramp *>(newParent);
    assert(newMulti);

    replacedInstruction *newReplacement = new replacedInstruction(this,
                                                                  newMulti);
    return newReplacement;
}

replacedInstruction::~replacedInstruction() {
}


bool replacedInstruction::generateCode(codeGen &gen,
                                       Address baseInMutatee,
                                       UNW_INFO_TYPE **) {
    // Easy-peasy. 
    assert(ast_);

    gen.setPoint(point());

    registerSpace *localRegSpace = registerSpace::actualRegSpace(point(), callPreInsn);
    gen.setRegisterSpace(localRegSpace);

    unsigned start = gen.used();
    addrInMutatee_ = baseInMutatee + start;

    if (!ast_->generateCode(gen, true)) return false;
    size_ = gen.used() - start;

    gen.setRegisterSpace(NULL);
    // Don't delete the register space. 

    generated_ = true;
    hasChanged_ = false;
    return true;
}

unsigned replacedInstruction::maxSizeRequired() {
    // Need to generate AST and determine size.
    return 1024;
}

bool replacedInstruction::safeToFree(codeRange *range) {
    if (dynamic_cast<replacedInstruction *>(range) == this) {
        return false;
    }
    return true;
}

AddressSpace *replacedInstruction::proc() const { return multi()->proc(); }

