/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: liveness.C,v 1.12 2008/09/04 21:06:20 bill Exp $

#if defined(cap_liveness)

#include "debug.h"
#include "image-func.h"
#include "function.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "debug.h"
#if defined(cap_instruction_api)
#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/Instruction.h"
#include "addressSpace.h"
using namespace Dyninst::InstructionAPI;
#else
#include "InstrucIter.h"
#endif // defined(cap_instruction_api)
#include "symtab.h"

#if defined(arch_x86) || defined(arch_x86_64)
// Special-casing for IA-32...
#include "inst-x86.h"
#include "instructionAPI/h/RegisterIDs-x86.h"
#endif

#if defined(cap_instruction_api)
ReadWriteInfo calcRWSets(Instruction::Ptr insn, image_basicBlock* blk, unsigned width);
#else
ReadWriteInfo calcRWSets(InstrucIter ii, image_basicBlock* blk, unsigned width);
#endif
InstructionCache image_basicBlock::cachedLivenessInfo = InstructionCache();

  

// Move this to a platform-specific file
#if defined(cap_instruction_api)
#include <boost/assign/list_of.hpp>
using namespace boost::assign;


map<IA32Regs, Register> reverseRegisterMap = map_list_of
(r_EAX, REGNUM_RAX)
    (r_ECX, REGNUM_RCX)
    (r_EDX, REGNUM_RDX)
    (r_EBX, REGNUM_RBX)
    (r_ESP, REGNUM_RSP)
    (r_EBP, REGNUM_RBP)
    (r_ESI, REGNUM_RSI)
    (r_EDI, REGNUM_RDI)
    (r_R8, REGNUM_R8)
    (r_R9, REGNUM_R9)
    (r_R10, REGNUM_R10)
    (r_R11, REGNUM_R11)
    (r_R12, REGNUM_R12)
    (r_R13, REGNUM_R13)
    (r_R14, REGNUM_R14)
    (r_R15, REGNUM_R15)
    (r_DummyFPR, REGNUM_DUMMYFPR)
    (r_OF, REGNUM_OF)
    (r_SF, REGNUM_SF)
    (r_ZF, REGNUM_ZF)
    (r_AF, REGNUM_AF)
    (r_PF, REGNUM_PF)
    (r_CF, REGNUM_CF)
    (r_TF, REGNUM_TF)
    (r_IF, REGNUM_IF)
    (r_DF, REGNUM_DF)
    (r_NT, REGNUM_NT)
    (r_RF, REGNUM_RF)
    (r_AH, REGNUM_RAX)
    (r_BH, REGNUM_RBX)
    (r_CH, REGNUM_RCX)
    (r_DH, REGNUM_RDX)
    (r_AL, REGNUM_RAX)
    (r_BL, REGNUM_RBX)
    (r_CL, REGNUM_RCX)
    (r_DL, REGNUM_RDX)
    (r_SPL, REGNUM_RSP)
    (r_BPL, REGNUM_RBP)
    (r_SIL, REGNUM_RSI)
    (r_DIL, REGNUM_RDI)
    (r_eAX, REGNUM_RAX)
    (r_eBX, REGNUM_RBX)
    (r_eCX, REGNUM_RCX)
    (r_eDX, REGNUM_RDX)
    (r_AX, REGNUM_RAX)
    (r_DX, REGNUM_RDX)
    (r_eSP, REGNUM_RSP)
    (r_eBP, REGNUM_RBP)
    (r_eSI, REGNUM_RSI)
    (r_eDI, REGNUM_RDI)
    // These are wrong, need to extend to make cmpxch8b work right
    (r_EDXEAX, REGNUM_RAX)
    (r_ECXEBX, REGNUM_RCX)
    (r_CS, REGNUM_IGNORED)
    (r_DS, REGNUM_IGNORED)
    (r_ES, REGNUM_IGNORED)
    (r_FS, REGNUM_IGNORED)
    (r_GS, REGNUM_IGNORED)
  (r_SS, REGNUM_IGNORED)
  (r_rAX, REGNUM_RAX)
  (r_rCX, REGNUM_RCX)
  (r_rDX, REGNUM_RDX)
  (r_rBX, REGNUM_RBX)
  (r_rSP, REGNUM_RSP)
  (r_rBP, REGNUM_RBP)
  (r_rSI, REGNUM_RSI)
  (r_rDI, REGNUM_RDI)
  (r_EFLAGS, REGNUM_IGNORED)
  (r_EIP, REGNUM_IGNORED)
  (r_RIP, REGNUM_IGNORED)
  (r_RAX, REGNUM_RAX)
  (r_RCX, REGNUM_RCX)
  (r_RDX, REGNUM_RDX)
  (r_RBX, REGNUM_RBX)
  (r_RSP, REGNUM_RSP)
  (r_RBP, REGNUM_RBP)
  (r_RSI, REGNUM_RSI)
  (r_RDI, REGNUM_RDI)
  (r_SI, REGNUM_RSI)
  (r_DI, REGNUM_RDI)
  (r_XMM0, REGNUM_XMM0)
  (r_XMM1, REGNUM_XMM1)
  (r_XMM2, REGNUM_XMM2)
  (r_XMM3, REGNUM_XMM3)
  (r_XMM4, REGNUM_XMM4)
  (r_XMM5, REGNUM_XMM5)
  (r_XMM6, REGNUM_XMM6)
  (r_XMM7, REGNUM_XMM7)
  (r_MM0, REGNUM_MM0)
  (r_MM1, REGNUM_MM1)
  (r_MM2, REGNUM_MM2)
  (r_MM3, REGNUM_MM3)
  (r_MM4, REGNUM_MM4)
  (r_MM5, REGNUM_MM5)
  (r_MM6, REGNUM_MM6)
  (r_MM7, REGNUM_MM7)
  (r_CR0, REGNUM_IGNORED)
  (r_CR1, REGNUM_IGNORED)
  (r_CR2, REGNUM_IGNORED)
  (r_CR3, REGNUM_IGNORED)
  (r_CR4, REGNUM_IGNORED)
  (r_CR5, REGNUM_IGNORED)
  (r_CR6, REGNUM_IGNORED)
  (r_CR7, REGNUM_IGNORED)
  (r_DR0, REGNUM_IGNORED)
  (r_DR1, REGNUM_IGNORED)
  (r_DR2, REGNUM_IGNORED)
  (r_DR3, REGNUM_IGNORED)
  (r_DR4, REGNUM_IGNORED)
  (r_DR5, REGNUM_IGNORED)
  (r_DR6, REGNUM_IGNORED)
  (r_DR7, REGNUM_IGNORED)
  (r_ALLGPRS, REGNUM_IGNORED)
