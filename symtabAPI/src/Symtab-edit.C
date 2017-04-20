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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <algorithm>

#include "common/src/Timer.h"
#include "common/src/debugOstream.h"
#include "common/src/serialize.h"
#include "common/src/pathName.h"

#include "Serialization.h"
#include "Symtab.h"
#include "Symbol.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"

#include "boost/tuple/tuple.hpp"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static Symbol deletedSymbol(std::string("DeletedSymbol"), Symbol::ST_DELETED, Symbol::SL_UNKNOWN, Symbol::SV_UNKNOWN, 0);
/*
 * We're changing the type of a symbol. Therefore we need to rip it out of the indices
 * for whatever it used to be (also, aggregations) and put it in the new ones. 
 * Oy. 
 */

bool Symtab::changeType(Symbol *sym, Symbol::SymbolType oldType)
{
    switch (oldType) {
    case Symbol::ST_FUNCTION: {
        Function *func = NULL;
        if (findFuncByEntryOffset(func, sym->getOffset())) {
            // Remove this symbol from the function
            func->removeSymbol(sym);
            // What if we removed the last symbol from the function?
            // Argh. Ah, well. Users may do that - leave it there for now.
        break;
        }
    }
    case Symbol::ST_TLS:
    case Symbol::ST_OBJECT: {
        Variable *var = NULL;
        if (findVariableByOffset(var, sym->getOffset())) {
            var->removeSymbol(sym);
            // See above
        }
        break;
    }
    case Symbol::ST_MODULE: {
        // TODO Module should be an Aggregation
        break;
    }
    default:
        break;
    }

    addSymbolToIndices(sym, false);
    addSymbolToAggregates(sym);

    return true;
}

bool Symtab::deleteFunction(Function *func) {
    // First, remove the function
    everyFunction.erase(std::remove(everyFunction.begin(), everyFunction.end(), func), everyFunction.end());
/*    std::vector<Function *>::iterator iter;
    for (iter = everyFunction.begin(); iter != everyFunction.end(); iter++) {
        if ((*iter) == func) {
            everyFunction.erase(iter);
        }
    }
*/
    funcsByOffset.erase(func->getOffset());

    // Now handle the Aggregate stuff
    return deleteAggregate(func);
}

bool Symtab::deleteVariable(Variable *var) {
    // First, remove the function
    everyVariable.erase(std::remove(everyVariable.begin(), everyVariable.end(), var), everyVariable.end());

    varsByOffset.erase(var->getOffset());
    return deleteAggregate(var);
}

bool Symtab::deleteAggregate(Aggregate *agg) {
    std::vector<Symbol *> syms;
    agg->getSymbols(syms);

    bool ret = true;
    for (unsigned i = 0; i < syms.size(); i++) {
        if (!deleteSymbolFromIndices(syms[i]))
            ret = false;
    }
    return ret;
}

bool Symtab::deleteSymbolFromIndices(Symbol *sym) {
  everyDefinedSymbol.erase(sym);
  undefDynSyms.erase(sym);
  return true;
}

bool Symtab::deleteSymbol(Symbol *sym)
{
    if (sym->aggregate_) {
        sym->aggregate_->removeSymbol(sym);
    }

    return deleteSymbolFromIndices(sym);
}

bool Symtab::changeSymbolOffset(Symbol *sym, Offset newOffset) {
    // If we aren't part of an aggregate, change the symbol offset
    // and update symsByOffset.
    // If we are part of an aggregate and the only symbol element,
    // do that and update funcsByOffset or varsByOffset.
    // If we are and not the only symbol, do 1), remove from 
    // the aggregate, and make a new aggregate.
  typedef indexed_symbols::index<offset>::type syms_by_off;
  syms_by_off& defindex = everyDefinedSymbol.get<offset>();
  syms_by_off::iterator found = defindex.find(sym->offset_);
  while(found != defindex.end() && 
	(*found)->getOffset() == sym->offset_)
  {
    if(*found == sym) 
    {
      sym->offset_ = newOffset;
      defindex.replace(found, sym);
      break;
    }
  }
  
  
  /*    Offset oldOffset = sym->offset_;
    std::vector<Symbol *>::iterator iter;
    for (iter = symsByOffset[oldOffset].begin();
         iter != symsByOffset[oldOffset].end();
         iter++) {
        if ((*iter) == sym) {
            symsByOffset[oldOffset].erase(iter);
            break;
        }
    }
    sym->offset_ = newOffset;
    symsByOffset[newOffset].push_back(sym);
  */

    if (sym->aggregate_ == NULL) return true;
    else 
        return sym->aggregate_->changeSymbolOffset(sym);

}

bool Symtab::changeAggregateOffset(Aggregate *agg, Offset oldOffset, Offset newOffset) {
    Function *func = dynamic_cast<Function *>(agg);
    Variable *var = dynamic_cast<Variable *>(agg);

    if (func) {
        funcsByOffset.erase(oldOffset);
        if (funcsByOffset.find(newOffset) == funcsByOffset.end())
            funcsByOffset[newOffset] = func;
        else {
            // Already someone there... odd, so don't do anything.
        }
    }
    if (var) {
        varsByOffset.erase(oldOffset);
        if (varsByOffset.find(newOffset) == varsByOffset.end())
            varsByOffset[newOffset] = var;
        else {
            // Already someone there... odd, so don't do anything.
        }
    }
    return true;
}

