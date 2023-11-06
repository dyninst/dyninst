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

#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/emit-power.h"
#include "dyninstAPI/src/function.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <set>
#include <memory>
#include <fstream>
#include "common/src/arch-power.h"
#include <sstream>
#include <limits>

bool shouldAssertIfInLongBranch = true;
bool failedLongBranchLocal = false;
class TrackStacktraces {
public:
  std::ofstream _out;
  std::set<std::string> _stacks;
  TrackStacktraces() {
    _out.open("generatorStacks.txt", std::ofstream::out);
  }

  void Insert(std::string s) {
    if (_stacks.find(s) == _stacks.end())
      _stacks.insert(s);
  }
  ~TrackStacktraces() {
    for (auto i : _stacks)
      _out << i << std::endl;
    _out.close();
  }

};


std::shared_ptr<TrackStacktraces> _global_stack_track;



// "Casting" methods. We use a "base + offset" model, but often need to 
// turn that into "current instruction pointer".
codeBuf_t *insnCodeGen::insnPtr(codeGen &gen) {
    return (instructUnion *)gen.cur_ptr();
}

#if 0
// Same as above, but increment offset to point at the next insn.
codeBuf_t *insnCodeGen::ptrAndInc(codeGen &gen) {
  // MAKE SURE THAT ret WILL STAY VALID!
  gen.realloc(gen.used() + sizeof(instruction));

  instructUnion *ret = insnPtr(gen);
  gen.moveIndex(instruction::size());
  return ret;
}
#endif

void insnCodeGen::generate(codeGen &gen, instruction&insn) {
  // void *buffer[50];
  // char **strings;
  // int nptrs;
  // nptrs = backtrace(buffer, 50);
  // strings = backtrace_symbols(buffer, nptrs);
  // std::stringstream ss;
  // if (strings != NULL) {
  //   for (int i = 0; i < nptrs; i++) 
  //     ss << strings[i] << std::endl;
  // }

  /*
  AddressSpace *as = gen.addrSpace();
  bool isLittleEndian = true;
  if (as) {
    const std::vector<mapped_object*> objs = as->mappedObjects();
    if (objs.size() > 0) {
      mapped_object *mo = objs[0];
      SymtabAPI::Symtab* sym = mo->parse_img()->getObject(); 
      isLittleEndian = !sym->isBigEndianDataEncoding();
    } else {
      fprintf(stderr, "No mapped_object object\n");
    }
  } else {
    fprintf(stderr, "No AddressSpace object\n");
  }
  unsigned raw;
  if (isLittleEndian) {
    // Writing an instruction.  Convert byte order if necessary.
    raw = swapBytesIfNeeded(insn.asInt());
  } else {
    raw = insn.asInt();
  }
  */
  // if (_global_stack_track.get() == NULL)
  //   _global_stack_track.reset(new TrackStacktraces());
  // _global_stack_track->Insert(ss.str());
  // if (gen.currAddr() == 0xe5f88d4) {
  //   fprintf(stderr, "%s\n", "Hello!, whats next???? " );
  //   fprintf(stderr, "%08x\n", insn.asInt());
  // }
  unsigned raw = insn.asInt();
  if (gen.currAddr() == 0xe93e3fc){
    std::cerr << "Generating b e93e3fc, e93e88c" << std::endl;
    //assert(1==0);
  }
  // fprintf(stderr, "Instruction Written: %08x at position: %16x\n", insn.asInt(), gen.currAddr());
  //fprintf(stderr, "Raw Written value %u\n", raw);
  gen.copy(&raw, sizeof(unsigned));
}

void insnCodeGen::generateIllegal(codeGen &gen) { // instP.h
    instruction insn;
    generate(gen,insn);
}

void insnCodeGen::generateTrap(codeGen &gen) {
    instruction insn(BREAK_POINT_INSN);
    generate(gen,insn);
}

void insnCodeGen::generateBranch(codeGen &gen, long disp, bool link)
{
  //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    if (ABS(disp) > MAX_BRANCH) {
	// Too far to branch, and no proc to register trap.
	fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
           (unsigned long)ABS(disp), (unsigned long) MAX_BRANCH);
	bperr( "Error: attempted a branch of 0x%lx\n", (unsigned long)disp);
	logLine("a branch too far\n");
	showErrorCallback(52, "Internal error: branch too far");
	bperr( "Attempted to make a branch of offset 0x%lx\n", (unsigned long)disp);
	assert(0);
    }


    instruction insn;
    IFORM_OP_SET(insn, Bop);
    IFORM_LI_SET(insn, disp >> 2);
    IFORM_AA_SET(insn, 0);
    if (link)
        IFORM_LK_SET(insn, 1);
    else
        IFORM_LK_SET(insn, 0);

    insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link) {

    long disp = (to - from);
//    fprintf(stderr, "[insnCodeGen::generateBranch] Generating branch from %p to %p\n", from, to);
    // if (from == 0x10000750 || from == 0x10000758) {
    //   fprintf(stderr, "Stop here\n");
    // }
    if (ABS(disp) > MAX_BRANCH) {
      //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        return generateLongBranch(gen, from, to, link);
    }
    //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    return generateBranch(gen, disp, link);
   
}

void insnCodeGen::generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to) {
    //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    generateBranch(gen, from, to, true);
}


void GenerateSavesBaseTrampStyle(codeGen &gen) {
  // Save everything, then all things are scratch registers...

    unsigned int width = gen.width();

    int gpr_off, fpr_off;
    gpr_off = TRAMP_GPR_OFFSET(width);
    fpr_off = TRAMP_FPR_OFFSET(width);

    // Make a stack frame.
    pushStack(gen);

    // Save GPRs
    saveGPRegisters(gen, gen.rs(), gpr_off);


    saveFPRegisters(gen, gen.rs(), fpr_off);
    //fprintf(stderr, "I am called!\n");
    // Save LR            
    saveLR(gen, REG_SCRATCH /* register to use */, TRAMP_SPR_OFFSET(width) + STK_LR);

    saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), true); // FIXME get liveness fixed
}


