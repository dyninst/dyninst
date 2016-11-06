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

// $Id: symtab.h,v 1.214 2008/10/27 17:23:53 mlam Exp $

#ifndef SYMTAB_HDR
#define SYMTAB_HDR
#define REGEX_CHARSET "^*|?"

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#if !defined(os_windows)
#include <regex.h>
#endif

#include <set>

#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/infHeap.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/h/BPatch_enums.h"

#include "common/src/Vector.h"
#include <unordered_map>
#include "common/src/List.h"
#include "common/src/Types.h"

#if defined(os_linux) || defined(os_freebsd)
#include "symtabAPI/h/Archive.h"
#endif

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Module.h"
#include "symtabAPI/h/Type.h"
#include "symtabAPI/h/Function.h"
#include "symtabAPI/h/Variable.h"

#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/CodeSource.h"

#include "dyninstAPI/src/Parsing.h"

using namespace std;
using namespace Dyninst;

typedef bool (*functionNameSieve_t)(const char *test, void *data);
#define RH_SEPERATOR '/'

/* contents of line number field if line is unknown */
#define UNKNOWN_LINE 0

#define TAG_LIB_FUNC 0x1
#define TAG_IO_OUT 0x2
#define TAG_IO_IN 0x4
#define TAG_MSG_SEND 0x8
#define TAG_MSG_RECV 0x10
#define TAG_SYNC_FUNC 0x20
#define TAG_CPU_STATE 0x40 /* does the func block waiting for ext. event */
#define TAG_MSG_FILT 0x80

#define DYN_MODULE "DYN_MODULE"
#define EXTRA_MODULE "EXTRA_MODULE"
#define USER_MODULE "USER_MODULE"
#define LIBRARY_MODULE "LIBRARY_MODULE"

#define NUMBER_OF_MAIN_POSSIBILITIES 8
extern char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20];

class image;
class lineTable;
class parse_func;
class image_variable;

class image_parRegion;

class Frame;

class pdmodule;
class module;
class BPatch_flowGraph;
class BPatch_loopTreeNode;
class instPoint;

// ParseAPI classes
class DynCFGFactory;
class DynParseCallback;

class PCProcess;

// File descriptor information
class fileDescriptor {
 public:
  static string emptyString;
  // Vector requires an empty constructor
  fileDescriptor();

  // Some platforms have split code and data. If yours is not one of them,
  // hand in the same address for code and data.
  fileDescriptor(string file, Address code, Address data, bool isShared = false,
                 Address dynamic = 0)
      :
#if defined(os_windows)
        procHandle_(INVALID_HANDLE_VALUE),
        fileHandle_(INVALID_HANDLE_VALUE),
#endif
        file_(file),
        member_(emptyString),
        code_(code),
        data_(data),
        dynamic_(dynamic),
        shared_(isShared),
        pid_(0),
        length_(0),
        rawPtr_(NULL) {
  }

  // ctor for non-files
  fileDescriptor(string file, Address code, Address data, Address length,
                 void *rawPtr, bool isShared = false, Address dynamic = 0)
      :
#if defined(os_windows)
        procHandle_(INVALID_HANDLE_VALUE),
        fileHandle_(INVALID_HANDLE_VALUE),
#endif
        file_(file),
        member_(emptyString),
        code_(code),
        data_(data),
        dynamic_(dynamic),
        shared_(isShared),
        pid_(0),
        length_(length),
        rawPtr_(rawPtr) {
  }

  ~fileDescriptor() {}

  bool operator==(const fileDescriptor &fd) const { return IsEqual(fd); }

  bool operator!=(const fileDescriptor &fd) const { return !IsEqual(fd); }

  // Not quite the same as above; is this the same on-disk file
  bool isSameFile(const fileDescriptor &fd) const {
    if ((file_ == fd.file_) && (member_ == fd.member_)) return true;
    ;
    return false;
  }

  const string &file() const { return file_; }
  const string &member() const { return member_; }
  Address code() const { return code_; }
  Address data() const { return data_; }
  bool isSharedObject() const { return shared_; }
  int pid() const { return pid_; }
  //     Address loadAddr() const { return loadAddr_; }
  Address dynamic() const { return dynamic_; }
  void setLoadAddr(Address a) {
    code_ += a;
    data_ += a;
  }
  void setCode(Address c) { code_ = c; }
  void setData(Address d) { data_ = d; }
  void setMember(string member) { member_ = member; }
  void setPid(int pid) { pid_ = pid; }
  void setIsShared(bool shared) { shared_ = shared; }
  Address length() const { return length_; }  // only for non-files
  void *rawPtr();                             // only for non-files

#if defined(os_windows)
  void setHandles(HANDLE proc, HANDLE file) {
    procHandle_ = proc;
    fileHandle_ = file;
  }

