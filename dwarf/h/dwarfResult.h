#include "dyn_regs.h"
#include "dyntypes.h"
#include <vector>
#include <stack>
#include "libdwarf.h"
#include "util.h"

namespace Dyninst {

class VariableLocation;
class ProcessReader;

namespace Dwarf {

class DYNDWARF_EXPORT DwarfResult {
   // An interface for building representations of Dwarf expressions.
   // In concrete mode, we have access to process state and
   // can calculate a value. In symbolic mode we lack this information
   // and instead produce a representation. 

  public:
   
   typedef enum {
      Add,
      Sub,
      Mul,
      Div,
      Mod,
      Deref,
      Pick,
      Drop,
      And,
      Or,
      Not,
      Xor,
      Abs,
      GE,
      LE,
      GT,
      LT,
      Eq,
      Neq,
      Shl,
      Shr,
      ShrArith
   } Operator;

  DwarfResult(Architecture a) : arch(a), error(false) {}
   virtual ~DwarfResult() {};
   
   virtual void pushReg(Dyninst::MachRegister reg) = 0;
   virtual void readReg(Dyninst::MachRegister reg) = 0;
   virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant) = 0;
   virtual void pushSignedVal(Dyninst::MachRegisterVal constant) = 0;
   virtual void pushOp(Operator op) = 0;
   virtual void pushOp(Operator op, unsigned ref) = 0;

   // The frame base is the logical top of the stack as reported
   // in the function's debug info
   virtual void pushFrameBase() = 0;
   
   // And the CFA is the top of the stack as reported in call frame
   // information. 
   virtual void pushCFA() = 0;

   bool err() const { return error; }

   // The conditional branch needs an immediate eval mechanism
   virtual bool eval(MachRegisterVal &val) = 0;

  protected:

   // Illegal
   Architecture arch;
   bool error;

};

class DYNDWARF_EXPORT SymbolicDwarfResult : public DwarfResult {

  public:
  SymbolicDwarfResult(VariableLocation &v, Architecture a) :
   DwarfResult(a), var(v) {};
   virtual ~SymbolicDwarfResult() {};
   
   virtual void pushReg(Dyninst::MachRegister reg);
   virtual void readReg(Dyninst::MachRegister reg);
   virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant);
   virtual void pushSignedVal(Dyninst::MachRegisterVal constant);
   virtual void pushOp(Operator op);
   virtual void pushOp(Operator op, unsigned ref);

   // DWARF logical "frame base", which may be the result of an expression
   // in itself. TODO: figure out what info we need to carry around so we
   // can compute it...
   virtual void pushFrameBase();
   virtual void pushCFA();

   VariableLocation &val();

   virtual bool eval(MachRegisterVal &) { return false; }

  private:
   std::stack<MachRegisterVal> operands;

   VariableLocation &var;
};

class DYNDWARF_EXPORT ConcreteDwarfResult : public DwarfResult {

  public:
  ConcreteDwarfResult(ProcessReader *r, Architecture a, 
                     Address p, Dwarf_Debug d) :
   DwarfResult(a), reader(r), 
      pc(p), dbg(d) {};
   virtual ~ConcreteDwarfResult() {};

   virtual void pushReg(Dyninst::MachRegister reg);
   virtual void readReg(Dyninst::MachRegister reg);
   virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant);
   virtual void pushSignedVal(Dyninst::MachRegisterVal constant);
   virtual void pushOp(Operator op);
   virtual void pushOp(Operator op, unsigned ref);

   // DWARF logical "frame base", which may be the result of an expression
   // in itself. TODO: figure out what info we need to carry around so we
   // can compute it...
   virtual void pushFrameBase();
   virtual void pushCFA();

   MachRegisterVal val();

   bool eval(MachRegisterVal &v);

  private:
   ProcessReader *reader;

   // For getting access to other expressions
   Address pc;
   Dwarf_Debug dbg;
     
   // Dwarf lets you access within the "stack", so we model 
   // it as a vector.
   std::vector<Dyninst::MachRegisterVal> operands;

   MachRegisterVal peek(int index);
   void pop(int num);
   void popRange(int start, int end);
   void push(MachRegisterVal v);

};

}

}
