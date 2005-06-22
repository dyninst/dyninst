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

#if ! defined( RANGE_LOOKUP_H )
#define RANGE_LOOKUP_H

#include <map>
#include <vector>

#include "common/h/Types.h"

/* The Windows C++ compiler is broken and won't instantiate this function when it's in RangeLookup proper. */
class RangeLookupImpl {
	public:
		typedef std::pair< Address, Address >						AddressRange;
		
		/* Explicit comparison functors seems slightly less confusing than using
		   operator <() via an implicit Less<> template argument to the maps. */
		struct AddressRangeLess {
			bool operator () ( const AddressRange & lhs, const AddressRange & rhs ) const;
			};
	}; /* end class RangeLookupImpl */

template< class Value, class ValueLess > class RangeLookup {
	protected:
		typedef RangeLookupImpl::AddressRange						AddressRange;
		typedef RangeLookupImpl::AddressRangeLess					AddressRangeLess;
	
		typedef std::multimap< Value, AddressRange, ValueLess > AddressRangeByValue;
		typedef std::multimap< AddressRange, Value, AddressRangeLess > ValueByAddressRange;

	public:		
		typedef typename ValueByAddressRange::const_iterator		const_iterator;
		
		RangeLookup();

		/* Values are copied: a RangeLookup considers itself the primary repository. */
		bool addValue( Value v, Address lowInclusiveAddr, Address highExclusiveAddr );
		bool addAddressRange( Address lowInclusiveAddr, Address highExclusiveAddr, Value v );

		/* Likewise, copies of the values are returned. */
		bool getValues( Address addressInRange, std::vector< Value > & values );
		bool getAddressRanges( Value v, std::vector< AddressRange > & ranges );

		const_iterator begin() const;
		const_iterator end() const;
		
		~RangeLookup();
		
		/* DEBUG */ void dump( FILE * stream );
		/* DEBUG */ static void testInsertionSpeed();
		
	protected:
		ValueByAddressRange valuesByAddressRangeMap;
		AddressRangeByValue addressRangesByValueMap;
	}; /* end class RangeLookup */

#endif /* ! RANGE_LOOKUP */
