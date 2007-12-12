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

#include "common/h/debugOstream.h"
#include "common/h/Timer.h"
#include "symtabAPI/src/Object.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/src/Collections.h"

#if !defined(os_windows)
#include "common/h/pathName.h"
#include <dlfcn.h>
#else
#include "windows.h"
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif

#include <libxml/xmlwriter.h>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

static std::string errMsg;
extern bool parseCompilerType(Object *);
bool regexEquiv( const std::string &str,const std::string &them, bool checkCase );
bool pattern_match( const char *p, const char *s, bool checkCase );
void pd_log_perror(const char *msg){
   errMsg = msg;
};

#if !defined(os_windows)
    //libxml2 functions
	void *hXML;
#else
	HINSTANCE hXML; 
#endif

xmlTextWriterPtr(*my_xmlNewTextWriterFilename)(const char *,int) = NULL; 
int(*my_xmlTextWriterStartDocument)(xmlTextWriterPtr, const char *, const char *, const char * ) = NULL;
int(*my_xmlTextWriterStartElement)(xmlTextWriterPtr, const xmlChar *) = NULL;
int(*my_xmlTextWriterWriteFormatElement)(xmlTextWriterPtr,const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndDocument)(xmlTextWriterPtr) = NULL;
void(*my_xmlFreeTextWriter)(xmlTextWriterPtr) = NULL;
int(*my_xmlTextWriterWriteFormatAttribute)(xmlTextWriterPtr, const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndElement)(xmlTextWriterPtr) = NULL;


// generateXML helper functions
bool generateXMLforSyms(xmlTextWriterPtr &writer, std::vector<Symbol *> &everyUniqueFunction, std::vector<Symbol *> &everyUniqueVariable, std::vector<Symbol *> &modSyms, std::vector<Symbol *> &notypeSyms);
bool generateXMLforSymbol(xmlTextWriterPtr &writer, Symbol *sym);
bool generateXMLforExcps(xmlTextWriterPtr &writer, std::vector<ExceptionBlock *> &excpBlocks);
bool generateXMLforRelocations(xmlTextWriterPtr &writer, std::vector<relocationEntry> &fbt);
bool generateXMLforModules(xmlTextWriterPtr &writer, std::vector<Module *> &mods);


static SymtabError serr;

std::vector<Symtab *> Symtab::allSymtabs;
builtInTypeCollection *Symtab::builtInTypes = NULL;
typeCollection *Symtab::stdTypes = NULL;
 
SymtabError Symtab::getLastSymtabError(){
    return serr;
}

