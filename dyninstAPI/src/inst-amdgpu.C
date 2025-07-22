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

#include "common/src/headers.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/inst-amdgpu.h"
#include "common/src/arch-amdgpu.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#include "parseAPI/h/CFG.h"

#include "emitter.h"
#include "emit-amdgpu.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

Address getMaxBranch() {
    return MAX_BRANCH_OFFSET;
}

std::unordered_map<std::string, unsigned> funcFrequencyTable;

void initDefaultPointFrequencyTable() {
    assert(0); //Not implemented
}

/************************************* Register Space **************************************/

void registerSpace::initialize32() {
      static bool done = false;
    if (done)
        return;

    std::vector < registerSlot * > registers;

    // TODO: This initialization doesn't consider all registers. Right now only the regs we typically use for instrumentation are considered. This might need work later on.

    //SGPRs
    for (unsigned idx = sgpr0; idx <= sgpr101; idx++) {
        char name[32];
        sprintf(name, "sgpr%u", idx - sgpr0);
        registers.push_back(new registerSlot(idx,
                                             name,
                                             /*offLimits =*/false,
                                             registerSlot::liveAlways,
                                             registerSlot::SGPR));
    }

    //VGPRs
    for (unsigned idx = v0; idx <= v255; idx++) {
        char name[32];
        sprintf(name, "vgpr%u", idx - v0);
        registers.push_back(new registerSlot(idx,
                                             name,
                                             /*offLimits =*/false,
                                             registerSlot::liveAlways,
                                             registerSlot::VGPR));
    }

    //SPRs
    registers.push_back(new registerSlot(flat_scratch_lo, "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(flat_scratch_lo, "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(xnack_mask_lo, "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(xnack_mask_lo, "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(vcc_lo, "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(vcc_lo, "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(exec_lo, "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(exec_lo, "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR));

    registerSpace::createRegisterSpace64(registers);
    done = true;
}

void registerSpace::initialize64() {
  assert(!"No 64-bit implementation for AMDGPU architecture!");
}

void registerSpace::initialize() {
    initialize32();
}

/*
 * Emit code to push down the stack
 */
void pushStack(codeGen & /*gen */)
{
}

void popStack(codeGen & /*gen*/)
{
}

/*********************************** Base Tramp ***********************************************/
bool baseTramp::generateSaves(codeGen & /* gen */, registerSpace *)
{
    return true;
}

bool baseTramp::generateRestores(codeGen & /* gen */, registerSpace *)
{
    return true;
}

/***********************************************************************************************/
/***********************************************************************************************/

//TODO: ALL THESE MUST GO AWAY ENTIRELY AS CODEGEN MATURES
void emitImm(opCode /* op */, Register /* src1 */, RegValue /* src2imm */, Register /* dest */,
        codeGen & /* gen */, bool /*noCost*/, registerSpace * /* rs */, bool /* s */)
{
  assert(0);
}

void cleanUpAndExit(int status);

Register emitFuncCall(opCode, codeGen &, std::vector <AstNodePtr> &, bool, Address) {
    assert(0);
    return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      std::vector <AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
    return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}

codeBufIndex_t emitA(opCode /* op */, Register /* src1 */, Register, long /* dest */,
        codeGen & /* gen */, RegControl /* rc */, bool)
{
    return 0;
}

Register emitR(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, bool /*noCost*/,
               const instPoint *, bool /*for_MT*/)
{
    registerSlot *regSlot = NULL;
    assert(regSlot);
    Register reg = regSlot->number;
    return reg;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &) {
    assert(0); //Not implemented
    // Not needed for memory instrumentation, otherwise TBD
}


// Yuhan(02/04/19): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP * /* as */, Register /* dest */, int /* stackShift */,
                codeGen & /* gen */,
                bool) {
    assert(0); // Not implemented
}

void emitCSload(const BPatch_addrSpec_NP *, Register, codeGen &,
                bool) {
    assert(0); // Not implemented
}

void emitVload(opCode /* op */, Address /* src1 */, Register /* src2 */, Register /* dest */,
               codeGen & /* gen */, bool /*noCost*/,
               registerSpace * /*rs*/, int /* size */,
               const instPoint * /* location */, AddressSpace *)
{

    assert(0); // Not implemented
}

void emitVstore(opCode /* op */, Register /* src1 */, Register /*src2*/, Address /* dest */,
        codeGen & /* gen */, bool,
        registerSpace * /* rs */, int /* size */,
        const instPoint * /* location */, AddressSpace *)
{
    assert(0); // Not implemented
}

void emitV(opCode /* op */, Register /* src1 */, Register /* src2 */, Register /* dest */,
        codeGen & /* gen */, bool /*noCost*/,
           registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, AddressSpace * /* proc */, bool /* s */)
{

    assert(0); // Not implemented
}

//
// I don't know how to compute cycles for AMDGPU instructions due to
//   multiple functional units.  However, we can compute the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode) {
    assert(0); //Not implemented
    return 0;
}


// This is used for checking wether immediate value should be encoded
// into a instruction. In fact, only being used for loading constant
// value into a register, and in ARMv8 there are 16 bits for immediate
// values in the instruction MOV.
// value here is never a negative value since constant values are saved
// as void* in the AST operand.
bool doNotOverflow(int64_t value)
{
    if ((value >= 0) && (value <= 0xFFFF)) return true;
    else return false;
}


// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee
// function is returned in "target_pdf", else it returns false.
bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &entry,
                             func_instance *&target_pdf, Address base_addr)
{
	if (isTerminated()) return false;

	// if the relocationEntry has not been bound yet, then the value
	// at rel_addr is the address of the instruction immediately following
	// the first instruction in the PLT entry (which is at the target_addr)
	// The PLT entries are never modified, instead they use an indirrect
	// jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the
	// function symbol is bound by the runtime linker, it changes the address
	// in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

	Address got_entry = entry.rel_addr() + base_addr;
	Address bound_addr = 0;
	if (!readDataSpace((const void*)got_entry, sizeof(Address),
				&bound_addr, true)){
		sprintf(errorLine, "read error in PCProcess::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,getPid());
		logLine(errorLine);
		//print_read_error_info(entry, target_pdf, base_addr);
		fprintf(stderr, "%s[%d]: %s\n", FILE__, __LINE__, errorLine);
		return false;
	}

   //fprintf(stderr, "%s[%d]:  hasBeenBound:  %p ?= %p ?\n", FILE__, __LINE__, bound_addr, entry.target_addr() + 6 + base_addr);
	if ( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
	  // the callee function has been bound by the runtime linker
	  // find the function and return it
	  target_pdf = findFuncByEntry(bound_addr);
	  if(!target_pdf){
	    return false;
	  }
	  return true;
	}
	return false;
}

bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &, Address,
                             func_instance *, Address) {
    assert(0); //Not implemented
    assert(0 && "TODO!");
    return false;
}

