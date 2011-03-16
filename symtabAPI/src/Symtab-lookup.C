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

/* Lookup functions defined in class Symtab. Separated to reduce file size and classify. */



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "common/h/Timer.h"
#include "common/h/debugOstream.h"
#include "common/h/serialize.h"
#include "common/h/pathName.h"

#include "Serialization.h"
#include "Symtab.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"
#include "annotations.h"

#include "symtabAPI/src/Object.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

extern SymtabError serr;

bool regexEquiv( const std::string &str,const std::string &them, bool checkCase );
bool pattern_match( const char *p, const char *s, bool checkCase );

static bool sort_by_sym_ptr(const Symbol *a, const Symbol *b) {
    return a < b;
}

std::vector<Symbol *> *Symtab::findSymbolByOffset(Offset o)
{
	//Symbol *s = NULL;
	dyn_hash_map<Offset, std::vector<Symbol *> >::iterator iter;
	iter = symsByOffset.find(o);
	if (iter == symsByOffset.end()) return NULL;
	return &(iter->second);
}

bool Symtab::findSymbol(std::vector<Symbol *> &ret, const std::string name,
                        Symbol::SymbolType sType, NameType nameType,
                        bool isRegex, bool checkCase)
{
    unsigned old_size = ret.size();

    std::vector<Symbol *> symsMangled;
    std::vector<Symbol *> symsPretty;
    std::vector<Symbol *> symsTyped;

    if (!isRegex) {
        // Easy case
        if (nameType & mangledName) {
            symsMangled = symsByMangledName[name];
        }
        if (nameType & prettyName) {
            symsPretty = symsByPrettyName[name];
        }
        if (nameType & typedName) {
            symsTyped = symsByTypedName[name];
        }
    }
    else {
        // Build the regex list of symbols
        // We need to iterate over every single symbol. Ugh.
        for (unsigned i = 0; i < everyDefinedSymbol.size(); i++) {
            if (nameType & mangledName) {
                if (regexEquiv(name, everyDefinedSymbol[i]->getName(), checkCase))
                    symsMangled.push_back(everyDefinedSymbol[i]);
            }
            if (nameType & prettyName) {
                if (regexEquiv(name, everyDefinedSymbol[i]->getPrettyName(), checkCase))
                    symsPretty.push_back(everyDefinedSymbol[i]);
            }
            if (nameType & typedName) {
                if (regexEquiv(name, everyDefinedSymbol[i]->getTypedName(), checkCase))
                    symsTyped.push_back(everyDefinedSymbol[i]);
            }
        }
    }

    std::vector<Symbol *> allSyms;
    
    for (unsigned i = 0; i < symsMangled.size(); i++) {
        if (   (sType == Symbol::ST_UNKNOWN) 
            || (sType == Symbol::ST_NOTYPE)
            || (symsMangled[i]->getType() == sType))
            allSyms.push_back(symsMangled[i]);
    }
    for (unsigned i = 0; i < symsPretty.size(); i++) {
        if    ((sType == Symbol::ST_UNKNOWN) 
           || (sType == Symbol::ST_NOTYPE)
           || (symsPretty[i]->getType() == sType))
            allSyms.push_back(symsPretty[i]);
    }
    for (unsigned i = 0; i < symsTyped.size(); i++) {
        if     ((sType == Symbol::ST_UNKNOWN) 
            || (sType == Symbol::ST_NOTYPE)
            || (symsTyped[i]->getType() == sType))
            allSyms.push_back(symsTyped[i]);
    }
    
    std::sort(allSyms.begin(), allSyms.end(), sort_by_sym_ptr);
    std::vector<Symbol *>::iterator endIter;
    endIter = std::unique(allSyms.begin(), allSyms.end());
    ret.insert(ret.end(), allSyms.begin(), endIter);

    if (ret.size() == old_size) {
        serr = No_Such_Symbol;
        return false;
    }
    else {
        return true;
    }
}

bool Symtab::getAllSymbols(std::vector<Symbol *> &ret)
{
    ret = everyDefinedSymbol;

    // add undefined symbols
    std::vector<Symbol *> temp;
    std::vector<Symbol *>::iterator it;
    getAllUndefinedSymbols(temp);
    for (it = temp.begin(); it != temp.end(); it++)
        ret.push_back(*it);

    if(ret.size() > 0)
        return true;
    serr = No_Such_Symbol;
    return false;
}

