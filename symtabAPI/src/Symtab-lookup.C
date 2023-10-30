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

/* Lookup functions defined in class Symtab. Separated to reduce file size and classify. */



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "common/src/Timer.h"
#include "common/src/pathName.h"

#include "debug.h"
#include "Symtab.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"
#include "annotations.h"
#include "symtab_impl.hpp"
#include "symtabAPI/src/Object.h"

#include <boost/iterator/function_output_iterator.hpp>
#include <boost/foreach.hpp>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

// extern thread_local SymtabError serr;

bool regexEquiv( const std::string &str,const std::string &them, bool checkCase );
bool pattern_match( const char *p, const char *s, bool checkCase );

std::vector<Symbol *> Symtab::findSymbolByOffset(Offset o)
{
   decltype(impl->everyDefinedSymbol)::by_offset_t::const_accessor oa;
   if(impl->everyDefinedSymbol.by_offset.find(oa, o))
       return oa->second;
   return std::vector<Symbol*>();
}

bool Symtab::findSymbol(std::vector<Symbol *> &ret, const std::string& name,
                        Symbol::SymbolType sType, NameType nameType,
                        bool isRegex, bool checkCase, bool includeUndefined)
{
    unsigned old_size = ret.size();

    std::vector<Symbol *> candidates;
    
    if (!isRegex) {
        // Easy case
        if (nameType & mangledName) {
          {
            decltype(impl->everyDefinedSymbol)::by_name_t::const_accessor ma;
            if(impl->everyDefinedSymbol.by_mangled.find(ma, name))
              candidates.insert(candidates.end(), ma->second.begin(), ma->second.end());
          }
          if(includeUndefined) {
            decltype(impl->undefDynSyms)::by_name_t::const_accessor ma;
            if(impl->undefDynSyms.by_mangled.find(ma, name))
              candidates.insert(candidates.end(), ma->second.begin(), ma->second.end());
          }
        }
        if (nameType & prettyName) {
          {
            decltype(impl->everyDefinedSymbol)::by_name_t::const_accessor pa;
            if(impl->everyDefinedSymbol.by_pretty.find(pa, name))
              candidates.insert(candidates.end(), pa->second.begin(), pa->second.end());
          }
          if(includeUndefined) {
            decltype(impl->undefDynSyms)::by_name_t::const_accessor pa;
            if(impl->undefDynSyms.by_pretty.find(pa, name))
              candidates.insert(candidates.end(), pa->second.begin(), pa->second.end());
          }
        }
        if (nameType & typedName) {
          {
            decltype(impl->everyDefinedSymbol)::by_name_t::const_accessor ta;
            if(impl->everyDefinedSymbol.by_typed.find(ta, name))
              candidates.insert(candidates.end(), ta->second.begin(), ta->second.end());
          }
          if(includeUndefined) {
            decltype(impl->undefDynSyms)::by_name_t::const_accessor ta;
            if(impl->undefDynSyms.by_typed.find(ta, name))
              candidates.insert(candidates.end(), ta->second.begin(), ta->second.end());
          }
        }
    }
    else {
        // Build the regex list of symbols
        // We need to iterate over every single symbol. Ugh.
       if (includeUndefined) {
          cerr << "Warning: regex search of undefined symbols is not supported" << endl;
       }

       for (auto i = impl->everyDefinedSymbol.begin(); i != impl->everyDefinedSymbol.end(); i++) {
          if (nameType & mangledName) {
	    if (regexEquiv(name, (*i)->getMangledName(), checkCase))
                candidates.push_back(*i);

          }
          if (nameType & prettyName) {
	    if (regexEquiv(name, (*i)->getPrettyName(), checkCase))
                candidates.push_back(*i);
          }
          if (nameType & typedName) {
	    if (regexEquiv(name, (*i)->getTypedName(), checkCase))
                candidates.push_back(*i);
          }
       }
    }

    std::set<Symbol *> matches;

    for (std::vector<Symbol *>::iterator iter = candidates.begin();
         iter != candidates.end(); ++iter) {
       if (sType == Symbol::ST_UNKNOWN ||
           sType == Symbol::ST_NOTYPE ||
           sType == (*iter)->getType() ||
           (sType == Symbol::ST_OBJECT && (*iter)->getType() == Symbol::ST_TLS)) //Treat TLS as variables
       {
          matches.insert(*iter);
       }
    }

    ret.insert(ret.end(), matches.begin(), matches.end());

    if (ret.size() == old_size) {
	setSymtabError(No_Such_Symbol);
        return false;
    }
    else {
        return true;
    }
}