void GenerateRestoresBaseTrampStyle(codeGen &gen) {
    unsigned int width = gen.width();

    int gpr_off, fpr_off;
    gpr_off = TRAMP_GPR_OFFSET(width);
    fpr_off = TRAMP_FPR_OFFSET(width);

    // Restore possible SPR saves
    restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

    // LR
    restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET(width) + STK_LR);

  restoreFPRegisters(gen, gen.rs(), fpr_off);

    // GPRs
    restoreGPRegisters(gen, gen.rs(), gpr_off);

    /*
    // Multithread GPR -- always save
    restoreRegister(gen, REG_MT_POS, TRAMP_GPR_OFFSET);
    */

    popStack(gen);
}

void insnCodeGen::generateMoveToSPR(codeGen &gen, Dyninst::Register toSPR,
                                    unsigned sprReg) {
  // Check that this SPR exists
  if (sprReg != SPR_TAR && sprReg != SPR_LR && sprReg != SPR_CTR)
    assert("SPR Dyninst::Register is not valid" == 0);

  // Move the register to the spr 
  instruction moveToBr;
  moveToBr.clear();
  XFXFORM_OP_SET(moveToBr, MTSPRop);
  XFXFORM_RT_SET(moveToBr, toSPR);
  XFORM_RA_SET(moveToBr, sprReg & 0x1f);
  XFORM_RB_SET(moveToBr, (sprReg >> 5) & 0x1f);
  XFXFORM_XO_SET(moveToBr, MTSPRxop); // From assembly manual
  insnCodeGen::generate(gen,moveToBr); 
}

void insnCodeGen::generateMoveFromSPR(codeGen &gen,  Dyninst::Register toSPR,
                                    unsigned sprReg) {
  // Check that this SPR exists
  if (sprReg != SPR_TAR && sprReg != SPR_LR && sprReg != SPR_CTR)
    assert("SPR Dyninst::Register is not valid" == 0);

  // Move the register to the spr 
  instruction moveToBr;
  moveToBr.clear();
  XFXFORM_OP_SET(moveToBr, MFSPRop);
  XFXFORM_RT_SET(moveToBr, toSPR);
  XFORM_RA_SET(moveToBr, sprReg & 0x1f);
  XFORM_RB_SET(moveToBr, (sprReg >> 5) & 0x1f);
  XFXFORM_XO_SET(moveToBr, MFSPRxop); // From assembly manual
  insnCodeGen::generate(gen,moveToBr); 
}

void insnCodeGen::generateVectorLoad(codeGen &gen, unsigned vectorReg, Dyninst::Register RegAddress) {
  //insnCodeGen::generateImm(gen, CALop,  rt, 0,  BOT_LO(value)); 
  instruction loadInstruction;
  XLFORM_OP_SET(loadInstruction, LXVD2Xop);
  XLFORM_BT_SET(loadInstruction, vectorReg); // From architecture manual
  XLFORM_BA_SET(loadInstruction, 0); // Unused
  XLFORM_BB_SET(loadInstruction, RegAddress); // Unused
  XLFORM_XO_SET(loadInstruction, LXVD2Xxo);
  XLFORM_LK_SET(loadInstruction, 0); // Unused? 
  insnCodeGen::generate(gen,loadInstruction);
}

void insnCodeGen::generateVectorStore(codeGen & gen, unsigned vectorReg, Dyninst::Register RegAddress) {
  instruction storeInstruction;
  XLFORM_OP_SET(storeInstruction, STXVD2Xop);
  XLFORM_BT_SET(storeInstruction, vectorReg); // From architecture manual
  XLFORM_BA_SET(storeInstruction, 0); // Unused
  XLFORM_BB_SET(storeInstruction, RegAddress); // Unused
  XLFORM_XO_SET(storeInstruction, STXVD2Xxo);
  XLFORM_LK_SET(storeInstruction, 0); // Unused? 
  insnCodeGen::generate(gen,storeInstruction);  
}

void insnCodeGen::saveVectors(codeGen & gen, int startStackOffset) {
  for (int i = 0; i < 32; i++) {
    insnCodeGen::generateImm(gen, CALop, registerSpace::r10 , registerSpace::r1,  BOT_LO(startStackOffset + (16*(i+1))));
    insnCodeGen::generateVectorStore(gen, i, registerSpace::r10);
  }
}
void insnCodeGen::restoreVectors(codeGen & gen, int startStackOffset) {
  for (int i = 0; i < 32; i++) {
    insnCodeGen::generateImm(gen, CALop, registerSpace::r10 , registerSpace::r1,  BOT_LO(startStackOffset + (16*(i+1))));
    insnCodeGen::generateVectorLoad(gen, i, registerSpace::r10);
  }
}

bool insnCodeGen::generateBranchTar(codeGen &gen, Dyninst::Register scratch,
                                    Dyninst::Address dest,
                                    bool isCall) {
  // Generates a branch using TAR to the address specified in dest. 
  // Returns true if this branch type was successfully used

  // TODO: Add liveness checking for TAR. We are assuming this is not live.

  // Move the address to the scratch register
  // Done because TAR can only be set called using a register value
  insnCodeGen::loadImmIntoReg(gen, scratch, dest);

  // Generate the instruction to move the reg -> tar
  insnCodeGen::generateMoveToSPR(gen, scratch, SPR_TAR);

  // Aaaand now branch, linking if appropriate
  instruction branchToBr;
  branchToBr.clear();
  XLFORM_OP_SET(branchToBr, BCTARop);
  XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
  XLFORM_BA_SET(branchToBr, 0); // Unused
  XLFORM_BB_SET(branchToBr, 0); // Unused
  XLFORM_XO_SET(branchToBr, BCTARxop);
  XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
  insnCodeGen::generate(gen,branchToBr);
  return true;
}

