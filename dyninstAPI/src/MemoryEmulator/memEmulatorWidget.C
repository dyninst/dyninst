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


#include "dyninstAPI/src/Relocation/Widgets/Widget.h"
#include "dyninstAPI/src/Relocation/CFG/RelocTarget.h"
#include "dyninstAPI/src/Relocation/Widgets/CFWidget.h" // CFPatch

// For our horribly horked memory effective address system
// Which I'm not fixing here. 
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/h/BPatch_addressSpace.h" // bpatch_address... you get the picture
#include "dyninstAPI/h/BPatch_point.h"
// Memory hackitude
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"

#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/Relocation/CodeBuffer.h"
#include "common/src/arch-x86.h"

#include "memEmulatorWidget.h"

#include "dyninstAPI/src/RegisterConversion.h"

#include "boost/tuple/tuple.hpp"
#include "memEmulator.h"


using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


MemEmulator::Ptr MemEmulator::create(Instruction::Ptr insn,
				     Address addr,
				     instPoint *point) {
  MemEmulator::Ptr ptr = MemEmulator::Ptr(new MemEmulator(insn, addr, point));
  return ptr;
}

const int ESI_SHIFT_REG = REGNUM_EBP;
const int EDI_SHIFT_REG = REGNUM_EBX;

bool MemEmulator::initialize(const codeGen &templ, const RelocBlock *t) {
	// Number chosen arbitrarily
	scratch.allocate(128);
	scratch.applyTemplate(templ);

	block = t->block();

	effAddr = Null_Register;
	stackOffset = 0;
	effAddrSaveOffset = 0;
	usesESI = false;
	usesEDI = false;
	debug = false;

	return true;
}

/* Block comment ho!
 * 
 *
 * We're trying to "emulate" the memory operation. That is, we want to redirect it
 * to (or from) a shadow of original memory. In general, this is simple, using the following
 * code fragment:
 * 
 * <determine safe register to use for effective address> -- EAX if it isn't read by the instruction; else ECX; else EDX. 
 * <shift stack down> -- so we don't stomp on any data the program stores under the stack.
 * <save caller-save registers> -- eax, ecx, edx; we save all three since we don't trust liveness.
 * <save flags> -- see the above comment
 * <calculate original effective address into EffAddr> -- LEA so we don't harm registers
 * <push translation arguments> -- EAX, and debugging arguments
 * <call translator> -- leaves new pointer in EAX
 * <move TransEffAddr to EffAddr> 
 * <restore flags>
 * <restore all registers except effaddr> 
 * <shift stack up>
 * <perform memory operation> 
 * <restore effaddr from under the stack> -- via a mov instruction
 * 
 * Now, that's the overview. But this is x86, and things get tricky. We handle the following special cases:
 * 1) Push/pop
 * 2) Write to (ESP)
 * 3) a1-a3 implicit use of EAX
 * 4) scas/ins/outs/cmps/stos/movs/lods implicit use of ESI/EDI
 * 
 * 1) Push/pop. I believe we can structure the common code to leave ESP untouched at the memory operation time, 
 *    which means this case is trivially solved. However, we need to be sure to restore effAddr from the right place.
 * 2) Write to (ESP). See case 1)
 * 3) a1-a3 implicit use of EAX. These are of the form mov eax, [mem] or mov [mem], eax; thus, we _skip_ all of
 *    the above complexity and just statically translate [mem].
 * 4) Implicit use of ESI/EDI (either or both). First, we need to keep two effective address registers, and we 
 *    need to call translate twice (possibly). Second, these operations modify ESI/EDI, and we want to emulate
 *    that modification as well. So we do the following:
 *    1) shiftESI := translate_shift(ESI) - return is the amount that esi needs to be _shifted_ to be translated.
 *    2) shiftEDI := translate_shift(EDI)
 *    3) ESI += shiftESI
 *    4) EDI += shiftEDI
 *    5) Emulate instruction
 *    6) ESI -= shiftESI (note: we can't use an LEA for this, thus we sub and have to save flags)
 *    7) EDI -= shiftEDI (see above) 
 */

