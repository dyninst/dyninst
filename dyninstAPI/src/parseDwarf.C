/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

/*
 * Copyright (c) 2003 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#include "common/h/Types.h"

#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "libdwarf.h"

#include "BPatch_module.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "symtab.h"
#include "process.h"

/* For location decode. */
#include <stack>

/* A bound attribute can be either (a constant) or (a reference
   to a DIE which (describes an object containing the bound value) or
   (a constant value itself)). */
bool decipherBound( Dwarf_Debug & dbg, Dwarf_Attribute boundAttribute, char ** boundString ) {
	Dwarf_Half boundForm;
	int status = dwarf_whatform( boundAttribute, & boundForm, NULL );
	assert( status == DW_DLV_OK );

	switch( boundForm ) {
		case DW_FORM_data1:
		case DW_FORM_data2:
		case DW_FORM_data4:
		case DW_FORM_data8:
		case DW_FORM_sdata:
		case DW_FORM_udata: {
			Dwarf_Unsigned constantBound;
			status = dwarf_formudata( boundAttribute, & constantBound, NULL );
			assert( status == DW_DLV_OK );

			* boundString = (char *)calloc( 12, sizeof( char ) );
			assert( * boundString != NULL );
			snprintf( * boundString, 12, "%lu", (unsigned long)constantBound );
			return true;
			} break;

		case DW_FORM_ref_addr:
		case DW_FORM_ref1:
		case DW_FORM_ref2:
		case DW_FORM_ref4:
		case DW_FORM_ref8:
		case DW_FORM_ref_udata: {
			/* Acquire the referenced DIE. */
			Dwarf_Off boundOffset;
			status = dwarf_global_formref( boundAttribute, & boundOffset, NULL );
			assert( status == DW_DLV_OK );

			Dwarf_Die boundEntry;
			status = dwarf_offdie( dbg, boundOffset, & boundEntry, NULL );
			assert( status == DW_DLV_OK );

			/* Does it have a name? */
			char * boundName;
			status = dwarf_diename( boundEntry, & boundName, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				* boundString = strdup( boundName );
				assert( * boundString != NULL );

				dwarf_dealloc( dbg, boundName, DW_DLA_STRING );
				return true;
				}
			dwarf_dealloc( dbg, boundName, DW_DLA_STRING );

			/* Does it describe a nameless constant? */
			Dwarf_Attribute constBoundAttribute;
			status = dwarf_attr( boundEntry, DW_AT_const_value, & constBoundAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned constBoundValue;
				status = dwarf_formudata( constBoundAttribute, & constBoundValue, NULL );
				assert( status == DW_DLV_OK );

				* boundString = (char *)calloc( 12, sizeof( char ) );
				assert( * boundString != NULL );
				snprintf( * boundString, 12, "%lu", (unsigned long)constBoundValue );

				dwarf_dealloc( dbg, constBoundAttribute, DW_DLA_ATTR );
				dwarf_dealloc( dbg, boundEntry, DW_DLA_DIE );
				return true;
				}

			dwarf_dealloc( dbg, constBoundAttribute, DW_DLA_ATTR );
			return false;
			} break;

		default:
			fprintf( stderr, "Invalid bound form 0x%x\n", boundForm );
			* boundString = "{invalid bound form}";
			return false;
			break;
		} /* end boundForm switch */
	} /* end decipherBound() */

/* We don't have a sane way of dealing with DW_TAG_enumeration bounds, so
   just put the name of the enumeration, or {enum} if none, in the string. */
void parseSubRangeDIE( Dwarf_Debug & dbg, Dwarf_Die subrangeDIE, char ** loBound, char ** hiBound, BPatch_module * module ) {
	assert( loBound != NULL ); * loBound = "{unknown or default}";
	assert( hiBound != NULL ); * hiBound = "{unknown or default}";

	/* Set the default lower bound, if we know it. */
	switch( module->getLanguage() ) {
		case BPatch_fortran:
		case BPatch_fortran90:
			* loBound = "1";
			break;
		case BPatch_c:
		case BPatch_cPlusPlus:
			* loBound = "0";
			break;
		default:
			break;
		} /* end default lower bound switch */

	Dwarf_Half subrangeTag;
	int status = dwarf_tag( subrangeDIE, & subrangeTag, NULL );
	assert( status == DW_DLV_OK );

	/* Could be an enumerated range. */
	if( subrangeTag == DW_TAG_enumeration_type ) {
		/* FIXME? First child of enumeration type is lowest, last is highest. */
		char * enumerationName = NULL;
		status = dwarf_diename( subrangeDIE, & enumerationName, NULL );
		assert( status != DW_DLV_ERROR );

		if( enumerationName != NULL ) {
			* loBound = strdup( enumerationName );
			assert( * loBound != NULL );
			* hiBound = strdup( enumerationName );
			assert( * hiBound != NULL );
			} else {
			* loBound = strdup( "{nameless enum lo}" );
			assert( * loBound != NULL );
			* hiBound = strdup( "{nameless enum hi}" );
			assert( * hiBound != NULL );
			}
		dwarf_dealloc( dbg, enumerationName, DW_DLA_STRING );
		return;
		} /* end if an enumeration type */

	/* Is a subrange type. */
	assert( subrangeTag == DW_TAG_subrange_type );

	/* Look for the lower bound. */
	Dwarf_Attribute lowerBoundAttribute;
	status = dwarf_attr( subrangeDIE, DW_AT_lower_bound, & lowerBoundAttribute, NULL );
	assert( status != DW_DLV_ERROR );

	if( status == DW_DLV_OK ) {
		decipherBound( dbg, lowerBoundAttribute, loBound );
		dwarf_dealloc( dbg, lowerBoundAttribute, DW_DLA_ATTR );
		} /* end if we found a lower bound. */

	/* Look for the upper bound. */
	Dwarf_Attribute upperBoundAttribute;
	status = dwarf_attr( subrangeDIE, DW_AT_upper_bound, & upperBoundAttribute, NULL );
	assert( status != DW_DLV_ERROR );
	if( status == DW_DLV_NO_ENTRY ) {
		status = dwarf_attr( subrangeDIE, DW_AT_count, & upperBoundAttribute, NULL );
		assert( status != DW_DLV_ERROR );
		}

	if( status == DW_DLV_OK ) {
		decipherBound( dbg, upperBoundAttribute, hiBound );
		dwarf_dealloc( dbg, upperBoundAttribute, DW_DLA_ATTR );
		} /* end if we found an upper bound or count. */

	/* Construct the range type. */
	char * subrangeName = "{anonymous range}"; // BPatch_type doesn't like NULL.
	status = dwarf_diename( subrangeDIE, & subrangeName, NULL );
	assert( status != DW_DLV_ERROR );
	int dwarvenName = status;

	Dwarf_Off subrangeOffset;
	status = dwarf_dieoffset( subrangeDIE, & subrangeOffset, NULL );
	assert( status != DW_DLV_ERROR );

	BPatch_type * rangeType = new BPatch_type( subrangeName, subrangeOffset, BPatchSymTypeRange, * loBound, * hiBound );
	assert( rangeType != NULL );
	// fprintf( stderr, "Adding range type '%s' (%lu) [%s, %s]\n", subrangeName, (unsigned long)subrangeOffset, * loBound, * hiBound );
	rangeType = module->moduleTypes->addOrUpdateType( rangeType );
	if( dwarvenName == DW_DLV_OK ) { dwarf_dealloc( dbg, subrangeName, DW_DLA_STRING ); }
	} /* end parseSubRangeDIE() */

/* We don't use the array constructor for BPatch_type arrays
   because DWARF arrays may be bounded by enumerations.  However,
   this means the size of these arrays are never set correctly.
   This corrects that problem, when it can.

   FIXME? when bounds are enumerations, let the size be the
   the number of elements in the enumeration. */
void setArraySize( BPatch_type * arrayType, const char * loBound, const char * hiBound ) {
	char * endPtr = NULL;
	long longLoBound = strtol( loBound, &endPtr, 0 );
	if( endPtr == loBound ) { arrayType->setSize( 0 ); return; }
	long longHiBound = strtol( hiBound, & endPtr, 0 );
	if( endPtr == hiBound ) { arrayType->setSize( 0 ); return; }

	long longCount = longHiBound - longLoBound + 1;
	arrayType->setSize( longCount * arrayType->getConstituentType()->getSize() );
	} /* end setArraySize() */

