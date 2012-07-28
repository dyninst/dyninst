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

#if !defined(cap_mem_emulation)
#error
#endif


#if !defined (_R_E_MEM_EMULATOR_H_)
#define _R_E_MEM_EMULATOR_H_

#include "dyninstAPI/src/Relocation/Widgets/Widget.h"
#include "dyninstAPI/src/codegen.h"
#include <stack>
class registerSlot;


namespace Dyninst {
namespace Relocation {

class MemEmulatorTranslator;

class MemEmulator : public Widget {
  friend class MemEmulatorTranslator;
  typedef std::map<Register, RelocBlockPtr> TranslatorMap;
 public:
	 typedef boost::shared_ptr<MemEmulator> Ptr;
   
   static Ptr create(InstructionAPI::Instruction::Ptr insn,
		     Address addr,
		     instPoint *point);

   virtual bool generate(const codeGen &, const RelocBlock *, CodeBuffer &);

   virtual ~MemEmulator() {};
   virtual std::string format() const;

   virtual Address addr() const { return addr_; }
   virtual unsigned size() const { return insn_->size(); }
   virtual InstructionAPI::Instruction::Ptr insn() const { return insn_; }

 private:
   MemEmulator(InstructionAPI::Instruction::Ptr insn,
	   Address addr,
	   instPoint *point)
      : insn_(insn), 
      addr_(addr),
      point_(point)
      {};

   // Set up the codeGen structures we use to hold code. 
   bool initialize(const codeGen &templ, const RelocBlock *);

   // Handle a0-a3 implicit EAX uses, or ESI/EDI instructions
   bool generateViaOverride(CodeBuffer &buffer);

   // Handle generic MOD/RM using instructions
   bool generateViaModRM(CodeBuffer &buffer);

   // Handle a0-a3 implicit EAX moves
   bool generateEAXMove(unsigned char opcode, CodeBuffer &buffer);

   // Handle ESI/EDI instructions
   bool generateESI_EDI(CodeBuffer &buffer);

   // Handle POPAD instructions
   bool generatePOPAD(CodeBuffer &buffer);

   // Drop in a trap for later debugging assistance
   void insertDebugMarker();

   // Initialize mod/rm specific data
   bool generateModRMInitialize();
   // Create the pre-call handling for MOD/RM instructions
   bool generateTranslatorSetup();
   // Create the call to the translator function
   bool generateTranslatorCall(CodeBuffer &buffer);
   // Create the teardown code
   bool generateTranslatorTeardown();

   // Move the stack down by a known amount
   bool shiftStack();
   // Determine which register we can use for the effective address
   bool determineEffAddr();
   // Save eax/ecx/edx
   bool saveRegisters();
   // Run the effective address calculation before we modify anything (except the stack pointer, sigh)
   bool calculateEffAddr();
   // Save the flags
   bool saveFlags();

   // Restore flags after the call 
   bool restoreFlags();
   // Restore caller-saved registers (except effAddr)
   bool restoreRegisters();
   // Restore the stack
   bool restoreStack();
   // Emulate the original instruction using effAddr instead of the original expression
   bool emulateOriginalInstruction();
   // And restore the effective address register
   bool restoreEffectiveAddr();

   // ESI/EDI implicit shtuff
   bool determineESI_EDIUse();
   bool saveRegistersESI_EDI(); // Almost, but not quite the same as saveRegisters. Yuck. 
   bool generateESIShift(CodeBuffer &buffer);
   bool generateEDIShift(CodeBuffer &buffer);
   bool saveShiftsAndRestoreRegs();
   bool emulateOriginalESI_EDI();
   bool emulateESI_EDIValues();
   bool restoreAllRegistersESI_EDI();

   // Copy code into the CodeBuffer (and reset the scratch codegen)
   bool copyScratchToCodeBuffer(CodeBuffer &);
   TrackerElement *tracker() const;
   bool push(Register);
   bool pop(Register);
   Address getTranslatorAddr(bool wantShiftFunc);


   /// Members
   Register effAddr;
   int stackOffset;
   int effAddrSaveOffset;
   block_instance *block;

   bool usesESI;
   bool usesEDI;

   codeGen scratch;
   bool debug;

   InstructionAPI::Instruction::Ptr insn_;
   Address addr_;
   instPoint *point_;
};

struct MemEmulatorPatch : public Patch {
   // Put in a call to the RTtranslateMemory
   // function
   MemEmulatorPatch(Register s,
	                Register t,
					Address o,
                    Address d)
		: source_(s), target_(t), orig_(o), dest_(d) {};
   virtual bool apply(codeGen &gen, CodeBuffer *buf);
   virtual unsigned estimate(codeGen &) { return 7; };
   virtual ~MemEmulatorPatch() {};

   Register source_;
   Register target_;
   Address orig_;
   Address dest_;
};

};
};
#endif
