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
 
// $Id: symtab.h,v 1.171 2005/02/03 23:46:53 bernat Exp $

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

#ifndef BPATCH_LIBRARY
#include "paradynd/src/resource.h"

#define CHECK_ALL_CALL_POINTS  // we depend on this for Paradyn
#endif

#include "common/h/Types.h"
#include "common/h/Symbol.h"
#include "dyninstAPI/src/inst.h"

#ifndef mips_unknown_ce2_11 //ccw 8 apr 2001
#include "dyninstAPI/src/FunctionExpansionRecord.h"
class LocalAlterationSet;
#endif


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
class int_function;
class Frame;
class ExceptionBlock;

class pdmodule;
class module;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class instPoint;

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

   pdstring fileName() const { return fileName_; }
   pdstring fullName() const { return fullName_; }
   supportedLanguages language() const { return language_;}
   void setLanguage(supportedLanguages lang) {language_ = lang;}
   Address addr() const { return addr_; }

   virtual pdvector<int_function *> *
      findFunction (const pdstring &name,pdvector<int_function *> *found) = 0;
   // virtual pdvector<int_function *> *
   // findFunctionFromAll(const pdstring &name,
   //		  pdvector<int_function *> *found) = 0;
		
   virtual void define(process *proc) = 0;    // defines module to paradyn
   virtual pdvector<int_function *> *getFunctions() = 0;

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
      lineInformation(NULL),

#ifndef BPATCH_LIBRARY
      modResource(0),
#endif
      exec_(e), 

     allFunctionsByMangledName( pdstring::hash ),
     allFunctionsByPrettyName( pdstring::hash )
      {
      }

   ~pdmodule();

void cleanProcessSpecific(process *p);

   void setLineAddr(unsigned line, Address addr) {
      lines_.setLineAddr(line, addr);}
   bool getLineAddr(unsigned line, Address &addr) { 
      return (lines_.getLineAddr(line, addr)); }

   image *exec() const { return exec_; }
   void mapLines() { }           // line number info is not used now
#ifdef CHECK_ALL_CALL_POINTS
   // JAW -- checking all call points is expensive and may not be necessary
   //    --  if we can do this on-the-fly
   void checkAllCallPoints();
#endif
   void define(process *proc);    // defines module to paradyn

   void updateForFork(process *childProcess, const process *parentProcess);

   pdvector<int_function *> * getFunctions();

   pdvector<int_function *> *findFunction (const pdstring &name, 
                                            pdvector<int_function *> *found);
 
   pdvector<int_function *> *findFunctionFromAll(const pdstring &name, 
                                                  pdvector<int_function *> *found, 
                                                  bool regex_case_sensitive=true);

   // Only one -- otherwise you can't distinguish between them at link time.
   int_function *findFunctionByMangled(const pdstring &name);

   bool isShared() const;
#ifndef BPATCH_LIBRARY
   resource *getResource() { return modResource; }
#endif
   void dumpMangled(char * prefix);

   LineInformation* lineInformation;
   pdstring* processDirectories(pdstring* fn);

#if defined(rs6000_ibm_aix4_1)

   void parseLineInformation(process* proc, 
                             pdstring* currentSourceFile,
                             char* symbolName,
                             SYMENT *sym,
                             Address linesfdptr,char* lines,int nlines);
#endif

#if !defined(mips_sgi_irix6_4) && !defined(alpha_dec_osf4_0) && !defined(i386_unknown_nt4_0)
   void parseFileLineInfo(process *proc);
#endif

   LineInformation* getLineInformation(process *proc);
   void initLineInformation();
   void cleanupLineInformation();


 private:

#ifndef BPATCH_LIBRARY
   resource *modResource;
