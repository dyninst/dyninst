/* -*- Mode: C; indent-tabs-mode: true; tab-width: 4 -*- */

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

#include "common/h/Types.h"

#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "libdwarf.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "BPatch_module.h"
#include "BPatch_typePrivate.h"
#include "BPatch_collections.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "symtab.h"
#include "process.h"

#include "ast.h"

/* For location decode. */
#include <stack>

#if defined(arch_x86_64)
#include "emit-x86.h"
#define DWARF_TO_MACHINE_ENC(n) \
    (code_emitter->Register_DWARFtoMachineEnc(n))
#else
#define DWARF_TO_MACHINE_ENC(n) (n)
#endif

#define DWARF_FALSE_IF(condition,...) \
	if( condition ) { bpwarn( __VA_ARGS__ ); return false; }
#define DWARF_RETURN_IF(condition,...) \
	if( condition ) { bpwarn( __VA_ARGS__ ); return; }
#define DWARF_NULL_IF(condition,...) \
	if( condition ) { bpwarn( __VA_ARGS__ ); return NULL; }

/* A bound attribute can be either (a constant) or (a reference
   to a DIE which (describes an object containing the bound value) or
   (a constant value itself)). */
bool decipherBound( Dwarf_Debug & dbg, Dwarf_Attribute boundAttribute, pdstring &boundString ) {
	Dwarf_Half boundForm;
	int status = dwarf_whatform( boundAttribute, & boundForm, NULL );
	DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: unable to decode form of bounds attribute.\n", __FILE__, __LINE__ );

	switch( boundForm ) {
		case DW_FORM_data1:
		case DW_FORM_data2:
		case DW_FORM_data4:
		case DW_FORM_data8:
		case DW_FORM_sdata:
		case DW_FORM_udata: {
			Dwarf_Unsigned constantBound;
			status = dwarf_formudata( boundAttribute, & constantBound, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: unable decode unsigned data in bounds attribute.\n", __FILE__, __LINE__ );

			boundString = pdstring((unsigned long)constantBound);
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
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: unable decode reference in bounds attribute.\n", __FILE__, __LINE__ );

			Dwarf_Die boundEntry;
			status = dwarf_offdie( dbg, boundOffset, & boundEntry, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: unable dereference DWARF pointer in bounds attribute.\n", __FILE__, __LINE__ );

			/* Does it have a name? */
			char * boundName = NULL;
			status = dwarf_diename( boundEntry, & boundName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error checking for name of bounds attribute.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
			    boundString = boundName;

				dwarf_dealloc( dbg, boundName, DW_DLA_STRING );
				return true;
				}

			/* Does it describe a nameless constant? */
			Dwarf_Attribute constBoundAttribute;
			status = dwarf_attr( boundEntry, DW_AT_const_value, & constBoundAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error checking for constant value of bounds attribute.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned constBoundValue;
				status = dwarf_formudata( constBoundAttribute, & constBoundValue, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error decoding unsigned data of bounds constant value attribute.\n", __FILE__, __LINE__ );

				boundString = pdstring((unsigned long)constBoundValue);

				dwarf_dealloc( dbg, boundEntry, DW_DLA_DIE );
				dwarf_dealloc( dbg, constBoundAttribute, DW_DLA_ATTR );
				return true;
				}

			return false;
			} break;

		case DW_FORM_block:
		case DW_FORM_block1:
		   {
			/* PGI extends DWARF to allow some bounds to be location lists.  Since we can't
			   do anything sane with them, ignore them. */
			// Dwarf_Locdesc * locationList;
			// Dwarf_Signed listLength;
			// status = dwarf_loclist( boundAttribute, & locationList, & listLength, NULL );
			boundString = "{PGI extension}";
			return false;
			} break;
		
		default:
			bperr( "Invalid bound form 0x%x\n", boundForm );
			boundString = "{invalid bound form}";
			return false;
			break;
		} /* end boundForm switch */
	} /* end decipherBound() */

/* We don't have a sane way of dealing with DW_TAG_enumeration bounds, so
   just put the name of the enumeration, or {enum} if none, in the string. */
void parseSubRangeDIE( Dwarf_Debug & dbg, Dwarf_Die subrangeDIE, pdstring & loBound, pdstring & hiBound, BPatch_module * module ) {
	loBound = "{unknown or default}";
	hiBound = "{unknown or default}";

	/* Set the default lower bound, if we know it. */
	switch( module->getLanguage() ) {
		case BPatch_fortran:
		case BPatch_fortran90:
	    case BPatch_fortran95:
			loBound = "1";
			break;
		case BPatch_c:
		case BPatch_cPlusPlus:
			loBound = "0";
			break;
		default:
			break;
		} /* end default lower bound switch */

	Dwarf_Half subrangeTag;
	int status = dwarf_tag( subrangeDIE, & subrangeTag, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: unable to obtain tag of subrange DIE.\n", __FILE__, __LINE__ );

	/* Could be an enumerated range. */
	if( subrangeTag == DW_TAG_enumeration_type ) {
		/* FIXME? First child of enumeration type is lowest, last is highest. */
		char * enumerationName = NULL;
		status = dwarf_diename( subrangeDIE, & enumerationName, NULL );
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error cehcking for name of enumeration.\n", __FILE__, __LINE__ );

		if( enumerationName != NULL ) {
			loBound = enumerationName;
			hiBound = enumerationName;
			} else {
			loBound = "{nameless enum lo}";
			hiBound = "{nameless enum hi}";
			}
		dwarf_dealloc( dbg, enumerationName, DW_DLA_STRING );
		return;
		} /* end if an enumeration type */

	/* Is a subrange type. */
	DWARF_RETURN_IF( subrangeTag != DW_TAG_subrange_type, "%s[%d]: unknown tag while parsing subrange\n", __FILE__, __LINE__ );

	/* Look for the lower bound. */
	Dwarf_Attribute lowerBoundAttribute;
	status = dwarf_attr( subrangeDIE, DW_AT_lower_bound, & lowerBoundAttribute, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for lower bound of subrange\n", __FILE__, __LINE__ );

	if( status == DW_DLV_OK ) {
		decipherBound( dbg, lowerBoundAttribute, loBound );
		dwarf_dealloc( dbg, lowerBoundAttribute, DW_DLA_ATTR );
		} /* end if we found a lower bound. */

	/* Look for the upper bound. */
	Dwarf_Attribute upperBoundAttribute;
	status = dwarf_attr( subrangeDIE, DW_AT_upper_bound, & upperBoundAttribute, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for upper bound of subrange\n", __FILE__, __LINE__ );
	if( status == DW_DLV_NO_ENTRY ) {
		status = dwarf_attr( subrangeDIE, DW_AT_count, & upperBoundAttribute, NULL );
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for count of subrange\n", __FILE__, __LINE__ );
		}
	if( status == DW_DLV_OK ) {
		decipherBound( dbg, upperBoundAttribute, hiBound );
		dwarf_dealloc( dbg, upperBoundAttribute, DW_DLA_ATTR );
		} /* end if we found an upper bound or count. */

	/* Construct the range type. */
	char * subrangeName = "{anonymous range}"; // BPatch_type doesn't like NULL.
	status = dwarf_diename( subrangeDIE, & subrangeName, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for name of subrange\n", __FILE__, __LINE__ );
	int dwarvenName = status;

	Dwarf_Off subrangeOffset;
	status = dwarf_dieoffset( subrangeDIE, & subrangeOffset, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dereferencing DWARF pointer\n", __FILE__, __LINE__ );

	BPatch_type * rangeType = new BPatch_typeRange( (int) subrangeOffset, 0, loBound.c_str(), hiBound.c_str(), subrangeName );
	assert( rangeType != NULL );
	rangeType = module->getModuleTypes()->addOrUpdateType( rangeType );
	if( dwarvenName == DW_DLV_OK ) { dwarf_dealloc( dbg, subrangeName, DW_DLA_STRING ); }	
	} /* end parseSubRangeDIE() */

BPatch_type * parseMultiDimensionalArray( Dwarf_Debug & dbg, Dwarf_Die range, BPatch_type * elementType, BPatch_module * module ) {
    char buf[32];
	/* Get the (negative) typeID for this range/subarray. */
	Dwarf_Off dieOffset;
	int status = dwarf_dieoffset( range, & dieOffset, NULL );
	DWARF_NULL_IF( status != DW_DLV_OK, "%s[%d]: error while parsing multidimensional array.\n", __FILE__, __LINE__ );

	/* Determine the range. */
	pdstring loBound;
	pdstring hiBound;
	parseSubRangeDIE( dbg, range, loBound, hiBound, module );

	/* Does the recursion continue? */
	Dwarf_Die nextSibling;
	status = dwarf_siblingof( dbg, range, & nextSibling, NULL );
	DWARF_NULL_IF( status == DW_DLV_ERROR, "%s[%d]: error checking for second dimension in array.\n", __FILE__, __LINE__ );

	snprintf(buf, 31, "__array%d", (int) dieOffset);

	if( status == DW_DLV_NO_ENTRY ) {
		/* Terminate the recursion by building an array type out of the elemental type.
		   Use the negative dieOffset to avoid conflicts with the range type created
		   by parseSubRangeDIE(). */
		// N.B.  I'm going to ignore the type id, and just create an anonymous type here
		BPatch_type * innermostType = new BPatch_typeArray( elementType, atoi( loBound.c_str() ), atoi( hiBound.c_str() ), buf );
		assert( innermostType != NULL );
		innermostType = module->getModuleTypes()->addOrUpdateType( innermostType );
		return innermostType;
		} /* end base-case of recursion. */

	/* If it does, build this array type out of the array type returned from the next recusion. */
	BPatch_type * innerType = parseMultiDimensionalArray( dbg, nextSibling, elementType, module );
	assert( innerType != NULL );
	// same here - type id ignored    jmo
	BPatch_type * outerType = new BPatch_typeArray( innerType, atoi(loBound.c_str()), atoi(hiBound.c_str()), buf);
	assert( outerType != NULL );
	outerType = module->getModuleTypes()->addOrUpdateType( outerType );

	dwarf_dealloc( dbg, nextSibling, DW_DLA_DIE );
	return outerType;
	} /* end parseMultiDimensionalArray() */

void deallocateLocationList( Dwarf_Debug & dbg, Dwarf_Locdesc * locationList, Dwarf_Signed listLength ) {
	for( int i = 0; i < listLength; i++ ) {
		dwarf_dealloc( dbg, locationList[i].ld_s, DW_DLA_LOC_BLOCK );
		}
	dwarf_dealloc( dbg, locationList, DW_DLA_LOCDESC );
	} /* end deallocateLocationList() */

void deallocateLocationList( Dwarf_Debug & dbg, Dwarf_Locdesc ** locationList, Dwarf_Signed listLength ) {
	for( int i = 0; i < listLength; i++ ) {
		dwarf_dealloc( dbg, locationList[i]->ld_s, DW_DLA_LOC_BLOCK );
		dwarf_dealloc( dbg, locationList[i], DW_DLA_LOCDESC );
		}
	dwarf_dealloc( dbg, locationList, DW_DLA_LIST );
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
	DWARF_NULL_IF( listLength != 1, "%s[%d]: unable to handle location lists of more than one element in frame base.\n", __FILE__, __LINE__ );
	Dwarf_Locdesc locationDescriptor = locationList[0];
	
	/* It is defined by a single operation. */
	DWARF_NULL_IF( locationDescriptor.ld_cents != 1, "%s[%d]: unable to handle multioperation locations in frame base.\n", __FILE__, __LINE__ );
	Dwarf_Loc location = locationDescriptor.ld_s[0];

	/* That operation is naming a register. */
	int registerNumber = 0;	
	if( DW_OP_reg0 <= location.lr_atom && location.lr_atom <= DW_OP_reg31 ) {
		    registerNumber = DWARF_TO_MACHINE_ENC(location.lr_atom - DW_OP_reg0);
		}
	else if( DW_OP_breg0 <= location.lr_atom && location.lr_atom <= DW_OP_breg31 ) {
		registerNumber = DWARF_TO_MACHINE_ENC(location.lr_atom - DW_OP_breg0);
		if( location.lr_number != 0 ) {
			/* Actually, we should be able whip up an AST node for this. */
			return NULL;
			}
		}
	else if( location.lr_atom == DW_OP_regx ) {
		registerNumber = DWARF_TO_MACHINE_ENC(location.lr_number);
		}
	else if( location.lr_atom == DW_OP_bregx ) {
		registerNumber = DWARF_TO_MACHINE_ENC(location.lr_number);
		if( location.lr_number2 != 0 ) {
			/* Actually, we should be able whip up an AST node for this. */
			return NULL;
			}
		}
	else {
		return NULL;
		}

	/* We have to make sure no arithmetic is actually done to the frame pointer,
	   so add zero to it and shove it in some other register. */
	AstNode *constantZero = AstNode::operandNode(AstNode::Constant, (void *)0);
	assert( constantZero != NULL );
	AstNode *framePointer = AstNode::operandNode(AstNode::DataReg, (void *)(long unsigned int)registerNumber);
	assert( framePointer != NULL );
	AstNode *moveFPtoDestination = AstNode::operatorNode(plusOp,
														 constantZero,
														 framePointer);
	
	return moveFPtoDestination;
	} /* end convertFrameBaseToAST(). */

bool decodeLocationListForStaticOffsetOrAddress( Dwarf_Locdesc * locationList, Dwarf_Signed listLength, long int * offset, int * regNum, long int * initialStackValue = NULL, BPatch_storageClass * storageClass = NULL ) {
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
	DWARF_FALSE_IF( listLength != 1, "%s[%d]: unable to decode location lists of non-unit length.\n", __FILE__, __LINE__ );

	if( storageClass != NULL ) { * storageClass = BPatch_storageAddr; }
	if( regNum != NULL ) { *regNum = -1; }

	/* Initialize the stack. */
	std::stack< long int > opStack = std::stack<long int>();
	if( initialStackValue != NULL ) { opStack.push( * initialStackValue ); }

	/* There is only one location. */
	Dwarf_Locdesc location = locationList[0];
	Dwarf_Loc * locations = location.ld_s;
	for( unsigned int i = 0; i < location.ld_cents; i++ ) {
		/* Handle the literals w/o 32 case statements. */
		if( DW_OP_lit0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_lit31 ) {
			dwarf_printf( "pushing named constant: %d\n", locations[i].lr_atom - DW_OP_lit0 );
			opStack.push( locations[i].lr_atom - DW_OP_lit0 );
			continue;
			}

		/* Haandle registers w/o 32 case statements. */
		if( DW_OP_reg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_reg31 ) {
			/* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
			dwarf_printf( "location is named register %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_reg0) );
			if( storageClass != NULL ) { * storageClass = BPatch_storageRegOffset; }
			if( regNum != NULL ) { 
                *regNum = DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_reg0);
            }
			if( offset != NULL ) { * offset = 0; }
			return true;
			}
			
		/* Haandle registers w/o 32 case statements. */
		if( DW_OP_breg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_breg31 ) {
			dwarf_printf( "setting storage class to named register, regNum to %d, offset %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_breg0), locations[i].lr_number );
			if( storageClass != NULL ) { * storageClass = BPatch_storageRegOffset; }
			if( regNum != NULL ) { 
                * regNum = DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_breg0);
            }
			opStack.push( locations[i].lr_number );
			continue;
			}

		switch( locations[i].lr_atom ) {
			case DW_OP_addr:
			case DW_OP_const1u:
			case DW_OP_const2u:
			case DW_OP_const4u:
			case DW_OP_const8u:
			case DW_OP_constu:
				dwarf_printf( "pushing unsigned constant %lu\n", (unsigned long)locations[i].lr_number );
				opStack.push( (Dwarf_Unsigned)locations[i].lr_number );
				break;

			case DW_OP_const1s:
			case DW_OP_const2s:
			case DW_OP_const4s:
			case DW_OP_const8s:
			case DW_OP_consts:
				dwarf_printf( "pushing signed constant %ld\n", (signed long)(locations[i].lr_number) );
				opStack.push( (Dwarf_Signed)(locations[i].lr_number) );
				break;

			case DW_OP_regx:
				/* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
				dwarf_printf( "location is register %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_number) );
				if( storageClass != NULL ) { * storageClass = BPatch_storageRegOffset; }
				if( regNum != NULL ) { 
                    * regNum = DWARF_TO_MACHINE_ENC(locations[i].lr_number); 
                }
				if( offset != NULL ) { * offset = 0; }
				return true;

			case DW_OP_fbreg:
				dwarf_printf( "setting storage class to frame base\n" );
				if( storageClass != NULL ) { * storageClass = BPatch_storageFrameOffset; }
				opStack.push( locations[i].lr_number );
				break;

			case DW_OP_bregx:
				dwarf_printf( "setting storage class to register, regNum to %d\n", locations[i].lr_number );
				if( storageClass != NULL ) { * storageClass = BPatch_storageRegOffset; }
				if( regNum != NULL ) { 
                    // Again with the lies about register numbers
                    // * regNum = locations[i].lr_number; 
                    *regNum = DWARF_TO_MACHINE_ENC( locations[i].lr_number );
                }
				opStack.push( locations[i].lr_number2 );
				break;

			case DW_OP_dup:
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				opStack.push( opStack.top() );
				break;

			case DW_OP_drop:
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				opStack.pop();
				break;

			case DW_OP_pick: {
				/* Duplicate the entry at index locations[i].lr_number. */
				std::stack< long int > temp = std::stack< long int >();
				for( unsigned int j = 0; j < locations[i].lr_number; j++ ) {
					temp.push( opStack.top() ); opStack.pop();
					}
				long int dup = opStack.top();
				for( unsigned int j = 0; j < locations[i].lr_number; j++ ) {
					opStack.push( temp.top() ); temp.pop();
					}
				opStack.push( dup );
				} break;

			case DW_OP_over: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second ); opStack.push( first ); opStack.push( second );
				} break;

			case DW_OP_swap: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first ); opStack.push( second );
				} break;

			case DW_OP_rot: {
				DWARF_FALSE_IF( opStack.size() < 3, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				long int third = opStack.top(); opStack.pop();
				opStack.push( first ); opStack.push( third ); opStack.push( second );
				} break;

			case DW_OP_abs: {
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int top = opStack.top(); opStack.pop();
				opStack.push( abs( top ) );
				} break;

			case DW_OP_and: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second & first );
				} break;
			
			case DW_OP_div: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second / first );
				} break;

			case DW_OP_minus: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second - first );
				} break;

			case DW_OP_mod: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second % first );
				} break;

			case DW_OP_mul: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second * first );
				} break;

			case DW_OP_neg: {
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				opStack.push( first * (-1) );
				} break;

			case DW_OP_not: {
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				opStack.push( ~ first );
				} break;

			case DW_OP_or: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second | first );
				} break;

			case DW_OP_plus: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second + first );
				} break;

			case DW_OP_plus_uconst: {
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				opStack.push( first + locations[i].lr_number );
				} break;

			case DW_OP_shl: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second << first );
				} break;

			case DW_OP_shr: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( (long int)((unsigned long)second >> (unsigned long)first) );
				} break;

			case DW_OP_shra: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second >> first );
				} break;

			case DW_OP_xor: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( second ^ first );
				} break;

			case DW_OP_le: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first <= second ? 1 : 0 );
				} break;

			case DW_OP_ge: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first >= second ? 1 : 0 );
				} break;

			case DW_OP_eq: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first == second ? 1 : 0 );
				} break;

			case DW_OP_lt: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first < second ? 1 : 0 );
				} break;

			case DW_OP_gt: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first > second ? 1 : 0 );
				} break;

			case DW_OP_ne: {
				DWARF_FALSE_IF( opStack.size() < 2, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
				long int first = opStack.top(); opStack.pop();
				long int second = opStack.top(); opStack.pop();
				opStack.push( first != second ? 1 : 0 );
				} break;

			case DW_OP_bra:
				DWARF_FALSE_IF( opStack.size() < 1, "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
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
				bperr( "Warning: dyninst does not handle multi-part variables.\n" );
				break;

			case DW_OP_nop:
				break;

			default:
				dwarf_printf( "Unrecognized or non-static location opcode 0x%x, aborting.\n", locations[i].lr_atom );
				return false;
			} /* end operand switch */
		} /* end iteration over Dwarf_Loc entries. */

	/* The top of the stack is the computed location. */
	if( opStack.empty() ) {
		dwarf_printf( "ignoring malformed location list (stack empty at end of list).\n" );
		return false;
		}
	dwarf_printf( "setting offset to %d\n", opStack.top() );
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
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dumping attribute list.\n", __FILE__, __LINE__ );

	Dwarf_Attribute * attributeList;
	Dwarf_Signed attributeCount;
	status = dwarf_attrlist( dieEntry, & attributeList, & attributeCount, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dumping attribute list.\n", __FILE__, __LINE__ );

	bperr( "DIE %s has attributes:", entryName );
	for( int i = 0; i < attributeCount; i++ ) {
		Dwarf_Half whatAttr = 0;
		status = dwarf_whatattr( attributeList[i], & whatAttr, NULL );
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dumping attribute list.\n", __FILE__, __LINE__ );
		fprintf( stderr, " 0x%x", whatAttr );
		
		dwarf_dealloc( dbg, attributeList[i], DW_DLA_ATTR );
		} /* end iteration over attributes */
	fprintf( stderr, "\n" );
	
	dwarf_dealloc( dbg, attributeList, DW_DLA_LIST );
	dwarf_dealloc( dbg, entryName, DW_DLA_STRING );
	} /* end dumpAttributeList() */

