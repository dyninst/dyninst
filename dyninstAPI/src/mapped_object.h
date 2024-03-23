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

// $Id: mapped_object.h,v 1.23 2008/10/27 17:23:53 mlam Exp $

#if !defined(_mapped_object_h)
#define _mapped_object_h

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/h/BPatch_enums.h"
#include <list>
#include "dyninstAPI/src/Relocation/DynObject.h"
#include "PCProcess.h"

class block_instance;
class func_instance;
class edge_instance;

#define CHECK_ALL_CALL_POINTS

using namespace std;
using namespace Dyninst;
using Dyninst::PatchAPI::DynCFGMaker;

class mapped_module;

class int_symbol {
 public:
    int_symbol(SymtabAPI::Symbol *sym, Address base) : addr_(base + sym->getOffset()), sym_(sym) {}
    int_symbol() : addr_(0), sym_(NULL) {}

    Address getAddr() const { return addr_; }
    unsigned getSize() const { return sym_->getSize(); }
    string symTabName() const { return sym_->getMangledName(); }
    string prettyName() const { return sym_->getPrettyName(); }
    string typedName() const { return sym_->getTypedName(); }
    const SymtabAPI::Symbol *sym() const { return sym_; }

 private:
    Address addr_;
    const SymtabAPI::Symbol *sym_;
};


class int_variable {
 private:
    int_variable() {}
 public:
    int_variable(image_variable *var,
                 Address base,
                 mapped_module *mod);

    int_variable(int_variable *parVar, mapped_module *child);

    Address getAddress() const { return addr_; }
    string symTabName() const;
    SymtabAPI::Aggregate::name_iter pretty_names_begin() const;
    SymtabAPI::Aggregate::name_iter pretty_names_end() const;
    SymtabAPI::Aggregate::name_iter symtab_names_begin() const;
    SymtabAPI::Aggregate::name_iter symtab_names_end() const;
    
    //const vector<string>& prettyNameVector() const;
    //const vector<string>& symTabNameVector() const;
    mapped_module *mod() const { return mod_; }
    //AddressSpace *as() const { return mod()->proc(); }
    const image_variable *ivar() const { return ivar_; }

    Address addr_{};
    unsigned size_{};
    image_variable *ivar_{};

    mapped_module *mod_{};
};

struct edgeStub {
    edgeStub(block_instance *s, Address t, EdgeTypeEnum y)
    { src = s; trg = t; type = y; }
    edgeStub(block_instance* s, Address t, EdgeTypeEnum y, bool b) :
        src(s), trg(t), type(y), checked(b) { }
    block_instance* src;
    Address trg;
    EdgeTypeEnum type;
    bool checked{};
};


#define 	SHAREDOBJECT_NOCHANGE	0
#define 	SHAREDOBJECT_ADDED	1
#define 	SHAREDOBJECT_REMOVED	2

class mapped_object : public codeRange, public Dyninst::PatchAPI::DynObject {
    friend class mapped_module;
    friend class func_instance;
    friend class block_instance;
    friend class edge_instance;
    friend class DynCFGMaker;
 private:
    mapped_object();
    mapped_object(fileDescriptor fileDesc,
                  image *img,
                  AddressSpace *proc,
                  BPatch_hybridMode mode = BPatch_normalMode);

 public:
    static mapped_object *createMappedObject(fileDescriptor &desc,
                                             AddressSpace *p,
                                             BPatch_hybridMode m = BPatch_normalMode,
                                             bool parseGaps = true);
    static mapped_object *createMappedObject(ProcControlAPI::Library::const_ptr lib,
                                             AddressSpace *p,
                                             BPatch_hybridMode m = BPatch_normalMode,
                                             bool parseGaps = true);


    mapped_object(const mapped_object *par_obj, AddressSpace *child);

    ~mapped_object();

    bool analyze();
    bool isAnalyzed() { return analyzed_; }

