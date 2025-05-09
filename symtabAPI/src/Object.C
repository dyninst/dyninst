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

// $Id: Object.C,v 1.31 2008/11/03 15:19:25 jaw Exp $

#include "symutil.h"
#include "Annotatable.h"

#include "Symtab.h"
#include "Module.h"
#include "Region.h"
#include "Collections.h"
#include "annotations.h"
#include "Symbol.h"

#include "Aggregate.h"
#include "Function.h"
#include "Variable.h"

#include "symtabAPI/src/Object.h"

#if defined(os_linux) || defined(os_freebsd)
#include "Object-elf.h"
#include "Object-pe.h"
#elif defined(os_windows)
#include "Object-nt.h"
#else
#error "unknown platform"
#endif

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

const char *Dyninst::SymtabAPI::supportedLanguages2Str(supportedLanguages s)
{
   switch(s) {
      CASE_RETURN_STR(lang_Unknown);
      CASE_RETURN_STR(lang_Assembly);
      CASE_RETURN_STR(lang_C);
      CASE_RETURN_STR(lang_CPlusPlus);
      CASE_RETURN_STR(lang_GnuCPlusPlus);
      CASE_RETURN_STR(lang_Fortran);
      CASE_RETURN_STR(lang_CMFortran);
   };
   return "bad_language";
}


bool Dyninst::SymtabAPI::symbol_compare(const Symbol *s1, const Symbol *s2) 
{
    // select the symbol with the lowest address
    Offset s1_addr = s1->getOffset();
    Offset s2_addr = s2->getOffset();
    if (s1_addr > s2_addr)
    	return false;
    if (s1_addr < s2_addr)
    	return true;

    // symbols are co-located at the same address
    // select the symbol which is not a function
    if ((s1->getType() != Symbol::ST_FUNCTION) && (s2->getType() == Symbol::ST_FUNCTION))
    	return true;
    if ((s2->getType() != Symbol::ST_FUNCTION) && (s1->getType() == Symbol::ST_FUNCTION))
    	return false;
    
    // symbols are both functions
    // select the symbol which has GLOBAL linkage
    if ((s1->getLinkage() == Symbol::SL_GLOBAL) && (s2->getLinkage() != Symbol::SL_GLOBAL))
    	return true;
    if ((s2->getLinkage() == Symbol::SL_GLOBAL) && (s1->getLinkage() != Symbol::SL_GLOBAL))
    	return false;
	
    // neither function is GLOBAL
    // select the symbol which has LOCAL linkage
    if ((s1->getLinkage() == Symbol::SL_LOCAL) && (s2->getLinkage() != Symbol::SL_LOCAL))
    	return true;
    if ((s2->getLinkage() == Symbol::SL_LOCAL) && (s1->getLinkage() != Symbol::SL_LOCAL))
    	return false;
    
    // both functions are WEAK
    
    // Apparently sort requires a strict weak ordering
    // and fails for equality. our compare
    // function behaviour should be as follows
    // f(x,y) => !f(y,x)
    // f(x,y),f(y,z) => f(x,z)
    // f(x,x) = false. 
    // So return which ever is first in the array. May be that would help.
    return (s1 < s2);
}


bool Object::needs_function_binding() const 
{
    return false;
}

bool Object::get_func_binding_table(std::vector<relocationEntry> &) const 
{
    return false;
}

bool Object::get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const 
{
    return false;
}

bool Object::addRelocationEntry(relocationEntry &)
{
    return true;
}

char *Object::mem_image() const
{
	return NULL;
}

DYNINST_EXPORT Offset ExceptionBlock::catchStart() const 
{
	return catchStart_;
}

#ifdef DEBUG 
ostream &operator<<(ostream &os, relocationEntry &q) {
   return q.operator<<(os);
}
#endif

/**************************************************
 *
 *  Stream based debuggering output - for regreesion testing.
 *  Dump info on state of object *this....
 *
 **************************************************/

DYNINST_EXPORT unsigned Object::nsymbols () const 
{ 
    unsigned n = 0;
    for (dyn_c_hash_map<std::string, std::vector<Symbol *> >::const_iterator i = symbols_.begin();
         i != symbols_.end();
         i++) {
        n += i->second.size();
    }
    return n;
}

DYNINST_EXPORT bool Object::get_symbols(string & name, 
      std::vector<Symbol *> &symbols ) 
{
   dyn_c_hash_map<std::string, std::vector<Symbol *>>::const_accessor ca;
   if ( !symbols_.find(ca, name)) {
      return false;
   }

   symbols = ca->second;
   return true;
}

DYNINST_EXPORT char* Object::code_ptr () const 
{ 
   return code_ptr_; 
}

DYNINST_EXPORT Offset Object::code_off () const 
{ 
   return code_off_; 
}

DYNINST_EXPORT Offset Object::code_len () const 
{ 
   return code_len_; 
}

DYNINST_EXPORT char* Object::data_ptr () const 
{ 
   return data_ptr_; 
}