bool Symtab::addSymbol(Symbol *newSym, Symbol *referringSymbol) 
{
    if (!newSym || !referringSymbol ) return false;

    if( !referringSymbol->getSymtab()->isStaticBinary() ) {
        if (!newSym->isInDynSymtab()) return false;

        newSym->setReferringSymbol(referringSymbol);

        string filename = referringSymbol->getModule()->exec()->name();
        vector<string> *vers, *newSymVers = new vector<string>;
        newSym->setVersionFileName(filename);
        std::string rstr;

        newSym->getVersionFileName(rstr);
        if (referringSymbol->getVersions(vers) && vers != NULL && vers->size() > 0) 
        {
            newSymVers->push_back((*vers)[0]);
            newSym->setVersions(*newSymVers);
        }
    }else{
        newSym->setReferringSymbol(referringSymbol);
    }

    return addSymbol(newSym);
}

bool Symtab::addSymbol(Symbol *newSym) 
{
   if (!newSym) {
    	return false;
   }

   // Expected default behavior: if there is no
   // module use the default.
   if (newSym->getModule() == NULL) {
      newSym->setModule(getDefaultModule());
   }
   
   // If there aren't any pretty names, create them
   if (newSym->getPrettyName() == "") {
      demangleSymbol(newSym);
   }
   
   // Add to appropriate indices
   addSymbolToIndices(newSym, false);
   
   // And to aggregates
   addSymbolToAggregates(newSym);

   return true;
}


Function *Symtab::createFunction(std::string name, 
                                 Offset offset, 
                                 size_t sz,
                                 Module *mod)
{
    Region *reg = NULL;
    
    if (!findRegion(reg, ".text") && !isDefensiveBinary()) {
        return NULL;
    }
    
    if (!reg) {
        reg = findEnclosingRegion(offset);
    }

    if (!reg) {
        return NULL;
    }
    
    // Let's get the module hammered out. 
    if (mod == NULL) {
        mod = getDefaultModule();
    }

    // Check to see if we contain this module...
    if(indexed_modules.get<1>().find(mod) == indexed_modules.get<1>().end()) return NULL;
//
//    bool found = false;
//    for (unsigned i = 0; i < indexed_modules.size(); i++) {
//        if (indexed_modules[i] == mod) {
//            found = true;
//            break;
//        }
//    }
//    if (!found) {
//        return NULL;
//    }
    
    Symbol *statSym = new Symbol(name, 
                                 Symbol::ST_FUNCTION, 
                                 Symbol::SL_GLOBAL,
                                 Symbol::SV_DEFAULT, 
                                 offset, 
                                 mod,
                                 reg, 
                                 sz,
                                 false,
                                 false);
    Symbol *dynSym = new Symbol(name,
                                Symbol::ST_FUNCTION,
                                Symbol::SL_GLOBAL,
                                Symbol::SV_DEFAULT,
                                offset,
                                mod,
                                reg,
                                sz,
                                true,
                                false);

    if (!addSymbol(statSym) || !addSymbol(dynSym)) {
        return NULL;
    }
    
    Function *func = statSym->getFunction();
    if (!func) {		
        return NULL;
    }
    
    return func;
}



Variable *Symtab::createVariable(std::string name, 
                                 Offset offset, 
                                 size_t sz,
                                 Module *mod)
{
    Region *reg = NULL;
    // Let's get the module hammered out. 
    if (mod == NULL) {
        mod = getDefaultModule();
    }
    // Check to see if we contain this module...
    if(indexed_modules.get<1>().find(mod) == indexed_modules.get<1>().end()) return NULL;
//
//    bool found = false;
//    for (unsigned i = 0; i < indexed_modules.size(); i++) {
//        if (indexed_modules[i] == mod) {
//            found = true;
//            break;
//        }
//    }
//    if (!found) {
//        return NULL;
//    }
    
    Symbol *statSym = new Symbol(name, 
                                 Symbol::ST_OBJECT, 
                                 Symbol::SL_GLOBAL,
                                 Symbol::SV_DEFAULT, 
                                 offset, 
                                 mod,
                                 reg, 
                                 sz,
                                 false,
                                 false);
    Symbol *dynSym = new Symbol(name,
                                Symbol::ST_OBJECT,
                                Symbol::SL_GLOBAL,
                                Symbol::SV_DEFAULT,
                                offset,
                                mod,
                                reg,
                                sz,
                                true,
                                false);
    
    statSym->setModule(mod);
    dynSym->setModule(mod);

    if (!addSymbol(statSym) || !addSymbol(dynSym)) {
        return NULL;
    }
    
    Variable *var = statSym->getVariable();
    if (!var) {		
        return NULL;
    }
    
    return var;
}

SYMTAB_EXPORT bool Symtab::updateRelocations(Address start,
                                             Address end,
                                             Symbol *oldsym,
                                             Symbol *newsym) {
   for (unsigned i = 0; i < codeRegions_.size(); ++i) {
      codeRegions_[i]->updateRelocations(start, end, oldsym, newsym);
   }
   return true;
}