;

Register convertRegID(IA32Regs toBeConverted)
{
    map<IA32Regs, Register>::const_iterator found = 
        reverseRegisterMap.find(toBeConverted);
    if(found == reverseRegisterMap.end()) {
        fprintf(stderr, "Register ID %d not found in reverseRegisterLookup!\n", toBeConverted);
        assert(!"Bad register ID");
    }
    return found->second;
}
#endif

// Code for register liveness detection

// Takes information from instPoint and resets
// regSpace liveness information accordingly
// Right now, all the registers are assumed to be live by default
void registerSpace::specializeSpace(const bitArray &liveRegs) {
    // Liveness info is stored as a single bitarray for all registers.

#if defined(arch_x86) || defined(arch_x86_64) 
    // We use "virtual" registers on the IA-32 platform (or AMD-64 in 
    // 32-bit mode), and thus the registerSlot objects have _no_ relation
    // to the liveRegs input set. We handle this as a special case, and
    // look only for the flags representation (which is used to set
    // the IA32_FLAG_VIRTUAL_REGISTER "register"
    assert(liveRegs.size() == getBitArray().size());
    if (addr_width == 4) {
        for (unsigned i = 1; i <= NUM_VIRTUAL_REGISTERS; i++) {
            registers_[i]->liveState = registerSlot::dead;
        }
        registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::dead;
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            if (liveRegs[i]) {
                registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::live;
                break;
            }
        }
        // All we care about for now.
        return;
    }
#endif
    assert(liveRegs.size() == getBitArray().size());
    for (regDictIter i = registers_.begin(); i != registers_.end(); i++) {
        if (liveRegs[i.currval()->number])
            i.currval()->liveState = registerSlot::live;
        else
            i.currval()->liveState = registerSlot::dead;
    }
#if defined(arch_x86_64)
    // ???
    registers_[REGNUM_RAX]->liveState = registerSlot::live;
#endif

}

const bitArray &image_basicBlock::getLivenessIn() {
    // Calculate if it hasn't been done already
    if (in.size() == 0)
        summarizeBlockLivenessInfo();
    return in;
}

