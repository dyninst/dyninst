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

#include "Expression.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    Expression::Expression(Result_Type t) :
      InstructionAST(), userSetValue(t)
    {
    } 
    Expression::~Expression() 
    {
    }
    const Result& Expression::eval() const
    {
      return userSetValue;
    }
    void Expression::setValue(const Result& knownValue) 
    {
      userSetValue = knownValue;
    }
    void Expression::clearValue()
    {
      userSetValue.defined = false;
    }
    int Expression::size() const
    {
      return userSetValue.size();
    }
    bool Expression::bind(Expression* expr, const Result& value)
    {
      bool retVal = false;
      if(*expr == *this)
      {
        //fprintf(stderr, "Binding %s to %ld\n", format().c_str(), value.convert<unsigned long>());
	setValue(value);
	return true;
      }
      //fprintf(stderr, "%s != %s in bind(), checking kids...\n", format().c_str(), expr->format().c_str());
      std::vector<InstructionAST::Ptr> children;
      getChildren(children);
      for(std::vector<InstructionAST::Ptr>::iterator curChild = children.begin();
	  curChild != children.end();
	  ++curChild)
      {
	Expression::Ptr curChild_asExpr = 
	dyn_detail::boost::dynamic_pointer_cast<Expression>(*curChild);
	if(curChild_asExpr)
	{
            bool tmp = curChild_asExpr->bind(expr, value);
            retVal = retVal || tmp;
	}
        else
        {
            //fprintf(stderr, "SKIPPING child %s, not an expression!\n", (*curChild)->format().c_str());
        }
      }
      return retVal;
    }
    bool Expression::isFlag() const
    {
      return false;
    }
    bool DummyExpr::isStrictEqual(const InstructionAST& ) const
    {
        return true;
    }
    bool DummyExpr::checkRegID(unsigned int ) const
    {
        return true;
    }

  };
};