bool Symtab::getAllSymbols(std::vector<Symbol *> &ret)
{
  std::copy(impl->everyDefinedSymbol.begin(), impl->everyDefinedSymbol.end(), back_inserter(ret));
  std::copy(impl->undefDynSyms.begin(), impl->undefDynSyms.end(), back_inserter(ret));
  
  if(ret.size() > 0) {
    return true;
  } else {
    setSymtabError(No_Such_Symbol);
    return false;
  }
}

bool Symtab::getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType)
{
    if (sType == Symbol::ST_UNKNOWN)
        return getAllSymbols(ret);

    unsigned old_size = ret.size();

    // Filter by the given type
    for (auto i = impl->everyDefinedSymbol.begin(); i != impl->everyDefinedSymbol.end(); i++) {
      if ((*i)->getType() == sType) 
      {
	ret.push_back(*i);
      }
      
    }
    for (auto j = impl->undefDynSyms.begin(); j != impl->undefDynSyms.end(); j++) {
      if ((*j)->getType() == sType) 
      {
	ret.push_back(*j);
      }
      
    }

    if (ret.size() > old_size) {
        return true;
    }
    else {
	setSymtabError(No_Such_Symbol);
        return false;
    }
}

bool Symtab::getAllDefinedSymbols(std::vector<Symbol *> &ret)
{
  ret.clear();

  std::copy(impl->everyDefinedSymbol.begin(), impl->everyDefinedSymbol.end(), back_inserter(ret));

    if(ret.size() > 0)
        return true;
    setSymtabError(No_Such_Symbol);
    return false;
}
 
bool Symtab::getAllUndefinedSymbols(std::vector<Symbol *> &ret){
    unsigned size = ret.size();

    ret.insert(ret.end(), impl->undefDynSyms.begin(), impl->undefDynSyms.end());

    if(ret.size()>size)
        return true;
    setSymtabError(No_Such_Symbol);
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
    {
        dyn_c_hash_map<Offset,Function*>::const_accessor ca;
        if (impl->funcsByOffset.find(ca, entry)) {
            ret = ca->second;
            return true;
        }
    }
    setSymtabError(No_Such_Symbol);
    return false;
}

bool sort_by_func_ptr(const Function *a, const Function *b) {
    return a < b;
}