bool MemEmulator::generate(const codeGen &templ,
                           const RelocBlock *t,
                           CodeBuffer &buffer) 
{
	if (!initialize(templ, t)) return false;

	if (generateViaOverride(buffer))
		return true;
	if (generateViaModRM(buffer))
		return true;

	cerr << "Error: failed to emulate memory operation @ " << hex << addr() << dec << ", " << insn()->format() << endl;
	unsigned char *tmp = (unsigned char *)insn()->ptr();
	cerr << hex << "\t raw: ";
	for (unsigned i = 0; i < insn()->size(); ++i) {
		cerr << tmp[i];
	}
	cerr << dec << endl;
	assert(0);
	return false;
}

bool MemEmulator::generateViaOverride(CodeBuffer &buffer) 
{
    // Watch for a0/a1/a2/a3 moves 
    unsigned char *buf = (unsigned char *)insn_->ptr();
    if ((unsigned char) 0xa0 <= buf[0] &&
        buf[0] <= (unsigned char) 0xa3) {
			if (!generateEAXMove(buf[0], buffer)) 
				assert(0);
			return true;
    }
                                          
   const InstructionAPI::Operation &op = insn_->getOperation();
   switch(op.getID()) {
      case e_scasb:
      case e_scasd:
      case e_scasw:
      case e_lodsb:
      case e_lodsd:
      case e_lodsw:
      case e_stosb:
      case e_stosd:
      case e_stosw:
      case e_movsb:
      case e_movsd:
      case e_movsw:
      case e_cmpsb:
      case e_cmpsd:
      case e_cmpsw:
      case e_insb:
      case e_insd:
      case e_insw:
      case e_outsb:
      case e_outsd:
      case e_outsw:
		  if (!generateESI_EDI(buffer)) {
			 assert(0);
		 }
		 return true;
         break;
      case e_popad:
        if (!generatePOPAD(buffer)) {
            assert(0);
        }
        return true;
        break;
      default:
         break;
   }
   return false;
}

bool MemEmulator::generateEAXMove(unsigned char opcode, 
                                  CodeBuffer &buffer) 
{
    // mov [offset], eax
    // We hates them.

    Address origTarget;
    switch(opcode) {
    case 0xa0:
    case 0xa1: {
        // read from memory
        std::set<Expression::Ptr> reads;
        insn_->getMemoryReadOperands(reads);
        assert(reads.size() == 1);
        Result res = (*(reads.begin()))->eval();
        assert(res.defined);
        origTarget = res.convert<Address>();
        break;
    }
    case 0xa2:
    case 0xa3: {
        // write
        std::set<Expression::Ptr> writes;
        insn_->getMemoryWriteOperands(writes);
        assert(writes.size() == 1);
        Result res = (*(writes.begin()))->eval();
        assert(res.defined);
        origTarget = res.convert<Address>();
        break;
    }
    default:
        assert(0);
        break;
    }
    // Map it to the new location
    bool valid; Address target;
    boost::tie(valid, target) = scratch.addrSpace()->getMemEm()->translate(origTarget);
    if (!valid) target = origTarget;
    //cerr << "Handling mov EAX, [offset]: opcode " << hex << opcode << ", orig dest " << origTarget << " and translated " << target << dec << endl;
    // And emit the insn
    assert(insn_->size() == 5);
    GET_PTR(buf, scratch);
    *buf = (char) opcode; buf++;
    int *tmp = (int *)buf;
    *tmp = target; tmp++;
    buf = (codeBuf_t *)tmp;
    SET_PTR(buf, scratch);
    buffer.addPIC(scratch, tracker());
    return true;
}