bool Symtab::getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType)
{
    if (sType == Symbol::ST_UNKNOWN)
        return getAllSymbols(ret);

    unsigned old_size = ret.size();
    // Filter by the given type
    for (unsigned i = 0; i < everyDefinedSymbol.size(); i++) {
        if (everyDefinedSymbol[i]->getType() == sType)
            ret.push_back(everyDefinedSymbol[i]);
    }
    // add undefined symbols
    std::vector<Symbol *> temp;
    getAllUndefinedSymbols(temp);
    for (unsigned i = 0; i < temp.size(); i++) {
        if (temp[i]->getType() == sType)
            ret.push_back(temp[i]);
    }

    if (ret.size() > old_size) {
        return true;
    }
    else {
        serr = No_Such_Symbol;
        return false;
    }
}

bool Symtab::getAllDefinedSymbols(std::vector<Symbol *> &ret)
{
    ret = everyDefinedSymbol;

    if(ret.size() > 0)
        return true;
    serr = No_Such_Symbol;
    return false;
}
 
bool Symtab::getAllUndefinedSymbols(std::vector<Symbol *> &ret){
    unsigned size = ret.size();
    map<string, std::vector<Symbol *> >::iterator it;
    std::vector<Symbol *>::iterator it2;
    for (it = undefDynSyms.begin(); it != undefDynSyms.end(); it++)
        for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
            ret.push_back(*it2);
    if(ret.size()>size)
        return true;
    serr = No_Such_Symbol;
    return false;
}

bool Symtab::findFuncByEntryOffset(Function *&ret, const Offset entry)
{
    /* XXX
     *
     * When working with relocatable files, a symbol is not uniquely identified
     * by its offset; it is uniquely identified by its Region and its offset.
     * This discrepancy is not taken into account here.
     */
    if (funcsByOffset.find(entry) != funcsByOffset.end()) {
        ret = funcsByOffset[entry];
        return true;
    }
    serr = No_Such_Function;
    return false;
}

bool sort_by_func_ptr(const Function *a, const Function *b) {
    return a < b;
}

bool Symtab::findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                 NameType nameType, bool isRegex, bool checkCase) {
    std::vector<Symbol *> funcSyms;
    if (!findSymbol(funcSyms, name, Symbol::ST_FUNCTION, nameType, isRegex, checkCase))
        return false;

    std::vector<Function *> unsortedFuncs;
    for (unsigned i = 0; i < funcSyms.size(); i++) 
    {
        if (doNotAggregate(funcSyms[i])) continue;
        if (!funcSyms[i]->getFunction())
        {
            fprintf(stderr, "%s[%d]:  WARNING:  internal inconsistency\n", FILE__, __LINE__);
            fprintf(stderr, "%s[%d]:  WARNING:  %s is %s a function\n", FILE__, __LINE__, name.c_str(), funcSyms[i]->isFunction() ? "" : "not");
            fprintf(stderr, "%s[%d]:  WARNING:  %s is %s a variable\n", FILE__, __LINE__, name.c_str(), funcSyms[i]->isVariable() ? "" : "not");
            continue;
        }
        unsortedFuncs.push_back(funcSyms[i]->getFunction());
    }

    std::sort(unsortedFuncs.begin(), unsortedFuncs.end(), sort_by_func_ptr);
    std::vector<Function *>::iterator endIter;
    endIter = std::unique(unsortedFuncs.begin(), unsortedFuncs.end());
    for (std::vector<Function *>::iterator iter = unsortedFuncs.begin();
         iter != endIter;
         iter++)
        ret.push_back(*iter);

    return true;
}

bool Symtab::getAllFunctions(std::vector<Function *> &ret) {
    ret = everyFunction;
    return (ret.size() > 0);
}

bool Symtab::findVariableByOffset(Variable *&ret, const Offset offset) {

    /* XXX
     *
     * See comment in findFuncByOffset about uniqueness of symbols in
     * relocatable files -- this discrepancy applies here as well.
     */
    if (varsByOffset.find(offset) != varsByOffset.end()) {
        ret = varsByOffset[offset];
        return true;
    }
    serr = No_Such_Variable;
    return false;
}

static bool sort_by_var_ptr(const Variable * a, const Variable *b) {
    return a < b;
}

