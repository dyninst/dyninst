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

#include <map>

#if ! defined( os_windows )
#include <ext/hash_set>
#else
/* We don't compile with gcc on Windows.  *sigh*  This will be slower,
   but should be functionally identical. */
#include <set>
#include <vector>
#endif

#include "common/h/Types.h"

class LineInformation {
	public:		
		typedef std::pair< const char *, unsigned int >		LineNoTuple;
		typedef std::pair< Address, Address >				AddressRange;
		
	protected:
		/* Explicit comparison functors seems slightly less confusing than using
		   operator <() via an implicit Less<> template argument to the maps. */
		struct LineNoTupleLess {
			bool operator () ( LineNoTuple lhs, LineNoTuple rhs ) const;
			};
		
		struct AddressRangeLess {
			bool operator () ( AddressRange lhs, AddressRange rhs ) const;
			};
			
		typedef std::multimap< LineNoTuple, AddressRange, LineNoTupleLess > LineNoTupleToAddressRangeMap;
		typedef std::multimap< AddressRange, LineNoTuple, AddressRangeLess > RangeMapByAddress;		

	public:		
		typedef RangeMapByAddress::const_iterator			const_iterator;
		
		LineInformation();

		/* You MAY freely deallocate the lineSource strings you pass in. */
		bool addLine( const char * lineSource, unsigned int lineNo, Address lowInclusiveAddr, Address highExclusiveAddr );
		bool addAddressRange( Address lowInclusiveAddr, Address highExclusiveAddr, const char * lineSource, unsigned int lineNo );
		
		/* You MUST NOT deallocate the strings returned. */
		bool getSourceLines( Address addressInRange, std::vector< LineNoTuple > & lines );
		bool getAddressRanges( const char * lineSource, unsigned int LineNo, std::vector< AddressRange > & ranges );
		
		const_iterator begin() const;
		const_iterator end() const;
		
		~LineInformation();
		
		/* DEBUG */ void dump( FILE * stream );
		/* DEBUG */ static void testInsertionSpeed();
		
	protected:		
		struct SourceLineCompare {
			bool operator () ( const char * lhs, const char * rhs ) const;
			};
			
		/* We maintain internal copies of all the source file names.  Because
		   both directions of the mapping include pointers to these names,
		   maintain a separate list of them, and only ever deallocate those
		   (in the destructor).  Note that it speeds and simplifies things
		   to have the string pointers be the same. */
#if ! defined( os_windows )		   
		typedef __gnu_cxx::hash_set< const char *, __gnu_cxx::hash< const char * >, SourceLineCompare > SourceLineInternTable;
#else
		struct SourceLineLess {
			bool operator () ( const char * lhs, const char * rhs ) const;
			};
			
		typedef std::set< const char *, SourceLineLess > SourceLineInternTable;
#endif 
		SourceLineInternTable sourceLineNames;
				
		LineNoTupleToAddressRangeMap lineNoTupleToAddressRangeMap;
		RangeMapByAddress rangesByAddress;

	}; /* end class LineInformation */

#if defined( rs6000_ibm_aix4_1 )
/* This class is only used in symtab.C; the only reason it's in
   this header file is so that template0.C can include it to
   instantiate pdvector< IncludeFileInfo >. */   

#include <common/h/String.h>
class IncludeFileInfo {
	public:
		unsigned int begin;
		unsigned int end;
		pdstring name;

		IncludeFileInfo() : begin(0), end(0) {};
		IncludeFileInfo( int _begin, const char *_name ) : begin(_begin), end(0), name(_name) {};
	};	
#endif /* defined( rs600_ibm_aix4_1 ) */

#endif /* ! LINE_INFORMATION_H */
