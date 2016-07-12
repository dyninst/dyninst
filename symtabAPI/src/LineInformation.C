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

#include "LineInformation.h"
#include <assert.h>
#include <list>
#include <cstring>
#include "boost/functional/hash.hpp"
#include "common/src/headers.h"
#include "Module.h"
#include "Serialization.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

LineInformation::LineInformation() : 
    Dyninst::SymtabAPI::RangeLookup< Statement, Statement::StatementLess >()
{
   size_ = 0;
} /* end LineInformation constructor */

bool LineInformation::addItem_impl(Statement s)
{
   size_++;

   bool ret = addValue( s, s.startAddr(), s.endAddr() );
	return ret;
}
bool LineInformation::addLine( const char * lineSource, 
      unsigned int lineNo, 
      unsigned int lineOffset, 
      Offset lowInclusiveAddr, 
      Offset highExclusiveAddr ) 
{

   bool ret = addItem_impl( Statement(lineSource, lineNo, lineOffset, 
                                      lowInclusiveAddr, highExclusiveAddr)); 

   return ret;
} /* end setLineToAddressRangeMapping() */

void LineInformation::addLineInfo(LineInformation *lineInfo)
{
   const_iterator iter = lineInfo->begin();

   for (; iter != lineInfo->end(); iter++)
   {
      addLine(iter->second.file_, iter->second.line_, iter->second.column,
            iter->first.first, iter->first.second);
   }
}

bool LineInformation::addAddressRange( Offset lowInclusiveAddr, 
      Offset highExclusiveAddr, 
      const char * lineSource, 
      unsigned int lineNo, 
      unsigned int lineOffset ) 
{
   return addLine( lineSource, lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr );
} /* end setAddressRangeToLineMapping() */

bool LineInformation::getSourceLines( Offset addressInRange, 
      vector< Statement *> & lines ) 
{
   return getValues( addressInRange, lines );
} /* end getLinesFromAddress() */

bool LineInformation::getSourceLines( Offset addressInRange, 
                                      vector<LineNoTuple> &lines) 
{
   vector<Statement *> plines;
   bool result = getValues(addressInRange, plines);
   if (!result) {
      return false;
   }
   for (vector<Statement *>::iterator i = plines.begin(); i != plines.end(); i++) {
      LineNoTuple lnt = **i;
      lines.push_back(lnt);
   }
   return true;
} /* end getLinesFromAddress() */

bool LineInformation::getAddressRanges( const char * lineSource, 
      unsigned int lineNo, vector< AddressRange > & ranges ) 
{
   bool ret = Dyninst::SymtabAPI::RangeLookup< Statement, Statement::StatementLess >::getAddressRanges( Statement( lineSource, lineNo ), ranges );

   return ret;
} /* end getAddressRangesFromLine() */

LineInformation::const_iterator LineInformation::begin() const 
{
   return Dyninst::SymtabAPI::RangeLookup< Statement, Statement::StatementLess >::begin();
} /* end begin() */

LineInformation::const_iterator LineInformation::end() const 
{
   return Dyninst::SymtabAPI::RangeLookup< Statement, Statement::StatementLess >::end();
} /* end begin() */

unsigned LineInformation::getSize() const
{
   return size_;
}

bool Statement::StatementLess::operator () ( const Statement &lhs, const Statement &rhs ) const
{
	//  dont bother with ordering by column information yet.

	int strcmp_res = strcmp( lhs.file_, rhs.file_);

	if (strcmp_res < 0 )
		return true;

	if ( strcmp_res == 0 )
	{
		if ( lhs.line_ < rhs.line_ )
			return true;
	}

	return false;
} /* end StatementLess() */

bool Statement::operator==(const Statement &cmp) const 
{
	if (line_ != cmp.line_) return false;
	if (column != cmp.column) return false;

	//  is compare-by-pointer OK here, or do we really have to really strcmp?
	return (file_ == cmp.file_);
}

/* We free the strings we allocated, and let the compiler clean up everything else:

   Section 10.4.6 [Stroustroup's C++]: "When a class object containing class
   objects is destroyed, the body of that object's own destructor is executed first,
   and then the members' destructors are executed in the reverse order of declaration." */

LineInformation::~LineInformation() 
{
} /* end LineInformation destructor */

