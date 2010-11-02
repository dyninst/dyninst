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
#include <map>

#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "libdwarf.h"

#include "Symtab.h"
#include "Type.h"
#include "Function.h"
#include "Module.h"
#include "symtabAPI/src/Object.h"
#include "Collections.h"
#include "common/h/pathName.h"
#include "Variable.h"
#include "Type-mem.h"
#include <stdarg.h>
#include "dynutil/h/Annotatable.h"
#include "annotations.h"

#ifndef DW_FRAME_CFA_COL3
//  This is a newer feature of libdwarf (which has been causing some other 
//  compilation problems locally) -- so we just fudge it for the moment
#define DW_FRAME_CFA_COL3               1036
/* Use this to get the cfa. */
extern "C" {
int dwarf_get_fde_info_for_cfa_reg3(
		Dwarf_Fde /*fde*/,
		Dwarf_Addr       /*pc_requested*/, 
		Dwarf_Small  *   /*value_type*/, 
		Dwarf_Signed *   /*offset_relevant*/,
		Dwarf_Signed *    /*register*/,  
		Dwarf_Signed *    /*offset_or_block_len*/,
		Dwarf_Ptr   *    /*block_ptr */,
		Dwarf_Addr*      /*row_pc_out*/,
		Dwarf_Error*     /*error*/)
{
	fprintf(stderr, "%s[%d]:  WARNING:  inside dummy dwarf functions\n", FILE__, __LINE__);
	return 0;
}
}
#endif

std::map<Dwarf_Off, fieldListType*> enclosureMap;

int dwarf_printf(const char *format, ...);

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

void setSymtabError(SymtabError new_err);

/* For location decode. */
#include <stack>

// on 64-bit x86_64 targets, the DWARF register number does not
// correspond to the machine encoding. See the AMD-64 ABI.


/*
  #define DWARF_FALSE_IF(condition,...) \
  if ( condition ) { //bpwarn ( __VA_ARGS__ ); return false; }
  #define DWARF_RETURN_IF(condition,...) \
  if ( condition ) { //bpwarn ( __VA_ARGS__ ); return; }
  #define DWARF_NULL_IF(condition,...) \
  if ( condition ) { //bpwarn ( __VA_ARGS__ ); return NULL; }
*/

#include "common/h/dwarfExpr.h"

