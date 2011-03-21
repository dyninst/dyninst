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

#include "block.h"

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
class int_block;
class int_edge;
class funcMod;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;


class int_function : public patchTarget {
  friend class int_block;
  friend class int_edge;
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

   const string &symTabName() const { return ifunc_->symTabName(); };
   const string &prettyName() const { return ifunc_->prettyName(); };
   const string &typedName() const { return ifunc_->typedName(); };
   const string &name() const { return symTabName(); }
   
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
   Address addr() const { return addr_; }

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

   typedef std::set<int_block *, int_block::compare> BlockSet;
   const BlockSet &blocks();

   int_block *entryBlock();
   const BlockSet &callBlocks();
   const BlockSet &exitBlocks();

   // Kevin's defensive mode shtuff
   // Blocks that have a sink target, essentially. 
   const BlockSet &unresolvedCF();
   // Blocks where we provisionally stopped parsing because things looked weird.
   const BlockSet &abruptEnds();

   // Perform a lookup (either linear or log(n)).
   bool findBlocksByAddr(Address addr, std::set<int_block *> &blocks);
   bool findBlocksByOffsetInFunc(Address offset, std::set<int_block *> &blocks) {
       return findBlocksByAddr(offset + baseAddr(), blocks);
   }
   bool findBlocksByOffset(Address offset, std::set<int_block *> &blocks) {
       return findBlocksByAddr(offsetToAddr(offset), blocks);
   }

   int_block *findBlock(ParseAPI::Block *block);
   int_block *findBlockByEntry(Address addr);
   int_block *findOneBlockByAddr(Address Addr);


   void findBlocksByRange(std::vector<int_block*> &funcs, 
                          Address start, Address end);

   void addMissingBlock(image_basicBlock *imgBlock);
   void addMissingBlocks();

   Offset addrToOffset(const Address addr) const;
   Address offsetToAddr(const Offset off) const; 


   bool hasNoStackFrame() const {return ifunc_->hasNoStackFrame();}
   bool savesFramePointer() const {return ifunc_->savesFramePointer();}


   ////////////////////////////////////////////////
   // Legacy/inter-module calls. Arguably should be an 
   // interprocedural edge, but I expect that would
   // break all manner of things
   ////////////////////////////////////////////////
   int_function *findCallee(int_block *callBlock);


   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   instPoint *entryPoint();
   instPoint *exitPoint(int_block *exitBlock);

   instPoint *findPoint(instPoint::Type type);
   instPoint *findPoint(instPoint::Type type, int_block *block);
   const std::map<Address, instPoint *> &findPoints(instPoint::Type type, int_block *block);

   bool isSignalHandler() {return handlerFaultAddr_ != 0;}
   Address getHandlerFaultAddr() {return handlerFaultAddr_;}
   Address getHandlerFaultAddrAddr() {return handlerFaultAddrAddr_;}
   void fixHandlerReturnAddr(Address newAddr);
   void setHandlerFaultAddr(Address fa);
   void setHandlerFaultAddrAddr(Address faa, bool set);

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


   ////////////////////////////////////////////////
   // Code overlapping
   ////////////////////////////////////////////////
   // Get all functions that "share" the block. Actually, the
   // int_block will not be shared (they are per function),
   // but the underlying image_basicBlock records the sharing status. 
   // So dodge through to the image layer and find out that info. 
   // Returns true if such functions exist.

   bool getSharingFuncs(int_block *b,
                        std::set<int_function *> &funcs);

   // The same, but for any function that overlaps with any of
   // our basic blocks.
   // OPTIMIZATION: we're not checking all blocks, only an exit
   // point; this _should_ work :) but needs to change if we
   // ever do flow-sensitive parsing
   bool getSharingFuncs(std::set<int_function *> &funcs);

   // Slower version of the above that also finds functions that occupy
   // the same address range, even if they do not share blocks - this can
   // be caused by overlapping but disjoint assembly sequences
   bool getOverlappingFuncs(std::set<int_function *> &funcs);
   bool getOverlappingFuncs(int_block *b, std::set<int_function *> &funcs);

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////


   const pdvector< int_parRegion* > &parRegions();

   bool containsSharedBlocks() const { return ifunc_->containsSharedBlocks(); }
   unsigned getNumDynamicCalls();

    // Fill the <callers> vector with pointers to the statically-determined
    // list of functions that call this function.
    void getStaticCallers(pdvector <int_function *> &callers);

    // Similar to the above, but gives us the actual block
    void getCallers(std::vector<int_block *> &callers);

   codeRange *copy() const;

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


   //bool removePoint(instPoint *point);
    void deleteBlock(int_block *block);
    void splitBlock(image_basicBlock *origBlock, image_basicBlock *newBlock);
	void triggerModified();

    void removeFromAll();
    int_block *setNewEntryPoint();
    void getReachableBlocks(const std::set<int_block*> &exceptBlocks,
                            const std::list<int_block*> &seedBlocks,
                            std::set<int_block*> &reachBlocks);//output
   

    // So we can assert(consistency());
    bool consistency() const;

        // Convenience function for int_block; get the base address
    // where we were loaded (AKA "int layer addr - image layer offset")
    Address baseAddr() const;
 private:

   ///////////////////// Basic func info
   Address addr_; // Absolute address of the start of the function
   Address ptrAddr_; // Absolute address of the function descriptor, if exists

   image_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// image_funcs to int_funcs

   ///////////////////// CFG and function body
   BlockSet blocks_; 
   BlockSet callBlocks_;
   BlockSet exitBlocks_;
   int_block *entry_;

   instPoint *entryPoint_; 
   typedef std::map<int_block *, instPoint *> InstPointMap;
   typedef std::map<Address, instPoint *> ArbitraryMapInt;
   typedef std::map<int_block *, ArbitraryMapInt> ArbitraryMap;
   typedef std::map<int_edge *, instPoint *> EdgePointMap;

   InstPointMap exitPoints_;
   InstPointMap preCallPoints_;
   InstPointMap postCallPoints_;
   InstPointMap blockEntryPoints_;
   ArbitraryMap preInsnPoints_;
   ArbitraryMap postInsnPoints_;
   EdgePointMap edgePoints_;

   // We need some method of doing up-pointers
   typedef std::map<image_basicBlock *, int_block *> BlockMap;
   BlockMap blockMap_;
   
   typedef std::map<ParseAPI::Edge *, int_edge *> EdgeMap;
   EdgeMap edgeMap_;

   Address handlerFaultAddr_; /* if this is a signal handler, faultAddr_ is 
                                 set to -1, or to the address of the fault 
                                 that last caused the handler to be invoked. */
   Address handlerFaultAddrAddr_; 

   bool isBeingInstrumented_;

   std::map<Address, instPoint *> instPsByAddr_;

   //////////////////////////  Parallel Regions 
   pdvector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */

   void addint_block(int_block *instance);

#if defined(os_windows) 
   callType callingConv;
   int paramSize;
#endif

    // Create and register
    int_block *createBlock(image_basicBlock *ib);
    int_block *createBlockFork(const int_block *parent);
    int_edge *getEdge(ParseAPI::Edge *, int_block *src, int_block *trg);
    void removeEdge(int_edge *e);

    void findPoints(int_block *, std::set<instPoint *> &foundPoints) const;
    bool validPoint(instPoint *) const;

    // HACKITY!
    std::map<int_block *, int_function *> callees_;

    // Defensive mode
    BlockSet unresolvedCF_;
    BlockSet abruptEnds_;


};



#endif /* FUNCTION_H */
