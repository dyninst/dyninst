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
 
// $Id: function.h,v 1.52 2008/09/08 16:44:02 bernat Exp $

#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>
#include "common/h/Vector.h"
#include "common/h/Types.h"
#include "common/h/Pair.h"
#include "codegen.h"
#include "codeRange.h"
#include "util.h"
#include "image-func.h"

#include "bitArray.h"

#include "dyn_detail/boost/shared_ptr.hpp"

class process;
class mapped_module;
class mapped_object;

class BPatch_flowGraph;
class BPatch_loopTreeNode;
class BPatch_basicBlock;
class BPatch_basicBlockLoop;

class instPointInstance;

#include "dyninstAPI/src/ast.h"

class instPoint;

class Frame;

class int_function;
class int_basicBlock;
class funcMod;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;

// A specific instance (relocated version) of a basic block
// It's really a semi-smart struct...
class bblInstance : public codeRange {
    friend class int_basicBlock;
    friend class int_function;

    // "We'll set things up later" constructor. Private because only int_basicBlock
    // should be playing in here
    bblInstance(int_basicBlock *parent, int version);
 public:
    bblInstance(Address start, Address last, Address end, int_basicBlock *parent, int version);
    bblInstance(const bblInstance *parent, int_basicBlock *block);
    ~bblInstance();

    Address firstInsnAddr() const { return firstInsnAddr_; }
    Address lastInsnAddr() const { return lastInsnAddr_; }
    // Just to be obvious -- this is the end addr of the block
    Address endAddr() const { return blockEndAddr_; }
    Address getSize() const { return blockEndAddr_ - firstInsnAddr_; }

    void setLastInsnAddr( Address newLast ) { lastInsnAddr_ = newLast; }
    void setEndAddr( Address newEnd ) { blockEndAddr_ = newEnd; }

    // And equivalence in addresses...
    Address equivAddr(int newVersion, Address addr) const;

    // As a note: do _NOT_ create an address-based comparison of this
    // class unless you just need some sort of ordering. We may create these
    // blocks in some random place; depending on address is just plain dumb.

    Address get_address() const { return firstInsnAddr(); }
    unsigned get_size() const { return getSize(); }
    void *getPtrToInstruction(Address addr) const;
    void *get_local_ptr() const;

    // Singular CF target...
    bblInstance *getTargetBBL();
    bblInstance *getFallthroughBBL();


    const void *getPtrToOrigInstruction(Address addr) const;
    unsigned getRelocInsnSize(Address addr) const;

    void getOrigInstructionInfo(Address addr, const void *&ptr, Address &origAddr, unsigned &origSize) const;

    //process *proc() const;
    AddressSpace *proc() const;
    int_function *func() const;
    int_basicBlock *block() const;
    int version() const;


#if defined(cap_instruction_api)
    void getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> >&instances) const;
    void disassemble() const;
#endif

    Address firstInsnAddr_;
    Address lastInsnAddr_;
    Address blockEndAddr_;
    int_basicBlock *block_;
    int version_;
};

class int_basicBlock {
    friend class int_function;
    friend class bblInstance;
 public:
    int_basicBlock(image_basicBlock *ib, Address baseAddr, int_function *func, int id);
    int_basicBlock(const int_basicBlock *parent, int_function *func, int id);
    ~int_basicBlock();

        // just because a block is an entry block doesn't mean it is
        // an entry block that this block's function cares about
    bool isEntryBlock() const;
    bool isExitBlock() const { return ib_->isExitBlock(); }

        // int_basicBlocks are not shared, but their underlying blocks
        // may be
    bool hasSharedBase() const { return ib_->isShared(); }

    image_basicBlock * llb() const { return ib_; }
    