std::string convertCharToString(char *ptr)
{
  std::string str;
  if (ptr)
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
  case DW_FORM_udata: 
    {
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
  case DW_FORM_ref_udata: 
    {
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

      if ( status == DW_DLV_OK ) {
	boundString = boundName;

	dwarf_dealloc( dbg, boundName, DW_DLA_STRING );
	return true;
      }

      /* Does it describe a nameless constant? */
      Dwarf_Attribute constBoundAttribute;
      status = dwarf_attr( boundEntry, DW_AT_const_value, & constBoundAttribute, NULL );
      DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error checking for constant value of bounds attribute.\n", __FILE__, __LINE__ );

      if ( status == DW_DLV_OK ) {
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
void parseSubRangeDIE( Dwarf_Debug & dbg, Dwarf_Die subrangeDIE, 
		       std::string & loBound, std::string & hiBound, Module * module, typeCollection *tc ) 
{
  loBound = "{unknown or default}";
  hiBound = "{unknown or default}";

  /* Set the default lower bound, if we know it. */
  switch ( module->language() ) {
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
  if ( subrangeTag == DW_TAG_enumeration_type ) {
    /* FIXME? First child of enumeration type is lowest, last is highest. */
    char * enumerationName = NULL;
    status = dwarf_diename( subrangeDIE, & enumerationName, NULL );
    DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error cehcking for name of enumeration.\n", __FILE__, __LINE__ );

    if ( enumerationName != NULL ) {
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

  if ( status == DW_DLV_OK ) {
    decipherBound( dbg, lowerBoundAttribute, loBound );
    dwarf_dealloc( dbg, lowerBoundAttribute, DW_DLA_ATTR );
  } /* end if we found a lower bound. */
      
  /* Look for the upper bound. */
  Dwarf_Attribute upperBoundAttribute;
  status = dwarf_attr( subrangeDIE, DW_AT_upper_bound, & upperBoundAttribute, NULL );
  DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for upper bound of subrange\n", __FILE__, __LINE__ );
  if ( status == DW_DLV_NO_ENTRY ) {
    status = dwarf_attr( subrangeDIE, DW_AT_count, & upperBoundAttribute, NULL );
    DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for count of subrange\n", __FILE__, __LINE__ );
  }
  if ( status == DW_DLV_OK ) {
    decipherBound( dbg, upperBoundAttribute, hiBound );
    dwarf_dealloc( dbg, upperBoundAttribute, DW_DLA_ATTR );
  } /* end if we found an upper bound or count. */

  /* Construct the range type. */
  static char * subrangeName = NULL;
  if (!subrangeName)
    subrangeName = strdup("{anonymous range}");
  status = dwarf_diename( subrangeDIE, & subrangeName, NULL );
  DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error while checking for name of subrange\n", __FILE__, __LINE__ );
  int dwarvenName = status;

  Dwarf_Off subrangeOffset;
  status = dwarf_dieoffset( subrangeDIE, & subrangeOffset, NULL );
  DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error dereferencing DWARF pointer\n", __FILE__, __LINE__ );

  errno = 0;
  unsigned long low_conv = strtoul(loBound.c_str(), NULL, 10);
  if (errno)
  {
	  dwarf_printf("%s[%d]:  module %s: error converting range limit '%s' to long: %s\n",
			  FILE__, __LINE__, module->fileName().c_str(), loBound.c_str(), strerror(errno));
	  low_conv = LONG_MIN;
  }

  errno = 0;
  unsigned long hi_conv = strtoul(hiBound.c_str(), NULL, 10);
  if (errno) 
  {
	  dwarf_printf("%s[%d]:  module %s: error converting range limit '%s' to long: %s\n",
			  FILE__, __LINE__, module->fileName().c_str(), hiBound.c_str(), strerror(errno));
	  hi_conv = LONG_MAX;
  }  

  std::string srName = subrangeName;
  typeSubrange * rangeType = new typeSubrange( (int) subrangeOffset, 0, low_conv, hi_conv, srName );
  assert( rangeType != NULL );
  rangeType = tc->addOrUpdateType( rangeType );
  if ( dwarvenName == DW_DLV_OK ) { dwarf_dealloc( dbg, subrangeName, DW_DLA_STRING ); }	
} /* end parseSubRangeDIE() */

typeArray *parseMultiDimensionalArray( Dwarf_Debug & dbg, Dwarf_Die range, 
				       Type * elementType, Module * module, typeCollection *tc) 
{
  char buf[32];
  /* Get the (negative) typeID for this range/subarray. */
  Dwarf_Off dieOffset;
  int status = dwarf_dieoffset( range, & dieOffset, NULL );
  DWARF_NULL_IF( status != DW_DLV_OK, "%s[%d]: error while parsing multidimensional array.\n", __FILE__, __LINE__ );

  /* Determine the range. */
  std::string loBound;
  std::string hiBound;
  parseSubRangeDIE( dbg, range, loBound, hiBound, module, tc);

  /* Does the recursion continue? */
  Dwarf_Die nextSibling;
  status = dwarf_siblingof( dbg, range, & nextSibling, NULL );
  DWARF_NULL_IF( status == DW_DLV_ERROR, "%s[%d]: error checking for second dimension in array.\n", __FILE__, __LINE__ );

  snprintf(buf, 31, "__array%d", (int) dieOffset);

  if ( status == DW_DLV_NO_ENTRY ) {
    /* Terminate the recursion by building an array type out of the elemental type.
       Use the negative dieOffset to avoid conflicts with the range type created
       by parseSubRangeDIE(). */
    // N.B.  I'm going to ignore the type id, and just create an anonymous type here
    std::string aName = convertCharToString(buf);
    typeArray* innermostType = new typeArray( elementType, atoi( loBound.c_str() ), atoi( hiBound.c_str() ), aName );
    assert( innermostType != NULL );
    Type * typ = tc->addOrUpdateType( innermostType );
    innermostType = dynamic_cast<typeArray *>(typ);
    return innermostType;
  } /* end base-case of recursion. */

  /* If it does, build this array type out of the array type returned from the next recusion. */
  typeArray * innerType = parseMultiDimensionalArray( dbg, nextSibling, elementType, module, tc);
  assert( innerType != NULL );
  // same here - type id ignored    jmo
  std::string aName = convertCharToString(buf);
  typeArray * outerType = new typeArray( innerType, atoi(loBound.c_str()), atoi(hiBound.c_str()), aName);
  assert( outerType != NULL );
  Type *typ = tc->addOrUpdateType( outerType );
  outerType = dynamic_cast<typeArray *>(typ);

  dwarf_dealloc( dbg, nextSibling, DW_DLA_DIE );
  return outerType;
} /* end parseMultiDimensionalArray() */

void deallocateLocationList( Dwarf_Debug & dbg, Dwarf_Locdesc * locationList, 
			     Dwarf_Signed listLength ) 
{
  for( int i = 0; i < listLength; i++ ) {
    dwarf_dealloc( dbg, locationList[i].ld_s, DW_DLA_LOC_BLOCK );
  }
  dwarf_dealloc( dbg, locationList, DW_DLA_LOCDESC );
} /* end deallocateLocationList() */

void deallocateLocationList( Dwarf_Debug & dbg, Dwarf_Locdesc ** locationList, 
			     Dwarf_Signed listLength ) 
{
  for( int i = 0; i < listLength; i++ ) {
    dwarf_dealloc( dbg, locationList[i]->ld_s, DW_DLA_LOC_BLOCK );
    dwarf_dealloc( dbg, locationList[i], DW_DLA_LOCDESC );
  }
  dwarf_dealloc( dbg, locationList, DW_DLA_LIST );
} /* end deallocateLocationList() */

/* An investigative function. */
void dumpLocListAddrRanges( Dwarf_Locdesc * locationList, Dwarf_Signed listLength ) 
{
  for( int i = 0; i < listLength; i++ ) {
    Dwarf_Locdesc location = locationList[i];
    fprintf( stderr, "0x%lx to 0x%lx; ", (Offset)location.ld_lopc, (Offset)location.ld_hipc );
  }
  fprintf( stderr, "\n" );
} /* end dumpLocListAddrRanges */

#if defined(arch_x86_64)
int convertFrameBaseToAST( Dwarf_Locdesc * locationList, Dwarf_Signed listLength, 
			   Symtab *proc /* process parameter only needed on x86_64*/) 
#else
  int convertFrameBaseToAST( Dwarf_Locdesc * locationList, Dwarf_Signed listLength, 
			     Symtab * /* process parameter only needed on x86_64*/) 
#endif
{
  /* Until such time as we see more-complicated location lists, assume single entries
     consisting of a register name.  Using an AST for this is massive overkill, but if
     we need to handle more complicated frame base calculations later, the infastructure
     will be in place. */

  /* There is only one location. */
  if (listLength != 1) {
    //bpwarn("%s[%d]: unable to handle location lists of more than one element in frame base.\n", __FILE__, __LINE__);
    return -1 ;
  }

  Dwarf_Locdesc locationDescriptor = locationList[0];

  /* It is defined by a single operation. */
  if (locationDescriptor.ld_cents != 1) {
    //bpwarn("%s[%d]: unable to handle multioperation locations in frame base.\n", __FILE__, __LINE__ );
    return -1;
  }
  Dwarf_Loc location = locationDescriptor.ld_s[0];

  /* That operation is naming a register. */
  int registerNumber = 0;	
  if ( DW_OP_reg0 <= location.lr_atom && location.lr_atom <= DW_OP_reg31 ) {
    registerNumber = DWARF_TO_MACHINE_ENC(location.lr_atom - DW_OP_reg0,
					  proc);
  }
  else if ( DW_OP_breg0 <= location.lr_atom && location.lr_atom <= DW_OP_breg31 ) {
    registerNumber = DWARF_TO_MACHINE_ENC(location.lr_atom - DW_OP_breg0,
					  proc);
    if ( location.lr_number != 0 ) {
      /* Actually, we should be able whip up an AST node for this. */
      return -1;
    }
  }
  else if ( location.lr_atom == DW_OP_regx ) {
    registerNumber = (int) DWARF_TO_MACHINE_ENC(location.lr_number,
						proc);
  }
  else if ( location.lr_atom == DW_OP_bregx ) {
    registerNumber = (int) DWARF_TO_MACHINE_ENC(location.lr_number,
						proc);
    if ( location.lr_number2 != 0 ) {
      /* Actually, we should be able whip up an AST node for this. */
      return -1;
    }
  }
  else {
    return -1;
  }

  return registerNumber;

  /* We have to make sure no arithmetic is actually done to the frame pointer,
     so add zero to it and shove it in some other register. */
  /*	AstNodePtr constantZero = AstNode::operandNode(AstNode::Constant, (void *)0);
    AstNodePtr framePointer = AstNode::operandNode(AstNode::DataReg, (void *)(long unsigned int)registerNumber);
    AstNodePtr moveFPtoDestination = AstNode::operatorNode(plusOp,
    constantZero,
    framePointer);

    return moveFPtoDestination;
  */
} /* end convertFrameBaseToAST(). */

bool decodeLocationListForStaticOffsetOrAddress( Dwarf_Locdesc **locationList, 
						 Dwarf_Signed listLength, 
						 Symtab * objFile, 
						 vector<VariableLocation>& locs, Address lowpc,
						 long int * initialStackValue = NULL)
{
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

  /* We now parse the complete location list for variables and parameters within a
   * function. We still ignore the location list defined for DW_AT_frame_base of the
   * function as the frame pointer is readily available on all platforms(except for IA64)
   * May be we would need to parse the location list for IA64 functions to store the 
   * register numbers and offsets and use it based on the pc value. 
   */
  /*
    DWARF_FALSE_IF( listLength != 1, "%s[%d]: unable to decode location lists of non-unit length.\n", __FILE__, __LINE__ );
    if ( listLength != 1)
    printf("%s[%d]: unable to decode location lists of non-unit length.\n", __FILE__, __LINE__ );
  */
	for (unsigned locIndex = 0 ; locIndex < listLength; locIndex++) 
	{
		bool isLocSet = false;
		VariableLocation loc;
		// Initialize location values.
		loc.stClass = storageAddr;
		loc.refClass = storageNoRef;
		loc.reg = -1;

		/* There is only one location. */
		Dwarf_Locdesc *location = locationList[locIndex];

		loc.lowPC = (Offset)location->ld_lopc;
		loc.hiPC = (Offset)location->ld_hipc;

		// if range of the variable is the entire address space, do not add lowpc
		if (loc.lowPC != (unsigned long) 0 || loc.hiPC != (unsigned long) ~0) 
		{
			loc.lowPC = (Offset)location->ld_lopc + lowpc;
			loc.hiPC = (Offset)location->ld_hipc + lowpc;
		}
		dwarf_printf( "CU lowpc %lx setting lowPC %lx hiPC %lx \n", lowpc, loc.lowPC, loc.hiPC );

      long int end_result = 0;
      bool result = decodeDwarfExpression(location, initialStackValue, &loc, isLocSet, NULL,
                                          objFile->getArchitecture(), end_result);
      if (!result) {
         return false;
      }

		if (!isLocSet)
		{
         dwarf_printf( "setting offset to %d\n", end_result);
         loc.frameOffset = end_result;
         locs.push_back(loc);
		}
		else
			locs.push_back(loc);
	}

	/* decode successful */
	return true;
} /* end decodeLocationListForStaticOffsetOrAddress() */

void convertFileNoToName( Dwarf_Debug & dbg, Dwarf_Signed fileNo, 
		char ** returnFileName, char ** newFileNames = NULL, Dwarf_Signed newFileNamesCount = 0 ) 
{
	static char ** fileNames = NULL;
	static Dwarf_Signed fileNamesCount = 0;

	/* Initialize? */
	if ( returnFileName == NULL && newFileNames != NULL ) {
		/* FIXME?  Did we want to normalize these filenames? */
		fileNames = newFileNames;
		fileNamesCount = newFileNamesCount;
		return;
	} /* end initialization. */

	/* Destroy? */
  if ( returnFileName == NULL && newFileNames == NULL ) {
    for( int i = 0; i < fileNamesCount; i++ ) {
      dwarf_dealloc( dbg, fileNames[i], DW_DLA_STRING );
    } /* end deallocation loop */
    dwarf_dealloc( dbg, fileNames, DW_DLA_LIST );
    fileNamesCount = 0;
    return;
  } /* end destruction. */

  /* Do lookup. */
  if ( fileNo <= fileNamesCount ) { * returnFileName = fileNames[fileNo - 1]; }
  else { * returnFileName = NULL; }
} /* end convertFileNoToName() */

/* Utility function. */
unsigned long tvDifference( struct timeval lhs, struct timeval rhs ) 
{
  unsigned long seconds = lhs.tv_sec - rhs.tv_sec;
  if ( seconds == 0 ) { return lhs.tv_usec - rhs.tv_usec; }
  else {
    seconds *= 1000000;
    seconds += lhs.tv_usec - rhs.tv_usec;
  }
  return seconds;
} /* end tvDifference() */

/* For debugging. */
void dumpAttributeList( Dwarf_Die dieEntry, Dwarf_Debug & dbg ) 
{
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
                     Address lowpc,
                     Function * currentFunction = NULL,
                     typeCommon * currentCommonBlock = NULL,
                     typeEnum *currentEnum = NULL,
                     fieldListType * currentEnclosure = NULL,
                     bool parseSibling = true,
                     int depth = 0
                     )
{
  /* optimization */ tail_recursion:
	typeCollection *tc = typeCollection::getModTypeCollection(module);
  bool walk_error = false;
  Dwarf_Half dieTag;
  int status = dwarf_tag( dieEntry, & dieTag, NULL );
  DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

  Dwarf_Off dieOffset;
  status = dwarf_dieoffset( dieEntry, & dieOffset, NULL );
  DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

  Dwarf_Off dieCUOffset;
  status = dwarf_die_CU_offset( dieEntry, & dieCUOffset, NULL );
  DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

  dwarf_printf( "Considering DIE at %lu (%lu CU-relative) with tag 0x%x at depth %d \n", (unsigned long)dieOffset, (unsigned long)dieCUOffset, dieTag, depth );

  // Map Insert is successful only the first time a dieEntry is encountered.
  enclosureMap.insert(pair <Dwarf_Off, fieldListType*> (dieOffset, currentEnclosure)); 

  /* If this entry is a function, common block, or structure (class),
     its children will be in its scope, rather than its
     enclosing scope. */

  Function * newFunction = currentFunction;

  typeCommon * newCommonBlock = currentCommonBlock;
  typeEnum *newEnum = currentEnum;
  fieldListType * newEnclosure = currentEnclosure;

  Object *elfobj = objFile->getObject();
  if (!elfobj)
  {
	  fprintf(stderr, "%s[%d]:  requested object does not exist!\n", FILE__, __LINE__);
	  return false;
  }

  bool parsedChild = false;

  /* Is this is an entry we're interested in? */
  switch( dieTag ) 
  {
    /* case DW_TAG_inline_subroutine: we don't care about these */
  case DW_TAG_subprogram:
  case DW_TAG_entry_point:
    {
		dwarf_printf(" DW_TAG_subprogram or DW_TAG_entry_point \n");

		/* Our goal is three-fold: First, we want to set the return type
		   of the function.  Second, we want to set the newFunction variable
		   so subsequent entries are handled correctly.  Third, we want to
		   record (the location of, or how to calculate) the frame base of 
		   this function for use by our instrumentation code later. */

		/* If we are revisiting this dieEntry by parsing Parent of specification entry or
		   abstract origin entry, get the original enclosure saved in the map to identify 
		   member functions correctly */

		fieldListType * dieEnclosure = enclosureMap.find(dieOffset)->second;

		char * functionName = NULL;

		Dwarf_Die abstractEntry = dieEntry;

		Dwarf_Bool isAbstractOrigin;
		status = dwarf_hasattr( dieEntry, DW_AT_abstract_origin, & isAbstractOrigin, NULL );

		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
				__FILE__, __LINE__ );

		if ( isAbstractOrigin ) 
		{
			dwarf_printf(" has DW_TAG_abstract_origin \n");

			Dwarf_Attribute abstractAttribute;
			status = dwarf_attr( dieEntry, DW_AT_abstract_origin, & abstractAttribute, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Off abstractOffset;
			status = dwarf_global_formref( abstractAttribute, & abstractOffset, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			status = dwarf_offdie( dbg, abstractOffset, & abstractEntry, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, abstractAttribute, DW_DLA_ATTR );
		} /* end if the function has a specification */

		/* Is this entry specified elsewhere?  We may need to look there for its name. */

		Dwarf_Die specEntry = isAbstractOrigin? abstractEntry : dieEntry;

		Dwarf_Bool hasSpecification;
		status = dwarf_hasattr( specEntry, DW_AT_specification, & hasSpecification, NULL );

		DWARF_NEXT_IF( status != DW_DLV_OK, 
				"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

		/* In order to do this, we need to find the function's (mangled) name.
		   If a function has a specification, its specification will have its
		   name. */

		if ( hasSpecification ) 
		{
			dwarf_printf(" has DW_TAG_specification \n");

			Dwarf_Attribute specAttribute;
			status = dwarf_attr( specEntry, DW_AT_specification, & specAttribute, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			Dwarf_Off specOffset;
			status = dwarf_global_formref( specAttribute, & specOffset, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
		} /* end if the function has a specification */

		/* Prefer linkage names. */

		Dwarf_Attribute linkageNameAttr;
		status = dwarf_attr( specEntry, DW_AT_MIPS_linkage_name, & linkageNameAttr, NULL );

		DWARF_NEXT_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

		bool hasLinkageName;

		if ( status == DW_DLV_OK ) 
		{
			hasLinkageName = true;
			status = dwarf_formstring( linkageNameAttr, & functionName, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, linkageNameAttr, DW_DLA_ATTR );
		} /* end if there's a linkage name. */
		else 
		{
			hasLinkageName = false;

			status = dwarf_diename( specEntry, & functionName, NULL );

			DWARF_NEXT_IF( status == DW_DLV_ERROR, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
		} /* end if there isn't a linkage name. */

		if ( functionName == NULL ) 
		{
			/* I'm not even sure what an anonymous function _means_,
			   but we sure can't do anything with it. */

			dwarf_printf( "Warning: anonymous function (type %lu).\n", (unsigned long)dieOffset );

			/* Don't parse the children, since we can't add them. */
			parsedChild = true;

			dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
			break;
		} /* end if there's no name at all. */

		dwarf_printf(" Function name %s \n", functionName);

		Function * thisFunction = NULL;
		vector<Function *> ret_funcs;

		// newFunction is scoped at the function level...

            Dwarf_Addr baseAddr = 0;
            Offset func_lowpc = 0;
            bool hasLowPC = false;
            status = dwarf_lowpc( dieEntry, & baseAddr, NULL );
            
            if (status == DW_DLV_OK) 
            {
               bool result = elfobj->convertDebugOffset(baseAddr, func_lowpc);
               hasLowPC = result;
            }

		if (objFile->findFunctionsByName(ret_funcs, functionName)) 
		{
			// Assert a single one?
			thisFunction = ret_funcs[0];
			dwarf_printf(" findFunction by name \n");
		}
		else 
		{
			// Didn't find by name (???), try to look up by address...
               if (!hasLowPC || !objFile->findFuncByEntryOffset(newFunction, func_lowpc)) 
			{
                  dwarf_printf( "Failed to find function at %lx -> %lx\n", func_lowpc, func_lowpc);
					break;
				}
			}


		if (thisFunction == NULL && parseSibling== true) 
		{
			dwarf_printf( "Failed to find function '%s'\n", functionName );
			break;
		} 
		else if (thisFunction != NULL && parseSibling == true) 
		{
			newFunction = thisFunction;
		} 
		else if (thisFunction != NULL && parseSibling == false ) 
		{
			// Parsing parents of specEntry or abstractOriginEntry - but the parent aleady has Function associated with it
			// Do not redundantly parse. break
			break;
		}

		/* Once we've found the Symbol pointer corresponding to this
		   DIE, record its frame base.  A declaration entry may not have a 
		   frame base, and some functions do not have frames. */

		Dwarf_Attribute frameBaseAttribute;
		status = dwarf_attr( dieEntry, DW_AT_frame_base, & frameBaseAttribute, NULL );
		
		DWARF_NEXT_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

		if ( status == DW_DLV_OK ) 
		{
			dwarf_printf(" Frame Pointer available \n");
			Dwarf_Locdesc ** locationList;
			Dwarf_Signed listLength;

			status = dwarf_loclist_n( frameBaseAttribute, & locationList, & listLength, NULL );
			
			if ( status != DW_DLV_OK ) 
			{
				/* I think DWARF 3 generically allows this abomination of empty loclists. */
				break;
			}

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			dwarf_dealloc( dbg, frameBaseAttribute, DW_DLA_ATTR );

			dwarf_printf(" Frame Pointer Variable decodeLocationListForStaticOffsetOrAddress \n");
			vector<VariableLocation> *funlocs = new vector<VariableLocation>();
			bool decodedAddressOrOffset = decodeLocationListForStaticOffsetOrAddress( locationList, 
                                                                                         listLength, objFile, *funlocs, 
                                                                                         lowpc, NULL);

			DWARF_NEXT_IF(!decodedAddressOrOffset, " Frame Pointer Variable - No location list \n");


			status = newFunction->setFramePtr(funlocs);

			DWARF_NEXT_IF ( !status, "%s[%d]: Frame pointer not set successfully.\n", __FILE__, __LINE__ );

			deallocateLocationList( dbg, locationList, listLength );
		} /* end if this DIE has a frame base attribute */

		/* Find its return type. */

		Dwarf_Attribute typeAttribute;
		if (hasSpecification) {
			status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
		}

		if ((!hasSpecification) || (status == DW_DLV_NO_ENTRY)) {
			status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
		}

		DWARF_NEXT_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

		Type * returnType = NULL;
		Type *voidType = tc->findType("void");

		if ( status == DW_DLV_NO_ENTRY ) 
		{
			if (parseSibling == true) 
			{
				// If return type is void, specEntry or abstractOriginEntry would have set it
				dwarf_printf(" Return type void \n");
				newFunction->setReturnType( voidType );
			}
		} /* end if the return type is void */
		else 
		{
			/* There's a return type attribute. */
			dwarf_printf(" Return type is not void \n");
			Dwarf_Off typeOffset;

			status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );

			DWARF_NEXT_IF( status != DW_DLV_OK, 
					"%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

			//parsing_printf("%s/%d: ret type %d\n",
			//			   __FILE__, __LINE__, typeOffset);

			returnType = tc->findOrCreateType( (int) typeOffset );
			newFunction->setReturnType( returnType );

			dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
		} /* end if not a void return type */

		/* If this is a member function, add it as a field, for backward compatibility */
		
		if ( dieEnclosure != NULL ) 
		{
			/* Using the mangled name allows us to distinguish between overridden
			   functions, but confuses the tests.  Since Type uses vectors
			   to hold field names, however, duplicate -- demangled names -- are OK. */

			char * demangledName = P_cplus_demangle( functionName, objFile->isNativeCompiler() );

			char * leftMost = NULL;
			if ( demangledName == NULL ) 
			{
				dwarf_printf( "%s[%d]: unable to demangle '%s', using mangled name.\n", 
						__FILE__, __LINE__, functionName );

				demangledName = strdup( functionName );
				assert( demangledName != NULL );
				leftMost = demangledName;
			}
			else 
			{
				/* Strip everything left of the rightmost ':' off; see above. */
				leftMost = demangledName;
				if ( strrchr( demangledName, ':' ) )
					leftMost = strrchr( demangledName, ':' ) + 1;
			}

			std::string fName = convertCharToString(leftMost);
			typeFunction *funcType = new typeFunction( (typeId_t) dieOffset, returnType, fName);

			dieEnclosure->addField( fName, funcType);
			dwarf_printf(" Adding function %s to class \n", leftMost);
			free( demangledName );
		}

		// Parse parent nodes and their children but not their sibling

		if ( isAbstractOrigin ) 
		{
			dwarf_printf( "parseParent : abstract entry  %lu with tag 0x%x \n", 
					(unsigned long)dieOffset, dieTag );

			bool result = walkDwarvenTree( dbg, abstractEntry, module, objFile, 
					cuOffset, srcFiles, lowpc, newFunction, 
					newCommonBlock, newEnum, newEnclosure, 
					false, depth+1 );

			DWARF_NEXT_IF(!result, "Failed walking Dwarven tree");

		} 
		else if ( hasSpecification ) 
		{ 
			dwarf_printf( "parseParent : spec entry  %lu with tag 0x%x \n",  
					(unsigned long)dieOffset, dieTag);

			bool result = walkDwarvenTree( dbg, specEntry, module, objFile, 
					cuOffset, srcFiles, lowpc, newFunction, 
					newCommonBlock, newEnum, newEnclosure, 
					false, depth+1 );

			DWARF_NEXT_IF(!result, "Failed walking Dwarven tree");
		}

		if ( isAbstractOrigin ) dwarf_dealloc( dbg, abstractEntry, DW_DLA_DIE ); 
		if ( hasSpecification ) dwarf_dealloc( dbg, specEntry, DW_DLA_DIE ); 
		dwarf_dealloc( dbg, functionName, DW_DLA_STRING );
	} break;


  case DW_TAG_common_block: 
	{
		dwarf_printf(" DW_TAG_common_block \n");
		char * commonBlockName;
		status = dwarf_diename( dieEntry, & commonBlockName, NULL );
		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

		Symbol *commonBlockVar;
		std::vector<Symbol *> commonBlockVars;
		if (!objFile->findSymbolByType(commonBlockVars, 
					commonBlockName, 
					Symbol::ST_OBJECT,
					mangledName))
		{
			if (!objFile->findSymbolByType(commonBlockVars, 
						commonBlockName, 
						Symbol::ST_OBJECT, 
						mangledName, 
						true))
			{
				//pgcc 6 is naming common blocks with a trailing underscore
				std::string cbvname = std::string(commonBlockName) + std::string("_");
				if (!objFile->findSymbolByType(commonBlockVars, 
							cbvname, 
							Symbol::ST_OBJECT,
							mangledName)){
					objFile->findSymbolByType(commonBlockVars,
							cbvname, 
							Symbol::ST_OBJECT, 
							mangledName,
							true);
				}
			}	
		}

		commonBlockVar = commonBlockVars[0];

		DWARF_NEXT_IF( !commonBlockVar, "%s[%d]: Couldn't find variable for common block\n", __FILE__, __LINE__);

		Type * commonBlockType = NULL;

		std::string cBName = commonBlockName;
		if (!commonBlockVar) {
			//bperr("unable to find variable %s\n", commonBlockName);
		} else {
			commonBlockType = dynamic_cast<typeCommon *>(tc->findVariableType(cBName));
			if (commonBlockType == NULL) {
				commonBlockType = new typeCommon( (typeId_t) dieOffset, cBName );
				assert( commonBlockType != NULL );
				tc->addGlobalVariable( cBName, commonBlockType );
			}	
		}
		dwarf_dealloc( dbg, commonBlockName, DW_DLA_STRING );

		/* This node's children are in the common block. */
		newCommonBlock = dynamic_cast<typeCommon*>(commonBlockType);
		if (newCommonBlock)
			newCommonBlock->beginCommonBlock();
	} break;

  case DW_TAG_constant: 
	{
		dwarf_printf(" DW_TAG_constant \n");
		//bperr ( "Warning: dyninst ignores named constant entries.\n" );
	} break;

	/* It's worth noting that a variable may have a constant value.  Since,
	   AFAIK, Dyninst does nothing with this information, neither will we.
	   (It will, however, explain why certain variables that otherwise would
	   don't have locations.) */
  case DW_TAG_variable: 
	{
		dwarf_printf(" DW_TAG_variable \n");
            /* Acquire the name, type, and line number. */
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
      
		/* We'll start with the location, since that's most likely to
		   require the _specification. */
		Dwarf_Attribute locationAttribute;
		status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
		DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
		if ( status == DW_DLV_NO_ENTRY ) { break; }
		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
      Dwarf_Locdesc **locationList;
      Dwarf_Signed listLength;
      status = dwarf_loclist_n( locationAttribute, & locationList, & listLength, NULL );
      dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );
      if ( status != DW_DLV_OK ) {
         /* I think this is OK if the local variable was optimized away. */
         break;
      }
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         
      vector<VariableLocation> locs;
      bool decodedAddressOrOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, locs, lowpc, NULL);
      deallocateLocationList( dbg, locationList, listLength );            
      if ( ! decodedAddressOrOffset ) 
      { 
         dwarf_printf("%s[%d]:  DW_TAG_variable: failed to decode loc list\n", FILE__, __LINE__);
         break;
      }
      
      for (unsigned i=0; i<locs.size(); i++) {
               //if (locs[i].stClass != storageAddr) 
               //continue;
         if (locs[i].lowPC) {
            Offset newlowpc = locs[i].lowPC;
            bool result = elfobj->convertDebugOffset(locs[i].lowPC, newlowpc);
            if (result)
               locs[i].lowPC = newlowpc;
         }
         if (locs[i].hiPC) {
            Offset newhipc = locs[i].hiPC;
            bool result = elfobj->convertDebugOffset(locs[i].hiPC, newhipc);
            if (result)
               locs[i].hiPC = newhipc;
         }
      }
      
      /* If this DIE has a _specification, use that for the rest of our inquiries. */
      Dwarf_Die specEntry = dieEntry;
      
      Dwarf_Attribute specAttribute;
      status = dwarf_attr( dieEntry, DW_AT_specification, & specAttribute, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
      if ( status == DW_DLV_OK ) {
         Dwarf_Off specOffset;
         status = dwarf_global_formref( specAttribute, & specOffset, NULL );
         DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         
         status = dwarf_offdie( dbg, specOffset, & specEntry, NULL );
         DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         
         dwarf_dealloc( dbg, specAttribute, DW_DLA_ATTR );
      } /* end if dieEntry has a _specification */
      
      /* Acquire the name, type, and line number. */
      char * variableName = NULL;
      status = dwarf_diename( specEntry, & variableName, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
      /* If we're fortran, get rid of the trailing _ */
      if (variableName && (currentFunction || currentEnclosure)) {
         supportedLanguages lang = module->language();
         if ( ( lang == lang_Fortran ||
                lang == lang_CMFortran ||
                lang == lang_Fortran_with_pretty_debug ) &&
              variableName[strlen(variableName)-1]=='_') 
            variableName[strlen(variableName)-1]='\0';
      }
      
      /* Acquire the parameter's type. */
      Dwarf_Attribute typeAttribute;
      status = dwarf_attr( specEntry, DW_AT_type, & typeAttribute, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
      if ( status == DW_DLV_NO_ENTRY ) { break; }
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      
      Dwarf_Off typeOffset;
      status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         
      /* The typeOffset forms a module-unique type identifier,
         so the Type look-ups by it rather than name. */
      Type * variableType = tc->findOrCreateType( (int) typeOffset );
      dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
      
      Dwarf_Unsigned variableLineNo;
      bool hasLineNumber = false;
      std::string fileName;
      Dwarf_Unsigned fileNameDeclVal;
      if (currentEnclosure || currentFunction) {
         /* Acquire the variable's lineNo. We don't use this for globals */
            
         Dwarf_Attribute fileDeclAttribute;
         status = dwarf_attr( specEntry, DW_AT_decl_file, & fileDeclAttribute, NULL );
         DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         if ( status == DW_DLV_OK ) {
            status = dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL);
            DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
            dwarf_dealloc( dbg, fileDeclAttribute, DW_DLA_ATTR );			
         }
         if ( status == DW_DLV_NO_ENTRY )
            fileName = "";
         else	
            fileName = convertCharToString(srcFiles[fileNameDeclVal-1]);
         
         Dwarf_Attribute lineNoAttribute;
         status = dwarf_attr( specEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
         DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
         
         /* We don't need to tell Dyninst a line number for C++ static variables,
            so it's OK if there isn't one. */
         if ( status == DW_DLV_OK ) {
            hasLineNumber = true;
            status = dwarf_formudata( lineNoAttribute, & variableLineNo, NULL );
            DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
            dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );			
         } /* end if there is a line number */
      }
      
      std::string vName;
      if (variableName)
         vName = convertCharToString(variableName);
      
      if ( currentFunction == NULL && currentEnclosure == NULL ) {
         /* The typeOffset forms a module-unique type identifier,
            so the Type look-ups by it rather than name. */
         dwarf_printf( "%s/%d: %s/%d\n", __FILE__, __LINE__, variableName ? variableName : "{NONAME}", typeOffset );
         Dyninst::Offset addr = 0;
         if (locs.size() && locs[0].stClass == storageAddr)
            addr = locs[0].frameOffset;
         Variable *var;
         bool result = module->exec()->findVariableByOffset(var, addr);
         if (result) {
            var->setType(variableType);
         }
            
         tc->addGlobalVariable( vName, variableType);
      } /* end if this variable is a global */
      else if (currentFunction) {
         /* We now have the variable name, type, offset, and line number.
            Tell Dyninst about it. */
         if (!variableName)
            break;
         dwarf_printf( "localVariable '%s', currentFunction %p\n", variableName, currentFunction );
         if (!hasLineNumber){
            variableLineNo = 0;
            fileName = "";
         }
         else {
            fileName = srcFiles[fileNameDeclVal-1];
         }
               localVar * newVariable = new localVar( vName, variableType, fileName, (int) variableLineNo, currentFunction);
         for (unsigned int i = 0; i < locs.size(); ++i)
         {
            newVariable->addLocation(locs[i]);
         }
         
         localVarCollection *lvs = NULL; 
         if (!currentFunction->getAnnotation(lvs, FunctionLocalVariablesAnno))
         {
            lvs = new localVarCollection();
            if (!currentFunction->addAnnotation(lvs, FunctionLocalVariablesAnno))
            {
               fprintf(stderr, "%s[%d]:  failed to add annotations here\n", 
                       FILE__, __LINE__);
               break;
            }
         }
         if (!lvs)
         {
            fprintf(stderr, "%s[%d]:  failed to getAnnotation here\n", 
                    FILE__, __LINE__);
            break;
         }
         lvs->addLocalVar(newVariable);
      } /* end if a local or static variable. */
      else if ( currentEnclosure != NULL ) {
         if (!variableName)
            break;
         assert( locs[0].stClass != storageRegOffset );
         currentEnclosure->addField( vName, variableType, locs[0].frameOffset);
      } /* end if this variable is not global */
   } 
   break;

  case DW_TAG_formal_parameter: 
  {
     dwarf_printf(" DW_TAG_formal_parameter \n");
     /* A formal parameter must occur in the context of a function.
        (That is, we can't do anything with a formal parameter to a
        function we don't know about.) */
     /* It's probably worth noting that a formal parameter may have a
        default value.  Since, AFAIK, Dyninst does nothing with this information,
        neither will we. */
     if ( currentFunction == NULL ) {
        dwarf_printf( "%s[%d]: ignoring formal parameter without corresponding function.\n", __FILE__, __LINE__ );
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
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     if ( !hasLocation ) {
        dwarf_printf( "%s[%d]: ignoring formal parameter without location.\n", __FILE__, __LINE__ );
        break;
     }
     
     /* Acquire the location of this formal parameter. */
     Dwarf_Attribute locationAttribute;
     status = dwarf_attr( dieEntry, DW_AT_location, & locationAttribute, NULL );
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     Dwarf_Locdesc **locationList;
     Dwarf_Signed listLength;
     status = dwarf_loclist_n( locationAttribute, & locationList, & listLength, NULL );
     dwarf_dealloc( dbg, locationAttribute, DW_DLA_ATTR );
     if ( status != DW_DLV_OK ) {
        /* I think this is legal if the parameter was optimized away. */
        dwarf_printf( "%s[%d]: ignoring formal parameter with bogus location.\n", __FILE__, __LINE__ );
        break;
     }
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     vector<VariableLocation> locs;
     bool decodedOffset = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, locs, lowpc, NULL);
     deallocateLocationList( dbg, locationList, listLength );
     
     if ( ! decodedOffset ) {
        dwarf_printf( "%s[%d]: ignoring formal parameter with undecodable location.\n", __FILE__, __LINE__ );
        break;
     }
     
     assert( locs[0].stClass != storageAddr );
         
     /* If the DIE has an _abstract_origin, we'll use that for the
        remainder of our inquiries. */
     Dwarf_Die originEntry = dieEntry;
         
     Dwarf_Attribute originAttribute;
     status = dwarf_attr( dieEntry, DW_AT_abstract_origin, & originAttribute, NULL );
     DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     if ( status == DW_DLV_OK ) {
        Dwarf_Off originOffset;
        status = dwarf_global_formref( originAttribute, & originOffset, NULL );
        DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
        
        status = dwarf_offdie( dbg, originOffset, & originEntry, NULL );
        DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
        
        dwarf_dealloc( dbg, originAttribute, DW_DLA_ATTR );
     } /* end if the DIE has an _abstract_origin */
     
     /* Acquire the parameter's name. */
     char * parameterName;
     status = dwarf_diename( originEntry, & parameterName, NULL );
     DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     /* We can't do anything with anonymous parameters. */
     if ( status == DW_DLV_NO_ENTRY ) { break; }
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     /* Acquire the parameter's type. */
     Dwarf_Attribute typeAttribute;
     status = dwarf_attr( originEntry, DW_AT_type, & typeAttribute, NULL );
     DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     if ( status == DW_DLV_NO_ENTRY ) { break; }
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     Dwarf_Off typeOffset;
     status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
     DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     /* The typeOffset forms a module-unique type identifier,
        so the Type look-ups by it rather than name. */
     dwarf_printf( "%s[%d]: found formal parameter %s with type %ld\n", __FILE__, __LINE__, parameterName, typeOffset );
     Type * parameterType = tc->findOrCreateType( (int) typeOffset );
     
     dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
     
         
     Dwarf_Attribute fileDeclAttribute;
     std::string fileName;
     status = dwarf_attr( originEntry, DW_AT_decl_file, & fileDeclAttribute, NULL );
     DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     Dwarf_Unsigned fileNameDeclVal;
     if ( status == DW_DLV_OK ) {
        status = dwarf_formudata(fileDeclAttribute, &fileNameDeclVal, NULL);
        DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
        dwarf_dealloc( dbg, fileDeclAttribute, DW_DLA_ATTR );
     }
     if ( status == DW_DLV_NO_ENTRY )
        fileName = "";
     else	
        fileName = convertCharToString(srcFiles[fileNameDeclVal-1]);
     
     /* Acquire the parameter's lineNo. */
     Dwarf_Attribute lineNoAttribute;
     Dwarf_Unsigned parameterLineNo;
     status = dwarf_attr( originEntry, DW_AT_decl_line, & lineNoAttribute, NULL );
     DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
     
     if ( status == DW_DLV_NO_ENTRY ) {
        dwarf_printf( "%s[%d]: ignoring formal parameter without line number.\n", __FILE__, __LINE__ );
        parameterLineNo = 0;
     }
     else{
        status = dwarf_formudata( lineNoAttribute, & parameterLineNo, NULL );
        DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
        dwarf_dealloc( dbg, lineNoAttribute, DW_DLA_ATTR );
      }	
         
         
     /* We now have the parameter's location, name, type, and line number.
        Tell Dyninst about it. */
     std::string pName = convertCharToString(parameterName);
     localVar * newParameter = new localVar( pName, parameterType, fileName, (int) parameterLineNo, currentFunction);
     assert( newParameter != NULL );
	  for (unsigned int i = 0; i < locs.size(); ++i)
	  {
		  newParameter->addLocation(locs[i]);
	  }

      /* This is just brutally ugly.  Why don't we take care of this invariant automatically? */
     localVarCollection *lvs = NULL;
     if (!currentFunction->getAnnotation(lvs, FunctionParametersAnno))
     {
        lvs = new localVarCollection();
        if (!currentFunction->addAnnotation(lvs, FunctionParametersAnno)) 
        {
           fprintf(stderr, "%s[%d]:  failed to add annotation here\n", 
                   FILE__, __LINE__);
           break;
        }
     }
     
     if (!lvs)
     {
        fprintf(stderr, "%s[%d]:  failed to add annotation here\n", 
                FILE__, __LINE__);
        break;
     }
         
     lvs->addLocalVar(newParameter);
     
     //TODO ??NOT REQUIRED??
     //currentFunction->addParam( parameterName, parameterType, parameterLineNo, parameterOffset );
     
     dwarf_printf( "%s[%d]: added formal parameter '%s' of type %p from line %lu.\n", __FILE__, __LINE__, parameterName, parameterType, (unsigned long)parameterLineNo );
  } break;

  case DW_TAG_base_type: 
    {
		dwarf_printf(" DW_TAG_base_type \n");
      /* What's the type's name? */
      char * typeName;
      status = dwarf_diename( dieEntry, & typeName, NULL );
      if(status == DW_DLV_OK){
	//DWARF_FALSE_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	/* How big is it? */
	Dwarf_Attribute byteSizeAttr;
	Dwarf_Unsigned byteSize;
	status = dwarf_attr( dieEntry, DW_AT_byte_size, & byteSizeAttr, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
	status = dwarf_formudata( byteSizeAttr, & byteSize, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	dwarf_dealloc( dbg, byteSizeAttr, DW_DLA_ATTR );

	/* Generate the appropriate built-in type; since there's no
	   reliable way to distinguish between a built-in and a scalar,
	   we don't bother to try. */
	std::string tName = convertCharToString(typeName);   
	typeScalar * baseType = new typeScalar( (typeId_t) dieOffset, (unsigned int) byteSize, tName );
	assert( baseType != NULL );

	/* Add the basic type to our collection. */
	dwarf_printf( "Adding base type '%s' (%lu) of size %lu to type collection %p\n", typeName, (unsigned long)dieOffset, (unsigned long)byteSize, tc );
	baseType = tc->addOrUpdateType( baseType );

	dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
      }
    } break;

  case DW_TAG_typedef: 
    {
		dwarf_printf(" DW_TAG_typedef \n");
      char * definedName = NULL;
      status = dwarf_diename( dieEntry, & definedName, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      Type * referencedType = NULL;
      Dwarf_Attribute typeAttribute;
      status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      if ( status == DW_DLV_NO_ENTRY ) {
	/* According to the DWARF spec, "A declaration of the type that is not also a definition."
	   This includes constructions like "typedef void _IO_lock_t", from libio.h, which
	   cause us to issue a lot of true but spurious-looking warnings about incomplete types.
	   So instead of ignoring this entry, point it to the void type.  (This is also more
	   in line with our handling of absent DW_AT_type tags everywhere else.) */
	referencedType = tc->findType( "void" );
      }
      else {
	Dwarf_Off typeOffset;
	status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );

	if (!module) 
	  {
	    fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
	    break;
	  }
	if (!tc) 
	  {
	    fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
	    break;
	  }
	referencedType = tc->findOrCreateType( (int) typeOffset );
      }
      string tName;
      if(!definedName) {
	switch(dieTag){
	case DW_TAG_const_type:
	  tName = "const " + referencedType->getName();
	  break;
	case DW_TAG_packed_type:
	  tName = "packed " + referencedType->getName();
	  break;
	case DW_TAG_volatile_type:
	  tName = "volatile " + referencedType->getName();
	  break;
	}
      }
      else {
	tName = convertCharToString(definedName);
      }

      typeTypedef * typedefType = new typeTypedef( (typeId_t) dieOffset, referencedType, tName);
      typedefType = tc->addOrUpdateType( typedefType );

      /* Sanity check: typedefs should not have children. */
      Dwarf_Die childDwarf;
      status = dwarf_child( dieEntry, & childDwarf, NULL );
      //assert( status == DW_DLV_NO_ENTRY );

      dwarf_dealloc( dbg, definedName, DW_DLA_STRING );
    } break;

  case DW_TAG_array_type: 
    {
		dwarf_printf(" DW_TAG_array \n");
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
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      /* Find the type of the elements. */
      Dwarf_Attribute typeAttribute;
      status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      Dwarf_Off typeOffset;
      status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );

      //parsing_printf("%s/%d: %s/%d\n",
      //			   __FILE__, __LINE__, arrayName, typeOffset);
      Type * elementType = tc->findOrCreateType( (int) typeOffset );

      /* Find the range(s) of the elements. */
      Dwarf_Die firstRange;
      status = dwarf_child( dieEntry, & firstRange, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      typeArray * baseArrayType = parseMultiDimensionalArray( dbg, firstRange, elementType, module, tc);
      assert( baseArrayType != NULL );

      /* The baseArrayType is an anonymous type with its own typeID.  Extract
	 the information and add an array type for this DIE. */
      std::string aName = convertCharToString(arrayName);
      typeArray *arrayType = new typeArray( (typeId_t) dieOffset, baseArrayType->getBaseType(), baseArrayType->getLow(),
					    baseArrayType->getHigh(), aName);
      assert( arrayType != NULL );
      //setArraySize( arrayType, baseArrayType->getLow(), baseArrayType->getHigh() );
      arrayType = tc->addOrUpdateType( arrayType );

      /* Don't parse the children again. */
      parsedChild = true;

      dwarf_dealloc( dbg, firstRange, DW_DLA_DIE );
      dwarf_dealloc( dbg, arrayName, DW_DLA_STRING );
    } break;

  case DW_TAG_subrange_type: 
    {
		dwarf_printf(" DW_TAG_subrange \n");
      std::string loBound;
      std::string hiBound;
      parseSubRangeDIE( dbg, dieEntry, loBound, hiBound, module, tc);
    } break;

  case DW_TAG_enumeration_type: 
    {
		dwarf_printf(" DW_TAG_enumeration \n");
      char * typeName = NULL;
      status = dwarf_diename( dieEntry, & typeName, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      std::string tName = convertCharToString(typeName);
      typeEnum* enumerationType = new typeEnum( (typeId_t) dieOffset, tName);
      assert( enumerationType != NULL );
      enumerationType = dynamic_cast<typeEnum *>(tc->addOrUpdateType( enumerationType ));
      newEnum = enumerationType;

      dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
    } break;

  case DW_TAG_inheritance: 
    {
		dwarf_printf(" DW_TAG_inheritance \n");
      /* Acquire the super class's type. */
      Dwarf_Attribute scAttr;
      status = dwarf_attr( dieEntry, DW_AT_type, & scAttr, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      Dwarf_Off scOffset;
      status = dwarf_global_formref( scAttr, & scOffset, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      dwarf_dealloc( dbg, scAttr, DW_DLA_ATTR );

      //parsing_printf("%s/%d: inherited %d\n",
      //			   __FILE__, __LINE__, scOffset);
      Type * superClass = tc->findOrCreateType( (int) scOffset );

      /* Acquire the visibility, if any.  DWARF calls it accessibility
	 to distinguish it from symbol table visibility. */
      Dwarf_Attribute visAttr;
      status = dwarf_attr( dieEntry, DW_AT_accessibility, & visAttr, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      visibility_t visibility = visPrivate;
      if ( status == DW_DLV_OK ) {
	Dwarf_Unsigned visValue;
	status = dwarf_formudata( visAttr, & visValue, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

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
  case DW_TAG_class_type: 
    {
		dwarf_printf(" DW_TAG_structure or DW_TAG_union DW_TAG_class \n");
      char * typeName = NULL;
      status = dwarf_diename( dieEntry, & typeName, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      Dwarf_Attribute sizeAttr;
      Dwarf_Unsigned typeSize = 0;
      status = dwarf_attr( dieEntry, DW_AT_byte_size, & sizeAttr, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      if ( status == DW_DLV_OK ) {
	status = dwarf_formudata( sizeAttr, & typeSize, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	dwarf_dealloc( dbg, sizeAttr, DW_DLA_ATTR );
      }

      fieldListType * containingType = NULL;
      std::string tName = convertCharToString(typeName);
      switch ( dieTag ) {
      case DW_TAG_structure_type: 
      case DW_TAG_class_type: {
	typeStruct *ts = new typeStruct( (typeId_t) dieOffset, tName);
	containingType = dynamic_cast<fieldListType *>(tc->addOrUpdateType(ts));
	break;
      }
	  case DW_TAG_union_type: 
	  {
		  typeUnion *tu = new typeUnion( (typeId_t) dieOffset, tName);
		  containingType = dynamic_cast<fieldListType *>(tc->addOrUpdateType(tu));
		  dwarf_printf("%s[%d]:  new union %s\n", FILE__, __LINE__, tName.c_str());
		  break;
	  }
	  }
	  newEnclosure = containingType;

      dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
    } break;

  case DW_TAG_enumerator: 
    {
		dwarf_printf(" DW_TAG_enumerator \n");
      /* An entry in an enumeration. */
      char * enumName = NULL;
      status = dwarf_diename( dieEntry, & enumName, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      Dwarf_Attribute valueAttr;
      status = dwarf_attr( dieEntry, DW_AT_const_value, & valueAttr, NULL );
      DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

      Dwarf_Signed enumValue = 0;
      if ( status == DW_DLV_OK ) {
	status = dwarf_formsdata( valueAttr, & enumValue, NULL );
	DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	dwarf_dealloc( dbg, valueAttr, DW_DLA_ATTR );
      }

      // //bperr ( "Adding enum '%s' (%ld) to enumeration '%s' (%d)\n", enumName, (signed long)enumValue, currentEnclosure->getName(), currentEnclosure->getID() );
      currentEnum->addConstant( enumName, (int) enumValue );

      dwarf_dealloc( dbg, enumName, DW_DLA_STRING );
    } break;

  case DW_TAG_member: 
	{
		dwarf_printf(" DW_TAG_member \n");
		char * memberName = NULL;
		status = dwarf_diename( dieEntry, & memberName, NULL );
		DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		if (!memberName)
		{
			//fprintf(stderr, "%s[%d]:  weird, dwarf_diename returned NULL w/out error\n", 
		//	FILE__, __LINE__);
		}
		Dwarf_Attribute typeAttribute;
		status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );

		if ( status == DW_DLV_NO_ENTRY ) 
		{
			dwarf_printf("%s[%d]:  parse member: no entry\n", FILE__, __LINE__);
			break;
		}

		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
				__FILE__, __LINE__ );

		Dwarf_Off typeOffset;
		status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );

		dwarf_printf("%s[%d]:  type id '%d' for type '%s'\n", FILE__, __LINE__, 
				typeOffset, memberName ? memberName : "no_name");

		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );

		vector<VariableLocation> locs;

		//parsing_printf("%s/%d: %s/%d\n",
		//			   __FILE__, __LINE__, memberName, typeOffset);

		Type *memberType = tc->findOrCreateType((int) typeOffset );

		Dwarf_Attribute locationAttr;
		status = dwarf_attr( dieEntry, DW_AT_data_member_location, & locationAttr, NULL );

		DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		if ( status == DW_DLV_NO_ENTRY ) 
		{
			dwarf_printf("%s[%d]:  parse member %s: no location\n", FILE__, __LINE__, memberName ? memberName : "nameless_member");
			//break;
			//  some members do not have locations???  (see unions)
		}
		else
		{

		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		Dwarf_Locdesc **locationList;
		Dwarf_Signed listLength;
		status = dwarf_loclist_n( locationAttr, & locationList, & listLength, NULL );
		DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		dwarf_dealloc( dbg, locationAttr, DW_DLA_ATTR );

		dwarf_printf(" Tag member %s decodeLocationListForStaticOffsetOrAddress \n", memberName);
		long int baseAddress = 0;
		bool decodedAddress = decodeLocationListForStaticOffsetOrAddress( locationList, listLength, objFile, locs, lowpc, & baseAddress );
		deallocateLocationList( dbg, locationList, listLength );

		if ( ! decodedAddress ) 
		{ 
			dwarf_printf("%s[%d]:  failed to decode member address\n", FILE__, __LINE__);
			break; 
		}

		assert( decodedAddress );

		assert( locs[0].stClass == storageAddr );
		}

		/* DWARF stores offsets in bytes unless the member is a bit field.
		   Correct memberOffset as indicated.  Also, memberSize is in bytes
		   from the underlying type, not the # of bits used from it, so
		   correct that as necessary as well. */
		long int memberSize = memberType->getSize();

		Dwarf_Attribute bitOffset;
		status = dwarf_attr( dieEntry, DW_AT_bit_offset, & bitOffset, NULL );
		DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", 
				FILE__, __LINE__ );

		if ( status == DW_DLV_OK && locs.size() ) 
		{
			Dwarf_Unsigned memberOffset_du = locs[0].frameOffset;
			status = dwarf_formudata( bitOffset, &memberOffset_du, NULL );
			DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
					FILE__, __LINE__ );

			dwarf_dealloc( dbg, bitOffset, DW_DLA_ATTR );

			Dwarf_Attribute bitSize;
			status = dwarf_attr( dieEntry, DW_AT_bit_size, & bitSize, NULL );
			DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
					FILE__, __LINE__ );

			Dwarf_Unsigned memberSize_du = memberSize;
			status = dwarf_formudata( bitSize, &memberSize_du, NULL );
			DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", 
					FILE__, __LINE__ );

			dwarf_dealloc( dbg, bitSize, DW_DLA_ATTR );

			/* If the DW_AT_byte_size field exists, there's some padding.
			   FIXME?  We ignore padding for now.  (We also don't seem to handle
			   bitfields right in getComponents() anyway...) */
		}
		else 
		{
			dwarf_printf("%s[%d]:  got status != OK\n", FILE__, __LINE__);
			if (locs.size())
				locs[0].frameOffset *= 8;
			memberSize *= 8;
		} /* end if not a bit field member. */

		if ( memberName != NULL ) 
		{
			std::string fName = convertCharToString(memberName);
			int offset_to_use = locs.size() ? locs[0].frameOffset : -1;
			currentEnclosure->addField( fName, memberType, offset_to_use);
			dwarf_dealloc( dbg, memberName, DW_DLA_STRING );
			dwarf_printf("%s[%d]:  new field: [%s] %s\n", FILE__, __LINE__, 
					memberType ? memberType->getName().c_str() : "no_type", fName.c_str());
		} 
		else 
		{
			/* An anonymous union [in a struct]. */
			std::string fName = "[anonymous union]";
			int offset_to_use = locs.size() ? locs[0].frameOffset : -1;
			currentEnclosure->addField( fName, memberType, offset_to_use);
			dwarf_printf("%s[%d]:  new field: [%s] %s\n", FILE__, __LINE__, 
					memberType ? memberType->getName().c_str() : "no_type", fName.c_str());
		}
	} 
	break;

  case DW_TAG_const_type:
  case DW_TAG_packed_type:
  case DW_TAG_volatile_type: 
	{
		dwarf_printf(" DW_TAG_const or DW_TAG_packed or DW_TAG_volatile \n");
    char * typeName = NULL;
    status = dwarf_diename( dieEntry, & typeName, NULL );
    DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

    /* Which type does it modify? */
    Dwarf_Attribute typeAttribute;
    status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );

    int typeSize = 0;
    Dwarf_Off typeOffset;
    Type * typeModified = NULL;
    if( status == DW_DLV_NO_ENTRY ) {
      /* Presumably, a pointer or reference to void. */
      typeModified = tc->findType( "void" );
    } else {			
      status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      //parsing_printf("%s/%d: %s/%d\n",
      //			   __FILE__, __LINE__, typeName, typeOffset);
      typeModified = tc->findOrCreateType( (int) typeOffset );
      typeSize = typeModified->getSize();

      dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
    } /* end if typeModified is not void */

    string tName;
    if(!typeName){
      switch(dieTag){
      case DW_TAG_const_type:
	tName = "const " + typeModified->getName();
	break;
      case DW_TAG_packed_type:
	tName = "packed " + typeModified->getName();
	break;
      case DW_TAG_volatile_type:
	tName = "volatile " + typeModified->getName();
	break;
      }
    }
    else
      // I'm taking out the type qualifiers for right now
      tName = convertCharToString(typeName);

    typeTypedef * modifierType = new typeTypedef((typeId_t) dieOffset, typeModified, tName);
    assert( modifierType != NULL );
    // //bperr ( "Adding modifier type '%s' (%lu) modifying (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );
    modifierType = tc->addOrUpdateType( modifierType );
    dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
  } break;

  case DW_TAG_subroutine_type:
    /* If the pointer specifies argument types, this DIE has
       children of those types. */
  case DW_TAG_ptr_to_member_type:
  case DW_TAG_pointer_type:
  case DW_TAG_reference_type: 
	{
		dwarf_printf(" DW_TAG_subroutine or DW_TAG_ptr_to_member or DW_TAG_pointer or DW_TAG_reference \n");
    char * typeName = NULL;
    status = dwarf_diename( dieEntry, & typeName, NULL );
    DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

    /* To which type does it point? */
    Dwarf_Attribute typeAttribute;
    status = dwarf_attr( dieEntry, DW_AT_type, & typeAttribute, NULL );
    DWARF_NEXT_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

    Dwarf_Off typeOffset = 0;
    Type * typePointedTo = NULL;
    if( status == DW_DLV_NO_ENTRY ) {
      /* Presumably, a pointer or reference to void. */
      typePointedTo = tc->findType("void");
    } else {			
      status = dwarf_global_formref( typeAttribute, & typeOffset, NULL );
      DWARF_NEXT_IF( status != DW_DLV_OK, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );
      //parsing_printf("%s/%d: %s/%d\n",
      //			   __FILE__, __LINE__, typeName, typeOffset);
      typePointedTo = tc->findOrCreateType( (int) typeOffset );

      dwarf_dealloc( dbg, typeAttribute, DW_DLA_ATTR );
    } /* end if typePointedTo is not void */

    Type * indirectType = NULL;
    std::string tName = convertCharToString(typeName);
    switch ( dieTag ) {
    case DW_TAG_subroutine_type:
      indirectType = new typeFunction((typeId_t) dieOffset, typePointedTo, tName);
      indirectType = tc->addOrUpdateType((typeFunction *) indirectType );
      break;
    case DW_TAG_ptr_to_member_type:
    case DW_TAG_pointer_type:
      indirectType = new typePointer((typeId_t) dieOffset, typePointedTo, tName);
      indirectType = tc->addOrUpdateType((typePointer *) indirectType );
      break;
    case DW_TAG_reference_type:
      indirectType = new typeRef((typeId_t) dieOffset, typePointedTo, tName);
      indirectType = tc->addOrUpdateType((typeRef *) indirectType );
      break;
    }

    assert( indirectType != NULL );
    dwarf_printf( "Added indirect type '%s' (%lu) pointing to (%lu)\n", typeName, (unsigned long)dieOffset, (unsigned long)typeOffset );

    dwarf_dealloc( dbg, typeName, DW_DLA_STRING );
  } break;

  case DW_TAG_variant_part:
    /* We don't support this (Pascal) type. */
		dwarf_printf(" DW_TAG_variant_part \n");
		break;
  case DW_TAG_string_type:
		dwarf_printf(" DW_TAG_string \n");
		break;
    /* We don't support this (FORTRAN) type. */
  default:
    /* Nothing of interest. */
    // //bperr ( "Entry %lu with tag 0x%x ignored.\n", (unsigned long)dieOffset, dieTag );
    dwarf_printf( "Entry %lu with tag 0x%x ignored.\n", (unsigned long)dieOffset, dieTag );

    break;
  } /* end dieTag switch */

  if (!walk_error)
  {
	  /* Recurse to its child, if any. */
	  Dwarf_Die childDwarf;
	  status = dwarf_child( dieEntry, & childDwarf, NULL );
	  DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	  if ( status == DW_DLV_OK && parsedChild == false ) 
	  {
		  dwarf_printf( "Start walkDwarvenTree to child of  %lu with tag 0x%x \n", (unsigned long)dieOffset, dieTag );
		  bool result = walkDwarvenTree( dbg, childDwarf, module, objFile, cuOffset, 
				  srcFiles, lowpc, newFunction, newCommonBlock, 
				  newEnum, newEnclosure, true, depth+1 );
		  if (!result) 
		  {
			  dwarf_printf("[%s:%u] - Failed to walk dwarven tree\n", 
					  __FILE__, __LINE__);
			  if (depth != 1)
				  return false;
		  }
	  }
  }

  if (parseSibling == true) 
  {
	  /* Recurse to its first sibling, if any. */
	  Dwarf_Die siblingDwarf;
	  status = dwarf_siblingof( dbg, dieEntry, & siblingDwarf, NULL );
	  DWARF_FALSE_IF( status == DW_DLV_ERROR, "%s[%d]: error walking DWARF tree.\n", __FILE__, __LINE__ );

	  /* Deallocate the entry we just parsed. */
	  dwarf_dealloc( dbg, dieEntry, DW_DLA_DIE );

	  if ( status == DW_DLV_OK ) 
	  {
		  /* Do the tail-call optimization by hand. */
		  dieEntry = siblingDwarf;
		  dwarf_printf( "Start walkDwarvenTree to sibling of  %lu with tag 0x%x  - tail recursion \n", (unsigned long)dieOffset, dieTag);
		  goto tail_recursion;
	  }
  }

  /* When would we return false? :) */
  return true;
} /* end walkDwarvenTree() */

extern void pd_dwarf_handler( Dwarf_Error, Dwarf_Ptr );

void Object::parseDwarfTypes( Symtab *objFile) 
{
	// Man do we do to a lot of trouble for this...
	/*	
		assert( moduleTypes );

		if ( moduleTypes->dwarfParsed() ) {
		dwarf_printf( "%s[%d]: already parsed %s, moduleTypes = %p\n", __FILE__, __LINE__, fileName, moduleTypes );
		std::vector<Symbol *> * bpfuncs = getProcedures( true );
		assert( bpfuncs );
		for( unsigned int i = 0; i < bpfuncs->size(); i++ ) {
		(*bpfuncs)[i]->fixupUnknown( this );
		}
		return;
		}
	 */		
	dwarf_printf( "%s[%d]: parsing %s...\n", __FILE__, __LINE__, objFile->file().c_str() );

	/* Start the dwarven debugging. */
	Module *mod = NULL, *fixUnknownMod = NULL;

	Dwarf_Debug *dbg_ptr = dwarf.dbg();
	DWARF_RETURN_IF( !dbg_ptr, "%s[%d]: error initializing libdwarf.\n", 
			__FILE__, __LINE__ );
	Dwarf_Debug &dbg = *dbg_ptr;

	/* Iterate over the compilation-unit headers. */
	Dwarf_Unsigned hdr;

	while ( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & hdr, NULL ) == DW_DLV_OK ) 
	{
		/* Obtain the module DIE. */
		Dwarf_Die moduleDIE;
		int status = dwarf_siblingof( dbg, NULL, &moduleDIE, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, 
				"%s[%d]: error finding next CU header.\n", __FILE__, __LINE__ );

		/* Make sure we've got the right one. */
		Dwarf_Half moduleTag;
		status = dwarf_tag( moduleDIE, & moduleTag, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, 
				"%s[%d]: error acquiring CU header tag.\n", __FILE__, __LINE__ );
		assert( moduleTag == DW_TAG_compile_unit );

		/* We may need this later. */
		Dwarf_Off cuOffset;
		status = dwarf_dieoffset( moduleDIE, & cuOffset, NULL );
		DWARF_RETURN_IF( status != DW_DLV_OK, 
				"%s[%d]: error acquiring CU header offset.\n", __FILE__, __LINE__ );

		/* Extract the name of this module. */
		char * moduleName;
		status = dwarf_diename( moduleDIE, & moduleName, NULL );

		if ( status == DW_DLV_NO_ENTRY ) 
		{
			moduleName = strdup( "{ANONYMOUS}" );
			assert( moduleName != NULL );
		}

		DWARF_RETURN_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error acquiring module name.\n", __FILE__, __LINE__ );

		dwarf_printf( "%s[%d]: considering compilation unit '%s'\n", 
				__FILE__, __LINE__, moduleName );

		/* Set the language, if any. */
		Dwarf_Attribute languageAttribute;
		status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );

		DWARF_RETURN_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error acquiring language attribute.\n", __FILE__, __LINE__ );

		//      /* Set the lowPC, if any. */
		//      Dwarf_Attribute lowPCAttribute;
		//      status = dwarf_attr( moduleDIE, DW_AT_low_pc, & lowPCAttribute, NULL );
		//      DWARF_RETURN_IF( status == DW_DLV_ERROR, "%s[%d]: error acquiring language attribute.\n", __FILE__, __LINE__ );

		Dwarf_Addr lowPCdwarf = 0;
		Address lowpc = 0;
		status = dwarf_lowpc( moduleDIE, &lowPCdwarf, NULL );
		if ( status == DW_DLV_OK)
		{
			lowpc = (Address)lowPCdwarf;
		}

		//      status = dwarf_formaddr( lowPCAttribute, &lowPCdwarf, NULL );
		//      if(status == DW_DLV_OK)
		//          lowpc = (Address)lowPCdwarf;

		/* Iterate over the tree rooted here; walkDwarvenTree() deallocates the passed-in DIE. */
		if (!objFile->findModuleByName(mod, moduleName))
		{
			std::string modName = moduleName;
			std::string fName = extract_pathname_tail(modName);

			if (!objFile->findModuleByName(mod, fName))
			{
				modName = objFile->file();

				if (!objFile->findModuleByName(mod, modName)) 
				{
               mod = objFile->getDefaultModule();
				}
			}
		}

		if (!fixUnknownMod)
			fixUnknownMod = mod;

		Dwarf_Signed cnt;
		char **srcfiles;
		status = dwarf_srcfiles(moduleDIE, &srcfiles,&cnt, NULL);
		DWARF_RETURN_IF( status == DW_DLV_ERROR, 
				"%s[%d]: error acquiring source file names.\n", __FILE__, __LINE__ );

		dwarf_printf( "%s[%d]: Start walkDwarvenTree for module '%s' \n", 
				__FILE__, __LINE__, moduleName );

		if ( !walkDwarvenTree( dbg, moduleDIE, mod, objFile, cuOffset, srcfiles, lowpc) ) 
		{
			enclosureMap.clear();
			//bpwarn ( "Error while parsing DWARF info for module '%s'.\n", moduleName );
			dwarf_printf( "%s[%d]: Error while parsing DWARF info for module '%s' \n", 
					__FILE__, __LINE__, moduleName );
			return;
		}

		enclosureMap.clear();
		dwarf_printf( "%s[%d]: Done walkDwarvenTree for module '%s' \n", 
				__FILE__, __LINE__, moduleName );

		if (status == DW_DLV_OK)
		{
			for (unsigned i = 0; i < cnt; ++i) 
			{
				/* use srcfiles[i] */
				dwarf_dealloc(dbg, srcfiles[i], DW_DLA_STRING);
			}
			dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
		}

		dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
	} /* end iteration over compilation-unit headers. */

	if (!fixUnknownMod)
		return;

	/* Fix type list. */
	typeCollection *moduleTypes = typeCollection::getModTypeCollection(fixUnknownMod);
	assert(moduleTypes);
	dyn_hash_map< int, Type * >::iterator typeIter =  moduleTypes->typesByID.begin();
	for (;typeIter!=moduleTypes->typesByID.end();typeIter++)
	{
		typeIter->second->fixupUnknowns(fixUnknownMod);
	} /* end iteratation over types. */

	/* Fix the types of variabls. */   
	std::string variableName;
	dyn_hash_map< std::string, Type * >::iterator variableIter = moduleTypes->globalVarsByName.begin();
	for (;variableIter!=moduleTypes->globalVarsByName.end();variableIter++)
	{ 
		if (variableIter->second->getDataClass() == dataUnknownType && 
				moduleTypes->findType( variableIter->second->getID() ) != NULL ) 
		{
			moduleTypes->globalVarsByName[ variableIter->first ] 
				= moduleTypes->findType( variableIter->second->getID() );
		} /* end if data class is unknown but the type exists. */
	} /* end iteration over variables. */


	moduleTypes->setDwarfParsed();
} /* end parseDwarfTypes() */

#include "common/h/dwarfSW.h"

bool Object::hasFrameDebugInfo()
{
   dwarf.dbg();
   assert(dwarf.sw);
   return dwarf.sw->hasFrameDebugInfo();
}

bool Object::getRegValueAtFrame(Address pc, 
		Dyninst::MachRegister reg, 
		Dyninst::MachRegisterVal &reg_result,
		MemRegReader *reader)
{
   dwarf.dbg();
   return dwarf.sw->getRegValueAtFrame(pc, reg, reg_result, getArch(), reader);
}

DwarfHandle::DwarfHandle(Object *obj_) :
   sw(NULL),
   obj(obj_),
   init_dwarf_status(dwarf_status_uninitialized)
{
   assert(obj);
}

Dwarf_Debug *DwarfHandle::dbg()
{
   int status;
   Dwarf_Error err;
   if (init_dwarf_status == dwarf_status_ok) {
      return &dbg_data;
   }

   if (init_dwarf_status == dwarf_status_error) {
      return NULL;
   }
   
   status = dwarf_elf_init(obj->elfHdrForDebugInfo.e_elfp(), DW_DLC_READ, 
                           &pd_dwarf_handler, obj->getErrFunc(), &dbg_data, &err);
   if (status != DW_DLV_OK) {
      init_dwarf_status = dwarf_status_error;
      return NULL;
   }
   sw = new DwarfSW(dbg_data, obj->getAddressWidth());

   init_dwarf_status = dwarf_status_ok;
   return &dbg_data;
}

DwarfHandle::~DwarfHandle()
{
   if (init_dwarf_status != dwarf_status_ok) 
      return;
   if (sw)
      delete sw;
   Dwarf_Error err;
   dwarf_finish(dbg_data, &err);
}
