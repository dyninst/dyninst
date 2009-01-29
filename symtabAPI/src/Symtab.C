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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
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

#include "debug.h"

#include "symtabAPI/src/Object.h"

#if !defined(os_windows)
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#include <iomanip>
#include <stdarg.h>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static std::string errMsg;
extern bool parseCompilerType(Object *);

void pd_log_perror(const char *msg)
{
   errMsg = std::string(msg);
};


SymtabError serr;

std::vector<Symtab *> Symtab::allSymtabs;
builtInTypeCollection *Symtab::builtInTypes = NULL;
typeCollection *Symtab::stdTypes = NULL;
 
SymtabError Symtab::getLastSymtabError()
{
    return serr;
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
        case Invalid_Flags:
            return "Flags passed are invalid.";
	case No_Error:
	    return "No previous Error.";
        default:
            return "Unknown Error";
    }		
}

void Symtab::setupTypes()
{
    /*
     * Create the "error" and "untyped" types.
     */
    std::string name = "<error>";
    type_Error   = Type::createFake(name);
    name = "<no type>";
    type_Untyped = Type::createFake(name);
    setupStdTypes();
}    

void Symtab::setupStdTypes() 
{
   if (builtInTypes)
    	return;

   builtInTypes = new builtInTypeCollection;
   typeScalar *newType;

   // NOTE: integral type  mean twos-complement
   // -1  int, 32 bit signed integral type
   // in stab document, size specified in bits, system size is in bytes
   builtInTypes->addBuiltInType(newType = new typeScalar(-1, 4, "int", true));
   newType->decrRefCount();
   // -2  char, 8 bit type holding a character. GDB & dbx(AIX) treat as signed
   builtInTypes->addBuiltInType(newType = new typeScalar(-2, 1, "char", true));
   newType->decrRefCount();
   // -3  short, 16 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-3, 2, "short", true));
   newType->decrRefCount();
   // -4  long, 32/64 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-4, sizeof(long), "long", true));
   newType->decrRefCount();
   // -5  unsigned char, 8 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-5, 1, "unsigned char"));
   newType->decrRefCount();
   // -6  signed char, 8 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-6, 1, "signed char", true));
   newType->decrRefCount();
   // -7  unsigned short, 16 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-7, 2, "unsigned short"));
   newType->decrRefCount();
   // -8  unsigned int, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-8, 4, "unsigned int"));
   newType->decrRefCount();
   // -9  unsigned, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-9, 4, "unsigned"));
   newType->decrRefCount();
   // -10 unsigned long, 32 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-10, sizeof(unsigned long), "unsigned long"));
   newType->decrRefCount();
   // -11 void, type indicating the lack of a value
   //  XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(newType = new typeScalar(-11, 0, "void", false));
   newType->decrRefCount();
   // -12 float, IEEE single precision
   builtInTypes->addBuiltInType(newType = new typeScalar(-12, sizeof(float), "float", true));
   newType->decrRefCount();
   // -13 double, IEEE double precision
   builtInTypes->addBuiltInType(newType = new typeScalar(-13, sizeof(double), "double", true));
   newType->decrRefCount();
   // -14 long double, IEEE double precision, size may increase in future
   builtInTypes->addBuiltInType(newType = new typeScalar(-14, sizeof(long double), "long double", true));
   newType->decrRefCount();
   // -15 integer, 32 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-15, 4, "integer", true));
   newType->decrRefCount();
   // -16 boolean, 32 bit type. GDB/GCC 0=False, 1=True, all other values
   //  have unspecified meaning
   builtInTypes->addBuiltInType(newType = new typeScalar(-16, sizeof(bool), "boolean"));
   newType->decrRefCount();
   // -17 short real, IEEE single precision
   //  XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(newType = new typeScalar(-17, sizeof(float), "short real", true));
   newType->decrRefCount();
   // -18 real, IEEE double precision XXX-size may not be correct jdd 4/22/99
   builtInTypes->addBuiltInType(newType = new typeScalar(-18, sizeof(double), "real", true));
   newType->decrRefCount();
   // -19 stringptr XXX- size of void * -- jdd 4/22/99
   builtInTypes->addBuiltInType(newType = new typeScalar(-19, sizeof(void *), "stringptr"));
   newType->decrRefCount();
   // -20 character, 8 bit unsigned character type
   builtInTypes->addBuiltInType(newType = new typeScalar(-20, 1, "character"));
   newType->decrRefCount();
   // -21 logical*1, 8 bit type (Fortran, used for boolean or unsigned int)
   builtInTypes->addBuiltInType(newType = new typeScalar(-21, 1, "logical*1"));
   newType->decrRefCount();
   // -22 logical*2, 16 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(newType = new typeScalar(-22, 2, "logical*2"));
   newType->decrRefCount();
   // -23 logical*4, 32 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(newType = new typeScalar(-23, 4, "logical*4"));
   newType->decrRefCount();
   // -24 logical, 32 bit type (Fortran, some for boolean or unsigned int)
   builtInTypes->addBuiltInType(newType = new typeScalar(-24, 4, "logical"));
   newType->decrRefCount();
   // -25 complex, consists of 2 IEEE single-precision floating point values
   builtInTypes->addBuiltInType(newType = new typeScalar(-25, sizeof(float)*2, "complex", true));
   newType->decrRefCount();
   // -26 complex, consists of 2 IEEE double-precision floating point values
   builtInTypes->addBuiltInType(newType = new typeScalar(-26, sizeof(double)*2, "complex*16", true));
   newType->decrRefCount();
   // -27 integer*1, 8 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-27, 1, "integer*1", true));
   newType->decrRefCount();
   // -28 integer*2, 16 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-28, 2, "integer*2", true));
   newType->decrRefCount();

   /* Quick hack to make integer*4 compatible with int for Fortran
      jnb 6/20/01 */
   // This seems questionable - let's try removing that hack - jmo 05/21/04
   /*
     builtInTypes->addBuiltInType(newType = new type("int",-29,
     built_inType, 4));
     newType->decrRefCount();
   */
   // -29 integer*4, 32 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-29, 4, "integer*4", true));
   newType->decrRefCount();
   // -30 wchar, Wide character, 16 bits wide, unsigned (unknown format)
   builtInTypes->addBuiltInType(newType = new typeScalar(-30, 2, "wchar"));
   newType->decrRefCount();
#if defined(os_windows)
   // -31 long long, 64 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-31, sizeof(LONGLONG), "long long", true));
   newType->decrRefCount();
   // -32 unsigned long long, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-32, sizeof(ULONGLONG), "unsigned long long"));
   newType->decrRefCount();
#else
   // -31 long long, 64 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-31, sizeof(long long), "long long", true));
   newType->decrRefCount();
   // -32 unsigned long long, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-32, sizeof(unsigned long long), "unsigned long long"));
   newType->decrRefCount();
#endif
   // -33 logical*8, 64 bit unsigned integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-33, 8, "logical*8"));
   newType->decrRefCount();
   // -34 integer*8, 64 bit signed integral type
   builtInTypes->addBuiltInType(newType = new typeScalar(-34, 8, "integer*8", true));
   newType->decrRefCount();

	/*
    * Initialize hash table of standard types.
    */
	if (stdTypes)
    	return;

   stdTypes = typeCollection::getGlobalTypeCollection();
   stdTypes->addType(newType = new typeScalar(-1, sizeof(int), "int"));
   newType->decrRefCount();

   Type *charType = new typeScalar(-2, sizeof(char), "char");
   stdTypes->addType(charType);

	std::string tName = "char *";
	typePointer *newPtrType;
   stdTypes->addType(newPtrType = new typePointer(-3, charType, tName));
   charType->decrRefCount();
   newPtrType->decrRefCount();

   Type *voidType = new typeScalar(-11, 0, "void", false);
   stdTypes->addType(voidType);

	tName = "void *";
   stdTypes->addType(newPtrType = new typePointer(-4, voidType, tName));
   voidType->decrRefCount();
   newPtrType->decrRefCount();

   stdTypes->addType(newType = new typeScalar(-12, sizeof(float), "float"));
   newType->decrRefCount();

#if defined(i386_unknown_nt4_0)
   stdTypes->addType(newType = new typeScalar(-31, sizeof(LONGLONG), "long long"));    
#else
   stdTypes->addType(newType = new typeScalar(-31, sizeof(long long), "long long"));
#endif

	newType->decrRefCount();

   return;
}

