/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */
 
// $Id: symtab.h,v 1.179 2005/07/29 22:16:30 bernat Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR
//#define REGEX_CHARSET "^*[]|?"
#define REGEX_CHARSET "^*|?"
extern "C" {
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11)
#include <regex.h>
#endif
}

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/List.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/h/BPatch_Vector.h"
#include "dyninstAPI/h/BPatch_basicBlock.h"
#include "common/h/String.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/function.h"


#include "common/h/Types.h"
#include "common/h/Symbol.h"
#include "dyninstAPI/src/inst.h"

typedef bool (*functionNameSieve_t)(const char *test,void *data);
#define RH_SEPERATOR '/'

/*
 * List of supported languages.
 *
 */
typedef enum { lang_Unknown,
	       lang_Assembly,
	       lang_C,
	       lang_CPlusPlus,
	       lang_GnuCPlusPlus,
	       lang_Fortran,
	       lang_Fortran_with_pretty_debug,
	       lang_CMFortran
	       } supportedLanguages;

enum { EntryPt, CallPt, ReturnPt, OtherPt };
class point_ {
public:
   point_(): point(0), index(0), type(0) {};
   point_(instPoint *p, unsigned i, unsigned t): point(p), index(i), type(t)
       {  };
   instPoint *point;
   unsigned index;
   unsigned type;
};

/* contents of line number field if line is unknown */
#define UNKNOWN_LINE	0

#define TAG_LIB_FUNC	0x1
#define TAG_IO_OUT	0x2
#define TAG_IO_IN       0x4
#define TAG_MSG_SEND	0x8
#define TAG_MSG_RECV    0x10
#define TAG_SYNC_FUNC	0x20
#define TAG_CPU_STATE	0x40	/* does the func block waiting for ext. event */
#define TAG_MSG_FILT    0x80

#define DYN_MODULE "DYN_MODULE"
#define EXTRA_MODULE "EXTRA_MODULE"
#define USER_MODULE "USER_MODULE"
#define LIBRARY_MODULE	"LIBRARY_MODULE"

class image;
class lineTable;
class image_func;
class image_variable;
class Frame;
class ExceptionBlock;

class pdmodule;
class module;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class instPoint;

class int_variable {
    // Should subclass this and function off the same thing...

 private:
    int_variable() {};
 public:
    int_variable(image_variable *var, 
                 Address base,
                 mapped_module *mod);

    int_variable(int_variable *parVar, mapped_module *child);

    Address getAddress() const { return addr_; }
    // Can variables have multiple names?
    const pdvector<pdstring> &prettyNameVector() const;
    const pdvector<pdstring> &symTabNameVector() const;
    mapped_module *mod() const { return mod_; };

    Address addr_;
    unsigned size_;
    // type?
    image_variable *ivar_;

    mapped_module *mod_;
};

class image_variable {
 private:
    image_variable() {};
 public:
    image_variable(Address offset,
                   const pdstring &name,
                   const pdmodule *mod);

    Address getOffset() const;
    const pdvector<pdstring> &symTabNameVector() const;
    const pdvector<pdstring> &prettyNameVector() const;

    bool addSymTabName(const pdstring &);
    bool addPrettyName(const pdstring &);

    const pdmodule *pdmod() const { return pdmod_; }

    pdvector<pdstring> symTabNames_;
    pdvector<pdstring> prettyNames_;

    Address offset_;
    const pdmodule *pdmod_;
};

/* Stores source code to address in text association for modules */
class lineDict {
 public:
   lineDict() : lineMap(uiHash) { }
   ~lineDict() { /* TODO */ }
   void setLineAddr (unsigned line, Address addr) { lineMap[line] = addr; }
   inline bool getLineAddr (const unsigned line, Address &adr);

 private:
   dictionary_hash<unsigned, Address> lineMap;
};


class module {
 public:
   module(){}
   module(supportedLanguages lang, Address adr, pdstring &fullNm,
          pdstring &fileNm): fileName_(fileNm), fullName_(fullNm), 
      language_(lang), addr_(adr){}
   virtual ~module(){}

   const pdstring &fileName() const { return fileName_; }
   const pdstring &fullName() const { return fullName_; }
   supportedLanguages language() const { return language_;}
   void setLanguage(supportedLanguages lang) {language_ = lang;}
   Address addr() const { return addr_; }

   virtual pdvector<image_func *> *
      findFunction (const pdstring &name,pdvector<image_func *> *found) = 0;
   // virtual pdvector<image_func *> *
   // findFunctionFromAll(const pdstring &name,
   //		  pdvector<image_func *> *found) = 0;
		