const bitArray image_basicBlock::getLivenessOut() const {
    bitArray out(in.size());

    // OUT(X) = UNION(IN(Y)) for all successors Y of X
    pdvector<image_edge *> target_edges;
    getTargets(target_edges);
    
    for(unsigned i = 0; i < target_edges.size(); i++) {
        if (target_edges[i]->getType() == ET_CALL) continue;
        // Is this correct?
        if (target_edges[i]->getType() == ET_CATCH) continue;
        
        // TODO: multiple entry functions and you?
        
        if (target_edges[i]->getTarget()) {
            out |= target_edges[i]->getTarget()->getLivenessIn();
        }
    }
    return out;
}

void image_basicBlock::summarizeBlockLivenessInfo() 
{
   if(in.size())
   {
      return;
   }

   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

   unsigned width = getFirstFunc()->img()->getObject()->getAddressWidth();

   in = registerSpace::getBitArray();
   def = in;
   use = in;

   //liveness_printf("%s[%d]: Getting liveness summary for block starting at 0x%lx\n", 
   //                FILE__, __LINE__, firstInsnOffset());


#if defined(cap_instruction_api)
   using namespace Dyninst::InstructionAPI;
   Address current = firstInsnOffset();
   InstructionDecoder decoder(reinterpret_cast<const unsigned char*>(getPtrToInstruction(firstInsnOffset())), 
                              getSize());
   Instruction::Ptr curInsn = decoder.decode();
   while(curInsn && curInsn->isValid())
   {
     ReadWriteInfo curInsnRW;
     if(!cachedLivenessInfo.getLivenessInfo(current, getFirstFunc(), curInsnRW))
     {
       curInsnRW = calcRWSets(curInsn, this, width);
       cachedLivenessInfo.insertInstructionInfo(current, curInsnRW, getFirstFunc());
     }
     use |= (curInsnRW.read & ~def);
     // And if written, then was defined
     def |= curInsnRW.written;
      
     //liveness_printf("%s[%d] After instruction %s at address 0x%lx:\n", 
     //                FILE__, __LINE__, curInsn.format().c_str(), current);
     //liveness_cerr << "        " << "?XXXXXXXXMMMMMMMMRNDITCPAZSOF11111100DSBSBDCA" << endl;
     //liveness_cerr << "        " << "?7654321076543210FTFFFFFFFFFP54321098IIPPXXXX" << endl;
     //liveness_cerr << "Read    " << curInsnRW.read << endl;
     //liveness_cerr << "Written " << curInsnRW.written << endl;
     //liveness_cerr << "Used    " << use << endl;
     //liveness_cerr << "Defined " << def << endl;

      current += curInsn->size();
      curInsn = decoder.decode();
   }
#else    
   InstrucIter ii(this);
   while(ii.hasMore()) {
     ReadWriteInfo curInsnRW;
     if(!cachedLivenessInfo.getLivenessInfo(*ii, getFirstFunc(), curInsnRW))
     {
       curInsnRW = calcRWSets(ii, this, width);
       cachedLivenessInfo.insertInstructionInfo(*ii, curInsnRW, getFirstFunc());
     }
      // If something is read, then it has been used.
      use |= (curInsnRW.read & ~def);
      // And if written, then was defined
      def |= curInsnRW.written;

	//liveness_printf("%s[%d] After instruction at address 0x%lx:\n", FILE__, __LINE__, *ii);
	//liveness_cerr << curInsnRW.read << endl << curInsnRW.written << endl << use << endl << def << endl;
        
      ++ii;
   }
#endif // (cap_instruction_api)
     //liveness_printf("%s[%d] Liveness summary for block:\n", FILE__, __LINE__);
     //liveness_cerr << in << endl << def << endl << use << endl;
     //liveness_printf("%s[%d] --------------------\n---------------------\n", FILE__, __LINE__);

   stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
   return;
}

/* This is used to do fixed point iteration until 
   the in and out don't change anymore */
bool image_basicBlock::updateBlockLivenessInfo() 
{
  bool change = false;

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

  // old_IN = IN(X)
  bitArray oldIn = in;
  // tmp is an accumulator
  bitArray out = getLivenessOut();
  
  // Liveness is a reverse dataflow algorithm
 
  // OUT(X) = UNION(IN(Y)) for all successors Y of X

  // IN(X) = USE(X) + (OUT(X) - DEF(X))
  in = use | (out - def);
  
  // if (old_IN != IN(X)) then change = true
  if (in != oldIn)
      change = true;
      
  //liveness_printf("%s[%d] Step: block 0x%lx, hasChanged %d\n", FILE__, __LINE__, firstInsnOffset(), change);
  //liveness_cerr << in << endl;

  stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

  return change;
}

