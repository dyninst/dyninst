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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/src/sw.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "unaligned_memory_access.h"

using namespace Dyninst;
using namespace Stackwalker;
using namespace std;

CallChecker::CallChecker(ProcessState * proc_) : proc(proc_) {}
CallChecker::~CallChecker() {}

bool CallChecker::isPrevInstrACall(Address addr, Address &target)
{
    sw_printf("[%s:%d] - isPrevInstrACall on %lx\n", FILE__, __LINE__, addr);
    if (addr == 0) return false;
    const unsigned max_call_length = 6;
    unsigned char buffer[max_call_length];

    Address start;
    for (unsigned size = max_call_length; size > 0; size--) {
        start = addr - size;
        if (proc->readMem(buffer, start, max_call_length)) break;
    }

    unsigned char * bufferPtr = buffer;
    
    Dyninst::Architecture arch = proc->getArchitecture();

    for (unsigned size = addr - start; size > 0; size--) {
        InstructionAPI::InstructionDecoder d(bufferPtr, size, arch);

        unsigned int aligned = 0;

        // Decode all instructions in the buffer
        InstructionAPI::Instruction tmp = d.decode();
        InstructionAPI::Instruction prevInsn = tmp;

        while (tmp.isValid()) {
            if (tmp.size() > size) break;
        
            aligned += tmp.size();
            prevInsn = tmp;
            tmp = d.decode();
        }

        // prevInsn was the last valid instruction found
        // is it (a) aligned and (b) a call?
        if ( (aligned == size) && 
             (prevInsn.getOperation().getID() == e_call) ) {
            int disp = Dyninst::read_memory_as<int32_t>(bufferPtr+(size-prevInsn.size() + 2));
            target = addr + disp;
            sw_printf("[%s:%d] - Found call encoded by %d to %lx (addr = %lx, disp = %x)\n",
                    FILE__, __LINE__,
                    buffer[0], target, addr, (unsigned int)disp);
            return true;
        } else {
            bufferPtr++;
            continue;
        }
    }
   
    target = 0x0; 
    return false;
}
