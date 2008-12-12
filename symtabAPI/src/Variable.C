/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;



Variable::Variable()
    : address_(0), module_(NULL) {}

Variable *Variable::createVariable(Symbol *sym) {
    Variable *var = new Variable();
    var->addSymbol(sym);
    return var;
}

Offset Variable::getAddress() const {
    return address_;
}

Module *Variable::getModule() const {
    return module_;
}

bool Variable::addSymbol(Symbol *sym) {
    if (address_ == 0) 
        address_ = sym->getAddr();
    else
        assert(address_ == sym->getAddr());

    // We keep a "primary" module, which is defined as "anything not DEFAULT_MODULE".
    if (module_ == NULL) {
        module_ = sym->getModule();
    }
    else if (module_->fileName() == "DEFAULT_MODULE") {
        module_ = sym->getModule();
    }
    // else keep current module.

    symbols_.push_back(sym);

    // We need to add the symbol names (if they aren't there already)
    // We can have multiple identical names - for example, there are
    // often two symbols for main (static and dynamic symbol table)
    
    bool found = false;
    for (unsigned j = 0; j < mangledNames_.size(); j++) {
        if (sym->getMangledName() == mangledNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) mangledNames_.push_back(sym->getMangledName());

    found = false;

    for (unsigned j = 0; j < prettyNames_.size(); j++) {
        if (sym->getPrettyName() == prettyNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) prettyNames_.push_back(sym->getPrettyName());

    found = false;
    for (unsigned j = 0; j < typedNames_.size(); j++) {
        if (sym->getTypedName() == typedNames_[j]) {
            found = true;
            break;
        }
    }
    if (!found) typedNames_.push_back(sym->getTypedName());;

    return true;
}

bool Variable::removeSymbol(Symbol *sym) {
    std::vector<Symbol *>::iterator iter;
    for (iter = symbols_.begin(); iter != symbols_.end(); iter++) {
        if (*iter == sym) {
            symbols_.erase(iter);
            return true;
        }
    }
    return false;
}

bool Variable::getAllSymbols(std::vector<Symbol *> &syms) const {
    syms = symbols_;
    return true;
}

Symbol * Variable::getFirstSymbol() const
{
    assert( symbols_.size()>0 );
    return symbols_[0];
}

DLLEXPORT bool Variable::addMangledName(string name, bool isPrimary) 
{
    // Check to see if we're duplicating
    for (unsigned i = 0; i < mangledNames_.size(); i++) {
        if (mangledNames_[i] == name)
            return false;
    }

    if (isPrimary) {
        std::vector<std::string>::iterator iter = mangledNames_.begin();
        mangledNames_.insert(iter, name);
    }
    else
        mangledNames_.push_back(name);

    /*
      // Need to create symbol for this new name
      if (getModule()->exec()) {
      getModule()->exec()->updateIndices(this, name, Symtab::mangledName);
      }
    */
    
    return true;
}																	

DLLEXPORT bool Variable::addPrettyName(string name, bool isPrimary) 
{
    // Check to see if we're duplicating
    for (unsigned i = 0; i < prettyNames_.size(); i++) {
        if (prettyNames_[i] == name)
            return false;
    }

    if (isPrimary) {
        std::vector<std::string>::iterator iter = prettyNames_.begin();
        prettyNames_.insert(iter, name);
    }
    else
        prettyNames_.push_back(name);

    /*
      // Need to create symbol for this new name
    if (getModule()->exec()) {
        getModule()->exec()->updateIndices(this, name, Symtab::prettyName);
    }
    */    
    return true;
}																	

DLLEXPORT bool Variable::addTypedName(string name, bool isPrimary) 
{
    // Check to see if we're duplicating
    for (unsigned i = 0; i < typedNames_.size(); i++) {
        if (typedNames_[i] == name)
            return false;
    }

    if (isPrimary) {
        std::vector<std::string>::iterator iter = typedNames_.begin();
        typedNames_.insert(iter, name);
    }
    else
        typedNames_.push_back(name);
    /*
      // Need to create symbol for this new name
    if (getModule()->exec()) {
        getModule()->exec()->updateIndices(this, name, Symtab::typedName);
    }
    */

    return true;
}																	
