
#if !defined(DYN_OPERATION_H)
#define DYN_OPERATION_H

#include "Register.h"
#include "Expression.h"
#include "../src/arch-x86.h"
#include <set>
#include <boost/dynamic_bitset.hpp>

class ia32_entry;

// OpCode = operation + encoding
// contents:
// hex value
// information on how to decode operands
// operation encoded

// Operation = what an instruction does
// contents:
// read/write semantics on explicit operands
// register implicit read/write lists, including flags
// string/enum representation of the operation

// Use cases:
// OpCode + raw instruction -> Operation + ExpressionPtrs
// Operation + ExpressionPtrs -> Instruction + Operands

namespace Dyninst
{
  namespace Instruction
  {
    /// An %Operation object represents a family of opcodes (operation encodings)
    /// that perform the same task (e.g. the \c MOV family).  It includes
    /// information about the number of operands, their read/write semantics,
    /// the implicit register reads and writes, and the control flow behavior
    /// of a particular assembly language operation.  It additionally provides
    /// access to the assembly mnemonic, which allows any semantic details that
    /// are not encoded in the %Instruction representation to be added by higher
    /// layers of analysis.
    ///
    /// As an example, the \c CMP operation on IA32/AMD64 processors has the following
    /// properties:
    ///   - %Operand 1 is read, but not written
    ///   - %Operand 2 is read, but not written
    ///   - The following flags are written:
    ///     - Overflow
    ///     - Sign
    ///     - Zero
    ///     - Parity
    ///     - Carry
    ///     - Auxiliary
    ///   - No other registers are read, and no implicit memory operations are performed
    ///
    /// %Operations are constructed by the %InstructionDecoder as part of the process
    /// of constructing an %Instruction.
    class Operation
    {
    public:
      typedef boost::dynamic_bitset<> bitSet;
      typedef std::set<RegisterAST> registerSet;
      typedef std::set<Expression::Ptr> VCSet;
  
    public:
      Operation(ia32_entry* e);
      Operation(const Operation& o);
      Operation operator=(const Operation& o);
      
      /// \brief Return which operands are read
      /// \return vector such that:
      ///   - the size of the vector is the number of operands for this operation
      ///   - each element is true if and only if the corresponding operand is read by the operation
      const bitSet& read() const;
      /// \brief Return which operands are written
      /// \return vector such that:
      ///   - the size of the vector is the number of operands for this operation
      ///   - each element is true if and only if the corresponding operand is written by the operation
      const bitSet& written() const;
      /// Returns the set of registers implicitly read (i.e. those not included in the operands, but read anyway)
      const registerSet& implicitReads() const;
      /// Returns the set of registers implicitly written (i.e. those not included in the operands, but written anyway)
      const registerSet& implicitWrites() const;
      /// Returns the mnemonic for the operation.  Like \c instruction::format, this is exposed for debugging
      /// and will be replaced with stream operators in the public interface.
      std::string format() const;
      int numOperands() const;
      entryID getID() const
      {
	return operationID;
      }
      

    private:
      std::string mnemonic;
      // should be dynamic_bitset in future
      bitSet readOperands;
      bitSet writtenOperands;
      registerSet otherRead;
      registerSet otherWritten;
      VCSet otherEffAddrsRead;
      VCSet otherEffAddrsWritten;
      entryID operationID;
      
    };
  };
};


#endif //!defined(DYN_OPERATION_H)