BPatch_type * parseMultiDimensionalArray( Dwarf_Debug & dbg, Dwarf_Die range, BPatch_type * elementType, BPatch_module * module ) {
	/* Get the (negative) typeID for this range/subarray. */
	Dwarf_Off dieOffset;
	int status = dwarf_dieoffset( range, & dieOffset, NULL );
	assert( status == DW_DLV_OK );

	/* Determine the range. */
	char * loBound = NULL;
	char * hiBound = NULL;
	parseSubRangeDIE( dbg, range, & loBound, & hiBound, module );

	/* Does the recursion continue? */
	Dwarf_Die nextSibling;
	status = dwarf_siblingof( dbg, range, & nextSibling, NULL );
	assert( status != DW_DLV_ERROR );

	if( status == DW_DLV_NO_ENTRY ) {
		/* Terminate the recursion by building an array type out of the elemental type.
		   Use the negative dieOffset to avoid conflicts with the range type created
		   by parseSubRangeDIE(). */
		BPatch_type * innermostType = new BPatch_type( NULL, (-1) * dieOffset, BPatch_dataArray, elementType, 0, 0 );
		assert( innermostType != NULL );
		innermostType->setLow( loBound );
		innermostType->setHigh( hiBound );
		setArraySize( innermostType, loBound, hiBound );
		// fprintf( stderr, "Adding inner-most type %lu\n", (unsigned long) dieOffset );
		innermostType = module->moduleTypes->addOrUpdateType( innermostType );
		return innermostType;
		} /* end base-case of recursion. */

	/* If it does, build this array type out of the array type returned from the next recusion. */
	BPatch_type * innerType = parseMultiDimensionalArray( dbg, nextSibling, elementType, module );
	assert( innerType != NULL );
	BPatch_type * outerType = new BPatch_type( NULL, (-1) * dieOffset, BPatch_dataArray, innerType, 0, 0 );
	outerType->setLow( loBound );
	outerType->setHigh( hiBound );
	setArraySize( outerType, loBound, hiBound );
	assert( outerType != NULL );
	// fprintf( stderr, "Adding inner type %lu\n", (unsigned long) dieOffset );
	outerType = module->moduleTypes->addOrUpdateType( outerType );

	dwarf_dealloc( dbg, nextSibling, DW_DLA_DIE );
	return outerType;
	} /* end parseMultiDimensionalArray() */

void deallocateLocationList( Dwarf_Debug & dbg, Dwarf_Locdesc * locationList, Dwarf_Signed listLength ) {
	for( int i = 0; i < listLength; i++ ) {
		dwarf_dealloc( dbg, locationList[i].ld_s, DW_DLA_LOC_BLOCK );
		}
	dwarf_dealloc( dbg, locationList, DW_DLA_LOCDESC );
	} /* end deallocateLocationList() */

/* An investigative function. */
void dumpLocListAddrRanges( Dwarf_Locdesc * locationList, Dwarf_Signed listLength ) {
	for( int i = 0; i < listLength; i++ ) {
		Dwarf_Locdesc location = locationList[i];
		fprintf( stderr, "0x%lx to 0x%lx; ", (Address)location.ld_lopc, (Address)location.ld_hipc );
		}
	fprintf( stderr, "\n" );
	} /* end dumpLocListAddrRanges */

AstNode * convertFrameBaseToAST( Dwarf_Locdesc * locationList, Dwarf_Signed listLength ) {
	/* Until such time as we see more-complicated location lists, assume single entries
	   consisting of a register name.  Using an AST for this is massive overkill, but if
	   we need to handle more complicated frame base calculations later, the infastructure
	   will be in place. */
	   
	/* There is only one location. */
	assert( listLength == 1 );
	Dwarf_Locdesc locationDescriptor = locationList[0];
	
	/* It is defined by a single operation. */
	assert( locationDescriptor.ld_cents == 1 );
	Dwarf_Loc location = locationDescriptor.ld_s[0];

	/* That operation is naming a register. */
	int registerNumber = 0;	
	if( DW_OP_reg0 <= location.lr_atom && location.lr_atom <= DW_OP_reg31 ) {
		registerNumber = location.lr_atom - DW_OP_reg0;
		}
	else if( location.lr_atom == DW_OP_regx ) {
		registerNumber = location.lr_number;
		}
	else {
		assert( 0 );
		}

	/* We have to make sure no arithmetic is actually done to the frame pointer,
	   so add zero to it and shove it in some other register. */
	AstNode * constantZero = new AstNode( AstNode::Constant, (void *)0 );
	assert( constantZero != NULL );
	AstNode * framePointer = new AstNode( AstNode::DataReg, (void *)(long unsigned int)registerNumber );
	assert( framePointer != NULL );
	AstNode * moveFPtoDestination = new AstNode( orOp, constantZero, framePointer );
	
	return moveFPtoDestination;
	} /* end convertLocListToAST(). */

bool decodeLocationListForStaticOffsetOrAddress( Dwarf_Locdesc * locationList, Dwarf_Signed listLength, long int * offset, long int * initialStackValue = NULL, bool * isFrameRelative = NULL ) {
	/* We make a few heroic assumptions about locations in this decoder.
	
	   We assume that all locations are either frame base-relative offsets,
	   encoded with DW_OP_fbreg, or are absolute addresses.  We assume these
	   locations are invariant with respect to the PC, which implies that all
	   location lists have a single entry.  We assume that no location is
	   calculated at run-time.
	   
	   We make these assumptions to match the assumptions of the rest of
	   Dyninst, which makes no provision for pc-variant or run-time calculated
	   locations, aside from the frame pointer.  However, it assumes that a frame
	   pointer is readily available, which, on IA-64, it is not.  For that reason,
	   when we encounter a function with a DW_AT_frame_base (effectively all of them),
	   we do NOT use this decoder; we decode the location into an AST, which we
	   will use to calculate the frame pointer when asked to do frame-relative operations.
	   (These calculations will be invalid until the frame pointer is established,
	   which may require some fiddling with the location of the 'entry' instpoint.) */
	assert( listLength == 1 );

	if( isFrameRelative != NULL ) { * isFrameRelative = false; }

	/* Initialize the stack. */
	std::stack< long int > opStack = std::stack<long int>();
	if( initialStackValue != NULL ) { opStack.push( * initialStackValue ); }

	/* There is only one location. */
	Dwarf_Locdesc location = locationList[0];
	Dwarf_Loc * locations = location.ld_s;
	for( unsigned int i = 0; i < location.ld_cents; i++ ) {
		/* Handle the literals w/o 32 case statements. */
		if( DW_OP_lit0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_lit31 ) {
			// fprintf( stderr, "Pushing named constant: %d\n", locations[i].lr_atom - DW_OP_lit0 );
			opStack.push( locations[i].lr_atom - DW_OP_lit0 );
			continue;
			}

		switch( locations[i].lr_atom ) {
			case DW_OP_addr:
			case DW_OP_const1u:
			case DW_OP_const2u:
			case DW_OP_const4u:
			case DW_OP_const8u:
			case DW_OP_constu:
				// fprintf( stderr, "Pushing constant %lu\n", (unsigned long)locations[i].lr_number );
				opStack.push( (Dwarf_Unsigned)locations[i].lr_number );
				break;

			case DW_OP_const1s:
			case DW_OP_const2s:
			case DW_OP_const4s:
			case DW_OP_const8s:
			case DW_OP_consts:
				// fprintf( stderr, "Pushing constant %ld\n", (signed long)(locations[i].lr_number) );
				opStack.push( (Dwarf_Signed)(locations[i].lr_number) );
				break;

			case DW_OP_fbreg:
				if( isFrameRelative != NULL ) { * isFrameRelative = true; }
				opStack.push( (Dwarf_Signed)(locations[i].lr_number) );
				break;

			case DW_OP_dup:
				opStack.push( opStack.top() );
				break;

			case DW_OP_drop:
				opStack.pop();
				break;

			case DW_OP_pick: {
				/* Duplicate the entry at index locations[i].lr_number. */
				std::stack< long int > temp = std::stack< long int >();
				for( unsigned int i = 0; i < locations[i].lr_number; i++ ) {
					temp.push( opStack.top() ); opStack.pop();
					}
				long int dup = opStack.top();
				for( unsigned int i = 0; i < locations[i].lr_number; i++ ) {
					opStack.push( temp.top() ); temp.pop();
					}
				opStack.push( dup );
				} break;

			case DW_OP_over: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second ); opStack.push( first ); opStack.push( second );
				} break;

			case DW_OP_swap: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first ); opStack.push( second );
				} break;

			case DW_OP_rot: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				long int third = opStack.top(); opStack.pop();
				opStack.push( first ); opStack.push( third ); opStack.push( second );
				} break;

			case DW_OP_abs: {
				long int top = opStack.top(); opStack.pop();
				opStack.push( abs( top ) );
				} break;

			case DW_OP_and: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second & first );
				} break;
			
			case DW_OP_div: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second / first );
				} break;

			case DW_OP_minus: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second - first );
				} break;

			case DW_OP_mod: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second % first );
				} break;

			case DW_OP_mul: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second * first );
				} break;

			case DW_OP_neg: {
				long int first = opStack.top(); opStack.pop();
				opStack.push( first * (-1) );
				} break;

			case DW_OP_not: {
				long int first = opStack.top(); opStack.pop();
				opStack.push( ~ first );
				} break;

			case DW_OP_or: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second | first );
				} break;

			case DW_OP_plus: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second + first );
				} break;

			case DW_OP_plus_uconst: {
				long int first = opStack.top(); opStack.pop();
				opStack.push( first + locations[i].lr_number );
				} break;

			case DW_OP_shl: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second << first );
				} break;

			case DW_OP_shr: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( (long int)((unsigned long)second >> (unsigned long)first) );
				} break;

			case DW_OP_shra: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second >> first );
				} break;

			case DW_OP_xor: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second ^ first );
				} break;

			case DW_OP_le: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first <= second ? 1 : 0 );
				} break;

			case DW_OP_ge: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first >= second ? 1 : 0 );
				} break;

			case DW_OP_eq: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first == second ? 1 : 0 );
				} break;

			case DW_OP_lt: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first < second ? 1 : 0 );
				} break;

			case DW_OP_gt: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first > second ? 1 : 0 );
				} break;

			case DW_OP_ne: {
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first != second ? 1 : 0 );
				} break;

			case DW_OP_bra:
				if( opStack.top() == 0 ) { break; }
				opStack.pop();
			case DW_OP_skip: {
				int bytes = (int)(Dwarf_Signed)locations[i].lr_number;
				unsigned int target = locations[i].lr_offset + bytes;

				int j = i;
				if( bytes < 0 ) {
					for( j = i - 1; j >= 0; j-- ) {
						if( locations[j].lr_offset == target ) { break; }
						} /* end search backward */
					} else {
					for( j = i + 1; j < location.ld_cents; j ++ ) {
						if( locations[j].lr_offset == target ) { break; }
						} /* end search forward */
					} /* end if positive offset */

				/* Because i will be incremented the next time around the loop. */
				i = j - 1;
				} break;

			case DW_OP_piece:
				/* For multi-part variables, which we don't handle. */
				fprintf( stderr, "Warning: dyninst does not handle multi-part variables.\n" );
				break;

			case DW_OP_nop:
				break;

			default:
				/* FIXME: register names for formal parameters in libstdc++? */
				// fprintf( stderr, "Unrecognized or non-static location opcode 0x%x, aborting.\n", locations[i].lr_atom );
				return false;
			} /* end operand switch */
		} /* end iteration over Dwarf_Loc entries. */

	/* The top of the stack is the computed location. */
	// fprintf( stderr, "Location decoded: %ld\n", opStack.top() );
	* offset = opStack.top();
	
	/* decode successful */
	return true;
	} /* end decodeLocationListForStaticOffsetOrAddress() */

