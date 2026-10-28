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

// $Id: liveness.C,v 1.12 2008/09/04 21:06:20 bill Exp $

#include "debug_dataflow.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/Location.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/Instruction.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "dataflowAPI/h/liveness.h"
#include "registers/abi.h"
#include <boost/bind/bind.hpp>
#include <functional>
#include <map>
#include <set>
#include "instructionAPI/h/syscalls.h"
#include "instructionAPI/h/interrupts.h"


using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

// Code for register liveness detection

// The architecture analyzed when only an address width is given: the one
// Dyninst generates code for.
static Architecture codegenArch(int width) {
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    (void)width;
    return Arch_amdgpu_gfx908;
#elif defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
    return (width == 4) ? Arch_x86 : Arch_x86_64;
#elif defined(DYNINST_CODEGEN_ARCH_POWER)
    (void)width;
    return Arch_ppc64;
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
    (void)width;
    return Arch_aarch64;
#elif defined(DYNINST_CODEGEN_ARCH_RISCV64)
    (void)width;
    return Arch_riscv64;
#else
    (void)width;
    return Arch_none;
#endif
}

LivenessAnalyzer::LivenessAnalyzer(int width): errorno((ErrorType)-1) {
    abi = &ABI::get(codegenArch(width));
}

LivenessAnalyzer::LivenessAnalyzer(Architecture arch, int): errorno((ErrorType)-1) {
    abi = &ABI::get(arch);
}

int LivenessAnalyzer::getIndex(MachRegister machReg){
   return abi->getIndex(machReg);
}

const LivenessSet& LivenessAnalyzer::getLivenessIn(Block *block) {
    // Calculate if it hasn't been done already
    liveness_cerr << endl << "LivenessAnalyzer::getLivenessIn()" << endl;
    liveness_cerr << "Getting liveness for block " << hex << block->start() << dec << endl;
    assert(blockLiveInfo.find(block) != blockLiveInfo.end());
    livenessData& data = blockLiveInfo[block];
    assert(data.in.getABI());
    return data.in;
}

void LivenessAnalyzer::processEdgeLiveness(Edge* e, livenessData& data, Block* block,
					   const LivenessSet& allRegsDefined)
{
  // covered by Intraproc predicate
  //if ((*eit)->type() == CALL) continue;
  // Is this correct?
  if (e->type() == CATCH) return;
  if (e->sinkEdge()) {
    liveness_cerr << "Sink edge from " << hex << block->start() << dec << endl;
    data.out |= allRegsDefined;
    return;
  }
  
  // TODO: multiple entry functions and you?
  data.out |= getLivenessIn(e->trg());
  liveness_cerr << "Accumulating from block " << hex << (e->trg())->start() << dec << endl;
  liveness_cerr << data.out << endl;
}


const LivenessSet& LivenessAnalyzer::getLivenessOut(Block *block, LivenessSet &allRegsDefined) {
       
	assert(blockLiveInfo.find(block) != blockLiveInfo.end());
	livenessData &data = blockLiveInfo[block];
	data.out = abi->makeEmpty();
	// ignore call, return edges
	Intraproc epred;
	

    // OUT(X) = UNION(IN(Y)) for all successors Y of X
    boost::lock_guard<Block> g(*block);
    const Block::edgelist & target_edges = block -> targets();

    liveness_cerr << "getLivenessOut for block [" << hex << block->start() << "," << block->end() << "]" << dec << endl;
    
    std::for_each(boost::make_filter_iterator(epred, target_edges.begin(), target_edges.end()),
		  boost::make_filter_iterator(epred, target_edges.end(), target_edges.end()),
		  boost::bind(&LivenessAnalyzer::processEdgeLiveness, 
			      this, 
				  boost::placeholders::_1,
			      boost::ref(data), 
			      block,
			      boost::ref(allRegsDefined)));
    
    liveness_cerr << " Returning liveness for out " << endl;
    liveness_cerr << "  " << data.out << endl;


    return data.out;
}