    struct compare {
        bool operator()(int_basicBlock * const &b1,
                        int_basicBlock * const &b2) const {
            if(b1->instances_[0]->firstInsnAddr() < b2->instances_[0]->firstInsnAddr())
                return true;
            if(b2->instances_[0]->firstInsnAddr() < b1->instances_[0]->firstInsnAddr())
                return false;

            if(b1 != b2) 
                fprintf(stderr,"error: two blocks (0x%p,0x%p) at 0x%lx \n",
                        b1,b2,b1->instances_[0]->firstInsnAddr());

            assert(b1 == b2);
            return false;
        }
    };

    const pdvector<bblInstance *> &instances() const;
    bblInstance *origInstance() const;
    bblInstance *instVer(unsigned index) const;
    void removeVersion(unsigned index,bool deleteInstance=true);
    unsigned numInstances() { return instances_.size(); }

    void debugPrint();

    void getSources(pdvector<int_basicBlock *> &ins) const;
    void getTargets(pdvector<int_basicBlock *> &outs) const;
    EdgeTypeEnum getSourceEdgeType(int_basicBlock *source) const;
    EdgeTypeEnum getTargetEdgeType(int_basicBlock *target) const;

    bool containsCall();
    int_basicBlock *getFallthrough() const;

    int id() const { return id_; }

    // True if we need to put a jump in here. If we're an entry block
    // or the target of an indirect jump (for now).
    bool needsJumpToNewVersion();

    // A block will need relocation (when instrumenting) if
    // its underlying image_basicBlock is shared (this is independant
    // of relocation requirements dependant on size available)
    bool needsRelocation() const;

    int_function *func() const { return func_; }
    //process *proc() const;
    AddressSpace *proc() const;

    void setHighLevelBlock(void *newb);
    void *getHighLevelBlock() const;


 private:
    void *highlevel_block; //Should point to a BPatch_basicBlock, if they've
                           //been created.
    int_function *func_;
    image_basicBlock *ib_;

    int id_;

    // A single "logical" basic block may correspond to multiple
    // physical areas of code. In particular, we may relocate the
    // block (either replaced or duplicated).
    pdvector<bblInstance *> instances_;
};

struct edgeStub {
    edgeStub(bblInstance *s, Address t, EdgeTypeEnum y) 
    { src = s; trg = t; type = y; }
    bblInstance* src;
    Address trg;
    EdgeTypeEnum type;
};

class int_function : public patchTarget {
  friend class bblInstance;
  friend class int_basicBlock;
 public:
   //static std::string emptyString;

   // Almost everything gets filled in later.
   int_function(image_func *f,
		Address baseAddr,
                mapped_module *mod);

   int_function(const int_function *parent,
                mapped_module *child_mod,
                process *childP);

   ~int_function();

   ////////////////////////////////////////////////
   // Passthrough functions.
   ////////////////////////////////////////////////
   // To minimize wasted memory (since there will be many copies of
   // this function) we make most methods passthroughs to the original
   // parsed version.

   const string &symTabName() const;
   const string &prettyName() const { return ifunc_->prettyName(); };
   const string &typedName() const { return ifunc_->typedName(); };
   
   const vector<string>& symTabNameVector() const { return ifunc_->symTabNameVector(); }
   const vector<string>& prettyNameVector() const { return ifunc_->prettyNameVector(); }
   const vector<string>& typedNameVector() const { return ifunc_->typedNameVector(); }

   // Debuggering functions
   void debugPrint() const;

   // And add...
   // Don't make the std::string a reference; we want a copy.
   void addSymTabName(const std::string name, bool isPrimary = false);
   void addPrettyName(const std::string name, bool isPrimary = false);

   // May change when we relocate...
   Address getAddress() const {return addr_;}
   Address getPtrAddress() const {return ptrAddr_;}

   // Don't use this...
   unsigned getSize_NP();

   // Not defined here so we don't have to play header file magic
   // Not const; we can add names via the Dyninst layer
   image_func *ifunc();
   mapped_module *mod() const;
   mapped_object *obj() const;
   //process *proc() const;
   AddressSpace *proc() const;