bool MemEmulator::generateViaModRM(CodeBuffer &buffer) {
   // We need a BPatch_something to do the memory handling. If that's 
   // not present, assume we don't need to emulate this piece of
   // memory.
   if (!scratch.addrSpace()->up_ptr()) {
      buffer.addPIC(insn_->ptr(), insn_->size(), tracker());
	  assert(0);
      return true;
   }

   insertDebugMarker();

   // Choose a register to hold the effective address
   if (!generateModRMInitialize()) return false;
   insertDebugMarker();
   // Shift the stack, save registers, calculate original effective address
   if (!generateTranslatorSetup()) return false;
   copyScratchToCodeBuffer(buffer);

   // Make the call to the translation function
   if (!generateTranslatorCall(buffer)) return false;
   // Teardown the call saves
   insertDebugMarker();
   if (!generateTranslatorTeardown()) return false;
   insertDebugMarker();

   copyScratchToCodeBuffer(buffer);

   return true;
}

void MemEmulator::insertDebugMarker() {
	if (debug || (dyn_debug_trap /*&& addr() > 0xab000 && addr() <0xad000*/)) 
		scratch.fill(1, codeGen::cgTrap);
}

bool MemEmulator::generateModRMInitialize() {
	// Placeholder for good code design...

	return true;
}

bool MemEmulator::generateTranslatorSetup() {
	// Our job:
	// Shift the stack down
	// Decide who will hold the effective address
	// Save caller-save registers
	// Calculate the original effective address
	// Save flags 

	if (!shiftStack()) return false;
	if (!determineEffAddr()) return false;
	if (!saveRegisters()) return false;
	if (!calculateEffAddr()) return false;
	if (!saveFlags()) return false;

	return true;
}

bool MemEmulator::generateTranslatorCall(CodeBuffer &buffer) {

	// This is easy. We don't know where we are, 
	// so we have to use the patch mechanic, but that's
	// not too bad. 
	// We also set up the arguments in the patch code so that we
	// have access to the current address (for debugging purposes)

	Address target = getTranslatorAddr(false);
	if (!target) return false;
	buffer.addPatch(new MemEmulatorPatch(effAddr, effAddr, addr_, target), tracker());
	return true;
}

bool MemEmulator::generateTranslatorTeardown() {
	// Restore flags
	// Restore registers (except effAddr)
	// Shift stack back up
	// Emulate memory operation
	// Restore effAddr
	insertDebugMarker();
	if (!restoreFlags()) return false;
	insertDebugMarker();
	if (!restoreRegisters()) return false;
	insertDebugMarker();
	if (!restoreStack()) return false;
	insertDebugMarker();
	if (!emulateOriginalInstruction()) return false;
	if (!restoreEffectiveAddr()) return false;

	return true;
}

bool MemEmulator::shiftStack() {
    ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, -1*MemoryEmulator::STACK_SHIFT_VAL, RealRegister(REGNUM_ESP), scratch);
	stackOffset -= MemoryEmulator::STACK_SHIFT_VAL;
	return true;
}

bool MemEmulator::determineEffAddr() {
	// We want to use whichever register isn't used by the instruction.
	// Technically, _written_ by the instruction. As that would be silly.

	// Theory: we can always use either ECX or EDX, as they're getting saved
	// anyway. 

	// We can use a register if it's only used for a memory operand. We cannot use
	// it if it's directly used, either mov [ptr], reg or mov reg, [ptr].
	// Technically, we could do mov reg, [ptr] so long as we don't restore reg. 
	// I'm not too worried, though. 

	static RegisterAST::Ptr ecx = RegisterAST::Ptr(new RegisterAST(x86::ecx));
	static RegisterAST::Ptr edx = RegisterAST::Ptr(new RegisterAST(x86::edx));

	bool useECX = true;
	bool useEDX = true;
	std::vector<InstructionAPI::Operand> operands;
	insn_->getOperands(operands);
	for (unsigned i = 0; i < operands.size(); ++i) {
		if (operands[i].readsMemory()) continue;
		if (operands[i].writesMemory()) continue;
		if (operands[i].isRead(ecx) || operands[i].isWritten(ecx)) useECX = false;
		if (operands[i].isRead(edx) || operands[i].isWritten(edx)) useEDX = false;
	}

	if (useECX) {
		effAddr = REGNUM_ECX;
	}
	if (useEDX) {
		effAddr = REGNUM_EDX;
	}

        block->obj()->addEmulInsn(addr(),effAddr);
    
    return (useECX || useEDX);
}