void LivenessAnalyzer::summarizeBlockLivenessInfo(Function* func, Block *block, LivenessSet &allRegsDefined) 
{
   if (blockLiveInfo.find(block) != blockLiveInfo.end()){
   	return;
   }
   liveness_printf("\tsummarize block info at block %lx\n", block->start());
 
   livenessData &data = blockLiveInfo[block];
   data.use = data.def = data.in = abi->makeEmpty();

   using namespace Dyninst::InstructionAPI;
   Address current = block->start();
   InstructionDecoder decoder(
                       reinterpret_cast<const unsigned char*>(getPtrToInstruction(block, block->start())),		     
                       block->size(),
                       block->obj()->cs()->getArch());
   Instruction curInsn = decoder.decode();
   while(curInsn.isValid()) {
     ReadWriteInfo curInsnRW;
     liveness_printf("%s[%d] After instruction %s at address 0x%lx:\n",
                     FILE__, __LINE__, curInsn.format().c_str(), current);
     if(!cachedLivenessInfo.getLivenessInfo(current, func, curInsnRW))
     {
       curInsnRW = calcRWSets(curInsn, block, current);
       cachedLivenessInfo.insertInstructionInfo(current, curInsnRW, func);
     }

     data.use |= (curInsnRW.read - data.def);
     // And if written, then was defined
     data.def |= curInsnRW.written;
      
     liveness_printf("%s[%d] After instruction at address 0x%lx:\n",
                     FILE__, __LINE__, current);
     liveness_cerr << "Read    " << curInsnRW.read << endl;
     liveness_cerr << "Written " << curInsnRW.written << endl;
     liveness_cerr << "Used    " << data.use << endl;
     liveness_cerr << "Defined " << data.def << endl;

      current += curInsn.size();
      curInsn = decoder.decode();
   }

   liveness_printf("%s[%d] Liveness summary for block:\n", FILE__, __LINE__);
   liveness_cerr << "Used " << data.in << endl;
   liveness_cerr << "Def  " << data.def << endl;
   liveness_cerr << "Use  " << data.use << endl;
   liveness_printf("%s[%d] --------------------\n---------------------\n", FILE__, __LINE__);

   allRegsDefined |= data.def;

   return;
}

/* This is used to do fixed point iteration until 
   the in and out don't change anymore */
bool LivenessAnalyzer::updateBlockLivenessInfo(Block* block, LivenessSet &allRegsDefined) 
{
  bool change = false;
  livenessData &data = blockLiveInfo[block];

  // old_IN = IN(X)
  LivenessSet oldIn = data.in;
  // tmp is an accumulator
  getLivenessOut(block, allRegsDefined);
  
  // Liveness is a reverse dataflow algorithm
 
  // OUT(X) = UNION(IN(Y)) for all successors Y of X

  // IN(X) = USE(X) + (OUT(X) - DEF(X))
  liveness_cerr << "Updating block info for block " << hex << block->start() << dec << endl;
  liveness_cerr << "Out: " << data.out << endl;
  liveness_cerr << "Def: " << data.def << endl;
  liveness_cerr << "Use: " << data.use << endl;
  data.in = data.use | (data.out - data.def);
  liveness_cerr << "In:  " << data.in << endl;
  
  // if (old_IN != IN(X)) then change = true
  if (data.in != oldIn)
      change = true;
      
  return change;
}

// Calculate basic block summaries of liveness information

void LivenessAnalyzer::analyze(Function *func) {
    if (liveFuncCalculated.find(func) != liveFuncCalculated.end()) return;
    liveness_printf("Caculate basic block level liveness information for function %s (%lx)\n", func->name().c_str(), func->addr());

    // Step 0: initialize the "registers this function has defined" bitarray
    assert(funcRegsDefined.find(func) == funcRegsDefined.end());
    // Let's assume the regs that are normally live at the entry to a function
    // are the regs a call can read.
    funcRegsDefined[func] = abi->getCallReadRegisters();
    LivenessSet &regsDefined = funcRegsDefined[func];

    // blocks() takes a recursive_mutex per call, so evaluate it once
    Function::blocklist blocks = func->blocks();

    // Step 1: gather the block summaries
    for (Block *block : blocks) {
       summarizeBlockLivenessInfo(func, block, regsDefined);
    }

    // Step 2: propagate the block summaries to a fixpoint. Liveness is a
    // backward analysis, so a change to IN(b) requires revisiting only b's
    // predecessors. Pending blocks are visited highest address first, which
    // visits most successors first because fall-through edges always ascend.
    //
    // This function's blocks by start address. A predecessor not found here
    // belongs to another function and is not updated.
    std::map<Address, Block*> blocksByAddr;
    std::set<Address, std::greater<Address>> worklist;
    for (Block *block : blocks) {
        blocksByAddr.emplace(block->start(), block);
        worklist.insert(block->start());
    }

    Intraproc epred;
    while (!worklist.empty()) {
        Block *block = blocksByAddr.at(*worklist.begin());
        worklist.erase(worklist.begin());
        if (!updateBlockLivenessInfo(block, regsDefined)) continue;

        Block::edgelist sources;
        block->copy_sources(sources);
        for (Edge *e : sources) {
            // Skip edges that getLivenessOut does not read IN(block) through:
            // those rejected by Intraproc, and CATCH edges, which
            // processEdgeLiveness ignores. This filter must never be stricter
            // than getLivenessOut's, or a predecessor would miss the change.
            if (!epred(e) || e->type() == CATCH) continue;
            // Skip a predecessor outside this function: it may have no summary
            // from step 1. Comparing the block, not only its address, also
            // rejects a block of another code region at the same address.
            auto it = blocksByAddr.find(e->src()->start());
            if (it == blocksByAddr.end() || it->second != e->src()) continue;
            // No-op when the predecessor is already pending
            worklist.insert(it->first);
        }
    }

    liveFuncCalculated[func] = true;
}


