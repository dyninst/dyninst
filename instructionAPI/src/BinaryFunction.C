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

#include "../h/BinaryFunction.h"
#include "Result.h"
#include "Visitor.h"

namespace Dyninst
{
  namespace InstructionAPI
  {

    Result doAddition(Result arg1, Result arg2, Result_Type ResultT)
    {
      switch(ResultT)
      {
      case s8:
	return Result(ResultT, arg1.convert<Result_type2type<s8>::type >() + arg2.convert<
		      Result_type2type<s8>::type >());
      case u8:
	return Result(ResultT, arg1.convert<Result_type2type<u8>::type >() + arg2.convert<
		      Result_type2type<u8>::type >());
      case s16:
	return Result(ResultT, arg1.convert<Result_type2type<s16>::type >() + arg2.convert<
		      Result_type2type<s16>::type >());
      case u16:
	return Result(ResultT, arg1.convert<Result_type2type<u16>::type >() + arg2.convert<
		      Result_type2type<u16>::type >());
      case s32:
	return Result(ResultT, arg1.convert<Result_type2type<s32>::type >() + arg2.convert<
		      Result_type2type<s32>::type >());
      case u32:
	return Result(ResultT, arg1.convert<Result_type2type<u32>::type >() + arg2.convert<
		      Result_type2type<u32>::type >());
      case s48:
	return Result(ResultT, arg1.convert<Result_type2type<s48>::type >() + arg2.convert<
		      Result_type2type<s48>::type >());
      case u48:
	return Result(ResultT, arg1.convert<Result_type2type<u48>::type >() + arg2.convert<
		      Result_type2type<u48>::type >());
      case s64:
	return Result(ResultT, arg1.convert<Result_type2type<s64>::type >() + arg2.convert<
		      Result_type2type<s64>::type >());
      case u64:
	return Result(ResultT, arg1.convert<Result_type2type<u64>::type >() + arg2.convert<
		      Result_type2type<u64>::type >());
      case sp_float:
	return Result(ResultT, arg1.convert<Result_type2type<sp_float>::type >() + arg2.convert<
		      Result_type2type<sp_float>::type >());
      case dp_float:
	return Result(ResultT, arg1.convert<Result_type2type<dp_float>::type >() + arg2.convert<
		      Result_type2type<dp_float>::type >());
      case bit_flag:
	return Result(ResultT, arg1.convert<Result_type2type<bit_flag>::type >() + arg2.convert<
		      Result_type2type<bit_flag>::type >());
      default:
	// m512 and dbl128 not implemented yet...
	return Result(ResultT);
      }
    }
    
    Result doMultiplication(Result arg1, Result arg2, Result_Type ResultT)
    {
      switch(ResultT)
      {
      case s8:
	return Result(ResultT, arg1.convert<Result_type2type<s8>::type >() * arg2.convert<
		      Result_type2type<s8>::type >());
      case u8:
	return Result(ResultT, arg1.convert<Result_type2type<u8>::type >() * arg2.convert<
		      Result_type2type<u8>::type >());
      case s16:
	return Result(ResultT, arg1.convert<Result_type2type<s16>::type >() * arg2.convert<
		      Result_type2type<s16>::type >());
      case u16:
	return Result(ResultT, arg1.convert<Result_type2type<u16>::type >() * arg2.convert<
		      Result_type2type<u16>::type >());
      case s32:
	return Result(ResultT, arg1.convert<Result_type2type<s32>::type >() * arg2.convert<
		      Result_type2type<s32>::type >());
      case u32:
	return Result(ResultT, arg1.convert<Result_type2type<u32>::type >() * arg2.convert<
		      Result_type2type<u32>::type >());
      case s48:
	return Result(ResultT, arg1.convert<Result_type2type<s48>::type >() * arg2.convert<
		      Result_type2type<s48>::type >());
      case u48:
	return Result(ResultT, arg1.convert<Result_type2type<u48>::type >() * arg2.convert<
		      Result_type2type<u48>::type >());
      case s64:
	return Result(ResultT, arg1.convert<Result_type2type<s64>::type >() * arg2.convert<
		      Result_type2type<s64>::type >());
      case u64:
	return Result(ResultT, arg1.convert<Result_type2type<u64>::type >() * arg2.convert<
		      Result_type2type<u64>::type >());
      case sp_float:
	return Result(ResultT, arg1.convert<Result_type2type<sp_float>::type >() * arg2.convert<
		      Result_type2type<sp_float>::type >());
      case dp_float:
	return Result(ResultT, arg1.convert<Result_type2type<dp_float>::type >() * arg2.convert<
		      Result_type2type<dp_float>::type >());
      case bit_flag:
	return Result(ResultT, arg1.convert<Result_type2type<bit_flag>::type >() * arg2.convert<
		      Result_type2type<bit_flag>::type >());
      default:
	// m512 and dbl128 not implemented yet...
	return Result(ResultT);
      }
    }
    Result operator+(const Result& arg1, const Result& arg2)
    {
      Result_Type ResultT = arg1.type > arg2.type ? arg1.type : arg2.type;
      if(!arg1.defined || !arg2.defined)
      {
	return Result(ResultT);
      }
      return doAddition(arg1, arg2, ResultT);

    }    
    Result operator*(const Result& arg1, const Result& arg2)
    {
      Result_Type ResultT = arg1.type > arg2.type ? arg1.type : arg2.type;
      if(!arg1.defined || !arg2.defined)
      {
	return Result(ResultT);
      }
      return doMultiplication(arg1, arg2, ResultT);

    }    
    const Result& BinaryFunction::eval() const
    {
        Expression::Ptr arg1 = boost::dynamic_pointer_cast<Expression>(m_arg1);
        Expression::Ptr arg2 = boost::dynamic_pointer_cast<Expression>(m_arg2);
        if(arg1 && arg2)
        {
            Result x = arg1->eval();
            Result y = arg2->eval();
            Result oracularResult = Expression::eval();
            if(x.defined && y.defined && !oracularResult.defined)
            {
                Result result = (*m_funcPtr)(x, y);
                const_cast<BinaryFunction*>(this)->setValue(result);
            }
        }
        return Expression::eval();
    }  
    bool BinaryFunction::bind(Expression* expr, const Result& value)
    {
        bool retVal = false;
        if(Expression::bind(expr, value))
        {
            return true;
        }
        retVal = retVal | m_arg1->bind(expr, value);
        retVal = retVal | m_arg2->bind(expr, value);
		if(retVal) clearValue();
        return retVal;
    }

    void BinaryFunction::apply(Visitor* v)
    {
        m_arg1->apply(v);
        m_arg2->apply(v);
        v->visit(this);
    }
    bool BinaryFunction::isAdd() const
    {
        return typeid(*m_funcPtr) == typeid(addResult);
    }
    bool BinaryFunction::isMultiply() const
    {
        return typeid(*m_funcPtr) == typeid(multResult);

    }
  };
};

