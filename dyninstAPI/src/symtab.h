/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 
// $Id: symtab.h,v 1.214 2008/10/27 17:23:53 mlam Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR
#define REGEX_CHARSET "^*|?"

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11)
#include <regex.h>
#endif

#include <set>

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/List.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/util.h"

#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/InstructionSource.h"

#include "dyninstAPI/src/infHeap.h"

#include "common/h/Types.h"
#include "dyninstAPI/src/inst.h"

#if defined(rs6000_ibm_aix4_1)||defined(rs6000_ibm_aix5_1)||defined(os_linux)||defined(os_solaris)
#include "symtabAPI/h/Archive.h"
#endif

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Module.h"
#include "symtabAPI/h/Type.h"
#include "symtabAPI/h/Function.h"
#include "symtabAPI/h/Variable.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

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

class pdmodule;
class module;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class instPoint;

// File descriptor information
class fileDescriptor {
 public:
    static string emptyString;
    // Vector requires an empty constructor
    fileDescriptor();

    // Some platforms have split code and data. If yours is not one of them,
    // hand in the same address for code and data.
    fileDescriptor(string file, Address code, Address data, 
                   bool isShared=false, Address dynamic=0) :
        file_(file),
        member_(emptyString),
        code_(code),
        data_(data),
        dynamic_(dynamic),
        shared_(isShared),
        pid_(0),
        loadAddr_(0)
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
     
     const string &file() const { return file_; }
     const string &member() const { return member_; }
     Address code() const { return code_; }
     Address data() const { return data_; }
     bool isSharedObject() const { return shared_; }
     int pid() const { return pid_; }
     Address loadAddr() const { return loadAddr_; }
     Address dynamic() const { return dynamic_; }
     void setLoadAddr(Address a);
     void setCode(Address c) { code_ = c; }
     void setData(Address d) { data_ = d; }
     void setMember(string member) { member_ = member; }
     void setPid(int pid) { pid_ = pid; }
     void setIsShared(bool shared) { shared_ = shared; }

#if defined(os_windows)
     // Windows gives you file handles. Since I collapsed the fileDescriptors
     // to avoid having to track allocated/deallocated memory, these moved here.
     fileDescriptor(string name, Address baseAddr, HANDLE procH, HANDLE fileH,
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
     string file_;
     // AIX: two strings define an object.
     string member_;
     Address code_;
     Address data_;
     Address dynamic_; //Used on Linux, address of dynamic section.
     bool shared_;      // TODO: Why is this here? We should probably use the image version instead...
     int pid_;
     Address loadAddr_;

     bool IsEqual( const fileDescriptor &fd ) const;
};

class image_variable {
 private:
 public:
    image_variable() {}
    image_variable(Variable *var,
    		   pdmodule *mod);

    Address getOffset() const;

    const string &symTabName() const { return var_->getAllMangledNames()[0]; }
    const vector<string>&  symTabNameVector() const;
    const vector<string>& prettyNameVector() const;

    bool addSymTabName(const std::string &, bool isPrimary = false);
    bool addPrettyName(const std::string &, bool isPrimary = false);

    pdmodule *pdmod() const { return pdmod_; }
    Variable *svar() const { return var_; }

    Variable *var_;
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

void print_func_vector_by_pretty_name(std::string prefix,
                                      pdvector<image_func *>*funcs);
void print_module_vector_by_short_name(std::string prefix,
                                       pdvector<pdmodule*> *mods);
std::string getModuleName(std::string constraint);
std::string getFunctionName(std::string constraint);

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
// file_
// name_
// imageOffset_
// imageLen_
// dataOffset_
// dataLen_
// linkedFile
// iSymsMap
// allImages
// varsByPretty
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
   static image *parseImage(const std::string file);
   static image *parseImage(fileDescriptor &desc, bool parseGaps=false);

   // And to get rid of them if we need to re-parse
   static void removeImage(image *img);

