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

namespace NS_x86 {
struct ia32_entry;
class ia32_prefixes;
}
class ia32_locations;

namespace Dyninst
{
  namespace InstructionAPI
  {
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
      
      INSTRUCTION_EXPORT const registerSet& implicitReads() ;

      INSTRUCTION_EXPORT const registerSet& implicitWrites() ;

      INSTRUCTION_EXPORT std::string format() const;

      INSTRUCTION_EXPORT entryID getID() const;

      INSTRUCTION_EXPORT prefixEntryID getPrefixID() const;

      INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) ;

      INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) ;

      INSTRUCTION_EXPORT const VCSet& getImplicitMemReads() ;

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
