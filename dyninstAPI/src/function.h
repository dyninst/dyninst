/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#include "instPoint.h"
#include "PatchCFG.h"

class process;
class mapped_module;
class mapped_object;

class func_instance;
class block_instance;
class edge_instance;
class instPoint;

typedef enum callType {
  unknown_call,
  cdecl_call,
  stdcall_call,
  fastcall_call,
  thiscall_call
} callType;


class func_instance : public patchTarget, public Dyninst::PatchAPI::PatchFunction {
  friend class block_instance;
  friend class edge_instance;
  friend class instPoint;
  public:
    // Almost everythcing gets filled in later.
    func_instance(parse_func *f,
                  Address baseAddr,
                  mapped_module *mod);

    func_instance(const func_instance *parent,
                  mapped_module *child_mod);

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

  Address getPtrAddress() const {return ptrAddr_;}
  Address addr() const { return addr_; }

  // Not defined here so we don't have to play header file magic
  // Not const; we can add names via the Dyninst layer
  parse_func *ifunc() { return ifunc_; }
  mapped_module *mod() const { return mod_; }
  mapped_object *obj() const;

  //process *proc() const;
  AddressSpace *proc() const;

  std::string format() const;

  ////////////////////////////////////////////////
  // CFG and other function body methods
  ////////////////////////////////////////////////
  typedef AddrOrderedBlockSet BlockSet;

  const BlockSet &blocks();

  block_instance *entryBlock();
  const BlockSet &callBlocks();
  const BlockSet &exitBlocks();

  // Kevin's defensive mode shtuff
  // Blocks that have a sink target, essentially.
  const BlockSet &unresolvedCF();
  // Blocks where we provisionally stopped parsing because things looked weird.
  const BlockSet &abruptEnds();

  block_instance *getBlock(const Address addr);

  Offset addrToOffset(const Address addr) const;

  bool hasNoStackFrame() const {return ifunc_->hasNoStackFrame();}
  bool savesFramePointer() const {return ifunc_->savesFramePointer();}

  ////////////////////////////////////////////////
  // Legacy/inter-module calls. Arguably should be an
  // interprocedural edge, but I expect that would
  // break all manner of things
  ////////////////////////////////////////////////
  func_instance *findCallee(block_instance *callBlock);

  bool isSignalHandler() {return handlerFaultAddr_ != 0;}
  Address getHandlerFaultAddr() {return handlerFaultAddr_;}
  Address getHandlerFaultAddrAddr() {return handlerFaultAddrAddr_;}
  void fixHandlerReturnAddr(Address newAddr);
  void setHandlerFaultAddr(Address fa);
  void setHandlerFaultAddrAddr(Address faa, bool set);

  bool isInstrumentable();

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
  template <class OutputIterator>
    void getCallerBlocks(OutputIterator result);
  template <class OutputIterator>
    void getCallerFuncs(OutputIterator result);

#if defined(arch_power)
  bool savesReturnAddr() const { return ifunc_->savesReturnAddr(); }
#endif

#if defined(os_windows)
  //Calling convention for this function
  callType func_instance::getCallingConvention();
  int getParamSize() { return paramSize; }
  void setParamSize(int s) { paramSize = s; }
#endif

  void removeFromAll();
  void getReachableBlocks(const std::set<block_instance*> &exceptBlocks,
                          const std::list<block_instance*> &seedBlocks,
                          std::set<block_instance*> &reachBlocks);//output


  // So we can assert(consistency());
  bool consistency() const;

  instPoint *findPoint(instPoint::Type type, bool create);
  instPoint *findPoint(instPoint::Type type, block_instance *b, bool create);
  instPoint *findPoint(instPoint::Type type, block_instance *b,
                       Address a, InstructionAPI::Instruction::Ptr ptr,
                       bool trusted, bool create);
  // And the "mass" version of the above
  bool findInsnPoints(instPoint::Type type, block_instance *b,
                      InsnInstpoints::const_iterator &begin,
                      InsnInstpoints::const_iterator &end);

  instPoint *findPoint(instPoint::Type type, edge_instance *e, bool create);

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
  // Defensive mode
  BlockSet unresolvedCF_;
  BlockSet abruptEnds_;

  ///////////////////// Function-level instPoints
  FuncInstpoints points_;
  std::map<block_instance *, BlockInstpoints> blockPoints_;
  std::map<edge_instance *, EdgeInstpoints> edgePoints_;


  Address handlerFaultAddr_; /* if this is a signal handler, faultAddr_ is
                                set to -1, or to the address of the fault
                                that last caused the handler to be invoked. */
  Address handlerFaultAddrAddr_;

  //////////////////////////  Parallel Regions
  pdvector<int_parRegion*> parallelRegions_; /* pointer to the parallel regions */

  void addblock_instance(block_instance *instance);

#if defined(os_windows)
  callType callingConv;
  int paramSize;
#endif
};

template <class OutputIterator>
void func_instance::getCallerBlocks(OutputIterator result)
{
  if(!ifunc_ || !ifunc_->entryBlock())
    return;

  const block_instance::edgelist &ins = entryBlock()->sources();
  for (block_instance::edgelist::const_iterator iter = ins.begin();
       iter != ins.end(); ++iter) {
    *result = (*iter)->src();
    ++result;
  }
}

template <class OutputIterator>
void func_instance::getCallerFuncs(OutputIterator result)
{
  std::set<block_instance *> callerBlocks;
  getCallerBlocks(std::inserter(callerBlocks, callerBlocks.end()));
  for (std::set<block_instance *>::iterator iter = callerBlocks.begin();
       iter != callerBlocks.end(); ++iter) {
    (*iter)->getFuncs(result);
  }
}


#endif /* FUNCTION_H */
