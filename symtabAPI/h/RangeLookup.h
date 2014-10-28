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

#if ! defined( RANGE_LOOKUP_H )
#define RANGE_LOOKUP_H

#include <map>
#include <vector>
#include <list>
#include <assert.h>
#include "dyntypes.h"
#include "util.h"

namespace Dyninst
{
namespace SymtabAPI
{

/* The Windows C++ compiler is broken and won't instantiate this function when it's in RangeLookup proper. */

class SYMTAB_EXPORT RangeLookupImpl 
{
	public:
		typedef std::pair< Offset, Offset >	AddressRange;

		/* Explicit comparison functors seems slightly less confusing than using
		   operator <() via an implicit Less<> template argument to the maps. */

		struct AddressRangeLess {
			bool operator()(const AddressRange &lhs, const AddressRange &rhs) const
			{
				if ( lhs.first < rhs.first ) 
				{ 
					return true; 
				}
				else if ( lhs.first == rhs.first ) 
				{
					if ( lhs.second < rhs.second ) 
					{
						return true;
					}
					else
					{
						return false; 
					}
				}

				return false; 
			} /* end AddressRangeLess() */
		};

}; /* end class RangeLookupImpl */

template< class Value, class ValueLess > 
class SYMTAB_EXPORT RangeLookup 
{
	protected:
		typedef RangeLookupImpl::AddressRange AddressRange;
		typedef RangeLookupImpl::AddressRangeLess AddressRangeLess;

		typedef std::multimap< Value, AddressRange, ValueLess > AddressRangeByValue;
		typedef std::multimap< AddressRange, Value, AddressRangeLess > ValueByAddressRange;

	public:		
		typedef typename ValueByAddressRange::const_iterator const_iterator;

		RangeLookup();

		/* Values are copied: a RangeLookup considers itself the primary repository. */
		bool addValue( Value v, Offset lowInclusiveAddr, Offset highExclusiveAddr );
		bool addAddressRange( Offset lowInclusiveAddr, Offset highExclusiveAddr, Value v );

		/* Likewise, copies of the values are returned. */
		bool getValues( Offset addressInRange, std::vector< Value *> & values );
		bool getAddressRanges( Value v, std::vector< AddressRange > & ranges );

		const_iterator begin() const;
		const_iterator end() const;

		~RangeLookup();

		// /* DEBUG */ void dump( FILE * stream );
		// /* DEBUG */ static void testInsertionSpeed();

