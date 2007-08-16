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
 
// $Id: symtab.h,v 1.199 2007/08/16 20:43:48 bill Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR
#define REGEX_CHARSET "^*|?"

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11)
#include <regex.h>
#endif

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/List.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/LineInformation.h"
#include "common/h/String.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/InstructionSource.h"


#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"

#if defined(rs6000_ibm_aix4_1)||defined(rs6000_ibm_aix5_1)
#include "symtabAPI/h/Dyn_Archive.h"
#else
#include "symtabAPI/h/Dyn_Symtab.h"
#endif

typedef bool (*functionNameSieve_t)(const char *test,void *data);
#define RH_SEPERATOR '/'

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

class image_parRegion;

class Frame;
class ExceptionBlock;

class pdmodule;
class module;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class instPoint;

// File descriptor information
class fileDescriptor {
    static pdstring emptyString;
 public:
    // Vector requires an empty constructor
    fileDescriptor();

    //
    // Some platforms have split code and data. If yours is not one of them,
    // hand in the same address for code and data.
    fileDescriptor(pdstring file, Address code, Address data, bool isShared) :
        file_(file),
        member_(emptyString),
        code_(code),
        data_(data),
        shared_(isShared),
        pid_(0)
        {}

     ~fileDescriptor() {}

     bool operator==(const fileDescriptor &fd) const {
         return IsEqual(fd );
     }

     bool operator!=(const fileDescriptor &fd) const {
         return !IsEqual(fd);
     }

     // Not quite the same as above; is this the same on-disk file
     bool isSameFile(const fileDescriptor &fd) const {
         if ((file_ == fd.file_) &&
             (member_ == fd.member_))
             return true;;
         return false;
     }

     const pdstring &file() const { return file_; }
     const pdstring &member() const { return member_; }
     Address code() const { return code_; };
     Address data() const { return data_; };
     bool isSharedObject() const { return shared_; }
     int pid() const { return pid_; }
     Address loadAddr() const { return loadAddr_; };
     
     void setLoadAddr(Address a);
     void setMember(pdstring member) { member_ = member; }
     void setPid(int pid) { pid_ = pid; }

#if defined(os_windows)
     // Windows gives you file handles. Since I collapsed the fileDescriptors
     // to avoid having to track allocated/deallocated memory, these moved here.
     fileDescriptor(pdstring name, Address baseAddr, HANDLE procH, HANDLE fileH,
                    bool isShared, Address loadAddr) :
         file_(name), code_(baseAddr), data_(baseAddr),
         procHandle_(procH), fileHandle_(fileH),
         shared_(isShared), pid_(0), loadAddr_(loadAddr) {}
     HANDLE procHandle() const { return procHandle_; }
     HANDLE fileHandle() const { return fileHandle_; }

 private:
     HANDLE procHandle_;
     HANDLE fileHandle_;
 public:
#endif


 private:
     pdstring file_;
     // AIX: two strings define an object.
     pdstring member_;
     Address code_;
     Address data_;
     bool shared_;
     int pid_;
     Address loadAddr_;

     bool IsEqual( const fileDescriptor &fd ) const;
};

class image_variable {
 private:
    image_variable() {};
 public:
    image_variable(Address offset,
                   const pdstring &name,
                   pdmodule *mod);

    image_variable(Dyn_Symbol *sym,
    		   pdmodule *mod);

    Address getOffset() const;

    const string &symTabName() const { return sym_->getName(); }
    const vector<string>&  symTabNameVector() const;
    const vector<string>& prettyNameVector() const;

    bool addSymTabName(const pdstring &, bool isPrimary = false);
    bool addPrettyName(const pdstring &, bool isPrimary = false);

    pdmodule *pdmod() const { return pdmod_; }
    Dyn_Symbol *symbol() const { return sym_; }

    Dyn_Symbol *sym_;	
    pdmodule *pdmod_;
    
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

void print_func_vector_by_pretty_name(pdstring prefix,
                                      pdvector<image_func *>*funcs);
void print_module_vector_by_short_name(pdstring prefix,
                                       pdvector<pdmodule*> *mods);
pdstring getModuleName(pdstring constraint);
pdstring getFunctionName(pdstring constraint);

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
class image : public codeRange, public InstructionSource {
   friend class process;
   friend class image_func; // Access to "add<foo>Name"
   friend class image_variable;

   //
   // ****  PUBLIC MEMBER FUNCTIONS  ****
   //
 public:
    // remove after testing
   void DumpAllStats();

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
                pdvector< Address > &newTargets,
                dictionary_hash< Address, image_func * > &preParseStubs);

   bool parseFunction( image_func* pdf, pdvector< Address >& callTargets,
                dictionary_hash< Address, image_func * >& preParseStubs); 

   image(fileDescriptor &desc, bool &err); 

   void analyzeIfNeeded();

#if defined(i386_unknown_nt4_0)
   const pdvector<Address> &getPossibleMains() const   { return possible_mains; }
#endif

 protected:
   ~image();

   // 7JAN05: go through the removeImage call!
   int destroy();
 public:

   // Check the list of symbols returned by the parser, return
   // name/addr pair