  HANDLE procHandle() const { return procHandle_; }
  HANDLE fileHandle() const { return fileHandle_; }

#endif

 private:
#if defined(os_windows)
  HANDLE procHandle_;
  HANDLE fileHandle_;
#endif
  string file_;
  string member_;
  Address code_;
  Address data_;
  Address dynamic_;  // Used on Linux, address of dynamic section.
  bool shared_;      // TODO: Why is this here? We should probably use the image
                     // version instead...
  int pid_;
  Address length_;  // set only if this is not really a file
  void *rawPtr_;    // set only if this is not really a file

  bool IsEqual(const fileDescriptor &fd) const;
};

class image_variable {
 private:
 public:
  image_variable() {}
  image_variable(SymtabAPI::Variable *var, pdmodule *mod);

  Address getOffset() const;

  string symTabName() const { return var_->getFirstSymbol()->getMangledName(); }
  SymtabAPI::Aggregate::name_iter symtab_names_begin() const;
  SymtabAPI::Aggregate::name_iter symtab_names_end() const;
  SymtabAPI::Aggregate::name_iter pretty_names_begin() const;
  SymtabAPI::Aggregate::name_iter pretty_names_end() const;

  bool addSymTabName(const std::string &, bool isPrimary = false);
  bool addPrettyName(const std::string &, bool isPrimary = false);

  pdmodule *pdmod() const { return pdmod_; }
  SymtabAPI::Variable *svar() const { return var_; }

  SymtabAPI::Variable *var_;
  pdmodule *pdmod_;
};

/* Stores source code to address in text association for modules */
class lineDict {
 public:
  lineDict() {}
  ~lineDict() { /* TODO */
  }
  void setLineAddr(unsigned line, Address addr) { lineMap[line] = addr; }
  inline bool getLineAddr(const unsigned line, Address &adr);

 private:
  std::unordered_map<unsigned, Address> lineMap;
};

std::string getModuleName(std::string constraint);
std::string getFunctionName(std::string constraint);

int rawfuncscmp(parse_func *&pdf1, parse_func *&pdf2);

typedef enum { unparsed, symtab, analyzing, analyzed } imageParseState_t;

//  Image class contains information about statically and
//  dynamically linked code belonging to a process
class image : public codeRange {
  friend class image_variable;
  friend class DynCFGFactory;

 public:
  static image *parseImage(fileDescriptor &desc, BPatch_hybridMode mode,
                           bool parseGaps);

  // And to get rid of them if we need to re-parse
  static void removeImage(image *img);

  // "I need another handle!"
  image *clone() {
    refCount++;
    return this;
  }

  image(fileDescriptor &desc, bool &err, BPatch_hybridMode mode,
        bool parseGaps);

  void analyzeIfNeeded();
  bool isParsed() { return parseState_ == analyzed; }
  parse_func *addFunction(Address functionEntryAddr, const char *name = NULL);

  // creates the module if it does not exist
  pdmodule *getOrCreateModule(SymtabAPI::Module *mod);

 protected:
  ~image();

  // 7JAN05: go through the removeImage call!
  int destroy();

 public:
  // find the named module
  pdmodule *findModule(const string &name, bool wildcard = false);

  // Find the vector of functions associated with a (demangled) name
  // Returns internal pointer, so label as const
  const pdvector<parse_func *> *findFuncVectorByPretty(const std::string &name);
  const pdvector<parse_func *> *findFuncVectorByMangled(
      const std::string &name);
  // Variables: nearly identical
  const pdvector<image_variable *> *findVarVectorByPretty(
      const std::string &name);
  const pdvector<image_variable *> *findVarVectorByMangled(
      const std::string &name);

