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

#if ! defined( LINE_INFORMATION_H )
#define LINE_INFORMATION_H

#include "symutil.h"
#include "RangeLookup.h"
#include "Serialization.h"
#include "Annotatable.h"
#include "Module.h"

namespace Dyninst{
namespace SymtabAPI{

class SourceLineInternalTableWrapper;

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
      SYMTAB_EXPORT bool getAddressRanges( const char * lineSource, unsigned int LineNo, std::vector< AddressRange > & ranges );

      SYMTAB_EXPORT const_iterator begin() const;
      SYMTAB_EXPORT const_iterator end() const;
      SYMTAB_EXPORT unsigned getSize() const;

      SYMTAB_EXPORT ~LineInformation();

      // double secret private:
      SourceLineInternalTableWrapper *getSourceLineNamesW();
      SourceLineInternalTableWrapper *sourceLineNamesPtr;

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
