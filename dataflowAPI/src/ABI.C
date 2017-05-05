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

using namespace Dyninst;
using namespace DataflowAPI;

bitArray ABI::callRead_;
bitArray ABI::callWritten_;
bitArray ABI::returnRead_;
bitArray ABI::returnRegs_;
bitArray ABI::callParam_;
bitArray ABI::syscallRead_;
bitArray ABI::syscallWritten_;

bitArray ABI::callRead64_;
bitArray ABI::callWritten64_;
bitArray ABI::returnRead64_;
bitArray ABI::returnRegs64_;
bitArray ABI::callParam64_;
bitArray ABI::syscallRead64_;
bitArray ABI::syscallWritten64_;
bitArray ABI::allRegs_;
bitArray ABI::allRegs64_;
ABI* ABI::globalABI_ = NULL;
ABI* ABI::globalABI64_ = NULL;

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
	globalABI_->index = &machRegIndex_ppc();
	globalABI64_->index = &machRegIndex_ppc();
#endif

	initialize32();
#if defined(cap_32_64)
	initialize64();
#endif
    }
    return (addr_width == 4) ? globalABI_ : globalABI64_;
}


const bitArray &ABI::getCallReadRegisters() const {
    if (addr_width == 4)
        return callRead_;
    else if (addr_width == 8)
        return callRead64_;
    else {
        assert(0);
        return callRead_;
    }
}
const bitArray &ABI::getCallWrittenRegisters() const {
    if (addr_width == 4)
        return callWritten_;
    else if (addr_width == 8)
        return callWritten64_;
    else {
        assert(0);
        return callWritten_;
    }
}

const bitArray &ABI::getReturnReadRegisters() const {
    if (addr_width == 4)
        return returnRead_;
    else if (addr_width == 8)
        return returnRead64_;
    else {
        assert(0);
        return returnRead_;
    }
}

const bitArray &ABI::getReturnRegisters() const {
    if (addr_width == 4)
        return returnRegs_;
    else if (addr_width == 8)
        return returnRegs64_;
    else {
        assert(0);
        return returnRegs_;
    }
}

const bitArray &ABI::getParameterRegisters() const {
    if (addr_width == 4)
        return callParam_;
    else if (addr_width == 8)
        return callParam64_;
    else {
        assert(0);
        return callParam_;
    }
}

const bitArray &ABI::getSyscallReadRegisters() const {
    if (addr_width == 4)
        return syscallRead_;
    else if (addr_width == 8)
        return syscallRead64_;
    else {
        assert(0);
        return syscallRead_;
    }
}
const bitArray &ABI::getSyscallWrittenRegisters() const {
    if (addr_width == 4)
        return syscallWritten_;
    else if (addr_width == 8)
        return syscallWritten64_;
    else {
        assert(0);
        return syscallWritten_;
    }
}

const bitArray &ABI::getAllRegs() const
{
   if (addr_width == 4)
      return allRegs_;
   else if (addr_width == 8)
      return allRegs64_;
   else {
      assert(0);
      return allRegs_;
   }
}

bitArray ABI::getBitArray()  {
  return bitArray(index->size());
}
#if defined(arch_x86) || defined(arch_x86_64)
void ABI::initialize32(){

   returnRegs_ = getBitArray(machRegIndex_x86().size());
   returnRegs_[machRegIndex_x86()[x86::eax]] = true;

   callParam_ = getBitArray(machRegIndex_x86().size());

   returnRead_ = getBitArray(machRegIndex_x86().size());
   // Callee-save registers...
   returnRead_[machRegIndex_x86()[x86::ebx]] = true;
   returnRead_[machRegIndex_x86()[x86::esi]] = true;
   returnRead_[machRegIndex_x86()[x86::edi]] = true;
   // And return value
   returnRead_[machRegIndex_x86()[x86::eax]] = true;
   // Return reads no registers

   callRead_ = getBitArray(machRegIndex_x86().size());
   // CallRead reads no registers
   // We wish...
   callRead_[machRegIndex_x86()[x86::ecx]] = true;
   callRead_[machRegIndex_x86()[x86::edx]] = true;

   // PLT entries use ebx
   callRead_[machRegIndex_x86()[x86::ebx]] = true;

   // TODO: Fix this for platform-specific calling conventions

   // Assume calls write flags
   callWritten_ = callRead_;

   callWritten_[machRegIndex_x86()[x86::of]] = true;
   callWritten_[machRegIndex_x86()[x86::sf]] = true;
   callWritten_[machRegIndex_x86()[x86::zf]] = true;
   callWritten_[machRegIndex_x86()[x86::af]] = true;
   callWritten_[machRegIndex_x86()[x86::pf]] = true;
   callWritten_[machRegIndex_x86()[x86::cf]] = true;
   callWritten_[machRegIndex_x86()[x86::tf]] = true;
   callWritten_[machRegIndex_x86()[x86::if_]] = true;
   callWritten_[machRegIndex_x86()[x86::df]] = true;
   callWritten_[machRegIndex_x86()[x86::nt_]] = true;
   callWritten_[machRegIndex_x86()[x86::rf]] = true;



    // And eax...
    callWritten_[machRegIndex_x86()[x86::eax]] = true;


    // And assume a syscall reads or writes _everything_
    syscallRead_ = getBitArray(machRegIndex_x86().size()).set();
    syscallWritten_ = syscallRead_;

#if defined(os_windows)
    // VERY conservative, but it's safe wrt the ABI.
    // Let's set everything and unset flags
    callRead_ = syscallRead_;
    callRead_[machRegIndex_x86()[x86::of]] = false;
    callRead_[machRegIndex_x86()[x86::sf]] = false;
    callRead_[machRegIndex_x86()[x86::zf]] = false;
    callRead_[machRegIndex_x86()[x86::af]] = false;
    callRead_[machRegIndex_x86()[x86::pf]] = false;
    callRead_[machRegIndex_x86()[x86::cf]] = false;
    callRead_[machRegIndex_x86()[x86::tf]] = false;
    callRead_[machRegIndex_x86()[x86::if_]] = false;
    callRead_[machRegIndex_x86()[x86::df]] = false;
    callRead_[machRegIndex_x86()[x86::nt_]] = false;
    callRead_[machRegIndex_x86()[x86::rf]] = false;


    callWritten_ = syscallWritten_;

// IF DEFINED KEVIN FUNKY MODE
	returnRead_ = callRead_;
	// Doesn't exist, but should
	//returnWritten_ = callWritten_;
// ENDIF DEFINED KEVIN FUNKY MODE


#endif

        allRegs_ = getBitArray(machRegIndex_x86().size()).set();

}

