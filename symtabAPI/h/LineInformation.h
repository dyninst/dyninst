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
#include "util.h"
#include "RangeLookup.h"

namespace Dyninst{
namespace SymtabAPI{

/* This is clumsy. */
namespace LineInformationImpl {
        class LineNoTuple{
           public:
           DLLEXPORT LineNoTuple(const char *file_, unsigned int line_, unsigned int col_=0) : 
             first(file_),
             second(line_),
             column(col_) {}
           const char *first; // really file
           unsigned int second; // really line
           unsigned int column;
           DLLEXPORT bool operator==(const LineNoTuple &cmp) const {
             if (second != cmp.second) return false;
             if (column != cmp.column) return false;
             //  is compare-by-pointer OK here, or do we really have to really strcmp?
             return (!strcmp(first,cmp.first)); 
           }
        };
	
	/* Explicit comparison functors seems slightly less confusing than using
	   operator <() via an implicit Less<> template argument to the maps. */
	struct LineNoTupleLess {
		bool operator () ( LineNoTuple lhs, LineNoTuple rhs ) const;
	};
} /* end namespace LineInformationImpl */			

DLLEXPORT typedef LineInformationImpl::LineNoTuple LineNoTuple;

class LineInformation : private RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess > {
	public:
		typedef LineInformationImpl::LineNoTuple LineNoTuple;
		typedef RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >::const_iterator const_iterator;
		typedef RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >::AddressRange AddressRange;
		
		DLLEXPORT LineInformation();

		/* You MAY freely deallocate the lineSource strings you pass in. */
		DLLEXPORT bool addLine( const char * lineSource, 
                              unsigned int lineNo, 
                              unsigned int lineOffset, 
                              Offset lowInclusiveAddr, 
                              Offset highExclusiveAddr );
		DLLEXPORT void addLineInfo(LineInformation *lineInfo);	      
		DLLEXPORT bool addAddressRange( Offset lowInclusiveAddr, 
                                      Offset highExclusiveAddr, 
                                      const char * lineSource, 
                                      unsigned int lineNo, 
                                      unsigned int lineOffset = 0 );
		
		/* You MUST NOT deallocate the strings returned. */
		DLLEXPORT bool getSourceLines( Offset addressInRange, std::vector< LineNoTuple > & lines );
		DLLEXPORT bool getAddressRanges( const char * lineSource, unsigned int LineNo, std::vector< AddressRange > & ranges );
		
		DLLEXPORT const_iterator begin() const;
		DLLEXPORT const_iterator end() const;
		
		DLLEXPORT ~LineInformation();
		
	protected:
		/* We maintain internal copies of all the source file names.  Because
		   both directions of the mapping include pointers to these names,
		   maintain a separate list of them, and only ever deallocate those
		   (in the destructor).  Note that it speeds and simplifies things
		   to have the string pointers be the same. */

#if ! defined( _MSC_VER )		   
		struct SourceLineCompare {
			bool operator () ( const char * lhs, const char * rhs ) const;
			};
			
		typedef __gnu_cxx::hash_set< const char *, __gnu_cxx::hash< const char * >, SourceLineCompare > SourceLineInternTable;
#else

		struct SourceLineLess {
			bool operator () ( const char * lhs, const char * rhs ) const;
			};
			
		typedef std::set< const char *, SourceLineLess > SourceLineInternTable;
#endif 
		SourceLineInternTable sourceLineNames;
	}; /* end class LineInformation */

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* ! LINE_INFORMATION_H */