   // Necessary for BPatch_set which needs a structure with a ()
   // operator. Odd.
   struct cmpAddr {
     int operator() (const int_function *f1, const int_function *f2) const {
       if (f1->getAddress() > f2->getAddress())
	 return 1;
       else if (f1->getAddress() < f2->getAddress())
	 return -1;
       else
	 return 0;
     }
   };

   // extra debuggering info....
   ostream & operator<<(ostream &s) const;
   friend ostream &operator<<(ostream &os, int_function &f);

   ////////////////////////////////////////////////
   // Process-dependent (inter-module) parsing
   ////////////////////////////////////////////////

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   const std::set< int_basicBlock* , int_basicBlock::compare > &blocks();



   // Perform a lookup (either linear or log(n)).
   int_basicBlock *findBlockByAddr(Address addr);
   int_basicBlock *findBlockByOffset(Address offset) { return findBlockByAddr(offsetToAddr(offset)); }
   int_basicBlock *findBlockByOffsetInFunc(Address offset) { return findBlockByAddr(offset + getAddress()); }
   bblInstance *findBlockInstanceByAddr(Address addr);
   int_basicBlock *findBlockByImage(image_basicBlock *block);
   bblInstance *findBlockInstanceByEntry(Address addr);

   int_basicBlock *entryBlock();

   void findBlocksByRange(std::vector<int_basicBlock*> &funcs, 
                          Address start, Address end);

   void addMissingBlock(image_basicBlock & imgBlock);
   void addMissingBlocks();
   void addMissingPoints();

   Offset addrToOffset(const Address addr) const;
   Address offsetToAddr(const Offset off) const; 


   bool hasNoStackFrame() const {return ifunc_->hasNoStackFrame();}
   bool savesFramePointer() const {return ifunc_->savesFramePointer();}

   //BPatch_flowGraph * getCFG();
   //BPatch_loopTreeNode * getLoopTree();

   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   void addArbitraryPoint(instPoint *insp);

   const pdvector<instPoint*> &funcEntries();
   // Note: the vector is constant, the instPoints aren't.
   const pdvector<instPoint*> &funcExits();
   const pdvector<instPoint*> &funcCalls();
   const pdvector<instPoint*> &funcArbitraryPoints();
   const std::set<instPoint*> &funcUnresolvedControlFlow();
   const std::set<instPoint*> &funcAbruptEnds();
   bool setPointResolved(instPoint* resolvedPt);

   bool parseNewEdges(const std::vector<edgeStub>& sources);

   bool isSignalHandler() {return handlerFaultAddr_ != 0;}
   Address getHandlerFaultAddr() {return handlerFaultAddr_;}
   Address getHandlerFaultAddrAddr() {return handlerFaultAddrAddr_;}
   void fixHandlerReturnAddr(Address newAddr);
   void setHandlerFaultAddr(Address fa);
   void setHandlerFaultAddrAddr(Address faa, bool set);

   instPoint *findInstPByAddr(Address addr);
   // get instPoints of function that are known to call into this one
   void getCallerPoints(std::vector<instPoint*>& callerPoints);

   // And for adding... we map by address to instPoint
   // as a single instPoint may have multiple instances
   void registerInstPointAddr(Address addr, instPoint *inst);
   void unregisterInstPointAddr(Address addr, instPoint *inst);

   bool isInstrumentable() const { return ifunc_->isInstrumentable(); }

   Address get_address() const;
   unsigned get_size() const;
   std::string get_name() const;
   
#if defined(arch_x86) || defined(arch_x86_64)
   //Replaces the function with a 'return val' statement.
   // currently needed only on Linux/x86
   // Defined in inst-x86.C
   bool setReturnValue(int val);

#endif

   ////////////////////////////////////////////////
   // Relocation
   ////////////////////////////////////////////////

   bool canBeRelocated() const { return ifunc_->canBeRelocated(); }
   int version() const { return version_; }


   ////////////////////////////////////////////////
   // Code overlapping
   ////////////////////////////////////////////////
   // Get all functions that "share" the block. Actually, the
   // int_basicBlock will not be shared (they are per function),
   // but the underlying image_basicBlock records the sharing status. 
   // So dodge through to the image layer and find out that info. 
   // Returns true if such functions exist.

