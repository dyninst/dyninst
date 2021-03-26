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

#include "common/src/vgannotations.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "common/src/Timer.h"
#include "common/src/debugOstream.h"
#include "common/src/pathName.h"

#include "Symtab.h"
#include "Module.h"
#include "Collections.h"
#include "Function.h"
#include "Variable.h"

#include "annotations.h"

#include "debug.h"

#include "symtabAPI/src/Object.h"


#if !defined(os_windows)
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <iomanip>
#include <stdarg.h>

#include "dyninstversion.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static std::string errMsg;

static const int Symtab_major_version = DYNINST_MAJOR_VERSION;
static const int Symtab_minor_version = DYNINST_MINOR_VERSION;
static const int Symtab_maintenance_version = DYNINST_PATCH_VERSION;


void Symtab::version(int& major, int& minor, int& maintenance)
{
    major = Symtab_major_version;
    minor = Symtab_minor_version;
    maintenance = Symtab_maintenance_version;
}


void symtab_log_perror(const char *msg)
{
   errMsg = std::string(msg);
};


static thread_local SymtabError serr = SymtabError::No_Error;

std::vector<Symtab *> Symtab::allSymtabs;

SymtabError Symtab::getLastSymtabError()
{
  SymtabError last = serr;
  serr = No_Error;
  return last;
}

void Symtab::setSymtabError(SymtabError new_err)
{
   serr = new_err;
}

std::string Symtab::printError(SymtabError serr)
{
    switch (serr)
    {
       case Obj_Parsing:
           return "Failed to parse the Object"+errMsg;
       case Syms_To_Functions:
           return "Failed to convert Symbols to Functions";
       case No_Such_Function:
           return "Function does not exist";
       case No_Such_Variable:
           return "Variable does not exist";
       case No_Such_Module:
          return "Module does not exist";
       case No_Such_Region:
           return "Region does not exist";
       case No_Such_Symbol:
           return "Symbol does not exist";
       case Not_A_File:
           return "Not a File. Call openArchive()";
       case Not_An_Archive:
           return "Not an Archive. Call openFile()";
       case Export_Error:
           return "Error Constructing XML"+errMsg;
       case Emit_Error:
           return "Error rewriting binary: " + errMsg;
       case Invalid_Flags:
          return "Flags passed are invalid.";
       case No_Error:
          return "No previous Error.";
       default:
          return "Unknown Error";
    }		
}

static LazySingleton<boost::shared_ptr<Type>> ls_type_Error;
boost::shared_ptr<Type>& Symtab::type_Error()
{
    return ls_type_Error.get([](){
        return Type::make_shared<Type>(std::string("<error"),0,dataUnknownType);
    });
}
static LazySingleton<boost::shared_ptr<Type>> ls_type_Untyped;
boost::shared_ptr<Type>& Symtab::type_Untyped()
{
    return ls_type_Untyped.get([](){
        return Type::make_shared<Type>(std::string("<no type>"), 0, dataUnknownType);
    });
}

static LazySingleton<boost::shared_ptr<builtInTypeCollection>> ls_builtInTypes;
boost::shared_ptr<builtInTypeCollection>& Symtab::builtInTypes()
{
    return ls_builtInTypes.get(setupBuiltinTypes);
}

static LazySingleton<boost::shared_ptr<typeCollection>> ls_stdTypes;
boost::shared_ptr<typeCollection>& Symtab::stdTypes()
{
    return ls_stdTypes.get(setupStdTypes);
}

boost::shared_ptr<builtInTypeCollection> Symtab::setupBuiltinTypes()
{
    boost::shared_ptr<builtInTypeCollection> builtInTypes =
       boost::shared_ptr<builtInTypeCollection>(new builtInTypeCollection);

   // NOTE: integral type  mean twos-complement
   // -1  int, 32 bit signed integral type
   // in stab document, size specified in bits, system size is in bytes
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-1, 4, "int", true));
   // -2  char, 8 bit type holding a character. GDB treats as signed
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-2, 1, "char", true));
   // -3  short, 16 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-3, 2, "short", true));
   // -4  long, 32/64 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-4, sizeof(long), "long", true));
   // -5  unsigned char, 8 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-5, 1, "unsigned char"));
   // -6  signed char, 8 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-6, 1, "signed char", true));
   // -7  unsigned short, 16 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-7, 2, "unsigned short"));
   // -8  unsigned int, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-8, 4, "unsigned int"));
   // -9  unsigned, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-9, 4, "unsigned"));
   // -10 unsigned long, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-10, sizeof(unsigned long), "unsigned long"));
   // -11 void, type indicating the lack of a value
   //  XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-11, 0, "void", false));
   // -12 float, IEEE single precision
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-12, sizeof(float), "float", true));
   // -13 double, IEEE double precision
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-13, sizeof(double), "double", true));
   // -14 long double, IEEE double precision, size may increase in future
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-14, sizeof(long double), "long double", true));
   // -15 integer, 32 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-15, 4, "integer", true));
   // -16 boolean, 32 bit type. GDB/GCC 0=False, 1=True, all other values
   //  have unspecified meaning
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-16, sizeof(bool), "boolean"));
   // -17 short real, IEEE single precision
   //  XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-17, sizeof(float), "short real", true));
   // -18 real, IEEE double precision XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-18, sizeof(double), "real", true));
   // -19 stringptr XXX- size of void * -- jdd 4/22/99
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-19, sizeof(void *), "stringptr"));
   // -20 character, 8 bit unsigned character type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-20, 1, "character"));
   // -21 logical*1, 8 bit type (Fortran, used for boolean or unsigned int)
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-21, 1, "logical*1"));
   // -22 logical*2, 16 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-22, 2, "logical*2"));
   // -23 logical*4, 32 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-23, 4, "logical*4"));
   // -24 logical, 32 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-24, 4, "logical"));
   // -25 complex, consists of 2 IEEE single-precision floating point values
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-25, sizeof(float)*2, "complex", true));
   // -26 complex, consists of 2 IEEE double-precision floating point values
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-26, sizeof(double)*2, "complex*16", true));
   // -27 integer*1, 8 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-27, 1, "integer*1", true));
   // -28 integer*2, 16 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-28, 2, "integer*2", true));

   /* Quick hack to make integer*4 compatible with int for Fortran
      jnb 6/20/01 */
   // This seems questionable - let's try removing that hack - jmo 05/21/04
   /*
     builtInTypes->addBuiltInType(newType = new type("int",-29,
     built_inType, 4));
     newType->decrRefCount();
   */
   // -29 integer*4, 32 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-29, 4, "integer*4", true));
   // -30 wchar, Wide character, 16 bits wide, unsigned (unknown format)
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-30, 2, "wchar"));
#if defined(os_windows)
   // -31 long long, 64 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-31, sizeof(LONGLONG), "long long", true));
   // -32 unsigned long long, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-32, sizeof(ULONGLONG), "unsigned long long"));
#else
   // -31 long long, 64 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-31, sizeof(long long), "long long", true));
   // -32 unsigned long long, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-32, sizeof(unsigned long long), "unsigned long long"));
#endif
   // -33 logical*8, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-33, 8, "logical*8"));
   // -34 integer*8, 64 bit signed integral type
   builtInTypes->addBuiltInType(Type::make_shared<typeScalar>(-34, 8, "integer*8", true));

   return builtInTypes;
}


boost::shared_ptr<typeCollection> Symtab::setupStdTypes() 
{
    boost::shared_ptr<typeCollection> stdTypes =
       boost::shared_ptr<typeCollection>(new typeCollection);

   stdTypes->addType(Type::make_shared<typeScalar>(-1, sizeof(int), "int"));
   auto charType = Type::make_shared<typeScalar>(-2, sizeof(char), "char");
   stdTypes->addType(charType);
   stdTypes->addType(Type::make_shared<typePointer>(-3, charType, "char *"));
   auto voidType = Type::make_shared<typeScalar>(-11, 0, "void", false);
   stdTypes->addType(voidType);
   stdTypes->addType(Type::make_shared<typePointer>(-4, voidType, "void *"));
   stdTypes->addType(Type::make_shared<typeScalar>(-12, sizeof(float), "float"));
#if defined(i386_unknown_nt4_0)
   stdTypes->addType(Type::make_shared<typeScalar>(-31, sizeof(LONGLONG), "long long"));    
#else
   stdTypes->addType(Type::make_shared<typeScalar>(-31, sizeof(long long), "long long"));
#endif

   return stdTypes;
}

SYMTAB_EXPORT unsigned Symtab::getAddressWidth() const 
{
   return address_width_;
}
 
SYMTAB_EXPORT bool Symtab::getABIVersion(int &major, int &minor) const
{
   return obj_private->getABIVersion(major, minor);
}

SYMTAB_EXPORT bool Symtab::isBigEndianDataEncoding() const
{
   return obj_private->isBigEndianDataEncoding();
}

SYMTAB_EXPORT Symtab::Symtab(MappedFile *mf_) :
   AnnotatableSparse(),
   member_offset_(0),
   parentArchive_(NULL),
   mf(mf_), mfForDebugInfo(NULL),
   imageOffset_(0), imageLen_(0),
   dataOffset_(0), dataLen_(0),
   is_a_out(false),
   main_call_addr_(0),
   address_width_(sizeof(int)),
   code_ptr_(NULL), data_ptr_(NULL),
   entry_address_(0), base_address_(0), load_address_(0),
   object_type_(obj_Unknown), is_eel_(false),
   no_of_sections(0),
   newSectionInsertPoint(0),
   no_of_symbols(0),
   sorted_everyFunction(false),
   isTypeInfoValid_(false),
   nlines_(0), fdptr_(0), lines_(NULL),
   stabstr_(NULL), nstabs_(0), stabs_(NULL),
   stringpool_(NULL),
   hasRel_(false), hasRela_(false), hasReldyn_(false),
   hasReladyn_(false), hasRelplt_(false), hasRelaplt_(false),
   isStaticBinary_(false), isDefensiveBinary_(false),
   func_lookup(NULL),
   mod_lookup_(NULL),
   obj_private(NULL),
   _ref_cnt(1)
{
    init_debug_symtabAPI();
}

SYMTAB_EXPORT Symtab::Symtab() :
   LookupInterface(),
   AnnotatableSparse(),
   member_offset_(0),
   parentArchive_(NULL),
   mf(NULL), mfForDebugInfo(NULL),
   imageOffset_(0), imageLen_(0),
   dataOffset_(0), dataLen_(0),
   is_a_out(false),
   main_call_addr_(0),
   address_width_(sizeof(int)),
   code_ptr_(NULL), data_ptr_(NULL),
   entry_address_(0), base_address_(0), load_address_(0),
   object_type_(obj_Unknown), is_eel_(false),
   no_of_sections(0),
   newSectionInsertPoint(0),
   no_of_symbols(0),
   sorted_everyFunction(false),
   isTypeInfoValid_(false),
   nlines_(0), fdptr_(0), lines_(NULL),
   stabstr_(NULL), nstabs_(0), stabs_(NULL),
   stringpool_(NULL),
   hasRel_(false), hasRela_(false), hasReldyn_(false),
   hasReladyn_(false), hasRelplt_(false), hasRelaplt_(false),
   isStaticBinary_(false), isDefensiveBinary_(false),
   func_lookup(NULL),
   mod_lookup_(NULL),
   obj_private(NULL),
   _ref_cnt(1)
{  
    init_debug_symtabAPI();
    create_printf("%s[%d]: Created symtab via default constructor\n", FILE__, __LINE__);
}

