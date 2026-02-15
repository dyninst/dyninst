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

#include <string.h>
#include <iostream>
#include "instructionAPI/h/InstructionDecoder.h"
#include "proccontrol/src/riscv_process.h"
#include "common/src/arch-riscv64.h"
#include "registers/riscv64_regs.h"
#include "registers/abstract_regs.h"

using namespace NS_riscv64;
using namespace std;

namespace {
  MachRegister as_system_register(unsigned int regNum) {
    switch(regNum) {
      case 0: return Dyninst::riscv64::x0;
      case 1: return Dyninst::riscv64::x1;
      case 2: return Dyninst::riscv64::x2;
      case 3: return Dyninst::riscv64::x3;
      case 4: return Dyninst::riscv64::x4;
      case 5: return Dyninst::riscv64::x5;
      case 6: return Dyninst::riscv64::x6;
      case 7: return Dyninst::riscv64::x7;
      case 8: return Dyninst::riscv64::x8;
      case 9: return Dyninst::riscv64::x9;
      case 10: return Dyninst::riscv64::x10;
      case 11: return Dyninst::riscv64::x11;
      case 12: return Dyninst::riscv64::x12;
      case 13: return Dyninst::riscv64::x13;
      case 14: return Dyninst::riscv64::x14;
      case 15: return Dyninst::riscv64::x15;
      case 16: return Dyninst::riscv64::x16;
      case 17: return Dyninst::riscv64::x17;
      case 18: return Dyninst::riscv64::x18;
      case 19: return Dyninst::riscv64::x19;
      case 20: return Dyninst::riscv64::x20;
      case 21: return Dyninst::riscv64::x21;
      case 22: return Dyninst::riscv64::x22;
      case 23: return Dyninst::riscv64::x23;
      case 24: return Dyninst::riscv64::x24;
      case 25: return Dyninst::riscv64::x25;
      case 26: return Dyninst::riscv64::x26;
      case 27: return Dyninst::riscv64::x27;
      case 28: return Dyninst::riscv64::x28;
      case 29: return Dyninst::riscv64::x29;
      case 30: return Dyninst::riscv64::x30;
      case 31: return Dyninst::riscv64::x31;
      case 32: return Dyninst::riscv64::f0;
      case 33: return Dyninst::riscv64::f1;
      case 34: return Dyninst::riscv64::f2;
      case 35: return Dyninst::riscv64::f3;
      case 36: return Dyninst::riscv64::f4;
      case 37: return Dyninst::riscv64::f5;
      case 38: return Dyninst::riscv64::f6;
      case 39: return Dyninst::riscv64::f7;
      case 40: return Dyninst::riscv64::f8;
      case 41: return Dyninst::riscv64::f9;
      case 42: return Dyninst::riscv64::f10;
      case 43: return Dyninst::riscv64::f11;
      case 44: return Dyninst::riscv64::f12;
      case 45: return Dyninst::riscv64::f13;
      case 46: return Dyninst::riscv64::f14;
      case 47: return Dyninst::riscv64::f15;
      case 48: return Dyninst::riscv64::f16;
      case 49: return Dyninst::riscv64::f17;
      case 50: return Dyninst::riscv64::f18;
      case 51: return Dyninst::riscv64::f19;
      case 52: return Dyninst::riscv64::f20;
      case 53: return Dyninst::riscv64::f21;
      case 54: return Dyninst::riscv64::f22;
      case 55: return Dyninst::riscv64::f23;
      case 56: return Dyninst::riscv64::f24;
      case 57: return Dyninst::riscv64::f25;
      case 58: return Dyninst::riscv64::f26;
      case 59: return Dyninst::riscv64::f27;
      case 60: return Dyninst::riscv64::f28;
      case 61: return Dyninst::riscv64::f29;
      case 62: return Dyninst::riscv64::f30;
      case 63: return Dyninst::riscv64::f31;
      case 64: return Dyninst::riscv64::pc;
    }
    return Dyninst::InvalidReg;
  }
}

//constructors, blank functions
riscv_process::riscv_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                         std::vector<std::string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f)
{
}

riscv_process::riscv_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p)
{
}

riscv_process::~riscv_process()
{
}