bool MemEmulator::saveRegisters() {
	// Theoretically we could use liveness to determine
	// whether to save a register or not. 
	// We want to save the effAddr register _first_ to make
	// our lives easier.
	if (effAddr == REGNUM_ECX) {
		push(REGNUM_ECX);
		effAddrSaveOffset = stackOffset;
		push(REGNUM_EDX);
	}
	else {
		push(REGNUM_EDX);
		effAddrSaveOffset = stackOffset;
		push(REGNUM_ECX);
	}
	push(REGNUM_EAX);

	return true;
}


bool MemEmulator::calculateEffAddr() {
	// Luckily, we have code to do this already.
	assert(scratch.addrSpace());
	BPatch_addressSpace *bproc = (BPatch_addressSpace *)scratch.addrSpace()->up_ptr();
	assert(bproc);

	assert(point_);
	BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, point_, BPatch_locInstruction);
	if (bpoint == NULL) {
		fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
			point_, addr_);
		return false;
	}
	const BPatch_memoryAccess *ma = bpoint->getMemoryAccess();

	const BPatch_addrSpec_NP *start = ma->getStartAddr(0); // Guessing on 0, here...

	// Now that we've done all the background work, we can emit an LEA to grab the
	// effective address.
	// The stackOffset parameter is a magic "if you used ESP, be sure to take into
	// account the fact we movled it". 
	emitASload(start, effAddr, stackOffset, scratch, true);

	return true;
}

bool MemEmulator::saveFlags() {
	// nice thing is, eax is already saved at this point. So our live, it be easy.
	emitSimpleInsn(0x9f, scratch);
	emitSaveO(scratch);

	push(REGNUM_EAX);
	return true;
}

bool MemEmulator::restoreFlags() {
	pop(REGNUM_EAX);

	emitRestoreO(scratch);
	emitSimpleInsn(0x9E, scratch);

	return true;
}

bool MemEmulator::restoreRegisters() {
	// Order needs to match that in saveRegisters
	// Also, don't restore over effAddr. Yet. 
	pop(REGNUM_EAX);
	if (effAddr == REGNUM_ECX)
		pop(REGNUM_EDX);
	else 
		pop(REGNUM_ECX);
	return true;
}

bool MemEmulator::restoreStack() {
	// We track stack depth in the stackOffset
	// parameter...
	::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, -1*stackOffset, RealRegister(REGNUM_ESP), scratch);
	stackOffset = 0;
	return true;
}

using namespace NS_x86;

bool MemEmulator::emulateOriginalInstruction() {
   NS_x86::instruction ugly_insn(insn_->ptr());

   if (!insnCodeGen::generateMem(scratch,
                                 ugly_insn,
                                 0, // ignored
                                 0, // ignored
                                 effAddr,
                                 Null_Register))
      return false;

	// Do we mess with the stack pointer?
	if (insn_->getOperation().getID() == e_push) {
		stackOffset -= 4;
	}
	if (insn_->getOperation().getID() == e_pop) {
		stackOffset += 4;
	}

	return true;
}

bool MemEmulator::restoreEffectiveAddr() {
	// We want to pull it out of wherever it is on the stack
	// We could LEA esp down, pop, and back up. But really. 
	// Instead, use a mov. 
	int restoreOffset = (effAddrSaveOffset - stackOffset);

	::emitMovRMToReg(RealRegister(effAddr), RealRegister(REGNUM_ESP), restoreOffset, scratch);
	return true;
}

bool MemEmulator::copyScratchToCodeBuffer(CodeBuffer &buffer) {
	// Copy it in and re-initialize the scratch buffer to hold more
	// goodies.
	if (scratch.used() == 0) return true;

	buffer.addPIC(scratch, tracker());

	scratch.allocate(128);
	return true;
}

TrackerElement *MemEmulator::tracker() const {
   EmulatorTracker *e = new EmulatorTracker(addr_, block, point_->func());
  return e;
}

bool MemEmulator::push(Register reg) {
	::emitPush(RealRegister(reg), scratch);
	stackOffset -= 4;
	return true;
}