SYMTAB_EXPORT bool Symtab::isExec() const 
{
    return is_a_out; 
}

SYMTAB_EXPORT bool Symtab::isStripped() 
{
#if defined(os_linux) || defined(os_freebsd)
    Region *sec;
    return !findRegion(sec,".symtab");
#else
    return (no_of_symbols==0);
#endif
}

SYMTAB_EXPORT Offset Symtab::preferedBase() const 
{
    return preferedBase_;
}

SYMTAB_EXPORT Offset Symtab::imageOffset() const 
{
    return imageOffset_;
}

SYMTAB_EXPORT Offset Symtab::dataOffset() const 
{ 
    return dataOffset_;
}

SYMTAB_EXPORT Offset Symtab::dataLength() const 
{
    return dataLen_;
} 

SYMTAB_EXPORT Offset Symtab::imageLength() const 
{
    return imageLen_;
}

SYMTAB_EXPORT void Symtab::fixup_code_and_data(Offset newImageOffset,
                                               Offset newImageLength,
                                               Offset newDataOffset,
                                               Offset newDataLength)
{
    imageOffset_ = newImageOffset;
    imageLen_ = newImageLength;
    dataOffset_ = newDataOffset;
    dataLen_ = newDataLength;

    // Should we update the underlying Object?
}

/*
SYMTAB_EXPORT char* Symtab::image_ptr ()  const 
{
   return code_ptr_;
}

SYMTAB_EXPORT char* Symtab::data_ptr ()  const 
{ 
   return data_ptr_;
}
*/
SYMTAB_EXPORT const char*  Symtab::getInterpreterName() const 
{
   if (interpreter_name_.length())
      return interpreter_name_.c_str();
   return NULL;
}
 
SYMTAB_EXPORT Offset Symtab::getEntryOffset() const 
{ 
   return entry_address_;
}

SYMTAB_EXPORT Offset Symtab::getBaseOffset() const 
{
   return base_address_;
}

SYMTAB_EXPORT Offset Symtab::getLoadOffset() const 
{ 
   return load_address_;
}

SYMTAB_EXPORT Offset Symtab::getTOCoffset(Function *func) const 
{
  return getTOCoffset(func ? func->getOffset() : 0); 
}

SYMTAB_EXPORT Offset Symtab::getTOCoffset(Offset off) const
{
  return obj_private->getTOCoffset(off);
}

void Symtab::setTOCOffset(Offset off) {
  obj_private->setTOCoffset(off);
  return;
}

SYMTAB_EXPORT string Symtab::getDefaultNamespacePrefix() const
{
    return defaultNamespacePrefix;
}

// Operations on the indexed_symbols compound table.
bool Symtab::indexed_symbols::insert(Symbol* s) {
    Offset o = s->getOffset();
    master_t::accessor a;
    if(master.insert(a, std::make_pair(s, o))) {
        {
            by_offset_t::accessor oa;
            by_offset.insert(oa, o);
            oa->second.push_back(s);
        }
        {
            by_name_t::accessor ma;
            by_mangled.insert(ma, s->getMangledName());
            ma->second.push_back(s);
        }
        {
            by_name_t::accessor pa;
            by_pretty.insert(pa, s->getPrettyName());
            pa->second.push_back(s);
        }
        {
            by_name_t::accessor ta;
            by_typed.insert(ta, s->getTypedName());
            ta->second.push_back(s);
        }

        return true;
    }
    return false;
}

void Symtab::indexed_symbols::clear() {
    master.clear();
    by_offset.clear();
    by_mangled.clear();
    by_pretty.clear();
    by_typed.clear();
}

void Symtab::indexed_symbols::erase(Symbol* s) {
    if(master.erase(s)) {
        {
            by_offset_t::accessor oa;
            if (!by_offset.find(oa, s->getOffset()))  {
                assert(!"by_offset.find(oa, s->getOffset())");
            }
            std::remove(oa->second.begin(), oa->second.end(), s);
        }
        {
            by_name_t::accessor ma;
            if (!by_mangled.find(ma, s->getMangledName()))  {
                assert(!"by_mangled.find(ma, s->getMangledName())");
            }
            std::remove(ma->second.begin(), ma->second.end(), s);
        }
        {
            by_name_t::accessor pa;
            if (!by_pretty.find(pa, s->getPrettyName()))  {
                assert(!"by_pretty.find(pa, s->getPrettyName())");
            }
            std::remove(pa->second.begin(), pa->second.end(), s);
        }
        {
            by_name_t::accessor ta;
            if (!by_typed.find(ta, s->getTypedName()))  {
                assert(!"by_typed.find(ta, s->getTypedName())");
            }
            std::remove(ta->second.begin(), ta->second.end(), s);
        }
    }
}


/*
 * extractSymbolsFromFile
 *
 * Create a Symtab-level list of symbols by pulling out data
 * from the low-level parse (linkedFile).
 * Technically this causes a duplication of symbols; however,
 * we will be rewriting these symbols and so we need our own
 * copy. 
 *
 * TODO: delete the linkedFile once we're done?
 */

bool Symtab::extractSymbolsFromFile(Object *linkedFile, std::vector<Symbol *> &raw_syms) 
{
   for (SymbolIter symIter(*linkedFile); symIter; symIter++)  {
      Symbol *sym = symIter.currval();
      if (!sym)  {
         create_printf("%s[%d]:  range error, stopping now\n", FILE__, __LINE__);
         return true;
      }

      // If a symbol starts with "." we want to skip it. These indicate labels in the
      // code. 
      
      // removed 1/09: this should be done in Dyninst, not Symtab
      
      // Have to do this before the undef check, below. 
      fixSymRegion(sym);
      
      // check for undefined dynamic symbols. Used when rewriting relocation section.
      // relocation entries have references to these undefined dynamic symbols.
      // We also have undefined symbols for the static binary case.
      if (sym->getRegion() == NULL && !sym->isAbsolute() && !sym->isCommonStorage()) {
         undefDynSyms.insert(sym);
         continue;
      }
      
      // Check whether this symbol has a valid offset. If they do not we have a
      // consistency issue. This should be a null check.
      
      // Symbols can have an offset of 0 if they don't refer to things within a file.
      
      raw_syms.push_back(sym);
   }
   
   return true;
}

bool Symtab::fixSymRegion(Symbol *sym) {
   if (!sym->getRegion()) return true;
   
   if (sym->getType() != Symbol::ST_FUNCTION &&
       sym->getType() != Symbol::ST_OBJECT) return true;
   
   if (sym->getRegion()->getMemOffset() <= sym->getOffset() &&
       (sym->getRegion()->getMemOffset() + sym->getRegion()->getMemSize()) > sym->getOffset())
      return true;
   
   sym->setRegion(findEnclosingRegion(sym->getOffset()));
   
   return true;
}

/*
 * fixSymModules
 * 
 * Add Module information to all symbols. 
 */

bool Symtab::fixSymModules(std::vector<Symbol *> &raw_syms) 
{
    Object *obj = getObject();
    if (!obj) {
       return false;
    }
    for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
    {
        (*i)->finalizeRanges();
    }

//    const std::vector<std::pair<std::string, Offset> > &mods = obj->modules_;
//    for (unsigned i=0; i< mods.size(); i++) {
//       getOrCreateModule(mods[i].first, mods[i].second);
//    }
    for (unsigned i = 0; i < raw_syms.size(); i++) {
        fixSymModule(raw_syms[i]);
    }

    return true;
}


/*
 * createIndices
 *
 * We index symbols by various attributes for quick lookup. Build those
 * indices here. 
 */

bool Symtab::createIndices(std::vector<Symbol *> &raw_syms, bool undefined) {
    #pragma omp parallel for schedule(dynamic)
    for (unsigned i = 0; i < raw_syms.size(); i++) {
       addSymbolToIndices(raw_syms[i], undefined);
    }
    return true;
}

/*
 * createAggregates
 *
 * Frequently there will be multiple Symbols that refer to a single 
 * code object (e.g., function or variable). We use separate objects
 * to refer to these aggregates, and build those objects here. 
 */

bool Symtab::createAggregates() 
{
  std::vector<Symbol*> syms(everyDefinedSymbol.begin(), everyDefinedSymbol.end());

  #pragma omp parallel for
  for(size_t i = 0; i < syms.size(); ++i)
  {
    if (!doNotAggregate(syms[i])) {
      addSymbolToAggregates(syms[i]);
    }
  }
    return true;
}
 
bool Symtab::fixSymModule(Symbol *&sym) 
{
    Module* mod = NULL;
    findModuleByOffset(mod, sym->getOffset());
    if(!mod) mod = getDefaultModule();
    sym->setModule(mod);
    return true;
}


bool Symtab::addSymbolToIndices(Symbol *&sym, bool undefined) 
{
   assert(sym);
   if (!undefined) {
       everyDefinedSymbol.insert(sym);
   }
   else {
       // multi-index container should handle duplication
       undefDynSyms.insert(sym);
   }
   
    return true;
}

