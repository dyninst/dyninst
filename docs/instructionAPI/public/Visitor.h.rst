Visitor.h
=========

.. cpp:namespace:: Dyninst::instructionAPI

Visitor Paradigm
----------------

An alternative to the bind/eval mechanism is to use a *visitor* 
over an expression tree. The visitor concept applies a user-specified
visitor class to all nodes in an expression tree (in a post-order
traversal). The visitor paradigm can be used as a more efficient
replacement for bind/eval, to identify whether an expression has a
desired pattern, or to locate children of an expression tree.

A visitor is a user-defined class that inherits from the ``Visitor``
class defined in ``Visitor.h``. That class is repeated here for
reference:

class Visitor public: Visitor() virtual  Visitor() virtual void
visit(BinaryFunction\* b) = 0; virtual void visit(Immediate\* i) = 0;
virtual void visit(RegisterAST\* r) = 0; virtual void
visit(Dereference\* d) = 0;;

A user provides implementations of the four ``visit`` methods. When
applied to an ``Expression`` (via the ``Expression::apply`` method) the
InstructionAPI will perform a post-order traversal of the tree, calling
the appropriate ``visit`` method at each node.

As a simple example, the following code prints out the name of each
register used in an ``Expression``:


.. code-block:: cpp

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

Visitors may also set and use internal state. For example, the following
visitor (presented without surrounding use code) matches x86 and x86-64
instructions that add 0 to a register (effectively a noop).

.. code-block:: cpp

   class nopVisitor : public Visitor
   {
     public:
      nopVisitor() : foundReg(false), foundImm(false), foundBin(false), isNop(true) {}
      virtual ~nopVisitor() {}
      
      bool foundReg;
      bool foundImm;
      bool foundBin;
      bool isNop;
      
      virtual void visit(BinaryFunction*)
      {
         if (foundBin) isNop = false;
         if (!foundImm) isNop = false;
         if (!foundReg) isNop = false;
         foundBin = true;
      }
      virtual void visit(Immediate *imm)
      {
         if (imm != 0) isNop = false;
         foundImm = true;
      }
      virtual void visit(RegisterAST *)
      {
         foundReg = true;
      }
      virtual void visit(Dereference *)
      {
         isNop = false;
      }
   };