// Calculate basic block summaries of liveness information
// TODO: move this to an image_func level. 

void image_func::calcBlockLevelLiveness() {
    if (livenessCalculated_) return;

    // Make sure we have parsed...
    blocks();

    // Step 1: gather the block summaries
    set<image_basicBlock*,image_basicBlock::compare>::iterator sit;
    for(sit = blockList.begin(); sit != blockList.end(); sit++) {
        (*sit)->summarizeBlockLivenessInfo();
    }
    
    // We now have block-level summaries of gen/kill info
    // within the block. Propagate this via standard fixpoint
    // calculation
    bool changed = true;
    while (changed) {
        changed = false;
        for(sit = blockList.begin(); sit != blockList.end(); sit++) {
            if ((*sit)->updateBlockLivenessInfo()) {
                changed = true;
            }
        }
    }

    livenessCalculated_ = true;
}

// This function does two things.
// First, it does a backwards iteration over instructions in its
// block to calculate its liveness.
// At the same time, we cache liveness (which was calculated) in
// any instPoints we cover. Since an iP only exists if the user
// asked for it, we take its existence to indicate that they'll
// also be instrumenting. 
void instPoint::calcLiveness() {
   // Assume that the presence of information means we
   // did this already.
   if (postLiveRegisters_.size()) {
      return;
   }
   // First, ensure that the block liveness is done.
   func()->ifunc()->calcBlockLevelLiveness();

   unsigned width = func()->ifunc()->img()->getObject()->getAddressWidth();

   if (func()->ifunc()->instLevel() == HAS_BR_INDIR)
   {
     //Unresolved indirect jumps could go anywhere.  
     //We'll be the most conservative possible in these cases, since
     //we're missing control flow.
     postLiveRegisters_ = (registerSpace::getRegisterSpace(width)->getAllRegs());
     return;
   }

   // We know: 
   //    liveness in at the block level:
   const bitArray &block_in = block()->llb()->getLivenessIn();
   //    liveness _out_ at the block level:
   const bitArray &block_out = block()->llb()->getLivenessOut();

   postLiveRegisters_ = block_out;

   assert(postLiveRegisters_.size());

   // We now want to do liveness analysis for straight-line code. 
        
   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);
#if defined(cap_instruction_api)
   using namespace Dyninst::InstructionAPI;
    
   Address blockBegin = block()->origInstance()->firstInsnAddr();
   Address blockEnd = block()->origInstance()->endAddr();
   std::vector<Address> blockAddrs;
   
   const unsigned char* insnBuffer = 
      reinterpret_cast<const unsigned char*>(block()->origInstance()->getPtrToInstruction(blockBegin));
    
   InstructionDecoder decoder(insnBuffer, block()->origInstance()->getSize());
   Address curInsnAddr = blockBegin;
   do
   {
     ReadWriteInfo rw;
     if(!block()->llb()->cachedLivenessInfo.getLivenessInfo(curInsnAddr, func()->ifunc(), rw))
     {
       Instruction::Ptr tmp = decoder.decode(insnBuffer);
       rw = calcRWSets(tmp, block()->llb(), width);
       block()->llb()->cachedLivenessInfo.insertInstructionInfo(curInsnAddr, rw, func()->ifunc());
     }
     blockAddrs.push_back(curInsnAddr);
     curInsnAddr += rw.insnSize;
     insnBuffer += rw.insnSize;
   } while(curInsnAddr < blockEnd);
    
    
   // We iterate backwards over instructions in the block. 

   std::vector<Address>::reverse_iterator current = blockAddrs.rbegin();

   //liveness_printf("%s[%d] instPoint calcLiveness: %d, 0x%lx, 0x%lx\n", 
   //                FILE__, __LINE__, current != blockAddrs.rend(), *current, addr());

   while(current != blockAddrs.rend() && *current > addr())
   {
      // Cache it in the instPoint we just covered (if such exists)
      instPoint *possiblePoint = func()->findInstPByAddr(*current);
      if (possiblePoint) {
         if (possiblePoint->postLiveRegisters_.size() == 0) {
            possiblePoint->postLiveRegisters_ = postLiveRegisters_;
         }
      }
      
      ReadWriteInfo rwAtCurrent;
      if(block()->llb()->cachedLivenessInfo.getLivenessInfo(*current, func()->ifunc(), rwAtCurrent))
      {
	//liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
	//		FILE__, __LINE__, addr(), *current);
	//liveness_cerr << "Pre: " << postLiveRegisters_ << endl;
	
	postLiveRegisters_ &= (~rwAtCurrent.written);
	postLiveRegisters_ |= rwAtCurrent.read;
	//liveness_cerr << "Post: " << postLiveRegisters_ << endl;
      
	++current;
      }
      else
      {
	Instruction::Ptr tmp = decoder.decode((const unsigned char*)(block()->origInstance()->getPtrToInstruction(*current)));
	rwAtCurrent = calcRWSets(tmp, block()->llb(), width);
	//assert(!"SERIOUS ERROR: read/write info cache state inconsistent");
	//liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
	//		FILE__, __LINE__, addr(), *current);
	//liveness_cerr << "Pre: " << postLiveRegisters_ << endl;
	
	postLiveRegisters_ &= (~rwAtCurrent.written);
	postLiveRegisters_ |= rwAtCurrent.read;
	//liveness_cerr << "Post: " << postLiveRegisters_ << endl;
	
	++current;
      }
      
   }
