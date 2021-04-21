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
#include <list>

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
   class MemoryEmulator;

   namespace InstructionAPI {
      class Instruction;
   }
};

// This file serves to define an "address space", a set of routines that 
// code generation and instrumentation rely on to perform their duties. 
// This was derived from the process class and serves as a parent to that
// class and the static_space class for the rewriter. 

// The methods in this class were determined by what the code currently
// uses, not a particular design. As such, I expect this to change wildly
// by the time the refactoring is complete. 
//
// bernat, 5SEP07

// Note: this is a pure virtual class; it serves as an interface
// specification.

class AddressSpace : public InstructionSource {
 public:

   // This is a little complex, so let me explain my logic
   // Map from B -> F_c -> F
   // B identifies a call site
   // F_c identifies an (optional) function context for the replacement
   //   ... if F_c is not specified, we use NULL
   // F specifies the replacement callee; if we want to remove the call entirely,
   // also use NULL
   //typedef std::map<block_instance *, std::map<func_instance *, func_instance *> > CallModMap;
   //typedef std::map<func_instance *, func_instance *> FuncModMap;
    
    // Down-conversion functions
    PCProcess *proc();
    BinaryEdit *edit();

    // Read/write

    // We have read/write for both "text" and "data". This comes in handy,
    // somewhere, I'm sure
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

    // Memory allocation
    // We don't specify how it should be done, only that it is. The model is
    // that you ask for an allocation "near" a point, where "near" has an
    // internal, platform-specific definition. The allocation mechanism does its
    // best to give you what you want, but there are no promises - check the
    // address of the returned buffer to be sure.

    virtual Address inferiorMalloc(unsigned size, inferiorHeapType type=anyHeap,
                                   Address near = 0, bool *err = NULL) = 0;
    virtual void inferiorFree(Address item) = 0;
    void inferiorFreeInternal(Address item);
    // And a "constrain" call to free unused memory. This is useful because our
    // instrumentation is incredibly wasteful.
    virtual bool inferiorRealloc(Address item, unsigned newSize) = 0;
    bool inferiorReallocInternal(Address item, unsigned newSize);
    bool inferiorShrinkBlock(heapItem *h, Address block, unsigned newSize);
    bool inferiorExpandBlock(heapItem *h, Address block, unsigned newSize);

    bool isInferiorAllocated(Address block);

    // Allow the AddressSpace to update any extra bookkeeping for trap-based
    // instrumentation
    virtual void addTrap(Address from, Address to, codeGen &gen) = 0;
    virtual void removeTrap(Address from) = 0;

    virtual bool getDyninstRTLibName();

    // InstructionSource 
    virtual bool isValidAddress(const Address) const;
    virtual void *getPtrToInstruction(const Address) const;
    virtual void *getPtrToData(const Address a) const { return getPtrToInstruction(a); }

    bool usesDataLoadAddress() const; // OS-specific
    virtual bool isCode(const Address) const;
    virtual bool isData(const Address) const;
    virtual bool isReadOnly(const Address) const;
    virtual Address offset() const = 0;
    virtual Address length() const = 0;
    virtual Architecture getArch() const = 0;

    // Trap address to base tramp address (for trap instrumentation)
    trampTrapMappings trapMapping;
    
    //////////////////////////////////////////////////////////////
    // Function/variable lookup code
    // Turns out that instrumentation needs this... so the 
    // AddressSpace keeps growing. 
    //////////////////////////////////////////////////////////////

    // findFuncByName: returns function associated with "func_name"
    // This routine checks both the a.out image and any shared object images 
    // for this function
    //func_instance *findFuncByName(const std::string &func_name);
    
    bool findFuncsByAll(const std::string &funcname,
                        std::vector<func_instance *> &res,
                        const std::string &libname = "");
    
    // Specific versions...
    bool findFuncsByPretty(const std::string &funcname,
                           std::vector<func_instance *> &res,
                           const std::string &libname = "");
    bool findFuncsByMangled(const std::string &funcname, 
                            std::vector<func_instance *> &res,
                            const std::string &libname = "");
    
    bool findVarsByAll(const std::string &varname,
                       std::vector<int_variable *> &res,
                       const std::string &libname = "");
    
    // And we often internally want to wrap the above to return one
    // and only one func...
    virtual func_instance *findOnlyOneFunction(const std::string &name,
                                              const std::string &libname = "",
                                              bool search_rt_lib = true);


    // This will find the named symbol in the image or in a shared object
    // Necessary since some things don't show up as a function or variable.
    //    bool getSymbolInfo( const std::string &name, Dyn_Symbol &ret );
    // This gets wrapped with an int_symbol and returned.
    bool getSymbolInfo( const std::string &name, int_symbol &ret );

    // getAllFunctions: returns a vector of all functions defined in the
    // a.out and in the shared objects
    void getAllFunctions(std::vector<func_instance *> &);
    
