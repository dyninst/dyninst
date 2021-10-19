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

#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


//#ifdef BINEDIT_DEBUG
bool ____sym_hdr_printed = false;
void print_symbols( std::vector< Symbol *>& allsymbols ) {
    FILE* fd = stdout;
    Symbol *sym;
    std::string modname;
    if (!____sym_hdr_printed) {
        fprintf(fd, "%-20s  %-15s  %-10s  %5s  SEC  TYP  LN  VIS  INFO\n", 
                "SYMBOL", "MODULE", "ADDR", "SIZE");
        ____sym_hdr_printed = true;
    }
    for (unsigned i=0; i<allsymbols.size(); i++) {
        sym = allsymbols[i];
        modname = (sym->getModule() ? sym->getModule()->fileName() : "");
        //if (sym->getName() == "__gmon_start__") {
        //if (modname == "libspecial.so" || modname == "libprofile.so") {
        //if (sym->getLinkage() == Symbol::SL_WEAK) {
        //if (sym->isInDynSymtab()) {
        if (1) {
            fprintf(fd, "%-20s  %-15s  0x%08x  %5u  %3u", 
                    sym->getMangledName().substr(0,20).c_str(), 
                //modname.size() > 15 ? modname.substr(modname.size()-15,15).c_str() : modname.c_str(),
                "",
                (unsigned)sym->getOffset(),
                (unsigned)sym->getSize(),
                    sym->getRegion() ? sym->getRegion()->getRegionNumber() : 0
                );
            switch (sym->getType()) {
                case Symbol::ST_FUNCTION: fprintf(fd, "  FUN"); break;
                case Symbol::ST_TLS:      fprintf(fd, "  TLS"); break;
                case Symbol::ST_OBJECT:   fprintf(fd, "  OBJ"); break;
                case Symbol::ST_MODULE:   fprintf(fd, "  MOD"); break;
                case Symbol::ST_SECTION:  fprintf(fd, "  SEC"); break;
                case Symbol::ST_DELETED:  fprintf(fd, "  DEL"); break;
                case Symbol::ST_NOTYPE:   fprintf(fd, "   - "); break;
                default:
                case Symbol::ST_UNKNOWN:  fprintf(fd, "  ???"); break;                 
            }
            switch (sym->getLinkage()) {
                case Symbol::SL_UNKNOWN: fprintf(fd, "  ??"); break;
                case Symbol::SL_GLOBAL:  fprintf(fd, "  GL"); break;
                case Symbol::SL_LOCAL:   fprintf(fd, "  LO"); break;
                case Symbol::SL_WEAK:    fprintf(fd, "  WK"); break;
                case Symbol::SL_UNIQUE:  fprintf(fd, "  UQ"); break;
            }
            switch (sym->getVisibility()) {
                case Symbol::SV_UNKNOWN:   fprintf(fd, "  ???"); break;
                case Symbol::SV_DEFAULT:   fprintf(fd, "   - "); break;
                case Symbol::SV_INTERNAL:  fprintf(fd, "  INT"); break;
                case Symbol::SV_HIDDEN:    fprintf(fd, "  HID"); break;
                case Symbol::SV_PROTECTED: fprintf(fd, "  PRO"); break;
            }
            fprintf(fd, " ");
            if (sym->isInSymtab())
                fprintf(fd, " STA");
            if (sym->isInDynSymtab())
                fprintf(fd, " DYN");
            if (sym->isAbsolute())
                fprintf(fd, " ABS");
            if (sym->isDebug())
                fprintf(fd, " DBG");
            std::string fileName;
            std::vector<std::string> *vers;
            if (sym->getVersionFileName(fileName))
                fprintf(fd, "  [%s]", fileName.c_str());
            if (sym->getVersions(vers)) {
                fprintf(fd, " {");
                for (unsigned j=0; j < vers->size(); j++) {
                    if (j > 0)
                        fprintf(fd, ", ");
                    fprintf(fd, "%s", (*vers)[j].c_str());
                }
                fprintf(fd, "}");
            }
            fprintf(fd,"\n");
        }
    }
}
void print_symbol_map( dyn_hash_map< std::string, std::vector< Symbol *> > *symbols) {
    dyn_hash_map< std::string, std::vector< Symbol *> >::iterator siter = symbols->begin();
    int total_syms = 0;
    while (siter != symbols->end()) {
        print_symbols(siter->second);
        total_syms += siter->second.size();
        siter++;
    }
    printf("%d total symbol(s)\n", total_syms);
}
//#endif


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


bool AObject::needs_function_binding() const 
{
    return false;
}

bool AObject::get_func_binding_table(std::vector<relocationEntry> &) const 
{
    return false;
}

bool AObject::get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const 
{
    return false;
}

bool AObject::addRelocationEntry(relocationEntry &)
{
    return true;
}

char *AObject::mem_image() const
{
	return NULL;
}

SYMTAB_EXPORT Offset ExceptionBlock::catchStart() const 
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

SYMTAB_EXPORT unsigned AObject::nsymbols () const 
{ 
    unsigned n = 0;
    for (dyn_c_hash_map<std::string, std::vector<Symbol *> >::const_iterator i = symbols_.begin();
         i != symbols_.end();
         i++) {
        n += i->second.size();
    }
    return n;
}

SYMTAB_EXPORT bool AObject::get_symbols(string & name, 
      std::vector<Symbol *> &symbols ) 
{
   dyn_c_hash_map<std::string, std::vector<Symbol *>>::const_accessor ca;
   if ( !symbols_.find(ca, name)) {
      return false;
   }

   symbols = ca->second;
   return true;
}

SYMTAB_EXPORT char* AObject::code_ptr () const 
{ 
   return code_ptr_; 
}

