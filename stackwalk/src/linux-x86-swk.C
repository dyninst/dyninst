/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/linux-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/x86-swk.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(arch_x86_64)

static int computeAddrWidth(int pid)
{
   /**
    * It's surprisingly difficult to figure out the word size of a process
    * without looking at the files it loads (we want to avoid disk accesses).
    *
    * /proc/PID/auxv offers a hackish opportunity to do this.  auxv contains
    * a list of name value pairs.  On 64 bit processes these name values are
    * a uint64/uint64 combo and on 32 bit processes they're uint32/uint32.
    *
    * The names are from a set of small integers (ranging from 0 to 37 at
    * the time of this writing).  Since these are small numbers, the top half
    * of name word will be 0x0 on 64 bit processes.  On 32-bit process this
    * word will contain a value, of which some should be non-zero.  
    *
    * We'll thus check ever word that is 1 mod 4.  If all are 0x0 we assume we're
    * looking at a 64-bit process.
    **/
   uint32_t buffer[256];
   char auxv_name[64];
   
   snprintf(auxv_name, 64, "/proc/%d/auxv", pid);
   int fd = open(auxv_name, O_RDONLY);
   if (fd == -1) { 
      sw_printf("[%s:%u] - Couldn't open %s to determine address width: %s",
                __FILE__, __LINE__, auxv_name, strerror(errno));      
      return -1;
   }

   long int result = read(fd, buffer, sizeof(buffer));
   long int words_read = result / sizeof(uint32_t);
   int word_size = 8;
   for (unsigned i=1; i<words_read; i+= 4)
   {
      if (buffer[i] != 0) {
         word_size = 4;
         break;
      }
   }
   close(fd);
   return word_size;
}

unsigned ProcDebugLinux::getAddressWidth()
{
   if (cached_addr_width)
      return cached_addr_width;

   sw_printf("[%s:%u] - Computing address size of proc %d\n",
             __FILE__, __LINE__, pid);
   
   int addr_width = computeAddrWidth(pid);
   if (addr_width == -1)
   {
      sw_printf("[%s:%u] - **ERROR** Failed to calculate address width, guessing 8\n",
                __FILE__, __LINE__);
      cached_addr_width = 8;
   }
   else
      cached_addr_width = addr_width;
   
   return cached_addr_width;
}

#else

unsigned ProcDebugLinux::getAddressWidth() 
{
   return sizeof(void*);
}

#endif

#if defined(arch_x86_64)

struct user32_regs_struct
{
  int ebx;
  int ecx;
  int edx;
  int esi;
  int edi;
  int ebp;
  int eax;
  int xds;
  int xes;
  int xfs;
  int xgs;
  int orig_eax;
  int eip;
  int xcs;
  int eflags;
  int esp;
  int xss;
};

struct user32 {
   struct user32_regs_struct regs;
   //Rest don't matter to StackwalkerAPI
};

#define USER64_OFFSET_OF(register) ((signed)(long) &(((struct user *) NULL)->regs.register))
#define USER32_OFFSET_OF(register) ((signed)(long) &(((struct user32 *) NULL)->regs.register))

#else

#define USER32_OFFSET_OF(register) ((signed int) &(((struct user *) NULL)->regs.register))

#endif

bool ProcDebugLinux::getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID t, 
                                 Dyninst::MachRegisterVal &val)
{
   long result;
   signed offset = -1;
   
   if (getAddressWidth() == 4)
   {
      switch (reg) {
         case Dyninst::MachRegPC:
            offset = USER32_OFFSET_OF(eip);
            break;
         case Dyninst::MachRegStackBase:
         case Dyninst::ESP:
            offset = USER32_OFFSET_OF(esp);
            break;
         case Dyninst::MachRegFrameBase:
         case Dyninst::EBP:
            offset = USER32_OFFSET_OF(ebp);
            break;
   }
   }
#if defined(arch_x86_64)
   else 
   {
      switch (reg) {
         case Dyninst::MachRegPC:
            offset = USER64_OFFSET_OF(rip);
         break;
         case Dyninst::MachRegStackBase:
         case Dyninst::RSP:
            offset = USER64_OFFSET_OF(rsp);
         break;
         case Dyninst::MachRegFrameBase:
         case Dyninst::RBP:
            offset = USER64_OFFSET_OF(rbp);
         break;
      }
   }
#endif
   if (offset == -1) {
         sw_printf("[%s:%u] - Request for unsupported register %d\n",
                   __FILE__, __LINE__, reg);
         setLastError(err_badparam, "Unknown register passed in reg field");
         return false;
   }
   
   errno = 0;
   result = ptrace(PTRACE_PEEKUSER, (pid_t) t, (void*) offset, NULL);
   if (errno)
   {
      int errnum = errno;
      sw_printf("[%s:%u] - Could not read gprs in %d: %s\n", 
                __FILE__, __LINE__, t, strerror(errnum));
      setLastError(err_procread, "Could not read registers from process");
      return false;
   }

   val = result;
   return true;
}

