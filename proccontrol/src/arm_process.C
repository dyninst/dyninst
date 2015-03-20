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

#warning "This file is not implemented yet!"

#include <string.h>
#include <iostream>
#include "proccontrol/src/arm_process.h"

#if defined(Arch_aarch64)
#include "common/src/arch-aarch64.h"
using namespace NS_aarch64;
#endif

using namespace std;

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
	assert(0); //not implemented
  return 4;
}

void arm_process::plat_breakpointBytes(unsigned char *buffer)
{
	assert(0); //not implemented
  //buffer[0] = 0x7d;
  //buffer[1] = 0x82;
  //buffer[2] = 0x10;
  //buffer[3] = 0x08;
}

bool arm_process::plat_breakpointAdvancesPC() const
{
	assert(0); //not implemented
   return false;
}

//static bool atomicLoad(const instruction &insn) {
//#warning "This function is not verified."
//	return false;
//}
//
//static bool atomicStore(const instruction &insn) {
//#warning "This function is not verified."
//	return false;
//}

//void clear_ss_state_cb(int_thread *thr) {
//#warning "This function is not verified."
//}

void arm_process::cleanupSSOnContinue(int_thread *thr)
{
	assert(0); //not implemented
}

void arm_process::registerSSClearCB()
{
	assert(0); //not implemented
}

async_ret_t arm_process::readPCForSS(int_thread *thr, Address &pc)
{
	assert(0); //not implemented
}

async_ret_t arm_process::readInsnForSS(Address pc, int_thread *, unsigned int &rawInsn)
{
	assert(0); //not implemented
}

async_ret_t arm_process::plat_needsEmulatedSingleStep(int_thread *thr, vector<Address> &addrResult) {
	assert(0); //not implemented
}

void arm_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr> resps)
{
	assert(0); //not implemented
}

bool arm_process::plat_convertToBreakpointAddress(Address &, int_thread *) {
	assert(0); //not implemented
   return true;
}

bool arm_process::plat_needsPCSaveBeforeSingleStep() 
{
	assert(0); //not implemented
   return true;
}

arm_thread::arm_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l), have_cached_pc(false), cached_pc(0)
{
	assert(0); //not implemented
}

arm_thread::~arm_thread()
{
	assert(0); //not implemented
}

bool arm_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &)
{
	assert(0); //not implemented
   return false;
}

bool arm_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &)
{
	assert(0); //not implemented
   return false;
}

unsigned arm_thread::hwBPAvail(unsigned)
{
	assert(0); //not implemented
   return 0;
}

EventBreakpoint::ptr arm_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal)
{
	assert(0); //not implemented
   return EventBreakpoint::ptr();
}

bool arm_thread::bpNeedsClear(hw_breakpoint *)
{
	assert(0); //not implemented
   return false;
}

void arm_thread::setCachedPC(Address pc)
{
	assert(0); //not implemented
}

void arm_thread::clearCachedPC()
{
	assert(0); //not implemented
}

bool arm_thread::haveCachedPC(Address &pc)
{
	assert(0); //not implemented
   return true;
}