std::string Symtab::printError(SymtabError serr)
{
    switch (serr){
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
        case No_Such_Section:
            return "Section does not exist";
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

void Symtab::setupTypes(){
    /*
     * Create the "error" and "untyped" types.
     */
    std::string name = "<error>";
    type_Error   = Type::createFake(name);
    name = "<no type>";
    type_Untyped = Type::createFake(name);
    setupStdTypes();
}    

void Symtab::setupStdTypes() {
    if(builtInTypes)
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
	if(stdTypes)
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
    newType->decrRefCount();
    Type *voidType = new typeScalar(-11, 0, "void", false);
    stdTypes->addType(voidType);
	tName = "void *";
    stdTypes->addType(newPtrType = new typePointer(-4, voidType, tName));
    voidType->decrRefCount();
    newType->decrRefCount();
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

DLLEXPORT unsigned Symtab::getAddressWidth() const {
    return linkedFile->getAddressWidth();
}
 
DLLEXPORT bool Symtab::isNativeCompiler() const {
    return nativeCompiler; 
}
 
DLLEXPORT Symtab::Symtab(){
}

DLLEXPORT bool Symtab::isExec() const {
    return is_a_out; 
}

DLLEXPORT Offset Symtab::imageOffset() const { 
    return imageOffset_;
}

DLLEXPORT Offset Symtab::dataOffset() const { 
    return dataOffset_;
}

DLLEXPORT Offset Symtab::dataLength() const { 
    return dataLen_;
} 

DLLEXPORT Offset Symtab::imageLength() const { 
    return imageLen_;
}
 
DLLEXPORT char* Symtab::image_ptr ()  const {
    return (char *)linkedFile->code_ptr(); 
}

DLLEXPORT char* Symtab::data_ptr ()  const { 
    return (char *)linkedFile->data_ptr();
}

DLLEXPORT const char*  Symtab::getInterpreterName() const {
    return linkedFile->interpreter_name();
}
 
DLLEXPORT Offset Symtab::getEntryOffset() const { 
    return linkedFile->getEntryAddress(); 
}

DLLEXPORT Offset Symtab::getBaseOffset() const {
    return linkedFile->getBaseAddress(); 
}

DLLEXPORT Offset Symtab::getLoadOffset() const { 
    return linkedFile->getLoadAddress(); 
}

DLLEXPORT Offset Symtab::getTOCoffset() const { 
    return linkedFile->getTOCoffset(); 
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

    if((mangled.length()>5) && (mangled.substr(0,5)==std::string("MPI__"))) { 
        return false;
    }	  

    /* If it's Fortran, eliminate the trailing underscores, if any. */
    if(lang == lang_Fortran 
      || lang == lang_CMFortran 
      || lang == lang_Fortran_with_pretty_debug )
    {
        if( mangled[ mangled.length() - 1 ] == '_' ) 
	{
            char * demangled = strdup( mangled.c_str() );
            demangled[ mangled.length() - 1 ] = '\0';
            pretty = std::string( demangled );
          
            free( demangled );
            return true;
        }
        else {
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
        char msg[256];
        sprintf(msg, "%s[%d]: 'demangling' versioned symbol: %s, to %s",
                  __FILE__, __LINE__, mangled.c_str(), pretty.c_str());
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
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool Symtab::symbolsToFunctions(std::vector<Symbol *> *raw_funcs) 
{

#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

    std::vector< Symbol *> lookUps;
    std::string symString;
    is_a_out = linkedFile->is_aout();

    // JAW 02-03 -- restructured below slightly to get rid of multiple loops
    // through entire symbol list
  
    // find the real functions -- those with the correct type in the symbol table
    for(SymbolIter symIter(*linkedFile); symIter;symIter++) 
    {
        Symbol *lookUp = symIter.currval();
        const char *np = lookUp->getName().c_str();

        //parsing_printf("Scanning file: symbol %s\n", lookUp->getName().c_str());

        //fprintf(stderr,"np %s\n",np);

        if (linkedFile->isEEL() && np[0] == '.')
        /* ignore these EEL symbols; we don't understand their values */
            continue; 
        
        // check for undefined dynamic symbols. Used when rewriting relocation section.
        // relocation entries have references to these undefined dynamic symbols.
        if(lookUp->getSec() == NULL && lookUp->isInDynSymtab()) {
            undefDynSyms[np] = lookUp;
            continue;
        }

    	if (lookUp->getType() == Symbol::ST_FUNCTION) 
	    {
            // /* DEBUG */ fprintf( stderr, "%s[%d]: considering function symbol %s in module %s\n", FILE__, __LINE__, lookUp.getName().c_str(), lookUp.getModuleName().c_str() );
            
            std::string msg;
            char tempBuffer[40];
            if (!isValidOffset(lookUp->getAddr())) 
            {
                sprintf(tempBuffer,"0x%lx",lookUp->getAddr());
                msg = std::string("Function ") + lookUp->getName() + std::string(" has bad address ")
                                                                            + std::string(tempBuffer);
                return false;
            }
            // Fill in _mods.
            Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
            delete(lookUp->getModule());
            lookUp->setModule(newMod);
            raw_funcs->push_back(lookUp);
        }
        if(lookUp->getType() == Symbol::ST_MODULE)
	{
	   const std::string mangledName = symIter.currkey();
           char * unmangledName =
                    P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
            if (unmangledName)
                lookUp->addPrettyName(unmangledName, true);
            else
                lookUp->addPrettyName(mangledName, true);
            Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
            delete(lookUp->getModule());
            lookUp->setModule(newMod);
            addModuleName(lookUp,mangledName);
            modSyms.push_back(lookUp);
	}
	else if(lookUp->getType() == Symbol::ST_NOTYPE)
        {
            const std::string mangledName = symIter.currkey();
            char * unmangledName =
            P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
            if (unmangledName)
                lookUp->addPrettyName(unmangledName, true);
            else
                lookUp->addPrettyName(mangledName, true);
            notypeSyms.push_back(lookUp);
        }	
	else if(lookUp->getType() == Symbol::ST_OBJECT)
	{
            const std::string mangledName = symIter.currkey();
#if 0
         fprintf(stderr, "Symbol %s, mod %s, addr 0x%x, type %d, linkage %d (obj %d, func %d)\n",
                 symInfo.name().c_str(),
                 symInfo.module().c_str(),
                 symInfo.addr(),
                 symInfo.type(),
                 symInfo.linkage(),
                 Symbol::ST_OBJECT,
                 Symbol::ST_FUNCTION);
#endif
#if !defined(os_windows)
         // Windows: variables are created with an empty module
            if (lookUp->getModuleName().length() == 0) 
            {
                //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
                continue;
            }
#endif
            char * unmangledName =
                    P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
                            
            //Fill in _mods.
            Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
            delete(lookUp->getModule());
            lookUp->setModule(newMod);
            Symbol *var;
            //bool addToPretty = false;
            if (varsByAddr.find(lookUp->getAddr())!=varsByAddr.end()) 
            {
                var = varsByAddr[lookUp->getAddr()];
                                    
                // Keep the new mangled name
                var->addMangledName(mangledName);
                if (unmangledName)
                var->addPrettyName(unmangledName, true);
                else
                var->addPrettyName(mangledName, true);
                                    
            }
            else
            {
                var = lookUp;
                varsByAddr[lookUp->getAddr()] = var;
                if (unmangledName)
                    var->addPrettyName(unmangledName, true);
                else
                    var->addPrettyName(mangledName, true);
                everyUniqueVariable.push_back(var);
            }
        }

    }

#if defined(TIMED_PARSE)
  	struct timeval endtime;
  	gettimeofday(&endtime, NULL);
  	unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  	unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  	unsigned long difftime = lendtime - lstarttime;
  	double dursecs = difftime/(1000 );
  	cout << __FILE__ << ":" << __LINE__ <<": symbolsToFunctions took "<<dursecs <<" msecs" << endl;
#endif
    return true;
}

#if defined(ppc64_linux)
/* Special case for ppc64 ELF binaries. Sometimes a function has a 3-byte descriptor symbol
 * along with it in the symbol table and "." preceding its original pretty name for the correct
 * function symbol. This checks to see if we have a corresponding 3-byte descriptor symbol existing
 * and if it does we remove the preceding "." from the name of the symbol
 */

void Symtab::checkPPC64DescriptorSymbols(){
     // find the real functions -- those with the correct type in the symbol table
     for(SymbolIter symIter(*linkedFile); symIter;symIter++)
     {
         Symbol *lookUp = symIter.currval();
         const char *np = lookUp->getName().c_str();
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
void Symtab::setModuleLanguages(hash_map<std::string, supportedLanguages> *mod_langs)
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
        if (currmod->isShared())
            continue;  // need to find some way to get shared object languages?
        if(mod_langs->find(currmod->fileName()) != mod_langs->end())
        {
            currLang = (*mod_langs)[currmod->fileName()];
                            currmod->setLanguage(currLang);
        }
        else
        {
            //cerr << __FILE__ << __LINE__ << ":  module " << currmod->fileName() 
            //      				   << " not found in module<->language map" << endl;
            //dump = 1;
            //  here we should probably try to guess, based on filename conventions
        }
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

    Module *fm = new Module();
    if (findModule(fm, nameToUse))
        return fm;
    delete fm;

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
    if (findModule(ret, name)) {
        return(ret);
    }

    //parsing_printf("=== image, creating new pdmodule %s, addr 0x%x\n",
    //				name.c_str(), addr);
    
    std::string fileNm, fullNm;
    fullNm = name;
    fileNm = extract_pathname_tail(name);

    // /* DEBUG */ fprintf( stderr, "%s[%d]: Creating new pdmodule '%s'/'%s'\n", FILE__, __LINE__, fileNm.c_str(), fullNm.c_str() );
    ret = new Module(lang, addr, fullNm, this);
    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    _mods.push_back(ret);
    
    return(ret);
}

 
//buildFunctionLists() iterates through image_funcs and constructs demangled 
//names. Demangling was moved here (names used to be demangled as image_funcs 
//were built) so that language information could be obtained _after_ the 
//functions and modules were built, but before name demangling takes place.  
//Thus we can use language information during the demangling process.

bool Symtab::buildFunctionLists(std::vector <Symbol *> &raw_funcs)
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    for (unsigned int i = 0; i < raw_funcs.size(); i++) 
    {
        Symbol *raw = raw_funcs[i];
        Module *rawmod = getOrCreateModule(raw->getModuleName(),raw->getAddr());
            
        assert(raw);
        assert(rawmod);
            
        // At this point we need to generate the following information:
        // A symtab name.
        // A pretty (demangled) name.
        // The symtab name goes in the global list as well as the module list.
        // Same for the pretty name.
        // Finally, check addresses to find aliases.
            
        std::string mangled_name = raw->getName();
        std::string working_name = mangled_name;
                
        std::string pretty_name = "<UNSET>";
        std::string typed_name = "<UNSET>";
#if !defined(os_windows)        
        //Remove extra stabs information
        const char *p = strchr(working_name.c_str(), ':');
        if( p )
        {
            unsigned nchars = p - mangled_name.c_str();
            working_name = std::string(mangled_name.c_str(), nchars);
        }
#endif        
        if (!buildDemangledName(working_name, pretty_name, typed_name,
                                nativeCompiler, rawmod->language())) 
        {
            pretty_name = working_name;
        }
        
        //parsing_printf("%s: demangled %s, typed %s\n",
        //	       mangled_name.c_str(),
        //	       pretty_name.c_str(),
        //	       typed_name.c_str());
            
        // Now, we see if there's already a function object for this
        // address. If so, add a new name; 
        Symbol *possiblyExistingFunction = NULL;
        //funcsByEntryAddr.find(raw->getAddr(), possiblyExistingFunction);
        if (funcsByEntryAddr.find(raw->getAddr())!=funcsByEntryAddr.end()) 
        {
            std::vector<Symbol *> &funcs = funcsByEntryAddr[raw->getAddr()];
            unsigned flag = 0;
            for(unsigned int j=0;j<funcs.size();j++)
            {
                possiblyExistingFunction = funcsByEntryAddr[raw->getAddr()][j];
                // On some platforms we see two symbols, one in a real module
                // and one in DEFAULT_MODULE. Replace DEFAULT_MODULE with
                // the real one
                Module *use = getOrCreateModule(possiblyExistingFunction->getModuleName(),
                                                            possiblyExistingFunction->getAddr());
                if(!(*rawmod == *use))
                {
                    if (rawmod->fileName() == "DEFAULT_MODULE")
                        rawmod = use;
                    if(use->fileName() == "DEFAULT_MODULE") 
                    {
                        possiblyExistingFunction->setModuleName(std::string(rawmod->fullName()));
                        use = rawmod; 
                    }
                }
            
                if(*rawmod == *use)
                {
                    // Keep the new mangled name
                    possiblyExistingFunction->addMangledName(mangled_name);
                    if (pretty_name != "<UNSET>")
                        possiblyExistingFunction->addPrettyName(pretty_name);
                    if (typed_name != "<UNSET>")
                        possiblyExistingFunction->addTypedName(typed_name);
                    raw_funcs[i] = NULL;
                    delete raw; // Don't leak
                    flag = 1;	
                    break;
                }	
            }	
            if(!flag)
            {
                funcsByEntryAddr[raw->getAddr()].push_back(raw);
                addFunctionName(raw, mangled_name, true);
                if (pretty_name != "<UNSET>")
                    raw->addPrettyName(pretty_name, true);
                if (typed_name != "<UNSET>")
                    raw->addTypedName(typed_name, true);
            }
        }
        else
        {
            funcsByEntryAddr[raw->getAddr()].push_back(raw);
            addFunctionName(raw, mangled_name, true);
            if (pretty_name != "<UNSET>")
                                    raw->addPrettyName(pretty_name, true);
            if (typed_name != "<UNSET>")
                raw->addTypedName(typed_name, true);
                
        }
    }
    
    // Now that we have a 1) unique and 2) demangled list of function
    // names, loop through once more and build the address range tree
    // and name lookup tables. 
    for (unsigned j = 0; j < raw_funcs.size(); j++) 
    {
        Symbol *func = raw_funcs[j];
        if (!func) continue;
        
        // May be NULL if it was an alias.
        enterFunctionInTables(func, true);
    }
    
    // Conspicuous lack: inst points. We're delaying.
#if defined(TIMED_PARSE)
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
    unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
    unsigned long difftime = lendtime - lstarttime;
    double dursecs = difftime/(1000 );
    cout << __FILE__ << ":" << __LINE__ <<": buildFunction Lists took "<<dursecs <<" msecs" << endl;
#endif
    return true;
} 

// Enter a function in all the appropriate tables
void Symtab::enterFunctionInTables(Symbol *func, bool wasSymtab)
{
    if (!func)
        return;
    
    funcsByEntryAddr[func->getAddr()].push_back(func);
    // Functions added during symbol table parsing do not necessarily
    // have valid sizes set, and should therefor not be added to
    // the code range tree. They will be added after parsing. 
    /*if(!wasSymtab)
        {
        // TODO: out-of-line insertion here
        //if (func->get_size_cr())
        //	funcsByRange.insert(func);
        }*/
    
    everyUniqueFunction.push_back(func);
    if (wasSymtab)
        exportedFunctions.push_back(func);
    else
        createdFunctions.push_back(func);
}

bool Symtab::addSymbol(Symbol *newSym, bool isDynamic)
{
    if(!newSym)
    	return false;
    if(isDynamic) {
        newSym->clearIsInSymtab();
        newSym->setDynSymtab();
    }	
    std::vector<std::string> names;
    char *unmangledName = NULL;
    std::string sname = newSym->getName();
#if !defined(os_windows)
    // Windows: variables are created with an empty module
    if (newSym->getModuleName().length() == 0) 
    {
        //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
        return false;
    }
#endif
    Module *newMod = getOrCreateModule(sname, newSym->getAddr());
    delete(newSym->getModule());
    newSym->setModule(newMod);
    if(newSym->getAllPrettyNames().size() == 0)
        unmangledName = P_cplus_demangle(sname.c_str(), nativeCompiler,false);
    if(newSym->getType() == Symbol::ST_FUNCTION)
    {
        names = newSym->getAllMangledNames();
        for(unsigned i=0;i<names.size();i++)
            addFunctionName(newSym, names[i], true);
        names = newSym->getAllPrettyNames();
        for(unsigned i=0;i<names.size();i++)
            addFunctionName(newSym, names[i], false);
        names = newSym->getAllTypedNames();
        for(unsigned i=0;i<names.size();i++)
            addFunctionName(newSym, names[i], false);
	enterFunctionInTables(newSym,false);
    }
    else if(newSym->getType() == Symbol::ST_OBJECT)
    {
        names = newSym->getAllMangledNames();
        for(unsigned i=0;i<names.size();i++)
            addVariableName(newSym, names[i], true);
        names = newSym->getAllPrettyNames();
        for(unsigned i=0;i<names.size();i++)
            addVariableName(newSym, names[i], false);
        names = newSym->getAllTypedNames();
        for(unsigned i=0;i<names.size();i++)
            addVariableName(newSym, names[i], false);
	everyUniqueVariable.push_back(newSym);    
    }
    else if(newSym->getType() == Symbol::ST_MODULE)
    {
        names = newSym->getAllMangledNames();
        for(unsigned i=0;i<names.size();i++)
            addModuleName(newSym, names[i]);
        names = newSym->getAllPrettyNames();
        for(unsigned i=0;i<names.size();i++)
            addModuleName(newSym, names[i]);
        modSyms.push_back(newSym);
    }	
    else
        notypeSyms.push_back(newSym);
    if(newSym->getAllPrettyNames().size() == 0)		// add the unmangledName if there are no prettyNames
    {
        if(unmangledName)
            newSym->addPrettyName(unmangledName, true);
        else
            newSym->addPrettyName(sname,true);
    }		
    return true;
}

void Symtab::addFunctionName(Symbol *func,
                                 const std::string newName,
                                 bool isMangled /*=false*/)
{    
    // Ensure a std::vector exists
    if (isMangled == false) {
        if(funcsByPretty.find(newName)==funcsByPretty.end())
            funcsByPretty[newName] = new std::vector<Symbol *>;
        funcsByPretty[newName]->push_back(func);    
    }
    else {
        if(funcsByMangled.find(newName)==funcsByMangled.end())
            funcsByMangled[newName] = new std::vector<Symbol *>;
        funcsByMangled[newName]->push_back(func);    
    }
}

void Symtab::addVariableName(Symbol *var,
                                 const std::string newName,
                                 bool isMangled /*=false*/)
{    
   // Ensure a vector exists
    if (isMangled == false) {
        if(varsByPretty.find(newName)==varsByPretty.end())
            varsByPretty[newName] = new std::vector<Symbol *>;
        varsByPretty[newName]->push_back(var);    
    }
    else {
        if(varsByMangled.find(newName)==varsByMangled.end())
            varsByMangled[newName] = new std::vector<Symbol *>;
        varsByMangled[newName]->push_back(var);    
    }
}

 
void Symtab::addModuleName(Symbol *mod, const std::string newName)
{
    if(modsByName.find(newName)==modsByName.end())
        modsByName[newName] = new std::vector<Symbol *>;
    modsByName[newName]->push_back(mod);    
}

Symtab::Symtab(std::string &filename,bool &err)
   : filename_(filename), is_a_out(false), main_call_addr_(0),
     nativeCompiler(false), isLineInfoValid_(false), isTypeInfoValid_(false),
     type_Error(NULL), type_Untyped(NULL)
{
    linkedFile = new Object(filename, pd_log_perror);
    name_ = extract_pathname_tail(filename);
    err = extractInfo();
}

Symtab::Symtab(char *mem_image, size_t image_size, bool &err):
   is_a_out(false), 
   main_call_addr_(0),
   nativeCompiler(false),
   isLineInfoValid_(false),
   isTypeInfoValid_(false),
   type_Error(NULL), type_Untyped(NULL)
{
    linkedFile = new Object(mem_image, image_size, pd_log_perror);
    err = extractInfo();
}

#if defined(os_aix)
Symtab::Symtab(std::string &filename, std::string &member_name, Offset offset, 
                       bool &err)
   : filename_(filename), member_name_(member_name), is_a_out(false),
     main_call_addr_(0), nativeCompiler(false), isLineInfoValid_(false),
     isTypeInfoValid_(false), type_Error(NULL), type_Untyped(NULL)
{
    linkedFile = new Object(filename, member_name, offset, pd_log_perror);
    name_ = extract_pathname_tail(filename);
    err = extractInfo();
}
#else
Symtab::Symtab(std::string &, std::string &, Offset, bool &)
{
    assert(0);
}
#endif

#if defined(os_aix)
Symtab::Symtab(char *mem_image, size_t image_size, std::string &member_name,
                       Offset offset, bool &err)
   : member_name_(member_name), is_a_out(false), main_call_addr_(0),
     nativeCompiler(false), isLineInfoValid_(false), isTypeInfoValid_(false), 
     type_Error(NULL), type_Untyped(NULL)
{
    linkedFile = new Object(mem_image, image_size, member_name, offset, 
                                                           pd_log_perror);
    err = extractInfo();
}
#else 
Symtab::Symtab(char *, size_t, std::string &, Offset, bool &)
{
    assert(0);
}
#endif

bool Symtab::extractInfo()
{
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    bool err = true;
    imageOffset_ = linkedFile->code_off();
    dataOffset_ = linkedFile->data_off();

    imageLen_ = linkedFile->code_len();
    dataLen_ = linkedFile->data_len();
    
    codeValidStart_ = linkedFile->code_vldS();
    codeValidEnd_ = linkedFile->code_vldE();
    dataValidStart_ = linkedFile->data_vldS();
    dataValidEnd_ = linkedFile->data_vldE();

    if (0 == imageLen_ || 0 == linkedFile->code_ptr()) {
        // for AIX, code_ptr()==NULL is normal behavior
#if !defined(os_aix)
       if (0 == linkedFile->code_ptr()) {
          fprintf(stderr, "[%s][%d]WARNING: null code pointer in Symtab, possibly due to a missing .text section.\n",__FILE__,__LINE__);
          linkedFile->code_ptr_ = (char *) linkedFile->code_off();
       }
       else {
#endif
          serr = Obj_Parsing;
          return false;
#if !defined(os_aix)
        }
#endif
   }
	
  //  if (!imageLen_ || !linkedFile->code_ptr()) {
  //      serr = Obj_Parsing; 
  //      return false; 
   // }

    no_of_sections = linkedFile->no_of_sections();
    newSectionInsertPoint = no_of_sections;
    no_of_symbols = linkedFile->no_of_symbols();

    sections_ = linkedFile->getAllSections();
    for(unsigned index=0;index<sections_.size();index++)
        secsByEntryAddr[sections_[index]->getSecAddr()] = sections_[index];

	/* insert error check here. check if parsed */
#if 0
	std::vector <Symbol *> tmods;
	
  	SymbolIter symIter(linkedFile);

   for(;symIter;symIter++)
	{
      Symbol *lookUp = symIter.currval();
      if (lookUp.getType() == Symbol::ST_MODULE)
		{
         const std::string &lookUpName = lookUp->getName();
         const char *str = lookUpName.c_str();

         assert(str);
         int ln = lookUpName.length();
          
         // directory definition -- ignored for now
         if (str[ln-1] != '/')
            tmods.push_back(lookUp);
      }
   }
 
	// sort the modules by address
   //statusLine("sorting modules");
   //sort(tmods.begin(), tmods.end(), symbol_compare);
#if defined(TIMED_PARSE)
 	struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": extract mods & sort took "<<dursecs <<" msecs" << endl;
#endif
  
   // remove duplicate entries -- some .o files may have the same 
   // address as .C files.  kludge is true for module symbols that 
   // I am guessing are modules
  
   unsigned int num_zeros = 0;
   // must use loop+1 not mods.size()-1 since it is an unsigned compare
   //  which could go negative - jkh 5/29/95
   for (unsigned loop=0; loop < tmods.size(); loop++)
	{
      if (tmods[loop].getAddr() == 0)
			num_zeros++;
      if ((loop+1 < tmods.size()) && (tmods[loop].getAddr() == tmods[loop+1].getAddr()))
         tmods[loop+1] = tmods[loop];
      else
         uniq.push_back(tmods[loop]);
   }
   // avoid case where all (ELF) module symbols have address zero
   
   if (num_zeros == tmods.size())
      uniq.resize(0);

#endif //if 0

#if defined(os_solaris) || defined(os_aix) || defined(os_linux)
    // make sure we're using the right demangler
    
    nativeCompiler = parseCompilerType(linkedFile);
    //parsing_printf("isNativeCompiler: %d\n", nativeCompiler);
#endif
  	 
    // define all of the functions
    //statusLine("winnowing functions");

#if defined(ppc64_linux)
   checkPPC64DescriptorSymbols();
#endif
    
    // a vector to hold all created functions until they are properly classified
    std::vector<Symbol *> raw_funcs;
	
    // define all of the functions, this also defines all of the modules
    if (!symbolsToFunctions(&raw_funcs))
    {
        fprintf(stderr, "Error converting symbols to functions in file %s\n", filename_.c_str());
        err = false;
        serr = Syms_To_Functions;
        return false;
    }
    sort(raw_funcs.begin(),raw_funcs.end(),symbol_compare);
	
    // wait until all modules are defined before applying languages to
    // them we want to do it this way so that module information comes
    // from the function symbols, first and foremost, to avoid any
    // internal module-function mismatching.
            
    // get Information on the language each modules is written in
    // (prior to making modules)
    hash_map<std::string, supportedLanguages> mod_langs;
    linkedFile->getModuleLanguageInfo(&mod_langs);
    setModuleLanguages(&mod_langs);
	
    // Once languages are assigned, we can build demangled names (in
    // the wider sense of demangling which includes stripping _'s from
    // fortran names -- this is why language information must be
    // determined before this step).
    
    // Also identifies aliases (multiple names with equal addresses)

    if (!buildFunctionLists(raw_funcs))
    {
        fprintf(stderr, "Error building function lists in file %s\n", filename_.c_str());
        err = false;
        serr = Build_Function_Lists;
        return false;
    }
    
    //addSymtabVariables();
    linkedFile->getAllExceptions(excpBlocks);

    vector<relocationEntry >fbt;
    linkedFile->get_func_binding_table(fbt);
    for(unsigned i=0; i<fbt.size();i++) {
        if(undefDynSyms.find(fbt[i].name()) != undefDynSyms.end())
            fbt[i].addDynSym(undefDynSyms[fbt[i].name()]);
        relocation_table_.push_back(fbt[i]);
    }
    return true;
}

Symtab::Symtab(const Symtab& obj)
   : LookupInterface()
{
    filename_ = obj.filename_;
    member_name_ = obj.member_name_;
    name_ = obj.name_;
    imageOffset_ = obj.imageOffset_;
    imageLen_ = obj.imageLen_;
    dataOffset_ = obj.dataOffset_;
    dataLen_ = obj.dataLen_;

    codeValidStart_ = obj.codeValidStart_;
    codeValidEnd_ = obj.codeValidEnd_;
    dataValidStart_ = obj.dataValidStart_;
    dataValidEnd_ = obj.dataValidEnd_;
        
    isLineInfoValid_ = obj.isLineInfoValid_;
    isTypeInfoValid_ = obj.isTypeInfoValid_;

    is_a_out = obj.is_a_out;
    main_call_addr_ = obj.main_call_addr_; // address of call to main()
    
    nativeCompiler = obj.nativeCompiler;
    
    // data from the symbol table  //what should be done here??
    linkedFile = obj.linkedFile; 
	
    //sections
    no_of_sections = obj.no_of_sections;
    unsigned i;
    for(i=0;i<obj.sections_.size();i++)
        sections_.push_back(new Section(*(obj.sections_[i])));
    for(i=0;i<sections_.size();i++)
        secsByEntryAddr[sections_[i]->getSecAddr()] = sections_[i];
    
    //symbols
    no_of_symbols = obj.no_of_symbols;
    
    for(i=0;i<obj.everyUniqueFunction.size();i++)
        everyUniqueFunction.push_back(new Symbol(*(obj.everyUniqueFunction[i])));
    for(i=0;i<everyUniqueFunction.size();i++)
    {
        funcsByEntryAddr[everyUniqueFunction[i]->getAddr()].push_back(everyUniqueFunction[i]);
        addFunctionName(everyUniqueFunction[i],everyUniqueFunction[i]->getName(),true);
        addFunctionName(everyUniqueFunction[i],everyUniqueFunction[i]->getPrettyName(),false);
    }	
    
    for(i=0;i<obj.everyUniqueVariable.size();i++)
        everyUniqueVariable.push_back(new Symbol(*(obj.everyUniqueVariable[i])));
    for(i=0;i<everyUniqueVariable.size();i++)
    {
        varsByAddr[everyUniqueVariable[i]->getAddr()] = everyUniqueVariable[i];
        addVariableName(everyUniqueVariable[i],everyUniqueVariable[i]->getName(),true);
        addVariableName(everyUniqueVariable[i],everyUniqueVariable[i]->getPrettyName(),false);
    }

    for(i=0;i<obj._mods.size();i++)
        _mods.push_back(new Module(*(obj._mods[i])));
    for(i=0;i<_mods.size();i++)
    {
        modsByFileName[_mods[i]->fileName()] = _mods[i];
        modsByFullName[_mods[i]->fullName()] = _mods[i];
    }
    for(i=0;i<modSyms.size();i++)
        modSyms.push_back(new Symbol(*(modSyms[i])));
    
    for(i=0;i<notypeSyms.size();i++)
        notypeSyms.push_back(new Symbol(*(obj.notypeSyms[i])));
    
    for(i=0; i<relocation_table_.size();i++) {
        relocation_table_.push_back(relocationEntry(obj.relocation_table_[i]));
        undefDynSyms[obj.relocation_table_[i].name()] = relocation_table_[i].getDynSym();
    }
    
    for(i=0;i<excpBlocks.size();i++)
        excpBlocks.push_back(new ExceptionBlock(*(obj.excpBlocks[i])));
    setupTypes();
}

bool Symtab::findModule(Module *&ret, const std::string name)
{
    if (modsByFileName.find(name)!=modsByFileName.end()) 
    {
        ret = modsByFileName[name];
        return true;
    }
    else if (modsByFullName.find(name)!=modsByFullName.end()) 
    {
        ret = modsByFullName[name];
        return true;
    }
  
    serr = No_Such_Module;
    ret = NULL;
    return false;
}

bool Symtab::getAllSymbolsByType(std::vector<Symbol *> &ret, Symbol::SymbolType sType)
{
    if(sType == Symbol::ST_FUNCTION)
        return getAllFunctions(ret);
    else if(sType == Symbol::ST_OBJECT)
        return getAllVariables(ret);
    else if(sType == Symbol::ST_MODULE)
    {
        if(modSyms.size()>0)
        {
            ret =  modSyms;
            return true;
        }
        serr = No_Such_Symbol;
        return false;
                
    }
    else if(sType == Symbol::ST_NOTYPE)
    {
        if(notypeSyms.size()>0)
        {
            ret =  notypeSyms;
            return true;
        }
        serr = No_Such_Symbol;
        return false;
    }
    else
        return getAllSymbols(ret);
}
 
 
bool Symtab::getAllFunctions(std::vector<Symbol *> &ret)
{
    if(everyUniqueFunction.size() > 0)
    {
        ret = everyUniqueFunction;
        return true;
    }	
    serr = No_Such_Function;
    return false;
}
 
bool Symtab::getAllVariables(std::vector<Symbol *> &ret)
{
    if(everyUniqueVariable.size() > 0)
    {
        ret = everyUniqueVariable;
        return true;
    }	
    serr = No_Such_Variable;
    return false;
}

bool Symtab::getAllSymbols(std::vector<Symbol *> &ret)
{
    std::vector<Symbol *> temp;
    getAllFunctions(ret);
    getAllVariables(temp);
    ret.insert(ret.end(), temp.begin(), temp.end());
    temp.clear();
    for(unsigned i=0;i<modSyms.size();i++)
        ret.push_back(modSyms[i]);
    for(unsigned i=0;i<notypeSyms.size();i++)
        ret.push_back(notypeSyms[i]);
    if(ret.size() > 0)
        return true;
    serr = No_Such_Symbol;
    return false;
}

bool Symtab::getAllModules(std::vector<Module *> &ret)
{
    if(_mods.size()>0)
    {
        ret = _mods;
        return true;
    }	
    serr = No_Such_Module;
    return false;
}

bool Symtab::getAllSections(std::vector<Section *>&ret)
{
    if(sections_.size() > 0)
    {
        ret = sections_;
        return true;
    }
    return false;
}

bool Symtab::getAllNewSections(std::vector<Section *>&ret)
{
    if(newSections_.size() > 0) {
    	ret = newSections_;
	return true;
    }
    return false;
}

bool Symtab::getAllExceptions(std::vector<ExceptionBlock *> &exceptions)
{
    if(excpBlocks.size()>0)
    {
        exceptions = excpBlocks;
        return true;
    }	
    return false;
}

bool Symtab::findException(ExceptionBlock &excp, Offset addr)
{
    for(unsigned i=0; i<excpBlocks.size(); i++)
    {
        if(excpBlocks[i]->contains(addr))
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
 
bool Symtab::findFuncByEntryOffset(std::vector<Symbol *>& ret, const Offset entry)
{
    if(funcsByEntryAddr.find(entry)!=funcsByEntryAddr.end()) {
        ret = funcsByEntryAddr[entry];
        return true;
    }
    serr = No_Such_Function;
    return false;
}
 
bool Symtab::findSymbolByType(std::vector<Symbol *> &ret, const std::string name,
                                  Symbol::SymbolType sType, bool isMangled,
                                  bool isRegex, bool checkCase)
{
    if(sType == Symbol::ST_FUNCTION)
        return findFunction(ret, name, isMangled, isRegex, checkCase);
    else if(sType == Symbol::ST_OBJECT)
        return findVariable(ret, name, isMangled, isRegex, checkCase);
    else if(sType == Symbol::ST_MODULE)
        return findMod(ret, name, isMangled, isRegex, checkCase);
    else if(sType == Symbol::ST_NOTYPE)
    {
        unsigned start = ret.size(),i;
        if(!isMangled && !isRegex)
        {
            for(i=0;i<notypeSyms.size();i++)
            {
                if(notypeSyms[i]->getPrettyName() == name)
                    ret.push_back(notypeSyms[i]);
            }	    
        }
        else if(isMangled&&!isRegex)
        {
            for(i=0;i<notypeSyms.size();i++)
            {
                if(notypeSyms[i]->getName() == name)
                    ret.push_back(notypeSyms[i]);
            }	
        }
        else if(!isMangled&&isRegex)
        {
            for(i=0;i<notypeSyms.size();i++)
            {
                if(regexEquiv(name, notypeSyms[i]->getPrettyName(), checkCase))
                    ret.push_back(notypeSyms[i]);
            }	
        }
        else
        {
            for(i=0;i<notypeSyms.size();i++)
            {
                if(regexEquiv(name, notypeSyms[i]->getName(), checkCase))
                    ret.push_back(notypeSyms[i]);
            }	
        }
        if(ret.size() > start)
            return true;
        serr = No_Such_Symbol;
        return false;
    }
    else if(sType == Symbol::ST_UNKNOWN)
    {
        findFunction(ret, name, isMangled, isRegex, checkCase);
        std::vector<Symbol *>syms;
        findVariable(syms, name, isMangled, isRegex, checkCase);
        ret.insert(ret.end(), syms.begin(), syms.end());
        syms.clear();
        findSymbolByType(syms, name, Symbol::ST_MODULE, isMangled, isRegex, checkCase);
        ret.insert(ret.end(), syms.begin(), syms.end());
        syms.clear();
        findSymbolByType(syms, name, Symbol::ST_NOTYPE, isMangled, isRegex, checkCase);
        ret.insert(ret.end(), syms.begin(), syms.end());
        syms.clear();
        if(ret.size() > 0)
            return true;
        else
        {
            serr = No_Such_Symbol;
            return false;
        }
    }
    return false;
}
				
bool Symtab::findFunction(std::vector <Symbol *> &ret,const std::string &name, 
                              bool isMangled, bool isRegex, 
                              bool checkCase)
{
    if(!isMangled&&!isRegex)
        return findFuncVectorByPretty(name, ret);
    else if(isMangled&&!isRegex)
        return findFuncVectorByMangled(name, ret);
    else if(!isMangled&&isRegex)
        return findFuncVectorByPrettyRegex(name, checkCase, ret);
    else
        return findFuncVectorByMangledRegex(name, checkCase, ret);
}

bool Symtab::findVariable(std::vector <Symbol *> &ret, const std::string &name,
                              bool isMangled, bool isRegex,
                              bool checkCase)
{
    if(!isMangled&&!isRegex)
        return findVarVectorByPretty(name, ret);
    else if(isMangled&&!isRegex)
        return findVarVectorByMangled(name, ret);
    else if(!isMangled&&isRegex)
        return findVarVectorByPrettyRegex(name, checkCase, ret);
    else
        return findVarVectorByMangledRegex(name, checkCase, ret);
}

bool Symtab::findMod(std::vector <Symbol *> &ret, const std::string &name,
                            bool /*isMangled*/, bool isRegex,
                            bool checkCase)
{
    if(!isRegex)
    {
        if(modsByName.find(name)!=modsByName.end())
        {
            ret = *(modsByName[name]);
            return true;	
        }
        serr = No_Such_Module;
        return false;
    }
    else
        return findModByRegex(name, checkCase, ret);
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files
bool Symtab::findFuncVectorByPretty(const std::string &name, std::vector<Symbol *> &ret)
{
    if (funcsByPretty.find(name)!=funcsByPretty.end()) 
    {
        ret = *(funcsByPretty[name]);
        return true;	
    }
    serr = No_Such_Function;
    return false;
}
 
bool Symtab::findFuncVectorByMangled(const std::string &name, std::vector<Symbol *> &ret)
{
    //fprintf(stderr,"findFuncVectorByMangled %s\n",name.c_str());
    //#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    //bperr( "%s[%d]:  inside findFuncVectorByMangled\n", FILE__, __LINE__);
    //#endif
    if (funcsByMangled.find(name)!=funcsByMangled.end()) 
    {
        ret = *(funcsByMangled[name]);
        return true;	
    }
    serr = No_Such_Function;
    return false;
}
 
bool Symtab::findVarVectorByPretty(const std::string &name, std::vector<Symbol *> &ret)
{
    //fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
    //#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    //bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
    //#endif
    if (varsByPretty.find(name)!=varsByPretty.end()) 
    {
        ret = *(varsByPretty[name]);
        return true;	
    }
    serr = No_Such_Variable;
    return false;
}

bool Symtab::findVarVectorByMangled(const std::string &name, std::vector<Symbol *> &ret)
{
    // fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
    //#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    //bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
    //#endif
    if (varsByMangled.find(name)!=varsByMangled.end()) 
    {
        ret = *(varsByMangled[name]);
        return true;	
    }
    serr = No_Such_Variable;
    return false;
}

bool Symtab::findFuncVectorByMangledRegex(const std::string &rexp, bool checkCase, std::vector<Symbol *>&ret)
{
    unsigned start = ret.size();	
    hash_map <std::string, std::vector<Symbol *>*>::iterator iter;
    for(iter = funcsByMangled.begin(); iter!=funcsByMangled.end(); iter++)
    {
        if(regexEquiv(rexp,iter->first,checkCase))
        {
            std::vector<Symbol *> funcs = *(iter->second);
            int size = funcs.size();
            for(int index = 0; index < size; index++)
                ret.push_back(funcs[index]);
        }
    }
    if(ret.size() > start)
        return true;
    serr = No_Such_Function;
    return false;
}
 
bool Symtab::findFuncVectorByPrettyRegex(const std::string &rexp, bool checkCase, std::vector<Symbol *>&ret)
{
    unsigned start = ret.size();
    hash_map <std::string, std::vector<Symbol *>*>::iterator iter;
    for(iter = funcsByPretty.begin(); iter!=funcsByPretty.end(); iter++)
    {
        if(regexEquiv(rexp,iter->first,checkCase))
        {
            std::vector<Symbol *> funcs = *(iter->second);
            int size = funcs.size();
            for(int index = 0; index < size; index++)
                ret.push_back(funcs[index]);
        }
    }
    if(ret.size() > start)
        return true;
    serr = No_Such_Function;
    return false;
}

bool Symtab::findVarVectorByMangledRegex(const std::string &rexp, bool checkCase, std::vector<Symbol *>&ret)
{
    unsigned start = ret.size();
    hash_map <std::string, std::vector<Symbol *>*>::iterator iter;
    for(iter = varsByMangled.begin(); iter!=varsByMangled.end(); iter++)
    {
        if(regexEquiv(rexp,iter->first,checkCase))
        {
            std::vector<Symbol *> vars = *(iter->second);
            int size = vars.size();
            for(int index = 0; index < size; index++)
                ret.push_back(vars[index]);
        }
    }
    if(ret.size() > start)
        return true;
    serr = No_Such_Variable;
    return false;
}

bool Symtab::findVarVectorByPrettyRegex(const std::string &rexp, bool checkCase, std::vector<Symbol *>&ret)
{
    unsigned start = ret.size();
    hash_map <std::string, std::vector<Symbol *>*>::iterator iter;
    for(iter = varsByPretty.begin(); iter!=varsByPretty.end(); iter++)
    {
        if(regexEquiv(rexp,iter->first,checkCase))
        {
            std::vector<Symbol *> vars = *(iter->second);
            int size = vars.size();
            for(int index = 0; index < size; index++)
                ret.push_back(vars[index]);
        }
    }
    if(ret.size() > start)
        return true;
    serr = No_Such_Variable;
    return false;
}

bool Symtab::findModByRegex(const std::string &rexp, bool checkCase, std::vector<Symbol *>&ret)
{
    unsigned start = ret.size();
    hash_map <std::string, std::vector<Symbol *>*>::iterator iter;
    for(iter = modsByName.begin(); iter!=modsByName.end(); iter++)
    {
        if(regexEquiv(rexp,iter->first,checkCase))
        {
            std::vector<Symbol *> vars = *(iter->second);
            int size = vars.size();
            for(int index = 0; index < size; index++)
                ret.push_back(vars[index]);
        }
    }
    if(ret.size() > start)
        return true;
    serr = No_Such_Module;
    return false;
}

bool Symtab::findSectionByEntry(Section *&ret, const Offset offset)
{
    if(secsByEntryAddr.find(offset) != secsByEntryAddr.end())
    {
        ret = secsByEntryAddr[offset];
        return true;
    }
    serr = No_Such_Section;
    return false;
}

bool Symtab::findSection(Section *&ret, const std::string secName)
{
    for(unsigned index=0;index<sections_.size();index++)
    {
        if(sections_[index]->getSecName() == secName)
        {
            ret = sections_[index];
            return true;
        }
    }
    serr = No_Such_Section;
    return false;
}
 
// Address must be in code or data range since some code may end up
// in the data segment
bool Symtab::isValidOffset(const Offset where) const
{
    return isCode(where) || isData(where);
}

bool Symtab::isCode(const Offset where)  const
{
    return (linkedFile->code_ptr() && 
            (where >= imageOffset_) && (where < (imageOffset_+imageLen_)));
}

bool Symtab::isData(const Offset where)  const
{
    return (linkedFile->data_ptr() && 
            (where >= dataOffset_) && (where < (dataOffset_+dataLen_)));
}

bool Symtab::getFuncBindingTable(std::vector<relocationEntry> &fbt) const
{
    fbt = relocation_table_;
    return true;
}

Symtab::~Symtab()
{
   // Doesn't do anything yet, moved here so we don't mess with symtab.h
   // Only called if we fail to create a process.
   // Or delete the a.out...
	
    unsigned i;
            
    for (i = 0; i < sections_.size(); i++) {
        delete sections_[i];
    }
    sections_.clear();
    
    for (i = 0; i < newSections_.size(); i++) {
        delete newSections_[i];
    }
    newSections_.clear();
    
    for (i = 0; i < _mods.size(); i++) {
        delete _mods[i];
    }
    _mods.clear();

    for(i=0; i< modSyms.size(); i++)
        delete modSyms[i];
    modSyms.clear();	
    modsByFileName.clear();
    modsByFullName.clear();

    for(i=0; i< notypeSyms.size(); i++)
        delete notypeSyms[i];
    notypeSyms.clear();	
    
    for (i = 0; i < everyUniqueVariable.size(); i++) {
        delete everyUniqueVariable[i];
    }
    everyUniqueVariable.clear();
    
    for (i = 0; i < everyUniqueFunction.size(); i++) {
        delete everyUniqueFunction[i];
    }
    everyUniqueFunction.clear();
    createdFunctions.clear();
    exportedFunctions.clear();
    
    undefDynSyms.clear();
    for(i=0;i<excpBlocks.size();i++)
        delete excpBlocks[i];

    
    for (i = 0; i < allSymtabs.size(); i++) {
        if (allSymtabs[i] == this)
            allSymtabs.erase(allSymtabs.begin()+i);
    }
}	
 
bool Module::findSymbolByType(std::vector<Symbol *> &found, const std::string name,
                                  Symbol::SymbolType sType, bool isMangled,
                                  bool isRegex, bool checkCase)
{
    unsigned orig_size = found.size();
    std::vector<Symbol *> obj_syms;
    if(!exec()->findSymbolByType(obj_syms, name, sType, isMangled, isRegex, checkCase))
        return false;
    for (unsigned i = 0; i < obj_syms.size(); i++) 
    {
        if(obj_syms[i]->getModule() == this)
            found.push_back(obj_syms[i]);
    }
    if (found.size() > orig_size) 
        return true;
    return false;	
}
 
DLLEXPORT const std::string &Module::fileName() const {
    return fileName_; 
}

DLLEXPORT const std::string &Module::fullName() const { 
    return fullName_; 
}
 
DLLEXPORT Symtab *Module::exec() const { 
    return exec_;
}

DLLEXPORT supportedLanguages Module::language() const { 
    return language_;
}

DLLEXPORT LineInformation *Module::getLineInformation(){
    if(exec_->isLineInfoValid_)
    	return lineInfo_;
    exec_->parseLineInformation();
    return lineInfo_;
}

DLLEXPORT bool Module::getAddressRanges(std::vector<pair<Offset, Offset> >&ranges,
                                                std::string lineSource, unsigned int lineNo)
{
    unsigned int originalSize = ranges.size();

    LineInformation *lineInformation = getLineInformation();
    if(lineInformation)
        lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );
     if( ranges.size() != originalSize )
     	return true;
     return false;
}

DLLEXPORT bool Module::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
    unsigned int originalSize = lines.size();

    LineInformation *lineInformation = getLineInformation();
    if(lineInformation)
        lineInformation->getSourceLines( addressInRange, lines );
     if( lines.size() != originalSize )
     	return true;
     return false;
	    
}

DLLEXPORT vector<Type *> *Module::getAllTypes(){
    if(!moduleTypes_)
    	moduleTypes_ = typeCollection::getModTypeCollection(this);
    return moduleTypes_->getAllTypes();
}

DLLEXPORT vector<pair<string, Type *> > *Module::getAllGlobalVars(){
    if(!moduleTypes_)
    	moduleTypes_ = typeCollection::getModTypeCollection(this);
    return moduleTypes_->getAllGlobalVariables();
}

DLLEXPORT typeCollection *Module::getModuleTypes(){
    if(!moduleTypes_)
    	moduleTypes_ = typeCollection::getModTypeCollection(this);
    return moduleTypes_;
}

DLLEXPORT bool Module::findType(Type *&type, std::string name)
{
    exec_->parseTypesNow();
    type = getModuleTypes()->findType(name);
    if(type == NULL)
    	return false;
    return true;	
}

DLLEXPORT bool Module::findVariableType(Type *&type, std::string name)
{
    exec_->parseTypesNow();
    type = getModuleTypes()->findVariableType(name);
    if(type == NULL)
    	return false;
    return true;	
}

void Symtab::parseTypesNow(){
   if(isTypeInfoValid_)
   	return;
   parseTypes();
}

DLLEXPORT bool Module::setLineInfo(LineInformation *lineInfo) {
    lineInfo_ = lineInfo;
    return true;
}

DLLEXPORT bool Module::findLocalVariable(std::vector<localVar *>&vars, std::string name)
{
   exec_->parseTypesNow();
   std::vector<Symbol *>mod_funcs;
   if(!getAllSymbolsByType(mod_funcs, Symbol::ST_FUNCTION))
   	return false;
   unsigned i, origSize = vars.size();
   for(i=0;i<mod_funcs.size();i++)
   	mod_funcs[i]->findLocalVariable(vars, name);
   if(vars.size()>origSize)
   	return true;
   return false;	
}

DLLEXPORT Module::Module(supportedLanguages lang, Offset adr, 
                                 std::string fullNm, Symtab *img)
    : fullName_(fullNm), language_(lang), addr_(adr), exec_(img),
      lineInfo_(NULL), moduleTypes_(NULL)
    
{
    fileName_ = extract_pathname_tail(fullNm);
}

DLLEXPORT Module::Module()
  : language_(lang_Unknown), addr_(0), exec_(NULL), 
    lineInfo_(NULL), moduleTypes_(NULL)
{
}

DLLEXPORT Module::Module(const Module &mod)
   : LookupInterface(),fileName_(mod.fileName_),
     fullName_(mod.fullName_), language_(mod.language_),
     addr_(mod.addr_), exec_(mod.exec_), lineInfo_(mod.lineInfo_),
     moduleTypes_(mod.moduleTypes_)
{
}

DLLEXPORT Module::~Module()
{
}

bool Module::isShared() const 
{ 
    return !exec_->isExec();
}

bool Module::getAllSymbolsByType(std::vector<Symbol *> &found, Symbol::SymbolType sType)
{
    unsigned orig_size = found.size();
    std::vector<Symbol *> obj_syms;
    if(!exec()->getAllSymbolsByType(obj_syms, sType))
        return false;
    for (unsigned i = 0; i < obj_syms.size(); i++) 
    {
        if(obj_syms[i]->getModule() == this)
           found.push_back(obj_syms[i]);
    }
    if (found.size() > orig_size) 
        return true;
    serr = No_Such_Symbol;	
    return false;
}

DLLEXPORT bool Module::operator==(const Module &mod) const {
    return ( (language_==mod.language_) &&
            (addr_==mod.addr_) &&
            (fullName_==mod.fullName_) &&
            (exec_==mod.exec_) &&
	    (lineInfo_ == mod.lineInfo_) 
          );
}

DLLEXPORT bool Module::setName(std::string newName) {
    fullName_ = newName;
    fileName_ = extract_pathname_tail(fullName_);
    return true;
}

DLLEXPORT void Module::setLanguage(supportedLanguages lang) 
{
    language_ = lang;
}

DLLEXPORT Offset Module::addr() const { 
    return addr_; 
}

// Use POSIX regular expression pattern matching to check if std::string s matches
// the pattern in this std::string
bool regexEquiv( const std::string &str,const std::string &them, bool checkCase ) 
{
    const char *str_ = str.c_str();
    const char *s = them.c_str();
    // Would this work under NT?  I don't know.
//#if !defined(os_windows)
#if 0
    regex_t r;
    bool match = false;
    int cflags = REG_NOSUB;
    if( !checkCase )
        cflags |= REG_ICASE;

    // Regular expressions must be compiled first, see 'man regexec'
    int err = regcomp( &r, str_, cflags );
    
    if( err == 0 ) {
        // Now we can check for a match
        err = regexec( &r, s, 0, NULL, 0 );
        if( err == 0 )
            match = true;
    }

    // Deal with errors
    if( err != 0 && err != REG_NOMATCH ) {
        char errbuf[80];
        regerror( err, &r, errbuf, 80 );
        cerr << "regexEquiv -- " << errbuf << endl;
    }

    // Free the pattern buffer
    regfree( &r );
    return match;
#else
    return pattern_match(str_, s, checkCase);
#endif

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

bool Symtab::openFile(Symtab *&obj, std::string filename){
    bool err;
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    unsigned numSymtabs = allSymtabs.size();
	  
    // AIX: it's possible that we're reparsing a file with better information
    // about it. If so, yank the old one out of the allSymtabs std::vector -- replace
    // it, basically.
    if(filename.find("/proc") == std::string::npos)
    {
        for (unsigned u=0; u<numSymtabs; u++) {
            if (filename == allSymtabs[u]->file()) {
                // return it
                obj = allSymtabs[u];
                return true;
            }
        }   
    }
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
    if(err == true)
    {
        if(filename.find("/proc") == std::string::npos)
            allSymtabs.push_back(obj);
        obj->setupTypes();	
    }
    else
        obj = NULL;
    return err;
}
	
bool Symtab::openFile(Symtab *&obj, char *mem_image, size_t size){
    bool err;
    obj = new Symtab(mem_image, size, err);
    if(err == false)
        obj = NULL;
    else
        obj->setupTypes();	
    return err;
}

bool Symtab::changeType(Symbol *sym, Symbol::SymbolType oldType)
{
    std::vector<std::string>names;
    if(oldType == Symbol::ST_FUNCTION)
    {
        unsigned i;
        std::vector<Symbol *> *funcs;
        std::vector<Symbol *>::iterator iter;
        names = sym->getAllMangledNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByMangled[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        names = sym->getAllPrettyNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByPretty[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        names = sym->getAllTypedNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByPretty[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        iter = find(everyUniqueFunction.begin(), everyUniqueFunction.end(), sym);
        everyUniqueFunction.erase(iter);
    }
    else if(oldType == Symbol::ST_OBJECT)
    {
        unsigned i;
        std::vector<Symbol *> *vars;
        std::vector<Symbol *>::iterator iter;
        names = sym->getAllMangledNames();
        for(i=0;i<names.size();i++)
        {
            vars = varsByMangled[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        names.clear();
        names = sym->getAllPrettyNames();
        for(i=0;i<names.size();i++)
        {
            vars = varsByPretty[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        names.clear();
        names = sym->getAllTypedNames();
        for(i=0;i<names.size();i++)
        {
            vars= varsByPretty[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        iter = find(everyUniqueVariable.begin(), everyUniqueVariable.end(), sym);
        everyUniqueVariable.erase(iter);
    }
    else if(oldType == Symbol::ST_MODULE)
    {
        unsigned i;
        std::vector<Symbol *> *mods;
        std::vector<Symbol *>::iterator iter;
        names = sym->getAllMangledNames();
        for(i=0;i<names.size();i++)
        {
            mods = modsByName[names[i]];
            iter = find(mods->begin(), mods->end(), sym);
            mods->erase(iter);
        }
        names.clear();
        iter = find(modSyms.begin(),modSyms.end(),sym);
        modSyms.erase(iter);
    }
    else if(oldType == Symbol::ST_NOTYPE)
    {
        std::vector<Symbol *>::iterator iter;
        iter = find(notypeSyms.begin(),notypeSyms.end(),sym);
        notypeSyms.erase(iter);
    }
    addSymbol(sym);
    return true;
}

bool Symtab::delSymbol(Symbol *sym)
{
    std::vector<std::string>names;
    if(sym->getType() == Symbol::ST_FUNCTION)
    {
        unsigned i;
        std::vector<Symbol *> *funcs;
        std::vector<Symbol *>::iterator iter;
        names = sym->getAllMangledNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByMangled[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        names = sym->getAllPrettyNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByPretty[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        names = sym->getAllTypedNames();
        for(i=0;i<names.size();i++)
        {
            funcs = funcsByPretty[names[i]];
            iter = find(funcs->begin(), funcs->end(), sym);
            funcs->erase(iter);
        }
        names.clear();
        iter = find(everyUniqueFunction.begin(), everyUniqueFunction.end(), sym);
        everyUniqueFunction.erase(iter);
    }
    else if(sym->getType() == Symbol::ST_OBJECT)
    {
        unsigned i;
        std::vector<Symbol *> *vars;
        std::vector<Symbol *>::iterator iter;
        names = sym->getAllMangledNames();
        for(i=0;i<names.size();i++)
        {
            vars = varsByMangled[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        names.clear();
        names = sym->getAllPrettyNames();
        for(i=0;i<names.size();i++)
        {
            vars = varsByPretty[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        names.clear();
        names = sym->getAllTypedNames();
        for(i=0;i<names.size();i++)
        {
            vars= varsByPretty[names[i]];
            iter = find(vars->begin(), vars->end(), sym);
            vars->erase(iter);
        }
        iter = find(everyUniqueVariable.begin(), everyUniqueVariable.end(), sym);
        everyUniqueVariable.erase(iter);
    }
    else if(sym->getType() == Symbol::ST_MODULE)
    {
        std::vector<Symbol *>::iterator iter;
        iter = find(modSyms.begin(),modSyms.end(),sym);
        modSyms.erase(iter);
    }
    else if(sym->getType() == Symbol::ST_NOTYPE)
    {
        std::vector<Symbol *>::iterator iter;
        iter = find(notypeSyms.begin(),notypeSyms.end(),sym);
        notypeSyms.erase(iter);
    }
    delete(sym);
    return true;
}

bool Symtab::addSection(Offset vaddr, void *data, unsigned int dataSize, std::string name, unsigned long flags, bool loadable)
{
    Section *sec;
    unsigned i;
    if(loadable)
    {
        sec = new Section(newSectionInsertPoint, name, vaddr, dataSize, data, flags, loadable);
	sections_.insert(sections_.begin()+newSectionInsertPoint, sec);
	for(i = newSectionInsertPoint+1; i < sections_.size(); i++)
		sections_[i]->setSecNumber(sections_[i]->getSecNumber() + 1);
    }
    else
    {
	sec = new Section(sections_.size()+1, name, vaddr, dataSize, data, flags, loadable);
	sections_.push_back(sec);
    }	
    newSections_.push_back(sec);
    return true;
}

bool Symtab::addSection(Section *sec)
{
    sections_.push_back(sec);
    newSections_.push_back(sec);
    return true;
}

void Symtab::parseLineInformation()
{
    hash_map <std::string, LineInformation> &lineInfo_ = linkedFile->getLineInfo();
    isLineInfoValid_ = true;	
    hash_map <std::string, LineInformation>::iterator iter;
    for(iter = lineInfo_.begin(); iter!=lineInfo_.end(); iter++)
    {
        Module *mod = NULL;
        if(findModule(mod, iter->first))
            mod->setLineInfo(&(iter->second));
	else if(findModule(mod, name_))
	{
	    LineInformation *lineInformation = mod->getLineInformation();
	    if(!lineInformation)
	        mod->setLineInfo(&(iter->second));
	    else
	    {
	    	lineInformation->addLineInfo(&(iter->second));
	    	mod->setLineInfo(lineInformation);
	    }	
	}
    }
}

DLLEXPORT bool Symtab::getAddressRanges(std::vector<pair<Offset, Offset> >&ranges,
                                            std::string lineSource, unsigned int lineNo)
{
    unsigned int originalSize = ranges.size();

    /* Iteratate over the modules, looking for ranges in each. */
    for( unsigned int i = 0; i < _mods.size(); i++ ) {
	LineInformation *lineInformation = _mods[i]->getLineInformation();
        if(lineInformation)
	     lineInformation->getAddressRanges( lineSource.c_str(), lineNo, ranges );
     } /* end iteration over modules */
     if( ranges.size() != originalSize )
     	return true;
     return false;
}

DLLEXPORT bool Symtab::getSourceLines(std::vector<LineNoTuple> &lines, Offset addressInRange)
{
    unsigned int originalSize = lines.size();

    /* Iteratate over the modules, looking for ranges in each. */
    for( unsigned int i = 0; i < _mods.size(); i++ ) {
	LineInformation *lineInformation = _mods[i]->getLineInformation();
        if(lineInformation)
	    lineInformation->getSourceLines( addressInRange, lines );
     } /* end iteration over modules */
     if( lines.size() != originalSize )
     	return true;
     return false;
	    
}

DLLEXPORT bool Symtab::addLine(std::string lineSource, unsigned int lineNo,
	                       unsigned int lineOffset, Offset lowInclAddr,
        	               Offset highExclAddr)
{
    Module *mod;
    if(!findModule(mod, lineSource))
    {
	std::string fileNm = extract_pathname_tail(lineSource);
    	if(!findModule(mod, fileNm))
	{
	    if(!findModule(mod, name_))
 	    return false;
	}    
    }
    LineInformation *lineInfo = mod->getLineInformation();
    if(!lineInfo)
    	return false;
    return(lineInfo->addLine(lineSource.c_str(), lineNo, lineOffset, lowInclAddr, highExclAddr));
}

DLLEXPORT bool Symtab::addAddressRange( Offset lowInclusiveAddr, Offset highExclusiveAddr,
					std::string lineSource, unsigned int lineNo,
	                	        unsigned int lineOffset)
{
    Module *mod;
    if(!findModule(mod, lineSource))
    {
	std::string fileNm = extract_pathname_tail(lineSource);
    	if(!findModule(mod, fileNm))
 	    return false;
    }
    LineInformation *lineInfo = mod->getLineInformation();
    if(!lineInfo)
    	return false;
    return(lineInfo->addAddressRange(lowInclusiveAddr, highExclusiveAddr, lineSource.c_str(), lineNo, lineOffset));
}
																			

void Symtab::parseTypes()
{
    linkedFile->parseTypeInfo(this);
    isTypeInfoValid_ = true;
}

bool Symtab::addType(Type *type)
{
    if(!APITypes)
    	APITypes = typeCollection::getGlobalTypeCollection();
    APITypes->addType(type);	
    return true;
}

DLLEXPORT vector<Type *> *Symtab::getAllstdTypes(){
    setupStdTypes();
    return stdTypes->getAllTypes(); 	
}

DLLEXPORT vector<Type *> *Symtab::getAllbuiltInTypes(){
    setupStdTypes();
    return builtInTypes->getAllBuiltInTypes();
}

DLLEXPORT bool Symtab::findType(Type *&type, std::string name)
{
    parseTypesNow();
    if(!_mods.size())
    	return false;
    type = _mods[0]->getModuleTypes()->findType(name);
    if(type == NULL)
    	return false;
    return true;	
}

DLLEXPORT bool Symtab::findVariableType(Type *&type, std::string name)
{
	parseTypesNow();
    if(!_mods.size())
    	return false;
    type = _mods[0]->getModuleTypes()->findVariableType(name);
    if(type == NULL)
    	return false;
    return true;	
}

DLLEXPORT bool Symtab::findLocalVariable(std::vector<localVar *>&vars, std::string name)
{
   parseTypesNow();
   unsigned i, origSize = vars.size();
   for(i=0;i<everyUniqueFunction.size();i++)
   	everyUniqueFunction[i]->findLocalVariable(vars, name);
   if(vars.size()>origSize)
   	return true;
   return false;	
}

/********************************************************************************
// Symtab::exportXML
// This functions generates the XML document for all the data in Symtab.
********************************************************************************/

bool Symtab::exportXML(std::string file)
{
    int rc;
#if defined(_MSC_VER)
    hXML = LoadLibrary(LPCSTR("../../../i386-unknown-nt4.0/lib/libxml2.dll"));
    if(hXML == NULL){
        serr = Export_Error;
        char buf[1000];
        int result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
                                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                                buf, 1000, NULL);
        errMsg = buf;
        return false;
    }
    my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))GetProcAddress(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))GetProcAddress(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))GetProcAddress(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndElement");
#else
    hXML = dlopen("libxml2.so", RTLD_LAZY);
    if(hXML == NULL){
    	serr = Export_Error;
    	errMsg = "Unable to find libxml2";
        return false;
    }	
    my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))dlsym(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))dlsym(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))dlsym(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))dlsym(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndElement");
#endif    	 
	    
    /* Create a new XmlWriter for DOM */
    xmlTextWriterPtr writer = my_xmlNewTextWriterFilename(file.c_str(), 0);
    if (writer == NULL) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error creating the xml writer";
	return false;
    }	
    rc = my_xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartDocument";
	return false;
    }
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Symtab");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "file",
                                             "%s", filename_.c_str());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "imageOff",
                                             "0x%lx", imageOffset_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "imageLen",
                                             "%ld", imageLen_);
    if (rc < 0) {
        serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "dataOff",
                                             "0x%lx", dataOffset_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "dataLen",
                                             "%ld", dataLen_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "isExec",
                                                 "%d", is_a_out);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    
    generateXMLforSyms(writer, everyUniqueFunction, everyUniqueVariable, modSyms, notypeSyms);
    generateXMLforExcps(writer, excpBlocks);
    std::vector<relocationEntry> fbt;
    getFuncBindingTable(fbt);
    generateXMLforRelocations(writer, fbt);

    //Module information including lineInfo & typeInfo
    //generateXMLforModules(writer, _mods);
    
    rc = my_xmlTextWriterEndDocument(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndDocument";
        return false;
    }
    my_xmlFreeTextWriter(writer);

#if defined(_MSC_VER)
    FreeLibrary(hXML);
#endif

    return true;
}

bool generateXMLforSyms( xmlTextWriterPtr &writer, std::vector<Symbol *> &everyUniqueFunction, std::vector<Symbol *> &everyUniqueVariable, std::vector<Symbol *> &modSyms, std::vector<Symbol *> &notypeSyms)
{
    unsigned tot = everyUniqueFunction.size()+everyUniqueVariable.size()+modSyms.size()+notypeSyms.size();
    unsigned i;
    if(!tot)
    	return true;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Symbols");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }

    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    
    for(i=0;i<everyUniqueFunction.size();i++)
    {
        if(!generateXMLforSymbol(writer, everyUniqueFunction[i]))
	    return false;
    }		
    for(i=0;i<everyUniqueVariable.size();i++)
    {
        if(!generateXMLforSymbol(writer, everyUniqueVariable[i]))
	    return false;
    }	    
    for(i=0;i<modSyms.size();i++)
    {
        if(!generateXMLforSymbol(writer, modSyms[i]))
	    return false;
    }	    
    for(i=0;i<notypeSyms.size();i++)
    {
        if(!generateXMLforSymbol(writer, notypeSyms[i]))
	    return false;
    }	   
    
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforSymbol(xmlTextWriterPtr &writer, Symbol *sym)
{
    int rc,j,tot;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Symbol");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "type",
                                             "%d", sym->getType());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "linkage",
                                             "%d", sym->getLinkage());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "addr",
                                             "0x%lx", sym->getAddr());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "size",
                                             "%ld", sym->getSize());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    tot = sym->getAllMangledNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "mangledNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	std::vector<std::string> names = sym->getAllMangledNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    tot = sym->getAllPrettyNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "prettyNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	std::vector<std::string> names = sym->getAllPrettyNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    tot = sym->getAllTypedNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "typedNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	std::vector<std::string> names = sym->getAllTypedNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "moduleName",
                                             "%s", sym->getModuleName().c_str());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforExcps(xmlTextWriterPtr &writer, std::vector<ExceptionBlock *> &excpBlocks)
{
    unsigned tot = excpBlocks.size(), i;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "ExcpBlocks");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    for(i=0;i<excpBlocks.size();i++)
    {
        rc = my_xmlTextWriterStartElement(writer, BAD_CAST "ExcpBlock");
        if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "hasTry",
                                             "%d", excpBlocks[i]->hasTry());
   	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
	if(excpBlocks[i]->hasTry())
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "tryStart",
            		                                 "0x%lx", excpBlocks[i]->tryStart());
    	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "trySize",
            		                                 "%ld", excpBlocks[i]->trySize());
    	    if (rc < 0) {
    		serr = Export_Error;
       		errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
		return false;
	    }	
	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "catchStart",
        	                                 "0x%lx", excpBlocks[i]->catchStart());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterEndElement(writer);
	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
            return false;
	}
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforRelocations(xmlTextWriterPtr &writer, std::vector<relocationEntry> &fbt)
{
    unsigned tot = fbt.size(), i;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Relocations");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    for(i=0;i<fbt.size();i++)
    {
        rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Relocation");
        if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "targetAddr",
        	                                 "%lx", fbt[i].target_addr());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "relAddr",
        	                                 "%lx", fbt[i].rel_addr());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
        	                                     "%s", fbt[i].name().c_str());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterEndElement(writer);
	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
            return false;
	}
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforModules(xmlTextWriterPtr &writer, std::vector<Module *> &mods)
{
    unsigned tot = mods.size(), i;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Modules");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }	
    for(i=0; i<mods.size(); i++)
    {
        rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Module");
        if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
	    return false;
	}    
	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "fullName",
        			                                 "%s", mods[i]->fullName().c_str());
    	if (rc < 0) {
	   serr = Export_Error;
       	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
       	   return false;
	}   
	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "addr",
        			                                 "0x%lx", mods[i]->addr());
    	if (rc < 0) {
	   serr = Export_Error;
       	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
       	   return false;
	}   
	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "lang",
        			                                 "%d", mods[i]->language());
    	if (rc < 0) {
	   serr = Export_Error;
       	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
       	   return false;
	}   
	
	LineInformation *lineInformation = mods[i]->getLineInformation();
        if(lineInformation) {
            rc = my_xmlTextWriterStartElement(writer, BAD_CAST "LineMap");
            if (rc < 0) {
    	    	serr = Export_Error;
            	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
	        return false;
	    }    
	    LineInformation::const_iterator iter = lineInformation->begin();
	    for(;iter!=lineInformation->end();iter++)
	    {
	        const std::pair<Offset, Offset> range = iter->first;
	        LineNoTuple line = iter->second;
                rc = my_xmlTextWriterStartElement(writer, BAD_CAST "LineMapEntry");
                if (rc < 0) {
    	    	    serr = Export_Error;
            	    errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
	            return false;
	        }
    		rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "rangeStart",
	        			                                 "0x%lx", range.first);
	    	if (rc < 0) {
    		   serr = Export_Error;
           	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
             	   return false;
		}   
    		rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "rangeEnd",
	        			                                 "0x%lx", range.second);
	    	if (rc < 0) {
    		   serr = Export_Error;
           	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
             	   return false;
		}   
    		rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "lineSource",
	        			                                 "%s", line.first);
	    	if (rc < 0) {
    		   serr = Export_Error;
           	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
             	   return false;
		}   
    		rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "lineNumber",
	        			                                 "%d", line.second);
	    	if (rc < 0) {
    		   serr = Export_Error;
           	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
             	   return false;
		}   
    		rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "lineColumn",
	        			                                 "%d", line.column);
	    	if (rc < 0) {
    		   serr = Export_Error;
           	   errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
             	   return false;
		}   
    		rc = my_xmlTextWriterEndElement(writer);
		if (rc < 0) {
    		    serr = Export_Error;
		    errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        	    return false;
		}
	    }
    	    rc = my_xmlTextWriterEndElement(writer);
	    if (rc < 0) {
    	        serr = Export_Error;
	        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
                return false;
	    }
	}
    	rc = my_xmlTextWriterEndElement(writer);
	if (rc < 0) {
    	    serr = Export_Error;
	    errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
            return false;
	}
    }
    return true;
}

