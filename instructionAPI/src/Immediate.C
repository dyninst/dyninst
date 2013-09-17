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

#include "Immediate.h"
#include "../../common/src/singleton_object_pool.h"
#include "Visitor.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    Immediate::Ptr Immediate::makeImmediate(const Result& val)
    {
      //static std::map<Result, Immediate::Ptr> builtImmediates;
      //static int cache_hits = 0;
      //std::map<Result, Immediate::Ptr>::const_iterator foundIt = builtImmediates.find(val);
      
      //if(foundIt == builtImmediates.end())
      //{
      Immediate::Ptr ret = make_shared(singleton_object_pool<Immediate>::construct(val));
	//builtImmediates[val] = ret;
      return ret;
	//}
	//return foundIt->second;
    }
    
    
    Immediate::Immediate(const Result& val) : Expression(val.type)
    {
      setValue(val);
    }

    Immediate::~Immediate()
    {
    }
    
    void Immediate::getChildren(vector<InstructionAST::Ptr>&) const
    {
      return;
    }
    void Immediate::getChildren(vector<Expression::Ptr>&) const
    {
        return;
    }

    bool Immediate::isUsed(InstructionAST::Ptr findMe) const
    {
      return *findMe == *this;
    }
    void Immediate::getUses(std::set<InstructionAST::Ptr>&)
    {
      return;
    }
    
    std::string Immediate::format(formatStyle) const
    {
      return eval().format();
    }
    bool Immediate::isStrictEqual(const InstructionAST& rhs) const
    {
      
        return (rhs.eval() == eval());
    }
    void Immediate::apply(Visitor* v)
    {
        v->visit(this);
    }
  
  };
};