bool Symtab::addSymbolToAggregates(const Symbol *sym_tmp) 
{
  Symbol* sym = const_cast<Symbol*>(sym_tmp);
  
    switch(sym->getType()) {
    case Symbol::ST_FUNCTION: 
    case Symbol::ST_INDIRECT:
      {
        // We want to do the following:
        // If no function exists, create and add. 
        // Combine this information
        //   Add this symbol's names to the function.
        //   Keep module information 

        Function *func = NULL;
        bool found = false;
        {
            dyn_c_hash_map<Offset,Function*>::accessor a;
            found = !funcsByOffset.insert(a, sym->getOffset());
            if(found) func = a->second;
            else {
            // Create a new function
            // Also, update the symbol to point to this function.
            func = new Function(sym);
                a->second = func;
        }
        }  // Release the lock on the offset/function pair
        if(found) {
            /* XXX 
             * For relocatable files, the offset of a symbol is relative to the
             * beginning of a Region. Therefore, a symbol in a relocatable file
             * is not uniquely identifiable by its offset, but it is uniquely
             * identifiable by its Region and its offset.
             *
             * For now, do not add these functions to funcsByOffset collection.
             */

            if( func->getRegion() != sym->getRegion() ) {
                func = new Function(sym);
                boost::unique_lock<dyn_rwlock> l(symbols_rwlock);
                everyFunction.push_back(func);
                sorted_everyFunction = false;
            }
            func->addSymbol(sym);
        } else {
            boost::unique_lock<dyn_rwlock> l(symbols_rwlock);
            everyFunction.push_back(func);
            sorted_everyFunction = false;
        }
        sym->setFunction(func);

        break;
    }
    case Symbol::ST_TLS:
    case Symbol::ST_OBJECT: {
        // The same as the above, but with variables.
        Variable *var = NULL;
        bool found = false;
        {
            VarsByOffsetMap::accessor a;
            found = !varsByOffset.insert(a, sym->getOffset());
            VarsByOffsetMap::mapped_type &vars = a->second;
            if (found)  {
                found = false;
                for (auto v: vars)  {
                    if (v->getSize() == sym->getSize())  {
                        found = true;
                        var = v;
                    }
                }
            }
            if (!found)  {
                // Create a new variable
                // Also, update the symbol to point to this variable.
                var = new Variable(sym);
                vars.push_back(var);
            }
        }
        if(found) {
            /* XXX
             * For relocatable files, the offset is not a unique identifier for
             * a Symbol. With functions, the Region and offset could be used to
             * identify the symbol. With variables, the Region and offset may 
             * not uniquely identify the symbol. The only case were this occurs
             * is with COMMON symbols -- their offset is their memory alignment
             * and their Region is undefined. In this case, always create a 
             * new variable.
             */
            if( obj_RelocatableFile == getObjectType() &&
                ( var->getRegion() != sym->getRegion() ||
                  NULL == sym->getRegion() ) )
            {
                var = new Variable(sym);
                boost::unique_lock<dyn_rwlock> l(symbols_rwlock);
                everyVariable.push_back(var);
            }else{
                var->addSymbol(sym);
            }
        } else {
            boost::unique_lock<dyn_rwlock> l(symbols_rwlock);
            everyVariable.push_back(var);
        }
        sym->setVariable(var);
        break;
    }
    default: {
        break;
    }
    }
    return true;
}

// A hacky override for specially treating symbols that appear
// to be functions or variables but aren't.
//
// Example: IA-32/AMD-64 libc (and others compiled with libc headers)
// uses outlined locking primitives. These are named _L_lock_<num>
// and _L_unlock_<num> and labelled as functions. We explicitly do
// not include them in function scope.
//
// Also, exclude symbols that begin with _imp_ in defensive mode.
// These symbols are entries in the IAT and shouldn't be treated
// as functions.
bool Symtab::doNotAggregate(const Symbol* sym) {
    const std::string& mangled = sym->getMangledName();

    if (isDefensiveBinary() && mangled.compare(0, 5, "_imp_", 5) == 0) {
        return true;
    }

  if (mangled.compare(0, strlen("_L_lock_"), "_L_lock_") == 0) {
    return true;
  }
  if (mangled.compare(0, strlen("_L_unlock_"), "_L_unlock_") == 0) {
    return true;
  }

#if 0
  // Disabling as a test; this means we find _zero_ Function objects. 
  // PPC64 Linux symbols in the .opd section appear to be functions,
  // but are not.
  if (sym->getRegion() && sym->getRegion()->getRegionName() == ".opd") {
      return true;
  }
#endif
  // return !isDefined(sym);
    return false;
}

/* Add the new name to the appropriate symbol index */

bool Symtab::updateIndices(Symbol * /*sym*/, std::string /*newName*/, NameType /*nameType*/) {

#if 0
     if (nameType & mangledName) {
        // Add this symbol under the given name (as mangled)
        symsByMangledName[newName].push_back(sym);
    }
    if (nameType & prettyName) {
        // Add this symbol under the given name (as pretty)
        symsByPrettyName[newName].push_back(sym);
    }
    if (nameType & typedName) {
        // Add this symbol under the given name (as typed)
        symsByTypedName[newName].push_back(sym);
    }
#endif
    return true;
}

#if 0
/* checkPPC64DescriptorSymbols() is no longer needed.  3-word descriptor
 * symbols are properly taken care of during symbol parsing.  See
 * parse_symbols() in Object-elf.C for details.
 */

#if defined(ppc64_linux)
/* Special case for ppc64 ELF binaries. Sometimes a function has a 3-byte descriptor symbol
 * along with it in the symbol table and "." preceding its original pretty name for the correct
 * function symbol. This checks to see if we have a corresponding 3-byte descriptor symbol existing
 * and if it does we remove the preceding "." from the name of the symbol
 */

void Symtab::checkPPC64DescriptorSymbols(Object *linkedFile)
{
   // find the real functions -- those with the correct type in the symbol table
   for(SymbolIter symIter(*linkedFile); symIter;symIter++)
   {
      Symbol *lookUp = symIter.currval();
      const char *np = lookUp->getMangledName().c_str();
      if(!np)
         continue;

      if(np[0] == '.' && (lookUp->getType() == Symbol::ST_FUNCTION))
      {
         std::vector<Symbol *>syms;
         std::string newName = np+1;
         if(linkedFile->get_symbols(newName, syms) && (syms[0]->getSize() == 24 || syms[0]->getSize() == 0))
         {
            //Remove the "." from the name
            lookUp->mangledNames[0] = newName;

            //Change the type of the descriptor symbol
            syms[0]->type_ = Symbol::ST_NOTYPE;
         }
      }
   }

}

#endif
#endif

//  setModuleLanguages is only called after modules have been defined.
//  it attempts to set each module's language, information which is needed
//  before names can be demangled.
void Symtab::setModuleLanguages(dyn_hash_map<std::string, supportedLanguages> *mod_langs)
{
   if (!mod_langs->size())
      return;  // cannot do anything here
   //  this case will arise on non-stabs platforms until language parsing can be introduced at this level
   Module *currmod = NULL;
   //int dump = 0;

    for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
      currmod = (*i);
      supportedLanguages currLang;
      if (currmod->isShared()) {
         continue;  // need to find some way to get shared object languages?
      }

      const std::string fn = currmod->fileName();
      if (mod_langs->find(currmod->fileName()) != mod_langs->end())
      {
         currLang = (*mod_langs)[fn];
      }
      else if (fn.rfind(".s") != std::string::npos ||
            fn.rfind(".asm") != std::string::npos)
      {
         currLang = lang_Assembly;
      }
      else if (fn.rfind(".c") != std::string::npos)
      {
         currLang = lang_C;
      }
      else if (fn.rfind(".cpp") != std::string::npos ||
            fn.rfind(".cc") != std::string::npos ||
            fn.rfind(".C") != std::string::npos)
      {
         currLang = lang_CPlusPlus;
      }
      else
      {
         continue;
      }
      currmod->setLanguage(currLang);
   }
}

void Symtab::createDefaultModule() {
    assert(indexed_modules.empty());
    Module *mod = new Module(lang_Unknown,
                     imageOffset_,
                     name(),
                     this);
    mod->addRange(imageOffset_, imageLen_ + imageOffset_);
    indexed_modules.push_back(mod);
    mod->finalizeRanges();
}



Module *Symtab::getOrCreateModule(const std::string &modName, 
                                  const Offset modAddr)
{
    if(indexed_modules.empty()) {
        createDefaultModule();
    }
   std::string nameToUse;
   if (modName.length() > 0)
      nameToUse = modName;
   else
      nameToUse = "DEFAULT_MODULE";

   Module *fm = NULL;
   if (findModuleByName(fm, nameToUse)) 
   {
       if(modAddr && (modAddr < fm->addr()))
       {
           fm->addr_ = modAddr;
       }
      return fm;
   }

    const char *str = nameToUse.c_str();
    int len = nameToUse.length();
    assert(len>0);

    // TODO ignore directory definitions for now
    if (str[len-1] == '/') 
        return NULL;

    return (newModule(nameToUse, modAddr, lang_Unknown));
}
 
Module *Symtab::newModule(const std::string &name, const Offset addr, supportedLanguages lang)
{
    Module *ret = NULL;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.

    if (findModuleByName(ret, name)) 
    {
        return(ret);
    }

    //parsing_printf("=== image, creating new pdmodule %s, addr 0x%x\n",
    //				name.c_str(), addr);
    
    std::string fileNm, fullNm;
    fullNm = name;
    fileNm = extract_pathname_tail(name);

    create_printf("%s[%d]: In %p: Creating new module '%s'/'%s'\n", FILE__, __LINE__, this, fileNm.c_str(), fullNm.c_str());

    ret = new Module(lang, addr, fullNm, this);
    assert(ret);

    /*
     * FIXME
     *
     * There are cases where the fileName can be the same, but the full name is
     * different and the modules are actually different. This is an inherent
     * problem with how modules are processed.
     */
    if (indexed_modules.get<2>().end() != indexed_modules.get<2>().find(ret->fileName()))
    {
       create_printf("%s[%d]:  WARN:  LEAK?  already have module with name %s\n", 
             FILE__, __LINE__, ret->fileName().c_str());
    }

    if (indexed_modules.get<3>().end() != indexed_modules.get<3>().find(ret->fullName()))
    {
       create_printf("%s[%d]:  WARN:  LEAK?  already have module with name %s\n", 
                     FILE__, __LINE__, ret->fullName().c_str());
    }

    indexed_modules.push_back(ret);
    
    return (ret);
}

Symtab::Symtab(std::string filename, bool defensive_bin, bool &err) :
   LookupInterface(),
   AnnotatableSparse(),
   member_offset_(0),
   parentArchive_(NULL),
   mf(NULL), mfForDebugInfo(NULL),
   imageOffset_(0), imageLen_(0),
   dataOffset_(0), dataLen_(0),
   is_a_out(false),
   main_call_addr_(0),
   address_width_(sizeof(int)),
   code_ptr_(NULL), data_ptr_(NULL),
   entry_address_(0), base_address_(0), load_address_(0),
   object_type_(obj_Unknown), is_eel_(false),
   no_of_sections(0),
   newSectionInsertPoint(0),
   no_of_symbols(0),
   sorted_everyFunction(false),
   isTypeInfoValid_(false),
   nlines_(0), fdptr_(0), lines_(NULL),
   stabstr_(NULL), nstabs_(0), stabs_(NULL),
   stringpool_(NULL),
   hasRel_(false), hasRela_(false), hasReldyn_(false),
   hasReladyn_(false), hasRelplt_(false), hasRelaplt_(false),
   isStaticBinary_(false), isDefensiveBinary_(defensive_bin),
   func_lookup(NULL),
   mod_lookup_(NULL),
   obj_private(NULL),
   _ref_cnt(1)
{
   init_debug_symtabAPI();
   // Initialize error parameter
   err = false;
   
   create_printf("%s[%d]: created symtab for %s\n", FILE__, __LINE__, filename.c_str());

#if defined (os_windows)
   extern void fixup_filename(std::string &);
   fixup_filename(filename);
#endif

   //  createMappedFile handles reference counting
   mf = MappedFile::createMappedFile(filename);
   if (!mf) {
      create_printf("%s[%d]: WARNING: creating symtab for %s, " 
                    "createMappedFile() failed\n", FILE__, __LINE__, 
                    filename.c_str());
      err = true;
      return;
   }

   obj_private = new Object(mf, defensive_bin, 
                            symtab_log_perror, true, this);
   if (obj_private->hasError()) {
      create_printf("%s[%d]: WARNING: creating symtab for %s, " 
                    "Object ctor failed\n", FILE__, __LINE__, 
                    filename.c_str());
     err = true;
     return;
   }
   if (!extractInfo(obj_private))
   {
      create_printf("%s[%d]: WARNING: creating symtab for %s, extractInfo() " 
                    "failed\n", FILE__, __LINE__, filename.c_str());
      err = true;
   }

   member_name_ = mf->filename();

   defaultNamespacePrefix = "";
}