bool Symtab::findVariablesByName(std::vector<Variable *> &ret, const std::string name,
                                 NameType nameType, bool isRegex, bool checkCase) {
    std::vector<Symbol *> varSyms;
    if (!findSymbol(varSyms, name, Symbol::ST_OBJECT, nameType, isRegex, checkCase))
        return false;

    std::vector<Variable *> unsortedVars;
    for (unsigned i = 0; i < varSyms.size(); i++) {
        if (doNotAggregate(varSyms[i])) continue;
        unsortedVars.push_back(varSyms[i]->getVariable());
    }

    std::sort(unsortedVars.begin(), unsortedVars.end(), sort_by_var_ptr);
    std::vector<Variable *>::iterator endIter;
    endIter = std::unique(unsortedVars.begin(), unsortedVars.end());
    for (std::vector<Variable *>::iterator iter = unsortedVars.begin();
         iter != endIter;
         iter++)
        ret.push_back(*iter);

    return true;
}

bool Symtab::getAllVariables(std::vector<Variable *> &ret) 
{
    ret = everyVariable;
    return (ret.size() > 0);
}

bool Symtab::getAllModules(std::vector<Module *> &ret)
{
    if (_mods.size() >0 )
    {
        ret = _mods;
        return true;
    }	

    serr = No_Such_Module;
    return false;
}

bool Symtab::findModuleByOffset(Module *&ret, Offset off)
{   
   //  this should be a hash, really
   for (unsigned int i = 0; i < _mods.size(); ++i) 
   {
      Module *mod = _mods[i];

      if (off == mod->addr()) 
      {
          ret = mod;
          return true;
      }
   } 
   return false;
}

bool Symtab::findModuleByName(Module *&ret, const std::string name)
{
   dyn_hash_map<std::string, Module *>::iterator loc;
   loc = modsByFileName.find(name);

   if (loc != modsByFileName.end()) 
   {
      ret = loc->second;
      return true;
   }

   loc = modsByFullName.find(name);

   if (loc != modsByFullName.end()) 
   {
      ret = loc->second;
      return true;
   }

   serr = No_Such_Module;
   ret = NULL;
   return false;
}

bool Symtab::getAllRegions(std::vector<Region *>&ret)
{
   if (regions_.size() > 0)
   {
      ret = regions_;
      return true;
   }

   return false;
}

bool Symtab::getCodeRegions(std::vector<Region *>&ret)
{
   if (codeRegions_.size() > 0)
   {
      ret = codeRegions_;
      return true;
   }

   return false;
}

bool Symtab::getDataRegions(std::vector<Region *>&ret)
{
   if (dataRegions_.size() > 0)
   {
      ret = dataRegions_;
      return true;
   }
   return false;
}


bool Symtab::getAllNewRegions(std::vector<Region *>&ret)
{
   std::vector<Region *> *retp = NULL;

   if (!getAnnotation(retp, UserRegionsAnno))
   {
      fprintf(stderr, "%s[%d]:  failed to get annotations here\n", FILE__, __LINE__);
      return false;
   }

   if (!retp)
   {
      fprintf(stderr, "%s[%d]:  failed to get annotations here\n", FILE__, __LINE__);
      return false;
   }

   ret = *retp;

   return true;
}

bool Symtab::getAllExceptions(std::vector<ExceptionBlock *> &exceptions)
{
   if (excpBlocks.size()>0)
   {
      exceptions = excpBlocks;
      return true;
   }	

   return false;
}


bool Symtab::findException(ExceptionBlock &excp, Offset addr)
{
   for (unsigned i=0; i<excpBlocks.size(); i++)
   {
      if (excpBlocks[i]->contains(addr))
      {
         excp = *(excpBlocks[i]);
         return true;
      }	
   }

   return false;
}

/**
 * Returns true if the Address range addr -> addr+size contains
 * a catch block, with excp pointing to the appropriate block
 **/
bool Symtab::findCatchBlock(ExceptionBlock &excp, Offset addr, unsigned size)
{
    int min = 0;
    int max = excpBlocks.size();
    int cur = -1, last_cur;

    if (max == 0)
        return false;

    //Binary search through vector for address
    while (true)
    {
        last_cur = cur;
        cur = (min + max) / 2;
    
        if (last_cur == cur)
            return false;

        Offset curAddr = excpBlocks[cur]->catchStart();
        if ((curAddr <= addr && curAddr+size > addr) ||
            (size == 0 && curAddr == addr))
        {
            //Found it
            excp = *(excpBlocks[cur]);
            return true;
        }
        if (addr < curAddr)
            max = cur;
        else if (addr > curAddr)
            min = cur;
    }
}
 
