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
#include "common/src/pathName.h"

#include "Symtab.h"
#include "Symbol.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"
#include "symtab_impl.hpp"
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
        }
        break;
    }
    case Symbol::ST_TLS:
    case Symbol::ST_OBJECT: {
        std::vector<Variable *> vars;
        if (findVariablesByOffset(vars, sym->getOffset())) {
            for (auto v: vars)  {
                if (v->getSize() == sym->getSize())  {
                    v->removeSymbol(sym);
                    // See above
                }
            }
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
    impl->funcsByOffset.erase(func->getOffset());

    // Now handle the Aggregate stuff
    return deleteAggregate(func);
}

bool Symtab::deleteVariable(Variable *var) {
    // remove variable from everyVariable
    everyVariable.erase(std::remove(everyVariable.begin(), everyVariable.end(), var), everyVariable.end());

    // remove variable from varsByOffset
    {
        decltype(impl->varsByOffset)::accessor a;
        bool found = !impl->varsByOffset.find(a, var->getOffset());
        if (found)  {
            decltype(impl->varsByOffset)::mapped_type &vars = a->second;
            vars.erase(std::remove(vars.begin(), vars.end(), var), vars.end());
            if (vars.empty())  {
                impl->varsByOffset.erase(a);
            }
        }
    }

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
  impl->everyDefinedSymbol.erase(sym);
  impl->undefDynSyms.erase(sym);
  return true;
}

bool Symtab::deleteSymbol(Symbol *sym)
{
    boost::unique_lock<dyn_rwlock> l(symbols_rwlock);
    if (sym->aggregate_) {
        sym->aggregate_->removeSymbol(sym);
    }
    bool result = deleteSymbolFromIndices(sym);
    return result;
}

bool Symtab::changeAggregateOffset(Aggregate *agg, Offset oldOffset, Offset newOffset) {
    Function *func = dynamic_cast<Function *>(agg);
    Variable *var = dynamic_cast<Variable *>(agg);

    if (func) {
        impl->funcsByOffset.erase(oldOffset);
        if (!impl->funcsByOffset.insert({newOffset, func})) {
            // Already someone there... odd, so don't do anything.
        }
    }
    if (var) {
        decltype(impl->varsByOffset)::accessor a;
        bool found = !impl->varsByOffset.find(a, oldOffset);
        decltype(impl->varsByOffset)::mapped_type &vars = a->second;
        if (found)  {
            vars.erase(std::remove(vars.begin(), vars.end(), var), vars.end());
            if (vars.empty())  {
                impl->varsByOffset.erase(a);
            }
        }  else  {
            assert(0);
        }

        found = !impl->varsByOffset.insert(a, newOffset);
        if (found)  {
            found = false;
            for (auto v: vars)  {
                if (v->getSize() == var->getSize())  {
                    found = true;
                }
            }
        }
        if (!found)  {
            vars.push_back(var);
        }  else  {
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
        vector<string> *vers{};
        newSym->setVersionFileName(filename);
        std::string rstr;

        newSym->getVersionFileName(rstr);
        if (referringSymbol->getVersions(vers) && vers != NULL && vers->size() > 0) 
        {
            auto newSymVers = std::vector<std::string>{(*vers)[0]};
            newSym->setVersions(newSymVers);
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
    if (!addSymbol(statSym)) {
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