void convertFileNoToName( Dwarf_Debug & dbg, Dwarf_Signed fileNo, char ** returnFileName, char ** newFileNames = NULL, Dwarf_Signed newFileNamesCount = 0 ) {
	static char ** fileNames = NULL;
	static Dwarf_Signed fileNamesCount = 0;

	/* Initialize? */
	if( returnFileName == NULL && newFileNames != NULL ) {
		/* FIXME?  Did we want to normalize these filenames? */
		fileNames = newFileNames;
		fileNamesCount = newFileNamesCount;
		return;
		} /* end initialization. */

	/* Destroy? */
	if( returnFileName == NULL && newFileNames == NULL ) {
		for( int i = 0; i < fileNamesCount; i++ ) {
			dwarf_dealloc( dbg, fileNames[i], DW_DLA_STRING );
			} /* end deallocation loop */
		dwarf_dealloc( dbg, fileNames, DW_DLA_LIST );
		fileNamesCount = 0;
		return;
		} /* end destruction. */

	/* Do lookup. */
	if( fileNo <= fileNamesCount ) { * returnFileName = fileNames[fileNo - 1]; }
	else { * returnFileName = NULL; }
	} /* end convertFileNoToName() */
	
/* Utility function. */
unsigned long tvDifference( struct timeval lhs, struct timeval rhs ) {
	unsigned long seconds = lhs.tv_sec - rhs.tv_sec;
	if( seconds == 0 ) { return lhs.tv_usec - rhs.tv_usec; }
	else {
		seconds *= 1000000;
		seconds += lhs.tv_usec - rhs.tv_usec;
		}
	return seconds;
	} /* end tvDifference() */

/* For debugging. */
void dumpAttributeList( Dwarf_Die dieEntry, Dwarf_Debug & dbg ) {
	char * entryName = NULL;
	int status = dwarf_diename( dieEntry, & entryName, NULL );
	assert( status != DW_DLV_ERROR );

	Dwarf_Attribute * attributeList;
	Dwarf_Signed attributeCount;
	status = dwarf_attrlist( dieEntry, & attributeList, & attributeCount, NULL );
	assert( status != DW_DLV_ERROR );

	fprintf( stderr, "DIE %s has attributes:", entryName );
	for( int i = 0; i < attributeCount; i++ ) {
		Dwarf_Half whatAttr = 0;
		status = dwarf_whatattr( attributeList[i], & whatAttr, NULL );
		assert( status != DW_DLV_ERROR );
		fprintf( stderr, " 0x%x", whatAttr );
		
		dwarf_dealloc( dbg, attributeList[i], DW_DLA_ATTR );
		} /* end iteration over attributes */
	fprintf( stderr, "\n" );
	
	dwarf_dealloc( dbg, attributeList, DW_DLA_LIST );
	dwarf_dealloc( dbg, entryName, DW_DLA_STRING );
	} /* end dumpAttributeList() */

