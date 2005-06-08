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

#include "LineInformation.h"
#include <assert.h>
#include <list>

LineInformation::LineInformation() : 
	sourceLineNames(),
	lineNoTupleToAddressRangeMap(),
	rangesByAddress() {
	} /* end LineInformation constructor */

/* Private refactoring function: erase()s value from map. */
template< class M >
bool removeByValue( M & map, const typename M::value_type & value ) {
	bool removedValue = false;
#if ! defined( os_windows )
	std::pair< typename M::iterator, typename M::iterator > range = map.equal_range( value.first );	
#else
	std::pair< M::iterator, M::iterator > range = map.equal_range( value.first );	
#endif
	for( ; range.first != range.second && range.first != map.end(); ++range.first ) {
		if( * range.first == value ) {
			map.erase( range.first );
			removedValue = true;
			}
		}
	return removedValue;
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
bool LineInformation::addLine( const char * lineSource, unsigned int lineNo, Address lowInclusiveAddr, Address highExclusiveAddr ) {
	/* Verify the input. */
	if( lowInclusiveAddr >= highExclusiveAddr ) { return false; }
	if( lineSource == NULL ) { return false; }

	/* If we haven't already, intern the lineSource. */
	const char * lineSourceInternal = NULL;
	typedef SourceLineInternTable::const_iterator IteratorType;
	IteratorType found = sourceLineNames.find( lineSource );
	if( found == sourceLineNames.end() ) {
		lineSourceInternal = strdup( lineSource );
		assert( lineSourceInternal != NULL );
		sourceLineNames.insert( lineSourceInternal );
		}
	else {
		lineSourceInternal = * found;
		}
	assert( lineSourceInternal != NULL );
	
	/* If we've already got this range, it's safe to insert it. */
	typedef RangeMapByAddress::iterator RangeIterator;
	std::pair< RangeIterator, RangeIterator > rangeOfRanges = rangesByAddress.equal_range( AddressRange( lowInclusiveAddr, highExclusiveAddr ) );
	if( rangeOfRanges.first != rangeOfRanges.second || rangesByAddress.size() == 0 ) {
		/* insert() is amortized constant time if the new value is inserted immediately before the hint. */
		rangesByAddress.insert( rangeOfRanges.second, std::make_pair( AddressRange( lowInclusiveAddr, highExclusiveAddr ), LineNoTuple( lineSourceInternal, lineNo ) ) );
		lineNoTupleToAddressRangeMap.insert( std::make_pair( LineNoTuple( lineSourceInternal, lineNo ), AddressRange( lowInclusiveAddr, highExclusiveAddr ) ) ); \
		return true;
		}
		
	/* Otherwise, we need to look for containing ranges. */
	typedef std::pair< AddressRange, LineNoTuple > Range;
	typedef std::list< Range > RangeList;
	RangeIterator downIterator = rangeOfRanges.second;
	if( rangeOfRanges.second != rangesByAddress.begin() ) { --downIterator; }
	RangeList containingRangeList;
	for( ; ( downIterator->first.first <= lowInclusiveAddr && highExclusiveAddr < downIterator->first.second ); --downIterator ) {
		// /* DEBUG */ fprintf( stderr, "%s[%d]: found containing range [0x%lx, 0x%lx) while inserting [0x%lx, 0x%lx) (%s, %d)\n", __FILE__, __LINE__, downIterator->first.first, downIterator->first.second, lowInclusiveAddr, highExclusiveAddr, lineSourceInternal, lineNo );
		containingRangeList.push_back( * downIterator );
		if( downIterator == rangesByAddress.begin() ) { break; }
		}
	
	/* We also need to look for contained ranges. */	
	RangeIterator upIterator = rangeOfRanges.second;	
	RangeList containedRangeList;
	for( ;	upIterator != rangesByAddress.end() &&
			(lowInclusiveAddr <= upIterator->first.first && upIterator->first.second < highExclusiveAddr );
			++upIterator ) {
			// /* DEBUG */ fprintf( stderr, "%s[%d]: found contained range [0x%lx, 0x%lx) while inserting [0x%lx, 0x%lx) (%s, %d)\n", __FILE__, __LINE__, upIterator->first.first, upIterator->first.second, lowInclusiveAddr, highExclusiveAddr, lineSourceInternal, lineNo );
			containedRangeList.push_back( * upIterator );
		} /* end iteration looking for contained ranges */
	
	if( containingRangeList.size() == 0 && containedRangeList.size() == 0 ) {
		/* insert() is amortized constant time if the new value is inserted immediately before the hint. */
		rangesByAddress.insert( rangeOfRanges.second, std::make_pair( AddressRange( lowInclusiveAddr, highExclusiveAddr ), LineNoTuple( lineSourceInternal, lineNo ) ) );
		lineNoTupleToAddressRangeMap.insert( std::make_pair( LineNoTuple( lineSourceInternal, lineNo ), AddressRange( lowInclusiveAddr, highExclusiveAddr ) ) ); \
		return true;
		}
	
	/* TODO: combine lists; if wasContaining, splitaddrlist is just highExclusiveAddr */
	
	if( containingRangeList.size() != 0 ) {
		Address splitAddress = highExclusiveAddr;
		
		/* Remove the old (containing) ranges, split them, insert the new ones. */
		RangeList::const_iterator containingIterator = containingRangeList.begin();
		for( ; containingIterator != containingRangeList.end(); ++containingIterator ) {
			AddressRange ar = containingIterator->first;
			LineNoTuple lnt = containingIterator->second;
		
			// /* DEBUG */ fprintf( stderr, "%s[%d]: removing range [0x%lx, 0x%lx) (%s, %u)...\n", __FILE__, __LINE__, ar.first, ar.second, lnt.first, lnt.second );
			assert( removeByValue( rangesByAddress, * containingIterator ) );
			assert( removeByValue( lineNoTupleToAddressRangeMap, std::make_pair( lnt, ar ) ) );
			// /* DEBUG */ fprintf( stderr, "%s[%d]: ... done removing range [0x%lx, 0%lx) (%s, %u)\n", __FILE__, __LINE__, ar.first, ar.second, lnt.first, lnt.second );
		
			// /* DEBUG */ fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, ar.first, splitAddress, lnt.first, lnt.second );
			rangesByAddress.insert( std::make_pair( AddressRange( ar.first, splitAddress ), lnt ) );
			lineNoTupleToAddressRangeMap.insert( std::make_pair( lnt, AddressRange( ar.first, splitAddress ) ) );
		
			// /* DEBUG */ fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, splitAddress, ar.second, lnt.first, lnt.second );
			rangesByAddress.insert( std::make_pair( AddressRange( splitAddress, ar.second ), lnt ) );
			lineNoTupleToAddressRangeMap.insert( std::make_pair( lnt, AddressRange( splitAddress, ar.second ) ) );
			} /* end removes/split/insert iteration */
		
		// /* DEBUG */ fprintf( stderr, "%s[%d]: inserting new range [0x%lx, 0x%lx) (%s, %u)\n\n", __FILE__, __LINE__, lowInclusiveAddr, highExclusiveAddr, lineSourceInternal, lineNo );
		
		/* Insert the new range. */
		rangesByAddress.insert( std::make_pair( AddressRange( lowInclusiveAddr, highExclusiveAddr ), LineNoTuple( lineSourceInternal, lineNo ) ) );
		lineNoTupleToAddressRangeMap.insert( std::make_pair( LineNoTuple( lineSourceInternal, lineNo ), AddressRange( lowInclusiveAddr, highExclusiveAddr ) ) ); \
		return true;
		} /* end if the range to be inserted is contained by other ranges. */
	
	if( containedRangeList.size() != 0 ) {
		/* We'll split the ranges at these points. */
		typedef std::list< Address > AddressList;
		AddressList splitAddressList;
	
		/* Overlapping ranges overlap until a discontuity, so we only need to
		   know about one of them. */
		RangeList::const_iterator containedIterator = containedRangeList.begin();
		Address splitAddress = containedIterator->first.second;
		++containedIterator;
				
		for( ; containedIterator != containedRangeList.end(); ++containedIterator ) {
			/* Is there a discontinuity? */
			if( containedIterator->first.first >= splitAddress ) {
				// /* DEBUG */ fprintf( stderr, "%s[%d]: will split at 0x%lx\n", __FILE__, __LINE__, splitAddress );
				
				/* If there is, our current splitAddress should be a split, */
				splitAddressList.push_back( splitAddress );				
				
				/* and the high end of the next contained range will be our next split point. */
				splitAddress = containedIterator->first.second;				
				}
			else { splitAddress = containedIterator->first.second; }
			} /* end split point determination iteration */
		// /* DEBUG */ fprintf( stderr, "%s[%d]: will split at 0x%lx\n", __FILE__, __LINE__, splitAddress );
		splitAddressList.push_back( splitAddress );
		
		/* Split the range to be inserted. */
		Address lowAddress = lowInclusiveAddr;
		LineNoTuple lnt( lineSourceInternal, lineNo );
		AddressList::const_iterator splitAddressIterator = splitAddressList.begin();
		Address highAddress;
		for( ; splitAddressIterator != splitAddressList.end(); ++ splitAddressIterator ) {
			highAddress = * splitAddressIterator;
			// /* DEBUG */ fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, lowAddress, highAddress, lnt.first, lnt.second );
			rangesByAddress.insert( std::make_pair( AddressRange( lowAddress, highAddress ), lnt ) );
			lineNoTupleToAddressRangeMap.insert( std::make_pair( lnt, AddressRange( lowAddress, highAddress ) ) );
			
			lowAddress = highAddress;
			} /* end iteration to split range to be inserted. */
		// /* DEBUG */ fprintf( stderr, "%s[%d]: inserting range [0x%lx, 0x%lx) (%s, %u)\n", __FILE__, __LINE__, highAddress, highExclusiveAddr, lnt.first, lnt.second );
		rangesByAddress.insert( std::make_pair( AddressRange( highAddress, highExclusiveAddr ), LineNoTuple( lineSourceInternal, lineNo ) ) );
		lineNoTupleToAddressRangeMap.insert( std::make_pair( LineNoTuple( lineSourceInternal, lineNo ), AddressRange( highAddress, highExclusiveAddr ) ) );	
		
		/* We done did it. */
		return true;
		} /* end if the range to be inserted contains other ranges. */
		
	/* Something Terrible happened. */
	return false;
	} /* end setLineToAddressRangeMapping() */
                                                                                                                                    
bool LineInformation::addAddressRange( Address lowInclusiveAddr, Address highExclusiveAddr, const char * lineSource, unsigned int lineNo ) {
	return addLine( lineSource, lineNo, lowInclusiveAddr, highExclusiveAddr );
	} /* end setAddressRangeToLineMapping() */


/*	We maintain the invariant that the ranges sort in the same order on
	their low and high addresses.  In this case, we only need to search from
	the range whose low address is the least too large down to the range
	whose high address is the first too small to find all ranges which
	contain the desired address.	
 */	   
bool LineInformation::getSourceLines( Address addressInRange, std::vector< LineNoTuple > & lines ) {
	/* We can't find an address if we have no ranges. */
	if( rangesByAddress.size() == 0 ) { return false; }
											
	/* Because the sort order of the low and high addresses is the same, we know every range
	   which could contain addressInRange must be below the range whose low address is one
	   larger, and above the range whose high address is the same.  We use addressInRange + 1
	   to make sure we find one past the end of a span of ranges with the same low address,
	   so we decrement the iterator to point it at the first range which could contain
	   addressInRange.  If equal_range() returns end(), then decrementing the iterator
	   ensures we start checking with a real range. */
	   
	typedef RangeMapByAddress::const_iterator RangeIterator;
	std::pair< RangeIterator, RangeIterator > lowRange = rangesByAddress.equal_range( AddressRange( addressInRange + 1, 0 ) );
	assert( lowRange.first == lowRange.second );
	RangeIterator hHighEnd = --(lowRange.second);

	/* Some implementations get stuck on rangesByAddress.begin(), apparently. */
	for( ; hHighEnd->first.second > addressInRange && hHighEnd != rangesByAddress.end(); --hHighEnd ) {
		if( hHighEnd->first.first <= addressInRange && addressInRange < hHighEnd->first.second ) {
			lines.push_back( hHighEnd->second );
			}
		if( hHighEnd == rangesByAddress.begin() ) { break; }
		} /* end iteration over possible range matches. */
	if( lines.size() == 0 ) { return false; }
	
	return true;
	} /* end getLinesFromAddress() */

bool LineInformation::getAddressRanges( const char * lineSource, unsigned int lineNo, std::vector< AddressRange > & ranges ) {
	/* Look for the specified lineSource:lineNo. */
	typedef LineNoTupleToAddressRangeMap::const_iterator IteratorType;
	std::pair< IteratorType, IteratorType > range = lineNoTupleToAddressRangeMap.equal_range( LineNoTuple( lineSource, lineNo ) );

	/* If equal_range() doesn't find anything, range.first and range.second will be equal. */
	if( range.first == range.second ) { return false; }

	/* Otherwise, copy out the found ranges. */
	for( IteratorType i = range.first; i != range.second; ++i ) {
		ranges.push_back( i->second );
		} /* end iteration over located address ranges. */

	return true;
	} /* end getAddressRangesFromLine() */

LineInformation::const_iterator LineInformation::begin() const {
	return rangesByAddress.begin();
	} /* end begin() */
	
LineInformation::const_iterator LineInformation::end() const {
	return rangesByAddress.end();
	} /* end begin() */

bool LineInformation::LineNoTupleLess::operator () ( LineNoTuple lhs, LineNoTuple rhs ) const {
	if( strcmp( lhs.first, rhs.first ) < 0 ) { return true; }
	else if( strcmp( lhs.first, rhs.first ) == 0 ) {
		if( lhs.second < rhs.second ) { return true; }
		else{ return false; }
		}
	else{ return false; }
	} /* end LineNoTupleLess() */

bool LineInformation::AddressRangeLess::operator () ( AddressRange lhs, AddressRange rhs ) const {
	if( lhs.first < rhs.first ) { return true; }
	else if( lhs.first == rhs.first ) {
		if( lhs.second < rhs.second ) { return true; }
		else{ return false; }
		}
	else{ return false; }
	} /* end AddressRangeLess() */

bool LineInformation::SourceLineCompare::operator () ( const char * lhs, const char * rhs ) const {
	return strcmp( lhs, rhs ) == 0;
	} /* end SourceLineCompare() */

#if defined( os_windows )
bool LineInformation::SourceLineLess::operator () ( const char * lhs, const char * rhs ) const {
	return strcmp( lhs, rhs ) < 0;
	} /* end SourceLineLess() */
#endif

/* We free the strings we allocated, and let the compiler clean up everything else:

   Section 10.4.6 [Stroustroup's C++]: "When a class object containing class
   objects is destroyed, the body of that object's own destructor is executed first,
   and then the members' destructors are executed in the reverse order of declaration." */
LineInformation::~LineInformation() {
	/* Apparently, the iterator depends on the hash of its current key
	   to continue.  This should probably be cached, to allow me to free
	   the current key if it's a pointer (and the hash over the pointed-to
	   data), but I guess it's not strictly a bug. */
	   
	const char * internedString = NULL;
	SourceLineInternTable::iterator iterator = sourceLineNames.begin();

	while( iterator != sourceLineNames.end() ) {
		internedString = * iterator;
		++iterator;
		free( const_cast< char * >( internedString ) );
		}	
	} /* end LineInformation destructor */

/* - - - - - - - - - - - - - - - - -  DEBUG  - - - - - - - - - - - - - - - - */
void LineInformation::dump( FILE * stream ) {
	RangeMapByAddress::iterator iterator = rangesByAddress.begin();
	for( ; iterator != rangesByAddress.end(); ++iterator ) {
		fprintf( stream, "\t[0x%lx,\t0x%lx)\t%s:%u\n", iterator->first.first, iterator->first.second, iterator->second.first, iterator->second.second );
		} fprintf( stream, "\n" );
	} /* end dumpMapping() */

#if ! defined( os_windows )                    

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#define RANGE_COUNT 0x000FFFFF
void LineInformation::testInsertionSpeed() {
	typedef RangeMapByAddress::iterator RangeIterator;
	
	RangeMapByAddress slow, fast, nolookup;
	struct timeval start, end;
	unsigned long difference;
	std::pair< RangeIterator, RangeIterator > rangeOfRanges;
	
	srand( time( NULL ) );
	int * low = (int *)malloc( sizeof( int ) * RANGE_COUNT );
	assert( low != NULL );
	int * high = (int *)malloc( sizeof( int ) * RANGE_COUNT );
	assert( high != NULL );
	for( int i = 0; i < RANGE_COUNT; i++ ) {
		low[i] = rand();
		high[i] = rand();
		if( low[i] >= high[i] ) { high[i] = low[i] + 1; }
		}

	assert( gettimeofday( & start, NULL ) == 0 );
	for( unsigned int i = 0; i < RANGE_COUNT; ++i ) {
		nolookup.insert( make_pair( AddressRange( low[i], high[i] ), LineNoTuple( "foo", 3 ) ) );
		}
	assert( gettimeofday( & end, NULL ) == 0 );
	difference = (end.tv_sec - start.tv_sec) * 1000000;
	difference += (end.tv_usec - start.tv_usec);
	difference = difference / 1000;
	fprintf( stderr, "%s[%d]: noolokup took %lu milliseconds\n", __FILE__, __LINE__, difference );	

	assert( gettimeofday( & start, NULL ) == 0);
	for( unsigned int i = 0; i < RANGE_COUNT; ++i ) {
		rangeOfRanges = fast.equal_range( AddressRange( low[i], high[i] ) );
		fast.insert( rangeOfRanges.second, make_pair( AddressRange( low[i], high[i] ), LineNoTuple( "foo", 3 ) ) );
		}
	assert( gettimeofday( & end, NULL ) == 0 );
	difference = (end.tv_sec - start.tv_sec) * 1000000;
	difference += (end.tv_usec - start.tv_usec);
	difference = difference / 1000;
	fprintf( stderr, "%s[%d]: fast took %lu milliseconds\n", __FILE__, __LINE__, difference );

	assert( gettimeofday( & start, NULL ) == 0 );
	for( unsigned int i = 0; i < RANGE_COUNT; ++i ) {
		rangeOfRanges = slow.equal_range( AddressRange( low[i], high[i] ) );
		slow.insert( make_pair( AddressRange( low[i], high[i] ), LineNoTuple( "foo", 3 ) ) );
		}
	assert( gettimeofday( & end, NULL ) == 0 );
	difference = (end.tv_sec - start.tv_sec) * 1000000;
	difference += (end.tv_usec - start.tv_usec);
	difference = difference / 1000;
	fprintf( stderr, "%s[%d]: slow took %lu milliseconds\n", __FILE__, __LINE__, difference );
	
	assert( gettimeofday( & start, NULL ) == 0 );
	for( unsigned int i = 0; i < RANGE_COUNT; ++i ) {
		rangeOfRanges = slow.equal_range( AddressRange( low[i], high[i] ) );
		}
	assert( gettimeofday( & end, NULL ) == 0 );
	difference = (end.tv_sec - start.tv_sec) * 1000000;
	difference += (end.tv_usec - start.tv_usec);
	difference = difference / 1000;
	fprintf( stderr, "%s[%d]: lookup-only took %lu milliseconds\n", __FILE__, __LINE__, difference );
	} /* end testInsertionSpeed() */
	
#else

void LineInformation::testInsertionSpeed() {
	fprintf( stderr, "%s[%d]: not implemented on Windows.\n", __FILE__, __LINE__ );
	} /* end testInsertionSpeed() */

#endif