Symtab::Symtab(unsigned char *mem_image, size_t image_size, 
               const std::string &name, bool defensive_bin, bool &err) :
   LookupInterface(),
   AnnotatableSparse(),
   member_offset_(0),
   parentArchive_(NULL),
   mf(NULL), mfForDebugInfo(NULL),
   imageOffset_(0), imageLen_(0),
   dataOffset_(0), dataLen_(0),
   is_a_out(false),
   main_call_addr_(0),
   address_width_(sizeof(int)),
   code_ptr_(NULL), data_ptr_(NULL),
   entry_address_(0), base_address_(0), load_address_(0),
   object_type_(obj_Unknown), is_eel_(false),
   no_of_sections(0),
   newSectionInsertPoint(0),
   no_of_symbols(0),
   sorted_everyFunction(false),
   isTypeInfoValid_(false),
   nlines_(0), fdptr_(0), lines_(NULL),
   stabstr_(NULL), nstabs_(0), stabs_(NULL),
   stringpool_(NULL),
   hasRel_(false), hasRela_(false), hasReldyn_(false),
   hasReladyn_(false), hasRelplt_(false), hasRelaplt_(false),
   isStaticBinary_(false),
   isDefensiveBinary_(defensive_bin),
   func_lookup(NULL),
   mod_lookup_(NULL),
   obj_private(NULL),
   _ref_cnt(1)
{
   // Initialize error parameter
   err = false;
  
   create_printf("%s[%d]: created symtab for memory image at addr %u\n", 
                 FILE__, __LINE__, mem_image);

   //  createMappedFile handles reference counting
   mf = MappedFile::createMappedFile(mem_image, image_size, name);
   if (!mf) {
      create_printf("%s[%d]: WARNING: creating symtab for memory image at " 
                    "addr %u, createMappedFile() failed\n", FILE__, __LINE__, 
                    mem_image);
      err = true;
      return;
   }

   obj_private = new Object(mf, defensive_bin, 
                            symtab_log_perror, true, this);
   if (obj_private->hasError()) {
     err = true;
     return;
   }

   if (!extractInfo(obj_private))
   {
      create_printf("%s[%d]: WARNING: creating symtab for memory image at addr" 
                    "%u, extractInfo() failed\n", FILE__, __LINE__, mem_image);
      err = true;
   }

   member_name_ = mf->filename();

   defaultNamespacePrefix = "";
}

bool sort_reg_by_addr(const Region* a, const Region* b)
{
  if (a->getMemOffset() == b->getMemOffset())
    return a->getMemSize() < b->getMemSize();
  return a->getMemOffset() < b->getMemOffset();
}

extern void print_symbols( std::vector< Symbol *>& allsymbols );
extern void print_symbol_map( dyn_hash_map< std::string, std::vector< Symbol *> > *symbols);

static bool ExceptionBlockCmp(ExceptionBlock *a, ExceptionBlock *b) {
    return a->catchStart() < b->catchStart();
}

bool Symtab::extractInfo(Object *linkedFile)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif

    /* FIXME 
     *
     * Some ELF .o's don't have contiguous code and data Regions so these data
     * members are imprecise. These members should probably be deprecated in
     * favor of the getCodeRegions and getDataRegions functions.
     */
#if defined(os_windows)
	preferedBase_ = linkedFile->getPreferedBase();
#else
	preferedBase_ = 0;
#endif
    imageOffset_ = linkedFile->code_off();
    dataOffset_ = linkedFile->data_off();

#if defined(os_windows)
	preferedBase_ = linkedFile->getPreferedBase();
#else
	preferedBase_ = 0;
#endif

    imageLen_ = linkedFile->code_len();
    dataLen_ = linkedFile->data_len();
    
    if (0 == imageLen_ || 0 == linkedFile->code_ptr()) 
    {
       if (0 == linkedFile->code_ptr()) {
          linkedFile->code_ptr_ = (char *) linkedFile->code_off();
       }
       else 
       {
           if( object_type_ != obj_RelocatableFile ||
               linkedFile->code_ptr() == 0)
           {
               setSymtabError(Obj_Parsing);
               return false;
           }
       }
   }
	
  //  if (!imageLen_ || !linkedFile->code_ptr()) {
  //      serr = Obj_Parsing; 
  //      return false; 
   // }

    no_of_sections = linkedFile->no_of_sections();
    newSectionInsertPoint = no_of_sections;
    no_of_symbols = linkedFile->no_of_symbols();
    
    isStaticBinary_ = linkedFile->isStaticBinary();

    hasRel_ = false;
    hasRela_ = false;
    hasReldyn_ = false;
    hasReladyn_ = false;
    hasRelplt_ = false;
    hasRelaplt_ = false;
    regions_ = linkedFile->getAllRegions();

    for (unsigned index=0;index<regions_.size();index++)
      {
      regions_[index]->setSymtab(this);

        if ( regions_[index]->isLoadable() ) 
        {
           if (     (regions_[index]->getRegionPermissions() == Region::RP_RX) 
                 || (isDefensiveBinary_ && 
                     regions_[index]->getRegionPermissions() == Region::RP_RW)
                 || (regions_[index]->getRegionPermissions() == Region::RP_RWX)) 
           {
              codeRegions_.push_back(regions_[index]);
           }
           else 
           {
              dataRegions_.push_back(regions_[index]);
           }
        }

        regionsByEntryAddr[regions_[index]->getMemOffset()] = regions_[index];

        if (regions_[index]->getRegionType() == Region::RT_REL) 
        {
            hasRel_ = true;
        }

        if (regions_[index]->getRegionType() == Region::RT_RELA) 
        {
            hasRela_ = true;
        }

#if defined(os_linux) || defined(os_freebsd)
        hasReldyn_ = linkedFile->hasReldyn();
	hasReladyn_ = linkedFile->hasReladyn();
        hasRelplt_ = linkedFile->hasRelplt();
        hasRelaplt_ = linkedFile->hasRelaplt();
#endif	

    }
    // sort regions_ & codeRegions_ vectors

    std::sort(codeRegions_.begin(), codeRegions_.end(), sort_reg_by_addr);
    std::sort(dataRegions_.begin(), dataRegions_.end(), sort_reg_by_addr);
    std::sort(regions_.begin(), regions_.end(), sort_reg_by_addr);

    /* insert error check here. check if parsed */
    address_width_ = linkedFile->getAddressWidth();
    is_a_out = linkedFile->is_aout();
    code_ptr_ = linkedFile->code_ptr();
    data_ptr_ = linkedFile->data_ptr();

    if (linkedFile->interpreter_name())
       interpreter_name_ = std::string(linkedFile->interpreter_name());

    entry_address_ = linkedFile->getEntryAddress();
    base_address_ = linkedFile->getBaseAddress();
    load_address_ = linkedFile->getLoadAddress();
    object_type_  = linkedFile->objType();
    is_eel_ = linkedFile->isEEL();
    linkedFile->getSegments(segments_);

    // define all of the functions
    //statusLine("winnowing functions");

    // a vector to hold all created symbols until they are properly classified
    std::vector<Symbol *> raw_syms;

#ifdef BINEDIT_DEBUG
    printf("== from linkedFile...\n");
    print_symbol_map(linkedFile->getAllSymbols());
#endif

    if (!extractSymbolsFromFile(linkedFile, raw_syms)) 
    {
        setSymtabError(Syms_To_Functions);
        return false;
    }

    if (!fixSymModules(raw_syms)) 
    {
        setSymtabError(Syms_To_Functions);
        return false;
    }
	Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
    obj->clearSymsToMods();

    // wait until all modules are defined before applying languages to
    // them we want to do it this way so that module information comes
    // from the function symbols, first and foremost, to avoid any
    // internal module-function mismatching.
            
    // get Information on the language each modules is written in
    // (prior to making modules)

    dyn_hash_map<std::string, supportedLanguages> mod_langs;
    linkedFile->getModuleLanguageInfo(&mod_langs);
    setModuleLanguages(&mod_langs);
	
    if (!createIndices(raw_syms, false))
    {
        setSymtabError(Syms_To_Functions);
        return false;
    }


    if (!createAggregates()) 
    {
        setSymtabError(Syms_To_Functions);
        return false;
    }
	
    // Once languages are assigned, we can build demangled names (in
    // the wider sense of demangling which includes stripping _'s from
    // fortran names -- this is why language information must be
    // determined before this step).
    
    // Also identifies aliases (multiple names with equal addresses)
#if !defined(os_windows)
    linkedFile->getDependencies(deps_);
#endif

    
    //addSymtabVariables();
    linkedFile->getAllExceptions(excpBlocks);
    sort(excpBlocks.begin(), excpBlocks.end(), ExceptionBlockCmp);

    vector<relocationEntry >fbt;
    linkedFile->get_func_binding_table(fbt);
    for(unsigned i=0; i<fbt.size();i++)
        relocation_table_.push_back(fbt[i]);
    return true;
}

