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
#if !defined(IA_POWER__H)
#define IA_POWER__H
#include <string>
#include "common/h/DynAST.h"
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/SymEval.h"
#include "dataflowAPI/h/slicing.h"
#include "IA_IAPI.h"


namespace Dyninst {
namespace InsnAdapter {


class PPC_BLR_Visitor: public ASTVisitor 
{
 public:
  typedef enum {
    PPC_BLR_UNSET,
    PPC_BLR_UNKNOWN,
    PPC_BLR_RETURN,
    PPC_BLR_NOTRETURN } ReturnState;

 PPC_BLR_Visitor(Address ret)
   : ret_(ret), return_(PPC_BLR_UNSET) {}

     virtual AST::Ptr visit(AST *);
     virtual AST::Ptr visit(DataflowAPI::BottomAST *);
     virtual AST::Ptr visit(DataflowAPI::ConstantAST *);
     virtual AST::Ptr visit(DataflowAPI::VariableAST *);
     virtual AST::Ptr visit(DataflowAPI::RoseAST *);
     //virtual AST::Ptr visit(StackAST *);
     virtual ASTPtr visit(InputVariableAST *) {return AST::Ptr();}
     virtual ASTPtr visit(ReferenceAST *) {return AST::Ptr();}
     virtual ASTPtr visit(StpAST *) {return AST::Ptr();}
     virtual ASTPtr visit(YicesAST *) {return AST::Ptr();}
     virtual ASTPtr visit(SemanticsAST *) {return AST::Ptr();}

  
   virtual ~PPC_BLR_Visitor() {}

  ReturnState returnState() const { return return_; }

  private:
     using ASTVisitor::visit;
  Address ret_;
  ReturnState return_;

};

class IA_power : public IA_IAPI {
    public:
        IA_power(Dyninst::InstructionAPI::InstructionDecoder dec_,
               Address start_, 
               Dyninst::ParseAPI::CodeObject* o,
               Dyninst::ParseAPI::CodeRegion* r,
               Dyninst::InstructionSource *isrc,
	       Dyninst::ParseAPI::Block * curBlk_);
	IA_power(const IA_power &);
	virtual IA_power* clone() const;
    virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction) const;
	virtual bool isNop() const;
	virtual bool isThunk() const;
	virtual bool isTailCall(const ParseAPI::Function* context, ParseAPI::EdgeTypeEnum type, unsigned int, const set<Address>& knownTargets) const;
	virtual bool savesFP() const;
	virtual bool isStackFramePreamble() const;
	virtual bool cleansStack() const;
	virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const;
	virtual bool isReturnAddrSave(Address& retAddr) const;
	virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const;
	virtual bool isFakeCall() const;
	virtual bool isIATcall(std::string &) const;
	virtual bool isLinkerStub() const;
	virtual bool isNopJump() const;
    private:
    using IA_IAPI::isFrameSetupInsn;
};

}
}
#endif
