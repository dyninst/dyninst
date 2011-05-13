/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: liveness.C,v 1.12 2008/09/04 21:06:20 bill Exp $

#if defined(cap_liveness)

#include "debug.h"
#include "parse-cfg.h"
#include "function.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "debug.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/Instruction.h"
#include "addressSpace.h"
using namespace Dyninst::InstructionAPI;
#include "symtab.h"

#if defined(arch_x86) || defined(arch_x86_64)
// Special-casing for IA-32...
#include "RegisterConversion.h"
#include "inst-x86.h"
#endif

#include "Parsing.h"
using namespace Dyninst::ParseAPI;

ReadWriteInfo calcRWSets(Instruction::Ptr insn, parse_block* blk, unsigned width, Address a);
InstructionCache parse_block::cachedLivenessInfo = InstructionCache();

  


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
      
      for (unsigned i = 0; i < realRegisters_.size(); ++i) {
         if (liveRegs[realRegisters_[i]->number])
	    realRegisters_[i]->liveState = registerSlot::live;
         else
	    realRegisters_[i]->liveState = registerSlot::dead;
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
      {
         i.currval()->liveState = registerSlot::dead;
      }
   }
}

const bitArray &parse_block::getLivenessIn(parse_func * context) {
    // Calculate if it hasn't been done already
    if (in.size() == 0)
        summarizeBlockLivenessInfo(context);
    assert(in.size());
    return in;
}

const bitArray parse_block::getLivenessOut(parse_func * context) {
    bitArray out(in.size());
    assert(out.size());
    // ignore call, return edges
    Intraproc epred;

    // OUT(X) = UNION(IN(Y)) for all successors Y of X
    Block::edgelist & target_edges = targets();
    Block::edgelist::iterator eit = target_edges.begin(&epred);

    liveness_cerr << "getLivenessOut for block [" << hex << start() << "," << end() << "]" << dec << endl;
   
    for( ; eit != target_edges.end(); ++eit) { 
        // covered by Intraproc predicate
        //if ((*eit)->type() == CALL) continue;
        // Is this correct?
        if ((*eit)->type() == CATCH) continue;
        
        // TODO: multiple entry functions and you?
        out |= ((parse_block*)(*eit)->trg())->getLivenessIn(context);
	liveness_cerr << "Accumulating from block " << hex << ((parse_block*)(*eit)->trg())->start() << dec << endl;
	liveness_cerr << out << endl;
    }
    
    liveness_cerr << " Returning liveness for out " << endl;
#if defined(arch_x86) || defined(arch_x86_64)
    liveness_cerr << "  ?XXXXXXXXMMMMMMMMRNDITCPAZSOF11111100DSBSBDCA" << endl;
    liveness_cerr << "  ?7654321076543210FTFFFFFFFFFP54321098IIPPXXXX" << endl;
#elif defined(arch_power)
    liveness_cerr << "  001101000000000000000000000000001010033222222222211111111110000000000" << endl;
    liveness_cerr << "  001101000000000000000000000000001010010987654321098765432109876543210" << endl;
#endif
    liveness_cerr << "  " << out << endl;


    return out;
}

void parse_block::summarizeBlockLivenessInfo(parse_func *context) 
{
   if(in.size())
   {
      return;
   }

   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

   unsigned width = region()->getAddressWidth();

   in = registerSpace::getBitArray();
   def = in;
   use = in;

   liveness_printf("%s[%d]: Getting liveness summary for block starting at 0x%lx in %s\n",
                   FILE__, __LINE__, firstInsnOffset(), context->img()->pathname().c_str());


   using namespace Dyninst::InstructionAPI;
   Address current = firstInsnOffset();
   InstructionDecoder decoder(
                       reinterpret_cast<const unsigned char*>(getPtrToInstruction(firstInsnOffset())),
                       getSize(),
                       obj()->cs()->getArch());
   Instruction::Ptr curInsn = decoder.decode();
   while(curInsn)
   {
     ReadWriteInfo curInsnRW;
     liveness_printf("%s[%d] After instruction %s at address 0x%lx:\n",
                     FILE__, __LINE__, curInsn->format().c_str(), current);
     if(!cachedLivenessInfo.getLivenessInfo(current, context, curInsnRW))
     {
       curInsnRW = calcRWSets(curInsn, this, width, current);
       cachedLivenessInfo.insertInstructionInfo(current, curInsnRW, context);
     }

     use |= (curInsnRW.read & ~def);
     // And if written, then was defined
     def |= curInsnRW.written;
      
     liveness_printf("%s[%d] After instruction at address 0x%lx:\n",
                     FILE__, __LINE__, current);
#if defined(arch_power)
     liveness_cerr << "        " << "000000000000000000000000000000000000033222222222211111111110000000000" << endl;
     liveness_cerr << "        " << "000000000000000000000000000000000000010987654321098765432109876543210" << endl;
#endif
     liveness_cerr << "Read    " << curInsnRW.read << endl;
     liveness_cerr << "Written " << curInsnRW.written << endl;
     liveness_cerr << "Used    " << use << endl;
     liveness_cerr << "Defined " << def << endl;

      current += curInsn->size();
      curInsn = decoder.decode();
   }
     liveness_printf("%s[%d] Liveness summary for block:\n", FILE__, __LINE__);
     liveness_cerr << in << endl << def << endl << use << endl;
     liveness_printf("%s[%d] --------------------\n---------------------\n", FILE__, __LINE__);

   stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
   return;
}