#else
   // We iterate backwards over instructions in the block. 

   InstrucIter ii(block());

   // set to the last instruction in the block; setCurrentAddress handles the x86
   // ii's inability to be a random-access iterator
   ii.setCurrentAddress(block()->origInstance()->lastInsnAddr());

    
   //liveness_printf("%s[%d] instPoint calcLiveness: %d, 0x%lx, 0x%lx\n", 
   //                FILE__, __LINE__, ii.hasPrev(), *ii, addr());

   while(ii.hasPrev() && (*ii > addr())) {

      // Cache it in the instPoint we just covered (if such exists)
      instPoint *possiblePoint = func()->findInstPByAddr(*ii);
      if (possiblePoint) {
         if (possiblePoint->postLiveRegisters_.size() == 0) {
            possiblePoint->postLiveRegisters_ = postLiveRegisters_;
         }
      }
      ReadWriteInfo regsAffected = calcRWSets(ii, block()->llb(), width);

      //liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
      //               FILE__, __LINE__, addr(), *ii);
      //liveness_cerr << "Pre: " << postLiveRegisters_ << endl;

      postLiveRegisters_ &= (~regsAffected.written);
      postLiveRegisters_ |= regsAffected.read;
      //liveness_cerr << "Post: " << postLiveRegisters_ << endl;

      --ii;
   }
#endif // defined(cap_instruction_api)
   stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

   assert(postLiveRegisters_.size());

   return;
}


// It'd be nice to do the calcLiveness here, but it's defined as const...
bitArray instPoint::liveRegisters(callWhen when) {
    // postLiveRegisters_ is our _output_ liveness. If the 
    // instrumentation is pre-point, we need to update it with
    // the effects of this instruction.

    unsigned width = func()->ifunc()->img()->getObject()->getAddressWidth();

    // Override: if we have an unparsed jump table in the _function_,
    // return "everything could be live".
    if (func()->ifunc()->instLevel() == HAS_BR_INDIR ||
        func()->ifunc()->instLevel() == UNINSTRUMENTABLE) {
        bitArray allOn(registerSpace::getBitArray().size());
        allOn.set();
        return allOn;
    }
        
    calcLiveness();

    if ((when == callPostInsn) ||
        (when == callBranchTargetInsn)) {
        return postLiveRegisters_;
    }
    assert(when == callPreInsn);

    bitArray ret(postLiveRegisters_);

    ReadWriteInfo curInsnRW;
    if(!block()->llb()->cachedLivenessInfo.getLivenessInfo(addr(), func()->ifunc(), curInsnRW))
    {
#if defined(cap_instruction_api)
      using namespace Dyninst::InstructionAPI;
      
      InstructionDecoder decoder;
      const unsigned char* bufferToDecode = 
      reinterpret_cast<const unsigned char*>(proc()->getPtrToInstruction(addr()));
      Instruction::Ptr currentInsn = decoder.decode(bufferToDecode);

      curInsnRW = calcRWSets(currentInsn, block()->llb(), width);

#else
    // We need to do one more step.
    // Get the current instruction iterator.
      InstrucIter ii(block());
      ii.setCurrentAddress(addr());

      curInsnRW = calcRWSets(ii, block()->llb(), width);

#endif // defined(cap_instruction_api)
    }
    
    ret &= (~curInsnRW.written);
    ret |= curInsnRW.read;

    //liveness_printf("Liveness out for instruction at 0x%lx\n",
    //                  addr());
    //liveness_cerr << "        " << "?XXXXXXXXMMMMMMMMRNDITCPAZSOF11111100DSBSBDCA" << endl;
    //liveness_cerr << "        " << "?7654321076543210FTFFFFFFFFFP54321098IIPPXXXX" << endl;
    //liveness_cerr << "Read    " << curInsnRW.read << endl;
    //liveness_cerr << "Written " << curInsnRW.written << endl;
    //liveness_cerr << "Live    " << ret << endl;

    return ret;
}


