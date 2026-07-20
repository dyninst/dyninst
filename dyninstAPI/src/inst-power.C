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

#include "dyninstAPI/src/image.h"
#include "common/src/arch-power.h"
#include "parsing/parse_func.h"

constexpr auto GET_IP = 0x429f0005;
constexpr auto MFLR_30 = 0x7fc802a6;
constexpr auto ADDIS_30_30 = 0x3fde0000;
constexpr auto ADDI_30_30 = 0x3bde0000;
constexpr auto LWZ_11_30 = 0x817e0000;
constexpr auto ADDIS_11_30 = 0x3d7e0000;

/*
 * If the target stub_addr is a glink stub, try to determine the actual
 * function called (through the GOT) and fill in that information.
 *
 * The function at stub_addr may not have been created when this method
 * is called.
 *
 * XXX Is this a candidate to move into general parsing code, or is
 *     this properly a Dyninst-only technique?
 */
bool image::updatePltFunc(parse_func *caller_func, Address stub_addr) {
  unsigned int *stub;
  Address got2 = 0;

  if(!getPltFuncs()) {
    return false;
  }

  // If we're calling an empty glink stub.
  unordered_map<Address, std::string>::iterator entry = pltFuncs->find(stub_addr);
  if(entry != pltFuncs->end() && entry->second == "@plt") {
    int state = 0;

    // Find GOT2 value
    auto bl = caller_func->blocks();
    auto bit = bl.begin();
    for(; bit != bl.end(); ++bit) {
      ParseAPI::Block *b = *bit;
      for(Address addr = b->start(); addr < b->end(); addr += NS_power::instruction::size()) // XXX 4
      {
        unsigned int *caller_insn =
            (unsigned int *)caller_func->isrc()->getPtrToInstruction(addr);

        if(state == 0 && *caller_insn == GET_IP) {
          got2 = addr + NS_power::instruction::size();
          ++state;
        } else if(state == 1 && *caller_insn == MFLR_30) {
          ++state;
        } else if(state == 4) {
          break;
        } else if(state >= 2 && (*caller_insn & 0xffff0000) == ADDIS_30_30) {
          got2 += ((signed short)(*caller_insn & 0x0000ffff)) << 16;
          ++state;
        } else if(state >= 2 && (*caller_insn & 0xffff0000) == ADDI_30_30) {
          got2 += (signed short)(*caller_insn & 0x0000ffff);
          ++state;
        }
      }
    }
    if(state != 4) {
      return false;
    }

    // Find stub offset
    stub = (unsigned int *)caller_func->isrc()->getPtrToInstruction(stub_addr);
    int offset = 0;
    if((stub[0] & 0xffff0000) == LWZ_11_30) {
      offset = (signed short)(stub[0] & 0x0000ffff);
    } else if((stub[0] & 0xffff0000) == ADDIS_11_30) {
      offset &= (stub[0] & 0x0000ffff) << 16;
      offset &= (stub[1] & 0x0000ffff);
    }

    // Update all PLT based structures
    Address plt_addr = got2 + offset;
    (*pltFuncs)[stub_addr] = (*pltFuncs)[plt_addr];
    getObject()->updateFuncBindingTable(stub_addr, plt_addr);
  }
  return true;
}
