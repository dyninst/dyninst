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

#include <map>
#include <utility>
#include <vector>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#if !defined(os_windows)
#include <regex.h>
#endif

#include <set>

#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/codeRange.h"
#include "dyninstAPI/src/infHeap.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/h/BPatch_enums.h"

#include <unordered_map>

#if defined(os_linux)||defined(os_freebsd)
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

typedef bool (*functionNameSieve_t)(const char *test,void *data);
#define RH_SEPERATOR '/'

#define UNKNOWN_LINE	0

#define TAG_LIB_FUNC	0x1
#define TAG_IO_OUT	0x2
#define TAG_IO_IN       0x4
#define TAG_MSG_SEND	0x8
#define TAG_MSG_RECV    0x10
#define TAG_SYNC_FUNC	0x20
#define TAG_CPU_STATE	0x40
#define TAG_MSG_FILT    0x80

#define DYN_MODULE "DYN_MODULE"
#define EXTRA_MODULE "EXTRA_MODULE"
#define USER_MODULE "USER_MODULE"
#define LIBRARY_MODULE	"LIBRARY_MODULE"

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

class DynCFGFactory;
class DynParseCallback;

class PCProcess;

class fileDescriptor {
 public:
    fileDescriptor();

    fileDescriptor(string file, Address code, Address data) : 
#if defined(os_windows)
		procHandle_(INVALID_HANDLE_VALUE),
		fileHandle_(INVALID_HANDLE_VALUE),
#endif
		file_(file),
        code_(code),
        data_(data),
        pid_(0),
        length_(0)
        {}

    fileDescriptor(string file, Address code, Address data, 
                   Address length, void* ) :
#if defined(os_windows)
		procHandle_(INVALID_HANDLE_VALUE),
		fileHandle_(INVALID_HANDLE_VALUE),
#endif
		file_(file),
        code_(code),
        data_(data),
        pid_(0),
        length_(length)
        {}

     bool operator==(const fileDescriptor &fd) const {
         return IsEqual(fd );
     }

     bool operator!=(const fileDescriptor &fd) const {
         return !IsEqual(fd);
     }

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
     int pid() const { return pid_; }
//     Address loadAddr() const { return loadAddr_; }
     void setLoadAddr(Address a) { 
        code_ += a;
        data_ += a;
     }
     void setCode(Address c) { code_ = c; }
     void setData(Address d) { data_ = d; }
     void setMember(string member) { member_ = member; }
     void setPid(int pid) { pid_ = pid; }
     Address length() const { return length_; }
     void* rawPtr();

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
     int pid_;
     Address length_;

     bool IsEqual( const fileDescriptor &fd ) const;
};

class image_variable {
 private:
 public:
    image_variable() {}
    image_variable(SymtabAPI::Variable *var,
    		   pdmodule *mod);

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

    SymtabAPI::Variable *var_{};
    pdmodule *pdmod_{};
    
};

std::string getModuleName(std::string constraint);
std::string getFunctionName(std::string constraint);

int rawfuncscmp( parse_func*& pdf1, parse_func*& pdf2 );

typedef enum {unparsed, symtab, analyzing, analyzed} imageParseState_t;

class image : public codeRange {
   friend class image_variable;
   friend class DynCFGFactory;
 public:
   static image *parseImage(fileDescriptor &desc, 
                            BPatch_hybridMode mode,
                            bool parseGaps);

   static void removeImage(image *img);

   image *clone() {
      refCount++; 
      return this; 
   }

   image(fileDescriptor &desc, bool &err, 
         BPatch_hybridMode mode,
         bool parseGaps);

   void analyzeIfNeeded();
   bool isParsed() { return parseState_ == analyzed; }
   parse_func* addFunction(Address functionEntryAddr, const char *name=NULL);

   pdmodule *getOrCreateModule (SymtabAPI::Module *mod);

 protected:
   ~image();

   int destroy();
 public:
   pdmodule *findModule(const string &name, bool wildcard = false);

