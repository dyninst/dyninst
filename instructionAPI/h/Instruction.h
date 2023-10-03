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

#if !defined(INSTRUCTION_H)
#define INSTRUCTION_H


#include <stddef.h>
#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <list>
#include "Expression.h"
#include "Operation_impl.h"
#include "Operand.h"
#include "InstructionCategories.h"
#include "ArchSpecificFormatters.h"

#include "util.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
      class InstructionDecoder;
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
        /// Component version information corresponding to \c libInstructionAPI.so.(major).(minor).(maintenance)
        /// Note that \c maintenance may be absent from the binary (in which case, it will be zero in the interface).

        INSTRUCTION_EXPORT static void version(int& major, int& minor, int& maintenance);
      union raw_insn_T
      {
#if defined(__powerpc__) || defined(__powerpc64__)
	unsigned int small_insn;
#else
	uintptr_t small_insn;
#endif
	unsigned char* large_insn;
      };
    public:
        friend class InstructionDecoder_x86;
        friend class InstructionDecoder_power;
        friend class InstructionDecoder_aarch64;
        friend class InstructionDecoder_amdgpu_gfx908;
        friend class InstructionDecoder_amdgpu_gfx90a;
        friend class InstructionDecoder_amdgpu_gfx940;

        struct CFT
        {
            Expression::Ptr target;
            bool isCall;
            bool isIndirect;
            bool isConditional;
            bool isFallthrough;
            CFT(Expression::Ptr t, bool call, bool indir, bool cond, bool ft) :
                    target(t), isCall(call), isIndirect(indir), isConditional(cond), isFallthrough(ft) {}
        };
      /// \param what Opcode of the instruction
      /// \param operandSource Contains the %Expressions to be transformed into %Operands
      /// \param size Contains the number of bytes occupied by the corresponding machine instruction
      /// \param raw Contains a pointer to the buffer from which this instruction object
      /// was decoded.
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

      INSTRUCTION_EXPORT Instruction(Operation what, size_t size, const unsigned char* raw,
                                     Dyninst::Architecture arch);
      INSTRUCTION_EXPORT Instruction();

      INSTRUCTION_EXPORT virtual ~Instruction();

      INSTRUCTION_EXPORT Instruction(const Instruction& o);
      INSTRUCTION_EXPORT const Instruction& operator=(const Instruction& rhs);


      /// \return The %Operation used by the %Instruction
      ///
      /// See Operation for details of the %Operation interface.
      INSTRUCTION_EXPORT Operation& getOperation();
      INSTRUCTION_EXPORT const Operation& getOperation() const;

      /// The vector \c operands has the instruction's operands appended to it
      /// in the same order that they were decoded.
      INSTRUCTION_EXPORT void getOperands(std::vector<Operand>& operands) const;

      /// Returns a vector of non-implicit operands in printed order
      INSTRUCTION_EXPORT std::vector<Operand> getDisplayOrderedOperands() const;

      /// The \c getOperand method returns the operand at position \c index, or
      /// an empty operand if \c index does not correspond to a valid operand in this
      /// instruction.
      INSTRUCTION_EXPORT Operand getOperand(int index) const;

      INSTRUCTION_EXPORT Operand getPredicateOperand() const;
      INSTRUCTION_EXPORT bool hasPredicateOperand() const;

      /// Returns a pointer to the buffer from which this instruction
      /// was decoded.
      INSTRUCTION_EXPORT unsigned char rawByte(unsigned int index) const;

      /// Returns the size of the corresponding machine instruction, in bytes.
      INSTRUCTION_EXPORT size_t size() const;

      /// Returns a pointer to the raw byte representation of the corresponding
      /// machine instruction.
      INSTRUCTION_EXPORT const void* ptr() const;

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

      INSTRUCTION_EXPORT void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

      /// \param regsRead Insert the set of registers read by the instruction into \c regsRead.
      ///
      /// If an operand is used to compute an effective address, the registers
      /// involved are read but not written, regardless of the effect on the operand.
      INSTRUCTION_EXPORT void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;

      /// \param candidate Subexpression to search for among the values read by this %Instruction object.
      ///
      /// Returns true if \c candidate is read by this %Instruction.
      INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) const;

      /// \param candidate Subexpression to search for among the values written by this %Instruction object.
      ///
      /// Returns true if \c candidate is written by this %Instruction.
      INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) const;


      /// \return Returns true if the instruction reads at least one memory address as data.
      ///
      /// If any operand containing a  %Dereference object is read, the instruction
      /// reads the memory at that address.
      /// Also, on platforms where a stack pop is guaranteed to read memory,
      /// \c readsMemory will return true for a pop operation.
      INSTRUCTION_EXPORT bool readsMemory() const;

      INSTRUCTION_EXPORT ArchSpecificFormatter& getFormatter() const;

      /// \return Returns true if the instruction writes at least one memory address.
      ///
      /// If any operand containing a  %Dereference object is written, the instruction
      /// writes the memory at that address.
      /// Also, on platforms where a stack push is guaranteed to write memory,
      /// \c writesMemory will return true for a push operation.
      INSTRUCTION_EXPORT bool writesMemory() const;

      /// \param memAccessors Addresses read by this instruction are inserted into \c memAccessors
      ///
      /// The addresses read are in the form of %Expressions, which may be evaluated once all of the
      /// registers that they use have had their values set.
      /// Note that this method returns ASTs representing address computations, and not address accesses.  For instance,
      /// an instruction accessing memory through a register dereference would return a %Expression tree containing
      /// just the register that determines the address being accessed, not a tree representing a dereference of that register.
      INSTRUCTION_EXPORT void getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const;

      /// \param memAccessors Addresses written by this instruction are inserted into \c memAccessors
      ///
      /// The addresses written are in the same form as those returned by \c getMemoryReadOperands above.
      INSTRUCTION_EXPORT void getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const;

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
      INSTRUCTION_EXPORT Expression::Ptr getControlFlowTarget() const;

      /// \return False if control flow will unconditionally go to the result of
      /// \c getControlFlowTarget after executing this instruction.
      INSTRUCTION_EXPORT bool allowsFallThrough() const;

      /// \return The instruction as a string of assembly language
      ///
      /// \c format is principally a helper function; %Instructions are meant to be written to
      /// output streams via \c operator<<.  \c format is included in the public interface for
      /// diagnostic purposes.
      INSTRUCTION_EXPORT std::string format(Address addr = 0) const;

      /// Returns true if this %Instruction object is valid.  Invalid instructions indicate that
      /// an %InstructionDecoder has reached the end of its assigned range, and that decoding should terminate.
      INSTRUCTION_EXPORT bool isValid() const;

      /// Returns true if this %Instruction object represents a legal instruction, as specified by the architecture
      /// used to decode this instruction.
      INSTRUCTION_EXPORT bool isLegalInsn() const;

      INSTRUCTION_EXPORT Architecture getArch() const;

      /// Currently, the valid categories are c_CallInsn, c_ReturnInsn, c_BranchInsn, c_CompareInsn,
      /// and c_NoCategory, as defined in %InstructionCategories.h.
      INSTRUCTION_EXPORT InsnCategory getCategory() const;

      typedef std::list<CFT>::const_iterator cftConstIter;
      INSTRUCTION_EXPORT cftConstIter cft_begin() const {
          return m_Successors.begin();
      }
        INSTRUCTION_EXPORT cftConstIter cft_end() const {
            return m_Successors.end();
        }
        INSTRUCTION_EXPORT bool operator<(const Instruction& rhs) const
        {
            if(m_size < rhs.m_size) return true;
            if(m_size <= sizeof(m_RawInsn.small_insn)) {
                return m_RawInsn.small_insn < rhs.m_RawInsn.small_insn;
            }
            return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) < 0;
        }
        INSTRUCTION_EXPORT bool operator==(const Instruction& rhs) const {
            if(m_size != rhs.m_size) return false;
            if(m_size <= sizeof(m_RawInsn.small_insn)) {
                return m_RawInsn.small_insn == rhs.m_RawInsn.small_insn;
            }
            return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) == 0;
        }
        INSTRUCTION_EXPORT void updateMnemonic(std::string new_mnemonic) {
                m_InsnOp.updateMnemonic(new_mnemonic);
            }


      typedef boost::shared_ptr<Instruction> Ptr;

    private:
      void updateSize(const unsigned int new_size) {m_size = new_size;}
      void decodeOperands() const;
      void addSuccessor(Expression::Ptr e, bool isCall, bool isIndirect, bool isConditional, bool isFallthrough, bool isImplicit = false) const;
      void appendOperand(Expression::Ptr e, bool isRead, bool isWritten, bool isImplicit = false, bool trueP = false, bool falseP = false) const;
      void copyRaw(size_t size, const unsigned char* raw);
      Expression::Ptr makeReturnExpression() const;
      mutable std::list<Operand> m_Operands;
      mutable Operation m_InsnOp;
      bool m_Valid;
      raw_insn_T m_RawInsn;
      unsigned int m_size;
      Architecture arch_decoded_from;
      mutable std::list<CFT> m_Successors;
      static int numInsnsAllocated;
      // formatter is a non-owning pointer to a singleton object
      ArchSpecificFormatter* formatter;
    };
  }
}



#endif //!defined(INSTRUCTION_H)
