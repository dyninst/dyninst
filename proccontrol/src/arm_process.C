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

#include <string.h>
#include <iostream>
#include "proccontrol/src/arm_process.h"

#if defined(Arch_aarch64)
#include "common/src/arch-aarch64.h"
using namespace NS_aarch64;
#endif

using namespace std;

//constructors, blank functions
arm_process::arm_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                         std::vector<std::string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f)
{
}

arm_process::arm_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p)
{
}

arm_process::~arm_process()
{
}

unsigned arm_process::plat_breakpointSize()
{
  //This size is the number of bytes of one
  //trap instruction. In aarch64, this is BRK
  //which stands for breakpoint, with a normal
  //length of 32bits == 4bytes
  return 4;
}

void arm_process::plat_breakpointBytes(unsigned char *buffer)
{
  //memory oppucation:
  //high---low addr
  //[3] [2] [1] [0]
  //this is a BRK instruction in aarch64, which incurs a
  //software exception, the encoding is
  //0b1101_0100_001x_xxxx_xxxx_xxxx_xxx0_0000
  //(x is for imm16)
  //the following instruction stands for
  //BRK #0;
  buffer[0] = 0x00;
  buffer[1] = 0x00;
  buffer[2] = 0x20;
  buffer[3] = 0xd4;
}

bool arm_process::plat_breakpointAdvancesPC() const
{
   //as doc says, arm will return to the interrupted intruction
   return false;
}

/*
async_ret_t arm_process::plat_needsEmulatedSingleStep(int_thread *thr, vector<Address> &addrResult) {
     return aret_success;
}

void arm_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr> resps)
{
	assert(0); //not implemented
}

bool arm_process::plat_convertToBreakpointAddress(Address &, int_thread *) {
#warning "This function is not verified."
   return true;
}

bool arm_process::plat_needsPCSaveBeforeSingleStep()
{
#warning "This function is not verified."
   //pc is saved in LR, which might be accomplished by BL instruction
   //but for single step, is this true as well?
   return false;
}
*/

arm_thread::arm_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l), have_cached_pc(false), cached_pc(0)
{
}

arm_thread::~arm_thread()
{
}

#warning "HWBreakpoint is not supported now."
bool arm_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &)
{
    return false;
}

bool arm_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &)
{
    return false;
}

unsigned arm_thread::hwBPAvail(unsigned)
{
    return 0;
}

EventBreakpoint::ptr arm_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal)
{
    return EventBreakpoint::ptr();
}

bool arm_thread::bpNeedsClear(hw_breakpoint *)
{
	assert(0); //not implemented
    return false;
}

void arm_thread::setCachedPC(Address pc)
{
    //what is cached PC?
    cached_pc = pc;
    have_cached_pc = true;
}

void arm_thread::clearCachedPC()
{
    have_cached_pc = false;
}

bool arm_thread::haveCachedPC(Address &pc)
{
    if (!have_cached_pc)
        return false;
    pc = cached_pc;
    return true;
}