Symtab::Symtab(const Symtab& obj) :
   LookupInterface(),
   AnnotatableSparse(),
   member_name_(obj.member_name_),
   member_offset_(obj.member_offset_),
   parentArchive_(NULL),
   mf(NULL), mfForDebugInfo(NULL),
   imageOffset_(obj.imageOffset_), imageLen_(obj.imageLen_),
   dataOffset_(obj.dataOffset_), dataLen_(obj.dataLen_),
   is_a_out(obj.is_a_out),
   main_call_addr_(obj.main_call_addr_),
   address_width_(sizeof(int)),
   code_ptr_(NULL), data_ptr_(NULL),
   entry_address_(0), base_address_(0), load_address_(0),
   object_type_(obj_Unknown), is_eel_(false),
   defaultNamespacePrefix(obj.defaultNamespacePrefix),
   no_of_sections(0),
   newSectionInsertPoint(0),
   no_of_symbols(obj.no_of_symbols),
   sorted_everyFunction(false),
   isTypeInfoValid_(obj.isTypeInfoValid_),
   nlines_(0), fdptr_(0), lines_(NULL),
   stabstr_(NULL), nstabs_(0), stabs_(NULL),
   stringpool_(NULL),
   hasRel_(false), hasRela_(false), hasReldyn_(false),
   hasReladyn_(false), hasRelplt_(false), hasRelaplt_(false),
   isStaticBinary_(false), isDefensiveBinary_(obj.isDefensiveBinary_),
   func_lookup(NULL),
   mod_lookup_(NULL),
   obj_private(NULL),
   _ref_cnt(1)
{
    create_printf("%s[%d]: Creating symtab 0x%p from symtab 0x%p\n", FILE__, __LINE__, this, &obj);

   unsigned i;

   for (i=0;i<obj.regions_.size();i++) {
     regions_.push_back(new Region(*(obj.regions_[i])));
     regions_.back()->setSymtab(this);
   }

   for (i=0;i<regions_.size();i++)
      regionsByEntryAddr[regions_[i]->getMemOffset()] = regions_[i];

   // TODO FIXME: copying symbols/Functions/Variables
   // (and perhaps anything else initialized zero above)


   for (i=0;i<obj.indexed_modules.size();i++)
   {
      Module *m = new Module(*(obj.indexed_modules[i]));
      indexed_modules.push_back(m);
   }

   for (i=0; i<obj.relocation_table_.size();i++)
   {
      relocation_table_.push_back(relocationEntry(obj.relocation_table_[i]));
   }

   for (i=0;i<obj.excpBlocks.size();i++)
   {
      excpBlocks.push_back(new ExceptionBlock(*(obj.excpBlocks[i])));
   }

   deps_ = obj.deps_;
}

// Address must be in code or data range since some code may end up
// in the data segment
bool Symtab::isValidOffset(const Offset where) const
{
   return isCode(where) || isData(where);
}

/* Performs a binary search on the codeRegions_ vector, which must
 * be kept in sorted order
 */
bool Symtab::isCode(const Offset where)  const
{
   if (!codeRegions_.size()) 
   {
      create_printf("%s[%d] No code regions in %s \n",
                    __FILE__, __LINE__, mf->filename().c_str());
      return false;
   }

   // search for "where" in codeRegions_ (code regions must not overlap)
   int first = 0; 
   int last = codeRegions_.size() - 1;

   while (last >= first) 
   {
      Region *curreg = codeRegions_[(first + last) / 2];
      if (where >= curreg->getMemOffset()
            && where < (curreg->getMemOffset()
               + curreg->getMemSize())) 
      {
         if (curreg->getRegionType() == Region::RT_BSS)
            return false;
         return true;
      }
      else if (where < curreg->getMemOffset()) 
      {
         last = ((first + last) / 2) - 1;
      }
      else if (where >= (curreg->getMemOffset() + curreg->getMemSize()))
      {
         first = ((first + last) / 2) + 1;
      }
      else 
      {  // "where" is in the range: 
         // [memOffset + diskSize , memOffset + memSize)
         // meaning that it's in an uninitialized data region 
         return false;
      }
   }

   return false;
}

/* Performs a binary search on the dataRegions_ vector, which must
 * be kept in sorted order */
bool Symtab::isData(const Offset where)  const
{
   if (!dataRegions_.size()) 
   {
      create_printf("%s[%d] No data regions in %s \n",
                    __FILE__,__LINE__,mf->filename().c_str());
      return false;
   }

   int first = 0; 
   int last = dataRegions_.size() - 1;

   while (last >= first) 
   {
      Region *curreg = dataRegions_[(first + last) / 2];

      if (     (where >= curreg->getMemOffset())
            && (where < (curreg->getMemOffset() + curreg->getMemSize())))
      {
         return true;
      }
      else if (where < curreg->getMemOffset()) 
      {
         last = ((first + last) / 2) - 1;
      }
      else 
      {
         first = ((first + last) / 2) + 1;
      }
   }

   return false;
}

SYMTAB_EXPORT bool Symtab::getFuncBindingTable(std::vector<relocationEntry> &fbt) const
{
   fbt = relocation_table_;
   return true;
}

SYMTAB_EXPORT bool Symtab::updateFuncBindingTable(Offset stub_addr, Offset plt_addr)
{
    int stub_idx = -1, plt_idx = -1;

    for (unsigned i = 0; i < relocation_table_.size(); ++i) {
        if (stub_addr == relocation_table_[i].target_addr())
            stub_idx = i;
        if (plt_addr  == relocation_table_[i].target_addr())
            plt_idx = i;
        if (stub_idx >= 0 && plt_idx >= 0)
            break;
    }
    if (stub_idx >= 0 && plt_idx >= 0) {
        relocation_table_[stub_idx] = relocation_table_[plt_idx];
        relocation_table_[stub_idx].setTargetAddr(stub_addr);
        return true;
    }
    return false;
}

SYMTAB_EXPORT std::vector<std::string> &Symtab::getDependencies(){
    return deps_;
}

SYMTAB_EXPORT Archive *Symtab::getParentArchive() const {
    return parentArchive_;
}

Symtab::~Symtab()
{
   // Doesn't do anything yet, moved here so we don't mess with symtab.h
   // Only called if we fail to create a process.
   // Or delete the a.out...


   for (unsigned i = 0; i < regions_.size(); i++) 
   {
      delete regions_[i];
   }

   regions_.clear();
   codeRegions_.clear();
   dataRegions_.clear();
   regionsByEntryAddr.clear();

   std::vector<Region *> *user_regions = NULL;
   getAnnotation(user_regions, UserRegionsAnno);

   if (user_regions)
   {
      for (unsigned i = 0; i < user_regions->size(); ++i) 
         delete (*user_regions)[i];
      user_regions->clear();
   }

   // Symbols are copied from linkedFile, and NOT deleted
   everyDefinedSymbol.clear();
   undefDynSyms.clear();


   for (unsigned i = 0; i < everyFunction.size(); i++) 
   {
      delete everyFunction[i];
   }

   everyFunction.clear();
   funcsByOffset.clear();

   for (unsigned i = 0; i < everyVariable.size(); i++) 
   {
      delete everyVariable[i];
   }

   everyVariable.clear();
   varsByOffset.clear();

    for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
      delete (*i);
   }
   indexed_modules.clear();

   for (unsigned i=0;i<excpBlocks.size();i++)
      delete excpBlocks[i];

   create_printf("%s[%d]: Symtab::~Symtab removing %p from allSymtabs\n", 
         FILE__, __LINE__, this);

   deps_.clear();

   for (unsigned i = 0; i < allSymtabs.size(); i++) 
   {
      if (allSymtabs[i] == this)
         allSymtabs.erase(allSymtabs.begin()+i);
   }

    delete func_lookup;
    delete mod_lookup_;

   // Make sure to free the underlying Object as it doesn't have a factory
   // open method
   delete obj_private;

   if (mf) MappedFile::closeMappedFile(mf);

}	

bool Symtab::exportXML(string)
{
   return false;
}

bool Symtab::exportBin(string) 
{
   return false;
}

Symtab *Symtab::importBin(std::string)
{
   return NULL;
}

bool Symtab::openFile(Symtab *&obj, void *mem_image, size_t size, 
                      std::string name, def_t def_bin)
{
   bool err = false;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   obj = new Symtab((unsigned char *) mem_image, size, name, (def_bin == Defensive), err);

#if defined(TIMED_PARSE)
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
    unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
    unsigned long difftime = lendtime - lstarttime;
    double dursecs = difftime/(1000 );
    cout << __FILE__ << ":" << __LINE__ <<": openFile "<< filename<< " took "<<dursecs <<" msecs" << endl;
#endif
    if(!err)
    {
       allSymtabs.push_back(obj);
    }
    else
    {
        delete obj;
       obj = NULL;
    }
    // returns true on success (not an error)
    return !err;
}

bool Symtab::closeSymtab(Symtab *st)
{
	bool found = false;
	if (!st) return false;

    --(st->_ref_cnt);

	std::vector<Symtab *>::reverse_iterator iter;
	for (iter = allSymtabs.rbegin(); iter != allSymtabs.rend() ; iter++)
	{
		if (*iter == st)
		{
            found = true;
			if(0 == st->_ref_cnt) {
			    allSymtabs.erase(iter.base() -1);
				break;
			}
		}
	}
    if(0 == st->_ref_cnt)
	    delete(st);
	return found;
}

Symtab *Symtab::findOpenSymtab(std::string filename)
{
   unsigned numSymtabs = allSymtabs.size();
	for (unsigned u=0; u<numSymtabs; u++) 
	{
		assert(allSymtabs[u]);
		if (filename == allSymtabs[u]->file() && 
          allSymtabs[u]->mf->canBeShared()) 
		{
            allSymtabs[u]->_ref_cnt++;
			// return it
			return allSymtabs[u];
		}
	}   
	return NULL;
}

bool Symtab::openFile(Symtab *&obj, std::string filename, def_t def_binary)
{
   bool err = false;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   if ( filename.find("/proc") == std::string::npos)
   {
	   obj = findOpenSymtab(filename);
	   if (obj)
	   {
		   return true;
   }
   }

   obj = new Symtab(filename, (def_binary == Defensive), err);

#if defined(TIMED_PARSE)
   struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": openFile "<< filename<< " took "<<dursecs <<" msecs" << endl;
#endif

   if (!err)
   {
      if (filename.find("/proc") == std::string::npos)
         allSymtabs.push_back(obj);
   }
   else
   {
       create_printf("%s[%d]: WARNING: failed to open symtab for %s\n", 
		     FILE__, __LINE__, filename.c_str());
       delete obj;
       obj = NULL;
   }

   // returns true on success (not an error)
   return !err;
}

bool Symtab::addRegion(Offset vaddr, void *data, unsigned int dataSize, std::string name, 
        Region::RegionType rType_, bool loadable, unsigned long memAlign, bool tls)
{
   Region *sec;
   unsigned i;
   if (loadable)
   {
      sec = new Region(newSectionInsertPoint, name, vaddr, dataSize, vaddr, 
            dataSize, (char *)data, Region::RP_R, rType_, true, tls, memAlign);
      sec->setSymtab(this);

      regions_.insert(regions_.begin()+newSectionInsertPoint, sec);

      for (i = newSectionInsertPoint+1; i < regions_.size(); i++)
      {
         regions_[i]->setRegionNumber(regions_[i]->getRegionNumber() + 1);
      }

      if (    (sec->getRegionType() == Region::RT_TEXT) 
            || (sec->getRegionType() == Region::RT_TEXTDATA))
      {
         codeRegions_.push_back(sec);
         std::sort(codeRegions_.begin(), codeRegions_.end(), sort_reg_by_addr);
      }

      if (    (sec->getRegionType() == Region::RT_DATA) 
            || (sec->getRegionType() == Region::RT_TEXTDATA))
      {
         dataRegions_.push_back(sec);
         std::sort(dataRegions_.begin(), dataRegions_.end(), sort_reg_by_addr);
      }
   }
   else
   {
      sec = new Region(regions_.size()+1, name, vaddr, dataSize, 0, 0, 
            (char *)data, Region::RP_R, rType_, loadable, tls, memAlign);
      sec->setSymtab(this);
      regions_.push_back(sec);
   }

   addUserRegion(sec);
   std::sort(regions_.begin(), regions_.end(), sort_reg_by_addr);
   return true;
}

