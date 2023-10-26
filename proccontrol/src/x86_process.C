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

#include "x86_process.h"
#include "int_event.h"
#include "Event.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

x86_process::x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f) :
  int_process(p, e, a, envp, f)
{
}

x86_process::x86_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p)
{
}

x86_process::~x86_process()
{
}

unsigned x86_process::plat_breakpointSize()
{
  return 1;
}

void x86_process::plat_breakpointBytes(unsigned char *buffer)
{
  buffer[0] = 0xcc;
}

bool x86_process::plat_breakpointAdvancesPC() const
{
   return true;
}

x86_thread::x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   dr7_val(0)
{
   for (int i=0; i<max_dr_regs; i++) {
      active[i] = NULL;
   }
}

x86_thread::~x86_thread()
{
}

#define PERM_X 1 << 0
#define PERM_W 1 << 1
#define PERM_R 1 << 2
unsigned int x86_thread::spaceNeeded(unsigned int perms)
{
   unsigned int needed = 0;
   if (perms & PERM_X) needed++;
   if ((perms & PERM_R) || (perms & PERM_W)) needed++;
   return needed;
}

unsigned int x86_thread::spaceAvail()
{
   unsigned int avail = 0;
   for (int i=0; i<max_dr_regs; i++) {
      if (!active[i]) avail++;
   }
   return avail;
}

int x86_thread::getAvailDR()
{
   for (int i=0; i<max_dr_regs; i++)
      if (!active[i]) return i;
   return -1;
}

int x86_thread::getSlotForHBP(hw_breakpoint *hwbp, int last_slot)
{
   for (int i=last_slot; i<max_dr_regs; i++)
      if (active[i] == hwbp) return i;
   return -1;
}

bool x86_thread::rmHWBreakpoint(hw_breakpoint *bp,
                                bool suspend,
                                std::set<response::ptr> &resps,
                                bool &done)
{
   done = false;
   unsigned addr_width = llproc()->getAddressWidth();

   MachRegisterVal new_dr7 = dr7_val;
   for (int i=0; i<max_dr_regs; i++) {
      if (active[i] != bp)
         continue;
      //Unset bits 0,2,4 or 6 to disable
      new_dr7 &= ~((unsigned long) (1 << i*2));

      //Unset size and type bits
      new_dr7 &= ~(0xfUL << (16+4*i));

      if (!suspend)
         active[i] = NULL;
   }

   if (new_dr7 != dr7_val) {
      MachRegister reg = (addr_width == 8 ? x86_64::dr7 : x86::dr7);
      result_response::ptr resp = result_response::createResultResponse();
      pthrd_printf("Setting register %s to %lx for HWBP\n",
                   reg.name().c_str(), new_dr7);
      bool result = setRegister(reg, new_dr7, resp);
      if (!result) {
         pthrd_printf("Error setting %s in thread %d\n", reg.name().c_str(),
                      lwp);
         return false;
      }
      dr7_val = new_dr7;
      resps.insert(resp);
   }

   done = true;
   return true;
}

bool x86_thread::addHWBreakpoint(hw_breakpoint *bp,
                                 bool resume,
                                 std::set<response::ptr> &resps,
                                 bool &done)
{
   if (!resume && spaceNeeded(bp->getPerms()) > spaceAvail()) {
      perr_printf("No space for hardware BP on %d: %u > %u\n", lwp,
                  spaceNeeded(bp->getPerms()), spaceAvail());
      setLastError(err_bpfull, "Not enough space for hardware breakpoint\n");
      return false;
   }

   done = false;
   unsigned int size = bp->getSize();
   unsigned int perms = bp->getPerms();
   Address addr = bp->getAddr();
   unsigned addr_width = llproc()->getAddressWidth();
   int last_slot = 0;

   MachRegisterVal new_dr7 = dr7_val;
   for (unsigned i=0; i<3; i++) {
      unsigned int perm_code, size_code = 0;
      //Figure out dr7 permissions code for this attempt:
      // 0 = X
      // 1 = W
      // 2 = unimplemented in processor
      // 3 = RW
      if (i == 0 && (perms & PERM_X)) {
         pthrd_printf("Installing execution HWBP into %d/%d at %lx\n",
                      llproc()->getPid(), lwp, addr);
         perm_code = 0;
      }
      else if (i == 1 && (perms & PERM_W) && !(perms & PERM_R)) {
         pthrd_printf("Installing write HWBP into %d/%d at %lx\n",
                      llproc()->getPid(), lwp, addr);
         perm_code = 1;
      }
      else if (i == 2 && (perms & PERM_R)) {
         pthrd_printf("Installing read/write HWBP into %d/%d at %lx\n",
                      llproc()->getPid(), lwp, addr);
         perm_code = 3;
      }
      else
         continue;

      //Figure out the size code:
      // 1 byte or PERM_X = 0
      // 2 bytes = 1
      // 4 bytes = 3
      // 8 bytes = 2
      if (size == 1 || !perm_code)
         size_code = 0;
      else if (size == 2)
         size_code = 1;
      else if (size == 4)
         size_code = 3;
      else if (size == 8)
         size_code = 2;
      else assert(0);

      //Get a slot
      int cur;
      if (resume) {
         cur = getSlotForHBP(bp, last_slot);
         last_slot = cur+1;
      }
      else
         cur = getAvailDR();
      assert(cur != -1);
      active[cur] = bp;

      //Enable local hw breakpoint in bits 0,2,4 or 6
      new_dr7 |= 1 << (cur * 2);

      //Set bits 16-17, 20-21, 24-25, or 28-29 to permissions
      unsigned int offset = cur*4+16;
      new_dr7 &= ~((unsigned long) (3 << offset));
      new_dr7 |= perm_code << offset;

      //Set bits 18-19, 22-23, 26-27, or 30-31 to size
      offset = cur*4+18;
      new_dr7 &= ~((unsigned long) (3U << offset));
      new_dr7 |= size_code << offset;

      //Set bit 8 to on (exact match required)
      new_dr7 |= 1<<8;

      if (resume) {
         //Skip the dr0..dr3 set if we're just resuming an old breakpoint.
         // It's already set.
//         continue;
      }

      //Set dr0, dr1, dr2 or dr3 to the BP address
      MachRegister reg;
      if (addr_width == 8) {
         switch (cur) {
            case 0: reg = x86_64::dr0; break;
            case 1: reg = x86_64::dr1; break;
            case 2: reg = x86_64::dr2; break;
            case 3: reg = x86_64::dr3; break;
            default: assert(0);
         }
      }
      else {
         switch (cur) {
            case 0: reg = x86::dr0; break;
            case 1: reg = x86::dr1; break;
            case 2: reg = x86::dr2; break;
            case 3: reg = x86::dr3; break;
            default: assert(0);
         }
      }

      pthrd_printf("Setting register %s to %lx for HWBP\n",
                   reg.name().c_str(), addr);
      result_response::ptr resp = result_response::createResultResponse();
      resp->markSyncHandled();
      bool result = setRegister(reg, addr, resp);
      if (!result) {
         pthrd_printf("Error setting %s in thread %d\n", reg.name().c_str(),
                      lwp);
         return false;
      }
      resps.insert(resp);
   }

   if (new_dr7 != dr7_val) {
      MachRegister reg = (addr_width == 8 ? x86_64::dr7 : x86::dr7);
      result_response::ptr resp = result_response::createResultResponse();
      resp->markSyncHandled();
      pthrd_printf("Setting register %s to %lx for HWBP\n",
                   reg.name().c_str(), new_dr7);
      bool result = setRegister(reg, new_dr7, resp);
      if (!result) {
         pthrd_printf("Error setting %s in thread %d\n", reg.name().c_str(),
                      lwp);
         return false;
      }
      dr7_val = new_dr7;
      resps.insert(resp);
   }
   done = true;
   return true;
}

