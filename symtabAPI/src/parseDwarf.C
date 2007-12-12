/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
			     
#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "libdwarf.h"

#include "Type.h"
#include "Symbol.h"
#include "Symtab.h"
#include "symtabAPI/src/Object.h"
#include "symtabAPI/src/Collections.h"
#include "common/h/pathName.h"

/*
#include "mapped_module.h"
#include "mapped_object.h"

#include "Module.h"
#include "TypePrivate.h"
#include "BPatch_collections.h"
#include "Symbol.h"
#include "BPatch_image.h"
#include "symtab.h"
#include "process.h"

#include "ast.h"
*/

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

/* For location decode. */
#include <stack>
/*
bool SymtabAPI::walkDwarvenTree(Dwarf_Debug & dbg, Dwarf_Die dieEntry,
                        Module * module,
                        Symtab * objFile,
                        Dwarf_Off cuOffset,
			char **srcFiles,
                        Symbol * currentFunction = NULL,
                        typeCommon * currentCommonBlock = NULL,
			typeEnum *currentEnum = NULL,
                        fieldListType * currentEnclosure = NULL );
*/																		

// on 64-bit x86_64 targets, the DWARF register number does not
// correspond to the machine encoding. See the AMD-64 ABI.


#if defined(arch_x86_64)
// We can only safely map the general purpose registers (0-7 on ia-32,
// 0-15 on amd-64)
#define IA32_MAX_MAP 7
#define AMD64_MAX_MAP 15
static int const amd64_register_map[] =
{
    0,  // RAX
    2,  // RDX
    1,  // RCX
    3,  // RBX
    6,  // RSI
    7,  // RDI
    5,  // RBP
    4,  // RSP
    8, 9, 10, 11, 12, 13, 14, 15    // gp 8 - 15
 /* This is incomplete. The x86_64 ABI specifies a mapping from
    dwarf numbers (0-66) to ("architecture number"). Without a
    corresponding mapping for the SVR4 dwarf-machine encoding for
    IA-32, however, it is not meaningful to provide this mapping. */
};

int Register_DWARFtoMachineEnc32(int n)
{
    if(n > IA32_MAX_MAP) {
       // dwarf_printf("%s[%d]: unexpected map lookup for DWARF register %d\n",
       //						             __FILE__,__LINE__,n);
    }
    return n;
}
												 

int Register_DWARFtoMachineEnc64(int n)
{
    if(n <= AMD64_MAX_MAP)
        return amd64_register_map[n];
    else {
        //dwarf_printf("%s[%d]: unexpected map lookup for DWARF register %d\n",
	//						             __FILE__,__LINE__,n);
        return n;
    }
}
						    
#define DWARF_TO_MACHINE_ENC(n, proc) \
  ((proc->getAddressWidth() == 4) ? Register_DWARFtoMachineEnc32(n) : Register_DWARFtoMachineEnc64(n))
#else
#define DWARF_TO_MACHINE_ENC(n, proc) (n)
#endif

/*
#define DWARF_FALSE_IF(condition,...) \
	if( condition ) { //bpwarn ( __VA_ARGS__ ); return false; }
#define DWARF_RETURN_IF(condition,...) \
	if( condition ) { //bpwarn ( __VA_ARGS__ ); return; }
#define DWARF_NULL_IF(condition,...) \
	if( condition ) { //bpwarn ( __VA_ARGS__ ); return NULL; }
*/

#define DWARF_FALSE_IF(condition,...) \
	if( condition ) { return false; }
#define DWARF_RETURN_IF(condition,...) \
	if( condition ) { return; }
#define DWARF_NULL_IF(condition,...) \
	if( condition ) { return NULL; }
	
std::string convertCharToString(char *ptr){
    std::string str;
    if(ptr)
        str = ptr;
    else
    	str = "";
    return str;	
}

/* A bound attribute can be either (a constant) or (a reference
   to a DIE which (describes an object containing the bound value) or
   (a constant value itself)). */
bool decipherBound( Dwarf_Debug & dbg, Dwarf_Attribute boundAttribute, std::string &boundString ) {
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
			char bString[40];
			sprintf(bString, "%lu", (unsigned long)constantBound);
			boundString = bString;
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

				char bString[40];
				sprintf(bString, "%lu", (unsigned long)constBoundValue);
				boundString = bString;

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
			//bperr ( "Invalid bound form 0x%x\n", boundForm );
			boundString = "{invalid bound form}";
			return false;
			break;
		} /* end boundForm switch */
	} /* end decipherBound() */

/* We don't have a sane way of dealing with DW_TAG_enumeration bounds, so
   just put the name of the enumeration, or {enum} if none, in the string. */