bool insnCodeGen::generateBranchLR(codeGen &gen, Dyninst::Register scratch,
                                    Dyninst::Address dest,
                                    bool isCall) {
  // Generates a branch using LR to the address specified in dest. 
  // Returns true if this branch type was successfully used

  // TODO: Add liveness checking for LR. We are assuming this is not live.

  // Move the address to the scratch register
  // Done because TAR can only be set called using a register value
  insnCodeGen::loadImmIntoReg(gen, scratch, dest);

  // Generate the instruction to move the reg -> tar
  insnCodeGen::generateMoveToSPR(gen, scratch, SPR_LR);

  // Aaaand now branch, linking if appropriate
  instruction branchToBr;
  branchToBr.clear();
  XLFORM_OP_SET(branchToBr, BCLRop);
  XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
  XLFORM_BA_SET(branchToBr, 0); // Unused
  XLFORM_BB_SET(branchToBr, 0); // Unused
  XLFORM_XO_SET(branchToBr, BCLRxop);
  XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
  insnCodeGen::generate(gen,branchToBr);
  return true;
}


bool insnCodeGen::generateBranchCTR(codeGen &gen, 
                                    Dyninst::Register scratch,
                                    Dyninst::Address dest,
                                    bool isCall) {
  // Generates a branch using TAR to the address specified in dest. 
  // Returns true if this branch type was successfully used

  // TODO: Add liveness checking for TAR. We are assuming this is not live.

  // Move the address to the scratch register
  // Done because TAR can only be set called using a register value
  insnCodeGen::loadImmIntoReg(gen, scratch, dest);

  // Generate the instruction to move the reg -> tar
  insnCodeGen::generateMoveToSPR(gen, scratch, SPR_LR);

  // Aaaand now branch, linking if appropriate
  instruction branchToBr;
  branchToBr.clear();
  XLFORM_OP_SET(branchToBr, BCCTRop);
  XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
  XLFORM_BA_SET(branchToBr, 0); // Unused
  XLFORM_BB_SET(branchToBr, 0); // Unused
  XLFORM_XO_SET(branchToBr, BCCTRxop);
  XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
  insnCodeGen::generate(gen,branchToBr);
  return true;
}



