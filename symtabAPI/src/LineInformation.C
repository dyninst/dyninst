/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "symtabAPI/h/LineInformation.h"
#include <assert.h>
#include <list>
#include <cstring>
#include "boost/functional/hash.hpp"
#include "common/h/headers.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;

#include "RangeLookup.t"

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
   Dyninst::SymtabAPI::RangeLookup< LineNoTuple, LineNoTuple::LineNoTupleLess >(),
   sourceLineNamesPtr(NULL) 
{
   size_ = 0;
} /* end LineInformation constructor */

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
   size_++;

   bool ret = addValue( LineNoTuple(lineSourceInternal, lineNo, lineOffset), 
         lowInclusiveAddr, highExclusiveAddr );

   return ret;
} /* end setLineToAddressRangeMapping() */

void LineInformation::addLineInfo(LineInformation *lineInfo)
{
   const_iterator iter = lineInfo->begin();

   for (; iter != lineInfo->end(); iter++)
   {
      addLine(iter->second.first, iter->second.second, iter->second.column, 
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
      vector< LineNoTuple > & lines ) 
{
   return getValues( addressInRange, lines );
} /* end getLinesFromAddress() */

bool LineInformation::getAddressRanges( const char * lineSource, 
      unsigned int lineNo, vector< AddressRange > & ranges ) 
{
   bool ret = Dyninst::SymtabAPI::RangeLookup< LineNoTuple, LineNoTuple::LineNoTupleLess >::getAddressRanges( LineNoTuple( lineSource, lineNo ), ranges );
   if (!ret)
   {
      fprintf(stderr, "%s[%d]:  failed to getAddressRanges for %s[%d]\n", FILE__, __LINE__, lineSource, lineNo);
   }

   return ret;
} /* end getAddressRangesFromLine() */

LineInformation::const_iterator LineInformation::begin() const 
{
   return Dyninst::SymtabAPI::RangeLookup< LineNoTuple, LineNoTuple::LineNoTupleLess >::begin();
} /* end begin() */

LineInformation::const_iterator LineInformation::end() const 
{
   return Dyninst::SymtabAPI::RangeLookup< LineNoTuple, LineNoTuple::LineNoTupleLess >::end();
} /* end begin() */

unsigned LineInformation::getSize() const
{
   return size_;
}

bool LineNoTuple::LineNoTupleLess::operator () ( LineNoTuple lhs, LineNoTuple rhs ) const 
{
   //  dont bother with ordering by column information yet.

   int strcmp_res = strcmp( lhs.first, rhs.first);

   if (strcmp_res < 0 ) 
      return true; 

   if ( strcmp_res == 0 ) 
   {
      if ( lhs.second < rhs.second )  
         return true; 
   }

   return false; 
} /* end LineNoTupleLess() */

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

LineNoTuple::LineNoTuple(const char *file_, unsigned int line_, unsigned int col_) :
   first(file_),
   second(line_),
   column(col_) 
{
}

bool LineNoTuple::operator==(const LineNoTuple &cmp) const 
{
   if (second != cmp.second) return false;
   if (column != cmp.column) return false;

   //  is compare-by-pointer OK here, or do we really have to really strcmp?
   return (!strcmp(first,cmp.first)); 
}

void LineInformation::serialize(SerializerBase *sb, const char *)
{
   fprintf(stderr, "%s[%d]:  LineInformation::serialize -- IMPLEMENT ME sb = %p\n", 
         FILE__, __LINE__, sb);
}