    const fileDescriptor &getFileDesc() const { return desc_; }
    const string &fullName() const { return fullName_; }
    string fileName() const;
    Address codeAbs() const;
    Address codeBase() const { return codeBase_; }
    Address imageOffset() const { return parse_img()->imageOffset(); }
    unsigned imageSize() const { return parse_img()->imageLength(); }
    unsigned memoryEnd();

    bool isCode(Address addr) const;
    bool isData(Address addr) const;

    Address getBaseAddress() const { return codeBase(); }

    Address dataAbs() const;
    Address dataBase() const { return dataBase_; }
    Address dataOffset() const { return parse_img()->dataOffset(); }
    unsigned dataSize() const { return parse_img()->dataLength(); }
    Address getTOCBaseAddress() const {return tocBase;}
    void setTOCBaseAddress(Address addr) {tocBase = addr;}

    image *parse_img() const { return image_; }
    bool isSharedLib() const;
    bool isStaticExec() const;
    static bool isSystemLib(const std::string &name);
    bool isMemoryImg() const { return memoryImg_; }

    void setMemoryImg() { memoryImg_ = true; }

    const std::string debugString() const;

    Address get_address() const { return codeAbs(); }
    void *get_local_ptr() const;
    unsigned get_size() const { return imageSize(); }

    AddressSpace *proc() const;

    mapped_module *findModule(string m_name, bool wildcard = false);
    mapped_module *findModule(pdmodule *mod);

    mapped_module *getDefaultModule();

    func_instance *findFuncByEntry(const Address addr);
    func_instance *findFuncByEntry(const block_instance *blk);

    bool getInfHeapList(std::vector<heapDescriptor> &infHeaps);
    void getInferiorHeaps(vector<pair<string, Address> > &infHeaps);

    bool findFuncsByAddr(const Address addr, std::set<func_instance *> &funcs);
    bool findBlocksByAddr(const Address addr, std::set<block_instance *> &blocks);
    block_instance *findBlockByEntry(const Address addr);
    block_instance *findOneBlockByAddr(const Address addr);

    void *getPtrToInstruction(Address addr) const;
    void *getPtrToData(Address addr) const;

    bool getAllFunctions(std::vector<func_instance *> &funcs);
    bool getAllVariables(std::vector<int_variable *> &vars);

    const std::vector<mapped_module *> &getModules();

    BPatch_hybridMode hybridMode() { return analysisMode_; }
    void enableDefensiveMode(bool on = true) {
        analysisMode_ = on ? BPatch_defensiveMode : BPatch_normalMode;
    }
    bool isExploratoryModeOn();
    bool parseNewEdges(const std::vector<edgeStub>& sources);
    bool parseNewFunctions(std::vector<Address> &funcEntryAddrs);
    bool updateCodeBytesIfNeeded(Address entryAddr);
    void updateCodeBytes(const std::list<std::pair<Address,Address> > &owRanges );
    void setCodeBytesUpdated(bool);
    void addProtectedPage(Address pageAddr);
    void removeProtectedPage(Address pageAddr);
    void removeEmptyPages();
    void remove(func_instance *func);
    void remove(instPoint *p);
    void splitBlock(block_instance *first, block_instance *second);
    bool findBlocksByRange(Address startAddr,
                          Address endAddr,
                          std::list<block_instance*> &pageBlocks);
    void findFuncsByRange(Address startAddr,
                          Address endAddr,
                          std::set<func_instance*> &pageFuncs);
    void addEmulInsn(Address insnAddr, Register effective_addr);
    bool isEmulInsn(Address insnAddr);
    Register getEmulInsnReg(Address insnAddr);
    void setEmulInsnVal(Address insnAddr, void * val);
    int codeByteUpdates() { return codeByteUpdates_; }

    void replacePLTStub(SymtabAPI::Symbol *PLTsym, 
                        func_instance *func, 
                        Address newAddr);
private:
    void updateCodeBytes(SymtabAPI::Region *reg);
    bool isUpdateNeeded(Address entryAddr);
    bool isExpansionNeeded(Address entryAddr);
    void expandCodeBytes(SymtabAPI::Region *reg);
public:

    bool  getSymbolInfo(const std::string &n, int_symbol &sym);

    const std::vector<func_instance *> *findFuncVectorByPretty(const std::string &funcname);
    const std::vector<func_instance *> *findFuncVectorByMangled(const std::string &funcname);

    bool findFuncsByAddr(std::vector<func_instance *> &funcs);
    bool findBlocksByAddr(std::vector<block_instance *> &blocks);

    const std::vector<int_variable *> *findVarVectorByPretty(const std::string &varname);
    const std::vector<int_variable *> *findVarVectorByMangled(const std::string &varname);
    const int_variable *getVariable(const std::string &varname);

	void setDirty(){ dirty_=true;}
	bool isDirty() { return dirty_; }

    func_instance *findFunction(ParseAPI::Function *img_func);

    int_variable *findVariable(image_variable *img_var);

    block_instance *findBlock(ParseAPI::Block *);
    edge_instance *findEdge(ParseAPI::Edge *, block_instance *src = NULL, block_instance *trg = NULL);

    func_instance *findGlobalConstructorFunc(const std::string &ctorHandler);
    func_instance *findGlobalDestructorFunc(const std::string &dtorHandler);

    std::string getCalleeName(block_instance *);
    void setCalleeName(block_instance *, std::string name);

    void setCallee(const block_instance *, func_instance *);
    func_instance *getCallee(const block_instance *) const;

    void destroy(PatchAPI::PatchFunction *f);
    void destroy(PatchAPI::PatchBlock *b);
    // void destroy(PatchAPI::PatchEdge *e);

  private:
    fileDescriptor desc_;
    string  fullName_;
    string  fileName_;
    // Address   codeBase_;
    Address   dataBase_;
    Address   tocBase;

    void set_short_name();

    std::vector<mapped_module *> everyModule;
    typedef std::unordered_map<std::string, std::vector<func_instance *> *> func_index_t;
    typedef std::unordered_map<std::string, std::vector<int_variable *> *> var_index_t;
    
    std::unordered_map<const image_variable *, int_variable *> everyUniqueVariable;
    func_index_t allFunctionsByMangledName;
    func_index_t allFunctionsByPrettyName;
    var_index_t allVarsByMangledName;
    var_index_t allVarsByPrettyName;

    codeRangeTree codeRangesByAddr_;

    void addFunction(func_instance *func);
    void addVariable(int_variable *var);

    typedef enum {
        mangledName = 1,
        prettyName = 2,
        typedName = 4 } nameType_t;
    void addFunctionName(func_instance *func, const std::string newName, 
                         func_index_t &index);

    bool dirty_;
    bool dirtyCalled_;

    image  *image_;
    bool dlopenUsed;
    AddressSpace *proc_;

    bool analyzed_;

    typedef enum  {
        PROTECTED,
        REPROTECTED,
        UNPROTECTED,
    } WriteableStatus;
    BPatch_hybridMode analysisMode_;
    map<Address,WriteableStatus> protPages_;
    std::set<SymtabAPI::Region*> expansionCheckedRegions_;
    bool pagesUpdated_;
    int codeByteUpdates_;
    typedef std::map<Address, std::pair<Register,void*> > EmulInsnMap;
    EmulInsnMap emulInsns_;

    Address memEnd_;

    mapped_module *getOrCreateForkedModule(mapped_module *mod);

    bool memoryImg_;

    std::map<block_instance *, std::string> calleeNames_;
    std::map<const block_instance *, func_instance *> callees_;

};

class mappedObjData : public codeRange {
 public:
    mappedObjData(mapped_object *obj_) : obj(obj_) {}
    Address get_address() const { return obj->dataAbs(); }
    unsigned get_size() const { return obj->dataSize(); }
    mapped_object *obj;
};


#endif
