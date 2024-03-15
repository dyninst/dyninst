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

#include "dataflowAPI/h/ABI.h"
#include "dataflowAPI/src/RegisterMap.h"
#include <stdio.h>

#if defined(arch_x86) || defined(arch_x86_64)
#  include "registers/x86_regs.h"
#  include "registers/x86_64_regs.h"
#endif

#if defined(arch_power)
#  include "registers/ppc32_regs.h"
#  include "registers/ppc64_regs.h"
#endif

#if defined(arch_aarch64)
#  include "registers/aarch64_regs.h"
#endif

using namespace Dyninst;
using namespace DataflowAPI;

// interface definition
struct idef {
  bitArray params;        // registers used to pass parameters to a function
  bitArray returnValues;  // registers used to return parameters from a function
  bitArray calleeSaved;   // registers whose value is preserved on function return
  bitArray global;        // registers that act as global variable by the ABI/runtime system
                          //  that are possibly read & written in the function and are valid after
                          //  a function such as `%rsp`, `%rip` and `%fs`.
};

struct liveness_t {
  bitArray callRead;        // Registers that may contain meaningful values to the function
  bitArray callWritten;     // Registers a function may overwrite
  bitArray returnRead;      // Registers that may have a meaningful value after a function returns
  bitArray returnValues;    // The same as idef::returnValues
  bitArray params;          // The same as idef::params
  bitArray syscallRead;     // Registers that may contain meaningful values to the syscall handler or OS
  bitArray syscallWritten;  // Registers the syscall handler or OS may overwrite
  bitArray all;             // All registers
};

dyn_tls liveness_t arch32, arch64;

dyn_tls ABI* ABI::globalABI_ = NULL;
dyn_tls ABI* ABI::globalABI64_ = NULL;

int ABI::getIndex(MachRegister machReg){
	if (index->find(machReg) == index->end()){
	    return -1;
	}
    	return (*index)[machReg];
}
std::map<MachRegister,int>* ABI::getIndexMap(){
	return index;
}

ABI* ABI::getABI(int addr_width){
    if (globalABI_ == NULL){
        globalABI_ = new ABI();
	globalABI_->addr_width = 4;
	globalABI64_ = new ABI();

#if defined(arch_x86) || defined(arch_x86_64)
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_x86();
	globalABI64_->index = &machRegIndex_x86_64();
#endif

#if defined(arch_power)
	globalABI64_->addr_width = 4;
	globalABI_->index = &machRegIndex_ppc();
	globalABI64_->index = &machRegIndex_ppc();

#endif

//#warning "This is not verified yet!"
#if defined(arch_aarch64)
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_aarch64();
	globalABI64_->index = &machRegIndex_aarch64();
#endif

// We _only_ support instrumenting 32-bit binaries on 64-bit systems
#if !defined arch_64bit || defined cap_32_64
	initialize32();
#endif

#ifdef arch_64bit
	initialize64();
#endif
    }
    return (addr_width == 4) ? globalABI_ : globalABI64_;
}


const bitArray &ABI::getCallReadRegisters() const {
    if (addr_width == 4)
        return arch32.callRead;
    else if (addr_width == 8)
        return arch64.callRead;
    else {
        assert(0);
        return arch32.callRead;
    }
}
const bitArray &ABI::getCallWrittenRegisters() const {
    if (addr_width == 4)
        return arch32.callWritten;
    else if (addr_width == 8)
        return arch64.callWritten;
    else {
        assert(0);
        return arch32.callWritten;
    }
}

const bitArray &ABI::getReturnReadRegisters() const {
    if (addr_width == 4)
        return arch32.returnRead;
    else if (addr_width == 8)
        return arch64.returnRead;
    else {
        assert(0);
        return arch32.returnRead;
    }
}

const bitArray &ABI::getReturnRegisters() const {
    if (addr_width == 4)
        return arch32.returnValues;
    else if (addr_width == 8)
        return arch64.returnValues;
    else {
        assert(0);
        return arch32.returnValues;
    }
}