void parseSubRangeDIE( Dwarf_Debug & dbg, Dwarf_Die subrangeDIE, std::string & loBound, std::string & hiBound, Module * module ) {
	loBound = "{unknown or default}";
	hiBound = "{unknown or default}";

	/* Set the default lower bound, if we know it. */
	switch( module->language() ) {
		case lang_Fortran:
		case lang_Fortran_with_pretty_debug:
		case lang_CMFortran:
			loBound = "1";
			break;
		case lang_C:
		case lang_CPlusPlus:
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
	char * subrangeName = "{anonymous range}"; // type doesn't like NULL.
	status = dwarf_diename( subrangeDIE, & subrangeName, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for name of subrange\n", __FILE__, __LINE__ );
	int dwarvenName = status;

	Dwarf_Off subrangeOffset;
	status = dwarf_dieoffset( subrangeDIE, & subrangeOffset, NULL );
	DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dereferencing DWARF pointer\n", __FILE__, __LINE__ );

	std::string srName = subrangeName;
	Type * rangeType = new typeSubrange( (int) subrangeOffset, 0, atoi(loBound.c_str()), atoi(hiBound.c_str()), srName );
	assert( rangeType != NULL );
	rangeType = module->getModuleTypes()->addOrUpdateType( rangeType );
	if( dwarvenName == DW_DLV_OK ) { dwarf_dealloc( dbg, subrangeName, DW_DLA_STRING ); }	
	} /* end parseSubRangeDIE() */

typeArray *parseMultiDimensionalArray( Dwarf_Debug & dbg, Dwarf_Die range, Type * elementType, Module * module ) {
    char buf[32];
	/* Get the (negative) typeID for this range/subarray. */
	Dwarf_Off dieOffset;
	int status = dwarf_dieoffset( range, & dieOffset, NULL );
	DWARF_NULL_IF( status != DW_DLV_OK, "%s[%d]: error while parsing multidimensional array.\n", __FILE__, __LINE__ );

	/* Determine the range. */
	std::string loBound;
	std::string hiBound;
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
		std::string aName = convertCharToString(buf);
		typeArray* innermostType = new typeArray( elementType, atoi( loBound.c_str() ), atoi( hiBound.c_str() ), aName );
		assert( innermostType != NULL );
		Type * typ = module->getModuleTypes()->addOrUpdateType( innermostType );
		innermostType = dynamic_cast<typeArray *>(typ);
		return innermostType;
		} /* end base-case of recursion. */

	/* If it does, build this array type out of the array type returned from the next recusion. */
	typeArray * innerType = parseMultiDimensionalArray( dbg, nextSibling, elementType, module );
	assert( innerType != NULL );
	// same here - type id ignored    jmo
	std::string aName = convertCharToString(buf);
	typeArray * outerType = new typeArray( innerType, atoi(loBound.c_str()), atoi(hiBound.c_str()), aName);
	assert( outerType != NULL );
	Type *typ = module->getModuleTypes()->addOrUpdateType( outerType );
	outerType = dynamic_cast<typeArray *>(typ);

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
		fprintf( stderr, "0x%lx to 0x%lx; ", (Offset)location.ld_lopc, (Offset)location.ld_hipc );
		}
	fprintf( stderr, "\n" );
	} /* end dumpLocListAddrRanges */