void ABI::initialize64(){

    returnRegs64_ = getBitArray(machRegIndex_x86_64().size());
    returnRegs64_[machRegIndex_x86_64()[x86_64::rax]] = true;
    returnRegs64_[machRegIndex_x86_64()[x86_64::rdx]] = true;

    callParam64_ = getBitArray(machRegIndex_x86_64().size());
    callParam64_[machRegIndex_x86_64()[x86_64::rdi]] = true;
    callParam64_[machRegIndex_x86_64()[x86_64::rsi]] = true;
    callParam64_[machRegIndex_x86_64()[x86_64::rdx]] = true;
    callParam64_[machRegIndex_x86_64()[x86_64::rcx]] = true;
    callParam64_[machRegIndex_x86_64()[x86_64::r8]] = true;
    callParam64_[machRegIndex_x86_64()[x86_64::r9]] = true;

    returnRead64_ = getBitArray(machRegIndex_x86_64().size());
    returnRead64_[machRegIndex_x86_64()[x86_64::rax]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::rcx]] = true; //Not correct, temporary
    // Returns also "read" any callee-saved registers
    returnRead64_[machRegIndex_x86_64()[x86_64::rbx]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::rdx]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::r12]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::r13]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::r14]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::r15]] = true;

    returnRead64_[machRegIndex_x86_64()[x86_64::xmm0]] = true;
    returnRead64_[machRegIndex_x86_64()[x86_64::xmm1]] = true;


    callRead64_ = getBitArray(machRegIndex_x86_64().size());
    callRead64_[machRegIndex_x86_64()[x86_64::rax]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::rcx]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::rdx]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::r8]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::r9]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::rdi]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::rsi]] = true;

    callRead64_[machRegIndex_x86_64()[x86_64::xmm0]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm1]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm2]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm3]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm4]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm5]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm6]] = true;
    callRead64_[machRegIndex_x86_64()[x86_64::xmm7]] = true;

    // Anything in those four is not preserved across a call...
    // So we copy this as a shorthand then augment it
    callWritten64_ = callRead64_;

    // As well as RAX, R10, R11
    callWritten64_[machRegIndex_x86_64()[x86_64::rax]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::r10]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::r11]] = true;
    // And flags

    callWritten64_[machRegIndex_x86_64()[x86_64::of]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::sf]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::zf]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::af]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::pf]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::cf]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::tf]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::if_]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::df]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::nt_]] = true;
    callWritten64_[machRegIndex_x86_64()[x86_64::rf]] = true;


    // And assume a syscall reads or writes _everything_
    syscallRead64_ = getBitArray(machRegIndex_x86_64().size()).set();
    syscallWritten64_ = syscallRead64_;

    allRegs64_ = getBitArray(machRegIndex_x86_64().size()).set();
}

#endif

#if defined(arch_power)
void ABI::initialize32(){
    returnRegs_ = getBitArray(machRegIndex_ppc().size());
    returnRegs_[machRegIndex_ppc()[ppc32::r3]] = true;

    callParam_ = getBitArray(machRegIndex_ppc().size());
    callParam_[machRegIndex_ppc()[ppc32::r3]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r4]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r5]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r6]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r7]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r8]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r9]] = true;
    callParam_[machRegIndex_ppc()[ppc32::r10]] = true;


    returnRead_ = getBitArray(machRegIndex_ppc().size());
    // Return reads r3, r4, fpr1, fpr2
    returnRead_[machRegIndex_ppc()[ppc32::r3]] = true;
    returnRead_[machRegIndex_ppc()[ppc32::r4]] = true;
    returnRead_[machRegIndex_ppc()[ppc32::fpr1]] = true;
    returnRead_[machRegIndex_ppc()[ppc32::fpr2]] = true;

    // Calls
    callRead_ = getBitArray(machRegIndex_ppc().size());
    // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)