unsigned riscv_process::plat_breakpointSize()
{
  //This size is the number of bytes of one
  //trap instruction. In riscv64, this is EBREAK
  //which stands for breakpoint, with a normal
  //length of 32bits == 4bytes.
  //In compressed mode, the instruction is C.EBREAK and
  //is 16bits == 2bytes.
  #if defined(__riscv_compressed)
    return 2;
  #else
    return 4;
  #endif
}

void riscv_process::plat_breakpointBytes(unsigned char *buffer)
{
  //memory oppucation:
  //high---low addr
  //[3] [2] [1] [0]
  //this is a EBREAK instruction in riscv64 for uncompressed
  //and a C.EBREAK instruction for compressed
  //compressed instruction is 2 bytes, uncompressed is 4 bytes
  #if defined(__riscv_compressed)
      buffer[0] = 0x02;
      buffer[1] = 0x90;
  #else
      buffer[0] = 0x73;
      buffer[1] = 0x00;
      buffer[2] = 0x10;
      buffer[3] = 0x00;
   #endif
}

bool riscv_process::plat_breakpointAdvancesPC() const
{
   //as doc says, riscv will return to the interrupted intruction
   return false;
}


bool riscv_process::plat_convertToBreakpointAddress(Address &, int_thread *) {
   return true;
}

// The following a few of functions are used for emulated
// singlestep. The scenario is quite similiar to what happens
// on RISC-V.
// ----
// LD
// ...
// ST
// ----
//

// to check if this insn is exclusive load/store
static bool atomicLoad(const instruction &insn) {
    return insn.isAtomicLoad();
}

static bool atomicStore(const instruction &insn) {
    return insn.isAtomicStore();
}

static void clear_ss_state_cb(int_thread *thr) {
   riscv_process *proc = dynamic_cast<riscv_process *>(thr->llproc());
   proc->cleanupSSOnContinue(thr);
}

void riscv_process::cleanupSSOnContinue(int_thread *thr)
{
   //Clear the PC for this thread
   map<int_thread *, reg_response::ptr>::iterator i = pcs_for_ss.find(thr);
   if (i == pcs_for_ss.end())
      return;
   pcs_for_ss.erase(i);

   if (!pcs_for_ss.empty()) {
      //We'll only clear the memory cache if every PC is done SS'ing
      return;
   }

   //Clear the memory cache
   map<Address, mem_response::ptr>::iterator j;
   for (j = mem_for_ss.begin(); j != mem_for_ss.end(); j++) {
      assert(j->second->isReady() || j->second->hasError());
      free(j->second->getBuffer());
   }
   mem_for_ss.clear();
}

void riscv_process::registerSSClearCB()
{
   static bool registered_ss_clear_cb = false;
   if (registered_ss_clear_cb)
      return;
   int_thread::addContinueCB(clear_ss_state_cb);
   registered_ss_clear_cb = true;
}

async_ret_t riscv_process::readPCForSS(int_thread *thr, Address &pc)
{
   riscv_thread *riscv_thr = dynamic_cast<riscv_thread *>(thr);
   assert(riscv_thr);
   if (riscv_thr->haveCachedPC(pc))
      return aret_success;

   if (!plat_needsAsyncIO())
   {
      //Fast-track for linux/riscv
      reg_response::ptr pcResponse = reg_response::createRegResponse();
      bool result = thr->getRegister(MachRegister::getPC(getTargetArch()), pcResponse);
      if (!result || pcResponse->hasError()) {
         pthrd_printf("Error reading PC address to check for emulated single step condition\n");
         return aret_error;
      }
      bool ready = pcResponse->isReady();
      assert(ready);
      if(!ready) return aret_error;
      pc = (Address) pcResponse->getResult();
      return aret_success;
   }

   //Store and cache the PC value on AsyncIO platforms
   registerSSClearCB();

   bool result = true;
   reg_response::ptr resp;
   map<int_thread *, reg_response::ptr>::iterator i = pcs_for_ss.find(thr);
   if (i == pcs_for_ss.end()) {
      resp = reg_response::createRegResponse();
      pcs_for_ss[thr] = resp;
      result = thr->getRegister(MachRegister::getPC(getTargetArch()), resp);
   }
   else {
      resp = i->second;
   }

   if (!result || resp->hasError()) {
      pthrd_printf("Error reading PC address to check for emulated single step condition\n");
      return aret_error;
   }
   if (!resp->isReady()) {
      pthrd_printf("Async return while getting PC for emulated single step test\n");
      return aret_async;
   }
   pc = (Address) resp->getResult();
   return aret_success;
}