#include "addressSpace.h"
#include "instPoint.h"
#include "function.h"
instPoint * GetInstPointPower(codeGen & gen, Dyninst::Address from) {
    // If this point is straight availible from the generator, return it
    instPoint *point = gen.point();
    if (point) 
      return point;

    // Take the hardest road....

    // Grab the function instance from addressSpace.
    AddressSpace * curAddressSpace = gen.addrSpace();

    // Find the func instance
    std::set<func_instance *> funcList;
    curAddressSpace->findFuncsByAddr(from, funcList);

    for (auto i :  funcList)
    {
      point = instPoint::funcEntry(i);
      if (point != NULL){
        if (point->addr_compat() == from){
          return point;
        }
      }
    }
    return NULL;
}
void insnCodeGen::generateLongBranch(codeGen &gen, 
                                     Dyninst::Address from,
                                     Dyninst::Address to,
                                     bool isCall) {
  bool usingLR = false;
  bool usingCTR = false;
  //fprintf(stderr, "%s\n", "inside generate long branch");
  // If we are a call, the LR is going to be free. Use TAR to save/restore any register
  if (isCall) {
    //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    //fprintf(stderr, "%s\n", "generating call long branch using LR");
      // This is making the assumption R2/R12 has already been setup correctly, 
      // First generate a scratch register by moving something, i choose R11 to send to TAR
      insnCodeGen::generateMoveToSPR(gen,registerSpace::r10, SPR_TAR);
      
      // r10 is now free to setup the branch instruction
      insnCodeGen::loadImmIntoReg(gen, registerSpace::r10, to);
      insnCodeGen::generateMoveToSPR(gen,registerSpace::r10, SPR_LR);

      // Return r10 to its original state
      insnCodeGen::generateMoveFromSPR(gen, registerSpace::r10, SPR_TAR);

      // Emit the branch instruction
      instruction branchToBr;
      branchToBr.clear();
      XLFORM_OP_SET(branchToBr, BCLRop);
      XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
      XLFORM_BA_SET(branchToBr, 0); // Unused
      XLFORM_BB_SET(branchToBr, 0); // Unused
      XLFORM_XO_SET(branchToBr, BCLRxop);
      XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
      insnCodeGen::generate(gen,branchToBr);
  } else {
    //fprintf(stderr, "%s\n", "generating non-call long branch using TAR");
    // What this does is the following:
    // 1. Attempt to allocate a scratch register, this is needed to store the destination 
    //    address temporarily because you can only move registers to SPRs like CTR/LR/TAR.
    //    - If a scratch register cannot be obtained, see if either CTR or LR are free.
    //    - If one of those are, store r11 (our new scratch register) into CTR or LR. 
    //    - after the destination address has been loaded to TAR, we will restore R11 from this value
    // 2. Calculate the destination address storing it into scratch.
    // 3. Move the register to the SPR (tar)
    // 4. Restore the original register value (if a scratch register was not found)
    // 5. build the branch instruction.
    //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
    Dyninst::Register scratch = Null_Register;
    // TODO: Fix this, this should work....
    //= gen.rs()->getScratchRegister(gen);
    if (scratch == Null_Register) {
      //fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__); 
        instPoint *point = GetInstPointPower(gen, from);//gen.point();
        if (!point) {
          // No clue if CTR or LR are filled, use broken trap and likely fail.
            //fprintf(stderr, "%s\n", "Couldn't grab point - Using a trap instruction.....");
            return generateBranchViaTrap(gen, from, to, isCall);
        }
        // Grab the register space, and see if LR or CTR are free.
        // What we are going to do here is use the LR/CTR as temporary store for an existing register value
        std::vector<Dyninst::Register> potentialRegisters = {registerSpace::r3, registerSpace::r4, registerSpace::r5, registerSpace::r6, registerSpace::r7, registerSpace::r8, registerSpace::r9, registerSpace::r10};
        bitArray liveRegs = point->liveRegisters();

        for (int iter =  potentialRegisters.size() - 1; iter >= 0; iter = iter - 1) {
          if (liveRegs[potentialRegisters[iter]] == false) {
            scratch = potentialRegisters[iter]; 
            break;
          }
        }
        if (scratch == Null_Register) {
          if (liveRegs[registerSpace::lr] == false && isCall) {
              usingLR = true;
              // Dyninst::Register 11 is the chosen one for using temporarily.
              generateMoveToSPR(gen, registerSpace::r10, SPR_LR);
          } else if (liveRegs[registerSpace::ctr] == false) {
              usingCTR = true;
              generateMoveToSPR(gen, registerSpace::r10, SPR_CTR);
          }
          if (!usingLR && !usingCTR) {
              //fprintf(stderr, "%s\n", "Couldn't grab free register - Using a trap instruction.....");
              return generateBranchViaTrap(gen, from, to, isCall);
          }
        }
    } else if (scratch != Null_Register) {
        //fprintf(stderr, "%s\n", "Generating branch with TAR.....");
        insnCodeGen::generateBranchTar(gen, scratch, to, isCall);
        return;
    }

    if (scratch == Null_Register) {
      // Now the fun stuff....
      // Loed destination value into r11, copy it to SPR_TAR, restore the original R11 value.
      insnCodeGen::loadImmIntoReg(gen, registerSpace::r10, to);
      insnCodeGen::generateMoveToSPR(gen, registerSpace::r10, SPR_TAR);
      if (usingCTR)
        insnCodeGen::generateMoveFromSPR(gen, registerSpace::r10, SPR_CTR);
      else if (usingLR)
        insnCodeGen::generateMoveFromSPR(gen, registerSpace::r10, SPR_LR);
      else 
        assert("SHOULD NEVER BE HERE" == 0);
    } else {
      insnCodeGen::loadImmIntoReg(gen, scratch, to);
      insnCodeGen::generateMoveToSPR(gen, scratch, SPR_TAR);      
    }
    // Emit the call instruction.
    instruction branchToBr;
    branchToBr.clear();
    XLFORM_OP_SET(branchToBr, BCTARop);
    XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
    XLFORM_BA_SET(branchToBr, 0); // Unused
    XLFORM_BB_SET(branchToBr, 0); // Unused
    XLFORM_XO_SET(branchToBr, BCTARxop);
    XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
    insnCodeGen::generate(gen, branchToBr);    
  }
  return;

  // bool everythingSaved = false;
  // //fprintf(stderr, "[insnCodeGen::generateLongBranch] Generating long branch from %p to %p\n", from, to);
  //   // First, see if we can cheap out
  //   long disp = (to - from);
  //   if (ABS(disp) <= MAX_BRANCH) {
  //       return generateBranch(gen, disp, isCall);
  //   }

  //   // We can use a register branch via the LR or CTR, if either of them
  //   // is free.
    
  //   // Let's see if we can grab a free GPregister...
  //   instPoint *point = gen.point();
  //   if (!point) {
  //       // fprintf(stderr, " %s[%d] No point generateBranchViaTrap \n", FILE__, __LINE__);
  //       fprintf(stderr, "This is an instruction which we would trap on\n");
  //       fprintf(stderr, "[insnCodeGen::generateLongBranch] Building long branch from %llx to %llx at position %llx and this branch call status is %d\n", from, to, gen.currAddr(), isCall);
  //       // Generate branch via traps is broken, never call it. 
  //       // assert(1 == 0);
  //       // return generateBranchViaTrap(gen, from, to, isCall);
  //   }

  //   if 
  //   // Could see if the codeGen has it, but right now we have assert
  //   // code there and we don't want to hit that.
  //   registerSpace *rs = registerSpace::actualRegSpace(point);
  //   gen.setRegisterSpace(rs);
  //   Dyninst::Register scratch = rs->getScratchRegister(gen, true);
  //   // 
  //   assert(scratch == Null_Register);

  //   if (scratch == Null_Register) { 
  //       // Just save and restore everything, this is bad but its likely safe and can be revisted later.
  //       // GenerateSavesBaseTrampStyle(gen);
  //       // everythingSaved = true;
  //       // do nothing, return
        
  //       // scratch = registerSpace::r12;
  //       // assert(everythingSaved != true)
  //       // On Linux we save under the stack and hope it doesn't
  //       // cause problems.
        
  //       // original
  //       //fprintf(stderr, " %s[%d] No registers generateBranchViaTrap \n", FILE__, __LINE__);
  //       return generateBranchViaTrap(gen, from, to, isCall);
  //   }
    
  //   // Load the destination into our scratch register
  //   insnCodeGen::loadImmIntoReg(gen, scratch, to);
  //   unsigned branchRegister = registerSpace::lr;
  //   // Find out whether the LR or CTR is "dead"...
  //   //bitArray liveRegs = point->liveRegisters();
  //   // unsigned branchRegister = 0;
  //   // if (liveRegs[registerSpace::lr] == false || everythingSaved == true) {
  //   //     branchRegister = registerSpace::lr;
  //   // }
  //   // else {
  //   //     // live LR means we need to save/restore somewhere
  //   //     if(isCall) return generateBranchViaTrap(gen, from, to, isCall);
  //   //     if (liveRegs[registerSpace::ctr] == false) {
  //   //         branchRegister = registerSpace::ctr;
  //   //     }
  //   // }

  //   if (!branchRegister) {
  //       fprintf(stderr, " %s[%d] No branch register generateBranchViaTrap \n", FILE__, __LINE__);
  //       return generateBranchViaTrap(gen, from, to, isCall); 
  //   }
    
  //   assert(branchRegister);

  //   instruction moveToBr;
  //   moveToBr.clear();
  //   XFXFORM_OP_SET(moveToBr, MTSPRop);
  //   XFXFORM_RT_SET(moveToBr, scratch);
  //   if (branchRegister == registerSpace::lr) {
  //       XFORM_RA_SET(moveToBr, SPR_LR & 0x1f);
  //       XFORM_RB_SET(moveToBr, (SPR_LR >> 5) & 0x1f);
  //       // The two halves (top 5 bits/bottom 5 bits) are _reversed_ in this encoding. 
  //   }
  //   else {
  //       XFORM_RA_SET(moveToBr, SPR_CTR & 0x1f);
  //       XFORM_RB_SET(moveToBr, (SPR_CTR >> 5) & 0x1f);
  //   }
  //   XFXFORM_XO_SET(moveToBr, MTSPRxop); // From assembly manual
  //   insnCodeGen::generate(gen,moveToBr);
  //   // Aaaand now branch, linking if appropriate
  //   instruction branchToBr;
  //   branchToBr.clear();
  //   XLFORM_OP_SET(branchToBr, BCLRop);
  //   XLFORM_BT_SET(branchToBr, 0x14); // From architecture manual
  //   XLFORM_BA_SET(branchToBr, 0); // Unused
  //   XLFORM_BB_SET(branchToBr, 0); // Unused
  //   if (branchRegister == registerSpace::lr) {
  //       XLFORM_XO_SET(branchToBr, BCLRxop);
  //   }
  //   else {
  //       XLFORM_XO_SET(branchToBr, BCCTRxop);
  //   }
  //   XLFORM_LK_SET(branchToBr, (isCall ? 1 : 0));
  //   insnCodeGen::generate(gen,branchToBr);

  //   // restore the world
  //   if(everythingSaved)
  //     GenerateRestoresBaseTrampStyle(gen);
}

