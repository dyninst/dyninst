#include "Instruction.h"
#include "Operand.h"
#include "Expression.h"
#include "Register.h"
#include "Visitor.h"
#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;

class PrintVisitor : public Visitor {
  public:
    PrintVisitor() {};
    ~PrintVisitor() {};
    virtual void visit(BinaryFunction* b) {};
    virtual void visit(Immediate* i) {};
    virtual void visit(RegisterAST* r) {
      cout << "\tVisiting register " << r->getID().name() << endl;
    }
    virtual void visit(Dereference* d) {};
};

void printRegisters(Instruction::Ptr insn) {
   PrintVisitor pv;
   std::vector<Operand> operands;
   insn->getOperands(operands);
   // c++11x allows auto to determine the type of a variable;
   // if not using c++11x, use 'std::vector<Operand>::iterator' instead.
   // For gcc, use the -std=c++0x argument.
   for (auto iter = operands.begin(); iter != operands.end(); ++iter) {
      cout << "Registers used for operand" << endl;
      (*iter).getValue()->apply(&pv);
   }
}


