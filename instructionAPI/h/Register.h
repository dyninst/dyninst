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

#if !defined(REGISTER_H)
#define REGISTER_H

#include "Expression.h"
#include <vector>
#include <map>
#include <boost/assign/list_of.hpp>
#include <sstream>

#include "RegisterIDs-x86.h"


namespace Dyninst
{
  namespace InstructionAPI
  {
    /// A %RegisterAST object represents a register contained in an operand.
    /// As a %RegisterAST is a %Expression, it may contain the physical register's contents if
    /// they are known.
    ///
    class RegisterAST : public Expression
    {
    public:
      /// \brief A type definition for a reference-counted pointer to a %RegisterAST.
      typedef boost::shared_ptr<RegisterAST> Ptr;
      
      /// Construct a register, assigning it the ID \c id.
      RegisterAST(int id);
  
      virtual ~RegisterAST();
      

      /// By definition, a %RegisterAST object has no children.
      /// \param children Since a %RegisterAST has no children, the \c children parameter is unchanged by this method.
      virtual void getChildren(vector<InstructionAST::Ptr>& children) const;

      /// By definition, the use set of a %RegisterAST object is itself.
      /// \param uses This %RegisterAST will be inserted into \c uses.
      virtual void getUses(set<InstructionAST::Ptr>& uses) const;

      /// \c isUsed returns true if \c findMe is a %RegisterAST that represents
      /// the same register as this %RegisterAST, and false otherwise.
      virtual bool isUsed(InstructionAST::Ptr findMe) const;

      /// The \c format method on a %RegisterAST object returns the name associated with its ID.
      virtual std::string format() const;

      /// Utility function to get a Register object that represents the program counter.
      ///
      /// \c makePC is provided to support platform-independent control flow analysis.
      static RegisterAST makePC();

      /// We define a partial ordering on registers by their register number so that they may be placed into sets
      /// or other sorted containers.
      bool operator<(const RegisterAST& rhs) const;

      /// The \c getID function returns the ID number of a register.
      unsigned int getID() const;
      

    protected:
      virtual bool isSameType(const InstructionAST& rhs) const;
      virtual bool isStrictEqual(const InstructionAST& rhs) const;
    private:
      unsigned int registerID;
    };
    
  };
};


  

#endif // !defined(REGISTER_H)