bool Walker::createDefaultSteppers()
{
  FrameStepper *stepper;
  WandererHelper *whelper_x86;
  LookupFuncStart *frameFuncHelper_x86;
  bool result = true;

  stepper = new DebugStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is DebugStepper\n",
            __FILE__, __LINE__, stepper);

  frameFuncHelper_x86 = LookupFuncStart::getLookupFuncStart(getProcessState());
  stepper = new FrameFuncStepper(this, frameFuncHelper_x86);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is FrameFuncStepper\n",
            __FILE__, __LINE__, stepper);

  //Call getLookupFuncStart twice to get reference counts correct.
  frameFuncHelper_x86 = LookupFuncStart::getLookupFuncStart(getProcessState());
  whelper_x86 = new WandererHelper(getProcessState());
  stepper = new StepperWanderer(this, whelper_x86, frameFuncHelper_x86);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is StepperWanderer\n",
            __FILE__, __LINE__, stepper);

  stepper = new SigHandlerStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is SigHandlerStepper\n",
            __FILE__, __LINE__, stepper);

  stepper = new BottomOfStackStepper(this);
  result = addStepper(stepper);
  if (!result)
     goto error;
  sw_printf("[%s:%u] - Stepper %p is BottomOfStackStepper\n",
            __FILE__, __LINE__, stepper);

  return true;
 error:
  sw_printf("[%s:%u] - Error adding stepper %p\n", stepper);
    return false;
}

static const int fp_offset_32 = 28;
static const int pc_offset_32 = 60;
static const int frame_size_32 = 64;
static const int fp_offset_64 = 120;
static const int pc_offset_64 = 168;
static const int frame_size_64 = 576;
gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   int fp_offset;
   int pc_offset;
   int frame_size;
   bool result;
   int addr_size = getProcessState()->getAddressWidth();
   if (addr_size == 4) {
      fp_offset = fp_offset_32;
      pc_offset = pc_offset_32;
      frame_size = frame_size_32;
   }
   else {
      fp_offset = fp_offset_64;
      pc_offset = pc_offset_64;
      frame_size = frame_size_64;
  }

   location_t fp_loc;
   Address fp = 0x0;
   fp_loc.location = loc_address;
   fp_loc.val.addr = in.getSP() + fp_offset;
   sw_printf("[%s:%u] - SigHandler Reading FP from %lx\n",
             __FILE__, __LINE__, fp_loc.val.addr);
   result = getProcessState()->readMem(&fp, fp_loc.val.addr, addr_size);
   if (!result) {
      return gcf_error;
   }

   location_t pc_loc;
   Address pc = 0x0;
   pc_loc.location = loc_address;
   pc_loc.val.addr = in.getSP() + pc_offset;
   sw_printf("[%s:%u] - SigHandler Reading PC from %lx\n",
             __FILE__, __LINE__, pc_loc.val.addr);
   result = getProcessState()->readMem(&pc, pc_loc.val.addr, addr_size);
   if (!result) {
      return gcf_error;
   }

   Address sp = in.getSP() + frame_size;

   out.setRA((Dyninst::MachRegisterVal) pc);
   out.setFP((Dyninst::MachRegisterVal) fp);
   out.setSP((Dyninst::MachRegisterVal) sp);
   out.setRALocation(pc_loc);
   out.setFPLocation(fp_loc);

   return gcf_success;
}

#if defined(cap_stackwalker_use_symtab)

#include "symtabAPI/h/Symtab.h"

bool DebugStepperImpl::isFrameRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4)
      return (reg == EBP);
   else 
      return (reg == RBP);
}

bool DebugStepperImpl::isStackRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4)
      return (reg == ESP);
   else 
      return (reg == RSP);
}

#endif

void ProcDebugLinux::detach_arch_cleanup()
{
   LookupFuncStart::clear_func_mapping(getProcessId());
}
