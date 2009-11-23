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

// $Id: inst-ia64.h,v 1.18 2008/02/23 02:09:05 jaw Exp $

#ifndef INST_IA64_H
#define INST_IA64_H

#include "common/h/Types.h"	// Address
class AddressSpace;
class IA64_bundle;
class IA64_instruction;

/* Stores locations of pre-baseTramp registers */
#define BP_GR0          0		//   0-127
#define BP_GR127		(BP_GR0 + 127)
#define BP_BR0          128		// 128-135
#define BP_BR7			(BP_BR0 + 7)
#define BP_PR           136		// 136
#define BP_AR0          137		// 137-264
#define BP_AR63			(BP_AR0 + 63)
#define BP_AR64			(BP_AR0 + 64)
#define BP_AR127		(BP_AR0 + 127)
#define BP_R_MAX        (BP_AR0 + 128)

/* aliases */
#define BP_KR0          BP_AR0
#define BP_AR_CSD       (BP_AR0 + 25)
#define BP_AR_SSD       (BP_AR0 + 26)
#define BP_AR_CCV       (BP_AR0 + 32)
#define BP_AR_PFS       (BP_AR0 + 64)

/* Required for ast.C */
#define REG_MT_POS	13

class InsnAddr {
	public:
		/* prefix increment */
		InsnAddr operator++ ();

		/* prefix decrement */
		InsnAddr operator-- ();

		/* postfix increment */
		InsnAddr operator++ (int dummy);

		/* postfix decrement */
		InsnAddr operator-- (int dummy);

		/* sum of two InsnAddrs */
		friend InsnAddr operator + ( InsnAddr lhs, InsnAddr rhs );

		/* difference of two InsnAddrs */
		friend InsnAddr operator - ( InsnAddr lhs, InsnAddr rhs );

		/* Returns the left-aligned instruction at this address. */
		uint64_t operator * ();

		friend bool operator < ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator <= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator > ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator >= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator == ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator != ( InsnAddr lhs, InsnAddr rhs );

		static InsnAddr generateFromAlignedDataAddress( Address addr, AddressSpace * p );
		bool writeMyBundleFrom( const unsigned char * savedCodeBuffer );
		bool saveMyBundleTo( unsigned char * savedCodeBuffer );
		bool saveBundlesTo( unsigned char * savedCodeBuffer, unsigned int numberOfBundles );
		bool writeBundlesFrom( unsigned char * savedCodeBuffer, unsigned int numberOfBundles );
		bool replaceBundleWith( const IA64_bundle & bundle );
		bool replaceBundlesWith( const IA64_bundle * replacementBundles, unsigned int numberOfReplacementBundles );
		bool writeStringAtOffset( unsigned int offsetInBundles, const char * str, unsigned int length );

	private:
		InsnAddr( Address addr, AddressSpace * p ) : encodedAddress( addr ), myProc( p ) { }
		Address encodedAddress;
		AddressSpace * myProc;
}; /* end class InsnAddr */

#include "arch-ia64.h"

class IA64_iterator {
	/* FIXME: if/when InsnAddr and/or InstrucIter are implemented, move the
	   address-munging functionality here. */
	public:
		IA64_iterator( Address addr );

		friend bool operator < ( IA64_iterator lhs, IA64_iterator rhs );

		/* Returns the left-aligned instruction at this address. */
		IA64_instruction * operator * ();
		
		/* postfix increment */
		const IA64_iterator operator++ (int dummy);

		Address getEncodedAddress() { return encodedAddress; }

	private:
		Address encodedAddress;
		IA64_bundle currentBundle;
}; /* end class IA64_iterator */

class dyn_lwp;
/* Necessary for get and restore registers() to Do The Right Thing
   for inferior RPCs. */
bool needToHandleSyscall( dyn_lwp * lwp, bool * pcMayHaveRewound = NULL );

/* Handle machine-encoded offsets correctly. */
uint64_t signExtend( bool signBit, uint64_t immediate );

#endif
