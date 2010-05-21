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

#if !defined(_R_T_MOVEMENT_ANALYSIS_H_)
#define _R_T_MOVEMENT_ANALYSIS_H_

#include "r_t_Base.h"
#include "LinearVariable.h"

#include "symEval/h/Absloc.h" // MemEmulator analysis
#include "symEval/h/AbslocInterface.h" // And more of the same

#include "Graph.h" // PC-sensitive transformer
#include "symEval/h/SymEval.h" // Variable class

namespace Dyninst {

// ROSE symeval AST types
 namespace SymbolicEvaluation {
 class BottomAST;
 class ConstantAST;
 class AbsRegionAST;
 class RoseAST;
 };

 class StackAST;

namespace Relocation {

class ExtPCSensVisitor : public ASTVisitor {
 public:
  ExtPCSensVisitor(const AbsRegion &a);
  
  virtual AST::Ptr visit(AST *);
  virtual AST::Ptr visit(SymbolicEvaluation::BottomAST *);
  virtual AST::Ptr visit(SymbolicEvaluation::ConstantAST *);
  virtual AST::Ptr visit(SymbolicEvaluation::VariableAST *);
  virtual AST::Ptr visit(SymbolicEvaluation::RoseAST *);
  virtual AST::Ptr visit(StackAST *);
  virtual ~ExtPCSensVisitor() {};
  
  bool isExtSens(AST::Ptr a);

 private:
  bool assignPC_;
  bool isExtSens_;

  typedef linVar<Dyninst::SymbolicEvaluation::Variable> DiffVar;

  std::stack<DiffVar > diffs_;
};
 
class PCSensitiveTransformer : public Transformer {
  typedef dyn_detail::boost::shared_ptr<RelocInsn> RelocInsnPtr;
  typedef std::list<Assignment::Ptr> AssignList;

 public:
  virtual bool processBlock(BlockList::iterator &);
 PCSensitiveTransformer(AddressSpace *as, PriorityMap &p) : 
  aConverter(false), addrSpace(as), priMap(p) {};
  virtual ~PCSensitiveTransformer() {};

 private:
  bool isPCSensitive(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     int_function *func,
		     AssignList &sensitiveAssignment);
  Graph::Ptr forwardSlice(Assignment::Ptr ptr,
			  image_basicBlock *block,
			  image_func *func);
  bool determineSensitivity(Graph::Ptr slice,
			    bool &intSens,
			    bool &extSens);

  bool insnIsThunkCall(InstructionAPI::Instruction::Ptr insn,
		       Address addr,
		       Absloc &destination);
  void handleThunkCall(BlockList::iterator &b_iter,
		       ElementList::iterator &iter,
		       Absloc &destination);
  void recordIntSensitive(Address addr);
  void emulateInsn(BlockList::iterator &b_iter,
		   ElementList::iterator &iter,
		   InstructionAPI::Instruction::Ptr insn,
		   Address addr);

  bool exceptionSensitive(Address addr, const bblInstance *bbl);

  AssignmentConverter aConverter;

  AddressSpace *addrSpace;

  PriorityMap &priMap;  
};

};
};

#endif