// This function does two things.
// First, it does a backwards iteration over instructions in its
// block to calculate its liveness.
// At the same time, we cache liveness (which was calculated) in
// any instPoints we cover. Since an iP only exists if the user
// asked for it, we take its existence to indicate that they'll
// also be instrumenting. 
bool LivenessAnalyzer::query(Location loc, Type type, LivenessSet &live) {
//TODO: consider the trustness of the location 

   if (!loc.isValid()){
   	errorno = Invalid_Location;
	return false;
   }

   // First, ensure that the block liveness is done.
   analyze(loc.func);

   Address addr = 0;
   // For "pre"-instruction we subtract one from the address. This is done
   // because liveness is calculated backwards; therefore, accumulating
   // up to <addr> is actually the liveness _after_ that instruction, not
   // before. Since we compare using > below, -1 means it will trigger. 

   switch(loc.type) {
      // First, don't be dumb if we're looking at (effectively) an initial
      // instruction of a CFG element.
      case Location::function_:
      	 if (type == Before){
	 	live = getLivenessIn(loc.func->entry());
		return true;
	 }
	 assert(0);
      case Location::block_:

      case Location::blockInstance_:
         
	 if (type == Before) {
	 	live = getLivenessIn(loc.block);
		return true;
	 }
	 addr = loc.block->lastInsnAddr()-1;
	 break;

      case Location::instruction_:

      case Location::instructionInstance_:

         if (type == Before) {
	 	if (loc.offset == loc.block->start()) {
			live = getLivenessIn(loc.block);
			return true;
		}
		addr = loc.offset - 1;
	 }
	 if (type == After) {
	 	if (loc.offset == loc.block->lastInsnAddr()) {
                   live = blockLiveInfo[loc.block].out;
                   return true;
		}
	 	addr = loc.offset;
	}
	 break;

      case Location::edge_:
         live = getLivenessIn(loc.edge->trg());
	 return true;
      case Location::entry_:
      	 if (type == Before) {
	 	live = getLivenessIn(loc.block);
		return true;
	 }
	 assert(0);
      case Location::call_:
	 if (type == Before) addr = loc.block->lastInsnAddr()-1;
	 if (type == After) {
            live = blockLiveInfo[loc.block].out;
            return true;
	 }
	 break;
      case Location::exit_:
      	 if (type == After){
	 	addr = loc.block->lastInsnAddr()-1;
		break;
	 }
	 assert(0);

      default:
         assert(0);
	 
  }
	
   // We know: 
   //    liveness _out_ at the block level:
   LivenessSet working = blockLiveInfo[loc.block].out;
   assert(working.getABI());

   // We now want to do liveness analysis for straight-line code. 
        
   using namespace Dyninst::InstructionAPI;
    
   Address blockBegin = loc.block->start();
   Address blockEnd = loc.block->end();
   std::vector<Address> blockAddrs;
   
   const unsigned char* insnBuffer = 
      reinterpret_cast<const unsigned char*>(getPtrToInstruction(loc.block, blockBegin));
   assert(insnBuffer);

   InstructionDecoder decoder(insnBuffer,loc.block->size(),
        loc.func->isrc()->getArch());
   Address curInsnAddr = blockBegin;
   do
   {
     ReadWriteInfo rw;
     if(!cachedLivenessInfo.getLivenessInfo(curInsnAddr, loc.func, rw))
     {
        Instruction tmp = decoder.decode(insnBuffer);
        rw = calcRWSets(tmp, loc.block, curInsnAddr);
        cachedLivenessInfo.insertInstructionInfo(curInsnAddr, rw, loc.func);
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
      if(!cachedLivenessInfo.getLivenessInfo(*current, loc.func, rwAtCurrent))
         assert(0);

      liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
                      FILE__, __LINE__, addr, *current);
      liveness_cerr << "Pre:    " << working << endl;
      working -= rwAtCurrent.written;
      working |= rwAtCurrent.read;
      liveness_cerr << "Post:   " << working << endl;
      liveness_cerr << "Current read:  " << rwAtCurrent.read << endl;
      liveness_cerr << "Current Write: " << rwAtCurrent.written << endl;
      
      ++current;
   }
   live = working;
   return true;
}

bool LivenessAnalyzer::query(Location loc, Type type, bitArray &bitarray) {
	LivenessSet liveRegs;
	if (!query(loc, type, liveRegs)) return false;
	bitarray = liveRegs.bits();
	return true;
}