SYMTAB_EXPORT unsigned Symtab::getAddressWidth() const 
{
   return address_width_;
}
 
SYMTAB_EXPORT bool Symtab::isNativeCompiler() const 
{
    return nativeCompiler; 
}
 

SYMTAB_EXPORT Symtab::Symtab(MappedFile *mf_) :
   AnnotatableSparse(),
   mf(mf_), 
   mfForDebugInfo(mf_),
   obj_private(NULL)
{   
    init_debug_symtabAPI();
}   


SYMTAB_EXPORT Symtab::Symtab() :
   obj_private(NULL)
{
    init_debug_symtabAPI();
    create_printf("%s[%d]: Created symtab via default constructor\n", FILE__, __LINE__);
    defaultNamespacePrefix = "";
}

SYMTAB_EXPORT bool Symtab::isExec() const 
{
    return is_a_out; 
}

SYMTAB_EXPORT bool Symtab::isStripped() 
{
#if defined(os_linux) || defined(os_solaris)
    Region *sec;
    return findRegion(sec,".symtab");
#else
    return (no_of_symbols==0);
#endif
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

SYMTAB_EXPORT Offset Symtab::getTOCoffset() const 
{
   return toc_offset_;
}

SYMTAB_EXPORT string Symtab::getDefaultNamespacePrefix() const
{
    return defaultNamespacePrefix;
}
	
	
// TODO -- is this g++ specific
bool Symtab::buildDemangledName( const std::string &mangled, 
      std::string &pretty,
      std::string &typed,
      bool nativeCompiler, 
      supportedLanguages lang )
{
   /* The C++ demangling function demangles MPI__Allgather (and other MPI__
    * functions with start with A) into the MPI constructor.  In order to
    * prevent this a hack needed to be made, and this seemed the cleanest
    * approach.
    */

   if ((mangled.length()>5) && (mangled.substr(0,5)==std::string("MPI__"))) 
   {
      return false;
   }	  

   /* If it's Fortran, eliminate the trailing underscores, if any. */
   if (lang == lang_Fortran 
         || lang == lang_CMFortran 
         || lang == lang_Fortran_with_pretty_debug )
   {
      if ( mangled[ mangled.length() - 1 ] == '_' ) 
      {
         char * demangled = P_strdup( mangled.c_str() );
         demangled[ mangled.length() - 1 ] = '\0';
         pretty = std::string( demangled );

         free ( demangled );
         return true;
      }
      else 
      {
         /* No trailing underscores, do nothing */
         return false;
      }
   } /* end if it's Fortran. */

   //  Check to see if we have a gnu versioned symbol on our hands.
   //  These are of the form <symbol>@<version> or <symbol>@@<version>
   //
   //  If we do, we want to create a "demangled" name for the one that
   //  is of the form <symbol>@@<version> since this is, by definition,
   //  the default.  The "demangled" name will just be <symbol>

   //  NOTE:  this is just a 0th order approach to dealing with versioned
   //         symbols.  We may need to do something more sophisticated
   //         in the future.  JAW 10/03

#if !defined(os_windows)

   char *atat;

   if (NULL != (atat = strstr(mangled.c_str(), "@@"))) 
   {
        pretty = mangled.substr(0 /*start pos*/, 
                        (int)(atat - mangled.c_str())/*len*/);
        //char msg[256];
        //sprintf(msg, "%s[%d]: 'demangling' versioned symbol: %s, to %s",
        //          __FILE__, __LINE__, mangled.c_str(), pretty.c_str());

        //cerr << msg << endl;
        //logLine(msg);
      
        return true;
    }

#endif

    bool retval = false;
  
    /* Try demangling it. */
    char * demangled = P_cplus_demangle( mangled.c_str(), nativeCompiler, false);
    if (demangled) 
    {
        pretty = std::string( demangled );
        retval = true;
    }
  
    char *t_demangled = P_cplus_demangle(mangled.c_str(), nativeCompiler, true);
    if (t_demangled && (strcmp(t_demangled, demangled) != 0)) 
    {
        typed = std::string(t_demangled);
        retval = true;
    }

    if (demangled)
        free(demangled);
    if (t_demangled)
        free(t_demangled);

    return retval;
} /* end buildDemangledName() */


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
    for (SymbolIter symIter(*linkedFile); symIter; symIter++) {
        Symbol *sym = symIter.currval();

        // If a symbol starts with "." we want to skip it. These indicate labels in the
        // code. 
        
        // removed 1/09: this should be done in Dyninst, not Symtab
#if 0
        if (sym->getMangledName()[0] == '.') 
            continue;
#endif

        // check for undefined dynamic symbols. Used when rewriting relocation section.
        // relocation entries have references to these undefined dynamic symbols.
        // We also have undefined symbols for the static binary case.
        
        if (sym->getSec() == NULL && !sym->isAbsolute()) {
            undefDynSyms[sym->getMangledName()].push_back(sym);
            continue;
        }

        // Check whether this symbol has a valid offset. If they do not we have a
        // consistency issue. This should be a null check.

        // Symbols can have an offset of 0 if they don't refer to things within a file.
#if 0
        if (!isValidOffset(sym->getAddr())) {
            fprintf(stderr, "Symbol %s has invalid offset 0x%lx\n", sym->getName().c_str(), sym->getAddr());
            fprintf(stderr, "... in file %s\n", name().c_str());
            return false;
        }
#endif

        raw_syms.push_back(sym);
    }

    return true;
}

/*
 * fixSymModules
 * 
 * Add Module information to all symbols. 
 */

bool Symtab::fixSymModules(std::vector<Symbol *> &raw_syms) 
{
    for (unsigned i = 0; i < raw_syms.size(); i++) {
        fixSymModule(raw_syms[i]);
    }
    return true;
}

/*
 * demangleSymbols
 *
 * Perform name demangling on all symbols.
 */

bool Symtab::demangleSymbols(std::vector<Symbol *> &raw_syms) 
{
    for (unsigned i = 0; i < raw_syms.size(); i++) {
        demangleSymbol(raw_syms[i]);
    }
    return true;
}