bool Symtab::findRegionByEntry(Region *&ret, const Offset offset)
{
    if(regionsByEntryAddr.find(offset) != regionsByEntryAddr.end())
    {
        ret = regionsByEntryAddr[offset];
        return true;
    }
    serr = No_Such_Region;
    return false;
}

/* Similar to binary search in isCode with the exception that here we
 * search to the end of regions without regards to whether they have
 * corresponding raw data on disk, and searches all regions.  
 *
 * regions_ elements that start at address 0 may overlap, ELF binaries
 * have 0 address iff they are not loadable, but xcoff places loadable
 * sections at address 0, including .text and .data
 */
Region *Symtab::findEnclosingRegion(const Offset where)
{
#if defined (os_aix) // regions overlap so do sequential search
    // try code regions first, then data, regions_ vector as last resort
    for (unsigned rIdx=0; rIdx < codeRegions_.size(); rIdx++) {
        if (where >= codeRegions_[rIdx]->getRegionAddr() &&
            where < (codeRegions_[rIdx]->getRegionAddr() 
                     + codeRegions_[rIdx]->getMemSize())) {
            return codeRegions_[rIdx];
        }
    }
    for (unsigned rIdx=0; rIdx < dataRegions_.size(); rIdx++) {
        if (where >= dataRegions_[rIdx]->getRegionAddr() &&
            where < (dataRegions_[rIdx]->getRegionAddr() 
                     + dataRegions_[rIdx]->getMemSize())) {
            return dataRegions_[rIdx];
        }
    }
    for (unsigned rIdx=0; rIdx < regions_.size(); rIdx++) {
        if (where >= regions_[rIdx]->getRegionAddr() &&
            where < (regions_[rIdx]->getRegionAddr() 
                     + regions_[rIdx]->getMemSize())) {
            return regions_[rIdx];
        }
    }
    return NULL;
#endif
    int first = 0; 
    int last = regions_.size() - 1;
    while (last >= first) {
        Region *curreg = regions_[(first + last) / 2];
        if (where >= curreg->getRegionAddr()
            && where < (curreg->getRegionAddr()
                        + curreg->getMemSize())) {
            return curreg;
        }
        else if (where < curreg->getRegionAddr()) {
            last = ((first + last) / 2) - 1;
        }
        else {/* where >= (cursec->getSecAddr()
                           + cursec->getSecSize()) */
            first = ((first + last) / 2) + 1;
        }
    }
    return NULL;
}

bool Symtab::findRegion(Region *&ret, const std::string secName)
{
    for(unsigned index=0;index<regions_.size();index++)
    {
        if(regions_[index]->getRegionName() == secName)
        {
            ret = regions_[index];
            return true;
        }
    }
    serr = No_Such_Region;
    return false;
}


bool Symtab::findRegion(Region *&ret, const Offset addr, const unsigned long size)
{
    for(unsigned index=0;index<regions_.size();index++)
    {
        if(regions_[index]->getRegionAddr() == addr && regions_[index]->getRegionSize() == size)
        {
            ret = regions_[index];
            return true;
        }
    }
    serr = No_Such_Region;
    return false;
}

///////////////////////// REGEX //////////////////////

// Use POSIX regular expression pattern matching to check if std::string s matches
// the pattern in this std::string
bool regexEquiv( const std::string &str,const std::string &them, bool checkCase ) 
{
   const char *str_ = str.c_str();
   const char *s = them.c_str();
   // Would this work under NT?  I don't know.
   //#if !defined(os_windows)
    return pattern_match(str_, s, checkCase);

}

// This function will match string s against pattern p.
// Asterisks match 0 or more wild characters, and a question
// mark matches exactly one wild character.  In other words,
// the asterisk is the equivalent of the regex ".*" and the
// question mark is the equivalent of "."