bool Symtab::addUserRegion(Region *reg)
{
   std::vector<Region *> *user_regions = NULL;

   if (!getAnnotation(user_regions, UserRegionsAnno))
   {
      user_regions = new std::vector<Region *>();
      if (!addAnnotation(user_regions, UserRegionsAnno))
      {
         create_printf("%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return false;
      }
   }

   if (!user_regions)
   {
      create_printf("%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return false;
   }

   user_regions->push_back(reg);

   return true;
}

bool Symtab::addUserType(Type *t)
{
   std::vector<Type *> *user_types = NULL;

   if (!getAnnotation(user_types, UserTypesAnno))
   {
      user_types = new std::vector<Type *>();
      if (!addAnnotation(user_types, UserTypesAnno))
      {
         create_printf("%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return false;
      }
   }
   if (!user_types)
   {
      create_printf("%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return false;
   }

   user_types->push_back(t);

   return true;
}

bool Symtab::addRegion(Region *sec)
{
  regions_.push_back(sec);
  sec->setSymtab(this);
  std::sort(regions_.begin(), regions_.end(), sort_reg_by_addr);
  addUserRegion(sec);
   return true;
}

void Symtab::parseLineInformation()
{
   Object *linkedFile = getObject();
   if (!linkedFile)
   {
     return;
   }
    linkedFile->parseFileLineInfo();
}

SYMTAB_EXPORT bool Symtab::getAddressRanges(std::vector<AddressRange > &ranges,
                                            std::string lineSource, unsigned int lineNo)
{
   unsigned int originalSize = ranges.size();
   parseLineInformation();
   
   /* Iteratate over the modules, looking for ranges in each. */
    for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
       StringTablePtr s = (*i)->getStrings();
       boost::unique_lock<dyn_mutex> l(s->lock);
       // Only check modules that have this filename present
       if(s->get<1>().find(lineSource) == s->get<1>().end()) {
           continue;
       }
       LineInformation *lineInformation = (*i)->parseLineInformation();
       if (lineInformation) {
           lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );
       }
   } /* end iteration over modules */

   if ( ranges.size() != originalSize )
      return true;

   return false;
}

SYMTAB_EXPORT bool Symtab::getSourceLines(std::vector<Statement::Ptr> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();
    std::set<Module*> mods_for_offset;
    findModuleByOffset(mods_for_offset, addressInRange);
    for(auto i = mods_for_offset.begin();
            i != mods_for_offset.end();
            ++i)
    {
        (*i)->getSourceLines(lines, addressInRange);
    }

   if ( lines.size() != originalSize )
      return true;

   return false;

}

SYMTAB_EXPORT bool Symtab::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
    std::vector<Statement::Ptr> tmp;
    getSourceLines(tmp, addressInRange);
    if(tmp.empty()) return false;
    for(auto i = tmp.begin(); i != tmp.end(); ++i)
    {
        lines.push_back(**i);
    }
    return true;
}

SYMTAB_EXPORT bool Symtab::addLine(std::string lineSource, unsigned int lineNo,
      unsigned int lineOffset, Offset lowInclAddr,
      Offset highExclAddr)
{
   Module *mod;

   if (!findModuleByName(mod, lineSource))
   {
      std::string fileNm = extract_pathname_tail(lineSource);

      if (!findModuleByName(mod, fileNm))
      {
         if (!findModuleByName(mod, mf->pathname()))
            return false;
      }    
   }

   LineInformation *lineInfo = mod->getLineInformation();

   if (!lineInfo)
      return false;

   return (lineInfo->addLine(lineSource.c_str(), lineNo, lineOffset, 
            lowInclAddr, highExclAddr));
}

SYMTAB_EXPORT bool Symtab::addAddressRange( Offset lowInclusiveAddr, Offset highExclusiveAddr,
      std::string lineSource, unsigned int lineNo,
      unsigned int lineOffset)
{
   Module *mod;

   if (!findModuleByName(mod, lineSource))
   {
      std::string fileNm = extract_pathname_tail(lineSource);

      if (!findModuleByName(mod, fileNm))
         return false;
   }

   LineInformation *lineInfo = mod->getLineInformation();

   if (!lineInfo)
      return false;

   return (lineInfo->addAddressRange(lowInclusiveAddr, highExclusiveAddr, 
            lineSource.c_str(), lineNo, lineOffset));
}

void Symtab::setTruncateLinePaths(bool value)
{
   getObject()->setTruncateLinePaths(value);
}

bool Symtab::getTruncateLinePaths()
{
   return getObject()->getTruncateLinePaths();
}

void Symtab::parseTypes()
{
   Object *linkedFile = getObject();
	if (!linkedFile)
	{
		return;
	}
    linkedFile->parseTypeInfo();

    for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
       (*i)->setModuleTypes(typeCollection::getModTypeCollection((*i)));
       (*i)->finalizeRanges();
   }

   //  optionally we might want to clear the static data struct in typeCollection
   //  here....  the parsing is over, and we have added all typeCollections as
   //  annotations proper.

   typeCollection::fileToTypesMap.clear();

}

bool Symtab::addType(Type *type)
{
  bool result = addUserType(type);
  if (!result)
    return false;

  return true;
}

SYMTAB_EXPORT void Symtab::getAllstdTypes(vector<boost::shared_ptr<Type>>& v)
{
   return stdTypes()->getAllTypes(v); 	
}

SYMTAB_EXPORT void Symtab::getAllbuiltInTypes(vector<boost::shared_ptr<Type>>& v)
{
   return builtInTypes()->getAllBuiltInTypes(v);
}

SYMTAB_EXPORT bool Symtab::findType(boost::shared_ptr<Type> &type, std::string name)
{
   parseTypesNow();

   if (indexed_modules.empty())
      return false;

   for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
	   typeCollection *tc = (*i)->getModuleTypes();
	   if (!tc) continue;
	   type = tc->findType(name, Type::share);
	   if (type) return true;
   }

   if (type == NULL)
      return false;

   return true;	
}

SYMTAB_EXPORT boost::shared_ptr<Type> Symtab::findType(unsigned type_id, Type::do_share_t)
{
	boost::shared_ptr<Type> t;
   parseTypesNow();

   if (indexed_modules.empty())
   {
      return NULL;
   }

   for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
	   typeCollection *tc = (*i)->getModuleTypes();
	   if (!tc) continue;
	   t = tc->findType(type_id, Type::share);
	   if (t)  break;
   }

   if (t == NULL)
   {
	   if (builtInTypes())
	   {
		   t = builtInTypes()->findBuiltInType(type_id, Type::share);
		   if (t) return t;
	   }

	   if (stdTypes())
	   {
		   t = stdTypes()->findType(type_id, Type::share);
		   if (t) return t;
	   }

	   return NULL;
   }

   return t;	
}

SYMTAB_EXPORT bool Symtab::findVariableType(boost::shared_ptr<Type>& type, std::string name)
{
   parseTypesNow();
    type = NULL;
   for (auto i = indexed_modules.begin(); i != indexed_modules.end(); ++i)
   {
	   typeCollection *tc = (*i)->getModuleTypes();
	   if (!tc) continue;
	   type = tc->findVariableType(name, Type::share);
	   if (type) break;
   }

   if (type == NULL)
      return false;

   return true;	
}

SYMTAB_EXPORT bool Symtab::findLocalVariable(std::vector<localVar *>&vars, std::string name)
{
   parseTypesNow();
   unsigned origSize = vars.size();

   for (unsigned i = 0; i < everyFunction.size(); i++)
   {
      everyFunction[i]->findLocalVariable(vars, name);
   }

   if (vars.size()>origSize)
      return true;

   return false;	
}

SYMTAB_EXPORT bool Symtab::hasRel() const
{
   return hasRel_;
}

SYMTAB_EXPORT bool Symtab::hasRela() const
{
   return hasRela_;
}

SYMTAB_EXPORT bool Symtab::hasReldyn() const
{
   return hasReldyn_;
}

SYMTAB_EXPORT bool Symtab::hasReladyn() const
{
   return hasReladyn_;
}

SYMTAB_EXPORT bool Symtab::hasRelplt() const
{
   return hasRelplt_;
}

SYMTAB_EXPORT bool Symtab::hasRelaplt() const
{
   return hasRelaplt_;
}

SYMTAB_EXPORT bool Symtab::isStaticBinary() const
{
   return isStaticBinary_;
}

bool Symtab::setDefaultNamespacePrefix(string &str)
{
   defaultNamespacePrefix = str;
   return true;
}

SYMTAB_EXPORT bool Symtab::emitSymbols(Object *linkedFile,std::string filename, unsigned flag)
{
    // Start with all the defined symbols
    std::set<Symbol* > allSyms;
    allSyms.insert(everyDefinedSymbol.begin(), everyDefinedSymbol.end());

    // Add the undefined dynamic symbols

    allSyms.insert(undefDynSyms.begin(), undefDynSyms.end());

    // Write the new file
    return linkedFile->emitDriver(filename, allSyms, flag);
}

SYMTAB_EXPORT bool Symtab::emit(std::string filename, unsigned flag)
{
	Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
   obj->mf->setSharing(false);
   return emitSymbols(obj, filename, flag);
}

SYMTAB_EXPORT void Symtab::addDynLibSubstitution(std::string oldName, std::string newName)
{
   dynLibSubs[oldName] = newName;
}

SYMTAB_EXPORT std::string Symtab::getDynLibSubstitution(std::string name)
{
#ifdef BINEDIT_DEBUG
   map<std::string, std::string>::iterator iter = dynLibSubs.begin();

   printf ("substitutions for %s:\n", mf->filename().c_str());

   while (iter != dynLibSubs.end()) 
   {
      printf("  \"%s\" => \"%s\"\n", iter->first.c_str(), iter->second.c_str());
      iter++;
   }
#endif

   map<std::string, std::string>::iterator loc = dynLibSubs.find(name);

   if (loc == dynLibSubs.end())
      return name;
   else
      return loc->second;
}

SYMTAB_EXPORT bool Symtab::getSegments(vector<Segment> &segs) const
{
   segs = segments_;

   if (!segments_.size()) 
      return false;

   return true;
}

