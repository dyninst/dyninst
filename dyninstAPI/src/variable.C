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
 
// $Id: variable.C,v 1.3 2005/12/06 20:01:25 bernat Exp $

// Variable.C

#include "mapped_object.h"

image_variable::image_variable(Address offset,
                               const pdstring &name,
                               pdmodule *mod) :
    offset_(offset),
    pdmod_(mod) {
    addSymTabName(name);
}

Address image_variable::getOffset() const {
    return offset_;
}

bool image_variable::addSymTabName(const pdstring &name, bool isPrimary) {
    pdvector<pdstring> newSymTabName;

    // isPrimary defaults to false
    if (isPrimary)
        newSymTabName.push_back(name);

    bool found = false;
    for (unsigned i = 0; i < symTabNames_.size(); i++) {
        if (symTabNames_[i] == name) {
            found = true;
        }
        else {
            newSymTabName.push_back(symTabNames_[i]);
        }
    }
    if (!isPrimary)
        newSymTabName.push_back(name);

    symTabNames_ = newSymTabName;

    if (!found) {
        // Add to image class...
        pdmod_->exec()->addVariableName(this, name, true);
    }

    // Bool: true if the name is new; AKA !found
    return (!found);
}

bool image_variable::addPrettyName(const pdstring &name, bool isPrimary) {
    pdvector<pdstring> newPrettyName;

    // isPrimary defaults to false
    if (isPrimary)
        newPrettyName.push_back(name);

    bool found = false;
    for (unsigned i = 0; i < prettyNames_.size(); i++) {
        if (prettyNames_[i] == name) {
            found = true;
        }
        else {
            newPrettyName.push_back(prettyNames_[i]);
        }
    }
    if (!isPrimary)
        newPrettyName.push_back(name);

    prettyNames_ = newPrettyName;

    if (!found) {
        // Add to image class...
        pdmod_->exec()->addVariableName(this, name, false);
    }

    // Bool: true if the name is new; AKA !found
    return (!found);
}


const pdvector<pdstring> &image_variable::symTabNameVector() const {
    return symTabNames_;
}

const pdvector<pdstring> &image_variable::prettyNameVector() const {
    return prettyNames_;
}

int_variable::int_variable(image_variable *var, 
                           Address base,
                           mapped_module *mod) :
    addr_(base + var->getOffset()),
    size_(0),
    ivar_(var),
    mod_(mod)
{
}

int_variable::int_variable(int_variable *parVar,
                           mapped_module *childMod) :
    addr_(parVar->addr_),
    size_(parVar->size_),
    ivar_(parVar->ivar_),
    mod_(childMod)
{
    // Mmm forkage
}

const pdvector<pdstring> &int_variable::prettyNameVector() const {
    return ivar_->prettyNameVector();
}

const pdvector<pdstring> &int_variable::symTabNameVector() const {
    return ivar_->symTabNameVector();
}