bool decodeLocationListForStaticOffsetOrAddress( Dwarf_Locdesc * locationList, Dwarf_Signed listLength, Symtab *objFile, long int * initialStackValue = NULL,loc_t *loc=NULL) {
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
	if( listLength != 1)
	printf("%s[%d]: unable to decode location lists of non-unit length.\n", __FILE__, __LINE__ );

	if( loc!= NULL ) { 
		loc->stClass = storageAddr;
		loc->refClass = storageNoRef;
		loc->reg = -1;
	}

	/* Initialize the stack. */
	std::stack< long int > opStack = std::stack<long int>();
	if( initialStackValue != NULL ) { opStack.push( * initialStackValue ); }

	/* There is only one location. */
	Dwarf_Locdesc location = locationList[0];
	Dwarf_Loc * locations = location.ld_s;
	for( unsigned int i = 0; i < location.ld_cents; i++ ) {
		/* Handle the literals w/o 32 case statements. */
		if( DW_OP_lit0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_lit31 ) {
			//dwarf_printf( "pushing named constant: %d\n", locations[i].lr_atom - DW_OP_lit0 );
			opStack.push( locations[i].lr_atom - DW_OP_lit0 );
			continue;
			}

		/* Haandle registers w/o 32 case statements. */
		if( DW_OP_reg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_reg31 ) {
			/* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
			//dwarf_printf( "location is named register %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_reg0, objFile) );
			if( loc != NULL ) { 
				loc->stClass = storageRegOffset;
				loc->refClass = storageNoRef;
                		loc->reg = DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_reg0, objFile);
				loc->frameOffset = 0;
				return true;
			}
		}	
			
		/* Haandle registers w/o 32 case statements. */
		if( DW_OP_breg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_breg31 ) {
			//dwarf_printf( "setting storage class to named register, regNum to %d, offset %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_breg0, objFile), locations[i].lr_number );
			if( loc != NULL ) { 
				loc->stClass = storageRegOffset;
				loc->refClass = storageNoRef;
                		loc->reg = DWARF_TO_MACHINE_ENC(locations[i].lr_atom - DW_OP_breg0, objFile);
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
				//dwarf_printf( "pushing unsigned constant %lu\n", (unsigned long)locations[i].lr_number );
				opStack.push( (Dwarf_Unsigned)locations[i].lr_number );
				break;

			case DW_OP_const1s:
			case DW_OP_const2s:
			case DW_OP_const4s:
			case DW_OP_const8s:
			case DW_OP_consts:
				//dwarf_printf( "pushing signed constant %ld\n", (signed long)(locations[i].lr_number) );
				opStack.push( (Dwarf_Signed)(locations[i].lr_number) );
				break;

			case DW_OP_regx:
				/* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
				//dwarf_printf( "location is register %d\n", DWARF_TO_MACHINE_ENC(locations[i].lr_number, objFile) );
				if( loc != NULL ) { 
					loc->stClass = storageRegOffset;
					loc->refClass = storageNoRef;
                    			loc->reg = DWARF_TO_MACHINE_ENC(locations[i].lr_number, objFile); 
					loc->frameOffset = 0;
				}
				return true;

			case DW_OP_fbreg:
				//dwarf_printf( "setting storage class to frame base\n" );
				//if( storageClass != NULL ) { * storageClass = storageFrameOffset; }
				if( loc != NULL ) { 
					loc->stClass = storageRegOffset;
					loc->refClass = storageNoRef;
				}
				opStack.push( locations[i].lr_number );
				break;

			case DW_OP_bregx:
				//dwarf_printf( "setting storage class to register, regNum to %d\n", locations[i].lr_number );
				if( loc != NULL ) { 
					loc->stClass = storageRegOffset;
					loc->refClass = storageNoRef;
                    			loc->reg = DWARF_TO_MACHINE_ENC( locations[i].lr_number, objFile );
					loc->frameOffset = 0;
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
				//bperr ( "Warning: dyninst does not handle multi-part variables.\n" );
				break;

			case DW_OP_nop:
				break;

			default:
				//dwarf_printf( "Unrecognized or non-static location opcode 0x%x, aborting.\n", locations[i].lr_atom );
				return false;
			} /* end operand switch */
		} /* end iteration over Dwarf_Loc entries. */

	/* The top of the stack is the computed location. */
	if( opStack.empty() ) {
		//dwarf_printf( "ignoring malformed location list (stack empty at end of list).\n" );
		return false;
		}
	//dwarf_printf( "setting offset to %d\n", opStack.top() );
	loc->frameOffset = opStack.top();
	
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

	//bperr ( "DIE %s has attributes:", entryName );
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


bool walkDwarvenTree(Dwarf_Debug & dbg, Dwarf_Die dieEntry,
                        Module * module,
                        Symtab * objFile,
                        Dwarf_Off cuOffset,
		            	char **srcFiles,
                        Symbol * currentFunction = NULL,
                        typeCommon * currentCommonBlock = NULL,
        			    typeEnum *currentEnum = NULL,
                        fieldListType * currentEnclosure = NULL )
{

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

	//dwarf_printf( "Considering DIE at %lu (%lu CU-relative) with tag 0x%x\n", (unsigned long)dieOffset, (unsigned long)dieCUOffset, dieTag );
	
	
	/* If this entry is a function, common block, or structure (class),
	   its children will be in its scope, rather than its
	   enclosing scope. */
	Symbol * newFunction = currentFunction;
	typeCommon * newCommonBlock = currentCommonBlock;
	typeEnum *newEnum = currentEnum;
	fieldListType * newEnclosure = currentEnclosure;

	bool parsedChild = false;
	/* Is this is an entry we're interested in? */
	switch( dieTag ) {
		/* case DW_TAG_inline_subroutine: we don't care about these */
		case DW_TAG_subprogram:
		case DW_TAG_entry_point:
		{

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
				//dwarf_printf( "Warning: anonymous function (type %lu).\n", (unsigned long)dieOffset );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
			} /* end if there's no name at all. */

			/* Try to find the function by its mangled name. */
			Dwarf_Addr baseAddr = 0;
			std::vector< Symbol * > ret_funcs;
			std::vector<Symbol *> functions;
			if (objFile->findSymbolByType(ret_funcs, functionName, Symbol::ST_FUNCTION, true)) {
				for (unsigned foo = 0; foo < ret_funcs.size(); foo++)
					functions.push_back(ret_funcs[foo]);
				ret_funcs.clear();	
			}
			else {
				/* If we can't find it by mangled name, try searching by address. */
				status = dwarf_lowpc( dieEntry, & baseAddr, NULL );
				if( status == DW_DLV_OK ) {
					/* The base addresses in DWARF appear to be image-relative. */
					// Symbol * intFunction = objFile->findFuncByAddr( baseAddr );
					Offset absAddr = objFile->getBaseOffset() + baseAddr;
					std::vector<Symbol *>funcs;
					if(objFile->findFuncByEntryOffset(funcs, absAddr)){
					    functions.push_back(funcs[0]);
					}
				}
			}
			if( functions.size() == 0 ) { // Still....
				/* If we can't find it by address, try searching by pretty name. */
				if( baseAddr != 0 ) { //dwarf_printf( "%s[%d]: unable to locate function %s by address 0x%llx\n", __FILE__, __LINE__, functionName, baseAddr ); 
				}
				std::vector<Symbol *> prettyFuncs;
				if (objFile->findSymbolByType(prettyFuncs, functionName, Symbol::ST_FUNCTION)) {
					for (unsigned bar = 0; bar < prettyFuncs.size(); bar++) {
						functions.push_back(prettyFuncs[bar]);
					}
				}
			}
			
			if( functions.size() == 0 ) {
				/* Don't parse the children, since we can't add them. */
				//dwarf_printf( "Failed to find function '%s'\n", functionName );
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;
			}
			else if( functions.size() > 1 ) {
				//dwarf_printf( "Warning: found more than one function '%s', unable to do anything reasonable.\n", functionName );

				/* Don't parse the children, since we can't add them. */
				parsedChild = true;

				if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
				dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
				break;		
			}
			else {
				Symbol * newIntFunction = functions[0];
				assert( newIntFunction != NULL );
				//newFunction = objFile->newFunctionCB( newIntFunction );
				newFunction = newIntFunction;
				assert( newFunction != NULL );
			} /* end findFunction() cases */

			/* Once we've found the Symbol pointer corresponding to this
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
				
				deallocateLocationList( dbg, locationList, listLength );
			} /* end if this DIE has a frame base attribute */
			
			/* Find its return type. */
			Dwarf_Attribute typeAttribute;
			status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Type * returnType = NULL;
			if( status == DW_DLV_NO_ENTRY ) { 
				returnType = module->getModuleTypes()->findType("void");
				newFunction->setReturnType( returnType );
				} /* end if the return type is void */
			else {
				/* There's a return type attribute. */
				Dwarf_Off typeOffset;
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				//parsing_printf("%s/%d: ret type %d\n",
				//			   __FILE__, __LINE__, typeOffset);
				returnType = module->getModuleTypes()->findOrCreateType( typeOffset );
				newFunction->setReturnType( returnType );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if not a void return type */

			/* If this is a member function, add it as a field, for backward compatibility */
			if( currentEnclosure != NULL ) {
				/* Using the mangled name allows us to distinguish between overridden
				   functions, but confuses the tests.  Since Type uses vectors
				   to hold field names, however, duplicate -- demangled names -- are OK. */
				char * demangledName = P_cplus_demangle( functionName, objFile->isNativeCompiler() );

				char * leftMost = NULL;
				if( demangledName == NULL ) {
					//dwarf_printf( "%s[%d]: unable to demangle '%s', using mangled name.\n", __FILE__, __LINE__, functionName );
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
				std::string fName = convertCharToString(leftMost);
				typeFunction *funcType = new typeFunction( dieOffset, returnType, fName);

				currentEnclosure->addField( fName, funcType);
				free( demangledName );
				}

			if( hasSpecification ) { dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); }
			dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
			} break;


		case DW_TAG_common_block: {
			char * commonBlockName;
			status = dwarf_diename( dieEntry, & commonBlockName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	
			Symbol *commonBlockVar;
			std::vector<Symbol *> commonBlockVars;
			if(!objFile->findSymbolByType(commonBlockVars, commonBlockName, Symbol::ST_OBJECT))
			{
			    if(!objFile->findSymbolByType(commonBlockVars, commonBlockName, Symbol::ST_OBJECT, true))
			    {
			   	//pgcc 6 is naming common blocks with a trailing underscore
			   	std::string cbvname = std::string(commonBlockName) + std::string("_");
				if(!objFile->findSymbolByType(commonBlockVars, cbvname, Symbol::ST_OBJECT)){
			    		objFile->findSymbolByType(commonBlockVars, cbvname, Symbol::ST_OBJECT, true);
				}
			    }	
			}
	
			commonBlockVar = commonBlockVars[0];

			DWARF_FALSE_IF( !commonBlockVar, "%s[%d]: Couldn't find variable for common block\n", __FILE__, __LINE__);

			Type * commonBlockType = NULL;
			
			std::string cBName = commonBlockName;
			if (!commonBlockVar) {
  	                    //bperr("unable to find variable %s\n", commonBlockName);
			} else {
			    commonBlockType = dynamic_cast<typeCommon *>(module->getModuleTypes()->findVariableType(cBName));
	                    if (commonBlockType == NULL) {
				commonBlockType = new typeCommon( dieOffset, cBName );
				assert( commonBlockType != NULL );
				module->getModuleTypes()->addGlobalVariable( cBName, commonBlockType );
			    }	
			}
			dwarf_dealloc( dbg, commonBlockName, DW_DLA_STRING );

			/* This node's children are in the common block. */
			newCommonBlock = dynamic_cast<typeCommon*>(commonBlockType);
			if (newCommonBlock)
			   newCommonBlock->beginCommonBlock();
			} break;

		case DW_TAG_constant: {
			//bperr ( "Warning: dyninst ignores named constant entries.\n" );
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
				   so the Type look-ups by it rather than name. */
				//dwarf_printf( "%s/%d: %s/%d\n", __FILE__, __LINE__, variableName, typeOffset );
				Type * variableType = module->getModuleTypes()->findOrCreateType( typeOffset );
				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				
				/* Tell Dyninst what this global's type is. */
				std::string vName = convertCharToString(variableName);
				module->getModuleTypes()->addGlobalVariable( vName, variableType );
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
				

				loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
				bool decodedAddressOrOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, NULL, loc);
				deallocateLocationList( dbg, locationList, listLength );
				
				if( ! decodedAddressOrOffset ) { break; }
			
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
				supportedLanguages lang = module->language();
				if ( ( lang == lang_Fortran ||
					 lang == lang_CMFortran ||
					 lang == lang_Fortran_with_pretty_debug ) &&
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
				   so the Type look-ups by it rather than name. */
				Type * variableType = module->getModuleTypes()->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
			
				/* Acquire the variable's lineNo. */
				Dwarf_Unsigned variableLineNo;
				bool hasLineNumber = false;
				std::string fileName;

				Dwarf_Attribute fileDeclAttribute;
				status = dwarf_attr( specEntry, DW_AT_decl_file, & fileDeclAttribute, NULL );
				DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				Dwarf_Unsigned fileNameDeclVal;
				if( status == DW_DLV_OK ) {
					status = dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL);
					DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
					dwarf_dealloc( dbg, fileDeclAttribute, DW_DLA_ATTR );			
				}
				if( status == DW_DLV_NO_ENTRY )
					fileName = "";
				else	
					fileName = convertCharToString(srcFiles[fileNameDeclVal-1]);

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
				//dwarf_printf( "localVariable '%s', currentFunction %p\n", variableName, currentFunction );
				if( currentFunction != NULL ) {
                    		    if(!hasLineNumber){
                        		variableLineNo = 0;
					fileName = "";
				    }
				    else
				    	fileName = srcFiles[fileNameDeclVal-1];
				    std::string vName = convertCharToString(variableName);
				    localVar * newVariable = new localVar( vName, variableType, fileName, variableLineNo);
				    newVariable->addLocation(loc);
				    if(!currentFunction->vars_)
			    	        currentFunction->vars_ = new localVarCollection();
				    currentFunction->vars_->addLocalVar( newVariable );
				} /* end if a local or static variable. */
				else if( currentEnclosure != NULL ) {
					//assert( sClass != storageFrameOffset );
					assert( loc->stClass != storageRegOffset );
					std::string vName = convertCharToString(variableName);
					currentEnclosure->addField( vName, variableType, loc->frameOffset);
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
				//dwarf_printf( "%s[%d]: ignoring formal parameter without corresponding function.\n", __FILE__, __LINE__ );
				break;
				}
			
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
				//dwarf_printf( "%s[%d]: ignoring formal parameter without location.\n", __FILE__, __LINE__ );
				break;
				}
		
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
				//dwarf_printf( "%s[%d]: ignoring formal parameter with bogus location.\n", __FILE__, __LINE__ );
				break;
				}
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
			bool decodedOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, NULL, loc );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedOffset ) {
				//dwarf_printf( "%s[%d]: ignoring formal parameter with undecodable location.\n", __FILE__, __LINE__ );
				break;
				}
			
			assert( loc->stClass != storageAddr );

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
			   so the Type look-ups by it rather than name. */
			//dwarf_printf( "%s[%d]: found formal parameter %s with type %ld\n", __FILE__, __LINE__, parameterName, typeOffset );
			Type * parameterType = module->getModuleTypes()->findOrCreateType( typeOffset );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );


			Dwarf_Attribute fileDeclAttribute;
			std::string fileName;
			status = dwarf_attr( originEntry, DW_AT_decl_file, & fileDeclAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			Dwarf_Unsigned fileNameDeclVal;
			if( status == DW_DLV_OK ) {
				status = dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL);
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				dwarf_dealloc( dbg, fileDeclAttribute, DW_DLA_ATTR );
			}
			if( status == DW_DLV_NO_ENTRY )
				fileName = "";
			else	
				fileName = convertCharToString(srcFiles[fileNameDeclVal-1]);
			
			/* Acquire the parameter's lineNo. */
			Dwarf_Attribute lineNoAttribute;
			Dwarf_Unsigned parameterLineNo;
			status = dwarf_attr( originEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			
			if( status == DW_DLV_NO_ENTRY ) {
				//dwarf_printf( "%s[%d]: ignoring formal parameter without line number.\n", __FILE__, __LINE__ );
				parameterLineNo = 0;
			}
			else{
				status = dwarf_formudata( lineNoAttribute, & parameterLineNo, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );
			}	
			
			
			/* We now have the parameter's location, name, type, and line number.
			   Tell Dyninst about it. */
			std::string pName = convertCharToString(parameterName);
			localVar * newParameter = new localVar( pName, parameterType, fileName, parameterLineNo);
			assert( newParameter != NULL );
			newParameter->addLocation(loc);
			
			/* This is just brutally ugly.  Why don't we take care of this invariant automatically? */
			if(!currentFunction->params_)
			    currentFunction->params_ = new localVarCollection();
			currentFunction->params_->addLocalVar( newParameter );
			
			//TODO ??NOT REQUIRED??
			//currentFunction->addParam( parameterName, parameterType, parameterLineNo, parameterOffset );

			//dwarf_printf( "%s[%d]: added formal parameter '%s' (at FP + %ld) of type %p from line %lu.\n", __FILE__, __LINE__, parameterName, parameterOffset, parameterType, (unsigned long)parameterLineNo );
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
			std::string tName = convertCharToString(typeName);   
			Type * baseType = new typeScalar( dieOffset, byteSize, tName );
			assert( baseType != NULL );

			/* Add the basic type to our collection. */
			//dwarf_printf( "Adding base type '%s' (%lu) of size %lu to type collection %p\n", typeName, (unsigned long)dieOffset, (unsigned long)byteSize, module->getModuleTypes() );
			baseType = module->getModuleTypes()->addOrUpdateType( baseType );
			
			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
			} break;

		case DW_TAG_typedef: {
			char * definedName = NULL;
			status = dwarf_diename( dieEntry, & definedName, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Type * referencedType = NULL;
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
				//parsing_printf("%s/%d: %s/%d\n",
				//			   __FILE__, __LINE__, definedName, typeOffset);
				referencedType = module->getModuleTypes()->findOrCreateType( typeOffset );
				}

			/* Add the typedef to our collection. */
			// //bperr ( "Adding typedef: '%s' as %lu (pointing to %lu)\n", definedName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			std::string dName = convertCharToString(definedName);
		  	Type * typedefType = new typeTypedef( dieOffset, referencedType, dName);
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
			
			//parsing_printf("%s/%d: %s/%d\n",
			//			   __FILE__, __LINE__, arrayName, typeOffset);
			Type * elementType = module->getModuleTypes()->findOrCreateType( typeOffset );

			/* Find the range(s) of the elements. */
			Dwarf_Die firstRange;
			status = dwarf_child( dieEntry, & firstRange, NULL );
			DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
			typeArray * baseArrayType = parseMultiDimensionalArray( dbg, firstRange, elementType, module );
			assert( baseArrayType != NULL );

			/* The baseArrayType is an anonymous type with its own typeID.  Extract
			   the information and add an array type for this DIE. */
			std::string aName = convertCharToString(arrayName);
			Type * arrayType = new typeArray( dieOffset, baseArrayType->getBaseType(), baseArrayType->getLow(),
											baseArrayType->getHigh(), aName);
			assert( arrayType != NULL );
			//setArraySize( arrayType, baseArrayType->getLow(), baseArrayType->getHigh() );
			// //bperr ( "Adding array type '%s' (%lu) [%s, %s] @ %p\n", arrayName, (unsigned long)dieOffset, baseArrayType->getLow(), baseArrayType->getHigh(), arrayType );
			arrayType = module->getModuleTypes()->addOrUpdateType( arrayType );

			/* Don't parse the children again. */
			parsedChild = true;

			dwarf_dealloc( dbg, firstRange, DW_DLA_DIE );
			dwarf_dealloc( dbg, arrayName, DW_DLA_STRING );
		} break;

		case DW_TAG_subrange_type: {
			std::string loBound;
			std::string hiBound;
			parseSubRangeDIE( dbg, dieEntry, loBound, hiBound, module );
		} break;

		case DW_TAG_enumeration_type: {
			char * typeName = NULL;
			status = dwarf_diename( dieEntry, & typeName, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			std::string tName = convertCharToString(typeName);
			typeEnum* enumerationType = new typeEnum( dieOffset, tName);
			assert( enumerationType != NULL );
			enumerationType = dynamic_cast<typeEnum *>(module->getModuleTypes()->addOrUpdateType( enumerationType ));
			newEnum = enumerationType;

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

			//parsing_printf("%s/%d: inherited %d\n",
			//			   __FILE__, __LINE__, scOffset);
			Type * superClass = module->getModuleTypes()->findOrCreateType( scOffset );

			/* Acquire the visibility, if any.  DWARF calls it accessibility
			   to distinguish it from symbol table visibility. */
			Dwarf_Attribute visAttr;
			status = dwarf_attr( dieEntry, DW_AT_accessibility, & visAttr, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			visibility_t visibility = visPrivate;
			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned visValue;
				status = dwarf_formudata( visAttr, & visValue, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

				switch( visValue ) {
					case DW_ACCESS_public: visibility = visPublic; break;
					case DW_ACCESS_protected: visibility = visProtected; break;
					case DW_ACCESS_private: visibility = visPrivate; break;
					default:
						//bperr ( "Uknown visibility, ignoring.\n" );
						break;
					} /* end visibility switch */

				dwarf_dealloc( dbg, visAttr, DW_DLA_ATTR );
				} /* end if the visibility is specified. */

			/* Add a readily-recognizable 'bad' field to represent the superclass.
			   Type::getComponents() will Do the Right Thing. */
			std::string fName = "{superclass}";
			currentEnclosure->addField( fName, superClass, -1, visibility );
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

			fieldListType * containingType = NULL;
			std::string tName = convertCharToString(typeName);
			switch ( dieTag ) {
				case DW_TAG_structure_type: 
				case DW_TAG_class_type: 
				   containingType = new typeStruct( dieOffset, tName);
				   break;
				case DW_TAG_union_type: 
				   containingType = new typeUnion( dieOffset, tName);
				   break;
				}
			
			assert( containingType != NULL );
			// //bperr ( "Adding structure, union, or class type '%s' (%lu)\n", typeName, (unsigned long)dieOffset );
			containingType = dynamic_cast<fieldListType *>(module->getModuleTypes()->addOrUpdateType( containingType ));
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

			// //bperr ( "Adding enum '%s' (%ld) to enumeration '%s' (%d)\n", enumName, (signed long)enumValue, currentEnclosure->getName(), currentEnclosure->getID() );
			currentEnum->addConstant( enumName, enumValue );

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
			
			//parsing_printf("%s/%d: %s/%d\n",
			//			   __FILE__, __LINE__, memberName, typeOffset);
			Type * memberType = module->getModuleTypes()->findOrCreateType( typeOffset );
				
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

			loc_t *loc = (loc_t *)malloc(sizeof(loc_t));
			long int baseAddress = 0;
			bool decodedAddress = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, & baseAddress, loc );
			deallocateLocationList( dbg, locationList, listLength );
			
			if( ! decodedAddress ) { break; }
			assert( decodedAddress );
			
			assert( loc->stClass == storageAddr );

			/* DWARF stores offsets in bytes unless the member is a bit field.
			   Correct memberOffset as indicated.  Also, memberSize is in bytes
			   from the underlying type, not the # of bits used from it, so
			   correct that as necessary as well. */
			long int memberSize = memberType->getSize();

			Dwarf_Attribute bitOffset;
			status = dwarf_attr( dieEntry, DW_AT_bit_offset, & bitOffset, NULL );
			DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			if( status == DW_DLV_OK ) {
				Dwarf_Unsigned memberOffset_du = loc->frameOffset;
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
				loc->frameOffset *= 8;
				memberSize *= 8;
				} /* end if not a bit field member. */

			if( memberName != NULL ) {
				// /* DEBUG */ fprint( stderr, "Adding member to enclosure '%s' (%d): '%s' with type %lu at %ld and size %d\n", currentEnclosure->getName(), currentEnclosure->getID(), memberName, (unsigned long)typeOffset, loc->frameOffset, memberType->getSize() );
				std::string fName = convertCharToString(memberName);
				currentEnclosure->addField( fName, memberType, loc->frameOffset);
				dwarf_dealloc( dbg, memberName, DW_DLA_STRING );
				} else {
				/* An anonymous union [in a struct]. */
				std::string fName = "[anonymous union]";
				currentEnclosure->addField( fName, memberType, loc->frameOffset);
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
			Type * typeModified = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typeModified = module->getModuleTypes()->findType( "void" );
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				//parsing_printf("%s/%d: %s/%d\n",
				//			   __FILE__, __LINE__, typeName, typeOffset);
				typeModified = module->getModuleTypes()->findOrCreateType( typeOffset );
				typeSize = typeModified->getSize();

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typeModified is not void */

			// I'm taking out the type qualifiers for right now
			std::string tName = convertCharToString(typeName);

//			Type * modifierType = new Type( typeName, dieOffset, TypeAttrib, typeSize, typeModified, dieTag );
			Type * modifierType = new typeTypedef(dieOffset, typeModified, tName);
			assert( modifierType != NULL );
			// //bperr ( "Adding modifier type '%s' (%lu) modifying (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
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
			Type * typePointedTo = NULL;
			if( status == DW_DLV_NO_ENTRY ) {
				/* Presumably, a pointer or reference to void. */
				typePointedTo = module->getModuleTypes()->findType("void");
				} else {			
				status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
				DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
				//parsing_printf("%s/%d: %s/%d\n",
				//			   __FILE__, __LINE__, typeName, typeOffset);
				typePointedTo = module->getModuleTypes()->findOrCreateType( typeOffset );

				dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
				} /* end if typePointedTo is not void */

			Type * indirectType = NULL;
			std::string tName = convertCharToString(typeName);
			switch ( dieTag ) {
			case DW_TAG_subroutine_type:
			   indirectType = new typeFunction(dieOffset, typePointedTo, tName);
			   break;
			case DW_TAG_ptr_to_member_type:
			case DW_TAG_pointer_type:
			   indirectType = new typePointer(dieOffset, typePointedTo, tName);
			   break;
			case DW_TAG_reference_type:
			   indirectType = new typeRef(dieOffset, typePointedTo, tName);
			   break;
			}

			assert( indirectType != NULL );
			//dwarf_printf( "Adding indirect type '%s' (%lu) pointing to (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
			indirectType = module->getModuleTypes()->addOrUpdateType( indirectType );

			dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
		} break;

		case DW_TAG_variant_part:
			/* We don't support this (Pascal) type. */
		case DW_TAG_string_type:
			/* We don't support this (FORTRAN) type. */
		default:
			/* Nothing of interest. */
			// //bperr ( "Entry %lu with tag 0x%x ignored.\n", (unsigned long)dieOffset, dieTag );
			break;
		} /* end dieTag switch */

	/* Recurse to its child, if any. */
	Dwarf_Die childDwarf;
	status = dwarf_child( dieEntry, & childDwarf, NULL );
	DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	
	if( status == DW_DLV_OK && parsedChild == false ) {		
		walkDwarvenTree( dbg, childDwarf, module, objFile, cuOffset, srcFiles, newFunction, newCommonBlock, newEnum, newEnclosure );
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

void Object::parseDwarfTypes( Symtab *objFile) {
	// Man do we do to a lot of trouble for this...
/*	
	assert( moduleTypes );

	if( moduleTypes->dwarfParsed() ) {
		//dwarf_printf( "%s[%d]: already parsed %s, moduleTypes = %p\n", __FILE__, __LINE__, fileName, moduleTypes );
		std::vector<Symbol *> * bpfuncs = getProcedures( true );
		assert( bpfuncs );
		for( unsigned int i = 0; i < bpfuncs->size(); i++ ) {
			(*bpfuncs)[i]->fixupUnknown( this );
			}
		return;
		}
*/		
	//dwarf_printf( "%s[%d]: parsing %s...\n", __FILE__, __LINE__, fileName );

	/* Start the dwarven debugging. */
	Dwarf_Debug dbg;
	Module *mod = NULL;

	int status = dwarf_elf_init( elfHdr.e_elfp(), DW_DLC_READ, & pd_dwarf_handler, getErrFunc(), & dbg, NULL );
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
		//dwarf_printf( "%s[%d]: considering compilation unit '%s'\n", __FILE__, __LINE__, moduleName );

		/* Set the language, if any. */
		Dwarf_Attribute languageAttribute;
		status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error acquiring language attribute.\n", __FILE__, __LINE__ );

		/* Iterate over the tree rooted here; walkDwarvenTree() deallocates the passed-in DIE. */
		if(!objFile->findModule(mod, moduleName)){
		    std::string modName = moduleName;
		    std::string fName = extract_pathname_tail(modName);
		    if(!objFile->findModule(mod, fName))
		    {
		    	modName = objFile->file();
			if(!objFile->findModule(mod, modName))
			    continue;
		    }
		}
	
		Dwarf_Signed cnt;
		char **srcfiles;
		int status;
		status = dwarf_srcfiles(moduleDIE, &srcfiles,&cnt, NULL);
		DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error acquiring source file names.\n", __FILE__, __LINE__ );
		
		if( !walkDwarvenTree( dbg, moduleDIE, mod, objFile, cuOffset, srcfiles ) ) {
			//bpwarn ( "Error while parsing DWARF info for module '%s'.\n", moduleName );
			return;
			}
		if(status == DW_DLV_OK){
			for (unsigned i = 0; i < cnt; ++i) {
				/* use srcfiles[i] */
				dwarf_dealloc(dbg, srcfiles[i], DW_DLA_STRING);
			}
			dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
		}

		dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
	} /* end iteration over compilation-unit headers. */

	if(!mod)
	    return;

        /* Fix type list. */
        typeCollection *moduleTypes = mod->getModuleTypes();
	assert(moduleTypes);
        hash_map< int, Type * >::iterator typeIter =  moduleTypes->typesByID.begin();
	for(;typeIter!=moduleTypes->typesByID.end();typeIter++){
	    typeIter->second->fixupUnknowns(mod);
	} /* end iteratation over types. */
	
        /* Fix the types of variables. */   
        std::string variableName;
        hash_map< std::string, Type * >::iterator variableIter = moduleTypes->globalVarsByName.begin();
        for(;variableIter!=moduleTypes->globalVarsByName.end();variableIter++){ 
	    if(variableIter->second->getDataClass() == dataUnknownType && 
  	        moduleTypes->findType( variableIter->second->getID() ) != NULL ) {
	        moduleTypes->globalVarsByName[ variableIter->first ] = moduleTypes->findType( variableIter->second->getID() );
	    } /* end if data class is unknown but the type exists. */
	} /* end iteration over variables. */


// Do Not clean up Elf. We need the Elf pointer when we are writing back to a file again
// writeBackSymbols will not work - giri(7/26/2007)

#if 0
	/* Clean up. */
	Elf * dwarfElf;
	status = dwarf_get_elf( dbg, & dwarfElf, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error during libdwarf cleanup.\n", __FILE__, __LINE__ );
#endif	

	status = dwarf_finish( dbg, NULL );
	DWARF_RETURN_IF( status != DW_DLV_OK, "%s[%d]: error during libdwarf cleanup.\n", __FILE__, __LINE__ );

//FIX THIS: Have to be moved, where this is going to be done on a module-by-module basis
#if 0
	/* Fix the type references in functions. */
	std::vector<Symbol *> funcs;
	getAllSymbolsByType(funcs, Symbol::ST_FUNCTION);
	for( unsigned int i = 0; i < funcs->size(); i++ ) {
		funcs[i].fixupUnknown( this );
	}
#endif	
	moduleTypes->setDwarfParsed();

} /* end parseDwarfTypes() */
