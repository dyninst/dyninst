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

#include "Annotatable.h"
#include "common/h/serialize.h"

#include "Symtab.h"
#include "symutil.h"
#include "Module.h"
#include "Collections.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"


#include "Aggregate.h"
#include "Symbol.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Aggregate::Aggregate() : module_(NULL)
{}

Offset Aggregate::getAddress() const {
    return symbols_[0]->getAddr();
}

Module *Aggregate::getModule() const {
    return module_;
}

const vector<std::string> &Aggregate::getAllMangledNames() {
    return mangledNames_;
}
const vector<std::string> &Aggregate::getAllPrettyNames() {
    return prettyNames_;
}
const vector<std::string> &Aggregate::getAllTypedNames() {
    return typedNames_;
}

bool Aggregate::addSymbol(Symbol *sym) {

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

bool Aggregate::removeSymbol(Symbol *sym) {
    std::vector<Symbol *>::iterator iter;
    for (iter = symbols_.begin(); iter != symbols_.end(); iter++) {
        if (*iter == sym) {
            symbols_.erase(iter);
            return true;
        }
    }
    // TODO: remove from names. Do we ever call this?

    return false;
}

bool Aggregate::getAllSymbols(std::vector<Symbol *> &syms) const 
{
    syms = symbols_;
    return true;
}

Symbol * Aggregate::getFirstSymbol() const
{
    assert( symbols_.size()>0 );
    return symbols_[0];
}

bool Aggregate::addMangledNameInt(string name, bool isPrimary) {
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
    return true;
}

bool Aggregate::addPrettyNameInt(string name, bool isPrimary) {
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
    return true;
}

bool Aggregate::addTypedNameInt(string name, bool isPrimary) {
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
    return true;
}

