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
#include "dyninstAPI/src/function.h"
#include "instructionAPI/h/Instruction.h" // Instruction::Ptr
#include "CFG.h"
#include "dyninstAPI/src/Relocation/CodeMover.h"

class baseTramp;
class block_instance;
class func_instance;

namespace Dyninst {
namespace Relocation {

class Transformer;

class TargetInt;

class TrackerElement;
class CodeTracker;
class CodeBuffer;

class CFAtom;
typedef dyn_detail::boost::shared_ptr<CFAtom> CFAtomPtr;
class Atom;
typedef dyn_detail::boost::shared_ptr<Atom> AtomPtr;

struct RelocEdge;
class Trace; 

struct RelocEdge {
   RelocEdge(TargetInt *s, TargetInt *t, ParseAPI::EdgeTypeEnum e) :
     src(s), trg(t), type(e) {};

   ~RelocEdge();

   TargetInt *src;
   TargetInt *trg;
   ParseAPI::EdgeTypeEnum type;
};

struct RelocEdges {
   RelocEdges() {};

   typedef std::list<RelocEdge *>::iterator iterator;
   typedef std::list<RelocEdge *>::const_iterator const_iterator;
   iterator begin() { return edges.begin(); }
   iterator end() { return edges.end(); }
   const_iterator begin() const { return edges.begin(); }
   const_iterator end() const { return edges.end(); }
   void push_back(RelocEdge *e) { edges.push_back(e); }
   void insert(RelocEdge *e) { edges.push_back(e); }

   RelocEdge *find(ParseAPI::EdgeTypeEnum e);
   void erase(RelocEdge *);
   
   bool contains(ParseAPI::EdgeTypeEnum e);
   std::list<RelocEdge *> edges;
};

class Trace {
  friend class Transformer;

 public:
   typedef int Label;
   static int TraceID;
   typedef std::list<AtomPtr> AtomList;
   typedef enum {
      Relocated,
      Instrumentation,
      Stub } Type;

   // Standard creation
   static Trace *createReloc(block_instance *block, func_instance *func);
   // Instpoint creation
   static Trace *createInst(instPoint *point, Address a, 
                        block_instance *block, func_instance *func);
   // Stub creation; we're creating an empty Trace associated
   // with some other block/function/thing
   static Trace *createStub(block_instance *block, func_instance *func);

   Trace *next() { return next_; }
   Trace *prev() { return prev_; }
   ~Trace() {};

   void setNext(Trace *next) { next_ = next; }
   void setPrev(Trace *prev) { prev_ = prev; }

   bool linkTraces(RelocGraph *cfg);
   bool determineSpringboards(PriorityMap &p);
   void determineNecessaryBranches(Trace *successor);

   Address origAddr() const { return origAddr_; }
   int id() const { return id_; }
   func_instance *func() const { return func_; }
   block_instance *block() const { return block_; }
   mapped_object *obj() const;
   std::string format() const;
   Label getLabel() const;
   
   // Non-const for use by transformer classes
   AtomList &elements() { return elements_; }
   CFAtomPtr &cfAtom() { return cfAtom_; }

   // Code generation
   bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);
   bool extractTrackers(CodeTracker &);
   bool generate(const codeGen &templ, CodeBuffer &buffer);

   void setType(Type type);
   Type type() const { return type_; }

   // Set up the CFAtom with our out-edges
   bool finalizeCF();

   RelocEdges *ins() { return &inEdges_; }
   RelocEdges *outs() { return &outEdges_; }

 private:
   
  Trace(block_instance *block, func_instance *f)
     : origAddr_(block->start()),
      block_(block),
      func_(f),
      id_(TraceID++),
      label_(-1),
      origTrace_(true),
      prev_(NULL),
      next_(NULL),
      type_(Relocated) {};
   // Constructor for a trace inserted later
  Trace(Address a, block_instance *b, func_instance *f)
      :origAddr_(a),
      block_(b),
      func_(f),
      id_(TraceID++),
      label_(-1),
      origTrace_(false),
      prev_(NULL),
      next_(NULL),
      type_(Instrumentation) { 
   };

   typedef enum {
      InEdge,
      OutEdge } EdgeDirection;

   void createCFAtom();
   void getPredecessors(RelocGraph *cfg);
   void getSuccessors(RelocGraph *cfg);
   void processEdge(EdgeDirection e, edge_instance *edge, RelocGraph *cfg);

   void preserveBlockGap();
   std::pair<bool, Address> getJumpTarget();

   bool isNecessary(TargetInt *target,
                    ParseAPI::EdgeTypeEnum edgeType);

   Address origAddr_;
   block_instance *block_;
   // If we're a func-specific copy
   func_instance *func_; 

   int id_;
   Label label_;  
   bool origTrace_;
   
   AtomList elements_;
   // This is convienient to avoid tons of dynamic_cast
   // equivalents
   CFAtomPtr cfAtom_;

   // We're building a mini-CFG, so might as well make it obvious. 
   // Also, this lets us reassign edges. We sort by edge type. 
   RelocEdges inEdges_;
   RelocEdges outEdges_;

   // We use the standard code generation mechanism of having a doubly-linked
   // list overlaid on a graph. Use the list to traverse in layout order; use the graph
   // to traverse in control flow order.

   Trace *prev_;
   Trace *next_;

   Type type_;
};

};
};
#endif
