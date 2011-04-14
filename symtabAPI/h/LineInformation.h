/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#if ! defined( LINE_INFORMATION_H )
#define LINE_INFORMATION_H

#include "symutil.h"
#include "RangeLookup.h"
#include "Serialization.h"
#include "Annotatable.h"
#include "Module.h"

#define NEW_GETSOURCELINES_INTERFACE

namespace Dyninst{
namespace SymtabAPI{

class LineInformation : public AnnotationContainer<Statement>, 
                        private RangeLookup< Statement, Statement::StatementLess > 
{
	SYMTAB_EXPORT bool addItem_impl(Statement);
	SYMTAB_EXPORT Serializable *ac_serialize_impl(SerializerBase *, const char * = "lineInformation") THROW_SPEC (SerializerError);
   public:
      typedef RangeLookup< Statement, Statement::StatementLess >::const_iterator const_iterator;
      typedef RangeLookup< Statement, Statement::StatementLess >::AddressRange AddressRange;

      SYMTAB_EXPORT LineInformation();

      /* You MAY freely deallocate the lineSource strings you pass in. */
      SYMTAB_EXPORT bool addLine( const char * lineSource, 
            unsigned int lineNo, 
            unsigned int lineOffset, 
            Offset lowInclusiveAddr, 
            Offset highExclusiveAddr );

      SYMTAB_EXPORT void addLineInfo(LineInformation *lineInfo);	      

      SYMTAB_EXPORT bool addAddressRange( Offset lowInclusiveAddr, 
            Offset highExclusiveAddr, 
            const char * lineSource, 
            unsigned int lineNo, 
            unsigned int lineOffset = 0 );

      /* You MUST NOT deallocate the strings returned. */
      SYMTAB_EXPORT bool getSourceLines( Offset addressInRange, std::vector< Statement *> & lines );
      SYMTAB_EXPORT bool getSourceLines( Offset addressInRange, std::vector< LineNoTuple > & lines);

      SYMTAB_EXPORT bool getAddressRanges( const char * lineSource, unsigned int LineNo, std::vector< AddressRange > & ranges );

      SYMTAB_EXPORT const_iterator begin() const;
      SYMTAB_EXPORT const_iterator end() const;
      SYMTAB_EXPORT unsigned getSize() const;

      SYMTAB_EXPORT ~LineInformation();

   protected:
      /* We maintain internal copies of all the source file names.  Because
         both directions of the mapping include pointers to these names,
         maintain a separate list of them, and only ever deallocate those
         (in the destructor).  Note that it speeds and simplifies things
         to have the string pointers be the same. */

      unsigned size_;
}; /* end class LineInformation */

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* ! LINE_INFORMATION_H */
