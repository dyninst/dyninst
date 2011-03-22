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


#if !defined(_R_CODE_MOVER_H_)
#define _R_CODE_MOVER_H_

#include "CFG.h"
#include "dyn_detail/boost/shared_ptr.hpp"
#include "common/h/Types.h"
#include <list>
#include <map>
#include "dyninstAPI/src/codegen.h" // codeGen structure

#include "Transformers/Transformer.h"

#include "Springboard.h"
#include "CodeBuffer.h"
#include "Springboard.h"

class codeGen;
class block_instance;
class func_instance;

namespace Dyninst {
  class AddressMapper;

namespace Relocation {

class Trace;
class Transformer;
class CodeMover;
class CodeTracker;

typedef std::map<block_instance *, Priority> PriorityMap;

class CodeMover {
 public:
  typedef dyn_detail::boost::shared_ptr<CodeMover> Ptr;
  typedef dyn_detail::boost::shared_ptr<Trace> TracePtr;
  typedef std::list<TracePtr> TraceList;
  typedef std::map<block_instance *, TracePtr> TraceMap;
  typedef std::set<func_instance *> FuncSet;
  typedef std::set<block_instance *> BlockSet;

  // A generic mover of code; an instruction, a basic block, or
  // a function. This is the algorithm (fixpoint) counterpart
  // of the classes described in relocation.h
  
  // Input: 
  //  A structured description of code in terms of an instruction, a
  //    block, a function, or a set of functions;
  //  A starting address for the moved code 
  //
  // Output: a buffer containing the moved code 

  // We take a CodeTracker as a reference parameter so that we don't 
  // have to copy it on output; CodeMovers are designed to be discarded
  // while CodeTrackers survive.
  static Ptr create(CodeTracker &);

  bool addFunctions(FuncSet::const_iterator begin, FuncSet::const_iterator end);

  // Apply the given Transformer to all blocks in the Mover
  bool transform(Transformer &t);
  
  // Does all the once-only work to generate code.
  bool initialize(const codeGen &genTemplate);

  // Allocates an internal buffer and relocates the code provided
  // to the constructor. Returns true for success or false for
  // catastrophic failure.
  // The codeGen parameter allows specification of various
  // codeGen-carried information
  bool relocate(Address addr);

  // Aaand debugging functionality
  void disassemble() const;

  void extractDefensivePads(AddressSpace *);

  // Get a map from original addresses to new addresses
  // for all blocks
  typedef std::map<Address, Address> EntryMap;
  const EntryMap &entryMap() { return entryMap_; }

  // Some things (LocalCFTransformer) require the
  // map of (original) addresses to TracePtrs
  // so they can refer to blocks other than those
  // they are transforming.
  const TraceMap &blockMap() const { return blockMap_; }
  // Not const so we can add others to it. 
  SpringboardMap &sBoardMap(AddressSpace *as);
  // Not const so that Transformers can modify it...
  PriorityMap &priorityMap();

  // Get either an estimate (pre-relocation) or actual
  // size
  unsigned size() const;

  // (void *) to start of code
  void *ptr() const;

  std::string format() const;

 private:
    
  CodeMover(CodeTracker &t) : addr_(0), tracker_(t), tracesFinalized_(false) {};

  
  void setAddr(Address &addr) { addr_ = addr; }
  template <typename TraceIter>
    bool addTraces(TraceIter begin, TraceIter end);

  bool addTrace(block_instance *block);

  void createInstrumentationSpringboards(AddressSpace *as);

  void finalizeTraces();

  TraceList blocks_;

  TraceMap blockMap_;

  Address addr_;

  EntryMap entryMap_;

  PriorityMap priorityMap_;
  
  SpringboardMap sboardMap_;

  CodeTracker &tracker_;

  CodeBuffer buffer_;
  codeGen &gen();

  bool tracesFinalized_;
  
};


};

};

#endif