#endif
   image *exec_;                      // what executable it came from 
   lineDict lines_;
   //  list of all found functions in module....
   // pdvector<int_function*> funcs;

   //bool shared_;                      // if image it belongs to is shared lib

 public:

   void addFunction( int_function * func );
   void addTypedPrettyName(int_function *func, const char *prettyName);
   void removeFunction(int_function *func);

 private:

   typedef dictionary_hash< pdstring, int_function * >::iterator FunctionsByMangledNameIterator;
   typedef dictionary_hash< pdstring, pdvector< int_function * > * >::iterator FunctionsByPrettyNameIterator;
   dictionary_hash< pdstring, int_function *> allFunctionsByMangledName;
   dictionary_hash< pdstring, pdvector< int_function * > * > allFunctionsByPrettyName;

   pdvector <int_function *> allUniqueFunctions;
};




void print_func_vector_by_pretty_name(pdstring prefix,
                                      pdvector<int_function *>*funcs);
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


int rawfuncscmp( int_function*& pdf1, int_function*& pdf2 );

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
   friend class int_function;

   //
   // ****  PUBLIC MEMBER FUBCTIONS  ****
   //
 public:
   static image *parseImage(const pdstring file);
   static image *parseImage(fileDescriptor *desc, Address newBaseAddr = 0); 

   // And to get rid of them if we need to re-parse
   static void removeImage(image *img);

   // "I need another handle!"
   image *clone() { refCount++; return this; }

   // And alternates
   static void removeImage(const pdstring file);
   static void removeImage(fileDescriptor *desc);

   // Cleaning function -- removes all process-dependent info
   void cleanProcessSpecific(process *p);

   // Fills  in raw_funcs with targets in callTargets
   void parseStaticCallTargets( pdvector< Address >& callTargets,
                                pdvector< int_function* > &raw_funcs,
                                pdmodule* mod );

   bool parseFunction( int_function* pdf, pdvector< Address >& callTargets); 
   image(fileDescriptor *desc, bool &err, Address newBaseAddr = 0); 

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
   bool findInternalSymbol(const pdstring &name, const bool warn, internalSym &iSym);

   // Check the list of symbols returned by the parser, return
   // all which start with the given prefix
   bool findInternalByPrefix(const pdstring &prefix, pdvector<Symbol> &found) const;

  
   //Address findInternalAddress (const pdstring &name, const bool warn, bool &err);
   void updateForFork(process *childProcess, const process *parentProcess);

   // find the named module  
   pdmodule *findModule(const pdstring &name);
   pdmodule *findModule(int_function *func);

   // Note to self later: find is a const operation, [] isn't, for
   // dictionary access. If we need to make the findFuncBy* methods
   // consts, we can move from [] to find()

   // Find the vector of functions associated with a (demangled) name
   pdvector <int_function *> *findFuncVectorByPretty(const pdstring &name);
   pdvector <int_function *> *findFuncVectorByMangled(const pdstring &name);

   // Find the vector of functions determined by a filter function
   pdvector <int_function *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    pdvector<int_function *> *found);
   pdvector <int_function *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     pdvector<int_function *> *found);

   int_function *findOnlyOneFunction(const pdstring &name);

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
   // REGEX search functions for Pretty and Mangled function names:
   // Callers can either provide a pre-compiled regex struct, or a
   // string pattern which will then be compiled.  This is set up
   // like this to provide a way for higher level functions to 
   // scan different images with the same compiled pattern -- thus
   // avoiding unnecessary re-compilation overhead.
   //
   // EXPENSIVE TO USE!!  Linearly searches dictionary hashes.  --jaw 01-03
   int findFuncVectorByPrettyRegex(pdvector<int_function *>*, pdstring pattern,
                                   bool case_sensitive = TRUE);
   int findFuncVectorByPrettyRegex(pdvector<int_function *>*, regex_t *);
   int findFuncVectorByMangledRegex(pdvector<int_function *>*, pdstring pattern,
                                    bool case_sensitive = TRUE);
   int findFuncVectorByMangledRegex(pdvector<int_function *>*, regex_t *);