   const std::vector <parse_func *> *findFuncVectorByPretty(const std::string &name);
   const std::vector <parse_func *> *findFuncVectorByMangled(const std::string &name);
   const std::vector <image_variable *> *findVarVectorByPretty(const std::string &name);
   const std::vector <image_variable *> *findVarVectorByMangled(const std::string &name);

   std::vector <parse_func *> *findFuncVectorByPretty(functionNameSieve_t bpsieve, 
                                                    void *user_data, 
                                                    std::vector<parse_func *> *found);
   std::vector <parse_func *> *findFuncVectorByMangled(functionNameSieve_t bpsieve, 
                                                     void *user_data, 
                                                     std::vector<parse_func *> *found);

   parse_func *findFuncByEntry(const Address &entry);
   int findFuncs(const Address offset, set<ParseAPI::Function *> & funcs);
   int findBlocksByAddr(const Address offset, set<ParseAPI::Block*> & blocks);
  
   void addTypedPrettyName(parse_func *func, const char *typedName);

   image_variable* createImageVariable(Address offset, std::string name, int size, pdmodule *mod);
	
   bool symbolExists(const std::string &);
   void postProcess(const std::string);

   string file() const {return desc_.file();}
   string name() const { return name_;}
   string pathname() const { return pathname_; }
   const fileDescriptor &desc() const { return desc_; }
   Address imageOffset() const { return imageOffset_;}
   Address dataOffset() const { return dataOffset_;}
   Address dataLength() const { return dataLen_;} 
   Address imageLength() const { return imageLen_;} 

   void setImageLength(Address newlen);

   Address get_address() const { return imageOffset(); }
   unsigned get_size() const { return imageLength(); }

   SymtabAPI::Symtab *getObject() const { return linkedFile; }
   ParseAPI::CodeObject *codeObject() const { return obj_; }

   bool isDyninstRTLib() const { return is_libdyninstRT; }
   bool isExecutable() const { return getObject()->isExec(); }
   bool isSharedLibrary() const { return getObject()->isSharedLibrary(); }
   bool isSharedObject() const { 
    return (getObject()->getObjectType() == SymtabAPI::obj_SharedLib); 
   }
   bool isRelocatableObj() const { 
    return (getObject()->getObjectType() == SymtabAPI::obj_RelocatableFile);
   }

   bool getExecCodeRanges(std::vector<std::pair<Address, Address> > &ranges);

   SymtabAPI::Symbol *symbol_info(const std::string& symbol_name);
   bool findSymByPrefix(const std::string &prefix, std::vector<SymtabAPI::Symbol *> &ret);

   const ParseAPI::CodeObject::funclist &getAllFunctions();
   const std::vector<image_variable*> &getAllVariables();

   BPatch_hybridMode hybridMode() const { return mode_; }
   bool hasNewBlocks() const { return 0 < newBlocks_.size(); }
   const vector<parse_block*> & getNewBlocks() const;
   void clearNewBlocks();
   void register_codeBytesUpdateCB(void *cb_arg0)
       { cb_arg0_ = cb_arg0; }
   void * cb_arg0() const { return cb_arg0_; }

   // XXX const std::vector<parse_func *> &getCreatedFunctions();

   const std::vector<image_variable *> &getExportedVariables() const;
   const std::vector<image_variable *> &getCreatedVariables();

   bool getInferiorHeaps(vector<pair<string, Address> > &codeHeaps,
                         vector<pair<string, Address> > &dataHeaps);

   bool getModules(vector<pdmodule *> &mods);

    int getNextBlockID() { return nextBlockID_++; }

   Address get_main_call_addr() const { return main_call_addr_; }

   void * getErrFunc() const { return (void *) dyninst_log_perror; }

   std::unordered_map<Address, std::string> *getPltFuncs();
   void getPltFuncs(std::map<Address, std::string> &out);
#if defined(arch_power)
   bool updatePltFunc(parse_func *caller_func, Address stub_targ);
#endif