const bitArray &ABI::getParameterRegisters() const {
    if (addr_width == 4)
        return arch32.params;
    else if (addr_width == 8)
        return arch64.params;
    else {
        assert(0);
        return arch32.params;
    }
}

const bitArray &ABI::getSyscallReadRegisters() const {
    if (addr_width == 4)
        return arch32.syscallRead;
    else if (addr_width == 8)
        return arch64.syscallRead;
    else {
        assert(0);
        return arch32.syscallRead;
    }
}
const bitArray &ABI::getSyscallWrittenRegisters() const {
    if (addr_width == 4)
        return arch32.syscallWritten;
    else if (addr_width == 8)
        return arch64.syscallWritten;
    else {
        assert(0);
        return arch32.syscallWritten;
    }
}

const bitArray &ABI::getAllRegs() const
{
   if (addr_width == 4)
      return arch32.all;
   else if (addr_width == 8)
      return arch64.all;
   else {
      assert(0);
      return arch32.all;
   }
}

bitArray ABI::getBitArray()  {
  return bitArray(index->size());
}

#if defined(arch_x86) || defined(arch_x86_64)
void ABI::initialize32(){
  idef defs;

  auto &reg_index = machRegIndex_x86();
  auto const num_bits = reg_index.size();

  defs.params.resize(num_bits);
  defs.returnValues.resize(num_bits);
  defs.calleeSaved.resize(num_bits);
  defs.global.resize(num_bits);

  defs.returnValues[reg_index[x86::eax]] = true;

  defs.calleeSaved[reg_index[x86::ebx]] = true;
  defs.calleeSaved[reg_index[x86::esi]] = true;
  defs.calleeSaved[reg_index[x86::edi]] = true;

  // PLT entries use ebx
  defs.global[reg_index[x86::ebx]] = true;

  // Assume calls write flags
  defs.global[reg_index[x86::of]] = true;
  defs.global[reg_index[x86::sf]] = true;
  defs.global[reg_index[x86::zf]] = true;
  defs.global[reg_index[x86::af]] = true;
  defs.global[reg_index[x86::pf]] = true;
  defs.global[reg_index[x86::cf]] = true;
  defs.global[reg_index[x86::tf]] = true;
  defs.global[reg_index[x86::if_]] = true;
  defs.global[reg_index[x86::df]] = true;
  defs.global[reg_index[x86::nt_]] = true;
  defs.global[reg_index[x86::rf]] = true;

  arch32.all.set();

  arch32.params = defs.params;

  arch32.returnValues = defs.returnValues;

  arch32.callRead = defs.params | defs.global;
  arch32.callRead[reg_index[x86::ecx]] = true;
  arch32.callRead[reg_index[x86::edx]] = true;

  arch32.callWritten = ~defs.calleeSaved | defs.global;

  arch32.returnRead = defs.returnValues | defs.calleeSaved | defs.global;

  // And assume a syscall reads or writes _everything_
  arch32.syscallRead.resize(num_bits);
  arch32.syscallRead.set();

  arch32.syscallWritten.resize(num_bits);
  arch32.syscallWritten.set();

  arch32.all.resize(num_bits);

#if defined(os_windows)
    // VERY conservative, but it's safe wrt the ABI.
    // Let's set everything and unset flags
    arch32.callRead = syscallRead_;
    arch32.callRead[reg_index[x86::of]] = false;
    arch32.callRead[reg_index[x86::sf]] = false;
    arch32.callRead[reg_index[x86::zf]] = false;
    arch32.callRead[reg_index[x86::af]] = false;
    arch32.callRead[reg_index[x86::pf]] = false;
    arch32.callRead[reg_index[x86::cf]] = false;
    arch32.callRead[reg_index[x86::tf]] = false;
    arch32.callRead[reg_index[x86::if_]] = false;
    arch32.callRead[reg_index[x86::df]] = false;
    arch32.callRead[reg_index[x86::nt_]] = false;
    arch32.callRead[reg_index[x86::rf]] = false;

    arch32.callWritten = arch32.syscallWritten;

// IF DEFINED KEVIN FUNKY MODE
  arch32.returnRead = arch32.callRead;
  // Doesn't exist, but should
  //returnWritten_ = callerSaved_;
// ENDIF DEFINED KEVIN FUNKY MODE

#endif
}
void ABI::initialize64(){
  idef defs;

  auto &reg_index = machRegIndex_x86_64();
  auto const num_bits = reg_index.size();

  defs.params.resize(num_bits);
  defs.returnValues.resize(num_bits);
  defs.calleeSaved.resize(num_bits);
  defs.global.resize(num_bits);

  defs.returnValues[reg_index[x86_64::rax]] = true;
  defs.returnValues[reg_index[x86_64::rdx]] = true;
  defs.returnValues[reg_index[x86_64::xmm0]] = true;
  defs.returnValues[reg_index[x86_64::xmm1]] = true;

  defs.params[reg_index[x86_64::rdi]] = true;
  defs.params[reg_index[x86_64::rsi]] = true;
  defs.params[reg_index[x86_64::rdx]] = true;
  defs.params[reg_index[x86_64::rcx]] = true;
  defs.params[reg_index[x86_64::r8]] = true;
  defs.params[reg_index[x86_64::r9]] = true;
  defs.params[reg_index[x86_64::xmm0]] = true;
  defs.params[reg_index[x86_64::xmm1]] = true;
  defs.params[reg_index[x86_64::xmm2]] = true;
  defs.params[reg_index[x86_64::xmm3]] = true;
  defs.params[reg_index[x86_64::xmm4]] = true;
  defs.params[reg_index[x86_64::xmm5]] = true;
  defs.params[reg_index[x86_64::xmm6]] = true;
  defs.params[reg_index[x86_64::xmm7]] = true;

  defs.calleeSaved[reg_index[x86_64::rbx]] = true;
  defs.calleeSaved[reg_index[x86_64::rdx]] = true;
  defs.calleeSaved[reg_index[x86_64::r12]] = true;
  defs.calleeSaved[reg_index[x86_64::r13]] = true;
  defs.calleeSaved[reg_index[x86_64::r14]] = true;
  defs.calleeSaved[reg_index[x86_64::r15]] = true;

  // Assume calls write flags
  defs.global[reg_index[x86_64::of]] = true;
  defs.global[reg_index[x86_64::sf]] = true;
  defs.global[reg_index[x86_64::zf]] = true;
  defs.global[reg_index[x86_64::af]] = true;
  defs.global[reg_index[x86_64::pf]] = true;
  defs.global[reg_index[x86_64::cf]] = true;
  defs.global[reg_index[x86_64::tf]] = true;
  defs.global[reg_index[x86_64::if_]] = true;
  defs.global[reg_index[x86_64::df]] = true;
  defs.global[reg_index[x86_64::nt_]] = true;
  defs.global[reg_index[x86_64::rf]] = true;

  arch64.all.resize(num_bits);
  arch64.all.set();

  arch64.params = defs.params;

  arch64.returnValues = defs.returnValues;

  arch64.callRead = defs.params | defs.global;
  arch64.callRead[reg_index[x86_64::rax]] = true;

  arch64.callWritten = ~defs.calleeSaved | defs.global;
  arch64.callWritten[reg_index[x86_64::rax]] = true;
  arch64.callWritten[reg_index[x86_64::r10]] = true;
  arch64.callWritten[reg_index[x86_64::r11]] = true;

  arch64.returnRead = defs.returnValues | defs.calleeSaved | defs.global;
  arch64.returnRead[reg_index[x86_64::rax]] = true;
  arch64.returnRead[reg_index[x86_64::rcx]] = true; //Not correct, temporary

  // And assume a syscall reads or writes _everything_
  arch64.syscallRead.resize(num_bits);
  arch64.syscallRead.set();

  arch64.syscallWritten.resize(num_bits);
  arch64.syscallWritten.set();
}

