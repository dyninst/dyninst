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

#if !defined(DYN_OPERATION_H)
#define DYN_OPERATION_H

#include "Register.h"
#include "Expression.h"
#include "entryIDs.h"
#include "Result.h"
#include <stddef.h>
#include <string>
#include <set>
#include <mutex>

#include "util.h"
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/flyweight.hpp>

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

namespace NS_x86 {
struct ia32_entry;
class ia32_prefixes;
}
class ia32_locations;

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
    
    class Operation : public boost::lockable_adapter<boost::recursive_mutex>
    {
    public:
      typedef std::set<RegisterAST::Ptr> registerSet;
      typedef std::set<Expression::Ptr> VCSet;
      friend class InstructionDecoder_power; // for editing mnemonics after creation
      friend class InstructionDecoder_aarch64;
      friend class InstructionDecoder_amdgpu_gfx908;
      friend class InstructionDecoder_amdgpu_gfx90a;
      friend class InstructionDecoder_amdgpu_gfx940;
      friend class Instruction; // to make use of the update size function
      
    public:
      INSTRUCTION_EXPORT Operation(NS_x86::ia32_entry* e, NS_x86::ia32_prefixes* p = NULL, ia32_locations* l = NULL,
                                  Architecture arch = Arch_none);
      INSTRUCTION_EXPORT Operation(const Operation& o);
      INSTRUCTION_EXPORT Operation();
      INSTRUCTION_EXPORT Operation(entryID id, std::string m, Architecture arch);

      INSTRUCTION_EXPORT const Operation& operator=(const Operation& o);
      
      /// Returns the set of registers implicitly read (i.e. those not included in the operands, but read anyway)
      INSTRUCTION_EXPORT const registerSet& implicitReads() ;
      /// Returns the set of registers implicitly written (i.e. those not included in the operands, but written anyway)
      INSTRUCTION_EXPORT const registerSet& implicitWrites() ;
      /// Returns the mnemonic for the operation.  Like \c instruction::format, this is exposed for debugging
      /// and will be replaced with stream operators in the public interface.
      INSTRUCTION_EXPORT std::string format() const;
      /// Returns the entry ID corresponding to this operation.  Entry IDs are enumerated values that correspond
      /// to assembly mnemonics.
      INSTRUCTION_EXPORT entryID getID() const;
      /// Returns the prefix entry ID corresponding to this operation, if any.
      /// Prefix IDs are enumerated values that correspond to assembly prefix mnemonics.
      INSTRUCTION_EXPORT prefixEntryID getPrefixID() const;

      /// Returns true if the expression represented by \c candidate is read implicitly.
      INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) ;
      /// Returns true if the expression represented by \c candidate is written implicitly.
      INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) ;
      /// Returns the set of memory locations implicitly read.
      INSTRUCTION_EXPORT const VCSet& getImplicitMemReads() ;
      /// Returns the set of memory locations implicitly written.
      INSTRUCTION_EXPORT const VCSet& getImplicitMemWrites() ;

      void updateMnemonic(std::string new_mnemonic){
        mnemonic = new_mnemonic;
      }


      INSTRUCTION_EXPORT const VCSet& getImplicitMemWrites() const;
      bool isVectorInsn;

    private:
        std::once_flag data_initialized;
      void SetUpNonOperandData();
      
      mutable registerSet otherRead;
      mutable registerSet otherWritten;
      mutable VCSet otherEffAddrsRead;
      mutable VCSet otherEffAddrsWritten;

    protected:
        mutable entryID operationID;
      Architecture archDecodedFrom;
      prefixEntryID prefixID;
      Result_Type addrWidth;
      int segPrefix;
      mutable std::string mnemonic;

      
    };
  }
}


#endif //!defined(DYN_OPERATION_H)