bool walkDwarvenTree(	Dwarf_Debug & dbg, char * moduleName, Dwarf_Die dieEntry,
			BPatch_module * module, 
			BPatch_function * currentFunction = NULL,
			BPatch_type * currentCommonBlock = NULL,
			BPatch_type * currentEnclosure = NULL ) {
	Dwarf_Half dieTag;
	int status = dwarf_tag( dieEntry, & dieTag, NULL );
	assert( status == DW_DLV_OK );
	
	Dwarf_Off dieOffset;
	status = dwarf_dieoffset( dieEntry, & dieOffset, NULL );
	assert( status == DW_DLV_OK );
	// fprintf( stderr, "Considering DIE at %lu with tag 0x%x\n", (unsigned long)dieOffset, dieTag );

	/* If this entry is a function, common block, or structure (class),
	   its children will be in its scope, rather than its
	   enclosing scope. */
	BPatch_function * newFunction = currentFunction;
	BPatch_type * newCommonBlock = currentCommonBlock;
	BPatch_type * newEnclosure = currentEnclosure;

	bool parsedChild = false;
	/* Is this is an entry we're interested in? */
	switch( dieTag ) {
		/* case DW_TAG_inline_subroutine: we don't care about these */
		case DW_TAG_subprogram:
		case DW_TAG_entry_point: {
			/* Is this entry specified elsewhere?  We may need to look there for its name. */
			Dwarf_Bool hasSpecification;
			status = dwarf_hasattr( dieEntry, DW_AT_specification, & hasSpecification, NULL );
			assert( status == DW_DLV_OK );

			/* Our goal is three-fold: First, we want to set the return type
			   of the function.  Second, we want to set the newFunction variable
			   so subsequent entries are handled correctly.  Third, we want to
			   record (the location of, or how to calculte) the frame base of 
			   this function for use by our instrumentation code later. */

			char * functionName = NULL;
			Dwarf_Die specEntry = dieEntry;

			/* In order to do this, we need to find the function's (mangled) name.
			   If a function has a specification, its specification will have its
			   name. */
			if( hasSpecification ) {
				Dwarf_Attribute specAttribute;
				status = dwarf_attr( dieEntry, DW_AT_specification, & specAttribute, NULL );
				assert( status == DW_DLV_OK );

				Dwarf_Off specOffset;
				status = dwarf_global_formref( specAttribute, & specOffset, NULL );
				assert( status == DW_DLV_OK );

				status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
				} /* end if the function has a specification */

			/* Prefer linkage names. */
			Dwarf_Attribute linkageNameAttr;
			status = dwarf_attr( specEntry, DW_AT_MIPS_linkage_name, & linkageNameAttr, NULL );
			assert( status != DW_DLV_ERROR );

			bool hasLinkageName;
			if( status == DW_DLV_OK ) {
				hasLinkageName = true;
				status = dwarf_formstring( linkageNameAttr, & functionName, NULL );
				assert( status == DW_DLV_OK );
				
				dwarf_dealloc( dbg, linkageNameAttr, DW_DLA_ATTR );
				} /* end if there's a linkage name. */
			else {
				hasLinkageName = false;
				status = dwarf_diename( specEntry, & functionName, NULL );
				assert( status != DW_DLV_ERROR );
				} /* end if there isn't a linkage name. */

			if( functionName == NULL ) {
				/* I'm not even sure what an anonymous function _means_,
				   but we sure can't do anything with it. */
				// fprintf( stderr, "Warning: anonymous function (type %lu).\n", (unsigned long)dieOffset );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
				} /* end if there's no name at all. */

			/* Try to find the function. */
			BPatch_Vector< BPatch_function * > functions = BPatch_Vector< BPatch_function * >();
			module->findFunction( functionName, functions, false );

			if( functions.size() == 0 ) {
				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
				}
			else if( functions.size() > 1 ) {
				// fprintf( stderr, "Warning: found more than one function '%s', unable to do anything reasonable.\n", functionName );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;		
				}
			else {
				newFunction = functions[0];
				} /* end findFunction() cases */

			/* Once we've found the BPatch_function pointer corresponding to this
			   DIE, record its frame base.  A declaration entry may not have a 
			   frame base, and some functions do not have frames. */
			Dwarf_Attribute frameBaseAttribute;
			status = dwarf_attr( dieEntry, DW_AT_frame_base, & frameBaseAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				Dwarf_Locdesc * locationList;
				Dwarf_Signed listLength;
				status = dwarf_loclist( frameBaseAttribute, & locationList, & listLength, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, frameBaseAttribute, DW_DLA_ATTR );
					
#if defined(ia64_unknown_linux2_4)
				/* Convert location list to an AST for later code generation. */
				newFunction->func->framePointerCalculator = convertFrameBaseToAST( locationList, listLength );
#endif
				
				deallocateLocationList( dbg, locationList, listLength );
				} /* end if this DIE has a frame base attribute */
			
			/* Find its return type. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			BPatch_type * returnType = NULL;
			if( status == DW_DLV_NO_ENTRY ) { 
				returnType = module->moduleTypes->findType( "void" );
				if( returnType == NULL ) {
					/* Go ahead and create a void type. */
					returnType = new BPatch_type( "void", 0, BPatch_built_inType, 0 );
					assert( returnType != NULL );
					returnType = module->moduleTypes->addOrUpdateType( returnType );
					}
				newFunction->setReturnType( returnType );
				} /* end if the return type is void */
			else {
				/* There's a return type attribute. */
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );

				returnType = module->moduleTypes->findOrCreateType( typeOffset );
				newFunction->setReturnType( returnType );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if not a void return type */

			/* If this is a member function, add it as a field, for backward compatibility */
			if( currentEnclosure != NULL ) {
				/* Using the mangled name allows us to distinguish between overridden
				   functions, but confuses the tests.  Since BPatch_type uses vectors
				   to hold field names, however, duplicate -- demangled names -- are OK. */
				char * demangledName = P_cplus_demangle( functionName, module->isNativeCompiler() );
				assert( demangledName != NULL );
								
				/* Strip everything left of the rightmost ':' off; see above. */
				char * leftMost = strrchr( demangledName, ':' );

				currentEnclosure->addField( leftMost + 1, BPatch_dataMethod, returnType, 0, 0 );
				free( demangledName );
				}

			if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
			dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
			} break;

		case DW_TAG_common_block: {
			char * commonBlockName;
			status = dwarf_diename( dieEntry, & commonBlockName, NULL );
			assert( status == DW_DLV_OK );

			BPatch_image * wholeProgram = (BPatch_image *) module->getObjParent();
			BPatch_variableExpr * commonBlockVar = 
					wholeProgram->findVariable( commonBlockName );

			BPatch_type * commonBlockType = NULL;
			if( commonBlockVar != NULL ) {
				commonBlockType = (BPatch_type *)commonBlockVar->getType();
				} /* end if there might be a pre-existing type */

			if( commonBlockType == NULL || commonBlockType->getDataClass() == BPatch_dataCommon ) {
				commonBlockType = new BPatch_type( commonBlockName, false );
				assert( commonBlockType != NULL );
				commonBlockVar->setType( commonBlockType );
				module->moduleTypes->addGlobalVariable( commonBlockName, commonBlockType );
				commonBlockType->setDataClass( BPatch_dataCommon );
				} /* end if we re-define the type. */

			dwarf_dealloc( dbg, commonBlockName, DW_DLA_STRING );

			/* This node's children are in the common block. */
			newCommonBlock = commonBlockType;
			newCommonBlock->beginCommonBlock();
			} break;

		case DW_TAG_constant: {
			fprintf( stderr, "Warning: dyninst ignores named constant entries.\n" );
			} break;
		
		/* It's worth noting that a variable may have a constant value.  Since,
		   AFAIK, Dyninst does nothing with this information, neither will we.
		   (It will, however, explain why certain variables that otherwise would
		   don't have locations.) */
		case DW_TAG_variable: {
			/* A variable may occur inside a function, as either static or local.
			   A variable may occur inside a container, as C++ static member.
			   A variable may not occur in either, as a global. 
			   
			   For the first two cases, we need the variable's name, its type,
			   its line number, and its offset or address in order to tell
			   Dyninst about it.  Dyninst only needs to know the name and type
			   of a global.  (Actually, it already knows the names of the globals;
			   we're really just telling it the types.)
			   
			   Variables may have two entries, the second, with a _specification,
			   being the only one with the location. */

			/* We will begin by determining which kind of variable we think it is,
			   and the determining if we need to wait for the DIE with a _specification
			   before telling Dyninst about it. */
			
			if( currentFunction == NULL && currentEnclosure == NULL ) {
				/* Then this variable must be a global.  Acquire its name and type. */
				char * variableName;
				status = dwarf_diename( dieEntry, & variableName, NULL );
				assert( status != DW_DLV_ERROR );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				assert( status == DW_DLV_OK );
				
				Dwarf_Attribute typeAttribute;
				status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
				assert( status != DW_DLV_ERROR );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				assert( status == DW_DLV_OK );
				
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );
				
				/* The typeOffset forms a module-unique type identifier,
				   so the BPatch_type look-ups by it rather than name. */
				BPatch_type * variableType = module->moduleTypes->findOrCreateType( typeOffset );
				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				
				/* Tell Dyninst what this global's type is. */
				module->moduleTypes->addGlobalVariable( variableName, variableType );
				} /* end if this variable is a global */
			else {
				/* We'll start with the location, since that's most likely to
				   require the _specification. */
				Dwarf_Attribute locationAttribute;
				status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
				assert( status != DW_DLV_ERROR );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				assert( status == DW_DLV_OK );
				
				Dwarf_Locdesc * locationList;
				Dwarf_Signed listLength;
				status = dwarf_loclist( locationAttribute, & locationList, & listLength, NULL );
				assert( status == DW_DLV_OK );			
				
				dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );

				bool isFrameRelative;
				long int variableOffset;
				bool decodedAddressOrOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & variableOffset, NULL, & isFrameRelative );
				deallocateLocationList( dbg, locationList, listLength );
				
				if( ! decodedAddressOrOffset ) { break; }
				assert( decodedAddressOrOffset );
			
				/* If this DIE has a _specification, use that for the rest of our inquiries. */
				Dwarf_Die specEntry = dieEntry;
				
				Dwarf_Attribute specAttribute;
				status = dwarf_attr( dieEntry, DW_AT_specification, & specAttribute, NULL );
				assert( status != DW_DLV_ERROR );
				
				if( status == DW_DLV_OK ) {
					Dwarf_Off specOffset;
					status = dwarf_global_formref( specAttribute, & specOffset, NULL );
					assert( status == DW_DLV_OK );
					
					status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );
					assert( status == DW_DLV_OK );
				
					dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
					} /* end if dieEntry has a _specification */
					
				/* Acquire the name, type, and line number. */
				char * variableName;
				status = dwarf_diename( specEntry, & variableName, NULL );
				assert( status != DW_DLV_ERROR );
				
				/* We can't do anything with an anonymous variable. */
				if( status == DW_DLV_NO_ENTRY ) { break; }
				assert( status == DW_DLV_OK );

				/* Acquire the parameter's type. */
				Dwarf_Attribute typeAttribute;
				status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
				assert( status != DW_DLV_ERROR );
			
				if( status == DW_DLV_NO_ENTRY ) { break; }
				assert( status == DW_DLV_OK );
			
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );
			
				/* The typeOffset forms a module-unique type identifier,
				   so the BPatch_type look-ups by it rather than name. */
				BPatch_type * variableType = module->moduleTypes->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
				/* Acquire the parameter's lineNo. */
				Dwarf_Unsigned variableLineNo;
				bool hasLineNumber = false;

				Dwarf_Attribute lineNoAttribute;
				status = dwarf_attr( specEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
				assert( status != DW_DLV_ERROR );
			
				/* We don't need to tell Dyninst a line number for C++ static variables,
				   so it's OK if there isn't one. */
				if( status == DW_DLV_OK ) {
					hasLineNumber = true;
					status = dwarf_formudata( lineNoAttribute, & variableLineNo, NULL );
					assert( status == DW_DLV_OK );
					
					dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );			
					} /* end if there is a line number */
							
				/* We now have the variable name, type, offset, and line number.
				   Tell Dyninst about it. */
				   
				if( currentFunction != NULL ) {
					if( !hasLineNumber ) { break; }
					assert( hasLineNumber );
					
					BPatch_localVar * newVariable = new BPatch_localVar( variableName, variableType, variableLineNo, variableOffset, 5, isFrameRelative );
					currentFunction->localVariables->addLocalVar( newVariable );
					} /* end if a local or static variable. */
				else if( currentEnclosure != NULL ) {
					assert( ! isFrameRelative );
					currentEnclosure->addField( variableName, variableType->getDataClass(), variableType, variableOffset, variableType->getSize() );
					} /* end if a C++ static member. */
				} /* end if this variable is not global */
			} break;
		
		/* It's probably worth noting that a formal parameter may have a
		   default value.  Since, AFAIK, Dyninst does nothing with this information,
		   neither will we. */
		case DW_TAG_formal_parameter: {
			/* A formal parameter must occur in the context of a function.
			   (That is, we can't do anything with a formal parameter to a
			   function we don't know about.) */
			if( currentFunction == NULL ) { break; }
			assert( currentFunction != NULL );
			
			/* We need the formal parameter's name, its type, its line number,
			   and its offset from the frame base in order to tell the 
			   rest of Dyninst about it.  A single _formal_parameter
			   DIE may not have all of this information; if it does not,
			   we will ignore it, hoping to catch it when it is later
			   referenced as an _abstract_origin from another _formal_parameter
			   DIE.  If we never find such a DIE, than there is not enough
			   information to introduce it to Dyninst. */
			
			/* We begin with the location, since this is the attribute
			   most likely to require using the _abstract_origin. */
			Dwarf_Bool hasLocation = false;
			status = dwarf_hasattr( dieEntry, DW_AT_location, & hasLocation, NULL );
			assert( status == DW_DLV_OK );
			
			if( !hasLocation ) { break; }
			assert( hasLocation );
		
			/* Acquire the location of this formal parameter. */
			Dwarf_Attribute locationAttribute;
			status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
			assert( status == DW_DLV_OK );
			
			Dwarf_Locdesc * locationList;
			Dwarf_Signed listLength;
			status = dwarf_loclist( locationAttribute, & locationList, & listLength, NULL );
			assert( status == DW_DLV_OK );
			
			dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );
			
			bool isFrameRelative;
			long int parameterOffset;
			bool decodedOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & parameterOffset, NULL, & isFrameRelative );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedOffset ) { break; }
			assert( decodedOffset );
			
			assert( isFrameRelative );
		
			/* If the DIE has an _abstract_origin, we'll use that for the
			   remainder of our inquiries. */
			Dwarf_Die originEntry = dieEntry;
		
			Dwarf_Attribute originAttribute;
			status = dwarf_attr( dieEntry, DW_AT_abstract_origin, & originAttribute, NULL );
			assert( status != DW_DLV_ERROR );
			
			if( status == DW_DLV_OK ) {
				Dwarf_Off originOffset;
				status = dwarf_global_formref( originAttribute, & originOffset, NULL );
				assert( status == DW_DLV_OK );
				
				status = dwarf_offdie( dbg, originOffset, & originEntry, NULL );
				assert( status == DW_DLV_OK );
				
				dwarf_dealloc( dbg, originAttribute, DW_DLA_ATTR );
				} /* end if the DIE has an _abstract_origin */
			
			/* Acquire the parameter's name. */
			char * parameterName;
			status = dwarf_diename( originEntry, & parameterName, NULL );
			assert( status != DW_DLV_ERROR );
			
			/* We can't do anything with anonymous parameters. */
			if( status == DW_DLV_NO_ENTRY ) { break; }
			assert( status == DW_DLV_OK );
			
			/* Acquire the parameter's type. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( originEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status != DW_DLV_ERROR );
			
			if( status == DW_DLV_NO_ENTRY ) { break; }
			assert( status == DW_DLV_OK );
			
			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			assert( status == DW_DLV_OK );
			
			/* The typeOffset forms a module-unique type identifier,
			   so the BPatch_type look-ups by it rather than name. */
			BPatch_type * parameterType = module->moduleTypes->findOrCreateType( typeOffset );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			/* Acquire the parameter's lineNo. */
			Dwarf_Attribute lineNoAttribute;
			status = dwarf_attr( originEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
			assert( status != DW_DLV_ERROR );
			
			if( status == DW_DLV_NO_ENTRY ) { break; }
			assert( status == DW_DLV_OK );
			
			Dwarf_Unsigned parameterLineNo;
			status = dwarf_formudata( lineNoAttribute, & parameterLineNo, NULL );
			assert( status == DW_DLV_OK );
			
			dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );
			
			/* We now have the parameter's location, name, type, and line number.
			   Tell Dyninst about it. */
			BPatch_localVar * newParameter = new BPatch_localVar( parameterName, parameterType, parameterLineNo, parameterOffset, 5, true );
			assert( newParameter != NULL );
			
			/* This is just brutally ugly.  Why don't we take care of this invariant automatically? */
			currentFunction->funcParameters->addLocalVar( newParameter );
			currentFunction->addParam( parameterName, parameterType, parameterLineNo, parameterOffset );
			
			// /* DEBUG */ fprintf( stderr, "Added formal parameter '%s' (at FP + %ld) of type %p from line %lu.\n", parameterName, parameterOffset, parameterType, (unsigned long)parameterLineNo );
			} break;

		case DW_TAG_base_type: {
			/* What's the type's name? */
			char * typeName;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			assert( status == DW_DLV_OK );

			/* How big is it? */
			Dwarf_Attribute byteSizeAttr;
			Dwarf_Unsigned byteSize;
			status = dwarf_attr( dieEntry, DW_AT_byte_size, & byteSizeAttr, NULL );
			assert( status == DW_DLV_OK );
			status = dwarf_formudata( byteSizeAttr, & byteSize, NULL );
			assert( status == DW_DLV_OK );

			dwarf_dealloc( dbg, byteSizeAttr, DW_DLA_ATTR );
			
			/* Generate the appropriate built-in type; since there's no
			   reliable way to distinguish between a built-in and a scalar,
			   we don't bother to try. */
			BPatch_type * baseType = new BPatch_type( typeName, dieOffset, BPatch_dataBuilt_inType, byteSize );
			assert( baseType != NULL );

			/* Add the basic type to our collection. */
			// fprintf( stderr, "Adding base type '%s' (%lu) of size %lu\n\n", typeName, (unsigned long)dieOffset, (unsigned long)byteSize );
			baseType = module->moduleTypes->addOrUpdateType( baseType );
			
			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_typedef: {
			char * definedName = NULL;
			status = dwarf_diename( dieEntry, & definedName, NULL );
			assert( status == DW_DLV_OK );

			BPatch_type * referencedType = NULL;
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status != DW_DLV_ERROR );
			if( status == DW_DLV_NO_ENTRY ) {
				/* According to the DWARF spec, "A declaration of the type that is not also a definition."
				   This includes constructions like "typedef void _IO_lock_t", from libio.h, which
				   cause us to issue a lot of true but spurious-looking warnings about incomplete types.
				   So instead of ignoring this entry, point it to the void type.  (This is also more
				   in line with our handling of absent DW_AT_type tags everywhere else.) */
                referencedType = module->moduleTypes->findType( "void" );
                if( referencedType == NULL ) {
                    /* Go ahead and create a void type. */
                    referencedType = new BPatch_type( "void", 0, BPatch_built_inType, 0 );
                    assert( referencedType != NULL );
                    referencedType = module->moduleTypes->addOrUpdateType( referencedType );
				    }
				}
			else {
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );

				/* Look up the referenced type. */
				referencedType = module->moduleTypes->findOrCreateType( typeOffset );
				}

			/* Add the typedef to our collection. */
			// fprintf( stderr, "Adding typedef: '%s' as %lu (pointing to %lu)\n", definedName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			BPatch_type * typedefType = new BPatch_type( definedName, dieOffset, BPatch_typeDefine, referencedType );
			typedefType = module->moduleTypes->addOrUpdateType( typedefType );

			/* Sanity check: typedefs should not have children. */
			Dwarf_Die childDwarf;
			status = dwarf_child( dieEntry, & childDwarf, NULL );
			assert( status == DW_DLV_NO_ENTRY );
			
			dwarf_dealloc( dbg, definedName, DW_DLA_STRING );
			} break;

		case DW_TAG_array_type: {
			char * arrayName = NULL;
			status = dwarf_diename( dieEntry, & arrayName, NULL );
			assert( status != DW_DLV_ERROR );

			/* Find the type of the elements. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status == DW_DLV_OK );
			
			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			assert( status == DW_DLV_OK );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			BPatch_type * elementType = module->moduleTypes->findOrCreateType( typeOffset );

			/* Find the range(s) of the elements. */
			Dwarf_Die firstRange;
			status = dwarf_child( dieEntry, & firstRange, NULL );
			assert( status == DW_DLV_OK );
			BPatch_type * baseArrayType = parseMultiDimensionalArray( dbg, firstRange, elementType, module );
			assert( baseArrayType != NULL );

			/* The baseArrayType is an anonymous type with its own typeID.  Extract
			   the information and add an array type for this DIE. */
			BPatch_type * arrayType = new BPatch_type( arrayName, dieOffset, 
				BPatch_dataArray, baseArrayType->getConstituentType(), 0, 0 );
			assert( arrayType != NULL );
			arrayType->setLow( baseArrayType->getLow() );
			arrayType->setHigh( baseArrayType->getHigh() );
			setArraySize( arrayType, baseArrayType->getLow(), baseArrayType->getHigh() );
			// fprintf( stderr, "Adding array type '%s' (%lu) [%s, %s] @ %p\n", arrayName, (unsigned long)dieOffset, baseArrayType->getLow(), baseArrayType->getHigh(), arrayType );
			arrayType = module->moduleTypes->addOrUpdateType( arrayType );

			/* Don't parse the children again. */
			parsedChild = true;

			dwarf_dealloc( dbg, firstRange, DW_DLA_DIE );
			dwarf_dealloc( dbg, arrayName, DW_DLA_STRING );
			} break;

		case DW_TAG_subrange_type: {
			char * loBound;
			char * hiBound;
			parseSubRangeDIE( dbg, dieEntry, & loBound, & hiBound, module );
			} break;

		case DW_TAG_enumeration_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			assert( status != DW_DLV_ERROR );

			BPatch_type * enumerationType = new BPatch_type( typeName, dieOffset, BPatch_dataEnumerated );
			assert( enumerationType != NULL );
			enumerationType = module->moduleTypes->addOrUpdateType( enumerationType );
			newEnclosure = enumerationType;

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_inheritance: {
			/* Acquire the super class's type. */
			Dwarf_Attribute scAttr;
			status = dwarf_attr( dieEntry, DW_AT_type, & scAttr, NULL );
			assert( status == DW_DLV_OK );

			Dwarf_Off scOffset;
			status = dwarf_global_formref( scAttr, & scOffset, NULL );
			assert( status == DW_DLV_OK );

			dwarf_dealloc( dbg, scAttr, DW_DLA_ATTR );

			BPatch_type * superClass = module->moduleTypes->findOrCreateType( scOffset );

			/* Acquire the visibility, if any.  DWARF calls it accessibility
			   to distinguish it from symbol table visibility. */
			Dwarf_Attribute visAttr;
			status = dwarf_attr( dieEntry, DW_AT_accessibility, & visAttr, NULL );
			assert( status != DW_DLV_ERROR );

			BPatch_visibility visibility = BPatch_private;
			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned visValue;
				status = dwarf_formudata( visAttr, & visValue, NULL );
				assert( status == DW_DLV_OK );

				switch( visValue ) {
					case DW_ACCESS_public: visibility = BPatch_public; break;
					case DW_ACCESS_protected: visibility = BPatch_protected; break;
					case DW_ACCESS_private: visibility = BPatch_private; break;
					default:
						fprintf( stderr, "Uknown visibility, ignoring.\n" );
						break;
					} /* end visibility switch */

				dwarf_dealloc( dbg, visAttr, DW_DLA_ATTR );
				} /* end if the visibility is specified. */

			/* Add a readily-recognizable 'bad' field to represent the superclass.
			   BPatch_type::getComponents() will Do the Right Thing. */
			currentEnclosure->addField( "{superclass}", BPatch_dataStructure, superClass, -1, visibility );
			} break;

		case DW_TAG_structure_type:
		case DW_TAG_union_type:
		case DW_TAG_class_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			assert( status != DW_DLV_ERROR );
			
			Dwarf_Attribute sizeAttr;
			Dwarf_Unsigned typeSize = 0;
			status = dwarf_attr( dieEntry, DW_AT_byte_size, & sizeAttr, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				status = dwarf_formudata( sizeAttr, & typeSize, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, sizeAttr, DW_DLA_ATTR );
				}

			BPatch_dataClass bpdc = BPatch_unknownType;
			switch ( dieTag ) {
				case DW_TAG_structure_type: bpdc = BPatch_dataStructure; break;
				case DW_TAG_union_type: bpdc = BPatch_dataUnion; break;
				case DW_TAG_class_type: bpdc = BPatch_dataStructure; break;
				}
			
			BPatch_type * containingType = new BPatch_type( typeName, dieOffset, bpdc, typeSize );
			assert( containingType != NULL );
			// fprintf( stderr, "Adding structure, union, or class type '%s' (%lu)\n", typeName, (unsigned long)dieOffset );
			containingType = module->moduleTypes->addOrUpdateType( containingType );
			newEnclosure = containingType;
			
			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;
			
		case DW_TAG_enumerator: {
			/* An entry in an enumeration. */
			char * enumName = NULL;
			status = dwarf_diename( dieEntry, & enumName, NULL );
			assert( status == DW_DLV_OK );

			Dwarf_Attribute valueAttr;
			status = dwarf_attr( dieEntry, DW_AT_const_value, & valueAttr, NULL );
			assert( status != DW_DLV_ERROR );
		
			Dwarf_Signed enumValue = 0;
			if( status == DW_DLV_OK ) {
				status = dwarf_formsdata( valueAttr, & enumValue, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, valueAttr, DW_DLA_ATTR );
				}

			// fprintf( stderr, "Adding enum '%s' (%ld) to enumeration '%s' (%d)\n", enumName, (signed long)enumValue, currentEnclosure->getName(), currentEnclosure->getID() );
			currentEnclosure->addField( enumName, BPatch_dataScalar, enumValue );

			dwarf_dealloc( dbg, enumName, DW_DLA_STRING );
			} break;

		case DW_TAG_member: {
			char * memberName = NULL;
			status = dwarf_diename( dieEntry, & memberName, NULL );
			assert( status != DW_DLV_ERROR );

			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status == DW_DLV_OK );

			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			assert( status == DW_DLV_OK );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			BPatch_type * memberType = module->moduleTypes->findOrCreateType( typeOffset );
				
			Dwarf_Attribute locationAttr;
			status = dwarf_attr( dieEntry, DW_AT_data_member_location, & locationAttr, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_NO_ENTRY ) { break; }
			assert( status == DW_DLV_OK );

			Dwarf_Locdesc * locationList;
			Dwarf_Signed listLength;
			status = dwarf_loclist( locationAttr, & locationList, & listLength, NULL );
			assert( status == DW_DLV_OK );

			dwarf_dealloc( dbg, locationAttr, DW_DLA_ATTR );

			bool isFrameRelative;
			long int memberOffset = 0;
			long int baseAddress = 0;
			bool decodedAddress = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & memberOffset, & baseAddress, & isFrameRelative );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedAddress ) { break; }
			assert( decodedAddress );
			
			assert( isFrameRelative == false );

			// fprintf( stderr, "Adding member to enclosure '%s' (%d): '%s' with type %lu at %ld and size %d\n", currentEnclosure->getName(), currentEnclosure->getID(), memberName, (unsigned long)typeOffset, memberOffset, memberType->getSize() );

			/* DWARF stores offsets in bytes unless the member is a bit field.
			   Correct memberOffset as indicated.  Also, memberSize is in bytes
			   from the underlying type, not the # of bits used from it, so
			   correct that as necessary as well. */
			long int memberSize = memberType->getSize();

			Dwarf_Attribute bitOffset;
			status = dwarf_attr( dieEntry, DW_AT_bit_offset, & bitOffset, NULL );
			assert( status != DW_DLV_ERROR );

			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned memberOffset_du = memberOffset;
				status = dwarf_formudata( bitOffset, &memberOffset_du, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, bitOffset, DW_DLA_ATTR );

				Dwarf_Attribute bitSize;
				status = dwarf_attr( dieEntry, DW_AT_bit_size, & bitSize, NULL );
				assert( status == DW_DLV_OK );

				Dwarf_Unsigned memberSize_du = memberSize;
				status = dwarf_formudata( bitSize, &memberSize_du, NULL );
				assert( status == DW_DLV_OK );

				dwarf_dealloc( dbg, bitSize, DW_DLA_ATTR );

				/* If the DW_AT_byte_size field exists, there's some padding.
				   FIXME?  We ignore padding for now.  (We also don't seem to handle
				   bitfields right in getComponents() anyway...) */
				}
			else {
				memberOffset *= 8;
				memberSize *= 8;
				} /* end if not a bit field member. */

			// fprintf( stderr, "Adding member '%s' to enclosure '%s'\n", memberName, currentEnclosure->getName() );
			currentEnclosure->addField( memberName, memberType->getDataClass(), memberType, memberOffset, memberSize );

			dwarf_dealloc( dbg, memberName, DW_DLA_STRING );
			} break;

		case DW_TAG_const_type:
		case DW_TAG_packed_type:
		case DW_TAG_volatile_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			assert( status != DW_DLV_ERROR );

			/* Which type does it modify? */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );

			int typeSize = 0;
			Dwarf_Off typeOffset;
			BPatch_type * typeModified = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typeModified = module->moduleTypes->findType( "void" );
				if( typeModified == NULL ) {
					/* Go ahead and create a void type. */
					typeModified = new BPatch_type( "void", 0, BPatch_built_inType, 0 );
					assert( typeModified != NULL );
					typeModified = module->moduleTypes->addOrUpdateType( typeModified );
					typeSize = 0;
					}
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );
				typeModified = module->moduleTypes->findOrCreateType( typeOffset );
				typeSize = typeModified->getSize();

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typeModified is not void */

			BPatch_type * modifierType = new BPatch_type( typeName, dieOffset, BPatch_typeAttrib, typeSize, typeModified, dieTag );
			assert( modifierType != NULL );
			// fprintf( stderr, "Adding modifier type '%s' (%lu) modifying (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			modifierType = module->moduleTypes->addOrUpdateType( modifierType );

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_subroutine_type:
			/* If the pointer specifies argument types, this DIE has
			   children of those types. */
		case DW_TAG_ptr_to_member_type:
		case DW_TAG_pointer_type:
		case DW_TAG_reference_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			assert( status != DW_DLV_ERROR );

			/* To which type does it point? */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			assert( status != DW_DLV_ERROR );

			Dwarf_Off typeOffset = 0;
			BPatch_type * typePointedTo = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typePointedTo = module->moduleTypes->findType( "void" );
				if( typePointedTo == NULL ) {
					/* Go ahead and create a void type. */
					typePointedTo = new BPatch_type( "void", 0, BPatch_built_inType, 0 );
					assert( typePointedTo != NULL );
					typePointedTo = module->moduleTypes->addOrUpdateType( typePointedTo );
					}
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				assert( status == DW_DLV_OK );
				typePointedTo = module->moduleTypes->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typePointedTo is not void */

			BPatch_type * indirectType = new BPatch_type( typeName, dieOffset, BPatch_dataPointer, typePointedTo );
			assert( indirectType != NULL );
			// fprintf( stderr, "Adding indirect type '%s' (%lu) pointing to (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			indirectType = module->moduleTypes->addOrUpdateType( indirectType );

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_variant_part:
			/* We don't support this (Pascal) type. */
		case DW_TAG_string_type:
			/* We don't support this (FORTRAN) type. */
		default:
			/* Nothing of interest. */
			// fprintf( stderr, "Entry %lu with tag 0x%x ignored.\n", (unsigned long)dieOffset, dieTag );
			break;
		} /* end dieTag switch */

	/* Recurse to its child, if any. */
	Dwarf_Die childDwarf;
	status = dwarf_child( dieEntry, & childDwarf, NULL );
	assert( status != DW_DLV_ERROR );

	if( status == DW_DLV_OK && parsedChild == false ) {
		walkDwarvenTree( dbg, moduleName, childDwarf, module, newFunction, newCommonBlock, newEnclosure );

		dwarf_dealloc( dbg, childDwarf, DW_DLA_DIE );
		}

	/* Recurse to its first sibling, if any. */
	Dwarf_Die siblingDwarf;
	status = dwarf_siblingof( dbg, dieEntry, & siblingDwarf, NULL );
	assert( status != DW_DLV_ERROR );

	if( status == DW_DLV_OK ) {
		/* Siblings should all use the same function, common block, and enclosure. */
		walkDwarvenTree( dbg, moduleName, siblingDwarf, module, currentFunction, currentCommonBlock, currentEnclosure );

		dwarf_dealloc( dbg, siblingDwarf, DW_DLA_DIE );
		}

	/* When would we return false? :) */
	return true;
	} /* end walkDwarvenTree() */