  // Find the vector of functions determined by a filter function
  pdvector<parse_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve,
                                                 void *user_data,
                                                 pdvector<parse_func *> *found);
  pdvector<parse_func *> *findFuncVectorByMangled(
      functionNameSieve_t bpsieve, void *user_data,
      pdvector<parse_func *> *found);

  /*********************************************************************/
  /**** Function lookup (by name or address) routines               ****/
  /****                                                             ****/
  /**** Overlapping region objects MUST NOT use these routines      ****/
  /*********************************************************************/
  // Find a function that begins at a particular address
  parse_func *findFuncByEntry(const Address &entry);
  // Code sharing allows multiple functions to overlap a given point
  int findFuncs(const Address offset, set<ParseAPI::Function *> &funcs);
  // Find the basic blocks that overlap the given address
  int findBlocksByAddr(const Address offset, set<ParseAPI::Block *> &blocks);

  // Add an extra pretty name to a known function (needed for handling
  // overloaded functions in paradyn)
  void addTypedPrettyName(parse_func *func, const char *typedName);

  // Create an image variable (e.g., malloced variable). Creates the
  // variable and adds to appropriate data structures.
  image_variable *createImageVariable(Address offset, std::string name,
                                      int size, pdmodule *mod);

  bool symbolExists(const std::string &); /* Check symbol existence */
  void postProcess(const std::string);    /* Load .pif file */

  // data member access

  string file() const { return desc_.file(); }
  string name() const { return name_; }
  string pathname() const { return pathname_; }
  const fileDescriptor &desc() const { return desc_; }
  Address imageOffset() const { return imageOffset_; }
  Address dataOffset() const { return dataOffset_; }
  Address dataLength() const { return dataLen_; }
  Address imageLength() const { return imageLen_; }

  void setImageLength(Address newlen);

  // codeRange interface implementation
  Address get_address() const { return imageOffset(); }
  unsigned get_size() const { return imageLength(); }

  SymtabAPI::Symtab *getObject() const { return linkedFile; }
  ParseAPI::CodeObject *codeObject() const { return obj_; }

  bool isDyninstRTLib() const { return is_libdyninstRT; }
  bool isAOut() const { return is_a_out; }
  bool isSharedObj() const {
    return (getObject()->getObjectType() == SymtabAPI::obj_SharedLib);
  }
  bool isRelocatableObj() const {
    return (getObject()->getObjectType() == SymtabAPI::obj_RelocatableFile);
  }

  bool getExecCodeRanges(std::vector<std::pair<Address, Address> > &ranges);

  bool isNativeCompiler() const { return nativeCompiler; }

  // Return symbol table information
  SymtabAPI::Symbol *symbol_info(const std::string &symbol_name);
  // And used for finding inferior heaps.... hacky, but effective.
  bool findSymByPrefix(const std::string &prefix,
                       pdvector<SymtabAPI::Symbol *> &ret);

  const ParseAPI::CodeObject::funclist &getAllFunctions();
  const pdvector<image_variable *> &getAllVariables();

  //-----------DEFENSIVE-MODE CODE------------//
  BPatch_hybridMode hybridMode() const { return mode_; }
  // element removal

  bool hasNewBlocks() const { return 0 < newBlocks_.size(); }
  const vector<parse_block *> &getNewBlocks() const;
  void clearNewBlocks();
  // callback that updates our view the binary's raw code bytes
  void register_codeBytesUpdateCB(void *cb_arg0) { cb_arg0_ = cb_arg0; }
  void *cb_arg0() const { return cb_arg0_; }

  // And when we parse, we might find more:
  // FIXME might be convenient to access HINT-only functions easily
  // XXX const pdvector<parse_func *> &getCreatedFunctions();

  const pdvector<image_variable *> &getExportedVariables() const;
  const pdvector<image_variable *> &getCreatedVariables();

  bool getInferiorHeaps(vector<pair<string, Address> > &codeHeaps,
                        vector<pair<string, Address> > &dataHeaps);

  bool getModules(vector<pdmodule *> &mods);

  int getNextBlockID() { return nextBlockID_++; }

  Address get_main_call_addr() const { return main_call_addr_; }

  void *getErrFunc() const { return (void *)dyninst_log_perror; }

  std::unordered_map<Address, std::string> *getPltFuncs();
  void getPltFuncs(std::map<Address, std::string> &out);
#if defined(arch_power)
  bool updatePltFunc(parse_func *caller_func, Address stub_targ);
#endif

  // Object deletion (defensive mode)
  void destroy(ParseAPI::Block *);
  void destroy(ParseAPI::Edge *);
  void destroy(ParseAPI::Function *);

 private:
  void findModByAddr(const SymtabAPI::Symbol *lookUp,
                     vector<SymtabAPI::Symbol *> &mods, string &modName,
                     Address &modAddr, const string &defName);

  //
  //  ****  PRIVATE MEMBERS FUNCTIONS  ****
  //

  // Platform-specific discovery of the "main" function
  // FIXME There is a minor but fundamental design flaw that
  //       needs to be resolved wrt findMain returning void.
  int findMain();

  bool determineImageType();
  bool addSymtabVariables();

  void getModuleLanguageInfo(
      std::unordered_map<std::string, SymtabAPI::supportedLanguages>
          *mod_langs);
  void setModuleLanguages(
      std::unordered_map<std::string, SymtabAPI::supportedLanguages>
          *mod_langs);

  // We have a _lot_ of lookup types; this handles proper entry
  void enterFunctionInTables(parse_func *func);

  bool buildFunctionLists(pdvector<parse_func *> &raw_funcs);
  void analyzeImage();

  //
  //  **** GAP PARSING SUPPORT  ****
  bool parseGaps() { return parseGaps_; }

  //
  //  ****  PRIVATE DATA MEMBERS  ****
  //
 private:
  fileDescriptor desc_; /* file descriptor (includes name) */
  string name_;         /* filename part of file, no slashes */
  string pathname_;     /* file name with path */

  Address imageOffset_;
  unsigned imageLen_;
  Address dataOffset_;
  unsigned dataLen_;

  std::unordered_map<Address, parse_func *> activelyParsing;

  // Address codeValidStart_;
  // Address codeValidEnd_;
  // Address dataValidStart_;
  // Address dataValidEnd_;

  bool is_libdyninstRT;
  bool is_a_out;
  Address main_call_addr_;  // address of call to main()

  bool nativeCompiler;

  // data from the symbol table
  SymtabAPI::Symtab *linkedFile;
