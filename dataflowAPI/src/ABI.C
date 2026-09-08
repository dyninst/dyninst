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
#include "dataflowAPI/src/ABIBridge.h"
#include <stdio.h>

#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
#  include "registers/x86_regs.h"
#  include "registers/x86_64_regs.h"
#endif

#if defined(DYNINST_CODEGEN_ARCH_POWER)
#  include "registers/ppc64_regs.h"
#endif

#if defined(DYNINST_CODEGEN_ARCH_AARCH64)
#  include "registers/aarch64_regs.h"
#endif

#if defined(DYNINST_CODEGEN_ARCH_RISCV64)
#  include "registers/riscv64_regs.h"
#endif

using namespace Dyninst;
using namespace DataflowAPI;

dyn_tls bitArray* ABI::callRead_ = NULL;
dyn_tls bitArray* ABI::callWritten_ = NULL;
dyn_tls bitArray* ABI::returnRead_ = NULL;
dyn_tls bitArray* ABI::returnRegs_ = NULL;
dyn_tls bitArray* ABI::callParam_ = NULL;
dyn_tls bitArray* ABI::syscallRead_ = NULL;
dyn_tls bitArray* ABI::syscallWritten_ = NULL;

dyn_tls bitArray* ABI::callRead64_ = NULL;
dyn_tls bitArray* ABI::callWritten64_ = NULL;
dyn_tls bitArray* ABI::returnRead64_ = NULL;
dyn_tls bitArray* ABI::returnRegs64_ = NULL;
dyn_tls bitArray* ABI::callParam64_ = NULL;
dyn_tls bitArray* ABI::syscallRead64_ = NULL;
dyn_tls bitArray* ABI::syscallWritten64_ = NULL;
dyn_tls bitArray* ABI::allRegs_ = NULL;
dyn_tls bitArray* ABI::allRegs64_ = NULL;
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

ABI* ABI::getABI(Architecture arch){
    if (globalABI_ == NULL){
	    globalABI64_ = new ABI();
	    globalABI64_->addr_width = 8;
        switch(arch) {
            case Arch_amdgpu_gfx908:
                globalABI64_->index = &machRegIndex_amdgpu_gfx908();
                break;
            case Arch_amdgpu_gfx90a:
                globalABI64_->index = &machRegIndex_amdgpu_gfx90a();
                break;
            case Arch_amdgpu_gfx940:
                globalABI64_->index = &machRegIndex_amdgpu_gfx940();
                break;
            case Arch_amdgpu_gfx950:
                globalABI64_->index = &machRegIndex_amdgpu_gfx950();
                break;
            default:
                assert(0 && "getABI(arch) currently only support AMDGPU!");
                break;
        }
        initialize64(arch);
    }
    if (arch != Arch_amdgpu_gfx908 && arch!= Arch_amdgpu_gfx90a && arch != Arch_amdgpu_gfx940 && arch != Arch_amdgpu_gfx950){
        assert(0 && "getABI(arch) currently only support AMDGPU!");
    }
    return globalABI64_;
}

ABI* ABI::getABI(int addr_width){
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    return getABI(Arch_amdgpu_gfx908);
#endif
    if (globalABI_ == NULL){
        globalABI_ = new ABI();
	globalABI_->addr_width = 4;
	globalABI64_ = new ABI();

#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_x86();
	globalABI64_->index = &machRegIndex_x86_64();
#endif

#if defined(DYNINST_CODEGEN_ARCH_POWER)
	// PowerPC is 64-bit only (ppc32 is unsupported, #1145), so both slots use
	// the ppc64 register map. Using the ppc32 map for the 64-bit ABI is what
	// made getIndex() return -1 for ppc64 registers, crashing liveness and
	// registerSpace (#1142, #1165).
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_ppc_64();
	globalABI64_->index = &machRegIndex_ppc_64();

#endif

//#warning "This is not verified yet!"
#if defined(DYNINST_CODEGEN_ARCH_AARCH64)
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_aarch64();
	globalABI64_->index = &machRegIndex_aarch64();
#endif

#if defined(DYNINST_CODEGEN_ARCH_RISCV64)
	globalABI64_->addr_width = 8;
	globalABI_->index = &machRegIndex_riscv64();
	globalABI64_->index = &machRegIndex_riscv64();
#endif

// We _only_ support instrumenting 32-bit binaries on 64-bit systems
#if !defined(DYNINST_CODEGEN_ARCH_64BIT) || defined(cap_32_64)
	initialize32();
#endif

#if defined(DYNINST_CODEGEN_ARCH_64BIT)
	initialize64();
#endif
    }
    return (addr_width == 4) ? globalABI_ : globalABI64_;
}