async_ret_t riscv_process::readInsnForSS(Address pc, int_thread *, max_instruction_t &rawInsn)
{
   if (!plat_needsAsyncIO())
   {
      //Fast-track for linux/riscv
      mem_response::ptr new_resp = mem_response::createMemResponse((char *) &rawInsn, sizeof(max_instruction_t));
      bool result = readMem(pc, new_resp);
      if (!result || new_resp->hasError()) {
         pthrd_printf("Error during memory read for pc\n");
         return aret_error;
      }
      bool ready = new_resp->isReady();
      assert(ready);
      if(!ready) return aret_error;
      return aret_success;
   }

   /**
    * Use larger reads and cache memory buffers when on an asyncIO system
    **/
   map<Address, mem_response::ptr>::iterator i;
   for (i = mem_for_ss.begin(); i != mem_for_ss.end(); i++) {
      if (pc >= i->first && pc+sizeof(max_instruction_t) <= i->first + i->second->getSize()) {
         if (!i->second->isReady()) {
            pthrd_printf("Returning async form memory read while doing emulated single step test\n");
            return aret_async;
         }
         if (i->second->hasError()) {
            pthrd_printf("Error during async read of memory during emulated single step test\n");
            return aret_error;
         }
         Offset offset = pc - i->first;
         memcpy(&rawInsn, i->second->getBuffer() + offset, sizeof(max_instruction_t));
         return aret_success;
      }
   }

   unsigned int read_size = plat_getRecommendedReadSize();

   // Don't read over page boundarys.  Could cause problems if reading from
   // last executable page.
   unsigned int page_size = getTargetPageSize();
   unsigned int page_offset = pc % page_size;
   if (page_offset + read_size > page_size) {
      read_size = page_size - page_offset;
   }
   assert(read_size >= sizeof(max_instruction_t));

   char *buffer = (char *) malloc(read_size);
   mem_response::ptr new_resp = mem_response::createMemResponse(buffer, read_size);
   bool result = readMem(pc, new_resp);
   mem_for_ss[pc] = new_resp;
   if (!result || new_resp->hasError()) {
      pthrd_printf("Error during async read of memory during emulated single step test\n");
      return aret_error;
   }
   if (!new_resp->isReady()) {
      pthrd_printf("Returning async from memory read during single step test\n");
      return aret_async;
   }
   memcpy(&rawInsn, new_resp->getBuffer(), sizeof(max_instruction_t));
   return aret_success;
}