SYMTAB_EXPORT bool Symtab::getMappedRegions(std::vector<Region *> &mappedRegs) const
{
   unsigned origSize = mappedRegs.size();

   for (unsigned i = 0; i < regions_.size(); i++)
   {
      if (regions_[i]->isLoadable())
         mappedRegs.push_back(regions_[i]);
   }

   if (mappedRegs.size() > origSize)
      return true;

   return false;
}

SYMTAB_EXPORT bool Symtab::fixup_RegionAddr(const char* name, Offset memOffset, long memSize)
{
    Region *sec;

    if (!findRegion(sec, name)) {
        return false;
    }

    vector<relocationEntry> relocs;
    Object *obj = getObject();

    // Fix relocation table with correct memory address
    if (obj) {
        obj->get_func_binding_table(relocs);

        for (unsigned i=0; i < relocs.size(); i++) {
            Offset value = relocs[i].rel_addr();
            relocs[i].setRelAddr(memOffset + value);
        }
    }
    relocation_table_ = relocs;

    vector<relocationEntry> &relref = sec->getRelocations();
    for (unsigned i=0; i < relref.size(); i++) {
        Offset value = relref[i].rel_addr();
        relref[i].setRelAddr(memOffset + value);
    }

#if defined(_MSC_VER)
    regionsByEntryAddr.erase(sec->getMemOffset());
#endif

    sec->setMemOffset(memOffset);
    sec->setMemSize(memSize);

#if defined(_MSC_VER)
    regionsByEntryAddr[sec->getMemOffset()] = sec;
#endif

    std::sort(codeRegions_.begin(), codeRegions_.end(), sort_reg_by_addr);
    std::sort(dataRegions_.begin(), dataRegions_.end(), sort_reg_by_addr);
    std::sort(regions_.begin(), regions_.end(), sort_reg_by_addr);
    return true;
}

SYMTAB_EXPORT bool Symtab::fixup_SymbolAddr(const char* name, Offset newOffset)
{
  Symbol* sym;
  {
    // Find the symbol.
    indexed_symbols::by_name_t::const_accessor ma;
    if(!everyDefinedSymbol.by_mangled.find(ma, name)) return false;
    if(ma->second.size() > 1)
      create_printf("*** Found %zu symbols with name %s.  Expecting 1.\n",
                    ma->second.size(), name);
    sym = ma->second[0];

    // Update symbol.
    indexed_symbols::master_t::accessor a;
    if (!everyDefinedSymbol.master.find(a, sym))  {
        assert(!"everyDefinedSymbol.master.find(a, sym)");
    }
    Offset old = a->second;

    sym->setOffset(newOffset);
    a->second = newOffset;

    // Update the by_offset table
    indexed_symbols::by_offset_t::accessor oa;
    if (!everyDefinedSymbol.by_offset.find(oa, old))  {
        assert(!"everyDefinedSymbol.by_offset.find(oa, old)");
    }
    std::remove(oa->second.begin(), oa->second.end(), sym);

    everyDefinedSymbol.by_offset.insert(oa, newOffset);
    oa->second.push_back(sym);
  }

  // Update aggregates.
  if (!doNotAggregate(sym)) {
    addSymbolToAggregates(sym);
  }

  return true;
}

SYMTAB_EXPORT bool Symtab::updateRegion(const char* name, void *buffer, unsigned size)
{
   Region *sec;

   if (!findRegion(sec, name))
      return false;

   sec->setPtrToRawData(buffer, size);

   return true;
}

SYMTAB_EXPORT bool Symtab::updateCode(void *buffer, unsigned size)
{
  return updateRegion(".text", buffer, size);
}

SYMTAB_EXPORT bool Symtab::updateData(void *buffer, unsigned size)
{
  return updateRegion(".data", buffer, size);
}

SYMTAB_EXPORT Offset Symtab::getFreeOffset(unsigned size) 
{
   // Look through sections until we find a gap with
   // sufficient space.
   Offset highWaterMark = 0;
   Offset secoffset = 0;
   Offset prevSecoffset = 0;
   Object *linkedFile = getObject();
   if (!linkedFile)
     {
       return 0;
     }
   
   for (unsigned i = 0; i < regions_.size(); i++) 
   {
      Offset end = regions_[i]->getMemOffset() + regions_[i]->getMemSize();
      if (regions_[i]->getMemOffset() == 0) 
         continue;

      prevSecoffset = secoffset;

      unsigned region_offset = (unsigned)((char *)(regions_[i]->getPtrToRawData())
                                          - linkedFile->mem_image());

      if (region_offset < (unsigned)prevSecoffset)
      {
         secoffset += regions_[i]->getMemSize();
      }
      else 
      {
         secoffset = (char *)(regions_[i]->getPtrToRawData()) - linkedFile->mem_image();
         secoffset += regions_[i]->getMemSize();
      }

      if (end > highWaterMark) 
      {
         newSectionInsertPoint = i+1;
         highWaterMark = end;
      }

      if (     (i < (regions_.size()-2)) 
               && ((end + size) < regions_[i+1]->getMemOffset())) 
      {
         newSectionInsertPoint = i+1;
         highWaterMark = end;
         break;
      }
   }

   //   return highWaterMark;
#if defined (os_windows)
	Object *obj = getObject();
	if (!obj)
	{
		return 0;
	}
	unsigned pgSize = obj->getSecAlign();
	//printf("pgSize:0x%x\n", pgSize);
	Offset newaddr = highWaterMark  - (highWaterMark & (pgSize-1));
	while(newaddr < highWaterMark)
      newaddr += pgSize;
	//printf("getfreeoffset:%lu\n", newaddr);
	return newaddr;

#else
	unsigned pgSize = P_getpagesize();

#if defined(os_linux)
	Object *obj = getObject();
	if (!obj)
	{
		return 0;
	}
#endif	
	Offset newaddr = highWaterMark  - (highWaterMark & (pgSize-1));
	if(newaddr < highWaterMark)
		newaddr += pgSize;
   return newaddr;
#endif	
}

SYMTAB_EXPORT ObjectType Symtab::getObjectType() const 
{
   return object_type_;
}

SYMTAB_EXPORT Dyninst::Architecture Symtab::getArchitecture() const
{
   return getObject()->getArch();
}

SYMTAB_EXPORT char *Symtab::mem_image() const 
{
   return (char *)mf->base_addr();
}

SYMTAB_EXPORT std::string Symtab::file() const 
{
   assert(mf);
   return mf->pathname();
}

SYMTAB_EXPORT std::string Symtab::name() const 
{
  return mf->filename();
}

SYMTAB_EXPORT std::string Symtab::memberName() const 
{
    return member_name_;
}

SYMTAB_EXPORT unsigned Symtab::getNumberOfRegions() const 
{
   return no_of_sections; 
}

SYMTAB_EXPORT unsigned Symtab::getNumberOfSymbols() const 
{
   return no_of_symbols; 
}


SYMTAB_EXPORT LookupInterface::LookupInterface() 
{
}

SYMTAB_EXPORT LookupInterface::~LookupInterface()
{
}


SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(Offset tStart, 
      unsigned tSize, 
      Offset cStart) 
: tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true),
  tryStart_ptr(0), tryEnd_ptr(0), catchStart_ptr(0), fdeStart_ptr(0), fdeEnd_ptr(0)
{
}

   SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(Offset cStart) 
: tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false),
  tryStart_ptr(0), tryEnd_ptr(0), catchStart_ptr(0), fdeStart_ptr(0), fdeEnd_ptr(0)
{
}

SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(const ExceptionBlock &eb) :
   tryStart_(eb.tryStart_), trySize_(eb.trySize_), 
   catchStart_(eb.catchStart_), hasTry_(eb.hasTry_),
   tryStart_ptr(eb.tryStart_ptr),
   tryEnd_ptr(eb.tryEnd_ptr),
   catchStart_ptr(eb.catchStart_ptr),
   fdeStart_ptr(eb.fdeStart_ptr),
   fdeEnd_ptr(eb.fdeEnd_ptr)
{
}
SYMTAB_EXPORT bool ExceptionBlock::hasTry() const
{ 
   return hasTry_; 
}

SYMTAB_EXPORT Offset ExceptionBlock::tryStart() const
{ 
   return tryStart_; 
}

SYMTAB_EXPORT Offset ExceptionBlock::tryEnd() const
{ 
   return tryStart_ + trySize_; 
}

SYMTAB_EXPORT Offset ExceptionBlock::trySize() const
{
   return trySize_; 
}

SYMTAB_EXPORT bool ExceptionBlock::contains(Offset a) const
{ 
   return (a >= tryStart_ && a < tryStart_ + trySize_); 
}

SYMTAB_EXPORT relocationEntry::relocationEntry() :
   target_addr_(0), 
   rel_addr_(0), 
   addend_(0), 
   rtype_(Region::RT_REL), 
   name_(""), 
   dynref_(NULL), 
   relType_(0),
   rel_struct_addr_(0)
{
}   

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, std::string n, 
      Symbol *dynref, unsigned long relType) :
   target_addr_(ta), 
   rel_addr_(ra), 
   addend_(0), 
   rtype_(Region::RT_REL), 
   name_(n), 
   dynref_(dynref), 
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, Offset add, 
      std::string n, Symbol *dynref, unsigned long relType) :
   target_addr_(ta), 
   rel_addr_(ra), 
   addend_(add), 
   rtype_(Region::RT_REL), 
   name_(n), 
   dynref_(dynref), 
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ra, std::string n, 
      Symbol *dynref, unsigned long relType, Region::RegionType rtype) :
   target_addr_(0), 
   rel_addr_(ra), 
   addend_(0), 
   rtype_(rtype), 
   name_(n), 
   dynref_(dynref), 
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, Offset add,
        std::string n, Symbol *dynref, unsigned long relType,
        Region::RegionType rtype) :
    target_addr_(ta),
    rel_addr_(ra),
    addend_(add),
    rtype_(rtype),
    name_(n),
    dynref_(dynref),
    relType_(relType),
    rel_struct_addr_(0)
{
}

SYMTAB_EXPORT Offset relocationEntry::target_addr() const 
{
    return target_addr_;
}

SYMTAB_EXPORT void relocationEntry::setTargetAddr(const Offset off)
{
    target_addr_ = off;
}

SYMTAB_EXPORT Offset relocationEntry::rel_addr() const 
{
    return rel_addr_;
}

SYMTAB_EXPORT void relocationEntry::setRelAddr(const Offset value)
{
    rel_addr_ = value;
}

SYMTAB_EXPORT const string &relocationEntry::name() const 
{
    return name_;
}

SYMTAB_EXPORT Symbol *relocationEntry::getDynSym() const 
{
    return dynref_;
}

SYMTAB_EXPORT bool relocationEntry::addDynSym(Symbol *dynref) 
{
    dynref_ = dynref;
    return true;
}

SYMTAB_EXPORT Region::RegionType relocationEntry::regionType() const
{
	return rtype_;
}

SYMTAB_EXPORT unsigned long relocationEntry::getRelType() const 
{
    return relType_;
}