SYMTAB_EXPORT Offset AObject::code_off () const 
{ 
   return code_off_; 
}

SYMTAB_EXPORT Offset AObject::code_len () const 
{ 
   return code_len_; 
}

SYMTAB_EXPORT char* AObject::data_ptr () const 
{ 
   return data_ptr_; 
}

SYMTAB_EXPORT Offset AObject::data_off () const 
{ 
   return data_off_; 
}

SYMTAB_EXPORT Offset AObject::data_len () const 
{ 
   return data_len_; 
}

SYMTAB_EXPORT bool AObject::is_aout() const 
{
   return is_aout_;  
}

SYMTAB_EXPORT bool AObject::isDynamic() const 
{
   return is_dynamic_;  
}

SYMTAB_EXPORT unsigned AObject::no_of_sections() const 
{ 
   return no_of_sections_; 
}

SYMTAB_EXPORT unsigned AObject::no_of_symbols() const 
{ 
   return no_of_symbols_;  
}

SYMTAB_EXPORT bool AObject::getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const
{
   for (unsigned i=0;i<catch_addrs_.size();i++)
      excpBlocks.push_back(new ExceptionBlock(catch_addrs_[i]));

   return true;
}

SYMTAB_EXPORT std::vector<Region *> AObject::getAllRegions() const
{
   return regions_;	
}

SYMTAB_EXPORT Offset AObject::loader_off() const 
{ 
   return loader_off_; 
}

SYMTAB_EXPORT unsigned AObject::loader_len() const 
{ 
   return loader_len_; 
}


SYMTAB_EXPORT int AObject::getAddressWidth() const 
{ 
   return addressWidth_nbytes; 
}

SYMTAB_EXPORT bool AObject::have_deferred_parsing(void) const
{ 
   return deferredParse;
}

SYMTAB_EXPORT void * AObject::getErrFunc() const 
{
   return (void *) err_func_; 
}

SYMTAB_EXPORT dyn_c_hash_map< string, std::vector< Symbol *> > *AObject::getAllSymbols()
{
   return &(symbols_);
}

SYMTAB_EXPORT AObject::~AObject() 
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
SYMTAB_EXPORT AObject::AObject(MappedFile *mf_, void (*err_func)(const char *), Symtab* st)
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
  associated_symtab(st)
{
}

//  a helper routine that selects a language based on information from the symtab
supportedLanguages AObject::pickLanguage(string &working_module, char *working_options,
      supportedLanguages working_lang)
{
   supportedLanguages lang = lang_Unknown;

   // (2) -- check suffixes -- try to keep most common suffixes near the top of the checklist
   string::size_type len = working_module.length();
   if((len>2) && (working_module.substr(len-2,2) == string(".c"))) lang = lang_C;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".C"))) lang = lang_CPlusPlus;
   else if ((len>4) && (working_module.substr(len-4,4) == string(".cpp"))) lang = lang_CPlusPlus;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".F"))) lang = lang_Fortran;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".f"))) lang = lang_Fortran;
   else if ((len>3) && (working_module.substr(len-3,3) == string(".cc"))) lang = lang_C;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".a"))) lang = lang_Assembly; // is this right?
   else if ((len>2) && (working_module.substr(len-2,2) == string(".S"))) lang = lang_Assembly;
   else if ((len>2) && (working_module.substr(len-2,2) == string(".s"))) lang = lang_Assembly;
   else
   {
      //(3) -- try to use options string -- if we have 'em
      if (working_options)
      {
         //  NOTE:  a binary is labeled "gcc2_compiled" even if compiled w/g77 -- thus this is
         //  quite inaccurate to make such assumptions
         if (strstr(working_options, "gcc"))
            lang = lang_C;
         else if (strstr(working_options, "g++"))
            lang = lang_CPlusPlus;
      }
   }
   //  This next section tries to determine the version of the debug info generator for a
   //  Sun fortran compiler.  Some leave the underscores on names in the debug info, and some
   //  have the "pretty" names, we need to detect this in order to properly read the debug.
   if (working_lang == lang_Fortran)
   {
      if (working_options)
      {
         char *dbg_gen = NULL;
         //cerr << FILE__ << __LINE__ << ":  OPT: " << working_options << endl;			
         if (NULL != (dbg_gen = strstr(working_options, "DBG_GEN=")))
         {
            //cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen << endl;
            // Sun fortran compiler (probably), need to examine version
            char *dbg_gen_ver_maj = dbg_gen + strlen("DBG_GEN=");
            //cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen_ver_maj << endl;
            char *next_dot = strchr(dbg_gen_ver_maj, '.');
            if (NULL != next_dot)
            {
               *next_dot = '\0';  //terminate major version number string
            }
         }
      }
   }
   return lang;
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

const std::string AObject::findModuleForSym(Symbol *sym) {
    dyn_c_hash_map<Symbol*, std::string>::const_accessor ca;
    if (!symsToModules_.find(ca, sym))  {
        assert(!"symsToModules_.find(ca, sym)");
    }
    return ca->second;
}

void AObject::clearSymsToMods() {
    symsToModules_.clear();
}

bool AObject::hasError() const
{
  return has_error;
}

void AObject::setTruncateLinePaths(bool)
{
}

bool AObject::getTruncateLinePaths()
{
   return false;
}

void AObject::setModuleForOffset(Offset sym_off, std::string module) {
    dyn_c_hash_map<Offset, std::vector<Symbol*>>::const_accessor found_syms;
    if(!symsByOffset_.find(found_syms, sym_off)) return;

    for(auto s = found_syms->second.begin();
            s != found_syms->second.end();
            ++s)
    {
        if (!symsToModules_.insert({*s, module}))  {
            assert(!"symsToModules_.insert({*s, module})");
        }
    }
}