DLLEXPORT bool Symtab::emitSymbols(std::string filename, unsigned flag)
{
    return linkedFile->emitDriver(this, filename, everyUniqueFunction, everyUniqueVariable, modSyms, notypeSyms, flag);
}

DLLEXPORT bool Symtab::emit(std::string filename, unsigned flag)
{
    return linkedFile->emitDriver(this, filename, everyUniqueFunction, everyUniqueVariable, modSyms, notypeSyms, flag);
}

DLLEXPORT bool Symtab::getSegments(vector<Segment> &segs) const
{
    return linkedFile->getSegments(segs);
}

DLLEXPORT bool Symtab::getMappedRegions(std::vector<Region> &regs) const
{
   return linkedFile->getMappedRegions(regs);
}

DLLEXPORT bool Symtab::updateCode(void *buffer, unsigned size)
{
    Section *sec;
    if(!findSection(sec, ".text"))
    	return false;
    sec->setPtrToRawData(buffer, size);
    return true;
}

DLLEXPORT bool Symtab::updateData(void *buffer, unsigned size)
{
    Section *sec;
    if(!findSection(sec, ".data"))
    	return false;
    sec->setPtrToRawData(buffer, size);
    return true;
}

DLLEXPORT Offset Symtab::getFreeOffset(unsigned size) {
    // Look through sections until we find a gap with
    // sufficient space.
    Offset highWaterMark = 0;

    for (unsigned i = 0; i < sections_.size(); i++) {
        Offset end = sections_[i]->getSecAddr() + sections_[i]->getSecSize();
        if (sections_[i]->getSecAddr() == 0) continue;
        /*fprintf(stderr, "%d: secAddr 0x%lx, size %d, end 0x%lx, looking for %d\n",
	                i, sections_[i]->getSecAddr(), sections_[i]->getSecSize(),
	                end,size);*/
	if (end > highWaterMark) {
	    //fprintf(stderr, "Increasing highWaterMark...\n");
	     newSectionInsertPoint = i+1;
	     highWaterMark = end;
        }
        if ((i <= (sections_.size()-1)) &&
               ((end + size) < sections_[i+1]->getSecAddr())) {
      /*      fprintf(stderr, "Found a hole between sections %d and %d\n",
					                    i, i+1);
            fprintf(stderr, "End at 0x%lx, next one at 0x%lx\n",
 		                 end, sections_[i+1]->getSecAddr());
    	*/   
	     newSectionInsertPoint = i+1;
	     return end;
       }
   }
   return highWaterMark;
}

