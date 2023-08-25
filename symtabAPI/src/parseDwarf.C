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
#include <map>

#include "elf.h"
#include "libelf.h"
#include "dwarf.h"
#include "elfutils/libdw.h"
#include "dwarfExprParser.h"
#include "dwarfFrameParser.h"

#include "Symtab.h"
#include "Type.h"
#include "Function.h"
#include "Module.h"
#include "Object.h"
#include "Collections.h"
#include "pathName.h"
#include "Variable.h"
#include "Type-mem.h"
#include <stdarg.h>
#include "Annotatable.h"
#include "annotations.h"
#include "debug.h"

#if 0
#ifndef DW_FRAME_CFA_COL3
//  This is a newer feature of libdwarf (which has been causing some other 
//  compilation problems locally) -- so we just fudge it for the moment
#define DW_FRAME_CFA_COL3               1036
/* Use this to get the cfa. */
extern "C" {
int dwarf_get_fde_info_for_cfa_reg3(
		Dwarf_FDE /*fde*/,
		Dwarf_Addr       /*pc_requested*/, 
		Dwarf_Small  *   /*value_type*/, 
		Dwarf_Sword *   /*offset_relevant*/,
		Dwarf_Sword *    /*register*/,  
		Dwarf_Sword *    /*offset_or_block_len*/,
		Dwarf_Ptr   *    /*block_ptr */,
		Dwarf_Addr*      /*row_pc_out*/,
		Dwarf_Error*     /*error*/)
{
	fprintf(stderr, "%s[%d]:  WARNING:  inside dummy dwarf functions\n", FILE__, __LINE__);
	return 0;
}
}
#endif
#endif

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;


std::string convertCharToString(char *ptr)
{
  std::string str;
  if (ptr)
    str = ptr;
  else
    str = "";
  return str;	
}

void Object::parseDwarfTypes( Symtab *) 
{
   assert(0);
} /* end parseDwarfTypes() */

bool Object::hasFrameDebugInfo()
{
   dwarf->frame_dbg();
   assert(dwarf->frameParser());
   return dwarf->frameParser()->hasFrameDebugInfo();
}

bool Object::getRegValueAtFrame(Address pc, 
		Dyninst::MachRegister reg, 
		Dyninst::MachRegisterVal &reg_result,
		MemRegReader *reader)
{
   DwarfDyninst::FrameErrors_t frame_error = DwarfDyninst::FE_No_Error;
   bool result;

   dwarf->frame_dbg();
   assert(dwarf->frameParser());
   result = dwarf->frameParser()->getRegValueAtFrame(pc, reg, reg_result, reader, frame_error);
   Symtab::setSymtabError((SymtabError) frame_error);
   return result;
}