/* This is used to do fixed point iteration until 
   the in and out don't change anymore */
bool parse_block::updateBlockLivenessInfo(parse_func * context) 
{
  bool change = false;

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

  // old_IN = IN(X)
  bitArray oldIn = in;
  // tmp is an accumulator
  bitArray out = getLivenessOut(context);
  
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
// TODO: move this to an parse_func level. 

void parse_func::calcBlockLevelLiveness() {
    if (livenessCalculated_) return;

    // Step 1: gather the block summaries
    Function::blocklist::iterator sit = blocks().begin();
    for( ; sit != blocks().end(); sit++) {
        ((parse_block*)(*sit))->summarizeBlockLivenessInfo(this);
    }
    
    // We now have block-level summaries of gen/kill info
    // within the block. Propagate this via standard fixpoint
    // calculation
    bool changed = true;
    while (changed) {
        changed = false;
        for(sit = blocks().begin(); sit != blocks().end(); sit++) {
            if (((parse_block*)(*sit))->updateBlockLivenessInfo(this)) {
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
   if (liveRegs_.size()) {
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
      liveRegs_ = (registerSpace::getRegisterSpace(width)->getAllRegs());
      return;
   }

   // This gets interesting with the new point definition, as "where" we are
   // really depends on the type of point. We're going to use an address as
   // the great equalizer.

   Address addr;
   // For "pre"-instruction we subtract one from the address. This is done
   // because liveness is calculated backwards; therefore, accumulating
   // up to <addr> is actually the liveness _after_ that instruction, not
   // before. Since we compare using > below, -1 means it will trigger. 

   switch(type()) {
      // First, don't be dumb if we're looking at (effectively) an initial
      // instruction of a CFG element.
      case FuncEntry:
         liveRegs_ = func()->entryBlock()->llb()->getLivenessIn(func()->ifunc());
         return;
      case BlockEntry:
         liveRegs_ = block()->llb()->getLivenessIn(func()->ifunc());
         return;
      case Edge:
         liveRegs_ = edge()->trg()->llb()->getLivenessIn(func()->ifunc());
         return;
      case FuncExit:
         // It would be great to use getLivenessOut, but it doesn't work
         // because we rely on the _return instruction_ to do liveness
         // calcs. Instead, we assign addr_ to ->last(). 
         // ... and subtract 1 so that we get pre-insn liveness.
         addr = block()->last() - 1;
         break;
      case PostCall:
         liveRegs_ = block()->llb()->getLivenessOut(func()->ifunc());
         return;

      case PreInsn:
         if (addr_ == block()->start()) {
            liveRegs_ = block()->llb()->getLivenessIn(func()->ifunc());
            return;
         }
         else addr = addr_ - 1;
         break;
      case PostInsn:
         if (addr_ == block()->last()) {
            liveRegs_ = block()->llb()->getLivenessOut(func()->ifunc());
            return;
         }
         else {
            addr = addr_;
         }
         break;
      case PreCall:
         addr = block()->last() - 1;
         break;
      default:
         assert(0);
         break;
   }

   // We know: 
   //    liveness _out_ at the block level:
   bitArray working = block()->llb()->getLivenessOut(func()->ifunc());
   assert(!working.empty());

   // We now want to do liveness analysis for straight-line code. 
        
   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);
   using namespace Dyninst::InstructionAPI;
    
   Address blockBegin = block()->start();
   Address blockEnd = block()->end();
   std::vector<Address> blockAddrs;
   
   const unsigned char* insnBuffer = 
      reinterpret_cast<const unsigned char*>(block()->getPtrToInstruction(blockBegin));
   assert(insnBuffer);

   InstructionDecoder decoder(insnBuffer,block()->size(),
        func()->ifunc()->isrc()->getArch());
   Address curInsnAddr = blockBegin;
   do
   {
     ReadWriteInfo rw;
     if(!block()->llb()->cachedLivenessInfo.getLivenessInfo(curInsnAddr, func()->ifunc(), rw))
     {
        Instruction::Ptr tmp = decoder.decode(insnBuffer);
        rw = calcRWSets(tmp, block()->llb(), width, curInsnAddr);
        block()->llb()->cachedLivenessInfo.insertInstructionInfo(curInsnAddr, rw, func()->ifunc());
     }
     blockAddrs.push_back(curInsnAddr);
     curInsnAddr += rw.insnSize;
     insnBuffer += rw.insnSize;
   } while(curInsnAddr < blockEnd);
    
    
   // We iterate backwards over instructions in the block, as liveness is 
   // a backwards flow process.

   std::vector<Address>::reverse_iterator current = blockAddrs.rbegin();

   liveness_printf("%s[%d] instPoint calcLiveness: %d, 0x%lx, 0x%lx\n", 
                   FILE__, __LINE__, current != blockAddrs.rend(), *current, addr);

   while(current != blockAddrs.rend() && *current > addr)
   {
      ReadWriteInfo rwAtCurrent;
      if(!block()->llb()->cachedLivenessInfo.getLivenessInfo(*current, func()->ifunc(), rwAtCurrent))
         assert(0);

      liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
                      FILE__, __LINE__, addr, *current);
      //liveness_cerr << "        " << "?XXXXXXXXMMMMMMMMRNDITCPAZSOF11111100DSBSBDCA" << endl;
      //liveness_cerr << "        " << "?7654321076543210FTFFFFFFFFFP54321098IIPPXXXX" << endl;
#if defined(arch_power)
      liveness_cerr << "        " << "000000000000000000000000000000000000033222222222211111111110000000000" << endl;
      liveness_cerr << "        " << "000000000000000000000000000000000000010987654321098765432109876543210" << endl;
#endif
      liveness_cerr << "Pre:    " << working << endl;
      
      working &= (~rwAtCurrent.written);
      working |= rwAtCurrent.read;
      liveness_cerr << "Post:   " << working << endl;
      
      ++current;
   }
   assert(!working.empty());

   liveRegs_ = working;
   stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

   return;
}


// It'd be nice to do the calcLiveness here, but it's defined as const...
bitArray instPoint::liveRegisters() {
   calcLiveness();
   return liveRegs_;
}

#if defined(arch_power)
int convertRegID(int in)
{
    if(in >= ppc32::r0 && in <= ppc32::r31)
    {
      return in - ppc32::r0 + registerSpace::r0;
    }
    else if(in >= ppc32::fpr0 && in <= ppc32::fpr31)
    {
        return in - ppc32::fpr0 + registerSpace::fpr0;
    }
/*    else if(in >= ppc32::fsr0 && in <= ppc32::fsr31)
    {
        return in - ppc32::fsr0 + registerSpace::fsr0;
    }
    */    else if(in == ppc32::xer)
    {
        return registerSpace::xer;
    }
    else if(in == ppc32::lr)
    {
        return registerSpace::lr;
    }
    else if(in == ppc32::mq)
    {
        return registerSpace::mq;
    }
    else if(in == ppc32::ctr)
    {
        return registerSpace::ctr;
    }
    else if(in >= ppc32::cr0 && in <= ppc32::cr7)
    {
        return registerSpace::cr;
    }
    else if(in >= ppc64::r0 && in <= ppc64::r31) {
      return in - ppc64::r0 + registerSpace::r0;
    }
    else if(in >= ppc64::fpr0 && in <= ppc64::fpr31)
    {
        return in - ppc64::fpr0 + registerSpace::fpr0;
    }
/*    else if(in >= ppc64::fsr0 && in <= ppc64::fsr31)
    {
        return in - ppc64::fsr0 + registerSpace::fsr0;
    }
    */    else if(in == ppc64::xer)
    {
        return registerSpace::xer;
    }
    else if(in == ppc64::lr)
    {
        return registerSpace::lr;
    }
    else if(in == ppc64::mq)
    {
        return registerSpace::mq;
    }
    else if(in == ppc64::ctr)
    {
        return registerSpace::ctr;
    }
    else if(in >= ppc64::cr0 && in <= ppc64::cr7)
    {
        return registerSpace::cr;
    }
    else
    {
        return registerSpace::ignored;
    }
}

#endif

ReadWriteInfo calcRWSets(Instruction::Ptr curInsn, parse_block* blk, unsigned int width,
                        Address a)
{
  liveness_cerr << "calcRWSets for " << curInsn->format() << " @ " << hex << a << dec << endl;
  ReadWriteInfo ret;
  ret.read = registerSpace::getBitArray();
  ret.written = registerSpace::getBitArray();
  ret.insnSize = curInsn->size();
  
  std::set<RegisterAST::Ptr> cur_read, cur_written;
  curInsn->getReadSet(cur_read);
  curInsn->getWriteSet(cur_written);
    liveness_printf("Read registers: \n");
      
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_read.begin(); 
       i != cur_read.end(); i++) 
  {
    liveness_printf("\t%s \n", (*i)->format().c_str());
#if defined(arch_x86) || defined(arch_x86_64)
        bool unused;
        Register converted = convertRegID(*i, unused);
        if(converted != REGNUM_EFLAGS)
        {
            ret.read[converted] = true;
        }
        else
        {
            ret.read[REGNUM_OF] = true;
            ret.read[REGNUM_CF] = true;
            ret.read[REGNUM_PF] = true;
            ret.read[REGNUM_AF] = true;
            ret.read[REGNUM_ZF] = true;
            ret.read[REGNUM_SF] = true;
            ret.read[REGNUM_DF] = true;
            ret.read[REGNUM_TF] = true;
            ret.read[REGNUM_NT] = true;
        }
#else
    int id = convertRegID((*i)->getID());
    if(id != registerSpace::ignored)
    {
        assert(id < registerSpace::lastReg && id >= registerSpace::r0);
        ret.read[id] = true;
    }
#endif
  }
  //liveness_printf("Written registers: \n");
      
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_written.begin(); 
       i != cur_written.end(); i++) {
    //liveness_printf("%s \n", (*i)->format().c_str());
#if defined(arch_x86) || defined(arch_x86_64)
    bool treatAsRead = false;
    Register r = convertRegID(*i, treatAsRead);
    if(r != REGNUM_EFLAGS)
    {
        ret.written[r] = true;
        if(treatAsRead) ret.read[r] = true;
    }
    else
    {
        ret.written[REGNUM_OF] = true;
        ret.written[REGNUM_CF] = true;
        ret.written[REGNUM_PF] = true;
        ret.written[REGNUM_AF] = true;
        ret.written[REGNUM_ZF] = true;
        ret.written[REGNUM_SF] = true;
        ret.written[REGNUM_DF] = true;
        ret.written[REGNUM_TF] = true;
        ret.written[REGNUM_NT] = true;
    }
        
#else
    
    int id = convertRegID((*i)->getID());
    
    if(id != registerSpace::ignored)
    {
        assert(id < registerSpace::lastReg && id >= registerSpace::r0);
        ret.written[id] = true;
    }
#endif
  }
  InsnCategory category = curInsn->getCategory();
  switch(category)
  {
  case c_CallInsn:
      // Call instructions not at the end of a block are thunks, which are not ABI-compliant.
      // So make conservative assumptions about what they may read (ABI) but don't assume they write anything.
      ret.read |= (registerSpace::getRegisterSpace(width)->getCallReadRegisters());
      if(blk->end() == a)
      {
          ret.written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
      }
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
      bool isInterrupt = false;
      bool isSyscall = false;


      if ((curInsn->getOperation().getID() == e_int) ||
	  (curInsn->getOperation().getID() == e_int3)) {
	isInterrupt = true;
      }
      static RegisterAST::Ptr gs(new RegisterAST(x86::gs));
      if (((curInsn->getOperation().getID() == e_call) &&
	   /*(curInsn()->getOperation().isRead(gs))) ||*/
	   (curInsn->getOperand(0).format(curInsn->getArch()) == "16")) ||
	  (curInsn->getOperation().getID() == e_syscall) || 
	  (curInsn->getOperation().getID() == e_int) || 
	  (curInsn->getOperation().getID() == power_op_sc)) {
	isSyscall = true;
      }

      if (curInsn->getOperation().getID() == power_op_svcs) {
	isSyscall = true;
      }

      if (isInterrupt || isSyscall) {
	ret.read |= (registerSpace::getRegisterSpace(width)->getSyscallReadRegisters());
	ret.written |= (registerSpace::getRegisterSpace(width)->getSyscallWrittenRegisters());
      }
    }
    break;
  }
  
  return ret;
}

#endif // cap_liveness