extern void pd_dwarf_handler( Dwarf_Error, Dwarf_Ptr );

void BPatch_module::parseDwarfTypes() {
    // fprintf( stderr, "Parsing module '%s'\n", mod->fileName().c_str() );

	/* Get the Object object. */
	image * moduleImage = mod->exec();
	assert( moduleImage != NULL );
	const Object & moduleObject = moduleImage->getObject();

	/* Start the dwarven debugging. */
	const char * fileName = moduleObject.getFileName();
	// fprintf( stderr, "Parsing object '%s'\n", fileName );

	Dwarf_Debug dbg;

	int fd = open( fileName, O_RDONLY );
	assert( fd != -1 );
	int status = dwarf_init( fd, DW_DLC_READ, & pd_dwarf_handler, moduleObject.getErrFunc(), & dbg, NULL );
	assert( status == DW_DLV_OK );

	/* Iterate over the compilation-unit headers. */
	Dwarf_Unsigned hdr;

	while( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & hdr, NULL ) == DW_DLV_OK ) {
		/* Obtain the module DIE. */
		Dwarf_Die moduleDIE;
		status = dwarf_siblingof( dbg, NULL, &moduleDIE, NULL);
		assert( status == DW_DLV_OK );

		/* Make sure we've got the right one. */
		Dwarf_Half moduleTag;
		status = dwarf_tag( moduleDIE, & moduleTag, NULL);
		assert( status == DW_DLV_OK );
		assert( moduleTag == DW_TAG_compile_unit );

		/* Extract the name of this module. */
		char * moduleName;
		status = dwarf_diename( moduleDIE, & moduleName, NULL );
		if( status == DW_DLV_NO_ENTRY ) {
			moduleName = strdup( "{ANONYMOUS}" );
			assert( moduleName != NULL );
			}
		assert( status != DW_DLV_ERROR );
		// fprintf( stderr, "%s[%d]: Considering compilation unit '%s'\n", 
		//	 __FILE__, __LINE__, moduleName );

		/* Set the language, if any. */
		Dwarf_Attribute languageAttribute;
		status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );
		assert( status != DW_DLV_ERROR );

		if( status == DW_DLV_OK ) {
			Dwarf_Unsigned languageConstant;
			status = dwarf_formudata( languageAttribute, & languageConstant, NULL );
			assert( status == DW_DLV_OK );

			switch( languageConstant ) {
				case DW_LANG_C:
				case DW_LANG_C89:
					setLanguage( BPatch_c );
					break;
				case DW_LANG_C_plus_plus:
					setLanguage( BPatch_cPlusPlus );
					break;
				case DW_LANG_Fortran77:
					setLanguage( BPatch_fortran );
					break;
				case DW_LANG_Fortran90:
					setLanguage( BPatch_fortran90 );
					break;
				default:
					/* We know what the language is but don't care. */
					setLanguage( BPatch_unknownLanguage );
					break;
				} /* end languageConstant switch */

			dwarf_dealloc( dbg, languageAttribute, DW_DLA_ATTR );
		} else {
			setLanguage( BPatch_unknownLanguage );
			} /* end language detection */

		/* Iterate over the tree rooted here. */
		assert( walkDwarvenTree( dbg, moduleName, moduleDIE, this ) );
		
		dwarf_dealloc( dbg, moduleDIE, DW_DLA_ATTR );
		dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
		} /* end iteration over compilation-unit headers. */

	/* Clean up. */
	Elf * dwarfElf;
	status = dwarf_get_elf( dbg, & dwarfElf, NULL );
	assert( status == DW_DLV_OK );

	status = dwarf_finish( dbg, NULL );
	assert( status == DW_DLV_OK );
	close( fd );

	/* Run a sanity check. */
	assert (moduleTypes);

	int tid;
	BPatch_type * bptype;
	dictionary_hash_iter<int, BPatch_type *> titer( moduleTypes->typesByID );
	while( titer.next( tid, bptype ) ){
		if( bptype->getDataClass() == BPatch_dataUnknownType ) {
			if( bptype->getConstituentType() != NULL ) {
				/* Forward-referenced typedef's have unknown dataClasses but non-NULL
				   constituents.  Correct their dataClass and move on. */
				bptype->setDataClass( bptype->getConstituentType()->getDataClass() );
				}
			else {
				fprintf( stderr, "Warning: type information may be incomplete (#%d).\n", bptype->getID() );
				}
			} /* end if the datatype is unknown. */
		} /* end iteration over moduleTypes */
	} /* end parseDwarfTypes() */