   // "I need another handle!"
   image *clone() { refCount++; return this; }

   // And alternates
   static void removeImage(const string file);
   static void removeImage(fileDescriptor &desc);

   image(fileDescriptor &desc, bool &err, bool parseGaps=false);

   void analyzeIfNeeded();

   image_func* addFunctionStub(Address functionEntryAddr, const char *name=NULL);


 protected:
   ~image();

   // 7JAN05: go through the removeImage call!
   int destroy();
 public:

   // Check the list of symbols returned by the parser, return
   // name/addr pair

   // find the named module  
   pdmodule *findModule(const string &name, bool wildcard = false);

   // Find the vector of functions associated with a (demangled) name
   // Returns internal pointer, so label as const
   const pdvector <image_func *> *findFuncVectorByPretty(const std::string &name);
   const pdvector <image_func *> *findFuncVectorByMangled(const std::string &name);
   // Variables: nearly identical
   const pdvector <image_variable *> *findVarVectorByPretty(const std::string &name);
   const pdvector <image_variable *> *findVarVectorByMangled(const std::string &name);

   // Find the vector of functions determined by a filter function
   pdvector <image_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    pdvector<image_func *> *found);
   pdvector <image_func *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     pdvector<image_func *> *found);

   // Find a function that begins at a particular address
   image_func *findFuncByEntry(const Address &entry);

   // FIXME This function is poorly named: it searches for a *function*
   //       that overlaps the given offset. Also, it fails to account
   //       for functions overlapping a particular range due to code
   //       sharing, so is broken by design.
   codeRange *findCodeRangeByOffset(const Address &offset);

   // Find the basic block that overlaps the given address
   image_basicBlock *findBlockByAddr(const Address &addr);
  
   //Add an extra pretty name to a known function (needed for handling
   //overloaded functions in paradyn)
   void addTypedPrettyName(image_func *func, const char *typedName);

   // Create an image variable (e.g., malloced variable). Creates the
   // variable and adds to appropriate data structures.
   image_variable* createImageVariable(Address offset, std::string name, int size, pdmodule *mod);
	
   bool symbolExists(const std::string &); /* Check symbol existence */
   void postProcess(const std::string);          /* Load .pif file */

   // data member access

   string file() const {return desc_.file();}
   string name() const { return name_;}
   string pathname() const { return pathname_; }
   const fileDescriptor &desc() const { return desc_; }
   Address imageOffset() const { return imageOffset_;}
   Address dataOffset() const { return dataOffset_;}
   Address dataLength() const { return dataLen_;} 
   Address imageLength() const { return imageLen_;} 


   // codeRange interface implementation
   Address get_address() const { return imageOffset(); }
   unsigned get_size() const { return imageLength(); }
   virtual void *getPtrToInstruction(Address offset) const;
   virtual void *getPtrToInstruction(Address offset, const image_func *context) const;
   // Heh... going by address is a really awful way to work on AIX.
   // Make it explicit.
   void *getPtrToData(Address offset) const;
   void * getPtrToDataInText( Address offset ) const;

   Symtab *getObject() const { return linkedFile; }

   // Figure out the address width in the image. Any ideas?
   virtual unsigned getAddressWidth() const { return linkedFile->getAddressWidth(); };

   bool isDyninstRTLib() const { return is_libdyninstRT; }

   bool isAOut() const { return is_a_out; }

   bool isSharedObj() const { return (getObject()->getObjectType() == obj_SharedLib); }
   bool isRelocatableObj() const { return (getObject()->getObjectType() == obj_RelocatableFile); }
 
   bool isCode(const Address &where) const;
   bool isData(const Address &where) const;
   virtual bool isValidAddress(const Address &where) const;
   virtual bool isExecutableAddress(const Address &where) const;
   bool isAligned(const Address where) const;

   bool isNativeCompiler() const { return nativeCompiler; }

   // Return symbol table information
   Symbol *symbol_info(const std::string& symbol_name);
   // And used for finding inferior heaps.... hacky, but effective.
   bool findSymByPrefix(const std::string &prefix, pdvector<Symbol *> &ret);

   const std::set<image_func*,image_func::compare> &getAllFunctions();
   const pdvector<image_variable*> &getAllVariables();

   // Get functions that were in a symbol table (exported funcs)
   const pdvector<image_func *> &getExportedFunctions() const;
   // And when we parse, we might find more:
   const pdvector<image_func *> &getCreatedFunctions();

   const pdvector<image_variable *> &getExportedVariables() const;
   const pdvector<image_variable *> &getCreatedVariables();

   bool getInferiorHeaps(vector<pair<string, Address> > &codeHeaps,
                         vector<pair<string, Address> > &dataHeaps);

   bool getModules(vector<pdmodule *> &mods);

    int getNextBlockID() { return nextBlockID_++; }

   //
   //  ****  PUBLIC DATA MEMBERS  ****
   //

   Address get_main_call_addr() const { return main_call_addr_; }

   void * getErrFunc() const { return (void *) dyninst_log_perror; }

   dictionary_hash<Address, std::string> *getPltFuncs();