SYMTAB_EXPORT Offset relocationEntry::addend() const
{
        return addend_;
}

SYMTAB_EXPORT void relocationEntry::setAddend(const Offset value)
{
        addend_ = value;
}

SYMTAB_EXPORT void relocationEntry::setRegionType(const Region::RegionType value)
{
        rtype_ = value;
}

SYMTAB_EXPORT void relocationEntry::setName(const std::string &newName) {
    name_ = newName;
}

bool relocationEntry::operator==(const relocationEntry &r) const
{
	if (target_addr_ != r.target_addr_) return false;
	if (rel_addr_ != r.rel_addr_) return false;
	if (addend_ != r.addend_) return false;
	if (rtype_ != r.rtype_) return false;
	if (name_ != r.name_) return false;
	if (relType_ != r.relType_) return false;
	if (dynref_ && !r.dynref_) return false;
	if (!dynref_ && r.dynref_) return false;
	if (dynref_)
	{
		if (dynref_->getMangledName() != r.dynref_->getMangledName()) return false;
		if (dynref_->getOffset() != r.dynref_->getOffset()) return false;
	}

	return true;
}

ostream & Dyninst::SymtabAPI::operator<< (ostream &os, const relocationEntry &r) 
{
    if( r.getDynSym() != NULL ) {
        os << "Name: " << setw(20) << ( "'" + r.getDynSym()->getMangledName() + "'" );
    }else{
        os << "Name: " << setw(20) << r.name();
    }
    os << " Offset: " << std::hex << std::setfill('0') << setw(8) << r.rel_addr() 
       << std::dec << std::setfill(' ')
       << " Offset: " << std::hex << std::setfill('0') << setw(8) << r.target_addr() 
       << std::dec << std::setfill(' ')
       << " Addend: " << r.addend()
       << " Region: " << Region::regionType2Str(r.regionType())
       << " Type: " << setw(15) << relocationEntry::relType2Str(r.getRelType())
       << "(" << r.getRelType() << ")";
    if( r.getDynSym() != NULL ) {
        os << " Symbol Offset: " << std::hex << std::setfill('0') << setw(8) << r.getDynSym()->getOffset();
        os << std::dec << std::setfill(' ');
        if( r.getDynSym()->isCommonStorage() ) {
            os << " COM";
        }else if( r.getDynSym()->getRegion() == NULL ) {
            os << " UND";
        }
    }
    return os;
}

const char *Symbol::symbolType2Str(SymbolType t) 
{
   switch (t) 
   {
      CASE_RETURN_STR(ST_UNKNOWN);
      CASE_RETURN_STR(ST_FUNCTION);
      CASE_RETURN_STR(ST_OBJECT);
      CASE_RETURN_STR(ST_MODULE);
      CASE_RETURN_STR(ST_SECTION);
      CASE_RETURN_STR(ST_TLS);
      CASE_RETURN_STR(ST_DELETED);
      CASE_RETURN_STR(ST_NOTYPE);
      CASE_RETURN_STR(ST_INDIRECT);
   };

   return "invalid symbol type";
}

const char *Symbol::symbolLinkage2Str(SymbolLinkage t) 
{
   switch (t) 
   {
      CASE_RETURN_STR(SL_UNKNOWN);
      CASE_RETURN_STR(SL_GLOBAL);
      CASE_RETURN_STR(SL_LOCAL);
      CASE_RETURN_STR(SL_WEAK);
      CASE_RETURN_STR(SL_UNIQUE);
   };

   return "invalid symbol linkage";
}

const char *Symbol::symbolTag2Str(SymbolTag t) 
{
   switch (t) 
   {
      CASE_RETURN_STR(TAG_UNKNOWN);
      CASE_RETURN_STR(TAG_USER);
      CASE_RETURN_STR(TAG_LIBRARY);
      CASE_RETURN_STR(TAG_INTERNAL);
   };

   return "invalid symbol tag";
}

const char *Symbol::symbolVisibility2Str(SymbolVisibility t) 
{
   switch(t) {
      CASE_RETURN_STR(SV_UNKNOWN);
      CASE_RETURN_STR(SV_DEFAULT);
      CASE_RETURN_STR(SV_INTERNAL);
      CASE_RETURN_STR(SV_HIDDEN);
      CASE_RETURN_STR(SV_PROTECTED);
   }
   return "invalid symbol visibility";
}

bool Symtab::hasStackwalkDebugInfo()
{

	Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
   return obj->hasFrameDebugInfo();
}

bool Symtab::getRegValueAtFrame(Address pc, 
                                Dyninst::MachRegister reg, 
                                Dyninst::MachRegisterVal &reg_result,
                                MemRegReader *reader)
{
	Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
   return obj->getRegValueAtFrame(pc, reg, reg_result, reader);
}

Object *Symtab::getObject()
{
   return obj_private;
}

const Object *Symtab::getObject() const
{
    return obj_private;
}

void Symtab::parseTypesNow()
{
   if (isTypeInfoValid_)
      return;
   isTypeInfoValid_ = true;

   parseTypes();
}

SYMTAB_EXPORT Offset Symtab::getElfDynamicOffset()
{
#if defined(os_linux) || defined(os_freebsd)
	Object *obj = getObject();
	if (!obj)
	{
		return 0;
	}
   return obj->getDynamicAddr();
#else
   return 0;
#endif
}

SYMTAB_EXPORT bool Symtab::removeLibraryDependency(std::string lib)
{
#if defined(os_windows)
   return false;
#else
   Object *obj = getObject();
	if (!obj) {
		return false;
	}
   return obj->removePrereqLibrary(lib);
#endif
}
   
SYMTAB_EXPORT bool Symtab::addLibraryPrereq(std::string name)
{
   Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
   // remove forward slashes and back slashes
   size_t size = name.find_last_of("/");
   size_t lastBS = name.find_last_of("\\");
   if (lastBS > size) {
      size = lastBS;
   }

   string filename = name.substr(size+1);

#if ! defined(os_windows) 
   obj->insertPrereqLibrary(filename);
#else 
   // must add a symbol for an exported function belonging to the library 
   // to get the Windows loader to load the library

   Symtab *symtab = Symtab::findOpenSymtab(name);
   if (!symtab) {
      if (!Symtab::openFile(symtab, name)) {
         return false;
      }
   }
   
   // find an exported function
   vector<Symbol*> funcs;
   symtab->getAllSymbolsByType(funcs, Symbol::ST_FUNCTION);
   vector<Symbol*>::iterator fit = funcs.begin(); 
   for (; fit != funcs.end() && !(*fit)->isInDynSymtab(); fit++);
   if (fit == funcs.end()) {
      return false;
   }
   
   string funcName = string((*fit)->getPrettyName());
   if (funcName.empty()) {
      funcName = string((*fit)->getMangledName());
      if (funcName.empty()) {
         assert(0);
         return false;
      }
   }
   symtab->getObject()->addReference((*fit)->getOffset(), 
                                     name, 
                                     funcName);
   obj->addReference((*fit)->getOffset(), filename, funcName);
#endif
   return true;
}

SYMTAB_EXPORT bool Symtab::addSysVDynamic(long name, long value)
{
#if defined(os_linux) || defined(os_freebsd)
	Object *obj = getObject();
	if (!obj)
	{
		return false;
	}
  obj->insertDynamicEntry(name, value);
   return true;
#else
   return false;
#endif
}

SYMTAB_EXPORT bool Symtab::addExternalSymbolReference(Symbol *externalSym, Region *localRegion,
        relocationEntry localRel)
{
    // Adjust this to the correct value
    localRel.setRegionType(getObject()->getRelType());

    // Create placeholder Symbol for external Symbol reference
    // Bernat, 7SEP2010 - according to Matt, these symbols should have
    // type "undefined", which means a region of NULL. Changing
    // from "localRegion" to NULL. 
    Symbol *symRef = new Symbol(externalSym->getMangledName(),
                                externalSym->getType(),
                                Symbol::SL_GLOBAL,
                                Symbol::SV_DEFAULT,
                                (Address)0,
                                getDefaultModule(),
                                NULL, // localRegion,
                                externalSym->getSize(),
                                true,
                                false);

   if( !addSymbol(symRef, externalSym) ) return false;

   localRegion->addRelocationEntry(localRel);

   // Make sure the Symtab holding the external symbol gets linked
   // with this Symtab
   explicitSymtabRefs_.insert(externalSym->getSymtab());

   return true;
}

// on windows we can't specify the trap table's location by adding a dynamic
// symbol as we don on windows
SYMTAB_EXPORT bool Symtab::addTrapHeader_win(Address ptr)
{
#if defined(os_windows)
   getObject()->setTrapHeader(ptr);
   return true;
#else
   (void) ptr; //keep compiler happy
   assert(0);
   return false;
#endif
}

bool Symtab::getExplicitSymtabRefs(std::set<Symtab *> &refs) {
    refs = explicitSymtabRefs_;
    return (refs.size() != 0);
}

SYMTAB_EXPORT bool Symtab::addLinkingResource(Archive *library) {
    linkingResources_.push_back(library);

    return true;
}

SYMTAB_EXPORT bool Symtab::getLinkingResources(std::vector<Archive *> &libs) {
    libs = linkingResources_;
    return (linkingResources_.size() != 0);
}

SYMTAB_EXPORT Address Symtab::getLoadAddress()
{
#if defined(os_linux) || defined(os_freebsd)
   return getObject()->getLoadAddress();
#else
   return 0x0;
#endif
}

SYMTAB_EXPORT bool Symtab::isDefensiveBinary() const
{
    return isDefensiveBinary_;
}

SYMTAB_EXPORT bool Symtab::canBeShared()
{
   return mf->canBeShared();
}

SYMTAB_EXPORT Offset Symtab::getInitOffset()
{
#if defined(os_linux) || defined(os_freebsd)
   return getObject()->getInitAddr();
#else
   return 0x0;
#endif

}

SYMTAB_EXPORT Offset Symtab::getFiniOffset()
{
#if defined(os_linux) || defined(os_freebsd)
   return getObject()->getFiniAddr();
#else
   return 0x0;
#endif

}

void Symtab::getSegmentsSymReader(std::vector<SymSegment> &segs) {
#if !defined(os_windows)
   obj_private->getSegmentsSymReader(segs);
#endif
}

void Symtab::rebase(Offset loadOff)
{
	getObject()->rebase(loadOff);
	load_address_ = loadOff;
}

ModRangeLookup *Symtab::mod_lookup() {
    if(!mod_lookup_) mod_lookup_ = new ModRangeLookup;
    return mod_lookup_;

}


void Symtab::dumpModRanges() {
  if (mod_lookup_) {
    mod_lookup_->PrintPreorder();
  }
}

void Symtab::dumpFuncRanges() {
  if (func_lookup) {
    func_lookup->PrintPreorder();
  }
}