DLLEXPORT ObjectType Symtab::getObjectType() const {
    return linkedFile->objType();
}

DLLEXPORT char *Symtab::mem_image() const { 
    return linkedFile->mem_image(); 
}

DLLEXPORT const std::string &Symtab::file() const {
    return filename_;
}

DLLEXPORT const std::string &Symtab::name() const {
    return name_;
}

DLLEXPORT unsigned Symtab::getNumberofSections() const { 
    return no_of_sections; 
}

DLLEXPORT unsigned Symtab::getNumberofSymbols() const { 
    return no_of_symbols; 
}

#if defined(os_aix)
void Symtab::get_stab_info(char *&stabstr, int &nstabs, void *&stabs, 
                               char *&stringpool) const

{
    linkedFile->get_stab_info(stabstr,nstabs, stabs, stringpool);
}


void Symtab::get_line_info(int& nlines, char*& lines,
                               unsigned long& fdptr) const
{
    linkedFile->get_line_info(nlines,lines,fdptr);
}
#else
void Symtab::get_stab_info(char *&, int &, void *&, char *&) const 
{
}


void Symtab::get_line_info(int &, char *&, unsigned long&) const
{
}
#endif


DLLEXPORT LookupInterface::LookupInterface() 
{
}

DLLEXPORT LookupInterface::~LookupInterface()
{
}