bool LivenessAnalyzer::query(Location loc, Type type, const MachRegister& machReg, bool &live){
	LivenessSet liveRegs;
	if (query(loc, type, liveRegs)){
        assert(getIndex(machReg) >= 0);
		live = liveRegs.contains(machReg);
		return true;
	}
	return false;

}


static bool isMMX(MachRegister reg) {
  auto const arch = reg.getArchitecture();
  if (arch == Arch_x86) return reg.regClass() == x86::MMX;
  if (arch == Arch_x86_64) return reg.regClass() == x86_64::MMX;
  return false;
}

ReadWriteInfo LivenessAnalyzer::calcRWSets(Instruction curInsn, Block *blk, Address a)
{

  liveness_cerr << "calcRWSets for " << curInsn.format() << " @ " << hex << a << dec << endl;
  ReadWriteInfo ret;
  ret.read = abi->makeEmpty();
  ret.written = abi->makeEmpty();
  ret.insnSize = curInsn.size();
  std::set<RegisterAST::Ptr> cur_read, cur_written;
  curInsn.getReadSet(cur_read);
  curInsn.getWriteSet(cur_written);
    liveness_printf("Read registers: \n");
  
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_read.begin(); 
       i != cur_read.end(); i++) 
  {
    MachRegister cur = (*i)->getID();
    liveness_printf("\t%s \n", cur.name().c_str());
    ret.read.insert(cur);
  }
  liveness_printf("Write Registers: \n"); 
  for (std::set<RegisterAST::Ptr>::const_iterator i = cur_written.begin(); 
       i != cur_written.end(); i++) {  
    MachRegister cur = (*i)->getID();
    liveness_printf("\t%s \n", cur.name().c_str());
    // insert() maps cur to its bit itself. Taking getBaseRegister() first
    // would be wrong on ppc64, where the decoder's ppc64::fpr<N> has base
    // vsr<32+N>, which has no bit in the ppc32 layout.
    ret.written.insert(cur);
    MachRegister const base = abi->canonicalize(cur);
    // A partial write keeps the rest of the register, so it is also a read:
    // a write of under 4 bytes, or of an MMX register, which is 64 of the 80
    // bits of its x87 register. The flags are tracked as a whole, so writing
    // one flag is not.
    bool const isFlags = (base == x86::flags || base == x86_64::flags);
    if (!isFlags && cur != base && (cur.size() < 4 || isMMX(cur))) ret.read.insert(base);
  }
  if(curInsn.isCall()) {
      // Call instructions not at the end of a block are thunks, which are not ABI-compliant.
      // So make conservative assumptions about what they may read (ABI) but don't assume they write anything.
      ret.read |= (abi->getCallReadRegisters());
      if(blk->lastInsnAddr() == a)
      {
          ret.written |= (abi->getCallWrittenRegisters());
      }
  }
  else if(curInsn.isReturn()) {
    ret.read |= (abi->getReturnReadRegisters());
    // Nothing written implicitly by a return
  }
  else if(curInsn.isBranch()) {
    if(!curInsn.allowsFallThrough() && isExitBlock(blk))
    {
      //Tail call, union of call and return
      ret.read |= ((abi->getCallReadRegisters()) | (abi->getReturnReadRegisters()));
      ret.written |= (abi->getCallWrittenRegisters());
    }
  }
  else {
    if (curInsn.isInterrupt() || curInsn.isSyscall()) {
      ret.read |= (abi->getSyscallReadRegisters());
      ret.written |= (abi->getSyscallWrittenRegisters());
    }
  }
  return ret;
}

void *LivenessAnalyzer::getPtrToInstruction(Block *block, Address addr) const{

	if (addr < block->start()) return NULL;
	if (addr > block->end()) return NULL;
	return block->region()->getPtrToInstruction(addr);
}
bool LivenessAnalyzer::isExitBlock(Block *block)
{
    boost::lock_guard<Block> g(*block);
    const Block::edgelist & trgs = block->targets();

    bool interprocEdge = false;
    bool intraprocEdge = false;
    for (Block::edgelist::const_iterator eit=trgs.begin(); eit != trgs.end(); ++eit){
        if ((*eit)->type() == CATCH) continue;
	if ((*eit)->interproc()) interprocEdge = true; else intraprocEdge = true;
    }

    if (interprocEdge && !intraprocEdge) return true; else return false;
}

void LivenessAnalyzer::clean(){

	blockLiveInfo.clear();
	liveFuncCalculated.clear();
	cachedLivenessInfo.clean();
}

void LivenessAnalyzer::clean(Function *func){

	if (liveFuncCalculated.find(func) != liveFuncCalculated.end()){		
		liveFuncCalculated.erase(func);
		Function::blocklist::iterator sit = func->blocks().begin();
		for( ; sit != func->blocks().end(); sit++) {
			blockLiveInfo.erase(*sit);
		}

	}
	if (cachedLivenessInfo.getCurFunc() == func) cachedLivenessInfo.clean();

}