    // Find the code sequence containing an address
    bool findFuncsByAddr(Address addr, std::set<func_instance *> &funcs, bool includeReloc = false);
    bool findBlocksByAddr(Address addr, std::set<block_instance *> &blocks, bool includeReloc = false);
    // Don't use this...
    // I take it back. Use it when you _know_ that you want one function,
    // picked arbitrarily, from the possible functions.
    func_instance *findOneFuncByAddr(Address addr);
    // And the one thing that is unique: entry address!
    func_instance *findFuncByEntry(Address addr);
    block_instance *findBlockByEntry(Address addr);

    // And a lookup by "internal" function to find clones during fork...
    func_instance *findFunction(parse_func *ifunc);
    block_instance *findBlock(parse_block *iblock);
    edge_instance *findEdge(ParseAPI::Edge *iedge);

	// Fast lookups across all mapped_objects
	func_instance *findFuncByEntry(const block_instance *block);

    //findJumpTargetFuncByAddr Acts like findFunc, but if it fails,
    // checks if 'addr' is a jump to a function.
    func_instance *findJumpTargetFuncByAddr(Address addr);
    
    // true if the addrs are in the same object and region within the object
    bool sameRegion(Dyninst::Address addr1, Dyninst::Address addr2);

    // findModule: returns the module associated with "mod_name" 
    // this routine checks both the a.out image and any shared object 
    // images for this module
    // if check_excluded is true it checks to see if the module is excluded
    // and if it is it returns 0.  If check_excluded is false it doesn't check
    //  if substring_match is true, the first module whose name contains
    //  the provided string is returned.
    // Wildcard: handles "*" and "?"
    mapped_module *findModule(const std::string &mod_name, bool wildcard = false);
    // And the same for objects
    // Wildcard: handles "*" and "?"
    mapped_object *findObject(std::string obj_name, bool wildcard = false) const;
    mapped_object *findObject(Address addr) const;
    mapped_object *findObject(fileDescriptor desc) const;
    mapped_object *findObject(const ParseAPI::CodeObject *co) const;

    mapped_object *getAOut() { assert(mapped_objects.size()); return mapped_objects[0];}
    
    // getAllModules: returns a vector of all modules defined in the
    // a.out and in the shared objects
    void getAllModules(std::vector<mapped_module *> &);

    // return the list of dynamically linked libs
    const std::vector<mapped_object *> &mappedObjects() { return mapped_objects;  } 

    // And a shortcut pointer
    std::set<mapped_object *> runtime_lib;
    // ... and keep the name around
    std::string dyninstRT_name;
    
    // If true is passed for ignore_if_mt_not_set, then an error won't be
    // initiated if we're unable to determine if the program is multi-threaded.
    // We are unable to determine this if the daemon hasn't yet figured out
    // what libraries are linked against the application.  Currently, we
    // identify an application as being multi-threaded if it is linked against
    // a thread library (eg. libpthreads.so on Linux).  There are cases where we
    // are querying whether the app is multi-threaded, but it can't be
    // determined yet but it also isn't necessary to know.
    virtual bool multithread_capable(bool ignore_if_mt_not_set = false) = 0;
    
    // Do we have the RT-side multithread functions available
    virtual bool multithread_ready(bool ignore_if_mt_not_set = false) = 0;

    //////////////////////////////////////////////////////
    // Process-level instrumentation (?)
    /////////////////////////////////////////////////////

    // instPoint isn't const; it may get an updated list of
    // instances since we generate them lazily.
    // Shouldn't this be an instPoint member function?

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

    // And this....
    typedef boost::shared_ptr<Dyninst::InstructionAPI::Instruction> InstructionPtr;
    bool getDynamicCallSiteArgs(InstructionAPI::Instruction insn,
                                Address addr,
                                std::vector<AstNodePtr> &args);

    // Default to "nope"
    virtual bool hasBeenBound(const SymtabAPI::relocationEntry &, 
                              func_instance *&, 
                              Address) { return false; }
    virtual bool bindPLTEntry(const SymtabAPI::relocationEntry & /*entry*/,
                              Address /*base_addr*/, 
                              func_instance * /*target_func*/,
                              Address /*target_addr*/) { return false; }
    
    // Trampoline guard get/set functions
    int_variable* trampGuardBase(void) { return trampGuardBase_; }
    AstNodePtr trampGuardAST(void);

    // Get the current code generator (or emitter)
    Emitter *getEmitter();

    //True if any reference to this address space needs PIC
    virtual bool needsPIC() = 0;
    //True if we need PIC to reference the given variable or function
    // from this addressSpace.
    bool needsPIC(int_variable *v); 
    bool needsPIC(func_instance *f);
    bool needsPIC(AddressSpace *s);
    
    unsigned getAddressWidth() const;
    
