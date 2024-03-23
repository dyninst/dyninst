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

// $Id: addressSpace.h,v 1.9 2008/06/20 22:00:04 legendre Exp $

#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "infHeap.h"
#include "codeRange.h"
#include "ast.h"
#include "symtabAPI/h/Symtab.h"
#include "dyninstAPI/src/trapMappings.h"
#include <assert.h>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include "dyntypes.h"
#include "common/src/IntervalTree.h"

#include "parseAPI/h/CodeObject.h"
#include "parseAPI/h/InstructionSource.h"
#include "Relocation/Relocation.h"
#include "Relocation/CodeTracker.h"
#include "Relocation/Springboard.h"
#include "Patching.h"

#include "PatchMgr.h"
#include "Command.h"

class codeRange;
class replacedFunctionCall;

class func_instance;
class block_instance;
class edge_instance;

class parse_func;
class parse_block;

struct edgeStub;
class int_variable;
class mapped_module;
class mapped_object;
class instPoint;

class BPatch_process;
class BPatch_function;
class BPatch_point;

class Emitter;
class fileDescriptor;

using namespace Dyninst;
//using namespace SymtabAPI;

class func_instance;
class int_symbol;

class Dyn_Symbol;
class BinaryEdit;
class PCProcess;
class trampTrapMappings;
class baseTramp;

namespace Dyninst {

   namespace InstructionAPI {
      class Instruction;
   }
}

class AddressSpace : public InstructionSource {
 public:

   //typedef std::map<block_instance *, std::map<func_instance *, func_instance *> > CallModMap;
   //typedef std::map<func_instance *, func_instance *> FuncModMap;
    
    PCProcess *proc();
    BinaryEdit *edit();

    // Read/write

    virtual bool readDataWord(const void *inOther, 
                              u_int amount, 
                              void *inSelf, 
                              bool showError) = 0;
    virtual bool readDataSpace(const void *inOther, 
                               u_int amount, 
                               void *inSelf, 
                               bool showError) = 0;
    virtual bool readTextWord(const void *inOther, 
                              u_int amount, 
                              void *inSelf) = 0;
    virtual bool readTextSpace(const void *inOther, 
                               u_int amount, 
                               void *inSelf) = 0;
    

    virtual bool writeDataWord(void *inOther,
                               u_int amount,
                               const void *inSelf) = 0;
    virtual bool writeDataSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;
    virtual bool writeTextWord(void *inOther,
                               u_int amount,
                               const void *inSelf) = 0;
    virtual bool writeTextSpace(void *inOther,
                                u_int amount,
                                const void *inSelf) = 0;

    Address getTOCoffsetInfo(func_instance *);