   void destroy(ParseAPI::Block *);
   void destroy(ParseAPI::Edge *);
   void destroy(ParseAPI::Function *);


 private:
   void findModByAddr (const SymtabAPI::Symbol *lookUp, vector<SymtabAPI::Symbol *> &mods,
                       string &modName, Address &modAddr, 
                       const string &defName);

   int findMain();

   bool determineImageType();
   bool addSymtabVariables();

   void setModuleLanguages(std::unordered_map<std::string, SymtabAPI::supportedLanguages> *mod_langs);

   void enterFunctionInTables(parse_func *func);

   bool buildFunctionLists(std::vector<parse_func *> &raw_funcs);
   void analyzeImage();

   void insertPLTParseFuncMap(const std::string&, parse_func*);

   bool parseGaps() { return parseGaps_; }

 private:
   fileDescriptor desc_;
   string name_;
   string pathname_;

   Address imageOffset_;
   unsigned imageLen_;
   Address dataOffset_;
   unsigned dataLen_;

   std::unordered_map<Address, parse_func *> activelyParsing;

   //Address codeValidStart_;
   //Address codeValidEnd_;
   //Address dataValidStart_;
   //Address dataValidEnd_;

   bool is_libdyninstRT;
   Address main_call_addr_;

   SymtabAPI::Symtab *linkedFile;
#if defined(os_linux) || defined(os_freebsd)
   SymtabAPI::Archive *archive;
#endif

   Dyninst::ParseAPI::CodeObject * obj_;
   Dyninst::ParseAPI::SymtabCodeSource * cs_;
   Dyninst::ParseAPI::SymtabCodeSource::hint_filt *filt;
   DynCFGFactory * img_fact_;
   DynParseCallback * parse_cb_;
   void *cb_arg0_;

   map<SymtabAPI::Module *, pdmodule *> mods_;


   std::vector<image_variable *> everyUniqueVariable;
   std::vector<image_variable *> createdVariables;
   std::vector<image_variable *> exportedVariables;

   std::vector<image_parRegion *> parallelRegions;

   int nextBlockID_;

   dyn_hash_map <string, pdmodule *> modsByFileName;

   std::unordered_map<Address, std::string> *pltFuncs;

   std::unordered_map <Address, image_variable *> varsByAddr;

   vector<pair<string, Address> > codeHeaps_;
   vector<pair<string, Address> > dataHeaps_;

   vector<parse_block*> newBlocks_;
   bool trackNewBlocks_;

   int refCount;
   imageParseState_t parseState_;
   bool parseGaps_;
   BPatch_hybridMode mode_;
   Dyninst::Architecture arch;

   dyn_hash_map<string, parse_func*> plt_parse_funcs;

};

class pdmodule {
   friend class image;
 public:
   pdmodule(SymtabAPI::Module *mod, image *e)
   	    : mod_(mod), exec_(e) {}

   void cleanProcessSpecific(PCProcess *p);

   bool getFunctions(std::vector<parse_func *> &funcs);

   bool findFunction(const std::string &name,
                      std::vector<parse_func *> &found);

   bool getVariables(std::vector<image_variable *> &vars);

   bool findFunctionByMangled (const std::string &name,
                               std::vector<parse_func *> &found);
   bool findFunctionByPretty (const std::string &name,
                              std::vector<parse_func *> &found);
   void dumpMangled(std::string &prefix) const;
   const string &fileName() const;
   SymtabAPI::supportedLanguages language() const;
   Address addr() const;
   bool isShared() const;

   SymtabAPI::Module *mod();

   image *imExec() const { return exec_; }
   
 private:
   SymtabAPI::Module *mod_;
   image *exec_;
};

class BPatch_basicBlock;

int instPointCompare( instPoint*& ip1, instPoint*& ip2 );
int basicBlockCompare( BPatch_basicBlock*& bb1, BPatch_basicBlock*& bb2 );

#endif

