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

#if !defined(_R_T_MOVEMENT_ANALYSIS_H_)
#define _R_T_MOVEMENT_ANALYSIS_H_

#include <list>
#include <map>
#include <utility>
#include "Transformer.h"
#include "dyninstAPI/src/LinearVariable.h"

#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/AbslocInterface.h" // And more of the same

#include "common/h/Graph.h" // PC-sensitive transformer
#include "dataflowAPI/h/SymEval.h" // Variable class
#include "Movement-adhoc.h"
#include "../CodeMover.h"

class parse_block;
class parse_func;

namespace Dyninst {

// ROSE symeval AST types
 namespace DataflowAPI {
 class BottomAST;
 class ConstantAST;
 class AbsRegionAST;
 class RoseAST;
 }

 class StackAST;

namespace Relocation {

class ExtPCSensVisitor : public ASTVisitor {
 public:
  ExtPCSensVisitor(const AbsRegion &a);
  
  virtual AST::Ptr visit(AST *);
  virtual AST::Ptr visit(DataflowAPI::BottomAST *);
  virtual AST::Ptr visit(DataflowAPI::ConstantAST *);
  virtual AST::Ptr visit(DataflowAPI::VariableAST *);
  virtual AST::Ptr visit(DataflowAPI::RoseAST *);
  virtual AST::Ptr visit(StackAST *);
  virtual ASTVisitor::ASTPtr visit(InputVariableAST *x) { return ASTVisitor::visit(x); }
  virtual ASTVisitor::ASTPtr visit(ReferenceAST *x) { return ASTVisitor::visit(x); }
  virtual ASTVisitor::ASTPtr visit(StpAST *x) { return ASTVisitor::visit(x); }
  virtual ASTVisitor::ASTPtr visit(YicesAST *x) { return ASTVisitor::visit(x); }
  virtual ASTVisitor::ASTPtr visit(SemanticsAST *x) { return ASTVisitor::visit(x); }


  virtual ~ExtPCSensVisitor() {}
  
  bool isExtSens(AST::Ptr a);

 private:
  bool assignPC_;
  bool isExtSens_;

  typedef linVar<Dyninst::DataflowAPI::Variable> DiffVar;

  std::stack<DiffVar > diffs_;
};
 
class PCSensitiveTransformer : public Transformer {
  typedef boost::shared_ptr<RelocInsn> RelocInsnPtr;
  typedef std::list<Assignment::Ptr> AssignList;

 public:
  virtual bool process(RelocBlock *, RelocGraph *);

  PCSensitiveTransformer(AddressSpace *as, PriorityMap &) 
        : aConverter(false, false), addrSpace(as),
     Sens_(0), extSens_(0), intSens_(0), thunk_(0), overApprox_(0), adhoc(as) {}
  virtual ~PCSensitiveTransformer() {}

  static void invalidateCache(func_instance *);
  static void invalidateCache(const block_instance *);

 private:
  bool analysisRequired(RelocBlock *);

  bool isPCSensitive(InstructionAPI::Instruction insn,
					 Address addr,
					 const func_instance *func,
					 const block_instance *block,
					 AssignList &sensitiveAssignment);
  Graph::Ptr forwardSlice(Assignment::Ptr ptr,
			  parse_block *block,
			  parse_func *func);
  bool determineSensitivity(Graph::Ptr slice,
			    bool &intSens,
			    bool &extSens);

  bool insnIsThunkCall(InstructionAPI::Instruction insn,
					   Address addr,
					   Absloc &destination);
  void handleThunkCall(RelocBlock *b_iter,
                       RelocGraph *cfg,
		       WidgetList::iterator &iter,
		       Absloc &destination);
  void emulateInsn(RelocBlock *b_iter,
				   RelocGraph *cfg,
				   WidgetList::iterator &iter,
				   InstructionAPI::Instruction insn,
				   Address addr);
  
  bool exceptionSensitive(Address addr, const block_instance *bbl);

  static void cacheAnalysis(const block_instance *bbl, Address addr, bool intSens, bool extSens);
  static bool queryCache(const block_instance *bbl, Address addr, bool &intSens, bool &extSens);


  AssignmentConverter aConverter;

  AddressSpace *addrSpace;

  long Sens_;
  long extSens_;
  long intSens_;
  long thunk_;
  long overApprox_;

  // And for times we don't want the overhead - if non-defensive or
  // system libraries
  adhocMovementTransformer adhoc;
  typedef std::pair<bool, bool> CacheData;
  typedef std::map<Address, CacheData> CacheEntry;
  typedef std::map<const block_instance *, CacheEntry > AnalysisCache;
  static AnalysisCache analysisCache_;
  
};

}
}

#endif
