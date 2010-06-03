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

#include "common/h/parseauxv.h"
#include "common/h/headers.h"

#include "common/h/addrtranslate.h"
#include "common/src/addrtranslate-sysv.h"

using namespace Dyninst;

/* 
 * This file factors out some functionality that is specific to platforms
 * where the r_debug structure and trap address are derived from symbols
 * in the interpreter/dynamic linker. There are two steps to this process.
 * The first is to determine the base address of the dynamic linker in
 * memory. The second is to combine the base address with the offset of
 * the symbol for the r_debug structure to get the address of the r_debug
 * structure in memory.
 *
 * This functionality is in a separate file because on some platforms,
 * the interpreter doesn't contain the necessary symbol information.
 */
bool AddressTranslateSysV::setInterpreterBase() {
    if (set_interp_base) return true;

    AuxvParser *parser = AuxvParser::createAuxvParser(pid, address_size);
    if (!parser) return false;

    interpreter_base = parser->getInterpreterBase();
    set_interp_base = true;

    parser->deleteAuxvParser();
    return true;
}

bool AddressTranslateSysV::parseInterpreter() {
   bool result;

   result = setInterpreter();
   if (!result) {
     translate_printf("[%s:%u] - Failed to set interpreter.\n", __FILE__, __LINE__);
     return false;
   }

   result = setAddressSize();
   if (!result) {
      translate_printf("[%s:%u] - Failed to set address size.\n", __FILE__, __LINE__);
      return false;
   }

   result = setInterpreterBase();
   if (!result) {
      translate_printf("[%s:%u] - Failed to set interpreter base.\n", __FILE__, __LINE__);
      return false;
   }

   if (interpreter) {
      r_debug_addr = interpreter->get_r_debug() + interpreter_base;
      trap_addr = getTrapAddrFromRdebug();
   }
   else {
      r_debug_addr = 0;
      trap_addr = 0;
   }

   return true;
}