#endif

#if defined(arch_power)
void ABI::initialize32(){

  auto &reg_index = machRegIndex_ppc();
  auto const num_bits = reg_index.size();

  arch32.all.resize(num_bits);
  arch32.callRead.resize(num_bits);
  arch32.callWritten.resize(num_bits);
  arch32.params.resize(num_bits);
  arch32.returnRead.resize(num_bits);
  arch32.returnValues.resize(num_bits);
  arch32.syscallRead.resize(num_bits);
  arch32.syscallWritten.resize(num_bits);

  arch32.returnValues[reg_index[ppc32::r3]] = true;

  arch32.params[reg_index[ppc32::r3]] = true;
  arch32.params[reg_index[ppc32::r4]] = true;
  arch32.params[reg_index[ppc32::r5]] = true;
  arch32.params[reg_index[ppc32::r6]] = true;
  arch32.params[reg_index[ppc32::r7]] = true;
  arch32.params[reg_index[ppc32::r8]] = true;
  arch32.params[reg_index[ppc32::r9]] = true;
  arch32.params[reg_index[ppc32::r10]] = true;

  // Return reads r3, r4, fpr1, fpr2
  arch32.returnRead[reg_index[ppc32::r3]] = true;
  arch32.returnRead[reg_index[ppc32::r4]] = true;
  arch32.returnRead[reg_index[ppc32::fpr1]] = true;
  arch32.returnRead[reg_index[ppc32::fpr2]] = true;

  // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)
  arch32.callRead[reg_index[ppc32::r3]] = true;
  arch32.callRead[reg_index[ppc32::r4]] = true;
  arch32.callRead[reg_index[ppc32::r5]] = true;
  arch32.callRead[reg_index[ppc32::r6]] = true;
  arch32.callRead[reg_index[ppc32::r7]] = true;
  arch32.callRead[reg_index[ppc32::r8]] = true;
  arch32.callRead[reg_index[ppc32::r9]] = true;
  arch32.callRead[reg_index[ppc32::r10]] = true;

  arch32.callRead[reg_index[ppc32::fpr1]] = true;
  arch32.callRead[reg_index[ppc32::fpr2]] = true;
  arch32.callRead[reg_index[ppc32::fpr3]] = true;
  arch32.callRead[reg_index[ppc32::fpr4]] = true;
  arch32.callRead[reg_index[ppc32::fpr5]] = true;
  arch32.callRead[reg_index[ppc32::fpr6]] = true;
  arch32.callRead[reg_index[ppc32::fpr7]] = true;
  arch32.callRead[reg_index[ppc32::fpr8]] = true;
  arch32.callRead[reg_index[ppc32::fpr9]] = true;
  arch32.callRead[reg_index[ppc32::fpr10]] = true;
  arch32.callRead[reg_index[ppc32::fpr11]] = true;
  arch32.callRead[reg_index[ppc32::fpr12]] = true;
  arch32.callRead[reg_index[ppc32::fpr13]] = true;

  // Calls write to pretty much every register we use for code generation
  arch32.callWritten[reg_index[ppc32::r0]] = true;

  arch32.callWritten[reg_index[ppc32::r3]] = true;
  arch32.callWritten[reg_index[ppc32::r4]] = true;
  arch32.callWritten[reg_index[ppc32::r5]] = true;
  arch32.callWritten[reg_index[ppc32::r6]] = true;
  arch32.callWritten[reg_index[ppc32::r7]] = true;
  arch32.callWritten[reg_index[ppc32::r8]] = true;
  arch32.callWritten[reg_index[ppc32::r9]] = true;
  arch32.callWritten[reg_index[ppc32::r10]] = true;
  arch32.callWritten[reg_index[ppc32::r11]] = true;
  arch32.callWritten[reg_index[ppc32::r12]] = true;

  arch32.callWritten[reg_index[ppc32::fpr0]] = true;
  arch32.callWritten[reg_index[ppc32::fpr1]] = true;
  arch32.callWritten[reg_index[ppc32::fpr2]] = true;
  arch32.callWritten[reg_index[ppc32::fpr3]] = true;
  arch32.callWritten[reg_index[ppc32::fpr4]] = true;
  arch32.callWritten[reg_index[ppc32::fpr5]] = true;
  arch32.callWritten[reg_index[ppc32::fpr6]] = true;
  arch32.callWritten[reg_index[ppc32::fpr7]] = true;
  arch32.callWritten[reg_index[ppc32::fpr8]] = true;
  arch32.callWritten[reg_index[ppc32::fpr9]] = true;
  arch32.callWritten[reg_index[ppc32::fpr10]] = true;
  arch32.callWritten[reg_index[ppc32::fpr11]] = true;
  arch32.callWritten[reg_index[ppc32::fpr12]] = true;
  arch32.callWritten[reg_index[ppc32::fpr13]] = true;

  arch32.syscallRead = arch32.callRead;
  arch32.syscallRead[reg_index[ppc32::r0]] = true;

  arch32.syscallWritten = arch32.callWritten;

  arch32.all.set();
}