const bitArray &ABI::getCallReadRegisters() const {
    if (addr_width == 4)
        return *callRead_;
    else if (addr_width == 8)
        return *callRead64_;
    else {
        assert(0);
        return *callRead_;
    }
}
const bitArray &ABI::getCallWrittenRegisters() const {
    if (addr_width == 4)
        return *callWritten_;
    else if (addr_width == 8)
        return *callWritten64_;
    else {
        assert(0);
        return *callWritten_;
    }
}

const bitArray &ABI::getReturnReadRegisters() const {
    if (addr_width == 4)
        return *returnRead_;
    else if (addr_width == 8)
        return *returnRead64_;
    else {
        assert(0);
        return *returnRead_;
    }
}

const bitArray &ABI::getReturnRegisters() const {
    if (addr_width == 4)
        return *returnRegs_;
    else if (addr_width == 8)
        return *returnRegs64_;
    else {
        assert(0);
        return *returnRegs_;
    }
}

const bitArray &ABI::getParameterRegisters() const {
    if (addr_width == 4)
        return *callParam_;
    else if (addr_width == 8)
        return *callParam64_;
    else {
        assert(0);
        return *callParam_;
    }
}

const bitArray &ABI::getSyscallReadRegisters() const {
    if (addr_width == 4)
        return *syscallRead_;
    else if (addr_width == 8)
        return *syscallRead64_;
    else {
        assert(0);
        return *syscallRead_;
    }
}
const bitArray &ABI::getSyscallWrittenRegisters() const {
    if (addr_width == 4)
        return *syscallWritten_;
    else if (addr_width == 8)
        return *syscallWritten64_;
    else {
        assert(0);
        return *syscallWritten_;
    }
}

const bitArray &ABI::getAllRegs() const
{
   if (addr_width == 4)
      return *allRegs_;
   else if (addr_width == 8)
      return *allRegs64_;
   else {
      assert(0);
      return *allRegs_;
   }
}

bitArray ABI::getBitArray()  {
  // A fresh all-zero liveness vector sized to match this architecture's
  // register bitArrays. Use the width already established for allRegs (built
  // once per arch in ABIBridge, sized to max-index-value + 1): index maps are
  // sparse -- the x86/i386 map leaves gaps for the 64-bit-only vector
  // registers so xmm/ymm/zmm alias arithmetic matches x86_64 -- so the entry
  // count (92 on i386) is NOT the right width (max index 163). This width must
  // equal that of the getXxxRegisters() sets, since calcRWSets combines them
  // with operator|= (whose operands must be equal-sized); an earlier
  // index->size() here both drove getIndex() writes out of bounds and produced
  // |= size mismatches, aborting liveness on 32-bit targets.
  return bitArray(getAllRegs().size());
}
#if defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)

// The x86/x86_64 ABI register sets are now produced by the architecture-
// independent common ABI (Dyninst::ABI) and bridged to bitArrays in
// ABIBridge.C. See buildABIBitArrays for the register-set semantics.
void ABI::initialize32(){
  DataflowAPI::buildABIBitArrays(Arch_x86, machRegIndex_x86(),
                  returnRegs_, callParam_, returnRead_, callRead_,
                  callWritten_, syscallRead_, syscallWritten_, allRegs_);
}

void ABI::initialize64(){
  DataflowAPI::buildABIBitArrays(Arch_x86_64, machRegIndex_x86_64(),
                  returnRegs64_, callParam64_, returnRead64_, callRead64_,
                  callWritten64_, syscallRead64_, syscallWritten64_, allRegs64_);
}
#endif

#if defined(DYNINST_CODEGEN_ARCH_POWER)
// The ppc64 ABI register sets are now produced by the architecture-independent
// common ABI (Dyninst::ABI) and bridged to bitArrays in ABIBridge.C. See
// buildABIBitArrays for the register-set semantics.
void ABI::initialize64(){
  DataflowAPI::buildABIBitArrays(Arch_ppc64, machRegIndex_ppc_64(),
                  returnRegs64_, callParam64_, returnRead64_, callRead64_,
                  callWritten64_, syscallRead64_, syscallWritten64_, allRegs64_);
}
#endif

#if defined(DYNINST_CODEGEN_ARCH_AARCH64)
// The aarch64 ABI register sets are now produced by the architecture-independent
// common ABI (Dyninst::ABI) and bridged to bitArrays in ABIBridge.C. See
// buildABIBitArrays for the register-set semantics.
void ABI::initialize64(){
  DataflowAPI::buildABIBitArrays(Arch_aarch64, machRegIndex_aarch64(),
                  returnRegs64_, callParam64_, returnRead64_, callRead64_,
                  callWritten64_, syscallRead64_, syscallWritten64_, allRegs64_);
}
#endif

