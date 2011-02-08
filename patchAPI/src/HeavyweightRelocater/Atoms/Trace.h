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

#if !defined(PATCHAPI_TRACE_H_)
#define PATCHAPI_TRACE_H_

#include "dyn_detail/boost/shared_ptr.hpp" // shared_ptr
#include "common/h/Types.h" // Address
#include "dyninstAPI/src/codegen.h" // codeGen
#include "instructionAPI/h/Instruction.h" // Instruction::Ptr
#include "CFG.h"

#include <list> // stl::list

class baseTramp;
class baseTrampInstance;

namespace Dyninst {
namespace PatchAPI {

class Transformer;
class Atom;
class RelocInsn;
class Inst;
class CFAtom;
class Trace;

class TargetInt;

struct Patch;
class TrackerElement;
class CodeTracker;
class CodeBuffer;

class CFAtom;
typedef dyn_detail::boost::shared_ptr<CFAtom> CFAtomPtr;

class Trace {
  friend class Transformer;

 public:
   typedef int Label;
   static int TraceID;
   typedef std::list<Atom::Ptr> AtomList;
   typedef dyn_detail::boost::shared_ptr<Trace> Ptr;
   typedef std::list<Patch *> Patches;

   static Ptr create(PatchAPI::Block *block);
   static Ptr create(Atom::Ptr atom, Address a, PatchAPI::Function *func);
   bool linkTraces(std::map<Block *, Trace::Ptr> &traces);
   void determineNecessaryBranches(Trace *successor);

   Address origAddr() const { return origAddr_; }
   int id() const { return id_; }
   Block *block() const { return block_; }
   Function *func() const;
   std::string format() const;
   Label getLabel() { assert(label_ != -1);  return label_; };
   
   // Non-const for use by transformer classes
   AtomList &elements() { return elements_; }
   CFAtomPtr cfAtom() { return cfAtom_; }

   // Code generation
   bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);
   bool extractTrackers(CodeTracker &);
   bool generate(const codeGen &templ, CodeBuffer &buffer);
   
   
 private:
   
  Trace(Block *block)
     :origAddr_(block->start()),
      block_(block),
      func_(block->func()),
      id_(TraceID++),
      label_(-1) {};
  Trace(Address origAddr, Function *func)
     :origAddr_(origAddr),
      block_(NULL),
      func_(func),
      id_(TraceID++),
      label_(-1) { 
   };

   struct Successor {
   Successor() : addr(0), type(ParseAPI::NOEDGE), target(NULL) {};
      Address addr;
      ParseAPI::EdgeTypeEnum type;
      TargetInt *target;
   };
   typedef std::list<Successor> Successors;

   void createCFAtom();
   void getSuccessors(Successors &, const std::map<Block *, Trace::Ptr> &traces);
   void preserveBlockGap();
   std::pair<bool, Address> getJumpTarget();

   Address origAddr_;
   Block *block_;
   Function *func_;  
   int id_;
   Label label_;  
   
   AtomList elements_;
   // This is convienient to avoid tons of dynamic_cast
   // equivalents
   CFAtomPtr cfAtom_;
   Patches patches_;
};

};
};
#endif