#if defined(os_linux) || defined(os_freebsd)
  SymtabAPI::Archive *archive;
#endif

  // ParseAPI
  Dyninst::ParseAPI::CodeObject *obj_;
  Dyninst::ParseAPI::SymtabCodeSource *cs_;
  Dyninst::ParseAPI::SymtabCodeSource::hint_filt *filt;
  DynCFGFactory *img_fact_;
  DynParseCallback *parse_cb_;
  void *cb_arg0_;  // argument for mapped_object callback

  map<SymtabAPI::Module *, pdmodule *> mods_;

  pdvector<image_variable *> everyUniqueVariable;
  pdvector<image_variable *> createdVariables;
  pdvector<image_variable *> exportedVariables;

  // This contains all parallel regions on the image
  // These line up with the code generated to support OpenMP, UPC, Titanium, ...
  pdvector<image_parRegion *> parallelRegions;

  // unique (by image) numbering of basic blocks
  int nextBlockID_;

  // TODO -- get rid of one of these
  // Note : as of 971001 (mcheyney), these hash tables only
  //  hold entries in includedMods --> this implies that
  //  it may sometimes be necessary to do a linear sort
  //  through excludedMods if searching for a module which
  //  was excluded....
  dyn_hash_map<string, pdmodule *> modsByFileName;
  dyn_hash_map<string, pdmodule *> modsByFullName;

  // "Function" symbol names that are PLT entries or the equivalent
  // FIXME remove
  std::unordered_map<Address, std::string> *pltFuncs;

  std::unordered_map<Address, image_variable *> varsByAddr;

  vector<pair<string, Address> > codeHeaps_;
  vector<pair<string, Address> > dataHeaps_;

  // new element tracking
  vector<parse_block *> newBlocks_;
  bool trackNewBlocks_;

  int refCount;
  imageParseState_t parseState_;
  bool parseGaps_;
  BPatch_hybridMode mode_;
  Dyninst::Architecture arch;
};

class pdmodule {
  friend class image;

 public:
  pdmodule(SymtabAPI::Module *mod, image *e) : mod_(mod), exec_(e) {}

  void cleanProcessSpecific(PCProcess *p);

  bool getFunctions(pdvector<parse_func *> &funcs);

  bool findFunction(const std::string &name, pdvector<parse_func *> &found);

  bool getVariables(pdvector<image_variable *> &vars);

  /* We can see more than one function with the same mangled
     name in the same object, because it's OK for different
     modules in the same object to define the same (local) symbol.
     However, we can't always determine module information (for instance,
     libc.a on AIX lacks debug information), which means one of our
     module classes may contain information about an entire object,
     and therefore, multiple functons with the same mangled name. */
  bool findFunctionByMangled(const std::string &name,
                             pdvector<parse_func *> &found);
  bool findFunctionByPretty(const std::string &name,
                            pdvector<parse_func *> &found);
  void dumpMangled(std::string &prefix) const;
  const string &fileName() const;
  const string &fullName() const;
  SymtabAPI::supportedLanguages language() const;
  Address addr() const;
  bool isShared() const;

  SymtabAPI::Module *mod();

  image *imExec() const { return exec_; }

 private:
  SymtabAPI::Module *mod_;
  image *exec_;
};

inline bool lineDict::getLineAddr(const unsigned line, Address &adr) {
  auto iter = lineMap.find(line);
  if (iter == lineMap.end()) return false;
  adr = iter->second;
  return true;
}

class BPatch_basicBlock;

int instPointCompare(instPoint *&ip1, instPoint *&ip2);
int basicBlockCompare(BPatch_basicBlock *&bb1, BPatch_basicBlock *&bb2);

#endif
