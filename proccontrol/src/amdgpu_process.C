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
#include "proccontrol/src/amdgpu_process.h"
#include "common/src/arch-amdgpu.h"

using namespace NS_aarch64;
using namespace std;

//constructors, blank functions
amdgpu_process::amdgpu_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                         std::vector<std::string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f) {
}

amdgpu_process::amdgpu_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p) {
}

amdgpu_process::~amdgpu_process() {
}

unsigned amdgpu_process::plat_breakpointSize() {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

void amdgpu_process::plat_breakpointBytes(unsigned char *buffer) {
  assert(false && "Not implemented for AMDGPU");
}

bool amdgpu_process::plat_breakpointAdvancesPC() const {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

bool amdgpu_process::plat_convertToBreakpointAddress(Address &, int_thread *) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

void amdgpu_process::cleanupSSOnContinue(int_thread *thr) {
  assert(false && "Not implemented for AMDGPU");
}

void amdgpu_process::registerSSClearCB() {
  assert(false && "Not implemented for AMDGPU");
  return aret_success;
}

async_ret_t amdgpu_process::readPCForSS(int_thread *thr, Address &pc) {
  assert(false && "Not implemented for AMDGPU");
  return aret_success;
}

async_ret_t amdgpu_process::readInsnForSS(Address pc, int_thread *, unsigned int &rawInsn) {
  assert(false && "Not implemented for AMDGPU");
  return aret_success;
}

async_ret_t amdgpu_process::plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Address> &addrResult) {
  assert(false && "Not implemented for AMDGPU");
  return aret_success;
}

void amdgpu_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr> resps) {
  assert(false && "Not implemented for AMDGPU");
}

// Thread functions implementations

amdgpu_thread::amdgpu_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l), have_cached_pc(false), cached_pc(0) {
}

amdgpu_thread::~amdgpu_thread() {
}

//#warning "HWBreakpoint is not supported now."
bool amdgpu_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

bool amdgpu_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &) {
  assert(false && "Not implemented for AMDGPU");
  return false;
}

unsigned amdgpu_thread::hwBPAvail(unsigned) {
  assert(false && "Not implemented for AMDGPU");
  return 0;
}

EventBreakpoint::ptr amdgpu_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal) {
  assert(false && "Not implemented for AMDGPU");
  return EventBreakpoint::ptr();
}

bool amdgpu_thread::bpNeedsClear(hw_breakpoint *) {
	assert(0); //not implemented
  return false;
}

void amdgpu_thread::setCachedPC(Address pc) {
  assert(false && "Not implemented for AMDGPU");
}

void amdgpu_thread::clearCachedPC() {
  assert(false && "Not implemented for AMDGPU");
}

bool amdgpu_thread::haveCachedPC(Address &pc) {
  assert(false && "Not implemented for AMDGPU");
}