DLLEXPORT ExceptionBlock::ExceptionBlock(Offset tStart, 
                                                 unsigned tSize, 
                                                 Offset cStart) 
   : tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) 
{
}

DLLEXPORT ExceptionBlock::ExceptionBlock(Offset cStart) 
   : tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) 
{
}

DLLEXPORT ExceptionBlock::ExceptionBlock(const ExceptionBlock &eb) 
   : tryStart_(eb.tryStart_), trySize_(eb.trySize_), 
     catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) 
{
}
 
DLLEXPORT bool ExceptionBlock::hasTry() const
{ 
    return hasTry_; 
}

DLLEXPORT Offset ExceptionBlock::tryStart() const
{ 
    return tryStart_; 
}

DLLEXPORT Offset ExceptionBlock::tryEnd() const
{ 
    return tryStart_ + trySize_; 
}

DLLEXPORT Offset ExceptionBlock::trySize() const
{
    return trySize_; 
}

DLLEXPORT bool ExceptionBlock::contains(Offset a) const
{ 
    return (a >= tryStart_ && a < tryStart_ + trySize_); 
}

DLLEXPORT Section::Section()
{
}

DLLEXPORT Section::Section(unsigned sidnumber, std::string sname, 
                                   Offset saddr, unsigned long ssize, 
                                   void *secPtr, unsigned long sflags, bool isLoadable) 
   : sidnumber_(sidnumber), sname_(sname), saddr_(saddr), ssize_(ssize), 
     rawDataPtr_(secPtr), sflags_(sflags), isLoadable_(isLoadable),
     isDirty_(false)
{
}