void insnCodeGen::generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall) {

  //fprintf(stderr, "[insnCodeGen::generateBranchViaTrap] Generating branch via trap from %p to %p\n", from, to);
    long disp = to - from;
    if (ABS(disp) <= MAX_BRANCH) {
        // We shouldn't be here, since this is an internal-called-only func.
        return generateBranch(gen, disp, isCall);
    }
    //assert (isCall == false); // Can't do this yet
    if (isCall) {
      // Screw using a trap, just emit a call and save/restore all registers (painful but whatever).
      //emitCall()
      assert(isCall == false);       
      //assert(shouldAssertIfInLongBranch != true);
      // failedLongBranchLocal = true;
    } else {    
      if (gen.addrSpace()) {
          // Too far to branch.  Use trap-based instrumentation.

        //fprintf(stderr, "I am in where we should be generating instructions\n" );
        
        // Here is a potential strategy
        // 1. Create a stack frame
        // 2. Push a (we like r10) register to the frame.
        // 3. Calculate the effective address into the register
        // 4. Push to TAR
        // 5. Restore previous register
        // 6. Delete frame
        // 7. branch to tar. 


        //instruction insn(NOOPraw);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        //insnCodeGen::generate(gen,insn);
        gen.addrSpace()->trapMapping.addTrapMapping(from, to, true);
        insnCodeGen::generateTrap(gen);        
      } else {
          // Too far to branch and no proc to register trap.
          fprintf(stderr, "ABS OFF: 0x%lx, MAX: 0x%lx\n",
                  (unsigned long)ABS(disp), (unsigned long) MAX_BRANCH);
          bperr( "Error: attempted a branch of 0x%lx\n", (unsigned long)disp);
          logLine("a branch too far\n");
          showErrorCallback(52, "Internal error: branch too far");
          bperr( "Attempted to make a branch of offset 0x%lx\n", (unsigned long)disp);
          assert(0);
      }
    }
}

void insnCodeGen::generateAddReg (codeGen & gen, int op, Dyninst::Register rt,
				   Dyninst::Register ra, Dyninst::Register rb)
{

  instruction insn;
  insn.clear();
  XOFORM_OP_SET(insn, op);
  XOFORM_RT_SET(insn, rt);
  XOFORM_RA_SET(insn, ra);
  XOFORM_RB_SET(insn, rb);
  XOFORM_OE_SET(insn, 0);
  XOFORM_XO_SET(insn, 266);
  XOFORM_RC_SET(insn, 0);

  insnCodeGen::generate (gen,insn);
}

void insnCodeGen::generateLoadReg(codeGen &gen, Dyninst::Register rt,
                                  Dyninst::Register ra, Dyninst::Register rb)
{
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, LXop);
    XFORM_RT_SET(insn, rt);
    XFORM_RA_SET(insn, ra);
    XFORM_RB_SET(insn, rb);
    XFORM_XO_SET(insn, LXxop);
    XFORM_RC_SET(insn, 0);

    insnCodeGen::generate (gen,insn);
}

void insnCodeGen::generateStoreReg(codeGen &gen, Dyninst::Register rt,
                                   Dyninst::Register ra, Dyninst::Register rb)
{
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, STXop);
    XFORM_RT_SET(insn, rt);
    XFORM_RA_SET(insn, ra);
    XFORM_RB_SET(insn, rb);
    XFORM_XO_SET(insn, STXxop);
    XFORM_RC_SET(insn, 0);

    insnCodeGen::generate (gen,insn);
}

