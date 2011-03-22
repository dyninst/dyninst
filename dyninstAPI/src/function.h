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
#include "parse-cfg.h"

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

class func_instance;
class block_instance;
class edge_instance;
class funcMod;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;


class func_instance : public patchTarget {
  friend class block_instance;
  friend class edge_instance;
 public:
   //static std::string emptyString;

   // Almost everything gets filled in later.
   func_instance(parse_func *f,
		Address baseAddr,
                mapped_module *mod);

   func_instance(const func_instance *parent,
                mapped_module *child_mod,
                process *childP);

   ~func_instance();

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
   parse_func *ifunc();
   mapped_module *mod() const;
   mapped_object *obj() const;
   //process *proc() const;
   AddressSpace *proc() const;

   // Necessary for BPatch_set which needs a structure with a ()
   // operator. Odd.
   struct cmpAddr {
     int operator() (const func_instance *f1, const func_instance *f2) const {
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
   friend ostream &operator<<(ostream &os, func_instance &f);

   ////////////////////////////////////////////////
   // Process-dependent (inter-module) parsing
   ////////////////////////////////////////////////

   ////////////////////////////////////////////////
   // CFG and other function body methods
   ////////////////////////////////////////////////

   typedef std::set<block_instance *, block_instance::compare> BlockSet;
   const BlockSet &blocks();

   block_instance *entryBlock();
   const BlockSet &callBlocks();
   const BlockSet &exitBlocks();

   // Kevin's defensive mode shtuff
   // Blocks that have a sink target, essentially. 
   const BlockSet &unresolvedCF();
   // Blocks where we provisionally stopped parsing because things looked weird.
   const BlockSet &abruptEnds();

   // Perform a lookup (either linear or log(n)).
   bool findBlocksByAddr(Address addr, std::set<block_instance *> &blocks);
   bool findBlocksByOffsetInFunc(Address offset, std::set<block_instance *> &blocks) {
       return findBlocksByAddr(offset + baseAddr(), blocks);
   }
   bool findBlocksByOffset(Address offset, std::set<block_instance *> &blocks) {
       return findBlocksByAddr(offsetToAddr(offset), blocks);
   }

   block_instance *findBlock(ParseAPI::Block *block);
   block_instance *findBlockByEntry(Address addr);
   block_instance *findOneBlockByAddr(Address Addr);


   void findBlocksByRange(std::vector<block_instance*> &funcs, 
                          Address start, Address end);

   void addMissingBlock(parse_block *imgBlock);
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
   func_instance *findCallee(block_instance *callBlock);


   ////////////////////////////////////////////////
   // Instpoints!
   ////////////////////////////////////////////////

   instPoint *entryPoint();
   instPoint *exitPoint(block_instance *exitBlock);

   instPoint *findPoint(instPoint::Type type);
   instPoint *findPoint(instPoint::Type type, block_instance *block);
   const std::map<Address, instPoint *> &findPoints(instPoint::Type type, block_instance *block);

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
   // block_instance will not be shared (they are per function),
   // but the underlying parse_block records the sharing status. 
   // So dodge through to the image layer and find out that info. 
   // Returns true if such functions exist.

   bool getSharingFuncs(block_instance *b,
                        std::set<func_instance *> &funcs);

   // The same, but for any function that overlaps with any of
   // our basic blocks.
   // OPTIMIZATION: we're not checking all blocks, only an exit
   // point; this _should_ work :) but needs to change if we
   // ever do flow-sensitive parsing
   bool getSharingFuncs(std::set<func_instance *> &funcs);

   // Slower version of the above that also finds functions that occupy
   // the same address range, even if they do not share blocks - this can
   // be caused by overlapping but disjoint assembly sequences
   bool getOverlappingFuncs(std::set<func_instance *> &funcs);
   bool getOverlappingFuncs(block_instance *b, std::set<func_instance *> &funcs);

   ////////////////////////////////////////////////
   // Misc
   ////////////////////////////////////////////////


   const pdvector< int_parRegion* > &parRegions();

   bool containsSharedBlocks() const { return ifunc_->containsSharedBlocks(); }
   unsigned getNumDynamicCalls();

    // Fill the <callers> vector with pointers to the statically-determined
    // list of functions that call this function.
    void getStaticCallers(pdvector <func_instance *> &callers);

    // Similar to the above, but gives us the actual block
    void getCallers(std::vector<block_instance *> &callers);

   codeRange *copy() const;

#if defined(arch_power)
   bool savesReturnAddr() const { return ifunc_->savesReturnAddr(); }
#endif

   void updateForFork(process *childProcess, const process *parentProcess);

#if defined(os_windows) 
   //Calling convention for this function
   callType func_instance::getCallingConvention();
   int getParamSize() { return paramSize; }
   void setParamSize(int s) { paramSize = s; }
#endif


   //bool removePoint(instPoint *point);
    void deleteBlock(block_instance *block);
    void splitBlock(parse_block *origBlock, parse_block *newBlock);
	void triggerModified();

    void removeFromAll();
    block_instance *setNewEntryPoint();
    void getReachableBlocks(const std::set<block_instance*> &exceptBlocks,
                            const std::list<block_instance*> &seedBlocks,
                            std::set<block_instance*> &reachBlocks);//output
   

    // So we can assert(consistency());
    bool consistency() const;

        // Convenience function for block_instance; get the base address
    // where we were loaded (AKA "int layer addr - image layer offset")
    Address baseAddr() const;
 private:

   ///////////////////// Basic func info
   Address addr_; // Absolute address of the start of the function
   Address ptrAddr_; // Absolute address of the function descriptor, if exists

   parse_func *ifunc_;
   mapped_module *mod_; // This is really a dodge; translate a list of
			// parse_funcs to int_funcs

   ///////////////////// CFG and function body
   BlockSet blocks_; 
   BlockSet callBlocks_;
   BlockSet exitBlocks_;
   block_instance *entry_;

   instPoint *entryPoint_; 
   typedef std::map<block_instance *, instPoint *> InstPointMap;
   typedef std::map<Address, instPoint *> ArbitraryMapInt;
   typedef std::map<block_instance *, ArbitraryMapInt> ArbitraryMap;
   typedef std::map<edge_instance *, instPoint *> EdgePointMap;

   InstPointMap exitPoints_;
   InstPointMap preCallPoints_;
   InstPointMap postCallPoints_;
   InstPointMap blockEntryPoints_;
   ArbitraryMap preInsnPoints_;
   ArbitraryMap postInsnPoints_;
   EdgePointMap edgePoints_;

   // We need some method of doing up-pointers
   typedef std::map<parse_block *, block_instance *> BlockMap;
   BlockMap blockMap_;
   
   typedef std::map<ParseAPI::Edge *, edge_instance *> EdgeMap;
   EdgeMap edgeMap_;

   Address handlerFaultAddr_; /* if this is a signal handler, faultAddr_ is 
                                 set to -1, or to the address of the fault 
                                 that last caused the handler to be invoked. */
   Address handlerFaultAddrAddr_; 

   bool isBeingInstrumented_;

   std::map<Address, instPoint *> instPsByAddr_;

   //////////////////////////  Parallel Regions 
   pdvector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */

   void addblock_instance(block_instance *instance);

#if defined(os_windows) 
   callType callingConv;
   int paramSize;
#endif

    // Create and register
    block_instance *createBlock(parse_block *ib);
    block_instance *createBlockFork(const block_instance *parent);
    edge_instance *getEdge(ParseAPI::Edge *, block_instance *src, block_instance *trg);
    void removeEdge(edge_instance *e);

    void findPoints(block_instance *, std::set<instPoint *> &foundPoints) const;
    bool validPoint(instPoint *) const;

    // HACKITY!
    std::map<block_instance *, func_instance *> callees_;

    // Defensive mode
    BlockSet unresolvedCF_;
    BlockSet abruptEnds_;


};



#endif /* FUNCTION_H */