void emitLoadPreviousStackFrameRegister(Address /* register_num */,
                                        Register /* dest */,
                                        codeGen & /* gen */,
                                        int /* size */,
                                        bool)
{
  assert(0); // Not imeplemented
}

void emitStorePreviousStackFrameRegister(Address,
                                         Register,
                                         codeGen &,
                                         int,
                                         bool) {
    assert(0);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction /* i */,
					  Address /* addr */,
					  std::vector<AstNodePtr> & /* args */)
{
    return true;
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
    Address val_to_write = f->addr();
    return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);
    return false;
}

Emitter *AddressSpace::getEmitter() {
    static EmitterAmdgpuVega vegaEmitter;
    return &vegaEmitter;
}

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

/*
bool image::updatePltFunc(parse_func *caller_func, Address stub_addr)
{
	assert(0); //Not implemented
    return true;
}
*/

Address Emitter::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;

    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
        case 4: jump_slot_size = 4; break;
        case 8: jump_slot_size = 8; break;
        default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !var) {
        assert(!"Invalid variable load (variable info is missing)");
    }

    // find the Symbol corresponding to the int_variable
    std::vector<SymtabAPI::Symbol *> syms;
    var->svar()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, var->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[8] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
}

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
    // from POWER64 getInterModuleFuncAddr

    AddressSpace *addrSpace = gen.addrSpace();
    if (!addrSpace)
        assert(0 && "No AddressSpace associated with codeGen object");

    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    
    unsigned int jump_slot_size;
    switch (addrSpace->getAddressWidth()) {
    case 4: jump_slot_size =  4; break; // l: not needed
    case 8: 
      jump_slot_size = 24;
      break;
    default: assert(0 && "Encountered unknown address width");
    }

    if (!binEdit || !func) {
        assert(!"Invalid function call (function info is missing)");
    }

    // find the Symbol corresponding to the func_instance
    std::vector<SymtabAPI::Symbol *> syms;
    func->ifunc()->func()->getSymbols(syms);

    if (syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s"
                , __FILE__, __LINE__, func->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for (unsigned k=0; k<syms.size(); k++) {
        if (syms[k]->isInDynSymtab()) {
            referring = syms[k];
            break;
        }
    }
    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if (!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[24] = {0};
        binEdit->writeDataSpace((void*)relocation_address, jump_slot_size, dat);
        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
    }
    return relocation_address;
}