   virtual void define(process *proc) = 0;    // defines module to paradyn
   virtual pdvector<image_func *> *getFunctions() = 0;

 private:
   pdstring fileName_;                   // short file 
   pdstring fullName_;                   // full path to file 
   supportedLanguages language_;
   Address addr_;                      // starting address of module
};

class pdmodule: public module {
   friend class image;
 public:

   pdmodule(supportedLanguages lang, Address adr, pdstring &fullNm,
            pdstring &fileNm, image *e): module(lang,adr,fullNm,fileNm),
#if 0
       // Moved to mapped_module
      hasParsedLineInformation( false ),
      lineInformation(),
#endif
      exec_(e), 

     allFunctionsByMangledName( pdstring::hash ),
     allFunctionsByPrettyName( pdstring::hash )
      {
      }

   void cleanProcessSpecific(process *p);

   image *exec() const { return exec_; }
#ifdef CHECK_ALL_CALL_POINTS
   // JAW -- checking all call points is expensive and may not be necessary
   //    --  if we can do this on-the-fly
   void checkAllCallPoints();
#endif
   void define(process *proc);    // defines module to paradyn

   pdvector<image_func *> * getFunctions();

   pdvector<image_func *> *findFunction (const pdstring &name, 
                                            pdvector<image_func *> *found);
 
   pdvector<image_func *> *findFunctionFromAll(const pdstring &name, 
                  pdvector<image_func *> *found, 
                  bool regex_case_sensitive=true,
                  bool dont_use_regex=false);

   /* We can see more than one function with the same mangled
      name in the same object, because it's OK for different
      modules in the same object to define the same (local) symbol.
      However, we can't always determine module information (for instance,
      libc.a on AIX lacks debug information), which means one of our
      module classes may contain information about an entire object,
      and therefore, multiple functons with the same mangled name. */
   pdvector<image_func *> *findFunctionByMangled(const pdstring &name);

   bool isShared() const;

   void dumpMangled(pdstring &prefix) const;
#if 0
   // Moved to mapped_module class
   bool hasParsedLineInformation;
   LineInformation lineInformation;
   pdstring* processDirectories(pdstring* fn) const;

#if defined(rs6000_ibm_aix4_1)

   void parseLineInformation(pdstring* currentSourceFile,
                             char* symbolName,
                             SYMENT *sym,
                             Address linesfdptr,char* lines,int nlines);
#endif

#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0) && !defined(i386_unknown_nt4_0)
   void parseFileLineInfo(LineInformation &lineInfo, process *proc);
#endif

#endif // if 0

 private:

   image *exec_;                      // what executable it came from 
   lineDict lines_;
   //  list of all found functions in module....
   // pdvector<image_func*> funcs;

   //bool shared_;                      // if image it belongs to is shared lib

 public:

   void addFunction( image_func * func );
   void addTypedPrettyName(image_func *func, const char *prettyName);
   void removeFunction(image_func *func);

 private:

   typedef dictionary_hash< pdstring, image_func * >::iterator FunctionsByMangledNameIterator;
   typedef dictionary_hash< pdstring, pdvector< image_func * > * >::iterator FunctionsByPrettyNameIterator;
   dictionary_hash< pdstring, pdvector<image_func *> * > allFunctionsByMangledName;
   dictionary_hash< pdstring, pdvector<image_func *> * > allFunctionsByPrettyName;

   pdvector <image_func *> allUniqueFunctions;
};




void print_func_vector_by_pretty_name(pdstring prefix,
                                      pdvector<image_func *>*funcs);
void print_module_vector_by_short_name(pdstring prefix,
                                       pdvector<pdmodule*> *mods);
pdstring getModuleName(pdstring constraint);
pdstring getFunctionName(pdstring constraint);

/*
 * symbols we need to find from our RTinst library.  This is how we know
 *   were our inst primatives got loaded as well as the data space variables
 *   we use to put counters/timers and inst trampolines.  An array of these
 *   is placed in the image structure.
 *
 */
class internalSym {
 public:
   internalSym() { }
   internalSym(const Address adr, const pdstring &nm) : name(nm), addr(adr) { }
   Address getAddr() const { return addr;}

 private:
   pdstring name;            /* name as it appears in the symbol table. */
   Address addr;      /* absolute address of the symbol */
};


int rawfuncscmp( image_func*& pdf1, image_func*& pdf2 );

