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

#include "ppc_process.h"
#include "common/src/arch-power.h"
#include <string.h>
#include <iostream>
#include "unaligned_memory_access.h"

using namespace NS_power;
using namespace std;

ppc_process::ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                         std::vector<std::string> envp, std::map<int, int> f) :
   int_process(p, e, a, envp, f)
{
}

ppc_process::ppc_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p)
{
}

ppc_process::~ppc_process()
{
}

unsigned ppc_process::plat_breakpointSize()
{
  return 4;
}

void ppc_process::plat_breakpointBytes(unsigned char *buffer)
{
   write_memory_as(buffer, uint32_t{0x7d821008});
   // MJMTODO = Assumes host and target architecture match (and algnment)
}

bool ppc_process::plat_breakpointAdvancesPC() const
{
   return false;
}

static bool atomicLoad(const instruction &insn) {
    return (    (XFORM_OP(insn) == LXop)
             && (XFORM_XO(insn) == LWARXxop) );
}

static bool atomicStore(const instruction &insn) {
    return (    (XFORM_OP(insn) == STXop)
             && (XFORM_XO(insn) == STWCXxop) );
}

static void clear_ss_state_cb(int_thread *thr) {
   ppc_process *proc = dynamic_cast<ppc_process *>(thr->llproc());
   proc->cleanupSSOnContinue(thr);
}

void ppc_process::cleanupSSOnContinue(int_thread *thr)
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

void ppc_process::registerSSClearCB()
{
   static bool registered_ss_clear_cb = false;
   if (registered_ss_clear_cb)
      return;
   int_thread::addContinueCB(clear_ss_state_cb);
   registered_ss_clear_cb = true;
}

async_ret_t ppc_process::readPCForSS(int_thread *thr, Address &pc)
{
   ppc_thread *ppc_thr = dynamic_cast<ppc_thread *>(thr);
   assert(ppc_thr);
   if (ppc_thr->haveCachedPC(pc))
      return aret_success;

   if (!plat_needsAsyncIO())
   {
      //Fast-track for linu/ppc
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

async_ret_t ppc_process::readInsnForSS(Address pc, int_thread *, unsigned int &rawInsn)
{
   if (!plat_needsAsyncIO())
   {
      //Fast-track for linux/ppc
      mem_response::ptr new_resp = mem_response::createMemResponse((char *) &rawInsn, sizeof(unsigned int));
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
      if (pc >= i->first && pc+4 <= i->first + i->second->getSize()) {
         if (!i->second->isReady()) {
            pthrd_printf("Returning async form memory read while doing emulated single step test\n");
            return aret_async;
         }
         if (i->second->hasError()) {
            pthrd_printf("Error during async read of memory during emulated single step test\n");
            return aret_error;
         }
         Offset offset = pc - i->first;
         memcpy(&rawInsn, i->second->getBuffer() + offset, sizeof(unsigned int));
         return aret_success;
      }
   }

   unsigned int read_size = plat_getRecommendedReadSize();

   //Don't read over page boundarys.  Could cause problems if reading from
   // last executable page.
   unsigned int page_size = getTargetPageSize();
   unsigned int page_offset = pc % page_size;
   if (page_offset + read_size > page_size) {
      read_size = page_size - page_offset;
   }
   assert(read_size >= sizeof(unsigned int));

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
   memcpy(&rawInsn, new_resp->getBuffer(), sizeof(unsigned int));
   return aret_success;
}

async_ret_t ppc_process::plat_needsEmulatedSingleStep(int_thread *thr, vector<Address> &addrResult) {
   assert(thr->singleStep());

   pthrd_printf("Checking for atomic instruction sequence before single step\n");

   /*
    * We need an emulated single step to single step an atomic
    * instruction sequence. The sequence looks something like this:
    *
    * lwarx
    * ...
    * stwcx.
    * <breapoint>
    *
    * We need to set the breakpoint at the instruction immediately after
    * stwcx.
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
        unsigned int rawInsn;
        aresult = readInsnForSS(pc, thr, rawInsn);
        if (aresult == aret_error || aresult == aret_async)
           return aresult;

        // Decode the current instruction
        instruction insn(rawInsn);
        if( atomicLoad(insn) ) {
            sequenceStarted = true;
            pthrd_printf("Found the start of an atomic instruction sequence at 0x%lx\n", pc);
        }else{
            if( !sequenceStarted ) break;
        }

        if( atomicStore(insn) && sequenceStarted ) {
            foundEnd = true;
        }

        // For control flow instructions, assume target is outside atomic instruction sequence
        // and place breakpoint there as well
        Address cfTarget = insn.getTarget(pc);
        if( cfTarget != 0 && sequenceStarted && !foundEnd ) {
            addrResult.push_back(cfTarget);
        }

        currentCount++;
        pc += 4;
    }while( !foundEnd && currentCount < maxSequenceCount );

    // The breakpoint should be set at the instruction following the sequence
    if( foundEnd ) {
        addrResult.push_back(pc);
        pthrd_printf("Atomic instruction sequence ends at 0x%lx\n", pc);
    }else if( sequenceStarted || addrResult.size() ) {
        addrResult.clear();
        pthrd_printf("Failed to find end of atomic instruction sequence\n");
        return aret_error;
    }else{
        pthrd_printf("No atomic instruction sequence found, safe to single step\n");
    }

    return aret_success;
}

void ppc_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr> resps)
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

bool ppc_process::plat_convertToBreakpointAddress(Address &, int_thread *) {
   return true;
}

bool ppc_process::plat_needsPCSaveBeforeSingleStep()
{
   return true;
}

ppc_thread::ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l), have_cached_pc(false), cached_pc(0)
{
}

ppc_thread::~ppc_thread()
{
}

bool ppc_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &)
{
   return false;
}

bool ppc_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &)
{
   return false;
}

unsigned ppc_thread::hwBPAvail(unsigned)
{
   return 0;
}

EventBreakpoint::ptr ppc_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal)
{
   return EventBreakpoint::ptr();
}

bool ppc_thread::bpNeedsClear(hw_breakpoint *)
{
   return false;
}

void ppc_thread::setCachedPC(Address pc)
{
   cached_pc = pc;
   have_cached_pc = true;
}

void ppc_thread::clearCachedPC()
{
   have_cached_pc = false;
}

bool ppc_thread::haveCachedPC(Address &pc)
{
   if (!have_cached_pc)
      return false;
   pc = cached_pc;
   return true;
}