void insnCodeGen::generateLoadReg64(codeGen &gen, Dyninst::Register rt,
                                    Dyninst::Register ra, Dyninst::Register rb)
{
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, LXop);
    XFORM_RT_SET(insn, rt);
    XFORM_RA_SET(insn, ra);
    XFORM_RB_SET(insn, rb);
    XFORM_XO_SET(insn, LDXxop);
    XFORM_RC_SET(insn, 0);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateStoreReg64(codeGen &gen, Dyninst::Register rs,
                                     Dyninst::Register ra, Dyninst::Register rb)
{
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, STXop);
    XFORM_RT_SET(insn, rs);
    XFORM_RA_SET(insn, ra);
    XFORM_RB_SET(insn, rb);
    XFORM_XO_SET(insn, STXxop);
    XFORM_RC_SET(insn, 0);

    insnCodeGen::generate(gen, insn);
}

void insnCodeGen::generateImm(codeGen &gen, int op, Dyninst::Register rt, Dyninst::Register ra, int immd)
 {
  // something should be here to make sure immd is within bounds
  // bound check really depends on op since we have both signed and unsigned
  //   opcodes.
  // We basically check if the top bits are 0 (unsigned, or positive signed)
  // or 0xffff (negative signed)
  // This is because we don't enforce calling us with LOW(immd), and
  // signed ints come in with 0xffff set. C'est la vie.
  // TODO: This should be a check that the high 16 bits are equal to bit 15,
  // really.
  assert (((immd & 0xffff0000) == (0xffff0000)) ||
          ((immd & 0xffff0000) == (0x00000000)));

  instruction insn;
  
  insn.clear();
  DFORM_OP_SET(insn, op);
  DFORM_RT_SET(insn, rt);
  DFORM_RA_SET(insn, ra);
  if (op==SIop) immd = -immd;
  DFORM_SI_SET(insn, immd);

  insnCodeGen::generate(gen,insn);
}

void insnCodeGen::generateMemAccess64(codeGen &gen, int op, int xop, Dyninst::Register r1, Dyninst::Register r2, int immd)
{
    assert(MIN_IMM16 <= immd && immd <= MAX_IMM16);
    assert((immd & 0x3) == 0);

    instruction insn;

    insn.clear();
    DSFORM_OP_SET(insn, op);
    DSFORM_RT_SET(insn, r1);
    DSFORM_RA_SET(insn, r2);
    DSFORM_DS_SET(insn, immd >> 2);
    DSFORM_XO_SET(insn, xop);

    insnCodeGen::generate(gen,insn);
}

// rlwinm ra,rs,n,0,31-n
void insnCodeGen::generateLShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	assert(shift<32);
	insn.clear();
	MFORM_OP_SET(insn, RLINMxop);
	MFORM_RS_SET(insn, rs);
	MFORM_RA_SET(insn, ra);
	MFORM_SH_SET(insn, shift);
	MFORM_MB_SET(insn, 0);
	MFORM_ME_SET(insn, 31-shift);
	MFORM_RC_SET(insn, 0);
	insnCodeGen::generate(gen,insn);

    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	insnCodeGen::generateLShift64(gen, rs, shift, ra);
    }
}

// rlwinm ra,rs,32-n,n,31
void insnCodeGen::generateRShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra, bool s)
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	assert(shift<32);
	insn.clear();
	MFORM_OP_SET(insn, RLINMxop);
	MFORM_RS_SET(insn, rs);
	MFORM_RA_SET(insn, ra);
	MFORM_SH_SET(insn, 32-shift);
	MFORM_MB_SET(insn, shift);
	MFORM_ME_SET(insn, 31);
	MFORM_RC_SET(insn, 0);
	insnCodeGen::generate(gen,insn);

    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	insnCodeGen::generateRShift64(gen, rs, shift, ra, s);
    }
}

// sld ra, rs, rb
void insnCodeGen::generateLShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
{
    instruction insn;

    assert(shift<64);
    insn.clear();
    MDFORM_OP_SET( insn, RLDop);
    MDFORM_RS_SET( insn, rs);
    MDFORM_RA_SET( insn, ra);
    MDFORM_SH_SET( insn, shift % 32);
    MDFORM_MB_SET( insn, (63-shift) % 32);
    MDFORM_MB2_SET(insn, (63-shift) / 32);
    MDFORM_XO_SET( insn, ICRxop);
    MDFORM_SH2_SET(insn, shift / 32);
    MDFORM_RC_SET( insn, 0);

    insnCodeGen::generate(gen,insn);
}