/*    for (unsigned i = r3; i <= r10; i++)
        callRead_[i] = true;
    for (unsigned i = fpr1; i <= fpr13; i++)
        callRead_[i] = true;*/

    callRead_[machRegIndex_ppc()[ppc32::r3]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r4]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r5]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r6]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r7]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r8]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r9]] = true;
    callRead_[machRegIndex_ppc()[ppc32::r10]] = true;

    callRead_[machRegIndex_ppc()[ppc32::fpr1]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr2]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr3]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr4]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr5]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr6]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr7]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr8]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr9]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr10]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr11]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr12]] = true;
    callRead_[machRegIndex_ppc()[ppc32::fpr13]] = true;

    callWritten_ = getBitArray(machRegIndex_ppc().size());
    // Calls write to pretty much every register we use for code generation
    callWritten_[machRegIndex_ppc()[ppc32::r0]] = true;
/*    for (unsigned i = r3; i <= r12; i++)
        callWritten_[i] = true;
    // FPRs 0->13 are volatile
    for (unsigned i = fpr0; i <= fpr13; i++)
        callWritten_[i] = true;*/

    callWritten_[machRegIndex_ppc()[ppc32::r3]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r4]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r5]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r6]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r7]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r8]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r9]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r10]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r11]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::r12]] = true;

    callWritten_[machRegIndex_ppc()[ppc32::fpr0]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr1]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr2]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr3]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr4]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr5]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr6]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr7]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr8]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr9]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr10]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr11]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr12]] = true;
    callWritten_[machRegIndex_ppc()[ppc32::fpr13]] = true;

    // Syscall - assume the same as call
    //syscallRead_ = getBitArray().set();
    //syscallWritten_ = getBitArray().set();
    syscallRead_ = callRead_;
    syscallRead_[machRegIndex_ppc()[ppc32::r0]] = true;
    syscallWritten_ = callWritten_;

    allRegs_ = getBitArray(machRegIndex_ppc().size()).set();
}

void ABI::initialize64(){
    returnRegs64_ = getBitArray(machRegIndex_ppc_64().size());
    returnRegs64_[machRegIndex_ppc_64()[ppc64::r3]] = true;

    callParam64_ = getBitArray(machRegIndex_ppc_64().size());
    callParam64_[machRegIndex_ppc_64()[ppc64::r3]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r4]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r5]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r6]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r7]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r8]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r9]] = true;
    callParam64_[machRegIndex_ppc_64()[ppc64::r10]] = true;

    returnRead64_ = getBitArray(machRegIndex_ppc_64().size());
    // Return reads r3, r4, fpr1, fpr2
    returnRead64_[machRegIndex_ppc_64()[ppc64::r3]] = true;
    returnRead64_[machRegIndex_ppc_64()[ppc64::r4]] = true;
    returnRead64_[machRegIndex_ppc_64()[ppc64::fpr3]] = true;
    returnRead64_[machRegIndex_ppc_64()[ppc64::fpr2]] = true;

    // Calls
    callRead64_ = getBitArray(machRegIndex_ppc_64().size());
    // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)

    callRead64_[machRegIndex_ppc_64()[ppc64::r3]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r4]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r5]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r6]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r7]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r8]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r9]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::r10]] = true;

    callRead64_[machRegIndex_ppc_64()[ppc64::fpr1]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr2]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr3]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr4]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr5]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr6]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr7]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr8]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr9]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr10]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr11]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr12]] = true;
    callRead64_[machRegIndex_ppc_64()[ppc64::fpr13]] = true;


    callWritten64_ = getBitArray(machRegIndex_ppc_64().size());
    // Calls write to pretty much every register we use for code generation
    callWritten64_[machRegIndex_ppc_64()[ppc64::r0]] = true;

    callWritten64_[machRegIndex_ppc_64()[ppc64::r3]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r4]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r5]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r6]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r7]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r8]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r9]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r10]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r11]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::r12]] = true;

    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr0]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr1]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr2]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr3]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr4]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr5]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr6]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr7]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr8]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr9]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr10]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr11]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr12]] = true;
    callWritten64_[machRegIndex_ppc_64()[ppc64::fpr13]] = true;

    // Syscall - assume the same as call
    syscallRead64_ = getBitArray(machRegIndex_ppc_64().size()).set();
    syscallWritten64_ = getBitArray(machRegIndex_ppc_64().size()).set();

    allRegs64_ = getBitArray(machRegIndex_ppc_64().size()).set();
}
#endif

//#warning "This is not verified!"
#if defined(arch_aarch64)
void ABI::initialize32(){
	assert(0);
}

void ABI::initialize64(){
	assert(0);
}
#endif
