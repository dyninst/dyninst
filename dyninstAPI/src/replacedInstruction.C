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

// $Id: replacedInstruction.C,v 1.6 2006/11/22 04:03:04 bernat Exp $

#include "multiTramp.h"
#include "process.h"
#include "instPoint.h"
#include "ast.h"

// Necessary methods

// Fork constructor...

replacedInstruction::replacedInstruction(replacedInstruction *parRI,
                                         multiTramp *cMT,
                                         process *child) :
    relocatedCode(parRI, child),
    oldInsn_(NULL), // Fill in below...
    ast_(NULL), // Fill in below...
    multiT_(cMT),
    addr_(parRI->addr_),
    size_(parRI->size_)
{
    oldInsn_ = new relocatedInstruction(parRI->oldInsn_,
                                        cMT,
                                        child);
    ast_ = assignAst(parRI->ast_);
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
                                       UNW_INFO_TYPE **unwindInformation) {
    // Easy-peasy. 
    assert(ast_);

    gen.setPoint(point());

    // Build a registerSpace to be used during code generation. For
    // now, just assume everything is live. TODO: pull out the
    // instPoint-stored register information

    registerSpace *localRegSpace = registerSpace::actualRegSpace(point());
    gen.setRegisterSpace(localRegSpace);

    int cost = 0;
    unsigned start = gen.used();
    addrInMutatee_ = baseInMutatee + start;

    if (!ast_->generateCode(gen, true)) return false;
    //ast_->generateTramp(proc(), point(), gen, &cost, false);
    size_ = gen.used() - start;

    gen.setRegisterSpace(NULL);
    free(localRegSpace);

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

process *replacedInstruction::proc() const { return multi()->proc(); }

