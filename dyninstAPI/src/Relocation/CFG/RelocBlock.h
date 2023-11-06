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

#if !defined(PATCHAPI_TRACE_H_)
#define PATCHAPI_TRACE_H_

#include <list>
#include <string>
#include <utility>
#include "dyntypes.h"
#include "dyninstAPI/src/codegen.h" // codeGen
#include "dyninstAPI/src/function.h"
#include "instructionAPI/h/Instruction.h" // Instruction::Ptr
#include "CFG.h"
#include "dyninstAPI/src/Relocation/CodeMover.h"
#include "RelocEdge.h"

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

class CFWidget;
typedef boost::shared_ptr<CFWidget> CFWidgetPtr;
class Widget;
typedef boost::shared_ptr<Widget> WidgetPtr;

struct RelocEdge;
struct RelocEdges;

class RelocBlock {
  friend class Transformer;

 public:
   typedef int Label;
   static int RelocBlockID;
   typedef std::list<WidgetPtr> WidgetList;
   typedef enum {
      Relocated,
      Instrumentation,
      Stub } Type;

   // Standard creation
   static RelocBlock *createReloc(block_instance *block, func_instance *func);
   // Instpoint creation
   static RelocBlock *createInst(instPoint *point, Address a, 
                        block_instance *block, func_instance *func);
   // Stub creation; we're creating an empty RelocBlock associated
   // with some other block/function/thing
   static RelocBlock *createStub(block_instance *block, func_instance *func);

   RelocBlock *next() { return next_; }
   RelocBlock *prev() { return prev_; }
   ~RelocBlock() {}

   void setNext(RelocBlock *next) { next_ = next; }
   void setPrev(RelocBlock *prev) { prev_ = prev; }

   bool linkRelocBlocks(RelocGraph *cfg);
   bool determineSpringboards(PriorityMap &p);
   void determineNecessaryBranches(RelocBlock *successor);

   Address origAddr() const { return origAddr_; }
   int id() const { return id_; }
   func_instance *func() const { return func_; }
   block_instance *block() const { return block_; }
   mapped_object *obj() const;
   std::string format() const;
   Label getLabel() const;
   
   // Non-const for use by transformer classes
   WidgetList &elements() { return elements_; }
   CFWidgetPtr &cfWidget() { return cfWidget_; }
   void setCF(CFWidgetPtr cf);

   // Code generation
   bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift);
   bool extractTrackers(CodeTracker &);
   bool generate(const codeGen &templ, CodeBuffer &buffer);

   void setType(Type type);
   Type type() const { return type_; }

   // Set up the CFWidget with our out-edges
   bool finalizeCF();

   RelocEdges *ins() { return &inEdges_; }
   RelocEdges *outs() { return &outEdges_; }

 private:
   
  RelocBlock(block_instance *block, func_instance *f)
     : origAddr_(block->start()),
      block_(block),
      func_(f),
      id_(RelocBlockID++),
      label_(-1),
      origRelocBlock_(true),
      prev_(NULL),
      next_(NULL),
      type_(Relocated) {}
   // Constructor for a trace inserted later
  RelocBlock(Address a, block_instance *b, func_instance *f)
      :origAddr_(a),
      block_(b),
      func_(f),
      id_(RelocBlockID++),
      label_(-1),
      origRelocBlock_(false),
      prev_(NULL),
      next_(NULL),
      type_(Instrumentation) { 
   }

  RelocBlock(Address a, block_instance *b, func_instance *f, bool relocateType)
      :origAddr_(a),
      block_(b),
      func_(f),
      id_(RelocBlockID++),
      label_(-1),
      origRelocBlock_(false),
      prev_(NULL),
      next_(NULL),
      type_(Relocated) { 
         if(relocateType == true)
            type_ = Relocated;
         else 
            type_ = Instrumentation;
   }


   typedef enum {
      InEdge,
      OutEdge } EdgeDirection;

   void createCFWidget();
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
   bool origRelocBlock_;
   
   WidgetList elements_;
   // This is convienient to avoid tons of dynamic_cast
   // equivalents
   CFWidgetPtr cfWidget_;

   // We're building a mini-CFG, so might as well make it obvious. 
   // Also, this lets us reassign edges. We sort by edge type. 
   RelocEdges inEdges_;
   RelocEdges outEdges_;

   // We use the standard code generation mechanism of having a doubly-linked
   // list overlaid on a graph. Use the list to traverse in layout order; use the graph
   // to traverse in control flow order.

   RelocBlock *prev_;
   RelocBlock *next_;

   Type type_;
};

}
}
#endif