#if defined(cap_instruction_api)
ReadWriteInfo calcRWSets(Instruction::Ptr curInsn, image_basicBlock* blk, unsigned int width)
{
  ReadWriteInfo ret;
  ret.read = registerSpace::getBitArray();
  ret.written = registerSpace::getBitArray();
  ret.insnSize = curInsn->size();
  
  std::set<RegisterAST::Ptr> cur_read, cur_written;
  curInsn->getReadSet(cur_read);
  curInsn->getWriteSet(cur_written);
  //liveness_printf("Read registers: ");
      
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_read.begin(); 
       i != cur_read.end(); i++) 
  {
    //liveness_printf("%s ", (*i)->format().c_str());
    ret.read[convertRegID(IA32Regs((*i)->getID()))] = true;
  }
  //liveness_printf("\nWritten registers: ");
      
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_written.begin(); 
       i != cur_written.end(); i++) {
    //liveness_printf("%s ", (*i)->format().c_str());
    ret.written[convertRegID(IA32Regs((*i)->getID()))] = true;
  }
  //liveness_printf("\n");
  InsnCategory category = curInsn->getCategory();
  switch(category)
  {
  case c_CallInsn:
    ret.read |= (registerSpace::getRegisterSpace(width)->getCallReadRegisters());
    ret.written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
    break;
  case c_ReturnInsn:
    ret.read |= (registerSpace::getRegisterSpace(width)->getReturnReadRegisters());
    // Nothing written implicitly by a return
    break;
  case c_BranchInsn:
    if(!curInsn->allowsFallThrough() && blk->isExitBlock())
    {
      //Tail call, union of call and return
      ret.read |= ((registerSpace::getRegisterSpace(width)->getCallReadRegisters()) |
		   (registerSpace::getRegisterSpace(width)->getReturnReadRegisters()));
      ret.written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
    }
    break;
  default:
    {
      entryID cur_op = curInsn->getOperation().getID();
      if(cur_op == e_syscall)
      {
	ret.read |= (registerSpace::getRegisterSpace(width)->getSyscallReadRegisters());
	ret.written |= (registerSpace::getRegisterSpace(width)->getSyscallWrittenRegisters());
      }
    }
    break;
  }
  return ret;
}

#else // POWER

ReadWriteInfo calcRWSets(InstrucIter ii, image_basicBlock* blk, unsigned int width)
{
  ReadWriteInfo ret;
  ret.read = registerSpace::getBitArray();
  ret.written = registerSpace::getBitArray();
  std::set<Register> tmpRead;
  std::set<Register> tmpWritten;
  ii.getAllRegistersUsedAndDefined(tmpRead, tmpWritten);
  
  for (std::set<Register>::const_iterator i = tmpRead.begin(); 
       i != tmpRead.end(); i++) {
    ret.read[*i] = true;
  }
  for (std::set<Register>::const_iterator i = tmpWritten.begin(); 
       i != tmpWritten.end(); i++) {
    ret.written[*i] = true;
  }
  
  // TODO "If trusting the ABI..."
  // Otherwise we should go interprocedural
  if (ii.isACallInstruction()) {
    ret.read |= (registerSpace::getRegisterSpace(width)->getCallReadRegisters());
    ret.written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
  }
  if (ii.isAReturnInstruction()) {
    ret.read |= (registerSpace::getRegisterSpace(width)->getReturnReadRegisters());
    // Nothing written implicitly by a return
  }
  if (ii.isAJumpInstruction() && blk->isExitBlock())
  {
    //Tail call, union of call and return
    ret.read |= ((registerSpace::getRegisterSpace(width)->getCallReadRegisters()) |
	     (registerSpace::getRegisterSpace(width)->getReturnReadRegisters()));
    ret.written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
  }
  if (ii.isSyscall()) {
    ret.read |= (registerSpace::getRegisterSpace(width)->getSyscallReadRegisters());
    ret.written |= (registerSpace::getRegisterSpace(width)->getSyscallWrittenRegisters());
  }
  return ret;
}

#endif // cap_instruction_api
#endif // cap_liveness