typedef enum {unparsed, symtab, analyzing, analyzed} imageParseState_t;

// modsByFileName
// modsbyFullName
// includedMods
// excludedMods
// allMods
// includedFunctions
// excludedFunctions
// funcsByAddr
// funcsByPretty
// file_
// name_
// codeOffset_
// codeLen_
// dataOffset_
// dataLen_
// linkedFile
// iSymsMap
// allImages
// varsByPretty
// knownJumpTargets
// COMMENTS????
//  Image class contains information about statically and dynamically linked code 
//  belonging to a process....
class image : public codeRange {
   friend class process;
   friend class image_func;

   //
   // ****  PUBLIC MEMBER FUBCTIONS  ****
   //
 public:
   static image *parseImage(const pdstring file);
   static image *parseImage(fileDescriptor &desc); 

   // And to get rid of them if we need to re-parse
   static void removeImage(image *img);

   // "I need another handle!"
   image *clone() { refCount++; return this; }

   // And alternates
   static void removeImage(const pdstring file);
   static void removeImage(fileDescriptor &desc);

   // Fills  in raw_funcs with targets in callTargets
   void parseStaticCallTargets( pdvector< Address >& callTargets,
                                pdvector< image_func* > &raw_funcs,
                                pdmodule* mod );

   bool parseFunction( image_func* pdf, pdvector< Address >& callTargets); 
   image(fileDescriptor &desc, bool &err); 

   void analyzeIfNeeded();

 protected:
   ~image();

   // 7JAN05: go through the removeImage call!
   int destroy() {
     refCount--;
     if (refCount == 0) {
       //delete this;
       // Uncomment that when we have a destructor that works...
     }
     if (refCount < 0)
       assert(0 && "NEGATIVE REFERENCE COUNT FOR IMAGE!");
     return refCount; 
   }
 public:

   // Check the list of symbols returned by the parser, return
   // name/addr pair
   //bool findInternalSymbol(const pdstring &name, const bool warn, internalSym &iSym);

   //Address findInternalAddress (const pdstring &name, const bool warn, bool &err);
   // find the named module  
   pdmodule *findModule(const pdstring &name, bool substring_match = false);

   // Note to self later: find is a const operation, [] isn't, for
   // dictionary access. If we need to make the findFuncBy* methods
   // consts, we can move from [] to find()

   // Find the vector of functions associated with a (demangled) name
   pdvector <image_func *> *findFuncVectorByPretty(const pdstring &name);
   pdvector <image_func *> *findFuncVectorByMangled(const pdstring &name);

   // Find the vector of functions determined by a filter function
   pdvector <image_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    pdvector<image_func *> *found);
   pdvector <image_func *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     pdvector<image_func *> *found);

   image_func *findOnlyOneFunction(const pdstring &name);

#if 0
   // We're not supporting this anymore. If the caller wants a regex,
   // they can get the list of all functions and apply a regex.
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
   // REGEX search functions for Pretty and Mangled function names:
   // Callers can either provide a pre-compiled regex struct, or a
   // string pattern which will then be compiled.  This is set up
   // like this to provide a way for higher level functions to 
   // scan different images with the same compiled pattern -- thus
   // avoiding unnecessary re-compilation overhead.
   //
   // EXPENSIVE TO USE!!  Linearly searches dictionary hashes.  --jaw 01-03
   int findFuncVectorByPrettyRegex(pdvector<image_func *>*, pdstring pattern,
                                   bool case_sensitive = TRUE);
   int findFuncVectorByPrettyRegex(pdvector<image_func *>*, regex_t *);
   int findFuncVectorByMangledRegex(pdvector<image_func *>*, pdstring pattern,
                                    bool case_sensitive = TRUE);
   int findFuncVectorByMangledRegex(pdvector<image_func *>*, regex_t *);
