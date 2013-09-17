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
    // Remove from global indices
    std::vector<Symbol *>::iterator iter;

    // everyDefinedSymbol
    for (iter = everyDefinedSymbol.begin(); iter != everyDefinedSymbol.end(); iter++) 
    {
        //  we use indexes in this vector as a unique id for symbols, so mark
        //  as deleted w/out changing vector
        if ((*iter) == sym) (*iter) = &deletedSymbol;
    }
    undefDynSymsByMangledName[sym->getMangledName()].erase(std::remove(undefDynSymsByMangledName[sym->getMangledName()].begin(),
                                                                       undefDynSymsByMangledName[sym->getMangledName()].end(), sym),
                                                           undefDynSymsByMangledName[sym->getMangledName()].end());
    undefDynSymsByPrettyName[sym->getPrettyName()].erase(std::remove(undefDynSymsByPrettyName[sym->getPrettyName()].begin(),
                                                                       undefDynSymsByPrettyName[sym->getPrettyName()].end(), sym),
                                                         undefDynSymsByPrettyName[sym->getPrettyName()].end());
    undefDynSymsByTypedName[sym->getTypedName()].erase(std::remove(undefDynSymsByTypedName[sym->getTypedName()].begin(),
                                                                       undefDynSymsByTypedName[sym->getTypedName()].end(), sym),
                                                           undefDynSymsByTypedName[sym->getTypedName()].end());
    undefDynSyms.erase(std::remove(undefDynSyms.begin(), undefDynSyms.end(), sym), undefDynSyms.end());

    symsByOffset[sym->getOffset()].erase(std::remove(symsByOffset[sym->getOffset()].begin(), symsByOffset[sym->getOffset()].end(),
                                                     sym), symsByOffset[sym->getOffset()].end());
    symsByMangledName[sym->getMangledName()].erase(std::remove(symsByMangledName[sym->getMangledName()].begin(),
                                                               symsByMangledName[sym->getMangledName()].end(), sym),
                                                   symsByMangledName[sym->getMangledName()].end());
    symsByPrettyName[sym->getPrettyName()].erase(std::remove(symsByPrettyName[sym->getPrettyName()].begin(),
                                                             symsByPrettyName[sym->getPrettyName()].end(), sym),
                                                 symsByPrettyName[sym->getPrettyName()].end());
    symsByTypedName[sym->getTypedName()].erase(std::remove(symsByTypedName[sym->getTypedName()].begin(),
                                                           symsByTypedName[sym->getTypedName()].end(), sym),
                                               symsByTypedName[sym->getTypedName()].end());
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

    Offset oldOffset = sym->offset_;
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

        bool ret = newSym->getVersionFileName(rstr);
        if (!ret) 
        {
           fprintf(stderr, "%s[%d]:  failed to getVersionFileName(%s)\n", 
                 FILE__, __LINE__, rstr.c_str());
        }

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
        assert(0 && "could not find text region");
        fprintf(stderr, "%s[%d]:  could not find text region\n", FILE__, __LINE__);
        return NULL;
    }
    
    if (!reg) {
        reg = findEnclosingRegion(offset);
    }

    if (!reg) {
        fprintf(stderr, "%s[%d]:  could not find region for func at %lx\n", 
                FILE__, __LINE__,offset);
        return NULL;
    }
    
    // Let's get the module hammered out. 
    if (mod == NULL) {
        mod = getDefaultModule();
    }

    // Check to see if we contain this module...
    bool found = false;
    for (unsigned i = 0; i < _mods.size(); i++) {
        if (_mods[i] == mod) {
            found = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Mod is %p/%s\n",
                mod, mod->fileName().c_str());
        for (unsigned i = 0; i < _mods.size(); i++) {
            fprintf(stderr, "Matched against %p/%s\n",
                    _mods[i], _mods[i]->fileName().c_str());
        }
        fprintf(stderr, "This %p; mod symtab %p\n",
                this, mod->exec());

        assert(0 && "passed invalid module\n");
        return NULL;
    }
    
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
        assert(0 && "failed to add symbol\n");
        fprintf(stderr, "%s[%d]:  symtab failed to addSymbol\n", FILE__, __LINE__);
        return NULL;
    }
    
    Function *func = statSym->getFunction();
    if (!func) {		
        assert(0 && "failed aggregate creation");
        fprintf(stderr, "%s[%d]:  symtab failed to create function\n", FILE__, __LINE__);
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
#if 0    
    if (!findRegion(reg, ".data") {
        fprintf(stderr, "%s[%d]:  could not find %s region\n", FILE__, __LINE__, regionName.c_str());
        return NULL;
    }
    
    if (!reg) {
        fprintf(stderr, "%s[%d]:  could not find data region\n", FILE__, __LINE__);
        return NULL;
    }
#endif    
    // Let's get the module hammered out. 
    if (mod == NULL) {
        mod = getDefaultModule();
    }
    // Check to see if we contain this module...
    bool found = false;
    for (unsigned i = 0; i < _mods.size(); i++) {
        if (_mods[i] == mod) {
            found = true;
            break;
        }
    }
    if (!found) return NULL;
    
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
        fprintf(stderr, "%s[%d]:  symtab failed to addSymbol\n", FILE__, __LINE__);
        return NULL;
    }
    
    Variable *var = statSym->getVariable();
    if (!var) {		
        fprintf(stderr, "%s[%d]:  symtab failed to create var\n", FILE__, __LINE__);
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

