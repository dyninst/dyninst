
#if !defined(INSTRUCTION_H)
#define INSTRUCTION_H


#include <vector>
#include <set>
#include "../h/Expression.h"
#include "../h/Operation.h"

#include "../h/Operand.h"


namespace Dyninst
{
  namespace Instruction
  {
    /// The %Instruction class is a generic instruction representation that contains operands,
    /// read/write semantic information about those operands, and information about
    /// what other registers and memory locations are affected by the operation the instruction performs.
    ///
    /// The purpose of an %Instruction object is to join an %Operation with a sequence of %Operands, and provide
    /// an interface for some common summary analyses (namely, the read/write sets, memory access information, 
    /// and control flow information).
    ///
    /// The %Operation contains knowledge about its mnemonic and sufficient semantic
    /// details to answer the following questions:
    ///   - What %Operands are read/written?
    ///   - What registers are implicitly read/written?
    ///   - What memory locations are implicitly read/written?
    ///   - What are the possible control flow successors of this instruction?
    /// 
    /// Each %Operand is an AST built from %RegisterAST and %Immediate leaves.  For each %Operand, you may determine:
    ///   - Registers read
    ///   - Registers written
    ///   - Whether memory is read or written
    ///   - Which memory addresses are read or written, given the state of all relevant registers
    ///
    /// Instructions should be constructed from an \c unsigned \c char* pointing to machine language, using the
    /// %InstructionDecoder class.  See InstructionDecoder for more details.
    ///
    class Instruction
    {
    public:
      /// \param what Opcode of the instruction
      /// \param operandSource Contains the %Expressions to be transformed into %Operands
      /// \param size Contains the number of bytes occupied by the corresponding machine instruction
      ///
      /// Construct an %Instruction from an %Operation and a collection of %Expressions.  This
      /// method is not intended to be used except by the %InstructionDecoder class, which serves as a
      /// factory class for producing %Instruction objects.  While an %Instruction object may be built
      /// "by hand" if desired, using the decoding interface ensures that the operation and operands
      /// are a sensible combination, and that the size reported is based on the actual size of a legal
      /// encoding of the machine instruction represented.
      /// In the course of constructing an %Instruction, the %Expressions in \c operandSource
      /// will be transformed to %Operand objects.  This transformation will map the semantic information about
      /// which operands are read and written
      /// in the %Operation object \c what to the value computations in \c operandSource.

      Instruction(const Operation& what, const std::vector<Expression::Ptr>& operandSource, unsigned char size);
  
      virtual ~Instruction();

      /// \return The %Operation used by the %Instruction
      ///
      /// See Operation for details of the %Operation interface.
      const Operation& getOperation() const;

      /// The vector \c operands has the instruction's operands appended to it
      /// in the same order that they were decoded.
      void getOperands(std::vector<Operand>& operands) const;

      Operand getOperand(int index) const;
  
      /// Returns the size of the corresponding machine instruction, in bytes.
      unsigned char size() const;
  
      /// \param regsWritten Insert the set of registers written by the instruction into \c regsWritten.
      ///
      /// The list of registers returned in \c regsWritten includes registers that are explicitly written as destination
      /// operands (like the destination of a move).  It also includes registers 
      /// that are implicitly written (like the
      /// stack pointer in a push or pop instruction).  It does not include
      /// any registers used only in computing the effective address of a write.
      /// \c pop \c *eax, for example, writes to \c esp, reads \c esp, and reads \c eax,
      /// but despite the fact that \c *eax is the destination operand, \c eax is not
      /// itself written.
      /// 
      /// For both the write set and the read set (below), it is possible to determine whether a register
      /// is accessed implicitly or explicitly by examining the %Operands.  An explicitly accessed register appears
      /// as an operand that is written or read; also, any registers used in any address calculations are explicitly
      /// read.  Any element of the write set or read set that is not explicitly written or read is implicitly
      /// written or read.  

      void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

      /// \param regsRead Insert the set of registers read by the instruction into \c regsRead.
      ///
      /// If an operand is used to compute an effective address, the registers
      /// involved are read but not written, regardless of the effect on the operand.
      void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;

      /// \param candidate Subexpression to search for among the values read by this %Instruction object.
      ///
      /// Returns true if \c candidate is read by this %Instruction.
      bool isRead(Expression::Ptr candidate) const;

      /// \param candidate Subexpression to search for among the values written by this %Instruction object.
      ///
      /// Returns true if \c candidate is written by this %Instruction.
      bool isWritten(Expression::Ptr candidate) const;
      

      /// \return Returns true if the instruction reads at least one memory address as data.
      ///
      /// If any operand containing a  %Dereference object is read, the instruction
      /// reads the memory at that address.
      /// Also, on platforms where a stack pop is guaranteed to read memory,
      /// \c readsMemory will return true for a pop operation.
      bool readsMemory() const;

      /// \return Returns true if the instruction writes at least one memory address.
      ///
      /// If any operand containing a  %Dereference object is written, the instruction
      /// writes the memory at that address.
      /// Also, on platforms where a stack push is guaranteed to write memory,
      /// \c writesMemory will return true for a push operation.
      bool writesMemory() const;

      /// \param memAccessors Addresses read by this instruction are inserted into \c memAccessors
      ///
      /// The addresses read are in the form of %Expressions, which may be evaluated once all of the
      /// registers that they use have had their values set.  
      /// Note that this method returns ASTs representing address computations, and not address accesses.  For instance,
      /// an instruction accessing memory through a register dereference would return a %Expression tree containing
      /// just the register that determines the address being accessed, not a tree representing a dereference of that register.
      void getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const;

      /// \param memAccessors Addresses written by this instruction are inserted into \c memAccessors
      ///
      /// The addresses written are in the same form as those returned by \c getMemoryReadOperands above.
      void getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const;
  
      /// \return An expression evaluating to the non-fallthrough control flow targets, if any, of this instruction.
      ///
      /// When called on an explicitly control-flow altering instruction, returns the 
      /// non-fallthrough control flow destination.  When called on any other instruction,
      /// returns \c NULL.
      ///
      /// For direct absolute branch instructions, \c getControlFlowTarget will return an immediate value.
      /// For direct relative branch instructions, \c getControlFlowTarget will return the expression
      /// \c PC + offset.
      /// In the case of indirect branches and calls, it returns a dereference of a register (or possibly
      /// a dereference of a more complicated expression).  In this case,
      /// data flow analysis will often allow the determination of the possible targets of the
      /// instruction.  We do not do analysis beyond the single-instruction level
      /// in the %Instruction API; if other code performs this type of analysis,
      /// it may update the information in the %Dereference object using the setValue method in the %Expression interface.
      /// More details about this may be found in Expression and Dereference.
      Expression::Ptr getControlFlowTarget() const;

      /// \return False if control flow will unconditionally go to the result of
      /// \c getControlFlowTarget after executing this instruction.
      bool allowsFallThrough() const;

      /// \return The instruction as a string of assembly language
      ///
      /// \c format is principally a helper function; %Instructions are meant to be written to
      /// output streams via \c operator<<.  \c format is included in the public interface for
      /// diagnostic purposes.
      std::string format() const;
  
    private:
      std::vector<Operand> m_Operands;
      Operation m_InsnOp;
      unsigned char m_Size;
    };
  };
};



#endif //!defined(INSTRUCTION_H)