#if defined(os_linux) && defined(arch_x86)

typedef struct dwarfEHFrame_t
{
   Elf *elf;
   Dwarf_Debug dbg;
   Dwarf_Cie *cie_data;
   Dwarf_Signed cie_element_count;
   Dwarf_Fde *fde_data;
   Dwarf_Signed fde_element_count;
   Address text_start;
   char *buffer;
} dwarfEHFrame_t;

static void patchVSyscallImage(char *mem_image, size_t image_size);
void cleanupVsysinfo(void *ehf);

/**
 * Linux loads a small elf shared object into the address space
 * (the vsyscall DSO) to make faster system calls.  This elf object
 * contains an .eh_frame section which contains dwarf unwind information
 * for stack walking out of the system call.  
 *
 * This function reads a process' vsyscall DSO, parses the unwind information,
 *  and returns a structure that can be passed to getRegValueAtFrame.  
 *
 * This assumes that the address of the DSO has already been calculated and
 *  can be gotten with p->getVsyscallStart()
 **/
void *parseVsyscallPage(char *buffer, unsigned dso_size, process *p)
{
   dwarfEHFrame_t *eh_frame = NULL;
   int iresult;
   Dwarf_Error err = 0;

   eh_frame = (dwarfEHFrame_t *) calloc(1, sizeof(dwarfEHFrame_t));
   assert(eh_frame);

   eh_frame->buffer = buffer;
   eh_frame->text_start = p->getVsyscallText();

   //Fix up some linux bugs before we load the image.
   patchVSyscallImage(eh_frame->buffer, dso_size);   

   //Get a handle to the dwarf object
   eh_frame->elf = elf_memory(eh_frame->buffer, dso_size);
   iresult = dwarf_elf_init(eh_frame->elf, DW_DLC_READ, pd_dwarf_handler, 
                            NULL, &eh_frame->dbg, &err);

   if (iresult != DW_DLV_OK) {
	 pdstring msg = "libdwarf failed to open the VSyscall DSO\n";
	 showErrorCallback(130, msg);
	 goto err_handler;
   }

   //Get the .eh_frame information from the image
   iresult = dwarf_get_fde_list_eh(eh_frame->dbg, &eh_frame->cie_data, 
	   &eh_frame->cie_element_count, &eh_frame->fde_data,
	   &eh_frame->fde_element_count, &err);				 
   if (iresult != DW_DLV_OK) {
	 fprintf(stderr, "Couldn't get fde list\n");
	 goto err_handler;   
   }
   
   return (void *) eh_frame;

 err_handler:   
   if (eh_frame != NULL)
     cleanupVsysinfo((void *) eh_frame);
   return NULL;
}

