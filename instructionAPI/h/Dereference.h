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

#if !defined(DEREFERENCE_H)
#define DEREFERENCE_H

#include "Operand.h"
#include "Expression.h"
#include "BinaryFunction.h"
#include "Immediate.h"
#include "Register.h"
#include "Visitor.h"
#include "ArchSpecificFormatters.h"

#include <string>
#include <vector>

namespace Dyninst
{
  namespace InstructionAPI
  {
    using std::vector;

    class INSTRUCTION_EXPORT Dereference : public Expression
    {

    public:

      Dereference(Expression::Ptr addr, Result_Type result_type) : Expression(result_type), addressToDereference(addr)
      {
      }
      virtual ~Dereference() 
      {
      }

      virtual void getChildren(vector<InstructionAST::Ptr>& children) const
      {
	children.push_back(addressToDereference);
	return;
      }
      virtual void getChildren(vector<Expression::Ptr>& children) const
      {
          children.push_back(addressToDereference);
          return;
      }

      virtual void getUses(set<InstructionAST::Ptr>& uses)
      {
	addressToDereference->getUses(uses);
	return;
      }

        virtual bool isUsed(InstructionAST::Ptr findMe) const
        {
            return addressToDereference->isUsed(findMe) || *findMe == *this;
        }

      virtual std::string format(formatStyle) const
      {
    std::string retVal;
#if defined(DEBUG_MEMORY_ACCESS_WIDTH)
        switch(Expression::userSetValue.type)
        {
            case u8:
                retVal += "u8 @ ";
                break;
            case s8:
                retVal += "s8 @ ";
                break;
            case u16:
                retVal += "u16 @ ";
                break;
            case s16:
                retVal += "s16 @ ";
                break;
            case u32:
                retVal += "u32 @ ";
                break;
            case s32:
                retVal += "s32 @ ";
                break;
            case u64:
                retVal += "u64 @ ";
                break;
            case s64:
                retVal += "s64 @ ";
                break;
            case sp_float:
                retVal += "float @ ";
                break;
            case dp_float:
                retVal += "double @ ";
                break;
            case dbl128:
                retVal += "packed double @ ";
                break;
            default:
                retVal += "UNKNOWN SIZE @ ";
                break;
        }
#endif
    retVal += "[" + addressToDereference->format() + "]";
        return retVal;
      }

        virtual std::string format(Architecture arch, formatStyle) const
        {
#if defined(DEBUG_MEMORY_ACCESS_WIDTH)
            switch(Expression::userSetValue.type)
            {
                case u8:
                    retVal << "u8 @ ";
                    break;
                case s8:
                    retVal << "s8 @ ";
                    break;
                case u16:
                    retVal << "u16 @ ";
                    break;
                case s16:
                    retVal << "s16 @ ";
                    break;
                case u32:
                    retVal << "u32 @ ";
                    break;
                case s32:
                    retVal << "s32 @ ";
                    break;
                case u64:
                    retVal << "u64 @ ";
                    break;
                case s64:
                    retVal << "s64 @ ";
                    break;
                case sp_float:
                    retVal << "float @ ";
                    break;
                case dp_float:
                    retVal << "double @ ";
                    break;
                case dbl128:
                    retVal << "packed double @ ";
                    break;
                default:
                    retVal << "UNKNOWN SIZE @ ";
                    break;
            }
#endif

#if 0 /* Which parts of the operand are available? */
            if(list.base)
                std::cout << "\tBase:         " << list.base << std::endl;
            if(list.offset)
                std::cout << "\tOffset:       " << list.offset << std::endl;
            if(list.segment)
                std::cout << "\tSegment:      " << list.segment << std::endl;
            if(list.scale)
                std::cout << "\tScale:        " << list.scale << std::endl;
            if(list.displacement)
                std::cout << "\tDisplacement: " << list.displacement << std::endl;
#endif

            return ArchSpecificFormatter::getFormatter(arch).formatDeref(addressToDereference->format(arch));
        }

      virtual bool bind(Expression* expr, const Result& value)
      {
          if(Expression::bind(expr, value))
          {
              return true;
          }
          return addressToDereference->bind(expr, value);
      }
      virtual void apply(Visitor* v)
      {
          addressToDereference->apply(v);
          v->visit(this);
      }
    

    protected:
      virtual bool isSameType(const InstructionAST& rhs) const
      {
	return dynamic_cast<const Dereference*>(&rhs) != NULL;
      }
      virtual bool isStrictEqual(const InstructionAST& rhs) const
      {
	const Dereference& other(dynamic_cast<const Dereference&>(rhs));
	return *(other.addressToDereference) == *addressToDereference;
      }
  
  
    private:
      Expression::Ptr addressToDereference;
  
    };
  }
}



#endif // !defined(DEREFERENCE_H)