void ABI::initialize64(){
  auto &reg_index = machRegIndex_ppc_64();
  auto const num_bits = reg_index.size();

  arch64.all.resize(num_bits);
  arch64.callRead.resize(num_bits);
  arch64.callWritten.resize(num_bits);
  arch64.params.resize(num_bits);
  arch64.returnRead.resize(num_bits);
  arch64.returnValues.resize(num_bits);
  arch64.syscallRead.resize(num_bits);
  arch64.syscallWritten.resize(num_bits);

  arch64.returnValues[reg_index[ppc64::r3]] = true;

  arch64.params[reg_index[ppc64::r3]] = true;
  arch64.params[reg_index[ppc64::r4]] = true;
  arch64.params[reg_index[ppc64::r5]] = true;
  arch64.params[reg_index[ppc64::r6]] = true;
  arch64.params[reg_index[ppc64::r7]] = true;
  arch64.params[reg_index[ppc64::r8]] = true;
  arch64.params[reg_index[ppc64::r9]] = true;
  arch64.params[reg_index[ppc64::r10]] = true;

  // Return reads r3, r4, fpr1, fpr2
  arch64.returnRead[reg_index[ppc64::r3]] = true;
  arch64.returnRead[reg_index[ppc64::r4]] = true;
  arch64.returnRead[reg_index[ppc64::fpr3]] = true;
  arch64.returnRead[reg_index[ppc64::fpr2]] = true;

  // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)
  arch64.callRead[reg_index[ppc64::r3]] = true;
  arch64.callRead[reg_index[ppc64::r4]] = true;
  arch64.callRead[reg_index[ppc64::r5]] = true;
  arch64.callRead[reg_index[ppc64::r6]] = true;
  arch64.callRead[reg_index[ppc64::r7]] = true;
  arch64.callRead[reg_index[ppc64::r8]] = true;
  arch64.callRead[reg_index[ppc64::r9]] = true;
  arch64.callRead[reg_index[ppc64::r10]] = true;

  arch64.callRead[reg_index[ppc64::fpr1]] = true;
  arch64.callRead[reg_index[ppc64::fpr2]] = true;
  arch64.callRead[reg_index[ppc64::fpr3]] = true;
  arch64.callRead[reg_index[ppc64::fpr4]] = true;
  arch64.callRead[reg_index[ppc64::fpr5]] = true;
  arch64.callRead[reg_index[ppc64::fpr6]] = true;
  arch64.callRead[reg_index[ppc64::fpr7]] = true;
  arch64.callRead[reg_index[ppc64::fpr8]] = true;
  arch64.callRead[reg_index[ppc64::fpr9]] = true;
  arch64.callRead[reg_index[ppc64::fpr10]] = true;
  arch64.callRead[reg_index[ppc64::fpr11]] = true;
  arch64.callRead[reg_index[ppc64::fpr12]] = true;
  arch64.callRead[reg_index[ppc64::fpr13]] = true;

  // Calls write to pretty much every register we use for code generation
  arch64.callWritten[reg_index[ppc64::r0]] = true;

  arch64.callWritten[reg_index[ppc64::r3]] = true;
  arch64.callWritten[reg_index[ppc64::r4]] = true;
  arch64.callWritten[reg_index[ppc64::r5]] = true;
  arch64.callWritten[reg_index[ppc64::r6]] = true;
  arch64.callWritten[reg_index[ppc64::r7]] = true;
  arch64.callWritten[reg_index[ppc64::r8]] = true;
  arch64.callWritten[reg_index[ppc64::r9]] = true;
  arch64.callWritten[reg_index[ppc64::r10]] = true;
  arch64.callWritten[reg_index[ppc64::r11]] = true;
  arch64.callWritten[reg_index[ppc64::r12]] = true;

  arch64.callWritten[reg_index[ppc64::fpr0]] = true;
  arch64.callWritten[reg_index[ppc64::fpr1]] = true;
  arch64.callWritten[reg_index[ppc64::fpr2]] = true;
  arch64.callWritten[reg_index[ppc64::fpr3]] = true;
  arch64.callWritten[reg_index[ppc64::fpr4]] = true;
  arch64.callWritten[reg_index[ppc64::fpr5]] = true;
  arch64.callWritten[reg_index[ppc64::fpr6]] = true;
  arch64.callWritten[reg_index[ppc64::fpr7]] = true;
  arch64.callWritten[reg_index[ppc64::fpr8]] = true;
  arch64.callWritten[reg_index[ppc64::fpr9]] = true;
  arch64.callWritten[reg_index[ppc64::fpr10]] = true;
  arch64.callWritten[reg_index[ppc64::fpr11]] = true;
  arch64.callWritten[reg_index[ppc64::fpr12]] = true;
  arch64.callWritten[reg_index[ppc64::fpr13]] = true;

  // Syscall - assume the same as call
  arch64.syscallRead.set();
  arch64.syscallWritten.set();

  arch64.all.set();
}
#endif

