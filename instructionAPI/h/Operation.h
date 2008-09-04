/*
 * Copyright (c) 2007-2008 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(DYN_OPERATION_H)
#define DYN_OPERATION_H

#include "Register.h"
#include "Expression.h"
#include "entryIDs-IA32.h"
#include <set>
#include <boost/dynamic_bitset.hpp>


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
  namespace InstructionAPI
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
    class ia32_entry;
    class ia32_prefixes;
    
    class Operation
    {
    public:
      typedef boost::dynamic_bitset<> bitSet;
      typedef std::set<RegisterAST::Ptr> registerSet;
      typedef std::set<Expression::Ptr> VCSet;
  
    public:
      Operation(Dyninst::InstructionAPI::ia32_entry* e, Dyninst::InstructionAPI::ia32_prefixes* p = NULL);
      Operation(const Operation& o);
      Operation();
      
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
      /// Returns the number of operands accepted by this operation.
      int numOperands() const;
      /// Returns the entry ID corresponding to this operation.  Entry IDs are enumerated values that correspond
      /// to assembly mnemonics.
      entryID getID() const
      {
	return operationID;
      }
      

    private:
      void SetUpNonOperandData();
      
      std::string mnemonic;
      // should be dynamic_bitset in future
      bitSet readOperands;
      bitSet writtenOperands;
      registerSet otherRead;
      registerSet otherWritten;
      VCSet otherEffAddrsRead;
      VCSet otherEffAddrsWritten;
      entryID operationID;
      static map<entryID, std::set<RegisterAST::Ptr> > nonOperandRegisterReads;
      static map<entryID, std::set<RegisterAST::Ptr> > nonOperandRegisterWrites;
      static map<entryID, std::set<Expression::Ptr> > nonOperandMemoryReads;
      static map<entryID, std::set<Expression::Ptr> > nonOperandMemoryWrites;
      
    };
  };
};


#endif //!defined(DYN_OPERATION_H)
