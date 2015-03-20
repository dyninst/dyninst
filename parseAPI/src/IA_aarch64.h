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

#if !defined(IA_AARCH64__H)
#define IA_AARCH64__H
#include "common/h/DynAST.h"
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/SymEval.h"
#include "dataflowAPI/h/slicing.h"


namespace Dyninst {
namespace InsnAdapter {


class AARCH64_BLR_Visitor: public ASTVisitor 
{
 public:
  typedef enum {
    AARCH64_BLR_UNSET,
    AARCH64_BLR_UNKNOWN,
    AARCH64_BLR_RETURN,
    AARCH64_BLR_NOTRETURN } ReturnState;

 AARCH64_BLR_Visitor(Address ret)
   : ret_(ret), return_(AARCH64_BLR_UNSET) {};

     virtual AST::Ptr visit(AST *);
     virtual AST::Ptr visit(DataflowAPI::BottomAST *);
     virtual AST::Ptr visit(DataflowAPI::ConstantAST *);
     virtual AST::Ptr visit(DataflowAPI::VariableAST *);
     virtual AST::Ptr visit(DataflowAPI::RoseAST *);
     //virtual AST::Ptr visit(StackAST *);
     virtual ASTPtr visit(InputVariableAST *) {return AST::Ptr();};
     virtual ASTPtr visit(ReferenceAST *) {return AST::Ptr();};
     virtual ASTPtr visit(StpAST *) {return AST::Ptr();};
     virtual ASTPtr visit(YicesAST *) {return AST::Ptr();};
     virtual ASTPtr visit(SemanticsAST *) {return AST::Ptr();};

  
   virtual ~AARCH64_BLR_Visitor() {};

  ReturnState returnState() const { return return_; };

  private:
  Address ret_;
  ReturnState return_;

};
}
}
#endif