//#warning "This is not verified!"
#if defined(arch_aarch64)
void ABI::initialize64(){
  auto &reg_index = machRegIndex_aarch64();
  auto const num_bits = reg_index.size();

  arch64.all.resize(num_bits);
  arch64.callRead.resize(num_bits);
  arch64.callWritten.resize(num_bits);
  arch64.params.resize(num_bits);
  arch64.returnRead.resize(num_bits);
  arch64.returnValues.resize(num_bits);
  arch64.syscallRead.resize(num_bits);
  arch64.syscallWritten.resize(num_bits);

  arch64.returnValues[reg_index[aarch64::x0]] = true;
  arch64.returnValues[reg_index[aarch64::q0]] = true;

  arch64.returnRead[reg_index[aarch64::x0]] = true;
  arch64.returnRead[reg_index[aarch64::q0]] = true;

  //Callee-saved registers
  //First, GPRs...
  arch64.returnRead[reg_index[aarch64::x19]] = true;
  arch64.returnRead[reg_index[aarch64::x20]] = true;
  arch64.returnRead[reg_index[aarch64::x21]] = true;
  arch64.returnRead[reg_index[aarch64::x22]] = true;
  arch64.returnRead[reg_index[aarch64::x23]] = true;
  arch64.returnRead[reg_index[aarch64::x24]] = true;
  arch64.returnRead[reg_index[aarch64::x25]] = true;
  arch64.returnRead[reg_index[aarch64::x26]] = true;
  arch64.returnRead[reg_index[aarch64::x27]] = true;
  arch64.returnRead[reg_index[aarch64::x28]] = true;
  arch64.returnRead[reg_index[aarch64::sp]] = true;

  //Now, SIMD regs...
  arch64.returnRead[reg_index[aarch64::q8]] = true;
  arch64.returnRead[reg_index[aarch64::q9]] = true;
  arch64.returnRead[reg_index[aarch64::q10]] = true;
  arch64.returnRead[reg_index[aarch64::q11]] = true;
  arch64.returnRead[reg_index[aarch64::q12]] = true;
  arch64.returnRead[reg_index[aarch64::q13]] = true;
  arch64.returnRead[reg_index[aarch64::q14]] = true;
  arch64.returnRead[reg_index[aarch64::q15]] = true;

  arch64.params[reg_index[aarch64::x0]] = true;
  arch64.params[reg_index[aarch64::x1]] = true;
  arch64.params[reg_index[aarch64::x2]] = true;
  arch64.params[reg_index[aarch64::x3]] = true;
  arch64.params[reg_index[aarch64::x4]] = true;
  arch64.params[reg_index[aarch64::x5]] = true;
  arch64.params[reg_index[aarch64::x6]] = true;
  arch64.params[reg_index[aarch64::x7]] = true;

  //First, GPRs...
  arch64.callRead[reg_index[aarch64::x0]] = true;
  arch64.callRead[reg_index[aarch64::x1]] = true;
  arch64.callRead[reg_index[aarch64::x2]] = true;
  arch64.callRead[reg_index[aarch64::x3]] = true;
  arch64.callRead[reg_index[aarch64::x4]] = true;
  arch64.callRead[reg_index[aarch64::x5]] = true;
  arch64.callRead[reg_index[aarch64::x6]] = true;
  arch64.callRead[reg_index[aarch64::x7]] = true;
  //Now, SIMD regs...
  arch64.callRead[reg_index[aarch64::q0]] = true;
  arch64.callRead[reg_index[aarch64::q1]] = true;
  arch64.callRead[reg_index[aarch64::q2]] = true;
  arch64.callRead[reg_index[aarch64::q3]] = true;
  arch64.callRead[reg_index[aarch64::q4]] = true;
  arch64.callRead[reg_index[aarch64::q5]] = true;
  arch64.callRead[reg_index[aarch64::q6]] = true;
  arch64.callRead[reg_index[aarch64::q7]] = true;

  arch64.callWritten = arch64.callRead;

  //First, GPRs...
  arch64.callWritten[reg_index[aarch64::x9]] = true;
  arch64.callWritten[reg_index[aarch64::x10]] = true;
  arch64.callWritten[reg_index[aarch64::x11]] = true;
  arch64.callWritten[reg_index[aarch64::x12]] = true;
  arch64.callWritten[reg_index[aarch64::x13]] = true;
  arch64.callWritten[reg_index[aarch64::x14]] = true;
  arch64.callWritten[reg_index[aarch64::x15]] = true;

  //Now, SIMD regs...
  arch64.callWritten[reg_index[aarch64::q16]] = true;
  arch64.callWritten[reg_index[aarch64::q17]] = true;
  arch64.callWritten[reg_index[aarch64::q18]] = true;
  arch64.callWritten[reg_index[aarch64::q19]] = true;
  arch64.callWritten[reg_index[aarch64::q20]] = true;
  arch64.callWritten[reg_index[aarch64::q21]] = true;
  arch64.callWritten[reg_index[aarch64::q22]] = true;
  arch64.callWritten[reg_index[aarch64::q23]] = true;
  arch64.callWritten[reg_index[aarch64::q24]] = true;
  arch64.callWritten[reg_index[aarch64::q25]] = true;
  arch64.callWritten[reg_index[aarch64::q26]] = true;
  arch64.callWritten[reg_index[aarch64::q27]] = true;
  arch64.callWritten[reg_index[aarch64::q28]] = true;
  arch64.callWritten[reg_index[aarch64::q29]] = true;
  arch64.callWritten[reg_index[aarch64::q30]] = true;
  arch64.callWritten[reg_index[aarch64::q31]] = true;

  arch64.syscallRead.set();
  arch64.syscallWritten.set();

  arch64.all.set();
}
#endif
