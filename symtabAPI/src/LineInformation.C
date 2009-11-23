/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "LineInformation.h"
#include <assert.h>
#include <list>
#include <cstring>
#include "boost/functional/hash.hpp"
#include "common/h/headers.h"
#include "Module.h"
#include "Serialization.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

#if ! defined(os_windows)      
struct SourceLineCompare 
{
   bool operator () ( const char * lhs, const char * rhs ) const;
};

typedef dyn_hash_set< const char *, boost::hash< const char * >, SourceLineCompare > SourceLineInternTable;

#else

struct SourceLineLess 
{
   bool operator () ( const char * lhs, const char * rhs ) const;
};

typedef std::set< const char *, SourceLineLess > SourceLineInternTable;

#endif 

namespace Dyninst {
namespace SymtabAPI {
class SourceLineInternalTableWrapper {
   public:
      SourceLineInternTable source_line_names;
      SourceLineInternalTableWrapper() {}
      ~SourceLineInternalTableWrapper(){}
      SourceLineInternTable &getTable() {return source_line_names;}
};
}}

SourceLineInternalTableWrapper *LineInformation::getSourceLineNamesW()
{
   if (sourceLineNamesPtr)
      return sourceLineNamesPtr;

   sourceLineNamesPtr = new SourceLineInternalTableWrapper();

   if (!sourceLineNamesPtr) 
      fprintf(stderr, "%s[%d]:  alloc failure here\n", FILE__, __LINE__);

   return sourceLineNamesPtr;
}

SourceLineInternTable &getSourceLineNames(LineInformation *li)
{
   if (!li->sourceLineNamesPtr)
      li->sourceLineNamesPtr = new SourceLineInternalTableWrapper();

   if (!li->sourceLineNamesPtr)
   {
      fprintf(stderr, "%s[%d]:  alloc prob\n", FILE__, __LINE__);
      abort();
   }

   return li->sourceLineNamesPtr->getTable();
}

LineInformation::LineInformation() : 
	AnnotationContainer<Statement>(),
   Dyninst::SymtabAPI::RangeLookup< Statement, Statement::StatementLess >(),
   sourceLineNamesPtr(NULL) 
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

   /* If we haven't already, intern the lineSource. */
   if ( lineSource == NULL ) 
   { 
      return false; 
   }

   const char * lineSourceInternal = NULL;
   typedef SourceLineInternTable::const_iterator IteratorType;
   SourceLineInternTable &sourceLineNames = getSourceLineNames(this);
   IteratorType found = sourceLineNames.find( lineSource );

   if ( found == sourceLineNames.end() ) 
   {
      lineSourceInternal = P_strdup( lineSource );
      assert( lineSourceInternal != NULL );
      sourceLineNames.insert( lineSourceInternal );
   }
   else 
   {
      lineSourceInternal = * found;
   }

   assert( lineSourceInternal != NULL );

   bool ret = addItem_impl( Statement(lineSourceInternal, lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr)); 

   return ret;
} /* end setLineToAddressRangeMapping() */

void LineInformation::addLineInfo(LineInformation *lineInfo)
{
   const_iterator iter = lineInfo->begin();

   for (; iter != lineInfo->end(); iter++)
   {
      addLine(iter->second.file_.c_str(), iter->second.line_, iter->second.column, 
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

	int strcmp_res = strcmp( lhs.file_.c_str(), rhs.file_.c_str());

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



#if ! defined( os_windows )
bool SourceLineCompare::operator () ( const char * lhs, const char * rhs ) const 
{
   return strcmp( lhs, rhs ) == 0;
} /* end SourceLineCompare() */
#else
bool SourceLineLess::operator () ( const char * lhs, const char * rhs ) const 
{
   return strcmp( lhs, rhs ) < 0;
} /* end SourceLineLess() */
#endif

/* We free the strings we allocated, and let the compiler clean up everything else:

   Section 10.4.6 [Stroustroup's C++]: "When a class object containing class
   objects is destroyed, the body of that object's own destructor is executed first,
   and then the members' destructors are executed in the reverse order of declaration." */

LineInformation::~LineInformation() 
{
   /* Apparently, the iterator depends on the hash of its current key
      to continue.  This should probably be cached, to allow me to free
      the current key if it's a pointer (and the hash over the pointed-to
      data), but I guess it's not strictly a bug. */

   const char * internedString = NULL;
   typedef SourceLineInternTable::const_iterator IteratorType;
   SourceLineInternTable &sourceLineNames = getSourceLineNames(this);
   SourceLineInternTable::iterator iterator = sourceLineNames.begin();

   while ( iterator != sourceLineNames.end() ) 
   {
      internedString = * iterator;
      ++iterator;
      free( const_cast< char * >( internedString ) );
   }	

} /* end LineInformation destructor */


Serializable *LineInformation::ac_serialize_impl(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
{
   //fprintf(stderr, "%s[%d]:  LineInformation::serialize -- IMPLEMENT ME sb = %p\n", 
   //      FILE__, __LINE__, sb);

	std::pair<int, int> mypair;
	std::pair<std::pair<int, int>, int> mypair2;

	ifxml_start_element(sb, tag);
	//gtranslate(sb, mypair);
	//gtranslate(sb, mypair2);
	gtranslate(sb, valuesByAddressRangeMap, "valuesByAddressRangeMap", "valueByAddressRange");
	gtranslate(sb, addressRangesByValueMap, "addressRangesByValueMap", "addressRangeByValue");
	gtranslate(sb, size_, "size");
	//multimap_translator<std::pair<Address, Address>, Statement> mt;
	//mt(sb, valuesByAddressRangeMap, "valuesByAddressRangeMap", "valueByAddressRange");
	//translate_multimap(sb, valuesByAddressRangeMap, "valuesByAddressRangeMap", "valueByAddressRange");

	//multimap_translator<std::pair<Address, Address>, Statement>(sb, addressRangesByValueMap, "addressRangesByValueMap", "addressRangeByValue");
	ifxml_end_element(sb, tag);
	return NULL;

}
