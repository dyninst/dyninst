#if !defined(IA_POWER__H)
#define IA_POWER__H
#include "dynutil/h/AST.h"
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/SymEval.h"
#include "dataflowAPI/h/slicing.h"


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
   : ret_(ret), return_(PPC_BLR_UNSET) {};

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

  
   virtual ~PPC_BLR_Visitor() {};

  ReturnState returnState() const { return return_; };

  private:
  Address ret_;
  ReturnState return_;

};
}
}
#endif