#endif

   // Given an address (offset into the image), find the function that occupies
   // that address
   int_function *findFuncByOffset(const Address &offset);
   int_function *findFuncByEntry(const Address &entry);
  
   // report modules to paradyn
   void defineModules(process *proc);
  
   //Add an extra pretty name to a known function (needed for handling
   //overloaded functions in paradyn)
   void addTypedPrettyName(int_function *func, const char *typedName);
	
   bool symbolExists(const pdstring &); /* Does the symbol exist in the image? */
   void postProcess(const pdstring);          /* Load .pif file */


   void addJumpTarget(Address addr) {
      if (!knownJumpTargets.defines(addr)) knownJumpTargets[addr] = addr; 
   }

   bool isJumpTarget(Address addr) { 
      return knownJumpTargets.defines(addr); 
   }


   // data member access

   pdstring file() const {return desc_->file();}
   pdstring name() const { return name_;}
   pdstring pathname() const { return pathname_; }
   const fileDescriptor *desc() const { return desc_; }
   Address codeOffset() const { return codeOffset_;}
   Address get_address() const { return codeOffset(); }
   Address dataOffset() const { return dataOffset_;}
   Address dataLength() const { return (dataLen_ << 2);} 
   Address codeLength() const { return (codeLen_ << 2);} 
   unsigned get_size() const { return codeLength(); }
   codeRange *copy() const { return new image(*this); }
   Address codeValidStart() const { return codeValidStart_; }
   Address codeValidEnd() const { return codeValidEnd_; }
   Address dataValidStart() const { return dataValidStart_; }
   Address dataValidEnd() const { return dataValidEnd_; }
   const Object &getObject() const { return linkedFile; }

   Object &getObjectNC() { return linkedFile; } //ccw 27 july 2000 : this is a TERRIBLE hack : 29 mar 2001

   bool isDyninstRTLib() const { return is_libdyninstRT; }

   bool isAOut() const { return is_a_out; }

   inline bool isCode(const Address &where) const;
   inline bool isData(const Address &where) const;
   inline bool isValidAddress(const Address &where) const;
   inline bool isAllocedCode(const Address &where) const;
   inline bool isAllocedData(const Address &where) const;
   inline bool isAllocedAddress(const Address &where) const;
   inline const Word get_instruction(Address adr) const;
   inline const unsigned char *getPtrToInstruction(Address adr) const;

   inline bool isNativeCompiler() const { return nativeCompiler; }

   // Return symbol table information
   inline bool symbol_info(const pdstring& symbol_name, Symbol& ret);


   const pdvector<int_function*> &getAllFunctions();
  
   // Tests if a symbol starts at a given point
   bool hasSymbolAtPoint(Address point) const;

#ifndef BPATCH_LIBRARY

   // get all modules, including excluded ones....
   const pdvector<pdmodule *> &getAllModules();

   // Called from the mdl -- lists of functions to look for
   static void watch_functions(pdstring& name, pdvector<pdstring> *vs, bool is_lib,
                               pdvector<int_function*> *updateDict);
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
   int removeFuncFromInstrumentable(int_function *func);

   int_function *makeOneFunction(pdvector<Symbol> &mods,
                                const Symbol &lookUp);

   // addMultipleFunctionNames is called after the argument int_function
   // has been found to have a conflicting address a function that has already
   // been found and inserted into the funcsByAddr map.  We assume that this
   // is a case of function name aliasing, so we merely take the names from the duplicate
   // function and add them as additional names for the one that was already found.
   void addMultipleFunctionNames(int_function *dup);
				

   //
   //  ****  PRIVATE MEMBERS FUNCTIONS  ****
   //

   // private methods for findind an excluded function by name or
   //  address....
   //bool find_excluded_function(const pdstring &name,
   //    pdvector<int_function*> &retList);
   //int_function *find_excluded_function(const Address &addr);

   // A helper routine for removeInstrumentableFunc -- removes function from specified hash
   void removeFuncFromNameHash(int_function *func, pdstring &fname,
                               dictionary_hash<pdstring, pdvector<int_function *> > *func_hash);