bool walkDwarvenTree(	Dwarf_Debug & dbg, char * moduleName, Dwarf_Die dieEntry,
			BPatch_module * module, 
			process * proc,
			Dwarf_Off cuOffset,
			BPatch_function * currentFunction = NULL,
			BPatch_typeCommon * currentCommonBlock = NULL,
			BPatch_fieldListType * currentEnclosure = NULL ) {

	/* optimization */ tail_recursion:;
	Dwarf_Half dieTag;
	int status = dwarf_tag( dieEntry, & dieTag, NULL );
	DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	
	Dwarf_Off dieOffset;
	status = dwarf_dieoffset( dieEntry, & dieOffset, NULL );
	DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	
	Dwarf_Off dieCUOffset;
	status = dwarf_die_CU_offset( dieEntry, & dieCUOffset, NULL );
	DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	dwarf_printf( "Considering DIE at %lu (%lu CU-relative) with tag 0x%x\n", (unsigned long)dieOffset, (unsigned long)dieCUOffset, dieTag );
	
	
	/* If this entry is a function, common block, or structure (class),
	   its children will be in its scope, rather than its
	   enclosing scope. */
	BPatch_function * newFunction = currentFunction;
	BPatch_typeCommon * newCommonBlock = currentCommonBlock;
	BPatch_fieldListType * newEnclosure = currentEnclosure;

	bool parsedChild = false;
	/* Is this is an entry we're interested in? */
	switch( dieTag ) {
		/* case DW_TAG_inline_subroutine: we don't care about these */
		case DW_TAG_subprogram:
		case DW_TAG_entry_point: {

			/* Is this entry specified elsewhere?  We may need to look there for its name. */
			Dwarf_Bool hasSpecification;
			status = dwarf_hasattr( dieEntry, DW_AT_specification, & hasSpecification, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			/* Our goal is three-fold: First, we want to set the return type
			   of the function.  Second, we want to set the newFunction variable
			   so subsequent entries are handled correctly.  Third, we want to
			   record (the location of, or how to calculate) the frame base of 
			   this function for use by our instrumentation code later. */

			char * functionName = NULL;
			Dwarf_Die specEntry = dieEntry;

			/* In order to do this, we need to find the function's (mangled) name.
			   If a function has a specification, its specification will have its
			   name. */
			if( hasSpecification ) {
				Dwarf_Attribute specAttribute;
				status = dwarf_attr( dieEntry, DW_AT_specification, & specAttribute, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				Dwarf_Off specOffset;
				status = dwarf_global_formref( specAttribute, & specOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
				} /* end if the function has a specification */

			/* Prefer linkage names. */
			Dwarf_Attribute linkageNameAttr;
			status = dwarf_attr( specEntry, DW_AT_MIPS_linkage_name, & linkageNameAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			bool hasLinkageName;
			if( status == DW_DLV_OK ) {
				hasLinkageName = true;
				status = dwarf_formstring( linkageNameAttr, & functionName, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				dwarf_dealloc( dbg, linkageNameAttr, DW_DLA_ATTR );
				} /* end if there's a linkage name. */
			else {
				hasLinkageName = false;
				status = dwarf_diename( specEntry, & functionName, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				} /* end if there isn't a linkage name. */

			if( functionName == NULL ) {
				/* I'm not even sure what an anonymous function _means_,
				   but we sure can't do anything with it. */
				dwarf_printf( "Warning: anonymous function (type %lu).\n", (unsigned long)dieOffset );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
				} /* end if there's no name at all. */

			/* Try to find the function by its mangled name. */
			Dwarf_Addr baseAddr = 0;
			mapped_object* fileOnDisk = module->lowlevel_mod()->obj();
			const pdvector< int_function * > *ret_funcs = fileOnDisk->findFuncVectorByMangled(functionName);
			pdvector<int_function *> functions;
			if (ret_funcs) {
				for (unsigned foo = 0; foo < ret_funcs->size(); foo++)
					functions.push_back((*ret_funcs)[foo]);
			}
			else {
				/* If we can't find it by mangled name, try searching by address. */
				status = dwarf_lowpc( dieEntry, & baseAddr, NULL );
				if( status == DW_DLV_OK ) {
					/* The base addresses in DWARF appear to be image-relative. */
					// int_function * intFunction = proc->findFuncByAddr( baseAddr );
					Address absAddr = fileOnDisk->getBaseAddress() + baseAddr;
					int_function * intFunction = fileOnDisk->findFuncByAddr( absAddr );
					if( intFunction != NULL ) {
						functions.push_back( intFunction );
					}
				}
			}
			if( functions.size() == 0 ) { // Still....
				/* If we can't find it by address, try searching by pretty name. */
				if( baseAddr != 0 ) { dwarf_printf( "%s[%d]: unable to locate function %s by address 0x%llx\n", __FILE__, __LINE__, functionName, baseAddr ); }
				const pdvector<int_function *> *prettyFuncs = fileOnDisk->findFuncVectorByPretty(functionName);
				if (prettyFuncs) {
					for (unsigned bar = 0; bar < prettyFuncs->size(); bar++) {
						functions.push_back((*prettyFuncs)[bar]);
					}
				}
			}
			
			if( functions.size() == 0 ) {
				/* Don't parse the children, since we can't add them. */
				dwarf_printf( "Failed to find function '%s'\n", functionName );
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
				}
			else if( functions.size() > 1 ) {
				dwarf_printf( "Warning: found more than one function '%s', unable to do anything reasonable.\n", functionName );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;		
				}
			else {
				int_function * newIntFunction = functions[0];
				assert( newIntFunction != NULL );
				newFunction = proc->newFunctionCB( newIntFunction );
				assert( newFunction != NULL );
			} /* end findFunction() cases */

			/* Once we've found the BPatch_function pointer corresponding to this
			   DIE, record its frame base.  A declaration entry may not have a 
			   frame base, and some functions do not have frames. */
			Dwarf_Attribute frameBaseAttribute;
			status = dwarf_attr( dieEntry, DW_AT_frame_base, & frameBaseAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
				Dwarf_Locdesc ** locationList;
				Dwarf_Signed listLength;
				status = dwarf_loclist_n( frameBaseAttribute, & locationList, & listLength, NULL );
				if( status != DW_DLV_OK ) {
					/* I think DWARF 3 generically allows this abomination of empty loclists. */
					break;
					}
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, frameBaseAttribute, DW_DLA_ATTR );
					
#if defined(ia64_unknown_linux2_4)
				/* Convert location list to an AST for later code generation. */
				newFunction->lowlevel_func()->ifunc()->framePointerCalculator = convertFrameBaseToAST( locationList[0], listLength );
#else
				static bool warned_no_locals = false;
				if (listLength > 1 && !warned_no_locals) {
				   bpwarn("WARNING:\tmutatee contains advanced frame pointer definitions\n\tlocal variable support unavailable\n");
				   warned_no_locals = true;
				}
#endif
				
				deallocateLocationList( dbg, locationList, listLength );
				} /* end if this DIE has a frame base attribute */
			
			/* Find its return type. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			BPatch_type * returnType = NULL;
			if( status == DW_DLV_NO_ENTRY ) { 
				returnType = module->getModuleTypes()->findType( "void" );
				newFunction->setReturnType( returnType );
				} /* end if the return type is void */
			else {
				/* There's a return type attribute. */
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				parsing_printf("%s/%d: ret type %d\n",
							   __FILE__, __LINE__, typeOffset);
				returnType = module->getModuleTypes()->findOrCreateType( typeOffset );
				newFunction->setReturnType( returnType );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if not a void return type */

			/* If this is a member function, add it as a field, for backward compatibility */
			if( currentEnclosure != NULL ) {
				/* Using the mangled name allows us to distinguish between overridden
				   functions, but confuses the tests.  Since BPatch_type uses vectors
				   to hold field names, however, duplicate -- demangled names -- are OK. */
				char * demangledName = P_cplus_demangle( functionName, module->isNativeCompiler() );

				char * leftMost = NULL;
				if( demangledName == NULL ) {
					dwarf_printf( "%s[%d]: unable to demangle '%s', using mangled name.\n", __FILE__, __LINE__, functionName );
					demangledName = strdup( functionName );
					assert( demangledName != NULL );
					leftMost = demangledName;
					}
				else {
					/* Strip everything left of the rightmost ':' off; see above. */
					leftMost = demangledName;
					if( strrchr( demangledName, ':' ) )
						leftMost = strrchr( demangledName, ':' ) + 1;
					}

				currentEnclosure->addField( leftMost, BPatch_dataMethod, returnType, 0, 0 );
				free( demangledName );
				}

			if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
			dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
			} break;


		case DW_TAG_common_block: {
			char * commonBlockName;
			status = dwarf_diename( dieEntry, & commonBlockName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			BPatch_image * wholeProgram = (BPatch_image *) module->getObjParent();
			BPatch_variableExpr * commonBlockVar = 
					wholeProgram->findVariable( commonBlockName, false );
			if (!commonBlockVar) {
			   //pgcc 6 is naming common blocks with a trailing underscore
			   pdstring cbvname = pdstring(commonBlockName) + pdstring("_");
			   commonBlockVar = wholeProgram->findVariable( cbvname.c_str(),
															false );
			}

			DWARF_FALSE_IF( !commonBlockVar, "%s[%d]: Couldn't find variable for common block\n", __FILE__, __LINE__);

			BPatch_type * commonBlockType = NULL;
			commonBlockType = const_cast< BPatch_type * >( commonBlockVar->getType() );

			if( commonBlockType == NULL || 
				commonBlockType->getDataClass() == BPatch_dataCommon ||
				commonBlockType->getDataClass() == BPatch_dataNullType) 
			{
				commonBlockType = new BPatch_typeCommon( dieOffset, commonBlockName );
				assert( commonBlockType != NULL );
				commonBlockVar->setType( commonBlockType );
				module->getModuleTypes()->addGlobalVariable( commonBlockName, commonBlockType );
			} /* end if we re-define the type. */

			dwarf_dealloc( dbg, commonBlockName, DW_DLA_STRING );

			/* This node's children are in the common block. */
			newCommonBlock = dynamic_cast<BPatch_typeCommon*>(commonBlockType);
			newCommonBlock->beginCommonBlock();
			} break;

		case DW_TAG_constant: {
			bperr( "Warning: dyninst ignores named constant entries.\n" );
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
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				Dwarf_Attribute typeAttribute;
				status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				/* The typeOffset forms a module-unique type identifier,
				   so the BPatch_type look-ups by it rather than name. */
				dwarf_printf( "%s/%d: %s/%d\n", __FILE__, __LINE__, variableName, typeOffset );
				BPatch_type * variableType = module->getModuleTypes()->findOrCreateType( typeOffset );
				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				
				/* Tell Dyninst what this global's type is. */
				module->getModuleTypes()->addGlobalVariable( variableName, variableType );
				} /* end if this variable is a global */
			else {
				/* We'll start with the location, since that's most likely to
				   require the _specification. */
				Dwarf_Attribute locationAttribute;
				status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				if( status == DW_DLV_NO_ENTRY ) { break; }
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				Dwarf_Locdesc * locationList;
				Dwarf_Signed listLength;
				status = dwarf_loclist( locationAttribute, & locationList, & listLength, NULL );
				dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );
				if( status != DW_DLV_OK ) {
					/* I think this is OK if the local variable was optimized away. */
					break;
					}
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				

				int regNum;
				BPatch_storageClass storageClass;
				long int variableOffset;
				bool decodedAddressOrOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & variableOffset, & regNum, NULL, & storageClass );
				deallocateLocationList( dbg, locationList, listLength );
				
				if( ! decodedAddressOrOffset ) { break; }
				assert( decodedAddressOrOffset );
			
				/* If this DIE has a _specification, use that for the rest of our inquiries. */
				Dwarf_Die specEntry = dieEntry;
				
				Dwarf_Attribute specAttribute;
				status = dwarf_attr( dieEntry, DW_AT_specification, & specAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				if( status == DW_DLV_OK ) {
					Dwarf_Off specOffset;
					status = dwarf_global_formref( specAttribute, & specOffset, NULL );
					DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
					
					status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );
					DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
					dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
					} /* end if dieEntry has a _specification */
					
				/* Acquire the name, type, and line number. */
				char * variableName;
				status = dwarf_diename( specEntry, & variableName, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				/* We can't do anything with an anonymous variable. */
				if( status == DW_DLV_NO_ENTRY ) { break; }
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				/* If we're fortran, get rid of the trailing _ */
				BPatch_language language = module->getLanguage();
				if ( ( language == BPatch_fortran ||
					 language == BPatch_fortran90 ||
					 language == BPatch_fortran95 ) &&
					 variableName[strlen(variableName)-1]=='_') 
				   variableName[strlen(variableName)-1]='\0';

				/* Acquire the parameter's type. */
				Dwarf_Attribute typeAttribute;
				status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
				if( status == DW_DLV_NO_ENTRY ) { break; }
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
				/* The typeOffset forms a module-unique type identifier,
				   so the BPatch_type look-ups by it rather than name. */
				BPatch_type * variableType = module->getModuleTypes()->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
				/* Acquire the parameter's lineNo. */
				Dwarf_Unsigned variableLineNo;
				bool hasLineNumber = false;

				Dwarf_Attribute lineNoAttribute;
				status = dwarf_attr( specEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
				/* We don't need to tell Dyninst a line number for C++ static variables,
				   so it's OK if there isn't one. */
				if( status == DW_DLV_OK ) {
					hasLineNumber = true;
					status = dwarf_formudata( lineNoAttribute, & variableLineNo, NULL );
					DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
					
					dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );			
					} /* end if there is a line number */
							
				/* We now have the variable name, type, offset, and line number.
				   Tell Dyninst about it. */
				dwarf_printf( "localVariable '%s', currentFunction %p\n", variableName, currentFunction );
				if( currentFunction != NULL ) {
                    if(!hasLineNumber)
                        variableLineNo = 0;
					
					BPatch_localVar * newVariable = new BPatch_localVar( variableName, variableType, variableLineNo, variableOffset, regNum, storageClass );
					currentFunction->localVariables->addLocalVar( newVariable );
					} /* end if a local or static variable. */
				else if( currentEnclosure != NULL ) {
					assert( storageClass != BPatch_storageFrameOffset );
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
			if( currentFunction == NULL ) {
				dwarf_printf( "%s[%d]: ignoring formal parameter without corresponding function.\n", __FILE__, __LINE__ );
				break;
				}
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
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			if( !hasLocation ) {
				dwarf_printf( "%s[%d]: ignoring formal parameter without location.\n", __FILE__, __LINE__ );
				break;
				}
			assert( hasLocation );
		
			/* Acquire the location of this formal parameter. */
			Dwarf_Attribute locationAttribute;
			status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Locdesc * locationList;
			Dwarf_Signed listLength;
			status = dwarf_loclist( locationAttribute, & locationList, & listLength, NULL );
			dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );
			if( status != DW_DLV_OK ) {
				/* I think this is legal if the parameter was optimized away. */
				dwarf_printf( "%s[%d]: ignoring formal parameter with bogus location.\n", __FILE__, __LINE__ );
				break;
				}
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			int regNum;
			BPatch_storageClass storageClass;
			long int parameterOffset;
			bool decodedOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & parameterOffset, & regNum, NULL, & storageClass );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedOffset ) {
				dwarf_printf( "%s[%d]: ignoring formal parameter with undecodable location.\n", __FILE__, __LINE__ );
				break;
				}
			assert( decodedOffset );
			
			assert( storageClass != BPatch_storageAddr );

			/* If the DIE has an _abstract_origin, we'll use that for the
			   remainder of our inquiries. */
			Dwarf_Die originEntry = dieEntry;
		
			Dwarf_Attribute originAttribute;
			status = dwarf_attr( dieEntry, DW_AT_abstract_origin, & originAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			if( status == DW_DLV_OK ) {
				Dwarf_Off originOffset;
				status = dwarf_global_formref( originAttribute, & originOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				status = dwarf_offdie( dbg, originOffset, & originEntry, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				
				dwarf_dealloc( dbg, originAttribute, DW_DLA_ATTR );
				} /* end if the DIE has an _abstract_origin */
			
			/* Acquire the parameter's name. */
			char * parameterName;
			status = dwarf_diename( originEntry, & parameterName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			/* We can't do anything with anonymous parameters. */
			if( status == DW_DLV_NO_ENTRY ) { break; }
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			/* Acquire the parameter's type. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( originEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			if( status == DW_DLV_NO_ENTRY ) { break; }
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			/* The typeOffset forms a module-unique type identifier,
			   so the BPatch_type look-ups by it rather than name. */
			dwarf_printf( "%s[%d]: found formal parameter %s with type %ld\n", __FILE__, __LINE__, parameterName, typeOffset );
			BPatch_type * parameterType = module->getModuleTypes()->findOrCreateType( typeOffset );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			/* Acquire the parameter's lineNo. */
			Dwarf_Attribute lineNoAttribute;
			status = dwarf_attr( originEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			if( status == DW_DLV_NO_ENTRY ) {
				dwarf_printf( "%s[%d]: ignoring formal parameter without line number.\n", __FILE__, __LINE__ );
				break;
				}
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Unsigned parameterLineNo;
			status = dwarf_formudata( lineNoAttribute, & parameterLineNo, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );
			
			/* We now have the parameter's location, name, type, and line number.
			   Tell Dyninst about it. */
			BPatch_localVar * newParameter = new BPatch_localVar( parameterName, parameterType, parameterLineNo, parameterOffset, regNum, storageClass );
			assert( newParameter != NULL );
			
			/* This is just brutally ugly.  Why don't we take care of this invariant automatically? */
			currentFunction->funcParameters->addLocalVar( newParameter );
			currentFunction->addParam( parameterName, parameterType, parameterLineNo, parameterOffset );

			dwarf_printf( "%s[%d]: added formal parameter '%s' (at FP + %ld) of type %p from line %lu.\n", __FILE__, __LINE__, parameterName, parameterOffset, parameterType, (unsigned long)parameterLineNo );
			} break;

		case DW_TAG_base_type: {
			/* What's the type's name? */
			char * typeName;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			/* How big is it? */
			Dwarf_Attribute byteSizeAttr;
			Dwarf_Unsigned byteSize;
			status = dwarf_attr( dieEntry, DW_AT_byte_size, & byteSizeAttr, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			status = dwarf_formudata( byteSizeAttr, & byteSize, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, byteSizeAttr, DW_DLA_ATTR );
			
			/* Generate the appropriate built-in type; since there's no
			   reliable way to distinguish between a built-in and a scalar,
			   we don't bother to try. */
			BPatch_type * baseType = new BPatch_typeScalar( dieOffset, byteSize, typeName );
			assert( baseType != NULL );

			/* Add the basic type to our collection. */
			dwarf_printf( "Adding base type '%s' (%lu) of size %lu to type collection %p\n", typeName, (unsigned long)dieOffset, (unsigned long)byteSize, module->getModuleTypes() );
			baseType = module->getModuleTypes()->addOrUpdateType( baseType );
			
			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_typedef: {
			char * definedName = NULL;
			status = dwarf_diename( dieEntry, & definedName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			BPatch_type * referencedType = NULL;
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			if( status == DW_DLV_NO_ENTRY ) {
				/* According to the DWARF spec, "A declaration of the type that is not also a definition."
				   This includes constructions like "typedef void _IO_lock_t", from libio.h, which
				   cause us to issue a lot of true but spurious-looking warnings about incomplete types.
				   So instead of ignoring this entry, point it to the void type.  (This is also more
				   in line with our handling of absent DW_AT_type tags everywhere else.) */
                referencedType = module->getModuleTypes()->findType( "void" );
			    }
			else {
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );

				/* Look up the referenced type. */
				parsing_printf("%s/%d: %s/%d\n",
							   __FILE__, __LINE__, definedName, typeOffset);
				referencedType = module->getModuleTypes()->findOrCreateType( typeOffset );
				}

			/* Add the typedef to our collection. */
			// bperr( "Adding typedef: '%s' as %lu (pointing to %lu)\n", definedName, (unsigned long)dieOffset, (unsigned long)typeOffset );
		  	BPatch_type * typedefType = new BPatch_typeTypedef( dieOffset, referencedType, definedName);
			typedefType = module->getModuleTypes()->addOrUpdateType( typedefType );

			/* Sanity check: typedefs should not have children. */
			Dwarf_Die childDwarf;
			status = dwarf_child( dieEntry, & childDwarf, NULL );
			assert( status == DW_DLV_NO_ENTRY );
			
			dwarf_dealloc( dbg, definedName, DW_DLA_STRING );
			} break;

		case DW_TAG_array_type: {
			/* Two words about pgf90 arrays.
			
			   Primus: the PGI extensions to DWARF are documented in 
				'/p/paradyn/doc/External/manuals/pgf90-dwarf-arrays.txt'.
				
			   Secundus: we ignore DW_AT_PGI_lbase, DW_AT_PGI_loffset, and DW_AT_PGI_lstride,
			   even though libdwarf recognizes them, because our type modelling doesn't allow
			   us to make use of this information.  Similarly, in virtually every place where
			   the Portland Group extends DWARF to allow _form_block attributes encoding location
			   lists, we ignore them.  We should, however, recognize these cases and ignore them
			   gracefully, that is, without an error. :)
			*/
				
			char * arrayName = NULL;
			status = dwarf_diename( dieEntry, & arrayName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			/* Find the type of the elements. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			parsing_printf("%s/%d: %s/%d\n",
						   __FILE__, __LINE__, arrayName, typeOffset);
			BPatch_type * elementType = module->getModuleTypes()->findOrCreateType( typeOffset );

			/* Find the range(s) of the elements. */
			Dwarf_Die firstRange;
			status = dwarf_child( dieEntry, & firstRange, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			BPatch_type * baseArrayType = parseMultiDimensionalArray( dbg, firstRange, elementType, module );
			assert( baseArrayType != NULL );

			/* The baseArrayType is an anonymous type with its own typeID.  Extract
			   the information and add an array type for this DIE. */
			BPatch_type * arrayType = new BPatch_typeArray( dieOffset,
															baseArrayType->getConstituentType(), 
															atoi(baseArrayType->getLow()),
															atoi(baseArrayType->getHigh()), 
															arrayName);
			assert( arrayType != NULL );
			//			setArraySize( arrayType, baseArrayType->getLow(), baseArrayType->getHigh() );
			// bperr( "Adding array type '%s' (%lu) [%s, %s] @ %p\n", arrayName, (unsigned long)dieOffset, baseArrayType->getLow(), baseArrayType->getHigh(), arrayType );
			arrayType = module->getModuleTypes()->addOrUpdateType( arrayType );

			/* Don't parse the children again. */
			parsedChild = true;

			dwarf_dealloc( dbg, firstRange, DW_DLA_DIE );
			dwarf_dealloc( dbg, arrayName, DW_DLA_STRING );
			} break;

		case DW_TAG_subrange_type: {
			pdstring loBound;
			pdstring hiBound;
			parseSubRangeDIE( dbg, dieEntry, loBound, hiBound, module );
			} break;

		case DW_TAG_enumeration_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			BPatch_fieldListType * enumerationType = new BPatch_typeEnum( dieOffset, typeName);
			assert( enumerationType != NULL );
			enumerationType = dynamic_cast<BPatch_fieldListType *>(module->getModuleTypes()->addOrUpdateType( enumerationType ));
			newEnclosure = enumerationType;

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_inheritance: {
			/* Acquire the super class's type. */
			Dwarf_Attribute scAttr;
			status = dwarf_attr( dieEntry, DW_AT_type, & scAttr, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Off scOffset;
			status = dwarf_global_formref( scAttr, & scOffset, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, scAttr, DW_DLA_ATTR );

			parsing_printf("%s/%d: inherited %d\n",
						   __FILE__, __LINE__, scOffset);
			BPatch_type * superClass = module->getModuleTypes()->findOrCreateType( scOffset );

			/* Acquire the visibility, if any.  DWARF calls it accessibility
			   to distinguish it from symbol table visibility. */
			Dwarf_Attribute visAttr;
			status = dwarf_attr( dieEntry, DW_AT_accessibility, & visAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			BPatch_visibility visibility = BPatch_private;
			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned visValue;
				status = dwarf_formudata( visAttr, & visValue, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				switch( visValue ) {
					case DW_ACCESS_public: visibility = BPatch_public; break;
					case DW_ACCESS_protected: visibility = BPatch_protected; break;
					case DW_ACCESS_private: visibility = BPatch_private; break;
					default:
						bperr( "Uknown visibility, ignoring.\n" );
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
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Attribute sizeAttr;
			Dwarf_Unsigned typeSize = 0;
			status = dwarf_attr( dieEntry, DW_AT_byte_size, & sizeAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
				status = dwarf_formudata( sizeAttr, & typeSize, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, sizeAttr, DW_DLA_ATTR );
				}

			BPatch_fieldListType * containingType = NULL;
			switch ( dieTag ) {
				case DW_TAG_structure_type: 
				case DW_TAG_class_type: 
				   containingType = new BPatch_typeStruct( dieOffset,
														   typeName);
				   break;
				case DW_TAG_union_type: 
				   containingType = new BPatch_typeUnion( dieOffset, 
														  typeName);
				   break;
				}
			
			assert( containingType != NULL );
			// bperr( "Adding structure, union, or class type '%s' (%lu)\n", typeName, (unsigned long)dieOffset );
			containingType = dynamic_cast<BPatch_fieldListType *>(module->getModuleTypes()->addOrUpdateType( containingType ));
			newEnclosure = containingType;
			
			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;
			
		case DW_TAG_enumerator: {
			/* An entry in an enumeration. */
			char * enumName = NULL;
			status = dwarf_diename( dieEntry, & enumName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Attribute valueAttr;
			status = dwarf_attr( dieEntry, DW_AT_const_value, & valueAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
		
			Dwarf_Signed enumValue = 0;
			if( status == DW_DLV_OK ) {
				status = dwarf_formsdata( valueAttr, & enumValue, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, valueAttr, DW_DLA_ATTR );
				}

			// bperr( "Adding enum '%s' (%ld) to enumeration '%s' (%d)\n", enumName, (signed long)enumValue, currentEnclosure->getName(), currentEnclosure->getID() );
			currentEnclosure->addField( enumName, enumValue );

			dwarf_dealloc( dbg, enumName, DW_DLA_STRING );
			} break;

		case DW_TAG_member: {
			char * memberName = NULL;
			status = dwarf_diename( dieEntry, & memberName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			if( status == DW_DLV_NO_ENTRY ) break;
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Off typeOffset;
			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
			parsing_printf("%s/%d: %s/%d\n",
						   __FILE__, __LINE__, memberName, typeOffset);
			BPatch_type * memberType = module->getModuleTypes()->findOrCreateType( typeOffset );
				
			Dwarf_Attribute locationAttr;
			status = dwarf_attr( dieEntry, DW_AT_data_member_location, & locationAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_NO_ENTRY ) { break; }
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Locdesc * locationList;
			Dwarf_Signed listLength;
			status = dwarf_loclist( locationAttr, & locationList, & listLength, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, locationAttr, DW_DLA_ATTR );

			int regNum;
			BPatch_storageClass storageClass;
			long int memberOffset = 0;
			long int baseAddress = 0;
			bool decodedAddress = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, & memberOffset, & regNum, & baseAddress, & storageClass );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedAddress ) { break; }
			assert( decodedAddress );
			
			assert( storageClass == BPatch_storageAddr );

			/* DWARF stores offsets in bytes unless the member is a bit field.
			   Correct memberOffset as indicated.  Also, memberSize is in bytes
			   from the underlying type, not the # of bits used from it, so
			   correct that as necessary as well. */
			long int memberSize = memberType->getSize();

			Dwarf_Attribute bitOffset;
			status = dwarf_attr( dieEntry, DW_AT_bit_offset, & bitOffset, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned memberOffset_du = memberOffset;
				status = dwarf_formudata( bitOffset, &memberOffset_du, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, bitOffset, DW_DLA_ATTR );

				Dwarf_Attribute bitSize;
				status = dwarf_attr( dieEntry, DW_AT_bit_size, & bitSize, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				Dwarf_Unsigned memberSize_du = memberSize;
				status = dwarf_formudata( bitSize, &memberSize_du, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				dwarf_dealloc( dbg, bitSize, DW_DLA_ATTR );

				/* If the DW_AT_byte_size field exists, there's some padding.
				   FIXME?  We ignore padding for now.  (We also don't seem to handle
				   bitfields right in getComponents() anyway...) */
				}
			else {
				memberOffset *= 8;
				memberSize *= 8;
				} /* end if not a bit field member. */

			if( memberName != NULL ) {
				// /* DEBUG */ fprint( stderr, "Adding member to enclosure '%s' (%d): '%s' with type %lu at %ld and size %d\n", currentEnclosure->getName(), currentEnclosure->getID(), memberName, (unsigned long)typeOffset, memberOffset, memberType->getSize() );
				currentEnclosure->addField( memberName, memberType->getDataClass(), memberType, memberOffset, memberSize );
				dwarf_dealloc( dbg, memberName, DW_DLA_STRING );
				} else {
				/* An anonymous union [in a struct]. */
				currentEnclosure->addField( "[anonymous union]", memberType->getDataClass(), memberType, memberOffset, memberSize );				
				}
			} break;

		case DW_TAG_const_type:
		case DW_TAG_packed_type:
		case DW_TAG_volatile_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			/* Which type does it modify? */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );

			int typeSize = 0;
			Dwarf_Off typeOffset;
			BPatch_type * typeModified = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typeModified = module->getModuleTypes()->findType( "void" );
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				parsing_printf("%s/%d: %s/%d\n",
							   __FILE__, __LINE__, typeName, typeOffset);
				typeModified = module->getModuleTypes()->findOrCreateType( typeOffset );
				typeSize = typeModified->getSize();

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typeModified is not void */

			// I'm taking out the type qualifiers for right now

//			BPatch_type * modifierType = new BPatch_type( typeName, dieOffset, BPatch_typeAttrib, typeSize, typeModified, dieTag );
			BPatch_type * modifierType = new BPatch_typeTypedef(dieOffset, typeModified, typeName);
			assert( modifierType != NULL );
			// bperr( "Adding modifier type '%s' (%lu) modifying (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			modifierType = module->getModuleTypes()->addOrUpdateType( modifierType );
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
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			/* To which type does it point? */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Off typeOffset = 0;
			BPatch_type * typePointedTo = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typePointedTo = module->getModuleTypes()->findType( "void" );
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				parsing_printf("%s/%d: %s/%d\n",
							   __FILE__, __LINE__, typeName, typeOffset);
				typePointedTo = module->getModuleTypes()->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typePointedTo is not void */

			BPatch_type * indirectType = NULL;
			switch ( dieTag ) {
			case DW_TAG_subroutine_type:
			   indirectType = new BPatch_typeFunction(dieOffset, typePointedTo, typeName);
			   break;
			case DW_TAG_ptr_to_member_type:
			case DW_TAG_pointer_type:
			   indirectType = new BPatch_typePointer(dieOffset, typePointedTo, typeName);
			   break;
			case DW_TAG_reference_type:
			   indirectType = new BPatch_typeRef(dieOffset, typePointedTo, typeName);
			   break;
			}



			assert( indirectType != NULL );
			dwarf_printf( "Adding indirect type '%s' (%lu) pointing to (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			indirectType = module->getModuleTypes()->addOrUpdateType( indirectType );

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_variant_part:
			/* We don't support this (Pascal) type. */
		case DW_TAG_string_type:
			/* We don't support this (FORTRAN) type. */
		default:
			/* Nothing of interest. */
			// bperr( "Entry %lu with tag 0x%x ignored.\n", (unsigned long)dieOffset, dieTag );
			break;
		} /* end dieTag switch */

	/* Recurse to its child, if any. */
	Dwarf_Die childDwarf;
	status = dwarf_child( dieEntry, & childDwarf, NULL );
	DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	
	if( status == DW_DLV_OK && parsedChild == false ) {		
		walkDwarvenTree( dbg, moduleName, childDwarf, module, proc, cuOffset, newFunction, newCommonBlock, newEnclosure );
		}

	/* Recurse to its first sibling, if any. */
	Dwarf_Die siblingDwarf;
	status = dwarf_siblingof( dbg, dieEntry, & siblingDwarf, NULL );
	DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	/* Deallocate the entry we just parsed. */
	dwarf_dealloc( dbg, dieEntry, DW_DLA_DIE );

	if( status == DW_DLV_OK ) {
		/* Do the tail-call optimization by hand. */
		dieEntry = siblingDwarf;
		goto tail_recursion;
		}

	/* When would we return false? :) */
	return true;
	} /* end walkDwarvenTree() */

extern void pd_dwarf_handler( Dwarf_Error, Dwarf_Ptr );

void BPatch_module::parseDwarfTypes() {
	const char *fileName = mod->obj()->fullName().c_str();
	// Man do we do to a lot of trouble for this...
	const Object &moduleObject = mod->obj()->parse_img()->getObject();
	assert( fileName );
	assert( moduleTypes );

#if defined(arch_x86_64)
	if( moduleObject.getAddressWidth() == 8 ) {
		emit64();
        }
	else {
		emit32();
		}
#endif

	if( moduleTypes->dwarfParsed() ) {
		dwarf_printf( "%s[%d]: already parsed %s, moduleTypes = %p\n", __FILE__, __LINE__, fileName, moduleTypes );
		BPatch_Vector<BPatch_function *> * bpfuncs = getProcedures( true );
		assert( bpfuncs );
		for( unsigned int i = 0; i < bpfuncs->size(); i++ ) {
			(*bpfuncs)[i]->fixupUnknown( this );
			}
		return;
		}
	dwarf_printf( "%s[%d]: parsing %s...\n", __FILE__, __LINE__, fileName );

	/* Start the dwarven debugging. */
	Dwarf_Debug dbg;

	int fd = open( fileName, O_RDONLY );
	assert( fd != -1 );
	int status = dwarf_init( fd, DW_DLC_READ, & pd_dwarf_handler, moduleObject.getErrFunc(), & dbg, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error initializing libdwarf.\n", __FILE__, __LINE__ );

	/* Iterate over the compilation-unit headers. */
	Dwarf_Unsigned hdr;

	while( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & hdr, NULL ) == DW_DLV_OK ) {
		/* Obtain the module DIE. */
		Dwarf_Die moduleDIE;
		status = dwarf_siblingof( dbg, NULL, &moduleDIE, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error finding next CU header.\n", __FILE__, __LINE__ );

		/* Make sure we've got the right one. */
		Dwarf_Half moduleTag;
		status = dwarf_tag( moduleDIE, & moduleTag, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error acquiring CU header tag.\n", __FILE__, __LINE__ );
		assert( moduleTag == DW_TAG_compile_unit );

		/* We may need this later. */
		Dwarf_Off cuOffset;
		status = dwarf_dieoffset( moduleDIE, & cuOffset, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error acquiring CU header offset.\n", __FILE__, __LINE__ );

		/* Extract the name of this module. */
		char * moduleName;
		status = dwarf_diename( moduleDIE, & moduleName, NULL );
		if( status == DW_DLV_NO_ENTRY ) {
			moduleName = strdup( "{ANONYMOUS}" );
			assert( moduleName != NULL );
			}
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error acquiring module name.\n", __FILE__, __LINE__ );
		dwarf_printf( "%s[%d]: considering compilation unit '%s'\n", __FILE__, __LINE__, moduleName );

		/* Set the language, if any. */
		Dwarf_Attribute languageAttribute;
		status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error acquiring language attribute.\n", __FILE__, __LINE__ );

		if( status == DW_DLV_OK ) {
			Dwarf_Unsigned languageConstant;
			status = dwarf_formudata( languageAttribute, & languageConstant, NULL );
			DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error formulating language attribute.\n", __FILE__, __LINE__ );

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
			    case DW_LANG_Fortran95:
				    setLanguage( BPatch_fortran95 );
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

		/* Iterate over the tree rooted here; walkDwarvenTree() deallocates the passed-in DIE. */
		if( !walkDwarvenTree( dbg, moduleName, moduleDIE, this, this->proc->llproc, cuOffset ) ) {
			bpwarn( "Error while parsing DWARF info for module '%s'.\n", moduleName );
			return;
			}

		dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
		} /* end iteration over compilation-unit headers. */

	/* Clean up. */
	Elf * dwarfElf;
	status = dwarf_get_elf( dbg, & dwarfElf, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error during libdwarf cleanup.\n", __FILE__, __LINE__ );

	status = dwarf_finish( dbg, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error during libdwarf cleanup.\n", __FILE__, __LINE__ );
	P_close( fd );

	/* Run a sanity check. */
	assert( moduleTypes );

	/* Fix type list. */
	int typeID;
	BPatch_type * bpType;
	dictionary_hash_iter< int, BPatch_type * > typeIter( moduleTypes->typesByID );
	while( typeIter.next( typeID, bpType ) ) {
		bpType->fixupUnknowns(this);
		} /* end iteratation over types. */
	
	/* Fix the types of variables. */   
	pdstring variableName;
	dictionary_hash_iter< pdstring, BPatch_type * > variableIter( moduleTypes->globalVarsByName );
	while( variableIter.next( variableName, bpType ) ) {
		if(	bpType->getDataClass() == BPatch_dataUnknownType && 
			moduleTypes->findType( bpType->getID() ) != NULL ) {
			moduleTypes->globalVarsByName[ variableName ] = moduleTypes->findType( bpType->getID() );
			} /* end if data class is unknown but the type exists. */
		} /* end iteration over variables. */

	/* Fix the type references in functions. */
	BPatch_Vector<BPatch_function *> *bpfuncs = getProcedures(true);
	assert( bpfuncs );
	for( unsigned int i = 0; i < bpfuncs->size(); i++ ) {
		(*bpfuncs)[i]->fixupUnknown( this );
	}
	moduleTypes->setDwarfParsed();
} /* end parseDwarfTypes() */


#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))

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

static void patchVSyscallImage(char *mem_image, size_t image_size, 
							   unsigned address_width);
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
   patchVSyscallImage(eh_frame->buffer, dso_size, p->getAddressWidth());   

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
	 bperr( "Couldn't get fde list\n");
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
Address getRegValueAtFrame(void *ehf, Address pc, int reg, 
						   Address *reg_map,
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

   Address calced_value = ((unsigned long) reg_map[target_reg]) + (offset_relevant ? offset : 0);
   if (registr != DW_FRAME_CFA_COL)
   {
	 p->readDataSpace((caddr_t) calced_value, p->getAddressWidth(),
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

struct section_starts {
   Address start;
   unsigned size;
};

static bool getTextAndEHFrameStart(Elf *elf, 
								   pdvector<Address> &text_starts,
								   pdvector<unsigned> &text_sizes,
								   Address &ehframe_start,
								   unsigned &ehframe_size,
								   unsigned &ehframe_offset,
								   unsigned address_width) 
{
   Elf32_Ehdr *ehdr32;
   Elf32_Shdr *shdr32;
   Elf64_Ehdr *ehdr64;
   Elf64_Shdr *shdr64;
   Elf_Scn *sec;
   char *sname;
   unsigned offset, size;
   Address start;
   
   if (address_width == 4)
	  ehdr32 = elf32_getehdr(elf);
   else
	  ehdr64 = elf64_getehdr(elf);

   sec = elf_nextscn(elf, NULL);
   for (; sec != NULL; sec = elf_nextscn(elf, sec))      
   {
	  if (address_width == 4) {	  
		 shdr32 = elf32_getshdr(sec);
		 sname = elf_strptr(elf, ehdr32->e_shstrndx, shdr32->sh_name);
		 start = shdr32->sh_addr;
		 offset = shdr32->sh_offset;
		 size = shdr32->sh_size;
	  }
	  else {	  
		 shdr64 = elf64_getshdr(sec);
		 sname = elf_strptr(elf, ehdr64->e_shstrndx, shdr64->sh_name);
		 start = shdr64->sh_addr;
		 offset = shdr64->sh_offset;
		 size = shdr64->sh_size;
	  }

      if (strstr(sname, ".text"))
      {
		 text_starts.push_back(start);
		 text_sizes.push_back(size);
      }
      else if (strcmp(sname, ".eh_frame") == 0)
      {
         ehframe_start = start;
         ehframe_size = size;
         ehframe_offset = offset;
      }
   }
   return true;
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
static void patchVSyscallImage(char *mem_image, size_t image_size, 
							   unsigned address_width)
{
   Elf *elf;
   Address eh_frame_start;
   unsigned eh_frame_size, eh_frame_offset, eh_offset;
   char *eh_ptr;
   pdvector<Address> text_starts;
   pdvector<unsigned> text_sizes;
   

   elf = elf_memory(mem_image, image_size);
   
   /**
    * Get the starting address and size of the .text and .eh_frame
    *  sections.  Also get the file offset at which the .eh_frame
    *  is stored.
    **/
   eh_frame_size = eh_frame_offset = eh_offset = eh_frame_start = 0x0;

   getTextAndEHFrameStart(elf, text_starts, text_sizes,
						  eh_frame_start, eh_frame_size, eh_frame_offset,
						  address_width);
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

   if (address_width == 4) {
	  uint32_t *size, *type, *addr;
	  while (eh_offset < eh_frame_size)
	  {
		 size = (uint32_t *) &eh_ptr[eh_offset];
		 eh_offset += sizeof(int32_t);
		 type = (uint32_t *) &eh_ptr[eh_offset];
		 addr = (uint32_t *) &eh_ptr[eh_offset+sizeof(int32_t)];
		 
		 assert(*size % sizeof(int32_t) == 0);
		 
		 if (*type == 0x0)
		 {
			//If this is a CIE then skip it
			eh_offset += *size;            
			continue;
		 }
		 
		 for (unsigned i=0; i<text_starts.size(); i++) 
		 {
			Address cur_pos = eh_frame_start + eh_offset + sizeof(int32_t);
			if (((signed) *addr) + cur_pos >= text_starts[i]  &&
				((signed) *addr) + cur_pos <  text_starts[i]+text_sizes[i])
			{
			   //If the address apears to be relative to the FDE data 
			   // structure (cur_pos) we'll fix it.
			   *addr = ((signed) *addr) + cur_pos - text_starts[i];
			}
			else if (*addr >= text_starts[i] && 
					 *addr < text_starts[i]+text_sizes[i])
			{
			   //If the address appears to be absolute, we'll also fix up.
			   *addr = *addr - text_starts[i];
			}
		 }
		 
		 eh_offset += *size;
	  }
   }
}

#endif