EventBreakpoint::ptr x86_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool have_reg,
                                                    Dyninst::MachRegisterVal regval)
{
   bool have_active_hw_breakpoint = false;
   x86_process *proc = dynamic_cast<x86_process *>(llproc());
   for (int i = 0; i < max_dr_regs; i++) {
      if (active[i]) {
         have_active_hw_breakpoint = true;
         break;
      }
   }
   if (!have_active_hw_breakpoint) {
      return EventBreakpoint::ptr();
   }

   MachRegister reg = proc->getAddressWidth() == 4 ? x86::dr6 : x86_64::dr6;
   if (!have_reg) {
      reg_response::ptr resp = reg_response::createRegResponse();
      bool result = getRegister(reg, resp);
      if (result)
         result = int_process::waitForAsyncEvent(resp);
      if (!result || resp->hasError()) {
         pthrd_printf("Failed to read dr6 for HW breakpoint decode on %d/%d\n",
                      proc->getPid(), getLWP());
         return EventBreakpoint::ptr();
      }
      regval = resp->getResult();
      pthrd_printf("Read dr6 to determine if event is HW breakpoint. Value is %lx\n", regval);
   }

   unsigned bps_triggered = regval & 0xf;
   if (!bps_triggered) {
      pthrd_printf("dr6 does not have any low bits set, not a HW breakpoint\n");
      return EventBreakpoint::ptr();
   }

   EventBreakpoint::ptr result;
   for (int i=0; i<max_dr_regs; i++) {
      if (!(bps_triggered & (1 << i)))
         continue;
      pthrd_printf("Decoding to HW breakpoint register dr%d on %d/%d\n", i, proc->getPid(), getLWP());
      hw_breakpoint *hwbp = active[i];
      assert(hwbp);

      int_eventBreakpoint *ievbp = new int_eventBreakpoint(hwbp, this);
      EventBreakpoint::ptr evbp = EventBreakpoint::ptr(new EventBreakpoint(ievbp));

      if (!result) {
         result = evbp;
      }
      else {
         evbp->setProcess(proc->proc());
         evbp->setThread(thread());
         evbp->setSyncType(Event::sync_thread);
         result->addSubservientEvent(evbp);
      }
   }

   MachRegisterVal new_dr6 = regval & ~0xfUL;
   if (new_dr6 != regval) {
      result_response::ptr result_resp = result_response::createResultResponse();
      bool r = setRegister(reg, new_dr6, result_resp);
      if (r)
         int_process::waitForAsyncEvent(result_resp);
      if (!r || result_resp->hasError()) {
         pthrd_printf("Error while resetting dr6 register on %d/%d\n", proc->getPid(), getLWP());
         return EventBreakpoint::ptr();
      }
   }

   return result;
}

unsigned x86_thread::hwBPAvail(unsigned mode)
{
   return spaceAvail() / spaceNeeded(mode);
}

bool x86_thread::bpNeedsClear(hw_breakpoint *hwbp)
{
   return (hwbp->getPerms() & PERM_X);
}