async_ret_t riscv_process::plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Address> &addrResult) {
   assert(thr->singleStep());

   pthrd_printf("Checking for atomic instruction sequence before single step\n");

   /*
    * On RISC-V we need to emulate single step in every case because ptrace SINGLESTEP
    * it's still not implemented in the kernel.
    * We also need an emulated single step to single step an atomic
    * instruction sequence. The sequence looks something like this:
    *
    * ld
    * ...
    * st.
    * <breapoint>
    *
    * We need to set the breakpoint at the instruction immediately after
    * st.
    */
   Address pc;
   async_ret_t aresult = readPCForSS(thr, pc);
   if (aresult == aret_error || aresult == aret_async)
      return aresult;

    // Check if the next instruction is a lwarx
    // If it is, scan forward until the terminating stwcx.
    bool foundEnd = false;
    bool sequenceStarted = false;
    int maxSequenceCount = 24; // arbitrary
    int currentCount = 0;
    do {
        // Read the current instruction
        max_instruction_t rawInsn;
        aresult = readInsnForSS(pc, thr, rawInsn);
        if (aresult == aret_error || aresult == aret_async)
           return aresult;

        bool isCompressed = (rawInsn & 3) != 3;

        instruction insn(&rawInsn, isCompressed);

        Dyninst::Address nextInsnAddr = pc + insn.size();
        
        if( atomicLoad(insn) ) {
            sequenceStarted = true;
            pthrd_printf("Found the start of an atomic instruction sequence at 0x%lx\n", pc);
        }

        if( atomicStore(insn) && sequenceStarted ) {
            foundEnd = true;
            addrResult.push_back(nextInsnAddr);
        }

        // For control flow instructions, assume target is outside atomic instruction sequence
        // and place breakpoint there as well
        // First check if is branch reg instruction.

        if (!sequenceStarted) {
         Address cfTarget;
         if( insn.isBranchReg() ){
             pthrd_printf("DEBUG: find Branch Reg instruction, going to retrieve the target address.\n");
             // get reg value from the target proc
 
             unsigned regNum = insn.getTargetReg();

             pthrd_printf("DEBUG: find Branch Reg instruction, target reg is %u\n", regNum);
 
             reg_response::ptr Response = reg_response::createRegResponse();
             bool result = thr->getRegister(as_system_register(regNum), Response);
             if (!result || Response->hasError()) {
                pthrd_printf("Error reading PC address to check for emulated single step condition\n");
                return aret_error;
             }
             bool ready = Response->isReady();
             assert(ready);
             if(!ready) return aret_error;
 
             Address regValue = (Address) Response->getResult();

             if (insn.isBranchOffset()) {
               signed long offset = insn.getBranchOffset();
               cfTarget = regValue + offset;
               pthrd_printf("DEBUG: found Branch Reg instruction with offset Imm, target address is 0x%lx + %ld = 0x%lx\n", regValue, offset, cfTarget);
             } else {
               cfTarget = regValue;
               pthrd_printf("DEBUG: found Branch Reg instruction, target address is 0x%lx\n", cfTarget);
             }
         }else{ // return target address by calculating the offset and pc
             cfTarget = insn.getTarget(pc);
             pthrd_printf("DEBUG: found Branch Imm instruction, getTarget is 0x%lx\n", cfTarget);
         }
         if (cfTarget == 0) {
            cfTarget = nextInsnAddr;
            pthrd_printf("DEBUG: No branch or branch 0, target address is next instruction: 0x%lx\n", cfTarget);
         }
         addrResult.push_back(cfTarget);

         // If the instruction is a conditional branch,
         // we need to set the breakpoint at the next instruction in case
         // the branch won't be taken.
         if (insn.isCondBranch() && cfTarget != nextInsnAddr) {
            pthrd_printf("DEBUG: Branch is conditional, set breakpoint also at the next instruction: 0x%lx\n", nextInsnAddr);
            addrResult.push_back(nextInsnAddr);
         }
         break;
        }

        currentCount++;
        pc += insn.size();
    }while( !foundEnd && currentCount < maxSequenceCount );

    // The breakpoint should be set at the instruction following the sequence
    if( sequenceStarted && !foundEnd ) {
        addrResult.clear();
        pthrd_printf("Failed to find end of atomic instruction sequence\n");
        return aret_error;
    }

    return aret_success;
}

void riscv_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr> resps)
{
   map<int_thread *, reg_response::ptr>::iterator i;
   for (i = pcs_for_ss.begin(); i != pcs_for_ss.end(); i++) {
      if (!i->second->isReady()) {
         resps.insert(i->second);
      }
   }
   map<Address, mem_response::ptr>::iterator j;
   for (j = mem_for_ss.begin(); j != mem_for_ss.end(); j++) {
      if (!j->second->isReady()) {
         resps.insert(j->second);
      }
   }
}

// ------------------------------------
// riscv thread functions implementations

riscv_thread::riscv_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l), have_cached_pc(false), cached_pc(0)
{
}

riscv_thread::~riscv_thread()
{
}

//#warning "HWBreakpoint is not supported now."
bool riscv_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &)
{
    return false;
}

bool riscv_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &)
{
    return false;
}

unsigned riscv_thread::hwBPAvail(unsigned)
{
    return 0;
}

EventBreakpoint::ptr riscv_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal)
{
    return EventBreakpoint::ptr();
}

bool riscv_thread::bpNeedsClear(hw_breakpoint *)
{
	assert(0); //not implemented
    return false;
}

void riscv_thread::setCachedPC(Address pc)
{
    //what is cached PC?
    cached_pc = pc;
    have_cached_pc = true;
}

void riscv_thread::clearCachedPC()
{
    have_cached_pc = false;
}

bool riscv_thread::haveCachedPC(Address &pc)
{
    if (!have_cached_pc)
        return false;
    pc = cached_pc;
    return true;
}
