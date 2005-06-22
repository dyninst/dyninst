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

#include "RangeLookup.t"

LineInformation::LineInformation() : 
	RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >(),
	sourceLineNames() {
	} /* end LineInformation constructor */

bool LineInformation::addLine( const char * lineSource, unsigned int lineNo, Address lowInclusiveAddr, Address highExclusiveAddr ) {
	/* If we haven't already, intern the lineSource. */
	if( lineSource == NULL ) { return false; }
	
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
	
	return addValue( LineNoTuple( lineSourceInternal, lineNo ), lowInclusiveAddr, highExclusiveAddr );
	} /* end setLineToAddressRangeMapping() */
                                                                                                                                    
bool LineInformation::addAddressRange( Address lowInclusiveAddr, Address highExclusiveAddr, const char * lineSource, unsigned int lineNo ) {
	return addLine( lineSource, lineNo, lowInclusiveAddr, highExclusiveAddr );
	} /* end setAddressRangeToLineMapping() */

bool LineInformation::getSourceLines( Address addressInRange, std::vector< LineNoTuple > & lines ) {
	return getValues( addressInRange, lines );
	} /* end getLinesFromAddress() */

bool LineInformation::getAddressRanges( const char * lineSource, unsigned int lineNo, std::vector< AddressRange > & ranges ) {
	return RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >::getAddressRanges( LineNoTuple( lineSource, lineNo ), ranges );
	} /* end getAddressRangesFromLine() */

LineInformation::const_iterator LineInformation::begin() const {
	return RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >::begin();
	} /* end begin() */
	
LineInformation::const_iterator LineInformation::end() const {
	return RangeLookup< LineInformationImpl::LineNoTuple, LineInformationImpl::LineNoTupleLess >::end();
	} /* end begin() */

bool LineInformationImpl::LineNoTupleLess::operator () ( LineNoTuple lhs, LineNoTuple rhs ) const {
	if( strcmp( lhs.first, rhs.first ) < 0 ) { return true; }
	else if( strcmp( lhs.first, rhs.first ) == 0 ) {
		if( lhs.second < rhs.second ) { return true; }
		else{ return false; }
		}
	else{ return false; }
	} /* end LineNoTupleLess() */

#if ! defined( os_windows )
bool LineInformation::SourceLineCompare::operator () ( const char * lhs, const char * rhs ) const {
	return strcmp( lhs, rhs ) == 0;
	} /* end SourceLineCompare() */
#else
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