    virtual Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                                   Address near = 0, bool *err = NULL) = 0;
    virtual void inferiorFree(Address item) = 0;
    void inferiorFreeInternal(Address item);
    virtual bool inferiorRealloc(Address item, unsigned newSize) = 0;
    bool inferiorReallocInternal(Address item, unsigned newSize);
    bool inferiorShrinkBlock(heapItem *h, Address block, unsigned newSize);
    bool inferiorExpandBlock(heapItem *h, Address block, unsigned newSize);

    bool isInferiorAllocated(Address block);

    virtual void addTrap(Address from, Address to, codeGen &gen) = 0;
    virtual void removeTrap(Address from) = 0;

    virtual bool getDyninstRTLibName();

    virtual bool isValidAddress(const Address) const;
    virtual void *getPtrToInstruction(const Address) const;
    virtual void *getPtrToData(const Address a) const { return getPtrToInstruction(a); }

    bool usesDataLoadAddress() const;
    virtual bool isCode(const Address) const;
    virtual bool isData(const Address) const;
    virtual bool isReadOnly(const Address) const;
    virtual Address offset() const = 0;
    virtual Address length() const = 0;
    virtual Architecture getArch() const = 0;

    trampTrapMappings trapMapping;
    
    //func_instance *findFuncByName(const std::string &func_name);
    
    bool findFuncsByAll(const std::string &funcname,
                        std::vector<func_instance *> &res,
                        const std::string &libname = "");
    
    bool findFuncsByPretty(const std::string &funcname,
                           std::vector<func_instance *> &res,
                           const std::string &libname = "");
    bool findFuncsByMangled(const std::string &funcname, 
                            std::vector<func_instance *> &res,
                            const std::string &libname = "");
    
    bool findVarsByAll(const std::string &varname,
                       std::vector<int_variable *> &res,
                       const std::string &libname = "");
    
    virtual func_instance *findOnlyOneFunction(const std::string &name,
                                              const std::string &libname = "",
                                              bool search_rt_lib = true);


    bool getSymbolInfo( const std::string &name, int_symbol &ret );

    void getAllFunctions(std::vector<func_instance *> &);
    
    bool findFuncsByAddr(Address addr, std::set<func_instance *> &funcs, bool includeReloc = false);
    bool findBlocksByAddr(Address addr, std::set<block_instance *> &blocks, bool includeReloc = false);

    func_instance *findOneFuncByAddr(Address addr);
    func_instance *findFuncByEntry(Address addr);
    block_instance *findBlockByEntry(Address addr);
    func_instance *findFunction(parse_func *ifunc);
    block_instance *findBlock(parse_block *iblock);
    edge_instance *findEdge(ParseAPI::Edge *iedge);

	func_instance *findFuncByEntry(const block_instance *block);

    func_instance *findJumpTargetFuncByAddr(Address addr);
    
    bool sameRegion(Dyninst::Address addr1, Dyninst::Address addr2);

    mapped_module *findModule(const std::string &mod_name, bool wildcard = false);
    mapped_object *findObject(std::string obj_name, bool wildcard = false) const;
    mapped_object *findObject(Address addr) const;
    mapped_object *findObject(fileDescriptor desc) const;
    mapped_object *findObject(const ParseAPI::CodeObject *co) const;

    mapped_object *getAOut() { assert(mapped_objects.size()); return mapped_objects[0];}
    
    void getAllModules(std::vector<mapped_module *> &);

    const std::vector<mapped_object *> &mappedObjects() { return mapped_objects;  } 

    std::set<mapped_object *> runtime_lib;
    std::string dyninstRT_name;
    
    virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0;
    
    virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0;


    void modifyCall(block_instance *callBlock, func_instance *newCallee, func_instance *context = NULL);
    void revertCall(block_instance *callBlock, func_instance *context = NULL);
    void replaceFunction(func_instance *oldfunc, func_instance *newfunc);
    bool wrapFunction(func_instance *original, 
                      func_instance *wrapper, 
                      SymtabAPI::Symbol *clone);
    void wrapFunctionPostPatch(func_instance *wrapped, Dyninst::SymtabAPI::Symbol *);

    void revertWrapFunction(func_instance *original);                      
    void revertReplacedFunction(func_instance *oldfunc);
    void removeCall(block_instance *callBlock, func_instance *context = NULL);
    const func_instance *isFunctionReplacement(func_instance *func) const;

    typedef boost::shared_ptr<Dyninst::InstructionAPI::Instruction> InstructionPtr;
    bool getDynamicCallSiteArgs(InstructionAPI::Instruction insn,
                                Address addr,
                                std::vector<AstNodePtr> &args);

    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, 
                              func_instance *&, 
                              Address) { return false; }
    virtual bool bindPLTEntry(const SymtabAPI::relocationEntry & /*entry*/,
                              Address /*base_addr*/, 
                              func_instance * /*target_func*/,
                              Address /*target_addr*/) { return false; }
    
    int_variable* trampGuardBase(void) { return trampGuardBase_; }
    AstNodePtr trampGuardAST(void);

    Emitter *getEmitter();

    virtual bool needsPIC() = 0;
    bool needsPIC(int_variable *v); 
    bool needsPIC(func_instance *f);
    bool needsPIC(AddressSpace *s);
    
    unsigned getAddressWidth() const;
    

 private:
    BPatch_function *(*new_func_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f);
    BPatch_point *(*new_instp_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f, 
                                  Dyninst::PatchAPI::Point *ip, 
                                  int type);
 public:
    BPatch_function *newFunctionCB(Dyninst::PatchAPI::PatchFunction *f) 
        { assert(new_func_cb); return new_func_cb(this, f); }
    BPatch_point *newInstPointCB(Dyninst::PatchAPI::PatchFunction *f, 
                                 Dyninst::PatchAPI::Point *pt, int type)
        { assert(new_instp_cb); return new_instp_cb(this, f, pt, type); }
    
    void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, 
                                                        Dyninst::PatchAPI::PatchFunction *f))
        { new_func_cb = f; }
    void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, 
                                                      Dyninst::PatchAPI::PatchFunction *f,
                                                      Dyninst::PatchAPI::Point *ip, int type))
        { new_instp_cb = f; }
    
    
    void *up_ptr() { return up_ptr_; }
    void set_up_ptr(void *ptr) { up_ptr_ = ptr; }
    

    void deleteAddressSpace();
    void copyAddressSpace(AddressSpace *parent);

    AddressSpace();
    virtual ~AddressSpace();


    Address getObservedCostAddr() const { return costAddr_; }
    void updateObservedCostAddr(Address addr) { costAddr_ = addr;}

    bool canUseTraps();
    void setUseTraps(bool usetraps);

    bool relocate();
		   

    void getRelocAddrs(Address orig,
                       block_instance *block,
                       func_instance *func,
                       std::list<Address> &relocs,
                       bool getInstrumentationAddrs) const;


    bool getAddrInfo(Address relocAddr,//input
                     Address &origAddr,
                     std::vector<func_instance *> &origFuncs,
                     baseTramp *&baseTramp);
    typedef Relocation::CodeTracker::RelocInfo RelocInfo;
    bool getRelocInfo(Address relocAddr,
                      RelocInfo &relocInfo);


    bool inEmulatedCode(Address addr);

    std::map<func_instance*,std::vector<edgeStub> > 
    getStubs(const std::list<block_instance *> &owBBIs,
             const std::set<block_instance*> &delBBIs,
             const std::list<func_instance*> &deadFuncs);

    void addDefensivePad(block_instance *callBlock, func_instance *callFunc,
                         Address padStart, unsigned size);

    void getPreviousInstrumentationInstances(baseTramp *bt,
					     std::set<Address>::iterator &b,
					     std::set<Address>::iterator &e);
    void addInstrumentationInstance(baseTramp *bt, Address addr);

    void addModifiedFunction(func_instance *func);
    void addModifiedBlock(block_instance *block);

    bool delayRelocation() const;
 protected:

    void inferiorFreeCompact();
    int findFreeIndex(unsigned size, int type, Address lo, Address hi);
    void addHeap(heapItem *h);
    void initializeHeap();
    
    Address inferiorMallocInternal(unsigned size, Address lo, Address hi, 
                                   inferiorHeapType type);
    void inferiorMallocAlign(unsigned &size);

    bool heapInitialized_;
    bool useTraps_;
    bool sigILLTrampoline_;
    inferiorHeap heap_;

    std::vector<mapped_object *> mapped_objects;

    int_variable* trampGuardBase_;
    AstNodePtr trampGuardAST_;

    void *up_ptr_;

    Address costAddr_;

    typedef std::list<Relocation::CodeTracker *> CodeTrackers;
    CodeTrackers relocatedCode_;

    bool transform(Dyninst::Relocation::CodeMoverPtr cm);
    Address generateCode(Dyninst::Relocation::CodeMoverPtr cm, Address near);
    bool patchCode(Dyninst::Relocation::CodeMoverPtr cm,
		   Dyninst::Relocation::SpringboardBuilderPtr spb);

    typedef std::set<func_instance *> FuncSet;
    std::map<mapped_object *, FuncSet> modifiedFunctions_;

    bool relocateInt(FuncSet::const_iterator begin, FuncSet::const_iterator end, Address near);
    Dyninst::Relocation::InstalledSpringboards::Ptr installedSpringboards_;
 public:
    Dyninst::Relocation::InstalledSpringboards::Ptr getInstalledSpringboards() 
    {
      return installedSpringboards_;
    }
 protected:

    typedef std::pair<Address, unsigned> DefensivePad;
    std::map<Address, std::map<func_instance*,std::set<DefensivePad> > > forwardDefensiveMap_;
    IntervalTree<Address, std::pair<func_instance*,Address> > reverseDefensiveMap_;

    std::map<baseTramp *, std::set<Address> > instrumentationInstances_;

    // Track desired function replacements/removals/call replacements
    // CallModMap callModifications_;
    // FuncModMap functionReplacements_;
    // FuncModMap functionWraps_;

    bool delayRelocation_;

    std::map<func_instance *, Dyninst::SymtabAPI::Symbol *> wrappedFunctionWorklist_;

  public:
    Dyninst::PatchAPI::PatchMgrPtr mgr() const { assert(mgr_); return mgr_; }
    void setMgr(Dyninst::PatchAPI::PatchMgrPtr m) { mgr_ = m; }
    void setPatcher(Dyninst::PatchAPI::Patcher::Ptr p) { patcher_ = p; }
    void initPatchAPI();
    void addMappedObject(mapped_object* obj);
    Dyninst::PatchAPI::Patcher::Ptr patcher() { return patcher_; }
    static bool patch(AddressSpace*);
  protected:
    Dyninst::PatchAPI::PatchMgrPtr mgr_;
    Dyninst::PatchAPI::Patcher::Ptr patcher_;
};


bool uninstrument(Dyninst::PatchAPI::Instance::Ptr);
extern int heapItemCmpByAddr(const heapItem **A, const heapItem **B);

#endif // ADDRESS_SPACE_H
