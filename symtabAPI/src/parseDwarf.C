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