#endif
#endif

   // Given an address (offset into the image), find the function that occupies
   // that address
   image_func *findFuncByOffset(const Address &offset);
   image_func *findFuncByEntry(const Address &entry);
  
   // report modules to paradyn
   void defineModules(process *proc);
  
   //Add an extra pretty name to a known function (needed for handling
   //overloaded functions in paradyn)
   void addTypedPrettyName(image_func *func, const char *typedName);
	
   bool symbolExists(const pdstring &); /* Does the symbol exist in the image? */
   void postProcess(const pdstring);          /* Load .pif file */


   void addJumpTarget(Address addr) {
      if (!knownJumpTargets.defines(addr)) knownJumpTargets[addr] = addr; 
   }

   bool isJumpTarget(Address addr) { 
      return knownJumpTargets.defines(addr); 
   }


   // data member access

   pdstring file() const {return desc_.file();}
   pdstring name() const { return name_;}
   pdstring pathname() const { return pathname_; }
   const fileDescriptor &desc() const { return desc_; }
   Address codeOffset() const { return codeOffset_;}
   Address dataOffset() const { return dataOffset_;}
   Address dataLength() const { return (dataLen_ << 2);} 
   Address codeLength() const { return (codeLen_ << 2);} 


   // codeRange versions
   Address get_address_cr() const { return codeOffset(); }
   unsigned get_size_cr() const { return codeLength(); }

   Address codeValidStart() const { return codeValidStart_; }
   Address codeValidEnd() const { return codeValidEnd_; }
   Address dataValidStart() const { return dataValidStart_; }
   Address dataValidEnd() const { return dataValidEnd_; }
   const Object &getObject() const { return linkedFile; }

   // Figure out the address width in the image. Any ideas?
   unsigned getAddressWidth() const { return linkedFile.getAddressWidth(); };

   //Object &getObjectNC() { return linkedFile; } //ccw 27 july 2000 : this is a TERRIBLE hack : 29 mar 2001

   bool isDyninstRTLib() const { return is_libdyninstRT; }

   bool isAOut() const { return is_a_out; }
 
#if defined( arch_x86 )
   inline bool isText(const Address& where ) const;
#endif
   inline bool isCode(const Address &where) const;
   inline bool isData(const Address &where) const;
   inline bool isValidAddress(const Address &where) const;
   inline bool isAllocedCode(const Address &where) const;
   inline bool isAllocedData(const Address &where) const;
   inline bool isAllocedAddress(const Address &where) const;
   //inline const Word get_instruction(Address adr) const;
   inline void *getPtrToOrigInstruction(Address Offset) const;
   // Heh... going by address is a really awful way to work on AIX.
   // Make it explicit.
   void *getPtrToData(Address offset) const;

   inline bool isNativeCompiler() const { return nativeCompiler; }

   // Return symbol table information
   inline bool symbol_info(const pdstring& symbol_name, Symbol& ret);


   const pdvector<image_func*> &getAllFunctions();
   const pdvector<image_variable*> &getAllVariables();

   // Get functions that were in a symbol table (exported funcs)
   const pdvector<image_func *> &getExportedFunctions() const;
   // And when we parse, we might find more:
   const pdvector<image_func *> &getCreatedFunctions();

   const pdvector<image_variable *> &getExportedVariables() const;
   const pdvector<image_variable *> &getCreatedVariables();

   const pdvector<Symbol> getHeaps() const;

   // Tests if a symbol starts at a given point
   bool hasSymbolAtPoint(Address point) const;

#ifndef BPATCH_LIBRARY

   // get all modules, including excluded ones....
   const pdvector<pdmodule *> &getAllModules();

   // Called from the mdl -- lists of functions to look for
   static void watch_functions(pdstring& name, pdvector<pdstring> *vs, bool is_lib,
                               pdvector<image_func*> *updateDict);
#else

#endif 
   const pdvector<pdmodule*> &getModules();

   //
   //  ****  PUBLIC DATA MEMBERS  ****
   //

   Address get_main_call_addr() const { return main_call_addr_; }
 private:

   void findModByAddr (const Symbol &lookUp, pdvector<Symbol> &mods,
                       pdstring &modName, Address &modAddr, 
                       const pdstring &defName);


   // Remove a function from the lists of instrumentable functions, once already inserted.
   int removeFuncFromInstrumentable(image_func *func);

   image_func *makeOneFunction(pdvector<Symbol> &mods,
                                const Symbol &lookUp);

   // addMultipleFunctionNames is called after the argument image_func
   // has been found to have a conflicting address a function that has already
   // been found and inserted into the funcsByAddr map.  We assume that this
   // is a case of function name aliasing, so we merely take the names from the duplicate
   // function and add them as additional names for the one that was already found.
   void addMultipleFunctionNames(image_func *dup);
				

   //
   //  ****  PRIVATE MEMBERS FUNCTIONS  ****
   //

   // private methods for findind an excluded function by name or
   //  address....
   //bool find_excluded_function(const pdstring &name,
   //    pdvector<image_func*> &retList);
   //image_func *find_excluded_function(const Address &addr);

   // A helper routine for removeInstrumentableFunc -- removes function from specified hash
   void removeFuncFromNameHash(image_func *func, pdstring &fname,
                               dictionary_hash<pdstring, pdvector<image_func *> > *func_hash);