/*
 * createIndices
 *
 * We index symbols by various attributes for quick lookup. Build those
 * indices here. 
 */

bool Symtab::createIndices(std::vector<Symbol *> &raw_syms) {
    for (unsigned i = 0; i < raw_syms.size(); i++) {
        addSymbolToIndices(raw_syms[i]);
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

bool Symtab::createAggregates() {
    for (unsigned i = 0; i < everyDefinedSymbol.size(); i++) {
        addSymbolToAggregates(everyDefinedSymbol[i]);
    }
    return true;
}
 
bool Symtab::fixSymModule(Symbol *&sym) {
#if !defined(os_windows)
    // Occasionally get symbols with no module.
    // Variables on Windows platform
    if (sym->getModuleName().length() == 0) return false;
#endif    

    Module *newMod = getOrCreateModule(sym->getModuleName(), sym->getAddr());
    assert(newMod);
    sym->setModule(newMod);
    return true;
}

bool Symtab::demangleSymbol(Symbol *&sym) {
    switch (sym->getType()) {
    case Symbol::ST_FUNCTION: {
        Module *rawmod = getOrCreateModule(sym->getModuleName(),sym->getAddr());
        
        assert(rawmod);
        
        // At this point we need to generate the following information:
        // A symtab name.
        // A pretty (demangled) name.
        // The symtab name goes in the global list as well as the module list.
        // Same for the pretty name.
        // Finally, check addresses to find aliases.
        
        std::string mangled_name = sym->getMangledName();
        std::string working_name = mangled_name;
        
#if !defined(os_windows)        
        //Remove extra stabs information
        const char *p = strchr(working_name.c_str(), ':');
        if( p ) {
            unsigned nchars = p - mangled_name.c_str();
            working_name = std::string(mangled_name.c_str(), nchars);
        }
#endif        
        
        std::string pretty_name = working_name;
        std::string typed_name = working_name;
        
        if (!buildDemangledName(working_name, pretty_name, typed_name,
                                nativeCompiler, rawmod->language())) {
            pretty_name = working_name;
        }

        sym->setPrettyName(pretty_name);
        sym->setTypedName(typed_name);
        
        break;
    }
    default: {
        // All cases where there really shouldn't be a mangled
        // name, since mangling is for functions.
        
        char *prettyName = P_cplus_demangle(sym->getMangledName().c_str(), nativeCompiler, false);
        if (prettyName) {
            sym->setPrettyName(prettyName);
        }
        else {
            sym->setPrettyName(sym->getMangledName().c_str());
        }
        break;
    }
    }
    return true;
}

bool Symtab::addSymbolToIndices(Symbol *&sym) {
    everyDefinedSymbol.push_back(sym);
    
    symsByOffset[sym->getAddr()].push_back(sym);
    
    symsByMangledName[sym->getMangledName()].push_back(sym);

    symsByPrettyName[sym->getPrettyName()].push_back(sym);

    symsByTypedName[sym->getTypedName()].push_back(sym);

    return true;
}

bool Symtab::addSymbolToAggregates(Symbol *&sym) {
    switch(sym->getType()) {
    case Symbol::ST_FUNCTION: {
        // We want to do the following:
        // If no function exists, create and add. 
        // Combine this information
        //   Add this symbol's names to the function.
        //   Keep module information 

        Function *func = NULL;
        findFuncByEntryOffset(func, sym->getAddr());
        if (!func) {
            // Create a new function
            // Also, update the symbol to point to this function.

            func = Function::createFunction(sym);

            everyFunction.push_back(func);
            funcsByOffset[sym->getAddr()] = func;
        }
        else {
            func->addSymbol(sym);
        }
        sym->setFunction(func);

        break;
    }
    case Symbol::ST_OBJECT: {
        // The same as the above, but with variables.
        Variable *var = NULL;
        findVariableByOffset(var, sym->getAddr());
        if (!var) {
            // Create a new function
            // Also, update the symbol to point to this function.
            var = Variable::createVariable(sym);
            
            everyVariable.push_back(var);
            varsByOffset[sym->getAddr()] = var;
        }
        else {
            var->addSymbol(sym);
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

/* Add the new name to the appropriate symbol index */

bool Symtab::updateIndices(Symbol *sym, std::string newName, nameType_t nameType) {
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
    return true;
}

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

//  setModuleLanguages is only called after modules have been defined.
//  it attempts to set each module's language, information which is needed
//  before names can be demangled.
void Symtab::setModuleLanguages(dyn_hash_map<std::string, supportedLanguages> *mod_langs)
{
   if (!mod_langs->size())
      return;  // cannot do anything here
   //  this case will arise on non-stabs platforms until language parsing can be introduced at this level
   std::vector<Module *> *modlist;
   Module *currmod = NULL;
   modlist = &_mods;
   //int dump = 0;

   for (unsigned int i = 0;  i < modlist->size(); ++i)
   {
      currmod = (*modlist)[i];
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

Module *Symtab::getOrCreateModule(const std::string &modName, 
      const Offset modAddr)
{
   std::string nameToUse;
   if (modName.length() > 0)
      nameToUse = modName;
   else
      nameToUse = "DEFAULT_MODULE";

   Module *fm = NULL;
   if (findModuleByName(fm, nameToUse)) 
   {
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
    Module *ret = new Module();
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.

    if (findModuleByName(ret, name)) 
    {
        return(ret);
    }

    delete ret;

    //parsing_printf("=== image, creating new pdmodule %s, addr 0x%x\n",
    //				name.c_str(), addr);
    
    std::string fileNm, fullNm;
    fullNm = name;
    fileNm = extract_pathname_tail(name);

     // /* DEBUG */ fprintf( stderr, "%s[%d]: In %p: Creating new pdmodule '%s'/'%s'\n", FILE__, __LINE__, this, fileNm.c_str(), fullNm.c_str() );

    ret = new Module(lang, addr, fullNm, this);
    assert(ret);

    if (modsByFileName.end() != modsByFileName.find(ret->fileName()))
    {
       fprintf(stderr, "%s[%d]:  WARN:  LEAK?  already have module with name %s\n", 
             FILE__, __LINE__, ret->fileName().c_str());
    }

    if (modsByFullName.end() != modsByFullName.find(ret->fullName()))
    {
       fprintf(stderr, "%s[%d]:  WARN:  LEAK?  already have module with name %s\n", 
             FILE__, __LINE__, ret->fullName().c_str());
    }

    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    _mods.push_back(ret);
    
    return (ret);
}

Symtab::Symtab(std::string filename,bool &err) :
   is_a_out(false), 
   main_call_addr_(0),
   nativeCompiler(false), 
   isLineInfoValid_(false), 
   isTypeInfoValid_(false),
   obj_private(NULL),
   type_Error(NULL), 
   type_Untyped(NULL)
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

   obj_private = new Object(mf, mfForDebugInfo, pd_log_perror, true);

   if (!extractInfo(obj_private))
   {
      create_printf("%s[%d]: WARNING: creating symtab for %s, extractInfo() " 
                    "failed\n", FILE__, __LINE__, filename.c_str());
      err = true;
   }

   defaultNamespacePrefix = "";
}


Symtab::Symtab(char *mem_image, size_t image_size, bool &err) :
   is_a_out(false), 
   main_call_addr_(0),
   nativeCompiler(false),
   isLineInfoValid_(false),
   isTypeInfoValid_(false),
   obj_private(NULL),
   type_Error(NULL), 
   type_Untyped(NULL)
{
   // Initialize error parameter
   err = false;
  
   create_printf("%s[%d]: created symtab for memory image at addr %u\n", 
                 FILE__, __LINE__, mem_image);

   //  createMappedFile handles reference counting
   mf = MappedFile::createMappedFile(mem_image, image_size);
   if (!mf) {
      create_printf("%s[%d]: WARNING: creating symtab for memory image at " 
                    "addr %u, createMappedFile() failed\n", FILE__, __LINE__, 
                    mem_image);
      err = true;
      return;
   }

   obj_private = new Object(mf, mfForDebugInfo, pd_log_perror, true);

   if (!extractInfo(obj_private))
   {
      create_printf("%s[%d]: WARNING: creating symtab for memory image at addr" 
                    "%u, extractInfo() failed\n", FILE__, __LINE__, mem_image);
      err = true;
   }

   defaultNamespacePrefix = "";
}

// Symtab constructor for archive members
#if defined(os_aix) || defined(os_linux) || defined(os_solaris)
Symtab::Symtab(std::string filename, std::string member_name, Offset offset, 
               bool &err, void *base) :
   member_name_(member_name), 
   member_offset_(offset),
   is_a_out(false),
   main_call_addr_(0), 
   nativeCompiler(false), 
   isLineInfoValid_(false),
   isTypeInfoValid_(false), 
   obj_private(NULL),
   type_Error(NULL), 
   type_Untyped(NULL)
{
   mf = MappedFile::createMappedFile(filename);
   assert(mf);
   obj_private = new Object(mf, mfForDebugInfo, member_name, offset, pd_log_perror, base);
   err = extractInfo(obj_private);
   defaultNamespacePrefix = "";
}
#else
Symtab::Symtab(std::string, std::string, Offset, bool &, void *base)
{
    assert(0);
}
#endif

#if defined(os_aix) || defined(os_linux) || defined(os_solaris)
Symtab::Symtab(char *mem_image, size_t image_size, std::string member_name,
                       Offset offset, bool &err, void *base) :
   member_name_(member_name), 
   is_a_out(false), 
   main_call_addr_(0),
   nativeCompiler(false), 
   isLineInfoValid_(false), 
   isTypeInfoValid_(false), 
   type_Error(NULL), 
   type_Untyped(NULL)
{
   mf = MappedFile::createMappedFile(mem_image, image_size);
   assert(mf);
   obj_private = new Object(mf, mf, member_name, offset, pd_log_perror, base);
   err = extractInfo(obj_private);
   defaultNamespacePrefix = "";
}
#else 
Symtab::Symtab(char *, size_t, std::string , Offset, bool &, void *)
{
    assert(0);
}
#endif

bool sort_reg_by_addr(const Region* a, const Region* b)
{
   return a->getRegionAddr() < b->getRegionAddr();
}


extern void print_symbols( std::vector< Symbol *>& allsymbols );
extern void print_symbol_map( dyn_hash_map< std::string, std::vector< Symbol *> > *symbols);

bool Symtab::extractInfo(Object *linkedFile)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    mfForDebugInfo = linkedFile->getMappedFileForDebugInfo();

    bool err = true;
    imageOffset_ = linkedFile->code_off();
    dataOffset_ = linkedFile->data_off();

    imageLen_ = linkedFile->code_len();
    dataLen_ = linkedFile->data_len();
    
    if (0 == imageLen_ || 0 == linkedFile->code_ptr()) 
    {
        // for AIX, code_ptr()==NULL is normal behavior
#if !defined(os_aix)
       if (0 == linkedFile->code_ptr()) {
          //fprintf(stderr, "[%s][%d]WARNING: null code pointer in Symtab for"
          //" file %s, possibly due to a missing .text section.\n",
          //__FILE__,__LINE__, file().c_str());
          linkedFile->code_ptr_ = (char *) linkedFile->code_off();
       }
       else 
#endif
       {
          serr = Obj_Parsing;
          return false;
       }
   }
	
  //  if (!imageLen_ || !linkedFile->code_ptr()) {
  //      serr = Obj_Parsing; 
  //      return false; 
   // }

    no_of_sections = linkedFile->no_of_sections();
    newSectionInsertPoint = no_of_sections;
    no_of_symbols = linkedFile->no_of_symbols();

    hasRel_ = false;
    hasRela_ = false;
    regions_ = linkedFile->getAllRegions();

    for (unsigned index=0;index<regions_.size();index++)
    {
        if ( regions_[index]->isLoadable() ) 
        {
           if (     (regions_[index]->getRegionPermissions() == Region::RP_RX) 
                 || (regions_[index]->getRegionPermissions() == Region::RP_RWX)) 
           {
              codeRegions_.push_back(regions_[index]);
           }
           else 
           {
                dataRegions_.push_back(regions_[index]);
            }
        }

        regionsByEntryAddr[regions_[index]->getRegionAddr()] = regions_[index];

        if (regions_[index]->getRegionType() == Region::RT_REL) 
        {
            hasRel_ = true;
        }

        if (regions_[index]->getRegionType() == Region::RT_RELA) 
        {
            hasRela_ = true;
        }
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
    toc_offset_ = linkedFile->getTOCoffset();
    object_type_  = linkedFile->objType();
    is_eel_ = linkedFile->isEEL();
    linkedFile->getSegments(segments_);

#if !defined(os_aix) && !defined(os_windows)
    linkedFile->getDependencies(deps_);
#endif

#if defined (os_aix)
    //  These should go away
    linkedFile->get_stab_info(stabstr_, nstabs_, stabs_, stringpool_);
    linkedFile->get_line_info(nlines_, lines_, fdptr_);
#endif

#if defined(os_solaris) || defined(os_aix) || defined(os_linux)
    // make sure we're using the right demangler
    
    nativeCompiler = parseCompilerType(linkedFile);
    //parsing_printf("isNativeCompiler: %d\n", nativeCompiler);
#endif
    
    // define all of the functions
    //statusLine("winnowing functions");

#if defined(ppc64_linux)
    checkPPC64DescriptorSymbols(linkedFile);
#endif

    // a vector to hold all created symbols until they are properly classified
    std::vector<Symbol *> raw_syms;

#ifdef BINEDIT_DEBUG
    printf("== from linkedFile...\n");
    print_symbol_map(linkedFile->getAllSymbols());
#endif

    if (!extractSymbolsFromFile(linkedFile, raw_syms)) 
    {
        err = false;
        serr = Syms_To_Functions;
        return false;
    }

#ifdef BINEDIT_DEBUG
    printf("== in Symtab now...\n");
    //print_symbols(raw_syms);
    std::vector<Symbol *> undefsyms;
    std::map<std::string, std::vector<Symbol *> >::iterator iter;
    std::vector<Symbol *>::iterator siter;
    for (iter = undefDynSyms.begin(); iter != undefDynSyms.end(); iter++)
        for (siter = iter->second.begin(); siter != iter->second.end(); siter++)
            undefsyms.push_back(*siter);
    //print_symbols(undefsyms);
    printf("%d total symbol(s)\n", raw_syms.size() + undefsyms.size());
#endif

    // don't sort the symbols--preserve the original ordering
    //sort(raw_syms.begin(),raw_syms.end(),symbol_compare);

    if (!fixSymModules(raw_syms)) 
    {
        err = false;
        serr = Syms_To_Functions;
        return false;
    }

    // wait until all modules are defined before applying languages to
    // them we want to do it this way so that module information comes
    // from the function symbols, first and foremost, to avoid any
    // internal module-function mismatching.
            
    // get Information on the language each modules is written in
    // (prior to making modules)

    dyn_hash_map<std::string, supportedLanguages> mod_langs;
    linkedFile->getModuleLanguageInfo(&mod_langs);
    setModuleLanguages(&mod_langs);
	
    // Be sure that module languages are set before demangling, or
    // we won't get very far.

    if (!demangleSymbols(raw_syms)) 
    {
        err = false;
        serr = Syms_To_Functions;
        return false;
    }

    if (!createIndices(raw_syms)) 
    {
        err = false;
        serr = Syms_To_Functions;
        return false;
    }

    if (!createAggregates()) 
    {
        err = false;
        serr = Syms_To_Functions;
        return false;
    }
	
#if 0
    // define all of the functions, this also defines all of the modules
    if (!symbolsToFunctions(linkedFile, &raw_syms))
    {
        fprintf(stderr, "%s[%d] Error converting symbols to functions in file %s\n", 
                __FILE__, __LINE__, mf->filename().c_str());
        err = false;
        serr = Syms_To_Functions;
        return false;
    }
#endif
	
    // Once languages are assigned, we can build demangled names (in
    // the wider sense of demangling which includes stripping _'s from
    // fortran names -- this is why language information must be
    // determined before this step).
    
    // Also identifies aliases (multiple names with equal addresses)
    
    //addSymtabVariables();
    linkedFile->getAllExceptions(excpBlocks);

    vector<relocationEntry >fbt;
    linkedFile->get_func_binding_table(fbt);
    for(unsigned i=0; i<fbt.size();i++)
        relocation_table_.push_back(fbt[i]);
    return true;
}

Symtab::Symtab(const Symtab& obj) :
   LookupInterface(),
   Serializable(),
   AnnotatableSparse()
{
    create_printf("%s[%d]: Creating symtab 0x%p from symtab 0x%p\n", FILE__, __LINE__, this, &obj);
  
    member_name_ = obj.member_name_;
    imageOffset_ = obj.imageOffset_;
    imageLen_ = obj.imageLen_;
    dataOffset_ = obj.dataOffset_;
    dataLen_ = obj.dataLen_;

   isLineInfoValid_ = obj.isLineInfoValid_;
   isTypeInfoValid_ = obj.isTypeInfoValid_;

   is_a_out = obj.is_a_out;
   main_call_addr_ = obj.main_call_addr_; // address of call to main()

   nativeCompiler = obj.nativeCompiler;
   defaultNamespacePrefix = obj.defaultNamespacePrefix;

   //sections
   no_of_sections = obj.no_of_sections;
   unsigned i;

   for (i=0;i<obj.regions_.size();i++)
      regions_.push_back(new Region(*(obj.regions_[i])));

   for (i=0;i<regions_.size();i++)
      regionsByEntryAddr[regions_[i]->getRegionAddr()] = regions_[i];

   // TODO FIXME: copying symbols/Functions/Variables

   for (i=0;i<obj._mods.size();i++)
   {
      Module *m = new Module(*(obj._mods[i]));
      _mods.push_back(m);
      modsByFileName[m->fileName()] = m;
      modsByFullName[m->fullName()] = m;
      fprintf(stderr, "%s[%d]:  copy ctor creating new module %s\n", 
            FILE__, __LINE__, m->fileName().c_str());
   }

   for (i=0; i<relocation_table_.size();i++) 
   {
      relocation_table_.push_back(relocationEntry(obj.relocation_table_[i]));
      //undefDynSyms[obj.relocation_table_[i].name()] = relocation_table_[i].getDynSym();
      undefDynSyms[obj.relocation_table_[i].name()].push_back(relocation_table_[i].getDynSym());

   }



#if 0
    is_a_out = obj.is_a_out;
    main_call_addr_ = obj.main_call_addr_; // address of call to main()
    
    nativeCompiler = obj.nativeCompiler;
    defaultNamespacePrefix = obj.defaultNamespacePrefix;
    
    //sections
    no_of_sections = obj.no_of_sections;
    unsigned i;
    for(i=0;i<obj.regions_.size();i++)
        regions_.push_back(new Region(*(obj.regions_[i])));
    for(i=0;i<regions_.size();i++)
        regionsByEntryAddr[regions_[i]->getRegionAddr()] = regions_[i];

    // TODO FIXME: copying symbols/Functions/Variables
    
    for(i=0;i<obj._mods.size();i++)
        _mods.push_back(new Module(*(obj._mods[i])));
    for(i=0;i<_mods.size();i++)
    {
        modsByFileName[_mods[i]->fileName()] = _mods[i];
        modsByFullName[_mods[i]->fullName()] = _mods[i];
    }
    
    for(i=0; i<relocation_table_.size();i++) {
        relocation_table_.push_back(relocationEntry(obj.relocation_table_[i]));
        //undefDynSyms[obj.relocation_table_[i].name()] = relocation_table_[i].getDynSym();
        undefDynSyms[obj.relocation_table_[i].name()].push_back(relocation_table_[i].getDynSym());
    }
    
    for(i=0;i<excpBlocks.size();i++)
        excpBlocks.push_back(new ExceptionBlock(*(obj.excpBlocks[i])));
#endif

   for (i=0;i<excpBlocks.size();i++)
   {
      excpBlocks.push_back(new ExceptionBlock(*(obj.excpBlocks[i])));
   }

   deps_ = obj.deps_;
   setupTypes();
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
      fprintf(stderr, "%s[%d] No code regions in %s \n",
            __FILE__, __LINE__, mf->filename().c_str());
      return false;
   }

   // search for "where" in codeRegions_ (code regions must not overlap)
   int first = 0; 
   int last = codeRegions_.size() - 1;

   while (last >= first) 
   {
      Region *curreg = codeRegions_[(first + last) / 2];
      if (where >= curreg->getRegionAddr()
            && where < (curreg->getRegionAddr()
               + curreg->getDiskSize())) 
      {
         return true;
      }
      else if (where < curreg->getRegionAddr()) 
      {
         last = ((first + last) / 2) - 1;
      }
      else if (where >= (curreg->getRegionAddr() + curreg->getMemSize()))
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
      fprintf(stderr, "%s[%d] No data regions in %s \n",
            __FILE__,__LINE__,mf->filename().c_str());
      return false;
   }

   int first = 0; 
   int last = dataRegions_.size() - 1;

   while (last >= first) 
   {
      Region *curreg = dataRegions_[(first + last) / 2];

      if (     (where >= curreg->getRegionAddr())
            && (where < (curreg->getRegionAddr() + curreg->getRegionSize())))
      {
         return true;
      }
      else if (where < curreg->getRegionAddr()) 
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

bool Symtab::getFuncBindingTable(std::vector<relocationEntry> &fbt) const
{
   fbt = relocation_table_;
   return true;
}

SYMTAB_EXPORT std::vector<std::string> &Symtab::getDependencies(){
    return deps_;
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

   // TODO make annotation
   userAddedSymbols.clear();
   symsByOffset.clear();
   symsByMangledName.clear();
   symsByPrettyName.clear();
   symsByTypedName.clear();

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

   for (unsigned i = 0; i < _mods.size(); i++) 
   {
      delete _mods[i];
   }
   _mods.clear();
   modsByFileName.clear();
   modsByFullName.clear();

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

   //fprintf(stderr, "%s[%d]:  symtab DTOR, mf = %p: %s\n", FILE__, __LINE__, mf, mf->filename().c_str());
   //if (mf) MappedFile::closeMappedFile(mf);
   //if (mfForDebugInfo) MappedFile::closeMappedFile(mfForDebugInfo);
}	

bool Symtab::exportXML(string file)
{
#if defined (cap_serialization)
   try 
   {
      SerializerXML sb("XMLTranslator", file, sd_serialize, true);
      serialize(&sb, "Symtab");
#if 0
      SymtabTranslatorXML trans(this, file);
      if ( serialize(*this, trans))
         return true;
#endif
   } 
   catch (const SerializerError &err) 
   {
      fprintf(stderr, "%s[%d]: error serializing xml: %s\n", FILE__, __LINE__, err.what());
      return false;
   }

   return false;
#else
   fprintf(stderr, "%s[%d]:  WARNING:  cannot produce %s, serialization not available\n", FILE__, __LINE__, file.c_str());
   return false;
#endif
}

#if defined (cap_serialization)
bool Symtab::exportBin(string file)
{
   try
   {
      //  This needs some work (probably want to do object cacheing and retrieval)
      SerializerBin sb("BinSerializer", file, sd_serialize, true);
      serialize(&sb, "Symtab");

#if 0 
      bool verbose = false;
      if (strstr(file.c_str(), "cache_ld")) verbose = true;
      SymtabTranslatorBin *transptr = SymtabTranslatorBin::getTranslator(this, file, sd_serialize, verbose);
      assert(transptr);
      SymtabTranslatorBin &trans = *transptr;
      if (serialize(*this, trans))
         return true;
#endif
      fprintf(stderr, "%s[%d]:  binary serialization ok\n", __FILE__, __LINE__);
      return true;
   }
   catch (const SerializerError &err)
   {
      if (err.code() == SerializerError::ser_err_disabled) 
      {
         fprintf(stderr, "%s[%d]:  WARN:  serialization is disabled for file %s\n",
               FILE__, __LINE__, file.c_str());
         return true;
      }
      else 
      {
         fprintf(stderr, "%s[%d]: %s\n\tfrom %s[%d], code %d\n", FILE__, __LINE__,
               err.what(), err.file().c_str(), err.line(), err.code());
      }
   }

   fprintf(stderr, "%s[%d]:  error doing binary serialization\n", __FILE__, __LINE__);
   return false;
}
#else
bool Symtab::exportBin(string) 
{
   fprintf(stderr, "%s[%d]:  WARNING:  serialization not available\n", FILE__, __LINE__);
   return false;
}
#endif

Symtab *Symtab::importBin(std::string file)
{
#if defined (cap_serialization)
   MappedFile *mf= MappedFile::createMappedFile(file);
   if (!mf) 
   {
      fprintf(stderr, "%s[%d]:  failed to map file %s\n", FILE__, __LINE__, file.c_str());
      return NULL;
   }

   Symtab *st = new Symtab(mf);

   try
   {
      bool verbose = false;
      if (strstr(file.c_str(), "ld-")) verbose = true;
      SerializerBin sb("BinTranslator", file, sd_deserialize, true);
      st->serialize(&sb);
#if 0
      SymtabTranslatorBin *transptr = SymtabTranslatorBin::getTranslator(st, file, sd_deserialize, verbose);
      assert(transptr);
      SymtabTranslatorBin &trans = *transptr;
      if (deserialize(*st, trans)) {
         fprintf(stderr, "%s[%d]:  deserialized '%s' from cache\n", FILE__, __LINE__, file.c_str());
         if (!st) fprintf(stderr, "%s[%d]:  FIXME:  no symtab\n", FILE__, __LINE__);
         return st;
      }
#endif
   }

   catch (const SerializerError &err)
   {
      if (err.code() == SerializerError::ser_err_disabled) 
      {
         fprintf(stderr, "%s[%d]:  WARN:  serialization is disabled for file %s\n",
               FILE__, __LINE__, file.c_str());
         return NULL;
      }

      fprintf(stderr, "%s[%d]: %s\n\tfrom: %s[%d]\n", FILE__, __LINE__,
            err.what(), err.file().c_str(), err.line());
   }


   fprintf(stderr, "%s[%d]:  error doing binary deserialization\n", __FILE__, __LINE__);
   delete st;
   return NULL;
#else
   fprintf(stderr, "%s[%d]:  WARNING:  cannot produce %s, serialization not available\n", FILE__, __LINE__, file.c_str());
   return NULL;
#endif
}

bool Symtab::openFile(Symtab *&obj, std::string filename)
{
   bool err = false;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif
   unsigned numSymtabs = allSymtabs.size();

   // AIX: it's possible that we're reparsing a file with better information
   // about it. If so, yank the old one out of the allSymtabs std::vector -- replace
   // it, basically.
   if ( filename.find("/proc") == std::string::npos)
   {
      for (unsigned u=0; u<numSymtabs; u++) 
      {
         assert(allSymtabs[u]);
         if (filename == allSymtabs[u]->file()) 
         {
            // return it
            obj = allSymtabs[u];
            return true;
         }
      }   
   }

#if defined (cap_serialization)
   obj = importBin(filename);

   if (!obj) 
   {
      fprintf(stderr, "%s[%d]:  importBin failed\n", FILE__, __LINE__);
   }
   else 
   {
      return true;
   }
#endif

   obj = new Symtab(filename, err);
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

      obj->setupTypes();	

#if defined (cap_serialization)
      fprintf(stderr, "%s[%d]:  doing bin-serialize for %s\n", 
            FILE__, __LINE__, filename.c_str());

      if (!obj->exportBin(filename))
      {
         fprintf(stderr, "%s[%d]:  failed to export symtab\n", FILE__, __LINE__);
      }
      else
         fprintf(stderr, "%s[%d]:  did bin-serialize for %s\n", 
               FILE__, __LINE__, filename.c_str());
#endif

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

bool Symtab::addRegion(Offset vaddr, void *data, unsigned int dataSize, std::string name, Region::RegionType rType_, bool loadable)
{
   Region *sec;
   unsigned i;
   if (loadable)
   {
      sec = new Region(newSectionInsertPoint, name, vaddr, dataSize, vaddr, 
            dataSize, (char *)data, Region::RP_R, rType_, true);

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
            (char *)data, Region::RP_R, rType_);
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
         fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return false;
      }
   }

   if (!user_regions)
   {
      fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
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
         fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
         return false;
      }
   }
   if (!user_types)
   {
      fprintf(stderr, "%s[%d]:  failed to addAnnotation here\n", FILE__, __LINE__);
      return false;
   }

   user_types->push_back(t);

   return true;
}

bool Symtab::addRegion(Region *sec)
{
   regions_.push_back(sec);
   std::sort(regions_.begin(), regions_.end(), sort_reg_by_addr);
   addUserRegion(sec);
   return true;
}

void Symtab::parseLineInformation()
{
   dyn_hash_map<std::string, LineInformation> *lineInfo = new dyn_hash_map <std::string, LineInformation>;

   Object *linkedFile = getObject();
   linkedFile->parseFileLineInfo(*lineInfo);

   isLineInfoValid_ = true;	
   dyn_hash_map <std::string, LineInformation>::iterator iter;

   for (iter = lineInfo->begin(); iter!=lineInfo->end(); iter++)
   {
      Module *mod = NULL;
      if (findModuleByName(mod, iter->first))
      {
         mod->setLineInfo(&(iter->second));
      }
      else if (findModuleByName(mod, mf->filename()))
      {
         LineInformation *lineInformation = mod->getLineInformation();
         if (!lineInformation) 
         {
            mod->setLineInfo(&(iter->second));
         } 
         else 
         {
            lineInformation->addLineInfo(&(iter->second));
            mod->setLineInfo(lineInformation);
         }
      }
      else {
	object_printf("[%s:%u] - Couldn't find module %s to go with line info\n",
		     __FILE__, __LINE__, iter->first.c_str()); 
      }
   }
}

SYMTAB_EXPORT bool Symtab::getAddressRanges(std::vector<pair<Offset, Offset> >&ranges,
      std::string lineSource, unsigned int lineNo)
{
   unsigned int originalSize = ranges.size();

   /* Iteratate over the modules, looking for ranges in each. */

   for ( unsigned int i = 0; i < _mods.size(); i++ ) 
   {
      LineInformation *lineInformation = _mods[i]->getLineInformation();

      if (lineInformation)
         lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );

   } /* end iteration over modules */

   if ( ranges.size() != originalSize )
      return true;

   return false;
}

SYMTAB_EXPORT bool Symtab::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
   unsigned int originalSize = lines.size();

   /* Iteratate over the modules, looking for ranges in each. */
   for ( unsigned int i = 0; i < _mods.size(); i++ ) 
   {
      LineInformation *lineInformation = _mods[i]->getLineInformation();

      if (lineInformation)
         lineInformation->getSourceLines( addressInRange, lines );

   } /* end iteration over modules */

   if ( lines.size() != originalSize )
      return true;

   return false;

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


void Symtab::parseTypes()
{
   Object *linkedFile = getObject();
   linkedFile->parseTypeInfo(this);
   isTypeInfoValid_ = true;
}

bool Symtab::addType(Type *type)
{
   if (!addUserType(type))
   {
      fprintf(stderr, "%s[%d]:  failed to addUserType\n", FILE__, __LINE__);
   }

   typeCollection *globaltypes = typeCollection::getGlobalTypeCollection();
   globaltypes->addType(type);

   return true;
}

SYMTAB_EXPORT vector<Type *> *Symtab::getAllstdTypes()
{
   setupStdTypes();
   return stdTypes->getAllTypes(); 	
}

SYMTAB_EXPORT vector<Type *> *Symtab::getAllbuiltInTypes()
{
   setupStdTypes();
   return builtInTypes->getAllBuiltInTypes();
}

SYMTAB_EXPORT bool Symtab::findType(Type *&type, std::string name)
{
   parseTypesNow();

   if (!_mods.size())
      return false;

   type = _mods[0]->getModuleTypes()->findType(name);

   if (type == NULL)
      return false;

   return true;	
}

SYMTAB_EXPORT bool Symtab::findVariableType(Type *&type, std::string name)
{
   parseTypesNow();

   if (!_mods.size())
      return false;

   type = _mods[0]->getModuleTypes()->findVariableType(name);

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

bool Symtab::setDefaultNamespacePrefix(string &str)
{
   defaultNamespacePrefix = str;
   return true;
}

SYMTAB_EXPORT bool Symtab::emitSymbols(Object *linkedFile,std::string filename, unsigned flag)
{
    // Start with all the defined symbols
    std::vector<Symbol *> allSyms;
    for (unsigned i = 0; i < everyDefinedSymbol.size(); i++) {
        allSyms.push_back(everyDefinedSymbol[i]);
    }

    // Add the undefined dynamic symbols
    map<string, std::vector<Symbol *> >::iterator iter;
    std::vector<Symbol *>::iterator siter;

    for (iter = undefDynSyms.begin(); iter != undefDynSyms.end(); iter++)
        for (siter=iter->second.begin(); siter != iter->second.end(); siter++)
            allSyms.push_back(*siter);

    // Write the new file
    return linkedFile->emitDriver(this, filename, allSyms, flag);
}

SYMTAB_EXPORT bool Symtab::emit(std::string filename, unsigned flag)
{
    return emitSymbols(getObject(), filename, flag);
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

SYMTAB_EXPORT bool Symtab::updateCode(void *buffer, unsigned size)
{
   Region *sec;

   if (!findRegion(sec, ".text"))
      return false;

   sec->setPtrToRawData(buffer, size);

   return true;
}

SYMTAB_EXPORT bool Symtab::updateData(void *buffer, unsigned size)
{
   Region *sec;

   if (!findRegion(sec, ".data"))
      return false;

   sec->setPtrToRawData(buffer, size);

   return true;
}

SYMTAB_EXPORT Offset Symtab::getFreeOffset(unsigned size) 
{
   // Look through sections until we find a gap with
   // sufficient space.
   Offset highWaterMark = 0;
   Offset secoffset = 0;
   Offset prevSecoffset = 0;

   Object *linkedFile = getObject();
   assert(linkedFile);

   for (unsigned i = 0; i < regions_.size(); i++) 
   {
      Offset end = regions_[i]->getRegionAddr() + regions_[i]->getDiskSize();

      if (regions_[i]->getRegionAddr() == 0) 
         continue;

      prevSecoffset = secoffset;

      unsigned region_offset = (unsigned)((char *)(regions_[i]->getPtrToRawData())
            - linkedFile->mem_image());

      if (region_offset < (unsigned)prevSecoffset)
      {
         secoffset += regions_[i]->getDiskSize();
      }
      else 
      {
         secoffset = (char *)(regions_[i]->getPtrToRawData()) - linkedFile->mem_image();
         secoffset += regions_[i]->getDiskSize();
      }

      /*fprintf(stderr, "%d: secAddr 0x%lx, size %d, end 0x%lx, looking for %d\n",
        i, regions_[i]->getSecAddr(), regions_[i]->getSecSize(),
        end,size);*/

      if (end > highWaterMark) 
      {
         //fprintf(stderr, "Increasing highWaterMark...\n");
         newSectionInsertPoint = i+1;
         highWaterMark = end;
      }

      if (     (i < (regions_.size()-2)) 
            && ((end + size) < regions_[i+1]->getRegionAddr())) 
      {
         /*      fprintf(stderr, "Found a hole between sections %d and %d\n",
                 i, i+1);
                 fprintf(stderr, "End at 0x%lx, next one at 0x%lx\n",
                 end, regions_[i+1]->getSecAddr());
          */   
         newSectionInsertPoint = i+1;
         highWaterMark = end;
         break;
      }
   }

   //   return highWaterMark;

   unsigned pgSize = P_getpagesize();
   Offset newaddr = highWaterMark - (highWaterMark & (pgSize-1)) + (secoffset & (pgSize-1));

   if (newaddr < highWaterMark)
      newaddr += pgSize;

   return newaddr;
}

SYMTAB_EXPORT ObjectType Symtab::getObjectType() const 
{
   return object_type_;
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

SYMTAB_EXPORT unsigned Symtab::getNumberofRegions() const 
{
   return no_of_sections; 
}

SYMTAB_EXPORT unsigned Symtab::getNumberofSymbols() const 
{
   return no_of_symbols; 
}

bool Symtab::setup_module_up_ptrs(SerializerBase *, Symtab *st)
{
   std::vector<Module *> &mods = st->_mods;

   for (unsigned int i = 0; i < mods.size(); ++i) 
   {
      Module *m = mods[i];
      m->exec_ = st;
   }

   return true;
}

bool Symtab::fixup_relocation_symbols(SerializerBase *, Symtab *st)
{
   std::vector<Module *> &mods = st->_mods;

   for (unsigned int i = 0; i < mods.size(); ++i) 
   {
      Module *m = mods[i];
      m->exec_ = st;
   }

   return true;
}

void Symtab::serialize(SerializerBase *sb, const char *tag)
{
   try 
   {
      ifxml_start_element(sb, tag);
      gtranslate(sb, imageOffset_, "imageOffset");
      gtranslate(sb, imageLen_, "imageLen");
      gtranslate(sb, dataOffset_, "dataOff");
      gtranslate(sb, dataLen_, "dataLen");
      gtranslate(sb, is_a_out, "isExec");
      gtranslate(sb, _mods, "Modules", "Module");
      //gtranslate(sb, everyUniqueFunction, "EveryUniqueFunction", "UniqueFunction");
      //gtranslate(sb, everyUniqueVariable, "EveryUniqueVariable", "UniqueVariable");
      //gtranslate(sb, modSyms, "ModuleSymbols", "ModuleSymbol");
      gtranslate(sb, excpBlocks, "ExceptionBlocks", "ExceptionBlock");
      ifxml_end_element(sb, tag);


      ifinput(Symtab::setup_module_up_ptrs, sb, this);
      ifinput(fixup_relocation_symbols, sb, this);

      //  Patch up module's exec_ (pointer to Symtab) at a higher level??
      //if (getSD().iomode() == sd_deserialize)
      //   param.exec_ = parent_symtab;
   } SER_CATCH("Symtab");
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
: tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) 
{
}

   SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(Offset cStart) 
: tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) 
{
}

SYMTAB_EXPORT ExceptionBlock::ExceptionBlock(const ExceptionBlock &eb) :
   Serializable(),
   tryStart_(eb.tryStart_), trySize_(eb.trySize_), 
   catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) 
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

void ExceptionBlock::serialize(SerializerBase *sb, const char *tag)
{
   try 
   {
      ifxml_start_element(sb, tag);
      gtranslate(sb, tryStart_, "tryStart");
      gtranslate(sb, trySize_, "trySize");
      gtranslate(sb, catchStart_, "catchStart");
      gtranslate(sb, hasTry_, "hasTry");
      ifxml_end_element(sb, tag);
   } SER_CATCH("Symtab");
}


SYMTAB_EXPORT relocationEntry::relocationEntry() :
   target_addr_(0), 
   rel_addr_(0), 
   addend_(0), 
   rtype_(Region::RT_REL), 
   name_(""), 
   dynref_(NULL), 
   relType_(0)
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
   relType_(relType)
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
   relType_(relType)
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
   relType_(relType)
{
}   

SYMTAB_EXPORT const relocationEntry& relocationEntry::operator=(const relocationEntry &ra) 
{
   target_addr_ = ra.target_addr_;
   rel_addr_ = ra.rel_addr_;
   addend_ = ra.addend_;
   rtype_ = ra.rtype_;
   name_ = ra.name_; 
   dynref_ = ra.dynref_;
   relType_ = ra.relType_;
   return *this;
}

SYMTAB_EXPORT void relocationEntry::setAddend(const Offset value) {
    addend_ = value;
}

SYMTAB_EXPORT Offset relocationEntry::addend() const {
    return addend_;
}

SYMTAB_EXPORT void relocationEntry::setRegionType(const Region::RegionType value) {
    rtype_ = value;
}

SYMTAB_EXPORT Region::RegionType relocationEntry::regionType() const {
	return rtype_;
}

void relocationEntry::serialize(SerializerBase *sb, const char *tag)
{
   try 
   {
      ifxml_start_element(sb, tag);
      gtranslate(sb, target_addr_, "targetAddress");
      gtranslate(sb, rel_addr_, "relocationAddress");
      gtranslate(sb, name_, "relocationName");
      gtranslate(sb, relType_, "relocationType");
      //  deserialize: Re-assign dynref_ symbol elsewhere (in Symtab class)
      ifxml_end_element(sb, tag);
   } SER_CATCH("relocationEntry");
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
      CASE_RETURN_STR(ST_NOTYPE);
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


Object *Symtab::getObject()
{
   if (obj_private)
      return obj_private;

   //TODO: This likely triggered because we serialized in an object
   // from cache, but now the user is requesting more information from
   // the on disk object.  We should create a new 'Object' from data
   // (likely a file path) serialized in.
   
   assert(0);
   //obj_private = new Object();
   return obj_private;
}

#if defined (cap_serialization)
//  Not sure this is strictly necessary, problems only seem to exist with Module 
// annotations when the file was split off, so there's probably something else that
//  can be done to instantiate the relevant functions.

bool dummy_for_ser_instance(std::string file, SerializerBase *sb)
{
   if (file == std::string("no_such_file")) 
   {
      if (!sb) 
      {
         fprintf(stderr, "%s[%d]:  really should not happen\n", FILE__, __LINE__);
         return false;
      }
#if 0
      bool r = false;
      const char *sbb = "no_name_dummy";
      r = init_anno_serialization<Dyninst::SymtabAPI::localVarCollection, symbol_parameters_a >(sbb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for symbol_params\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::localVarCollection, symbol_variables_a>(sbb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for symbol_vars\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::LineInformation *, module_line_info_a>(sbb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for module_line_info\n", FILE__, __LINE__);}
      r = false;
      r = init_anno_serialization<Dyninst::SymtabAPI::typeCollection *, module_type_info_a>(sbb);
      if (!r) {fprintf(stderr, "%s[%d]:  failed to init anno serialize for module_type_info\n", FILE__, __LINE__);}
      r = false;
#else
      fprintf(stderr, "%s[%d]:  WARN:  disabled serializer init here\n", FILE__, __LINE__);
#endif
   }
   return true;
}
#endif