/**
 * Given an address in the program, use stack walking debug data to find
 * the value of a register.
 *
 * 'ebf' is a structure returned from parseVsyscallPage
 * 'pc' is the address of the program counter in this frame.
 * 'reg' is a dwarf register id refering to the register we want 
 *   the value of.  
 * 'reg_map' is an array of integers that map dwarf register id's to 
 *   register values.  Example: on x86 the %esp register has a dwarf id of
 *   4, so reg_map[4] should contain the current value of %esp. 
 * 'error' is set to true if an error occurs.
 **/
Address getRegValueAtFrame(void *ehf, Address pc, int reg, int *reg_map,
                           process *p, bool *error)
{
   int result;
   Dwarf_Fde fde;
   Dwarf_Addr low, hi;
   Dwarf_Error err = 0;
   Dwarf_Signed offset_relevant, target_reg, offset;
   Dwarf_Half registr;
   dwarfEHFrame_t *eh_frame = (dwarfEHFrame_t *) ehf;	  

   *error = false;
   registr = (Dwarf_Half) reg;

   //PC is an absolute address, the eh_frame info is relative
   pc -= eh_frame->text_start;

   /**
    * Get the stack unwinding information by first reading the 
    * fde at the appropriate PC, then getting the specific 
    * register rule out of that FDE.
    **/
   result = dwarf_get_fde_at_pc(eh_frame->fde_data, (Dwarf_Addr) pc, 
                                &fde, &low, &hi, &err);
   if (result != DW_DLV_OK)
   {
     *error = true;
     return 0;
   }

   result = dwarf_get_fde_info_for_reg(fde, registr, pc, &offset_relevant,
                                       &target_reg, &offset, &low, &err);
   if (result != DW_DLV_OK || target_reg == DW_FRAME_UNDEFINED_VAL)
   {
     *error = true;
     return 0;
   }

   /**
    * Translate the register rules into a value.
	*
	* The dwarf .eh_frame information provides a register and an offset.
	* The question is, do we return the value register+offset, or the 
	* value in memory at address register+offset.  The standard isn't clear
	* on when we should use which.
	*
	* From what I've seen, values of DW_FRAME_SAME_VAL and when getting
	* the CFA value involve just a calculation.  Everything else needs
	* a calculation plus a memory read.  
    **/
   if (target_reg == DW_FRAME_SAME_VAL)
   {
	 return reg_map[registr];
   }

   Address calced_value = reg_map[target_reg] + (offset_relevant ? offset : 0);
   if (registr != DW_FRAME_CFA_COL)
   {
	 p->readDataSpace((caddr_t) calced_value, sizeof(int),
					  (caddr_t) &calced_value, true);
   }
   return calced_value;
}

