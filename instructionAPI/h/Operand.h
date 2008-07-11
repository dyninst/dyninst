
#if !defined(OPERAND_H)
#define OPERAND_H

#include "Expression.h"
#include "Register.h"
#include <set>
#include <string>

namespace Dyninst
{
  namespace InstructionAPI
  {

    /// An %Operand object contains an AST built from %RegisterAST and %Immediate leaves,
    /// and information about whether the %Operand
    /// is read, written, or both. This allows us to determine which of the registers
    /// that appear in the %Operand are read and which are written, as well as whether
    /// any memory accesses are reads, writes, or both.
    /// An %Operand, given full knowledge of the values of the leaves of the AST, and knowledge of
    /// the logic associated with the tree's internal nodes, can determine the
    /// result of any computations that are encoded in it.  It will rarely be the case
    /// that an %Instruction is built with its %Operands' state fully specified.  This mechanism is
    /// instead intended to allow a user to fill in knowledge about the state of the processor
    /// at the time the %Instruction is executed.
    
    class Operand
    {
    public:
      /// \brief Create an operand from a %Expression and flags describing whether the %ValueComputation
      /// is read, written or both.
      /// \param val Reference-counted pointer to the %Expression that will be contained in the %Operand being constructed
      /// \param read True if this operand is read
      /// \param written True if this operand is written
      Operand(Expression::Ptr val, bool read, bool written) : op_value(val), m_isRead(read), m_isWritten(written) 
      {
      }

      /// \brief Get the registers read by this operand
      /// \param[out] regsRead Has the registers read inserted into it
      void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;
      /// \brief Get the registers written by this operand
      /// \param[out] regsRead Has the registers written  inserted into it
      void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

      bool isRead(Expression::Ptr candidate) const;
      bool isWritten(Expression::Ptr candidate) const;
      
      /// Returns true if this operand reads memory
      bool readsMemory() const;
      /// Returns true if this operand writes memory
      bool writesMemory() const;
      /// \brief Inserts the effective addresses read by this operand into memAccessors
      /// \param[out] memAccessors If this is a memory read operand, insert the \c %Expression::Ptr representing
      /// the address being read into \c memAccessors.
      void addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const;
      /// \brief Inserts the effective addresses written by this operand into memAccessors
      /// \param[out] memAccessors If this is a memory write operand, insert the \c %Expression::Ptr representing
      /// the address being written into \c memAccessors.
      void addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const;
      /// \brief Return a printable string representation of the operand
      /// \return The operand in a disassembly format
      std::string format() const;

      Expression::Ptr getValue() const;
      
    private:
      Expression::Ptr op_value;
      bool m_isRead;
      bool m_isWritten;
    };
  };
};




#endif //!defined(OPERAND_H)