#ifdef CHECK_ALL_CALL_POINTS
   void checkAllCallPoints();
#endif

#if 0
   bool addInternalSymbol(const pdstring &str, const Address symValue);
#endif
   // creates the module if it does not exist
   pdmodule *getOrCreateModule (const pdstring &modName, const Address modAddr);
   pdmodule *newModule(const pdstring &name, const Address addr, supportedLanguages lang);

   bool symbolsToFunctions(pdvector<Symbol> &mods, pdvector<image_func *> *raw_funcs);

   //bool addAllVariables();
   bool addSymtabVariables();
   // And all those we find via analysis... like, how?
   bool addDiscoveredVariables();

   void getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs);
   void setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs);

   // We have a _lot_ of lookup types; this handles proper entry
   void enterFunctionInTables(image_func *func, pdmodule *mod);

   bool buildFunctionLists(pdvector<image_func *> &raw_funcs);
   bool analyzeImage();
   //
   //  ****  PRIVATE DATA MEMBERS  ****
   //

   fileDescriptor desc_; /* file descriptor (includes name) */
   pdstring name_;		 /* filename part of file, no slashes */
   pdstring pathname_;      /* file name with path */

   Address codeOffset_;
   unsigned codeLen_;
   Address dataOffset_;
   unsigned dataLen_;

   Address codeValidStart_;
   Address codeValidEnd_;
   Address dataValidStart_;
   Address dataValidEnd_;

   bool is_libdyninstRT;
   bool is_a_out;
   Address main_call_addr_; // address of call to main()

   bool nativeCompiler;

   // data from the symbol table 
   Object linkedFile;

   //dictionary_hash <pdstring, internalSym*> iSymsMap;   // internal RTinst symbols

   // A vector of all images. Used to avoid duplicating
   // an "image" that already exists.
   static pdvector<image*> allImages;

   // knownJumpTargets: the addresses in this image that are known to 
   // be targets of jumps. It is used to check points with multiple 
   // instructions.
   // This is a subset of the addresses that are actually targets of jumps.
   dictionary_hash<Address, Address> knownJumpTargets;

   pdvector<pdmodule *> _mods;

   // The dictionary of all symbol addresses in the image. We use it as a hack
   // on x86 to scavenge some bytes past a function exit for the exit-point
   // instrumentation
   dictionary_hash<Address, unsigned> knownSymAddrs;
   //
   // Hash Tables of Functions....
   //

   // functions by address for all modules.  Only contains instrumentable
   //  funtions.
   codeRangeTree funcsByRange;
   // Keep this one as well for O(1) entry lookups
   dictionary_hash <Address, image_func *> funcsByEntryAddr;
   // note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   dictionary_hash <pdstring, pdvector<image_func*>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   dictionary_hash <pdstring, pdvector<image_func*>*> funcsByMangled;
   // A way to iterate over all the functions efficiently
   pdvector<image_func *> everyUniqueFunction;
   // We make an initial list of functions based off the symbol table,
   // and may create more when we actually analyze. Keep track of
   // those created ones so we can distinguish them if necessary
   pdvector<image_func *> createdFunctions;
   // And the counterpart "ones that are there right away"
   pdvector<image_func *> exportedFunctions;
   pdvector<image_variable *> everyUniqueVariable;
   pdvector<image_variable *> createdVariables;
   pdvector<image_variable *> exportedVariables;

   // And a list of heaps found in the image. Specially handled via mapped_object.
   pdvector<Symbol> infHeapSymbols;

   // TODO -- get rid of one of these
   // Note : as of 971001 (mcheyney), these hash tables only 
   //  hold entries in includedMods --> this implies that
   //  it may sometimes be necessary to do a linear sort
   //  through excludedMods if searching for a module which
   //  was excluded....
   dictionary_hash <pdstring, pdmodule *> modsByFileName;
   dictionary_hash <pdstring, pdmodule*> modsByFullName;
   // Variables indexed by pretty (non-mangled) name
   dictionary_hash <pdstring, pdvector <image_variable *> *> varsByPretty;
   dictionary_hash <pdstring, pdvector <image_variable *> *> varsByMangled;
   dictionary_hash <Address, image_variable *> varsByAddr;

   int refCount;

   imageParseState_t parseState_;
};