/**
 * Deallocate the structure returned from parseVsyscallPage
 **/
void cleanupVsysinfo(void *ehf)
{
  dwarfEHFrame_t *eh_frame = (dwarfEHFrame_t *) ehf;
  Dwarf_Error err;
  int i;

  if (eh_frame == NULL)
    return;

  //Free cie and fde data
  if (eh_frame->cie_data != NULL)
  {
    for (i = 0; i < eh_frame->cie_element_count; i++)
      dwarf_dealloc(eh_frame->dbg, eh_frame->cie_data[i], DW_DLA_CIE);
    dwarf_dealloc(eh_frame->dbg, eh_frame->cie_data, DW_DLA_LIST);
  }
  if (eh_frame->fde_data != NULL)
  {
    for (i = 0; i < eh_frame->fde_element_count; i++)
      dwarf_dealloc(eh_frame->dbg, eh_frame->fde_data[i], DW_DLA_FDE);
    dwarf_dealloc(eh_frame->dbg, eh_frame->fde_data, DW_DLA_LIST);
  }

  //Free dwarf and elf objects
  if (eh_frame->dbg != NULL)
    dwarf_finish(eh_frame->dbg, &err);
  if (eh_frame->elf != NULL)
    elf_end(eh_frame->elf);

  //Dealloc buffer and eh_frame object
  if (eh_frame->buffer != NULL)
    free(eh_frame->buffer);
  free(eh_frame);
}

/**
 * Work around for an x86 Linux 2.6 bug.  The FDEs, which map a PC value to a 
 *  unwinding rule in the VSyscall DSO don't have the correct value for
 *  the PC value.  It's supposed to be a value that's relative to the 
 *  .text section, instead it's relative to the location at which the 
 *  FDE is stored.  
 * 
 * This function iterates through each FDE and inspects the PC value,
 *  if the value appears to be relative to the FDE data structure, 
 *  we'll fix it us.  As an extra, we'll also fix absolute addresses
 *  so that they become relative (not that I've actually seen that happen).
 **/
static void patchVSyscallImage(char *mem_image, size_t image_size)
{
   Elf *elf;
   Elf32_Ehdr *ehdr;
   Elf32_Shdr *shdr;
   Elf_Scn *sec;
   Address text_start, eh_frame_start, *addr;
   unsigned eh_frame_size, eh_frame_offset, eh_offset;
   unsigned text_size;
   int *size, *type;
   char *sname, *eh_ptr;

   elf = elf_memory(mem_image, image_size);
   ehdr = elf32_getehdr(elf);
   
   /**
    * Get the starting address and size of the .text and .eh_frame
    *  sections.  Also get the file offset at which the .eh_frame
    *  is stored.
    **/
   eh_frame_size = eh_frame_offset = eh_offset = text_size =
     text_start = eh_frame_start = 0x0;
   for (sec = elf_nextscn(elf, NULL); sec != NULL; sec = elf_nextscn(elf, sec))      
   {
      shdr = elf32_getshdr(sec);
      sname = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
      if (strcmp(sname, ".text") == 0)
      {
         text_start = shdr->sh_addr;
         text_size = shdr->sh_size;
      }
      else if (strcmp(sname, ".eh_frame") == 0)
      {
         eh_frame_start = shdr->sh_addr;
         eh_frame_size = shdr->sh_size;
         eh_frame_offset = shdr->sh_offset;
      }
   }
   assert(text_start != 0x0 && eh_frame_start != 0x0);
   elf_end(elf);

   /**
    * There are two structures stored in the .eh_frame sections,
    *  FDEs and CIEs.
    *
    * The CIE structure starts out as { size, 0x0, version, ... }
    * The FDE structure starts out as { size, ptr, initial_location, ...}
    *
    * We want to modify the initial_location field of the FDEs.
    **/
   eh_offset = 0;
   eh_ptr = &(mem_image[eh_frame_offset]);
   while (eh_offset < eh_frame_size)
   {
      size = (int *) &eh_ptr[eh_offset];
      eh_offset += sizeof(int);
      type = (int *) &eh_ptr[eh_offset];
      addr = (Address *) &eh_ptr[eh_offset+sizeof(int)];

      assert(*size % sizeof(int) == 0);

      if (*type == 0x0)
      {
         //If this is a CIE then skip it
         eh_offset += *size;            
         continue;
      }

      Address cur_pos = eh_frame_start + eh_offset + 4;
      if (((signed) *addr) + cur_pos >= text_start  &&
          ((signed) *addr) + cur_pos <  text_start+text_size)
      {
         //If the address apears to be relative to the FDE data structure
         // (cur_pos) we'll fix it.
         *addr = ((signed) *addr) + cur_pos - text_start;
      }
      else if (*addr >= text_start && *addr < text_start+text_size)
      {
         //If the address appears to be absolute, we'll also fix up.
         *addr = *addr - text_start;
      }
      
      eh_offset += *size;
   }
}

#endif
