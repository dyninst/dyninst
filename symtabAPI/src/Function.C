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
#include "Function.h"

#include "symtabAPI/src/Object.h"

#include "annotations.h"

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Function *Function::createFunction(Symbol *sym) {
    Function *func = new Function();
    func->addSymbol(sym);
    return func;
}

Function::Function()
    : address_(0), module_(NULL), retType_(NULL), framePtrRegNum_(-1) {}

Offset Function::getAddress() const
{
    return address_;
}

Module * Function::getModule() const
{
    return module_;
}

bool Function::addSymbol(Symbol *sym)
{
    // Functions are defined as "all function symbols existing
    // at a particular address".

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

const vector<std::string> &Function::getAllMangledNames() {
    return mangledNames_;
}

const vector<std::string> &Function::getAllPrettyNames() {
    return prettyNames_;
}

const vector<std::string> &Function::getAllTypedNames() {
    return typedNames_;
}

bool Function::removeSymbol(Symbol *sym) {
    std::vector<Symbol *>::iterator iter;
    for (iter = symbols_.begin(); iter != symbols_.end(); iter++) {
        if (*iter == sym) {
            symbols_.erase(iter);
            return true;
        }
    }
    return false;
}

bool Function::getAllSymbols(std::vector<Symbol *> &syms) const
{
    syms = symbols_;
    return true;
}

Symbol * Function::getFirstSymbol() const
{
    assert( symbols_.size()>0 );
    return symbols_[0];
}

Type * Function::getReturnType() const
{
    return retType_;
}

bool Function::setReturnType(Type *newType)
{
    retType_ = newType;
    return true;
}

int Function::getFramePtrRegnum() const
{
    return framePtrRegNum_;
}

bool Function::setFramePtrRegnum(int regnum)
{
    framePtrRegNum_ = regnum;
    return true;
}

bool Function::findLocalVariable(std::vector<localVar *> &vars, std::string name)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL, *lps = NULL;
   bool res1 = false, res2 = false;
   res1 = getAnnotation(lvs, FunctionLocalVariablesAnno);
   res2 = getAnnotation(lps, FunctionParametersAnno);

   if (!res1 && !res2)
      return false;

   unsigned origSize = vars.size();	

   if (lvs)
   {
      localVar *var = lvs->findLocalVar(name);
      if (var) 
         vars.push_back(var);
   }

   if (lps)
   {
      localVar *var = lps->findLocalVar(name);
      if (var) 
         vars.push_back(var);
   }

   if (vars.size() > origSize)
      return true;

   return false;
}

bool Function::getLocalVariables(std::vector<localVar *> &vars)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL;
   if (!getAnnotation(lvs, FunctionLocalVariablesAnno))
   {
      return false;
   }
   if (!lvs)
   {
      fprintf(stderr, "%s[%d]:  FIXME:  NULL ptr for annotation\n", FILE__, __LINE__);
      return false;
   }

   vars = *(lvs->getAllVars());

   if (vars.size())
      return true;
   return false;
}

bool Function::getParams(std::vector<localVar *> &params)
{
   module_->exec()->parseTypesNow();	

   localVarCollection *lvs = NULL;
   if (!getAnnotation(lvs, FunctionParametersAnno))
   {
      return false;
   }

   if (!lvs)
   {
      fprintf(stderr, "%s[%d]:  FIXME:  NULL ptr for annotation\n", FILE__, __LINE__);
      return false;
   }

   params = *(lvs->getAllVars());

   if (params.size())
      return true;
   return false;
}

bool Function::addLocalVar(localVar *locVar)
{
   localVarCollection *lvs = NULL;

   if (!getAnnotation(lvs, FunctionLocalVariablesAnno))
   {
      lvs = new localVarCollection();

      if (!addAnnotation(lvs, FunctionLocalVariablesAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add local var collecton anno\n", 
               FILE__, __LINE__);
         return false;
      }
   }

   lvs->addLocalVar(locVar);
   return true;
}

bool Function::addParam(localVar *param)
{
   localVarCollection *ps = NULL;

   if (!getAnnotation(ps, FunctionParametersAnno))
   {
      ps = new localVarCollection();

      if (!addAnnotation(ps, FunctionParametersAnno))
      {
         fprintf(stderr, "%s[%d]:  failed to add local var collecton anno\n", 
               FILE__, __LINE__);
         return false;
      }
   }

   ps->addLocalVar(param);

   return true;
}


DLLEXPORT bool Function::addMangledName(string name, bool isPrimary) 
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

DLLEXPORT bool Function::addPrettyName(string name, bool isPrimary) 
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

DLLEXPORT bool Function::addTypedName(string name, bool isPrimary) 
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