#if defined(arch_power)
   bool updatePltFunc(image_func *caller_func, image_func *stub_func);
#endif

   // This method is invoked after parsing a function to record it in tables
   // and to update other symtab-level data structures, like mangled names
   void recordFunction(image_func *);

   // This method is invoked to find the global constructors function and add a
   // symbol for the function if the image has no symbols
   bool findGlobalConstructorFunc();
   bool findGlobalDestructorFunc();

 private:

   void findModByAddr (const Symbol *lookUp, vector<Symbol *> &mods,
                       string &modName, Address &modAddr, 
                       const string &defName);


   // Remove a function from the lists of instrumentable functions, once already inserted.
   int removeFuncFromInstrumentable(image_func *func);

   image_func *makeImageFunction(Function *lookUp);


   //
   //  ****  PRIVATE MEMBERS FUNCTIONS  ****
   //

   // Platform-specific discovery of the "main" function
   // FIXME There is a minor but fundamental design flaw that
   //       needs to be resolved wrt findMain returning void.
   void findMain();

#ifdef CHECK_ALL_CALL_POINTS
   void checkAllCallPoints();
#endif

   // creates the module if it does not exist
   pdmodule *getOrCreateModule (Module *mod);

   bool symbolsToFunctions(pdvector<image_func *> &raw_funcs);


   //bool addAllVariables();
   bool addSymtabVariables();

   // And all those we find via analysis... like, how?
   bool addDiscoveredVariables();

   void getModuleLanguageInfo(dictionary_hash<std::string, supportedLanguages> *mod_langs);
   void setModuleLanguages(dictionary_hash<std::string, supportedLanguages> *mod_langs);

   // We have a _lot_ of lookup types; this handles proper entry
   void enterFunctionInTables(image_func *func);

   bool buildFunctionLists(pdvector<image_func *> &raw_funcs);
   bool analyzeImage();
   bool parseGaps() { return parseGaps_; }

   //
   //  **** GAP PARSING SUPPORT  ****
#if defined(cap_stripped_binaries)
   bool compute_gap(
        Address,
        set<image_func *, image_func::compare>::const_iterator &,
        Address &, Address &);
   
   bool gap_heuristics(Address addr); 
   bool gap_heuristic_GCC(Address addr);
   bool gap_heuristic_MSVS(Address addr);