bool MemEmulator::pop(Register reg) {
	::emitPop(RealRegister(reg), scratch);
	stackOffset += 4;
	return true;
}

// wrap whole thing in check that esp is outside of the stack segment? 
bool MemEmulator::generatePOPAD(CodeBuffer &buffer) {
   point_->func()->proc()->getMemEm()->addPOPAD(addr());
	scratch.fill(1, codeGen::cgTrap);
   buffer.addPIC(scratch, tracker());
	return true;
}

/* Block quote redux!
 *
 * We need to:
 * 1) Determine if we use ESI, EDI, or both;
 * 2) Shift the stack
 * 3) Retain a register (ECX) for the "shift" involved in translating ESI
 *    (that is, tESI = ESI + ECX)
 * 4) Do the same for EDI/EDX
 * 5) Save caller-saved registers
 * 6) Save the flags
 * 7) If using ESI, ECX := shift(ESI)
 * 8) If using ESI, ESI += ECX
 * 9) If using EDI, EDX := shift(EDI)
 * 10) If using EDI, EDI += EDX
 * 11) Restore flags and EAX. The "EAX" is very important.
 * 12) Run the instruction
 * 13) Save EAX and flags (again)
 * 14) If using ESI, ESI -= ECX
 * 15) If using EDI, EDI -= EDX
 * 16) Restore EAX and flags
 * 17) Restore the stack.
 */

bool MemEmulator::generateESI_EDI(CodeBuffer &buffer) {
	if (!determineESI_EDIUse()) return false;

	insertDebugMarker();

	if (!shiftStack()) return false;

	if (!saveRegistersESI_EDI()) return false;

	if (!saveFlags()) return false;

	copyScratchToCodeBuffer(buffer);

	if (usesESI && !generateESIShift(buffer)) return false;
	if (usesEDI && !generateEDIShift(buffer)) return false;

	if (!saveShiftsAndRestoreRegs()) return false;

	insertDebugMarker();
	if (!emulateOriginalESI_EDI()) return false;
	if (!emulateESI_EDIValues()) return false;
	if (!restoreAllRegistersESI_EDI()) return false;
	if (!restoreStack()) return false;
	assert(stackOffset == 0);
	insertDebugMarker();

	copyScratchToCodeBuffer(buffer);
	
	return true;
}


bool MemEmulator::determineESI_EDIUse() {
   const InstructionAPI::Operation &op = insn_->getOperation();

   switch(op.getID()) {
      case e_insb:
      case e_insd:
      case e_insw:
		  usesEDI = true;
		  break;
      case e_movsb:
      case e_movsd:
      case e_movsw:
		  usesESI = true;
		  usesEDI = true;
		  break;
      case e_outsb:
      case e_outsd:
      case e_outsw:
		  usesESI = true;
		  break;
      case e_lodsb:
      case e_lodsd:
      case e_lodsw:
		  usesESI = true;
		  break;
      case e_stosb:
      case e_stosd:
      case e_stosw:
		  usesEDI = true;
		  break;
      case e_cmpsb:
      case e_cmpsw:
      case e_cmpsd:
		  usesESI = true;
		  usesEDI = true;
		  break;
	  case e_scasb:
      case e_scasd:
      case e_scasw:
		  usesEDI = true;
		  break;
      default:
         assert(0);
		 break;
   }
   return true;
}

bool MemEmulator::saveRegistersESI_EDI() {
	// Always save EAX, but save it last
	// so that it's easy to access.

	// We use EBX/EBP to hold shift values so that we can restore EA/C/DX pre-instruction

	push(REGNUM_EBX);
	push(REGNUM_EBP);

	push(REGNUM_EDX);
	push(REGNUM_ECX);
	push(REGNUM_EAX);
	return true;
}

bool MemEmulator::generateESIShift(CodeBuffer &buffer) {
	Address destination = getTranslatorAddr(true);
	if (!destination) return false;
	buffer.addPatch(new MemEmulatorPatch(REGNUM_ESI, ESI_SHIFT_REG, addr_, destination), tracker());

	// Also, set up ESI to have the right value
	::emitLEA(RealRegister(REGNUM_ESI), RealRegister(ESI_SHIFT_REG), 0, 0, RealRegister(REGNUM_ESI), scratch);
	copyScratchToCodeBuffer(buffer);

	return true;
}