   bool getSharingFuncs(int_basicBlock *b,
                        std::set<int_function *> &funcs);

   // The same, but for any function that overlaps with any of
   // our basic blocks.
   // OPTIMIZATION: we're not checking all blocks, only an exit
   // point; this _should_ work :) but needs to change if we
   // ever do flow-sensitive parsing
   bool getOverlappingFuncs(std::set<int_function *> &funcs);


   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////


   const pdvector< int_parRegion* > &parRegions();

   bool containsSharedBlocks() const { return ifunc_->containsSharedBlocks(); }
   unsigned getNumDynamicCalls();

    // Fill the <callers> vector with pointers to the statically-determined
    // list of functions that call this function.
    void getStaticCallers(pdvector <int_function *> &callers);

   codeRange *copy() const;

#if defined(sparc_sun_solaris2_4)
   bool is_o7_live(){ return ifunc_->is_o7_live(); }
#endif

#if defined(arch_power)
   bool savesReturnAddr() const { return ifunc_->savesReturnAddr(); }
#endif

   void updateForFork(process *childProcess, const process *parentProcess);

#if defined(os_windows) 
   //Calling convention for this function
   callType int_function::getCallingConvention();
   int getParamSize() { return paramSize; }
   void setParamSize(int s) { paramSize = s; }
#endif


    bool removePoint(instPoint *point);
    void deleteBlock(int_basicBlock *block);
    void removeFromAll();
    int_basicBlock *setNewEntryPoint();
    void getReachableBlocks(const std::set<bblInstance*> &exceptBlocks,
                            const std::list<bblInstance*> &seedBlocks,
                            std::set<bblInstance*> &reachBlocks);//output
   


 private:

   ///////////////////// Basic func info
   Address addr_; // Absolute address of the start of the function
   Address ptrAddr_; // Absolute address of the function descriptor, if exists

   image_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// image_funcs to int_funcs

   ///////////////////// CFG and function body
   std::set< int_basicBlock* , int_basicBlock::compare > blockList; 

   // Added to support translation between function-specific int_basicBlocks
   // and potentially shared image_basicBlocks
   dictionary_hash<int, int> blockIDmap;

   //BPatch_flowGraph *flowGraph;

   ///////////////////// Instpoints 

   pdvector<instPoint*> entryPoints_;	/* place to instrument entry (often not addr) */
   pdvector<instPoint*> exitPoints_;	/* return point(s). */
   pdvector<instPoint*> callPoints_;	/* pointer to the calls */
   pdvector<instPoint*> arbitraryPoints_;  /* arbitrary points */
   std::set<instPoint*> unresolvedPoints_; /* statically unresolved ctrl flow */
   std::set<instPoint*> abruptEnds_; /* block endpoints that end abruptly by 
                                      running to the end of valid memory or by 
                                      reaching an invalid instruction */

   Address handlerFaultAddr_; /* if this is a signal handler, faultAddr_ is 
                                 set to -1, or to the address of the fault 
                                 that last caused the handler to be invoked. */
   Address handlerFaultAddrAddr_; 

   bool isBeingInstrumented_;

   dictionary_hash<Address, instPoint *> instPsByAddr_;

   //////////////////////////  Parallel Regions 
   pdvector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */


   // Used to sync with instPoints
   int version_;

   codeRangeTree blocksByAddr_;
   std::map<Address, bblInstance *> blocksByEntry_;
   void addBBLInstance(bblInstance *instance);

#if defined(os_windows) 
   callType callingConv;
   int paramSize;
#endif

    // the number of blocks in a function can go up or down, I need 
    // to keep track of the next block ID to use
    int nextBlockID;

    // 
    // Local instrumentation-based auxiliary functions
    void getNewInstrumentation(std::set<instPoint *> &);
    void getAnyInstrumentation(std::set<instPoint *> &);

};



#endif /* FUNCTION_H */