   //Address findInternalAddress (const pdstring &name, const bool warn, bool &err);
   // find the named module  
   pdmodule *findModule(const pdstring &name, bool wildcard = false);

   // Note to self later: find is a const operation, [] isn't, for
   // dictionary access. If we need to make the findFuncBy* methods
   // consts, we can move from [] to find()

   // Find the vector of functions associated with a (demangled) name
   // Returns internal pointer, so label as const
   const pdvector <image_func *> *findFuncVectorByPretty(const pdstring &name);
   const pdvector <image_func *> *findFuncVectorByMangled(const pdstring &name);
   // Variables: nearly identical
   const pdvector <image_variable *> *findVarVectorByPretty(const pdstring &name);
   const pdvector <image_variable *> *findVarVectorByMangled(const pdstring &name);



   // Find the vector of functions determined by a filter function
   pdvector <image_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    pdvector<image_func *> *found);
   pdvector <image_func *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     pdvector<image_func *> *found);

   image_func *findOnlyOneFunction(const pdstring &name);

   // Given an address (offset into the image), find the function that occupies
   // that address
   image_func *findFuncByOffset(const Address &offset);
   // (Possibly) faster version checking only entry address
   image_func *findFuncByEntry(const Address &entry);

   // And raw version
   codeRange *findCodeRangeByOffset(const Address &offset);

   // Blocks by address
   image_basicBlock *findBlockByAddr(const Address &addr);
  
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
   Address dataLength() const { return dataLen_;} 
   Address codeLength() const { return codeLen_;} 


   // codeRange versions
   Address get_address_cr() const { return codeOffset(); }
   unsigned get_size_cr() const { return codeLength(); }
   virtual void *getPtrToInstruction(Address offset) const;
   // Heh... going by address is a really awful way to work on AIX.
   // Make it explicit.
   void *getPtrToData(Address offset) const;
   void * getPtrToDataInText( Address offset ) const;

   Dyn_Symtab *getObject() const { return linkedFile; }

   // Figure out the address width in the image. Any ideas?
   virtual unsigned getAddressWidth() const { return linkedFile->getAddressWidth(); };

   //Object &getObjectNC() { return linkedFile; } //ccw 27 july 2000 : this is a TERRIBLE hack : 29 mar 2001

   bool isDyninstRTLib() const { return is_libdyninstRT; }

   bool isAOut() const { return is_a_out; }
 
#if defined( arch_x86 )
   bool isText(const Address& where ) const;
#endif
   bool isCode(const Address &where) const;
   bool isData(const Address &where) const;
   virtual bool isValidAddress(const Address &where) const;
   bool isAligned(const Address where) const;

   bool isNativeCompiler() const { return nativeCompiler; }

   // Return symbol table information
   bool symbol_info(const pdstring& symbol_name, Dyn_Symbol& ret);
   // And used for finding inferior heaps.... hacky, but effective.
   bool findSymByPrefix(const pdstring &prefix, pdvector<Dyn_Symbol *> &ret);

   const pdvector<image_func*> &getAllFunctions();
   const pdvector<image_variable*> &getAllVariables();

   // Get functions that were in a symbol table (exported funcs)
   const pdvector<image_func *> &getExportedFunctions() const;
   // And when we parse, we might find more:
   const pdvector<image_func *> &getCreatedFunctions();

   const pdvector<image_variable *> &getExportedVariables() const;
   const pdvector<image_variable *> &getCreatedVariables();

   const pdvector<pdmodule*> &getModules();

    int getNextBlockID() { return nextBlockID_++; }

   //
   //  ****  PUBLIC DATA MEMBERS  ****
   //

   Address get_main_call_addr() const { return main_call_addr_; }

   void * getErrFunc() const { return (void *) pd_log_perror; }
 private:

   void findModByAddr (const Dyn_Symbol *lookUp, vector<Dyn_Symbol *> &mods,
                       pdstring &modName, Address &modAddr, 
                       const pdstring &defName);


   // Remove a function from the lists of instrumentable functions, once already inserted.
   int removeFuncFromInstrumentable(image_func *func);

   image_func *makeOneFunction(vector<Dyn_Symbol *> &mods,
                                Dyn_Symbol *lookUp);


   //
   //  ****  PRIVATE MEMBERS FUNCTIONS  ****
   //

   // private methods for findind an excluded function by name or
   //  address....
   //bool find_excluded_function(const pdstring &name,
   //    pdvector<image_func*> &retList);
   //image_func *find_excluded_function(const Address &addr);

   void findMain();

   // A helper routine for removeInstrumentableFunc -- removes function from specified hash
   void removeFuncFromNameHash(image_func *func, pdstring &fname,
                               dictionary_hash<pdstring, pdvector<image_func *> > *func_hash);

#ifdef CHECK_ALL_CALL_POINTS
   void checkAllCallPoints();
