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
 
// $Id: variable.C,v 1.7 2007/01/19 22:12:44 giri Exp $

// Variable.C

#include "mapped_object.h"

image_variable::image_variable(Address offset,
                               const pdstring &name,
                               pdmodule *mod) :
    pdmod_(mod) {
    sym_ = new Dyn_Symbol(name.c_str(), mod->fileName(), Dyn_Symbol::ST_OBJECT , Dyn_Symbol:: SL_GLOBAL,
                                                                    offset);
    mod->imExec()->getObject()->addSymbol(sym_);
    sym_->setUpPtr(this);								    
//    addSymTabName(name);
}

image_variable::image_variable(Dyn_Symbol *sym,
                               pdmodule *mod) :
    sym_(sym),			       
    pdmod_(mod) {
}								    

Address image_variable::getOffset() const {
    return sym_->getAddr();
}

bool image_variable::addSymTabName(const pdstring &name, bool isPrimary) {
       if(sym_->addMangledName(name.c_str(), isPrimary)){
	    // Add to image class...
            //pdmod_->imExec()->addVariableName(this, name, true);
            return true;
       }
       // Bool: true if the name is new; AKA !found
       return false;
}
					   
bool image_variable::addPrettyName(const pdstring &name, bool isPrimary) {
       if(sym_->addPrettyName(name.c_str(), isPrimary)){
	    // Add to image class...
            //pdmod_->imExec()->addVariableName(this, name, false);
            return true;
       }
       // Bool: true if the name is new; AKA !found
       return false;
}       

const vector<string>& image_variable::symTabNameVector() const {
    return sym_->getAllMangledNames();
}

const vector<string>& image_variable::prettyNameVector() const {
    return sym_->getAllPrettyNames();
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

const vector<string>& int_variable::prettyNameVector() const {
    return ivar_->prettyNameVector();
}

const vector<string>& int_variable::symTabNameVector() const {
    return ivar_->symTabNameVector();
}

const string &int_variable::symTabName() const {
    return ivar_->symTabName();
}