DYNINST_EXPORT Offset Object::data_off () const 
{ 
   return data_off_; 
}

DYNINST_EXPORT Offset Object::data_len () const 
{ 
   return data_len_; 
}

DYNINST_EXPORT bool Object::is_aout() const 
{
   return is_aout_;  
}

DYNINST_EXPORT bool Object::isDynamic() const 
{
   return is_dynamic_;  
}

DYNINST_EXPORT unsigned Object::no_of_sections() const 
{ 
   return no_of_sections_; 
}

DYNINST_EXPORT unsigned Object::no_of_symbols() const 
{ 
   return no_of_symbols_;  
}

DYNINST_EXPORT bool Object::getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const
{
   for (unsigned i=0;i<catch_addrs_.size();i++)
      excpBlocks.push_back(new ExceptionBlock(catch_addrs_[i]));

   return true;
}

DYNINST_EXPORT std::vector<Region *> Object::getAllRegions() const
{
   return regions_;	
}

DYNINST_EXPORT Offset Object::loader_off() const 
{ 
   return loader_off_; 
}

DYNINST_EXPORT unsigned Object::loader_len() const 
{ 
   return loader_len_; 
}


DYNINST_EXPORT int Object::getAddressWidth() const 
{ 
   return addressWidth_nbytes; 
}

DYNINST_EXPORT bool Object::have_deferred_parsing(void) const
{ 
   return deferredParse;
}

DYNINST_EXPORT void * Object::getErrFunc() const 
{
   return (void *) err_func_; 
}

DYNINST_EXPORT dyn_c_hash_map< string, std::vector< Symbol *> > *Object::getAllSymbols()
{
   return &(symbols_);
}

DYNINST_EXPORT Object::~Object() 
{
    using std::string;
    using std::vector;

    dyn_c_hash_map<string,vector<Symbol *> >::iterator it = symbols_.begin();
    for( ; it != symbols_.end(); ++it) {
        vector<Symbol *> & v = (*it).second;
        for(unsigned i=0;i<v.size();++i)
            delete v[i];
        v.clear();
    }
}

// explicitly protected
DYNINST_EXPORT Object::Object(MappedFile *mf_, void (*err_func)(const char *), Symtab* st)
: mf(mf_),
   code_ptr_(0), code_off_(0), code_len_(0),
   data_ptr_(0), data_off_(0), data_len_(0),
   code_vldS_(0), code_vldE_(0),
   data_vldS_(0), data_vldE_(0),
   loader_off_(0), loader_len_(0),
   is_aout_(false), is_dynamic_(false),
   has_error(false), is_static_binary_(false),
   no_of_sections_(0), no_of_symbols_(0),
  deferredParse(false), parsedAllLineInfo(false), err_func_(err_func), addressWidth_nbytes(4),
  associated_symtab(st), file_format_(FileFormat::Unknown)
{
}

Object *Dyninst::SymtabAPI::parseObjectFile(MappedFile *mf,
                                            bool is_defensive,
                                            void (*err)(const char *),
                                            bool alloc_syms,
                                            Symtab *symtab)
{
    const char *mfa = (const char *)mf->base_addr();
    if (mfa[1] == 'E' && mfa[2] == 'L' && mfa[3] == 'F') {
        return new ObjectELF(mf, is_defensive, err, alloc_syms, symtab);
    } else if (mfa[0] == 'M' && mfa[1] == 'Z') {
        return new ObjectPE(mf, is_defensive, err, alloc_syms, symtab);
    } else {
        return NULL;
    }
}

SymbolIter::SymbolIter( Object & obj ) 
: symbols(obj.getAllSymbols()), currentPositionInVector(0) 
{
   symbolIterator = obj.getAllSymbols()->begin();
}

SymbolIter::SymbolIter( const SymbolIter & src ) 
: symbols(src.symbols),currentPositionInVector(0),
   symbolIterator( src.symbolIterator ) 
{
}

SymbolIter::~SymbolIter () 
{
}


void SymbolIter::reset () 
{
   currentPositionInVector = 0;
   symbolIterator = symbols->begin();
}

SymbolIter::operator bool() const
{
   return (symbolIterator!=symbols->end());
}

void SymbolIter::operator++ ( int ) 
{
   if ( currentPositionInVector + 1 < (symbolIterator->second).size())
   {
      currentPositionInVector++;
      return;
   }

   /* Otherwise, we need a new std::vector. */
   currentPositionInVector = 0;			
   symbolIterator++;
}

const string & SymbolIter::currkey() const 
{
   return symbolIterator->first;
}

/* If it's important that this be const, we could try to initialize
   currentVector to '& symbolIterator.currval()' in the constructor. */

Symbol *SymbolIter::currval() 
{
	if (currentPositionInVector >= symbolIterator->second.size())
	{
		return NULL;
	}
   return ((symbolIterator->second)[ currentPositionInVector ]);
}

bool Object::hasError() const
{
  return has_error;
}

void Object::setTruncateLinePaths(bool)
{
}

bool Object::getTruncateLinePaths()
{
   return false;
}