#endif

   // creates the module if it does not exist
   pdmodule *getOrCreateModule (const pdstring &modName, const Address modAddr);
   pdmodule *newModule(const pdstring &name, const Address addr, supportedLanguages lang);

   bool symbolsToFunctions(vector<Dyn_Symbol *> &mods, pdvector<image_func *> *raw_funcs);

   //bool addAllVariables();
   bool addSymtabVariables();
   // And all those we find via analysis... like, how?
   bool addDiscoveredVariables();

   void getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs);
   void setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs);

   // We have a _lot_ of lookup types; this handles proper entry
   // wasSymtab: name was found in symbol table. False if invented name
   void enterFunctionInTables(image_func *func, bool wasSymtab);
   //void addFunctionName(image_func *func, const pdstring newName, bool isMangled = false);
   //void addVariableName(image_variable *var, const pdstring newName, bool isMangled = false);

   bool buildFunctionLists(pdvector<image_func *> &raw_funcs);
   bool analyzeImage();
   Address getBaseAddress() { return baseAddr_; }
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

   //Address codeValidStart_;
   //Address codeValidEnd_;
   //Address dataValidStart_;
   //Address dataValidEnd_;

   bool is_libdyninstRT;
   bool is_a_out;
   Address main_call_addr_; // address of call to main()

   bool nativeCompiler;

   // data from the symbol table 
   Dyn_Symtab *linkedFile;


   // A vector of all images. Used to avoid duplicating
   // an "image" that already exists.
   static pdvector<image*> allImages;

   // knownJumpTargets: the addresses in this image that are known to 
   // be targets of jumps. It is used to check points with multiple 
   // instructions.
   // This is a subset of the addresses that are actually targets of jumps.
   dictionary_hash<Address, Address> knownJumpTargets;

   pdvector<pdmodule *> _mods;

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
/*
   dictionary_hash <pdstring, pdvector<image_func*>*> funcsByPretty;
   // Hash table holding functions by mangled name.
   // Should contain same functions as funcsByPretty....
   dictionary_hash <pdstring, pdvector<image_func*>*> funcsByMangled;
   // A way to iterate over all the functions efficiently
*/   
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

   // This contains all parallel regions on the image
   // These line up with the code generated to support OpenMP, UPC, Titanium, ...
   pdvector<image_parRegion *> parallelRegions;

   // The following were added to support parsing
   // (nater) 13.Oct.05
   // basic blocks by address.
   codeRangeTree basicBlocksByRange;
   // Quick lookups
   //dictionary_hash<Address, image_basicBlock *> basicBlocksByAddr;
    int nextBlockID_;

   // TODO -- get rid of one of these
   // Note : as of 971001 (mcheyney), these hash tables only 
   //  hold entries in includedMods --> this implies that
   //  it may sometimes be necessary to do a linear sort
   //  through excludedMods if searching for a module which
   //  was excluded....
   dictionary_hash <pdstring, pdmodule *> modsByFileName;
   dictionary_hash <pdstring, pdmodule*> modsByFullName;
/* 
   // Variables indexed by pretty (non-mangled) name
   dictionary_hash <pdstring, pdvector <image_variable *> *> varsByPretty;
   dictionary_hash <pdstring, pdvector <image_variable *> *> varsByMangled;
*/   
   dictionary_hash <Address, image_variable *> varsByAddr;

#if defined(i386_unknown_nt4_0)
	pdvector<Address> possible_mains; //Addresses of functions that may be main
#endif

   int refCount;
   Address baseAddr_;
   imageParseState_t parseState_;
};


class pdmodule: public Dyn_Module {
   friend class image;
 public:
   pdmodule(supportedLanguages lang, Address adr, pdstring &fullNm, image *e)
            : Dyn_Module(lang,(OFFSET)adr,fullNm.c_str(),e->getObject()), exec_(e){}
   pdmodule(Dyn_Module &mod, image *e) : Dyn_Module(mod), exec_(e) {}

   void cleanProcessSpecific(process *p);

#ifdef CHECK_ALL_CALL_POINTS
   // JAW -- checking all call points is expensive and may not be necessary
   //    --  if we can do this on-the-fly
   void checkAllCallPoints();
#endif
   bool getFunctions(pdvector<image_func *> &funcs);

   bool findFunction(const pdstring &name,
                      pdvector<image_func *> &found);

   bool getVariables(pdvector<image_variable *> &vars);

   /* We can see more than one function with the same mangled
      name in the same object, because it's OK for different
      modules in the same object to define the same (local) symbol.
      However, we can't always determine module information (for instance,
      libc.a on AIX lacks debug information), which means one of our
      module classes may contain information about an entire object,
      and therefore, multiple functons with the same mangled name. */
   bool findFunctionByMangled (const pdstring &name,
                               pdvector<image_func *> &found);
   bool findFunctionByPretty (const pdstring &name,
                              pdvector<image_func *> &found);
   void dumpMangled(pdstring &prefix) const;

   image *imExec() const { return exec_; }
 private:
  image *exec_;
};

inline bool lineDict::getLineAddr (const unsigned line, Address &adr) {
   if (!lineMap.defines(line)) {
      return false;
   } else {
      adr = lineMap[line];
      return true;
   }
}

class BPatch_basicBlock;

int instPointCompare( instPoint*& ip1, instPoint*& ip2 );
int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 );

#endif