DLLEXPORT Section::Section(unsigned sidnumber, std::string sname, 
                                   unsigned long ssize, void *secPtr, 
                                   unsigned long sflags, bool isLoadable)
   : sidnumber_(sidnumber), sname_(sname), saddr_(0), ssize_(ssize), 
     rawDataPtr_(secPtr), sflags_(sflags), isLoadable_(isLoadable),
     isDirty_(false)
{
}

DLLEXPORT Section::Section(const Section &sec)
   : sidnumber_(sec.sidnumber_),sname_(sec.sname_), saddr_(sec.saddr_), 
     ssize_(sec.ssize_), rawDataPtr_(sec.rawDataPtr_), 
     sflags_(sec.sflags_), isLoadable_(sec.isLoadable_),
     isDirty_(sec.isDirty_)
{
}

DLLEXPORT Section& Section::operator=(const Section &sec)
{
    sidnumber_ = sec.sidnumber_;
    sname_ = sec.sname_;
    saddr_ = sec.saddr_;
    ssize_ = sec.ssize_;
    rawDataPtr_ = sec.rawDataPtr_;
    sflags_ = sec.sflags_;
    isLoadable_ = sec.isLoadable_;
    isDirty_ = sec.isDirty_;
    return *this;
}
	
DLLEXPORT ostream& Section::operator<< (ostream &os) 
{
    return os   << "{"
                << " id="      << sidnumber_
                << " name="    << sname_
                << " addr="    << saddr_
                << " size="    << ssize_
		<< " loadable" << isLoadable_
                << " }" << endl;
}