    //////////////////////////////////////////////////////
    // BPatch-level stuff
    //////////////////////////////////////////////////////
    // Callbacks for higher level code (like BPatch) to learn about new 
    //  functions and InstPoints.
 private:
    BPatch_function *(*new_func_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f);
    BPatch_point *(*new_instp_cb)(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f, 
                                  Dyninst::PatchAPI::Point *ip, 
                                  int type);
 public:
    //Trigger the callbacks from a lower level
    BPatch_function *newFunctionCB(Dyninst::PatchAPI::PatchFunction *f) 
        { assert(new_func_cb); return new_func_cb(this, f); }
    BPatch_point *newInstPointCB(Dyninst::PatchAPI::PatchFunction *f, 
                                 Dyninst::PatchAPI::Point *pt, int type)
        { assert(new_instp_cb); return new_instp_cb(this, f, pt, type); }
    
    //Register callbacks from the higher level
    void registerFunctionCallback(BPatch_function *(*f)(AddressSpace *p, 
                                                        Dyninst::PatchAPI::PatchFunction *f))
        { new_func_cb = f; };
    void registerInstPointCallback(BPatch_point *(*f)(AddressSpace *p, 
                                                      Dyninst::PatchAPI::PatchFunction *f,
                                                      Dyninst::PatchAPI::Point *ip, int type))
        { new_instp_cb = f; }
    
    
    //Anonymous up pointer to the containing process.  This is BPatch_process
    // in Dyninst.  Currently stored as an void pointer in case we do
    // anything with this during the library split.
    void *up_ptr() { return up_ptr_; }
    void set_up_ptr(void *ptr) { up_ptr_ = ptr; }
    
    //////////////////////////////////////////////////////
    // Internal and cleanup 
    //////////////////////////////////////////////////////

    // Clear things out (e.g., deleteProcess)
    void deleteAddressSpace();
    // Fork psuedo-constructor
    void copyAddressSpace(AddressSpace *parent);

    // Aaand constructor/destructor
    AddressSpace();
    virtual ~AddressSpace();


    //////////////////////////////////////////////////////
    // Yuck
    //////////////////////////////////////////////////////
    Address getObservedCostAddr() const { return costAddr_; }
    void updateObservedCostAddr(Address addr) { costAddr_ = addr;}

    // Can we use traps if necessary?
    bool canUseTraps();
    void setUseTraps(bool usetraps);

    //////////////////////////////////////////////////////
    // The New Hotness
    //////////////////////////////////////////////////////
    //
    // This is the top interface for the new (experimental)
    // (probably not working) code generation interface. 
    // The core idea is to feed a set of func_instances 
    // (actually, a set of blocks, but functions are convenient)
    // into a CodeMover class, let it chew on the code, and 
    // spit out a buffer of moved code. 
    // We also get a priority list of patches; (origAddr,
    // movedAddr) pairs. We then get to decide what we want
    // to do with those patches: put in a branch or say to 
    // heck with it.
    
    bool relocate();
		   

    // Get the list of addresses an address (in a block) 
    // has been relocated to.
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
    // defensive mode code // 

    void causeTemplateInstantiations();

    // Debugging method
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

    void updateMemEmulator();
    bool isMemoryEmulated() { return emulateMem_; }
    bool emulatingPC() { return emulatePC_; }
    MemoryEmulator *getMemEm();

    bool delayRelocation() const;
 protected:

    // inferior malloc support functions
    void inferiorFreeCompact();
    int findFreeIndex(unsigned size, int type, Address lo, Address hi);
    void addHeap(heapItem *h);
    void initializeHeap();
    
    // Centralization of certain inferiorMalloc operations
    Address inferiorMallocInternal(unsigned size, Address lo, Address hi, 
                                   inferiorHeapType type);
    void inferiorMallocAlign(unsigned &size);

    bool heapInitialized_;
    bool useTraps_;
    bool sigILLTrampoline_;
    inferiorHeap heap_;

    // Loaded mapped objects (may be just 1)
    std::vector<mapped_object *> mapped_objects;

    int_variable* trampGuardBase_; // Tramp recursion index mapping
    AstNodePtr trampGuardAST_;

    void *up_ptr_;

    Address costAddr_;

    /////// New instrumentation system
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
    // defensive mode code
    typedef std::pair<Address, unsigned> DefensivePad;
    std::map<Address, std::map<func_instance*,std::set<DefensivePad> > > forwardDefensiveMap_;
    IntervalTree<Address, std::pair<func_instance*,Address> > reverseDefensiveMap_;

    // Tracking instrumentation for fast removal
    std::map<baseTramp *, std::set<Address> > instrumentationInstances_;

    // Track desired function replacements/removals/call replacements
    // CallModMap callModifications_;
    // FuncModMap functionReplacements_;
    // FuncModMap functionWraps_;

    void addAllocatedRegion(Address start, unsigned size);
    void addModifiedRegion(mapped_object *obj);

    MemoryEmulator *memEmulator_;

    bool emulateMem_;
    bool emulatePC_;

    bool delayRelocation_;

    std::map<func_instance *, Dyninst::SymtabAPI::Symbol *> wrappedFunctionWorklist_;

  // PatchAPI stuffs
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