#ifdef CHECK_ALL_CALL_POINTS
   void checkAllCallPoints();
#endif

#if 0
   bool addInternalSymbol(const pdstring &str, const Address symValue);
#endif
   // creates the module if it does not exist
   pdmodule *getOrCreateModule (const pdstring &modName, const Address modAddr);
   pdmodule *newModule(const pdstring &name, const Address addr, supportedLanguages lang);

   bool symbolsToFunctions(pdvector<Symbol> &mods, pdvector<int_function *> *raw_funcs);

   bool addAllVariables();
   void getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs);
   void setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs);

   // We have a _lot_ of lookup types; this handles proper entry
   void enterFunctionInTables(int_function *func, pdmodule *mod);

   bool buildFunctionLists(pdvector<int_function *> &raw_funcs);
   bool analyzeImage();
   //
   //  ****  PRIVATE DATA MEMBERS  ****
   //

   fileDescriptor *desc_; /* file descriptor (includes name) */
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
   dictionary_hash <Address, int_function *> funcsByEntryAddr;
   // note, a prettyName is not unique, it may map to a function appearing
   // in several modules.  Also only contains instrumentable functions....
   dictionary_hash <pdstring, pdvector<int_function*>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   dictionary_hash <pdstring, pdvector<int_function*>*> funcsByMangled;
   // And a way to iterate over all the functions efficiently
   pdvector<int_function *> everyUniqueFunction;

   // TODO -- get rid of one of these
   // Note : as of 971001 (mcheyney), these hash tables only 
   //  hold entries in includedMods --> this implies that
   //  it may sometimes be necessary to do a linear sort
   //  through excludedMods if searching for a module which
   //  was excluded....
   dictionary_hash <pdstring, pdmodule *> modsByFileName;
   dictionary_hash <pdstring, pdmodule*> modsByFullName;
   // Variables indexed by pretty (non-mangled) name
   dictionary_hash <pdstring, pdvector<pdstring>*> varsByPretty;
 
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


#ifndef BPATCH_LIBRARY
// TODO -- remove this
extern resource *moduleRoot;
#endif

inline bool lineDict::getLineAddr (const unsigned line, Address &adr) {
   if (!lineMap.defines(line)) {
      return false;
   } else {
      adr = lineMap[line];
      return true;
   }
}

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

// return a pointer to the instruction at address adr
inline const unsigned char *image::getPtrToInstruction(Address adr) const {
   assert(isValidAddress(adr));
   if (isCode(adr)) {
      adr -= codeOffset_;
      const unsigned char *inst = (const unsigned char *)linkedFile.code_ptr();
      return (&inst[adr]);
   } else if (isData(adr)) {
      adr -= dataOffset_;
      const unsigned char *inst = (const unsigned char *)linkedFile.data_ptr();
      return (&inst[adr]);
   } else {
      abort();
      return 0;
   }
}


// Address must be in code or data range since some code may end up
// in the data segment
inline bool image::isValidAddress(const Address &where) const{
   return (isAligned(where) && (isCode(where) || isData(where)));
}

inline bool image::isAllocedAddress(const Address &where) const{
   return (isAligned(where) && (isAllocedCode(where) || isAllocedData(where)));
}

inline bool image::isCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeOffset_) && (where < (codeOffset_+(codeLen_<<2))));
}

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

   if (linkedFile.get_symbol(symbol_name, ret_sym))
      return true;

   if (varsByPretty.defines(symbol_name)) {
      pdvector<pdstring> *mangledNames = varsByPretty[symbol_name];
      assert(mangledNames && mangledNames->size() == 1);
      if (linkedFile.get_symbol((*mangledNames)[0], ret_sym))
         return true;
   }

   return false;
}


int instPointCompare( instPoint*& ip1, instPoint*& ip2 );
int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 );

#endif