bool
pattern_match( const char *p, const char *s, bool checkCase ) {
   //const char *p = ptrn;
   //char *s = str;

    while ( true ) {
        // If at the end of the pattern, it matches if also at the end of the string
        if( *p == '\0' )
            return ( *s == '\0' );

        // Process a '*'
        if( *p == MULTIPLE_WILDCARD_CHARACTER ) {
            ++p;

            // If at the end of the pattern, it matches
            if( *p == '\0' )
                return true;

            // Try to match the remaining pattern for each remaining substring of s
            for(; *s != '\0'; ++s )
                if( pattern_match( p, s, checkCase ) )
                    return true;
            // Failed
            return false;
        }

        // If at the end of the string (and at this point, not of the pattern), it fails
        if( *s == '\0' )
            return false;

        // Check if this character matches
        bool matchChar = false;
        if( *p == WILDCARD_CHARACTER || *p == *s )
            matchChar = true;
        else if( !checkCase ) {
            if( *p >= 'A' && *p <= 'Z' && *s == ( *p + ( 'a' - 'A' ) ) )
                matchChar = true;
            else if( *p >= 'a' && *p <= 'z' && *s == ( *p - ( 'a' - 'A' ) ) )
                matchChar = true;
        }

        if( matchChar ) {
            ++p;
            ++s;
            continue;
        }

        // Did not match
        return false;
    }
}

struct Dyninst::SymtabAPI::SymbolCompareByAddr
{
    bool operator()(Function *a, Function *b)
    {
       return (a->offset_ < b->offset_);
    }
};

bool Symtab::getContainingFunction(Offset offset, Function* &func)
{
   if (!isCode(offset)) {
      return false;
   }
   if (everyFunction.size() && !sorted_everyFunction)
   {
      std::sort(everyFunction.begin(), everyFunction.end(),
                SymbolCompareByAddr());
      sorted_everyFunction = true;
   }
   
   unsigned low = 0;
   unsigned high = everyFunction.size();
   unsigned last_mid = high+1;
   unsigned mid;
   if (!high) return false;
   for (;;)
   {
      mid = (low + high) / 2;
      if (last_mid == mid)
         break;
      last_mid = mid;
      Offset cur = everyFunction[mid]->getOffset();
      if (cur > offset) {
         high = mid;
         continue;
      }
      if (cur < offset) {
         low = mid;
         continue;
      }
      if (cur == offset) {
         func = everyFunction[mid];
         return true;
      }
   }

   if ((everyFunction[low]->getOffset() <= offset) &&
       ((low+1 == everyFunction.size()) || 
        (everyFunction[low+1]->getOffset() > offset)))
   {
         func = everyFunction[low];
         return true;
   }
   return false;
}

Module *Symtab::getDefaultModule() {
    Module *mod = NULL;
    // TODO: automatically pick the module that contains this address?
    // For now, DEFAULT_MODULE or (if we have only one) that one.
    if (_mods.size() == 1)
        return _mods[0];
    else {
        if (!findModuleByName(mod, "DEFAULT_MODULE"))
            return NULL;
    }
    return mod;
}

unsigned Function::getSize() {
   if (functionSize_)
      return functionSize_;
   for (unsigned i=0; i<symbols_.size(); i++) {
      if (symbols_[i]->getSize()) { 
         functionSize_ = symbols_[i]->getSize();;
         return functionSize_;
      }
   }

   Symtab *symtab = getFirstSymbol()->getSymtab();
   if (symtab->everyFunction.size() && !symtab->sorted_everyFunction)
   {
      std::sort(symtab->everyFunction.begin(), symtab->everyFunction.end(),
                SymbolCompareByAddr());
      symtab->sorted_everyFunction = true;
   }

   Offset offset = getOffset();
   unsigned low = 0;
   unsigned high = symtab->everyFunction.size();
   unsigned last_mid = high+1;
   unsigned mid;
   for (;;)
   {
      mid = (low + high) / 2;
      if (last_mid == mid)
         return 0;
      last_mid = mid;
      Offset cur = symtab->everyFunction[mid]->getOffset();
      if (cur > offset) {
         high = mid;
         continue;
      }
      if (cur < offset) {
         low = mid;
         continue;
      }
      if (cur == offset) {
         if (mid + 1 >= symtab->everyFunction.size())
            return 0;
         Function *next_func = symtab->everyFunction[mid+1];
         functionSize_ = next_func->getOffset() - getOffset();
         return functionSize_;
      }
   }
}