DLLEXPORT bool Section::operator== (const Section &sec)
{
    return ((sidnumber_ == sec.sidnumber_)&&
           (sname_ == sec.sname_)&&
           (saddr_ == sec.saddr_)&&
           (ssize_ == sec.ssize_)&&
           (rawDataPtr_ == sec.rawDataPtr_));
}

DLLEXPORT Section::~Section() 
{
}
 
DLLEXPORT unsigned Section::getSecNumber() const
{ 
    return sidnumber_; 
}

DLLEXPORT bool Section::setSecNumber(unsigned sidnumber)
{
    sidnumber_ = sidnumber;
    return true;
}

DLLEXPORT std::string Section::getSecName() const
{ 
    return sname_; 
}

DLLEXPORT Offset Section::getSecAddr() const
{ 
    return saddr_; 
}

DLLEXPORT void *Section::getPtrToRawData() const
{ 
    return rawDataPtr_; 
}

DLLEXPORT bool Section::setPtrToRawData(void *buf, unsigned long size)
{
    rawDataPtr_ = buf;
    ssize_ = size;
    isDirty_ = true;
    return true;
}

DLLEXPORT unsigned long Section::getSecSize() const
{ 
    return ssize_; 
}

DLLEXPORT unsigned Section::getFlags() const
{
    return sflags_;
}