	protected:
		ValueByAddressRange valuesByAddressRangeMap;
		AddressRangeByValue addressRangesByValueMap;
}; /* end class RangeLookup */

template< class Value, class ValueRange > RangeLookup< Value, ValueRange >::RangeLookup() :
	valuesByAddressRangeMap(),
	addressRangesByValueMap() {}

/* Private refactoring function: erase()s value from map. */
template< class M >
bool removeByValue( M & map, const typename M::value_type & value ) 
{
	std::pair< typename M::iterator, typename M::iterator > range = map.equal_range( value.first );	
#if 0
#if ! defined( os_windows )
	std::pair< typename M::iterator, typename M::iterator > range = map.equal_range( value.first );	
#else
	std::pair< M::iterator, M::iterator > range = map.equal_range( value.first );	
#endif
#endif

	for ( ; range.first != range.second && range.first != map.end(); ++range.first ) 
	{
		typename M::value_type &cmp = *range.first;
		if (cmp == value ) 
		{
			map.erase( range.first );
			return true;
		}
	}

	return false;
	
} /* end removeByValue() */

/* We maintain the invariant that low and high address sort orders are the same.  (If we
   didn't, we could only find one of the two endpoints of the range of ranges we should
   check.  If we find one endpoint and iterate over the other sort order until we found
   the other, a range contained by another range would change the sort order and cause
   ranges containing the search target to be skippped.)  So we check, whenever we insert
   a new range, if it contains or is contained by any other range.

   Since we insert elements one at a time, we can assume that no range already present
   contains another range.  Furthermore, if no range contains another range, then any
   range (B) between the inserted range (I) and a range which might contain it (A)
   must also contain I.  (Where ranges are ordered by their low address.)

   (Proof by contradiction: suppose there exists a B not containing I.  Because B is between
   I and A, B's low address is lower than I's.  Therefore, because B does not contain I, its
   high address must be lower than I's.  Because I is contained by A, its high address is
   lower than A's.  By transitivity, B's high address is lower than A's.  Since B's low
   address is higher than A's (because B is between A and I), B is contained by A.  Contradiction.)

   Therefore, we only need to check ranges lower than I until one of them does not contain it.
   If we split each containing range in the middle of I, we can insert I and maintain our
   no-overlap invariant.

   Similar logic applies in reverse direction: if I contains a range, it contains every
   range between it and I.

   Furthermore, the same range can't be both contained and contained-by: if it's contained-by,
   its container contains whatever it does.

   We want to split ranges so that their low and high addresses sort in the same order.  (Strictly
   speaking, the above conditions ought to be "not a range which will cause the low and high address
   sort order to be different, but it ends up not mattering, AFAICT.)

   If we're inserting a contained range, we split the containing ranges on the contained range's right edge.

   If we're inserted a containing range, we split it and the other ranges on the least upper address in
   each group of contained ranges.
 */

template <class Value, class ValueRange> 
bool RangeLookup< Value, ValueRange >::addValue(Value v, 
		Offset lowInclusiveAddr, 
		Offset highExclusiveAddr) 
{
	/* Verify the input. */
	if ( lowInclusiveAddr >= highExclusiveAddr ) 
	{ 
		return false; 
	}

	/* If we've already got this range, it's safe to insert it. */

	typedef typename ValueByAddressRange::iterator RangeIterator;
	std::pair< RangeIterator, RangeIterator > rangeOfRanges;

	rangeOfRanges = valuesByAddressRangeMap.equal_range(AddressRange(lowInclusiveAddr, 
				highExclusiveAddr));

	if (   (rangeOfRanges.first != rangeOfRanges.second) 
			|| (valuesByAddressRangeMap.size() == 0) ) 
	{
		// insert() is amortized constant time if the new value is inserted 
		// immediately before the hint. 
	  /* DEBUG */ //fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx)\n", __FILE__, __LINE__, lowInclusiveAddr, highExclusiveAddr);

		valuesByAddressRangeMap.insert( rangeOfRanges.second, 
				std::make_pair(AddressRange(lowInclusiveAddr, 
						highExclusiveAddr), v));

		addressRangesByValueMap.insert( std::make_pair( v, AddressRange(lowInclusiveAddr, 
						highExclusiveAddr)));
		return true;
	}

	/* Otherwise, we need to look for containing ranges. */

	typedef std::pair< AddressRange, Value > Range;
	typedef std::list< Range > RangeList;
	RangeIterator downIterator = rangeOfRanges.second;

	if (rangeOfRanges.second != valuesByAddressRangeMap.begin()) 
	{
		--downIterator; 
	}

	RangeList containingRangeList;
	for (; ( (downIterator->first.first <= lowInclusiveAddr)
				&& (highExclusiveAddr < downIterator->first.second) )
			; --downIterator ) 
	{

	  /* DEBUG */ //fprintf( stderr, "%s[%d]: found containing range [0x%lx, 0x%lx) while inserting [0x%lx, 0x%lx) \n", __FILE__, __LINE__, downIterator->first.first, downIterator->first.second, lowInclusiveAddr, highExclusiveAddr);

		containingRangeList.push_back( * downIterator );

		if ( downIterator == valuesByAddressRangeMap.begin() ) 
			break; 
	}

	/* We also need to look for contained ranges. */	
	RangeIterator upIterator = rangeOfRanges.second;	
	RangeList containedRangeList;

	for (; (upIterator != valuesByAddressRangeMap.end()) 
			&& ((lowInclusiveAddr <= upIterator->first.first)
				&& (upIterator->first.second < highExclusiveAddr) )
			; ++upIterator ) 
	{

	  /* DEBUG */ //fprintf( stderr, "%s[%d]: found contained range [0x%lx, 0x%lx) while inserting [0x%lx, 0x%lx) \n", __FILE__, __LINE__, upIterator->first.first, upIterator->first.second, lowInclusiveAddr, highExclusiveAddr);

		containedRangeList.push_back( * upIterator );
	} /* end iteration looking for contained ranges */

	if (containingRangeList.size() == 0 && containedRangeList.size() == 0 ) 
	{
	  /* DEBUG */ //fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx)\n", __FILE__, __LINE__, lowInclusiveAddr, highExclusiveAddr);
		/* insert() is amortized constant time if the new value is inserted immediately before the hint. */
		valuesByAddressRangeMap.insert(rangeOfRanges.second, 
				std::make_pair(AddressRange(lowInclusiveAddr, 
						highExclusiveAddr), v));
		addressRangesByValueMap.insert( std::make_pair(v,AddressRange(lowInclusiveAddr, 
						highExclusiveAddr)));
		return true;
	}

	/* TODO: combine lists; if wasContaining, splitaddrlist is just highExclusiveAddr */

	if (containingRangeList.size() != 0) 
	{
		Offset splitAddress = highExclusiveAddr;

		/* Remove the old (containing) ranges, split them, insert the new ones. */
		typename RangeList::const_iterator containingIterator = containingRangeList.begin();

		for ( ; containingIterator != containingRangeList.end(); ++containingIterator ) 
		{
			AddressRange ar = containingIterator->first;
			Value lnt = containingIterator->second;

			/* DEBUG */ //fprintf( stderr, "%s[%d]: removing range [0x%lx, 0x%lx) (%s, %u)...\n", __FILE__, __LINE__, ar.first, ar.second, lnt.first, lnt.second );

			// These could be out of sync, though I'm not sure how. We know that
			// values by address range has what we're looking for, because we've got
			// an iterator; the converse is not automatically true. So check that remove
			// first. Furthermore, bail with "return false" rather than asserting.
			if(!(removeByValue( addressRangesByValueMap, std::make_pair( lnt, ar ) )) ||
			   !(removeByValue( valuesByAddressRangeMap, * containingIterator )))
			{
			  return false;
			  
			}
			//assert( removeByValue( valuesByAddressRangeMap, * containingIterator ) );
			//assert( removeByValue( addressRangesByValueMap, std::make_pair( lnt, ar ) ) );

		        /* DEBUG */ //fprintf( stderr, "%s[%d]: ... done removing range [0x%lx, 0%lx) (%s, %u)\n", __FILE__, __LINE__, ar.first, ar.second, lnt.first, lnt.second );
			/* DEBUG */ //fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, ar.first, splitAddress, lnt.first, lnt.second );

			valuesByAddressRangeMap.insert(std::make_pair(AddressRange(ar.first, 
							splitAddress), lnt));
			addressRangesByValueMap.insert( std::make_pair(lnt,AddressRange(ar.first, 
							splitAddress)));

			/* DEBUG */ //fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, splitAddress, ar.second, lnt.first, lnt.second );

			valuesByAddressRangeMap.insert(std::make_pair(AddressRange(splitAddress, 
							ar.second ), lnt ) );
			addressRangesByValueMap.insert(std::make_pair(lnt, AddressRange(splitAddress, 
							ar.second ) ) );
		} /* end removes/split/insert iteration */

		/* DEBUG */ //fprintf( stderr, "%s[%d]: inserting new range [0x%lx, 0x%lx)\n\n", __FILE__, __LINE__, lowInclusiveAddr, highExclusiveAddr );

		/* Insert the new range. */
		valuesByAddressRangeMap.insert(std::make_pair(AddressRange(lowInclusiveAddr, 
						highExclusiveAddr ), v ) );
		addressRangesByValueMap.insert(std::make_pair(v, AddressRange(lowInclusiveAddr, 
						highExclusiveAddr ) ) );
		return true;
	} /* end if the range to be inserted is contained by other ranges. */

	if (containedRangeList.size() != 0 ) 
	{
		/* We'll split the ranges at these points. */
		typedef std::list< Offset > AddressList;
		AddressList splitAddressList;

		/* Overlapping ranges overlap until a discontuity, so we only need to
		   know about one of them. */

		typename RangeList::const_iterator containedIterator = containedRangeList.begin();
		Offset splitAddress = containedIterator->first.second;
		++containedIterator;

		for ( ; containedIterator != containedRangeList.end(); ++containedIterator ) 
		{
			/* Is there a discontinuity? */

			if (containedIterator->first.first >= splitAddress) 
			{
			  /* DEBUG */ //fprintf( stderr, "%s[%d]: will split at 0x%lx\n", __FILE__, __LINE__, splitAddress );

				/* If there is, our current splitAddress should be a split, */
				splitAddressList.push_back( splitAddress );				

				/* and the high end of the next contained range will be our next split point. */
				splitAddress = containedIterator->first.second;				
			}
			else 
			{
				splitAddress = containedIterator->first.second; 
			}
		} /* end split point determination iteration */

		/* DEBUG */ fprintf( stderr, "%s[%d]: will split at 0x%lx\n", __FILE__, __LINE__, splitAddress );

		splitAddressList.push_back( splitAddress );

		/* Split the range to be inserted. */

		Offset lowAddress = lowInclusiveAddr;
		AddressList::const_iterator splitAddressIterator = splitAddressList.begin();
		Offset highAddress = 0;

		for ( ; splitAddressIterator != splitAddressList.end(); ++ splitAddressIterator ) 
		{
			highAddress = * splitAddressIterator;

			/* DEBUG */ // fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) \n", __FILE__, __LINE__, lowAddress, highAddress);

			valuesByAddressRangeMap.insert(std::make_pair(AddressRange(lowAddress, highAddress ), v ) );
			addressRangesByValueMap.insert(std::make_pair(v, AddressRange(lowAddress, highAddress ) ) );

			lowAddress = highAddress;
		} /* end iteration to split range to be inserted. */

		/* DEBUG */ //fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx)\n", __FILE__, __LINE__, highAddress, highExclusiveAddr);

		valuesByAddressRangeMap.insert(std::make_pair(AddressRange( highAddress, highExclusiveAddr ), v ) );
		addressRangesByValueMap.insert(std::make_pair(v, AddressRange( highAddress, highExclusiveAddr ) ) );	

		/* We done did it. */
		return true;
	} /* end if the range to be inserted contains other ranges. */

	/* Something Terrible happened. */
	return false;
} /* end addValue() */

template< class Value, class ValueRange > 
bool RangeLookup< Value, ValueRange >::addAddressRange( Offset lowInclusiveAddr, 
		Offset highExclusiveAddr, 
		Value v ) 
{
	return addValue( v, lowInclusiveAddr, highExclusiveAddr );
} /* end addAddressRange() */


/*	We maintain the invariant that the ranges sort in the same order on
	their low and high addresses.  In this case, we only need to search from
	the range whose low address is the least too large down to the range
	whose high address is the first too small to find all ranges which
	contain the desired address.	
 */	   
template< class Value, class ValueRange > 
bool RangeLookup< Value, ValueRange >::getValues( Offset addressInRange, 
		std::vector< Value *> & values ) 
{
	/* We can't find an address if we have no ranges. */
	if ( valuesByAddressRangeMap.size() == 0 ) 
		return false;

	/* Because the sort order of the low and high addresses is the same, we know every range
	   which could contain addressInRange must be below the range whose low address is one
	   larger, and above the range whose high address is the same.  We use addressInRange + 1
	   to make sure we find one past the end of a span of ranges with the same low address,
	   so we decrement the iterator to point it at the first range which could contain
	   addressInRange.  If equal_range() returns end(), then decrementing the iterator
	   ensures we start checking with a real range. */

	typedef typename ValueByAddressRange::const_iterator RangeIterator;
	std::pair< RangeIterator, RangeIterator > lowRange 
		= valuesByAddressRangeMap.equal_range( AddressRange( addressInRange + 1, 0 ) );

	if (lowRange.second == valuesByAddressRangeMap.begin())
		return false; //Searched for an address lower than any in the map.

	assert( lowRange.first == lowRange.second );
	RangeIterator hHighEnd = --(lowRange.second);

	/* Some implementations get stuck on valuesByAddressRangeMap.begin(), apparently. */

	for ( ; hHighEnd->first.second > addressInRange && hHighEnd != valuesByAddressRangeMap.end();
		   	--hHighEnd ) 
	{
		if (   (hHighEnd->first.first <= addressInRange)
				&& (addressInRange < hHighEnd->first.second) ) 
		{
			values.push_back( const_cast<Value *> (& hHighEnd->second) );
		}

		if (hHighEnd == valuesByAddressRangeMap.begin() ) 
			break; 
	} /* end iteration over possible range matches. */

	if ( values.size() == 0 ) 
		return false; 

	return true;
} /* end getLinesFromAddress() */

template< class Value, class ValueRange > 
bool RangeLookup< Value, ValueRange >::getAddressRanges(Value v, 
		std::vector<AddressRange> &ranges) 
{
	/* Look for the specified lineSource:lineNo. */
	typedef typename AddressRangeByValue::const_iterator IteratorType;
	std::pair< IteratorType, IteratorType > range = addressRangesByValueMap.equal_range( v );

	/* If equal_range() doesn't find anything, range.first and range.second will be equal. */

	if ( range.first == range.second ) 
	{
		return false;
	}

	/* Otherwise, copy out the found ranges. */
	for ( IteratorType i = range.first; i != range.second; ++i ) 
	{
		//    ranges.push_back( AddressRange(i->second));
		ranges.push_back(i->second);
	} /* end iteration over located address ranges. */

	return true;
} /* end getAddressRangesFromLine() */

template< class Value, class ValueRange > 
typename RangeLookup< Value, ValueRange >::const_iterator RangeLookup< Value, ValueRange >::begin() const 
{
	return valuesByAddressRangeMap.begin();
} /* end begin() */

template< class Value, class ValueRange > 
typename RangeLookup< Value, ValueRange >::const_iterator RangeLookup< Value, ValueRange >::end() const 
{
	return valuesByAddressRangeMap.end();
} /* end begin() */

template< class Value, class ValueRange > 
RangeLookup< Value, ValueRange >::~RangeLookup() 
{
} /* end RangeLookup destructor */


} //  namespace SymtabAPI
} // namespace Dyninst
#endif