bool MemEmulator::generateEDIShift(CodeBuffer &buffer) {
	Address destination = getTranslatorAddr(true);
	if (!destination) return false;
	buffer.addPatch(new MemEmulatorPatch(REGNUM_EDI, EDI_SHIFT_REG, addr_, destination), tracker());

	::emitLEA(RealRegister(REGNUM_EDI), RealRegister(EDI_SHIFT_REG), 0, 0, RealRegister(REGNUM_EDI), scratch);

	copyScratchToCodeBuffer(buffer);

	return true;
}

bool MemEmulator::saveShiftsAndRestoreRegs() {
	// Several instructions implicitly use EAX, and the REP prefix can use ECX. 

	// Currently the stack looks like this:
	// <safety gap>
	// EBX
	// EBP
	// EDX
	// ECX
	// EAX
	// Flags

	// We want it to look like this:
	// <safety gap>
	// EBX
	// EBP

	if (!restoreFlags()) return false;
	pop(REGNUM_EAX);
	pop(REGNUM_ECX);
	pop(REGNUM_EDX);

	return true;
}

bool MemEmulator::emulateOriginalESI_EDI() {
	scratch.copy(insn_->ptr(), insn_->size());
	return true;
}

bool MemEmulator::emulateESI_EDIValues() {
	// We need to:
	// EDI -= EDX
	// ESI -= ECX
	// And "subtract" means "save the flags"
	// ... which in turn means "save eax, fool"
	push(REGNUM_EAX);
	saveFlags();

	if (usesESI) ::emitSubRegReg(RealRegister(REGNUM_ESI), RealRegister(ESI_SHIFT_REG), scratch);
	if (usesEDI) ::emitSubRegReg(RealRegister(REGNUM_EDI), RealRegister(EDI_SHIFT_REG), scratch);

	restoreFlags();
	pop(REGNUM_EAX);
	return true;
}

bool MemEmulator::restoreAllRegistersESI_EDI() {

	pop(REGNUM_EBP);
	pop(REGNUM_EBX);
	return true;
}

string MemEmulator::format() const {
  stringstream ret;
  ret << "MemE(" << insn_->format()
      << "," << std::hex << addr_ << std::dec
      << ")";

  return ret.str();
}

Address MemEmulator::getTranslatorAddr(bool wantShift) {
   if (wantShift) {
      // Function lookup time
      func_instance *func = scratch.addrSpace()->findOnlyOneFunction("RTtranslateMemoryShift");
      // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
      if (!func) return 0;
      // assert(func);
      return func->addr();
   }
   else {
      // Function lookup time
      func_instance *func = scratch.addrSpace()->findOnlyOneFunction("RTtranslateMemory");
      // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
      if (!func) return 0;
      // assert(func);
      return func->addr();
   }
}

bool MemEmulatorPatch::apply(codeGen &gen,
                             CodeBuffer *) {
   relocation_cerr << "MemEmulatorPatch::apply @ " << hex << gen.currAddr() << dec << endl;
   relocation_cerr << "\tSource reg " << source_ << endl;
   assert(!gen.bt());

   // Two debugging assists
   ::emitPushImm(gen.currAddr(), gen);
   ::emitPushImm(orig_, gen);
	// And our argument
   ::emitPush(RealRegister(source_), gen);

   // Step 2: call the translator
   Address src = gen.currAddr() + 5;
   relocation_cerr << "\tCall " << hex << dest_ << ", offset " << dest_ - src << dec << endl;
   assert(dest_);
   emitCallRel32(dest_ - src, gen);
   if (target_ != REGNUM_EAX) 
   {
	   ::emitMovRegToReg(RealRegister(target_), RealRegister(REGNUM_EAX), gen);
   }
   ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 12, RealRegister(REGNUM_ESP), gen);

   return true;
}