DLLEXPORT bool Section::isBSS() const
{ 
    return sname_==".bss";
}

DLLEXPORT bool Section::isText() const
{ 
    return sname_ == ".text"; 
}

DLLEXPORT bool Section::isData() const
{ 
    return (sname_ == ".data"||sname_ == ".data2"); 
}

DLLEXPORT bool Section::isOffsetInSection(const Offset &offset) const
{
    return (offset > saddr_ && offset < saddr_ + ssize_);
}

DLLEXPORT bool Section::isLoadable() const
{
    return isLoadable_;
}

DLLEXPORT bool Section::isDirty() const
{
    return isDirty_;
}

DLLEXPORT unsigned long Section::flags() const{
    return sflags_;
}

DLLEXPORT bool Section::addRelocationEntry(Offset ra, Symbol *dynref, unsigned long relType){
    relocationEntry re(ra, dynref->getPrettyName(), dynref, relType);
//    return linkedFile->addRelocationEntry(re);
    return true;
}
 

DLLEXPORT relocationEntry::relocationEntry()
   :target_addr_(0),rel_addr_(0), dynref_(NULL), relType_(0)
{
}   

DLLEXPORT relocationEntry::relocationEntry(Offset ta,Offset ra, std::string n, Symbol *dynref, unsigned long relType)
   : target_addr_(ta), rel_addr_(ra),name_(n), dynref_(dynref), relType_(relType)
{
}   

DLLEXPORT relocationEntry::relocationEntry(Offset ra, std::string n, Symbol *dynref, unsigned long relType)
   : target_addr_(0), rel_addr_(ra),name_(n), dynref_(dynref), relType_(relType)
{
}   

DLLEXPORT const relocationEntry& relocationEntry::operator=(const relocationEntry &ra) 
{
    target_addr_ = ra.target_addr_; rel_addr_ = ra.rel_addr_; 
    name_ = ra.name_; 
    dynref_ = ra.dynref_;
    relType_ = ra.relType_;
    return *this;
}