// srd ra, rs, rb
void insnCodeGen::generateRShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra, bool)
{
    // This function uses rotate-left to implement right shift.
    // Rotate left 64-n bits is rotating right n bits.
    // However, rotation cannot correctly represent signed right shifting.
    // So, this piece of code is wrong...
    instruction insn;

    assert(shift<64);
    insn.clear();
    MDFORM_OP_SET( insn, RLDop);
    MDFORM_RS_SET( insn, rs);
    MDFORM_RA_SET( insn, ra);
    MDFORM_SH_SET( insn, (64 - shift) % 32);
    MDFORM_MB_SET( insn, shift % 32);
    MDFORM_MB2_SET(insn, shift / 32);
    MDFORM_XO_SET( insn, ICLxop);
    MDFORM_SH2_SET(insn, (64 - shift) / 32);
    MDFORM_RC_SET( insn, 0);

    insnCodeGen::generate(gen,insn);
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
void insnCodeGen::generateNOOP(codeGen &gen, unsigned size)
{
    assert ((size % instruction::size()) == 0);
    while (size) {
        instruction insn(NOOPraw);
        insnCodeGen::generate(gen,insn);
        size -= instruction::size();
    }
}

void insnCodeGen::generateRelOp(codeGen &gen, int cond, int mode, Dyninst::Register rs1,
                                Dyninst::Register rs2, Dyninst::Register rd, bool s)
{
    instruction insn;

    // cmpd rs1, rs2
    insn.clear();
    XFORM_OP_SET(insn, CMPop);
    XFORM_RT_SET(insn, 0x1);    // really bf & l sub fields of rt we care about. Set l = 1 for 64 bit operation
    XFORM_RA_SET(insn, rs1);
    XFORM_RB_SET(insn, rs2);
    if (s)
        XFORM_XO_SET(insn, CMPxop);
    else
        XFORM_XO_SET(insn, CMPLxop);
    insnCodeGen::generate(gen,insn);

    // li rd, 1
    insnCodeGen::generateImm(gen, CALop, rd, 0, 1);

    // b??,a +2
    insn.clear();
    BFORM_OP_SET(insn, BCop);
    BFORM_BI_SET(insn, cond);
    BFORM_BO_SET(insn, mode);
    BFORM_BD_SET(insn, 2);		// + two instructions */
    BFORM_AA_SET(insn, 0);
    BFORM_LK_SET(insn, 0);
    insnCodeGen::generate(gen,insn);

    // clr rd
    insnCodeGen::generateImm(gen, CALop, rd, 0, 0);
}

// Given a value, load it into a register.
void insnCodeGen::loadImmIntoReg(codeGen &gen, Dyninst::Register rt, long value)
{
   // Writing a full 64 bits takes 5 instructions in the worst case.
   // Let's see if we use sign-extention to cheat.
   if (MIN_IMM16 <= value && value <= MAX_IMM16) {
      insnCodeGen::generateImm(gen, CALop,  rt, 0,  BOT_LO(value));      
   } else if (MIN_IMM32 <= value && value <= MAX_IMM32) {
      insnCodeGen::generateImm(gen, CAUop,  rt, 0,  BOT_HI(value));
      insnCodeGen::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
   } 
#if defined(arch_64bit)
   else if (MIN_IMM48 <= value && value <= MAX_IMM48) {
      insnCodeGen::generateImm(gen, CALop,  rt, 0,  TOP_LO(value));
      insnCodeGen::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         insnCodeGen::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      if (BOT_LO(value))
         insnCodeGen::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
      
   } else {

      insnCodeGen::generateImm(gen, CAUop,  rt,  0, TOP_HI(value));
      if (TOP_LO(value))
         insnCodeGen::generateImm(gen, ORILop, rt, rt, TOP_LO(value));
      insnCodeGen::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         insnCodeGen::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      if (BOT_LO(value))
         insnCodeGen::generateImm(gen, ORILop, rt, rt, BOT_LO(value));
   }
#endif
}

// Helper method.  Fills register with partial value to be completed
// by an operation with a 16-bit signed immediate.  Such as loads and
// stores.
void insnCodeGen::loadPartialImmIntoReg(codeGen &gen, Dyninst::Register rt, long value)
{
   if (MIN_IMM16 <= value && value <= MAX_IMM16) return;
   
   if (BOT_LO(value) & 0x8000) {
      // high bit of lowest half-word is set, so the sign extension of
      // the next op will cause the wrong effective addr to be computed.
      // so we subtract the sign ext value from the other half-words.
      // sounds odd, but works and saves an instruction - jkh 5/25/95
      value = ((value >> 16) - (std::numeric_limits<unsigned int>::max() >> 16)) << 16;
   }
   
   if (MIN_IMM32 <= value && value <= MAX_IMM32) {
      insnCodeGen::generateImm(gen, CAUop,  rt, 0,  BOT_HI(value));       
   } 
#if defined(arch_64bit)
   else if (MIN_IMM48 <= value && value <= MAX_IMM48) {
      insnCodeGen::generateImm(gen, CALop,  rt, 0,  TOP_LO(value));
      insnCodeGen::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         insnCodeGen::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
      
   } else {
      insnCodeGen::generateImm(gen, CAUop,  rt,  0, TOP_HI(value));
      if (TOP_LO(value))
         insnCodeGen::generateImm(gen, ORILop, rt, rt, TOP_LO(value));
      insnCodeGen::generateLShift64(gen, rt, 32, rt);
      if (BOT_HI(value))
         insnCodeGen::generateImm(gen, ORIUop, rt, rt, BOT_HI(value));
   }
#endif
}

int insnCodeGen::createStackFrame(codeGen &gen, int numRegs, std::vector<Dyninst::Register>& freeReg, std::vector<Dyninst::Register>& excludeReg){
              int gpr_off, stack_size;
                //create new stack frame
                gpr_off = TRAMP_GPR_OFFSET_32;
                pushStack(gen);
                // Save GPRs
                stack_size = saveGPRegisters(gen, gen.rs(), gpr_off, numRegs);
		assert (stack_size == numRegs);
		for (int i = 0; i < numRegs; i++){
			Dyninst::Register scratchReg = gen.rs()->getScratchRegister(gen, excludeReg, true);
			assert (scratchReg != Null_Register);
			freeReg.push_back(scratchReg);
			excludeReg.push_back(scratchReg);
		}
		return freeReg.size();
}

void insnCodeGen::removeStackFrame(codeGen &gen) {
                int gpr_off = TRAMP_GPR_OFFSET_32;
                restoreGPRegisters(gen, gen.rs(), gpr_off);
                popStack(gen);
}

                // {insn_ = {byte = {0xa6, 0x3, 0x8, 0x7c}, raw = 0x7c0803a6}}    
                // {insn_ = {byte = {0xa6, 0x3, 0x8, 0x7c}, raw = 0x7c0803a6}}       
bool insnCodeGen::generateMem(codeGen &,
                              instruction&,
                              Dyninst::Address,
                              Dyninst::Address,
                              Dyninst::Register,
                  Dyninst::Register) {return false; }

void insnCodeGen::generateMoveFromLR(codeGen &gen, Dyninst::Register rt) {
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, MFSPRop);
    XFORM_RT_SET(insn, rt);
    XFORM_RA_SET(insn, SPR_LR & 0x1f);
    XFORM_RB_SET(insn, (SPR_LR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MFSPRxop);
    generate(gen,insn);
}

void insnCodeGen::generateMoveToLR(codeGen &gen, Dyninst::Register rs) {
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, MTSPRop);
    XFORM_RT_SET(insn, rs);
    XFORM_RA_SET(insn, SPR_LR & 0x1f);
    XFORM_RB_SET(insn, (SPR_LR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    generate(gen,insn);
}
void insnCodeGen::generateMoveToCR(codeGen &gen, Dyninst::Register rs) {
    instruction insn;
    insn.clear();
    XFORM_OP_SET(insn, MTSPRop);
    XFORM_RT_SET(insn, rs);
    XFORM_RA_SET(insn, SPR_CTR & 0x1f);
    XFORM_RB_SET(insn, (SPR_CTR >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    generate(gen,insn);
}    

bool insnCodeGen::modifyJump(Dyninst::Address target,
			     NS_power::instruction &,
			     codeGen &gen) {
  failedLongBranchLocal = false;
  shouldAssertIfInLongBranch = false;
//  fprintf(stderr, "Setting link: %d Target Dyninst::Address: %p\n", IFORM_LK(insn), target);
  //assert(IFORM_LK(insn) == true);
  generateBranch(gen,
		 gen.currAddr(),
		 target,
		 false);
  if (failedLongBranchLocal == true){
    failedLongBranchLocal = false;
    shouldAssertIfInLongBranch = true;
    return false;
  }
  return true;
}

bool insnCodeGen::modifyJumpCall(Dyninst::Address target,
           NS_power::instruction &,
           codeGen &gen) {
  failedLongBranchLocal = false;
  shouldAssertIfInLongBranch = false;
  //fprintf(stderr, "Setting link: %d Target Dyninst::Address: %p\n", IFORM_LK(insn), target);
  //assert(IFORM_LK(insn) == true);
  generateBranch(gen,
     gen.currAddr(),
     target,
     true);
  if (failedLongBranchLocal == true){
    failedLongBranchLocal = false;
    shouldAssertIfInLongBranch = true;
    return false;
  }
  return true;
}


bool insnCodeGen::modifyJcc(Dyninst::Address target,
			    NS_power::instruction &insn,
			    codeGen &gen) {
  // We can be handed a conditional call or return instruction here. In these cases,
  // "fake" a conditional branch with the same condition code and then pass that
  // through. 

  long disp = target - gen.currAddr();

  if (ABS(disp) >= MAX_CBRANCH) {
    if ((BFORM_OP(insn) == BCop) && ((BFORM_BO(insn) & BALWAYSmask) == BALWAYScond)) {
      // Make sure to use the (to, from) version of generateBranch()
      // in case the branch is too far, and trap-based instrumentation
      // is needed.
	    insnCodeGen::generateBranch(gen, gen.currAddr(), target, BFORM_LK(insn));
        return true;
    } else {
      // Figure out if the original branch was predicted as taken or not
      // taken.  We'll set up our new branch to be predicted the same way
      // the old one was.
      
      // This makes my brain melt... here's what I think is happening. 
      // We have two sources of information, the bd (destination) 
      // and the predict bit. 
      // The processor predicts the jump as taken if the offset
      // is negative, and not taken if the offset is positive. 
      // The predict bit says "invert whatever you decided".
      // Since we're forcing the offset to positive, we need to
      // invert the bit if the offset was negative, and leave it
      // alone if positive.
      
      // Get the old flags (includes the predict bit)
      int flags = BFORM_BO(insn);
      
      if ((BFORM_OP(insn) == BCop) && (BFORM_BD(insn) < 0)) {
	// Flip the bit.
	// xor operator
	flags ^= BPREDICTbit;
      }
      
      instruction newCondBranch(insn);
      
      // Reset the opcode to 16 (BCop, branch conditional)
      BFORM_OP_SET(newCondBranch, BCop);

      // Set up the flags
      BFORM_BO_SET(newCondBranch, flags);

      // And condition register
      BFORM_BI_SET(newCondBranch, BFORM_BI(insn));

      // Change the branch to move one instruction ahead
      BFORM_BD_SET(newCondBranch, 2);

      BFORM_LK_SET(newCondBranch, 0); // This one is non-linking for sure      

      generate(gen,newCondBranch);

      // We don't "relocate" the fallthrough target of a conditional
      // branch; instead relying on a third party to make sure
      // we go back to where we want to. So in this case we 
      // generate a "dink" branch to skip past the next instruction.
      // We could also just invert the condition on the first branch;
      // but I don't have the POWER manual with me.
      // -- bernat, 15JUN05
      
      insnCodeGen::generateBranch(gen,
				  2*instruction::size());
      
      bool link = (BFORM_LK(insn) == 1);
      // Make sure to use the (to, from) version of generateBranch()
      // in case the branch is too far, and trap-based instrumentation
      // is needed.
      insnCodeGen::generateBranch(gen,
				  gen.currAddr(),
				  target,
				  link);
      return true;
    }
  } else {
    instruction newInsn(insn);

    // Reset the opcode to 16 (BCop, branch conditional)
    BFORM_OP_SET(newInsn, BCop);

    BFORM_BD_SET(newInsn, disp >> 2);
    BFORM_AA_SET(newInsn, 0);
    
    generate(gen,newInsn);
    return true;
  }
  return false;
}

bool insnCodeGen::modifyCall(Dyninst::Address target,
			     NS_power::instruction &insn,
			     codeGen &gen) {
  // This is actually a mashup of conditional/unconditional handling
  if (insn.isUncondBranch())
    return modifyJumpCall(target, insn, gen);
  else
    return modifyJcc(target, insn, gen);
}

//FIXME
//This function is used for PC-relative and hence may not be required for PPC. Consider for update/removal.
bool insnCodeGen::modifyData(Dyninst::Address /*target*/,
			     NS_power::instruction &insn,
			     codeGen &gen) {
  // Only know how to "modify" syscall...
  if (insn.opcode() != SVCop) return false;
  gen.copy(insn.ptr(), insn.size());
  return true;
}