bool Symtab::findFunctionsByName(std::vector<Function *> &ret, const std::string name,
                                 NameType nameType, bool isRegex, bool checkCase) {
    std::vector<Symbol *> funcSyms;
    if (!findSymbol(funcSyms, name, Symbol::ST_FUNCTION, nameType, isRegex, checkCase)) {
      return false;
    }
    std::vector<Function *> unsortedFuncs;
    for (unsigned i = 0; i < funcSyms.size(); i++) 
    {
      if (doNotAggregate(funcSyms[i])) {
	continue;
      }
      if (!funcSyms[i]->getFunction())
        {
           create_printf("%s[%d]:  WARNING:  internal inconsistency\n", FILE__, __LINE__);
           create_printf("%s[%d]:  WARNING:  %s is %s a function\n", FILE__, __LINE__, name.c_str(), funcSyms[i]->isFunction() ? "" : "not");
           create_printf("%s[%d]:  WARNING:  %s is %s a variable\n", FILE__, __LINE__, name.c_str(), funcSyms[i]->isVariable() ? "" : "not");
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

bool Symtab::findVariablesByOffset(std::vector<Variable *> &ret, const Offset offset) {

    /* XXX
     *
     * See comment in findFuncByOffset about uniqueness of symbols in
     * relocatable files -- this discrepancy applies here as well.
     */
    {
        decltype(impl->varsByOffset)::const_accessor ca;
        if (impl->varsByOffset.find(ca, offset)) {
            ret = ca->second;
            return true;
        }
    }
    ret.clear();
    setSymtabError(No_Such_Symbol);
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
  auto const size = ret.size();
  std::copy(impl->modules.begin(), impl->modules.end(), std::back_inserter(ret));
  return ret.size() != size;
}


bool Symtab::findModuleByOffset(Module *&ret, Offset off)
{
   ret = findModuleByOffset(off);
   return ret != nullptr;
}

Module* Symtab::findModuleByOffset(Offset offset) const {
  return impl->modules.find(offset);
}

std::vector<Module*> Symtab::findModulesByName(std::string const& name) const {
  return impl->modules.find(name);
}

Module* Symtab::getContainingModule(Offset offset) const {
  return impl->getContainingModule(offset);
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
      return false;
   }

   if (!retp)
   {
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
    setSymtabError(No_Such_Region);
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
    int first = 0; 
    int last = regions_.size() - 1;
    while (last >= first) {
        Region *curreg = regions_[(first + last) / 2];
        if (where >= curreg->getMemOffset()
            && where < (curreg->getMemOffset()
                        + curreg->getMemSize())) {
            return curreg;
        }
        else if (where < curreg->getMemOffset()) {
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
    setSymtabError(No_Such_Region);
    return false;
}


bool Symtab::findRegion(Region *&ret, const Offset addr, const unsigned long size)
{
   ret = NULL;
   for(unsigned index=0;index<regions_.size();index++) {
      if(regions_[index]->getMemOffset() == addr && regions_[index]->getMemSize() == size) {
         if (ret) {
#if 0
            cerr << "Error: region inconsistency" << endl;
            cerr << "\t" << ret->getRegionName() << " @ "
                 << hex << ret->getMemOffset() << "/" << ret->getMemSize() 
		 << ", type " << Region::regionType2Str(ret->getRegionType()) << endl;
            cerr << "\t" << regions_[index]->getRegionName() << " @ "
                 << regions_[index]->getMemOffset() << "/" << regions_[index]->getMemSize() 
		 << ", type " << Region::regionType2Str(regions_[index]->getRegionType()) << endl;
#endif
	   assert((addr == 0) ||
		  (ret->getRegionType() == Region::RT_BSS) ||
		  (regions_[index]->getRegionType() == Region::RT_BSS));

	    // Probably don't want bss
	    if (ret->getRegionType() == Region::RT_BSS) {
	      ret = regions_[index];
	    }


	    setSymtabError(Multiple_Region_Matches);
            return false;
         }
         ret = regions_[index];
      }
   }
   if (ret) return true;
   setSymtabError(No_Such_Region);
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
    bool operator()(Function *a, Function *b) const
    {
       return (a->offset_ < b->offset_);
    }
};


bool Symtab::addFunctionRange(FunctionBase *func, Dyninst::Offset next_start)
{
   Dyninst::Offset sym_low, sym_high;
   bool found_sym_range = false;

   sym_low = func->getOffset();
   if (func->getSize())
      sym_high = sym_low + func->getSize();
   else if (next_start) {
      sym_high = next_start;
   }
   else {
      //Inlined symbol, no real way to get size estimates.  Have to rely 
      // on debug info.
      sym_low = sym_high = 0;
   }
   
   //Add dwarf/debug info ranges to func_lookup
   FuncRangeCollection &ranges = const_cast<FuncRangeCollection &>(func->getRanges());
   for (FuncRangeCollection::iterator i = ranges.begin(); i != ranges.end(); i++) {
      FuncRange &range = *i;
      if (range.low() == sym_low && range.high() == sym_high)
         found_sym_range = true;      
      impl->func_lookup.insert(&range);
   }

   //Add symbol range to func_lookup, if present and not already added
   if (!found_sym_range && sym_low && sym_high) {
      FuncRange *frange = new FuncRange(sym_low, sym_high - sym_low, func);      
      impl->func_lookup.insert(frange);
   }

   //Recursively add inlined functions
   const InlineCollection &inlines = func->getInlines();
   for (InlineCollection::const_iterator i = inlines.begin(); i != inlines.end(); i++) {
      addFunctionRange(*i, 0);
   }
   return true;
}

bool Symtab::parseFunctionRanges()
{
   parseTypesNow();

   if (everyFunction.size() && !sorted_everyFunction)
   {
      std::sort(everyFunction.begin(), everyFunction.end(),
                SymbolCompareByAddr());
      sorted_everyFunction = true;
   }

   for (vector<Function *>::iterator i = everyFunction.begin(); i != everyFunction.end(); i++) {

      //Compute the start of the next function, if any.  Use region end if no
      // next function.
      vector<Function *>::iterator next = i+1;
      Address next_addr = 0;
      if (next != everyFunction.end()) {
         next_addr = (*next)->getOffset();
      }
      else {
         Region *region = findEnclosingRegion((*i)->getOffset());
         if (region) {
            next_addr = region->getMemOffset() + region->getMemSize(); 
         }
      }

      //Add current function to lookups.
      addFunctionRange(*i, next_addr);
   }

   return true;
}

bool Symtab::getContainingFunction(Offset offset, Function* &func)
{
   if (!isCode(offset)) {
      return false;
   }

   // Lazily parse the function ranges, but ensure we only do it once.
   std::call_once(impl->funcRangesAreParsed, [this](){ this->parseFunctionRanges(); });
   
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

bool Symtab::getContainingInlinedFunction(Offset offset, FunctionBase* &func)
{   
   // Lazily parse the function ranges, but ensure we only do it once.
   std::call_once(impl->funcRangesAreParsed, [this](){ this->parseFunctionRanges(); });
   
   set<FuncRange *> ranges;
   int num_found = impl->func_lookup.find(offset, ranges);
   if (num_found == 0) {
      func = NULL;
      return false;
   }
   if (num_found == 1) {
      func = (*ranges.begin())->container;
      return true;
   }

   // Theoretically, if an address is inlined through a call chain,
   // There should be only one call chain that contains this address.
   // However, due to bad DWARF generated by old compilers regarding
   // OpenMP outlined code regions, it is possible that an address
   // is included in multiple disjointed call chains.
   // From one call chain, it looks like the address is not inlined.
   // From the other, it looks like the address is inlined.
   // Therefore, here we heuristicaly prefer the deeper call chain
   // to reflect inlining information.
   int maxDepth = 0;
   for (auto range : ranges) {      
      FunctionBase *cur_func = range->container;
      int depth = 0;
      while (cur_func) {
         depth++;
         cur_func = cur_func->getInlinedParent();
      }
      if (depth > maxDepth) {
         maxDepth = depth;
         func = range->container;
      }
   }   
   return true;
}

Module *Symtab::getDefaultModule() const {
    return impl->default_module;
}

unsigned Function::getSymbolSize() const {
   if (functionSize_)
      return functionSize_;
   for (unsigned i=0; i<symbols_.size(); i++) {
      if (symbols_[i]->getSize()) { 
         functionSize_ = symbols_[i]->getSize();;
         return functionSize_;
      }
   }
   return 0;
}

unsigned Function::getSize() const {
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

Dyninst::Offset Symtab::fileToDiskOffset(Dyninst::Offset fileOffset) const {
   for (unsigned j = 0; j < regions_.size(); ++j) {
      if (regions_[j]->getFileOffset() <= fileOffset &&
          ((regions_[j]->getFileOffset() + regions_[j]->getDiskSize()) > fileOffset)) {
         return fileOffset - regions_[j]->getFileOffset() + regions_[j]->getDiskOffset();
      }
   }
   return (Dyninst::Offset) -1;
}

Dyninst::Offset Symtab::fileToMemOffset(Dyninst::Offset fileOffset) const {
   for (unsigned j = 0; j < regions_.size(); ++j) {
      if (regions_[j]->getFileOffset() <= fileOffset &&
          ((regions_[j]->getFileOffset() + regions_[j]->getDiskSize()) > fileOffset)) {
         return fileOffset - regions_[j]->getFileOffset() + regions_[j]->getMemOffset();
      }
   }
   return (Dyninst::Offset) -1;
}