/*
 * a definition of a library function that we may wish to identify.  This is
 *   how we describe it to the symbol table parser, not how it appears in
 *   the symbol table.  Library functions are placed in a pseudo module 
 *   named LIBRARY_MODULE. 
 *
 */


class libraryFunc {
 public:
   libraryFunc(const pdstring n, unsigned t) : name(n), tags(t) { }
   unsigned getTags() const { return tags;}

 private:
   pdstring name;
   unsigned tags;
};

/**
 * Used to represent something like a C++ try/catch block.  
 * Currently only used on Linux/x86
 **/
class ExceptionBlock {
 public:
   ExceptionBlock(Address tStart, unsigned tSize, Address cStart) :
     tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) {}
   ExceptionBlock(Address cStart) :
      tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) {}
   ExceptionBlock(const ExceptionBlock &eb) : tryStart_(eb.tryStart_), 
      trySize_(eb.trySize_), catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) {}
   ExceptionBlock() : tryStart_(0), trySize_(0), catchStart_(0), hasTry_(false) {}
   ~ExceptionBlock() {}

   bool hasTry() const { return hasTry_; }
   Address tryStart() const { return tryStart_; }
   Address tryEnd() const { return tryStart_ + trySize_; }
   Address trySize() const { return trySize_; }
   Address catchStart() const { return catchStart_; }
   Address contains(Address a) const 
      { return (a >= tryStart_ && a < tryStart_ + trySize_); }

 private:
   Address tryStart_;
   unsigned trySize_;
   Address catchStart_;
   bool hasTry_;
}; 


inline bool lineDict::getLineAddr (const unsigned line, Address &adr) {
   if (!lineMap.defines(line)) {
      return false;
   } else {
      adr = lineMap[line];
      return true;
   }
}

#if 0
inline const Word image::get_instruction(Address adr) const{
   // TODO remove assert
   // assert(isValidAddress(adr));
   if(!isValidAddress(adr)){
      // logLine("address not valid in get_instruction\n");
      return 0;
   }

   if (isCode(adr)) {
      adr -= codeOffset_;
      adr >>= 2;
      const Word *inst = linkedFile.code_ptr();
      return (inst[adr]);
   } else if (isData(adr)) {
      adr -= dataOffset_;
      adr >>= 2;
      const Word *inst = linkedFile.data_ptr();
      return (inst[adr]);
   } else {
      abort();
      return 0;
   }
}
#endif

// return a pointer to the instruction at address adr
inline void *image::getPtrToOrigInstruction(Address offset) const {
   assert(isValidAddress(offset));
   if (isCode(offset)) {
      offset -= codeOffset_;
      unsigned char *inst = (unsigned char *)linkedFile.code_ptr();
      return (void *)(&inst[offset]);
   } else if (isData(offset)) {
      offset -= dataOffset_;
      unsigned char *inst = (unsigned char *)linkedFile.data_ptr();
      return (void *)(&inst[offset]);
   } else {
      abort();
      return 0;
   }
}


// Address must be in code or data range since some code may end up
// in the data segment
inline bool image::isValidAddress(const Address &where) const{
    return (instruction::isAligned(where) && (isCode(where) || isData(where)));
}

inline bool image::isAllocedAddress(const Address &where) const{
    return (instruction::isAligned(where) && (isAllocedCode(where) || isAllocedData(where)));
}

inline bool image::isCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeOffset_) && (where < (codeOffset_+(codeLen_<<2))));
}

#if defined( arch_x86 )
inline bool image::isText( const Address &where) const
{
    return ( linkedFile.isText( where ) );
}
#endif
inline bool image::isData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataOffset_) && (where < (dataOffset_+(dataLen_<<2))));
}

inline bool image::isAllocedCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeValidStart_) && (where < codeValidEnd_));
}

inline bool image::isAllocedData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataValidStart_) && (where < dataValidEnd_));
}

inline bool image::symbol_info(const pdstring& symbol_name, Symbol &ret_sym) {

   /* We temporarily adopt the position that an image has exactly one
      symbol per name.  While local functions (etc) make this untrue, it
      dramatically minimizes the amount of rewriting. */
   pdvector< Symbol > symbols;
   linkedFile.get_symbols( symbol_name, symbols );
   if( symbols.size() == 1 ) {
       ret_sym = symbols[0];
       return true;
       } else if ( symbols.size() > 1 ) {
       return false;
    }

   return false;
}

 
int instPointCompare( instPoint*& ip1, instPoint*& ip2 );
int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 );

#endif