#endif

   //
   //  ****  PRIVATE DATA MEMBERS  ****
   //
 private:
   fileDescriptor desc_; /* file descriptor (includes name) */
   string name_;		 /* filename part of file, no slashes */
   string pathname_;      /* file name with path */

   Address imageOffset_;
   unsigned imageLen_;
   Address dataOffset_;
   unsigned dataLen_;

   dictionary_hash<Address, image_func *> activelyParsing;

   //Address codeValidStart_;
   //Address codeValidEnd_;
   //Address dataValidStart_;
   //Address dataValidEnd_;

   bool is_libdyninstRT;
   bool is_a_out;
   Address main_call_addr_; // address of call to main()

   bool nativeCompiler;

   // data from the symbol table 
   Symtab *linkedFile;
#if defined (os_aix) || defined(os_linux) || defined(os_solaris)
   Archive *archive;
#endif

   map<Module *, pdmodule *> mods_;

   //
   // Hash Tables of Functions....
   //

   // functions by address for all modules.  Only instrumentable functions
   codeRangeTree funcsByRange;
   // Keep this one as well for O(1) entry lookups
   dictionary_hash <Address, image_func *> funcsByEntryAddr;

   // Populated during symbol table parsing; these are the functions
   // that are stored in "exportedFunctions" after a successful parse
   pdvector<image_func *> symtabCandidateFuncs;

   // A way to iterate over all the functions efficiently
   std::set<image_func *,image_func::compare> everyUniqueFunction;

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

   // Basic blocks by spanned address range
   codeRangeTree basicBlocksByRange;

   // unique (by image) numbering of basic blocks
   int nextBlockID_;

   // TODO -- get rid of one of these
   // Note : as of 971001 (mcheyney), these hash tables only 
   //  hold entries in includedMods --> this implies that
   //  it may sometimes be necessary to do a linear sort
   //  through excludedMods if searching for a module which
   //  was excluded....
   dyn_hash_map <string, pdmodule *> modsByFileName;
   dyn_hash_map <string, pdmodule*> modsByFullName;

   // "Function" symbol names that are PLT entries or the equivalent
   dictionary_hash<Address, std::string> *pltFuncs;

   dictionary_hash <Address, image_variable *> varsByAddr;

   vector<pair<string, Address> > codeHeaps_;
   vector<pair<string, Address> > dataHeaps_;

   int refCount;
   imageParseState_t parseState_;
   bool parseGaps_;

   vector< pair<image_basicBlock *, image_func *> > reparse_shared;
};


class pdmodule {
   friend class image;
 public:
   pdmodule(Module *mod, image *e)
   	    : mod_(mod), exec_(e) {}

   void cleanProcessSpecific(process *p);

#ifdef CHECK_ALL_CALL_POINTS
   // JAW -- checking all call points is expensive and may not be necessary
   //    --  if we can do this on-the-fly
   void checkAllCallPoints();
#endif
   bool getFunctions(pdvector<image_func *> &funcs);

   bool findFunction(const std::string &name,
                      pdvector<image_func *> &found);

   bool getVariables(pdvector<image_variable *> &vars);

   /* We can see more than one function with the same mangled
      name in the same object, because it's OK for different
      modules in the same object to define the same (local) symbol.
      However, we can't always determine module information (for instance,
      libc.a on AIX lacks debug information), which means one of our
      module classes may contain information about an entire object,
      and therefore, multiple functons with the same mangled name. */
   bool findFunctionByMangled (const std::string &name,
                               pdvector<image_func *> &found);
   bool findFunctionByPretty (const std::string &name,
                              pdvector<image_func *> &found);
   void dumpMangled(std::string &prefix) const;
   const string &fileName() const;
   const string &fullName() const;
   supportedLanguages language() const;
   Address addr() const;
   bool isShared() const;
   // Control flow targets that fail isCode or isValidAddress checks,
   // or are statically unresolvable
   const std::set<image_instPoint*> &getUnresolvedControlFlow();
   void addUnresolvedControlFlow(image_instPoint* badPt);

   Module *mod();

   image *imExec() const { return exec_; }
   
 private:
   std::set<image_instPoint*> unresolvedControlFlow;
   Module *mod_;
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