//#warning "This is not verified!"
#if defined(DYNINST_CODEGEN_ARCH_RISCV64)
void ABI::initialize64(){
    RegisterMap riscv64Map = machRegIndex_riscv64();
	int sz = riscv64Map.size();

	returnRegs64_ = getBitArray(sz);
    (*returnRegs64_)[riscv64Map[riscv64::a0]] = true;
    (*returnRegs64_)[riscv64Map[riscv64::a1]] = true;

	returnRead64_ = getBitArray(sz);
    (*returnRegs64_)[riscv64Map[riscv64::a0]] = true;
    (*returnRegs64_)[riscv64Map[riscv64::a1]] = true;
    //Callee-saved registers
    (*returnRead64_)[riscv64Map[riscv64::s0]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s1]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s2]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s3]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s4]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s5]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s6]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s7]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s8]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s9]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s10]] = true;
    (*returnRead64_)[riscv64Map[riscv64::s11]] = true;
    (*returnRead64_)[riscv64Map[riscv64::sp]] = true;
    (*returnRead64_)[riscv64Map[riscv64::gp]] = true;
    (*returnRead64_)[riscv64Map[riscv64::tp]] = true;

    callParam64_ = getBitArray(sz);
    (*callParam64_)[riscv64Map[riscv64::a0]] = true;
    (*callParam64_)[riscv64Map[riscv64::a1]] = true;
    (*callParam64_)[riscv64Map[riscv64::a2]] = true;
    (*callParam64_)[riscv64Map[riscv64::a3]] = true;
    (*callParam64_)[riscv64Map[riscv64::a4]] = true;
    (*callParam64_)[riscv64Map[riscv64::a5]] = true;
    (*callParam64_)[riscv64Map[riscv64::a6]] = true;
    (*callParam64_)[riscv64Map[riscv64::a7]] = true;

	callRead64_ = getBitArray(sz);
	(*callRead64_)[riscv64Map[riscv64::a0]] = true;
	(*callRead64_)[riscv64Map[riscv64::a1]] = true;
	(*callRead64_)[riscv64Map[riscv64::a2]] = true;
	(*callRead64_)[riscv64Map[riscv64::a3]] = true;
	(*callRead64_)[riscv64Map[riscv64::a4]] = true;
	(*callRead64_)[riscv64Map[riscv64::a5]] = true;
	(*callRead64_)[riscv64Map[riscv64::a6]] = true;
	(*callRead64_)[riscv64Map[riscv64::a7]] = true;

	callWritten64_ = callRead64_;
	//First, GPRs...
	(*callWritten64_)[riscv64Map[riscv64::t0]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t1]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t2]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t3]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t4]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t5]] = true;
	(*callWritten64_)[riscv64Map[riscv64::t6]] = true;

	syscallRead64_ = &getBitArray(sz)->set();
	syscallWritten64_ = &getBitArray(sz)->set();

	allRegs64_ = &getBitArray(sz)->set();
}
#endif

void ABI::initialize64(Architecture arch){
    int sz;
    switch(arch){
        case Arch_amdgpu_gfx908:{
            RegisterMap amdgpu_gfx908_map = machRegIndex_amdgpu_gfx908();
            sz = amdgpu_gfx908_map.size();
            break;
        }
        case Arch_amdgpu_gfx90a:{
            RegisterMap amdgpu_gfx90a_map = machRegIndex_amdgpu_gfx90a();
            sz = amdgpu_gfx90a_map.size();
            break;
        }
        case Arch_amdgpu_gfx940:{
            RegisterMap amdgpu_gfx940_map = machRegIndex_amdgpu_gfx940();
            sz = amdgpu_gfx940_map.size();
            break;
        }
        case Arch_amdgpu_gfx950:{
            RegisterMap amdgpu_gfx950_map = machRegIndex_amdgpu_gfx950();
            sz = amdgpu_gfx950_map.size();
            break;
        }
        default:
        assert(0 && "This call is currently implemented for AMDGPU gfx908, gfx90a, gfx940 and gfx950 only");
    }
    returnRegs64_ = getBitArray(sz);
    returnRead64_ = getBitArray(sz);
    callParam64_ = getBitArray(sz);
    callRead64_ = getBitArray(sz);
    callWritten64_ = callRead64_;
    syscallRead64_ = &getBitArray(sz)->set();
    syscallWritten64_ = &getBitArray(sz)->set();
    allRegs64_ = &getBitArray(sz)->set();
}
