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

#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/libstate.h"
#include "common/h/dyntypes.h"
#include "common/h/VariableLocation.h"
#include "dwarfFrameParser.h"
#include "dwarfHandle.h"
#include "Architecture.h"
#include "registers/abstract_regs.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"

#ifdef arch_aarch64
# include "registers/aarch64_regs.h"
#endif

#if defined(WITH_SYMTAB_API)
#include "symtabAPI/h/Symtab.h"
#endif

using namespace Dyninst;
using namespace Stackwalker;
using namespace DwarfDyninst;

static std::map<std::string, DwarfFrameParser::Ptr> dwarf_info;

#include <sys/ucontext.h>
#include <stdarg.h>
#include "dwarf.h"
#include "elfutils/libdw.h"
#include "Elf_X.h"

static DwarfFrameParser::Ptr getAuxDwarfInfo(std::string s)
{
   static std::map<std::string, DwarfFrameParser::Ptr > dwarf_aux_info;

   std::map<std::string, DwarfFrameParser::Ptr >::iterator i = dwarf_aux_info.find(s);
   if (i != dwarf_aux_info.end())
      return i->second;

   SymReader *orig_reader = LibraryWrapper::getLibrary(s);
   if (!orig_reader) {
      sw_printf("[%s:%d] - Error.  Could not find elf handle for %s\n",
                FILE__, __LINE__, s.c_str());
      return DwarfFrameParser::Ptr();
   }
   Elf_X *orig_elf = (Elf_X *) orig_reader->getElfHandle();
   if (!orig_elf) {
      sw_printf("[%s:%d] - Error. Could not find elf handle for file %s\n",
                FILE__, __LINE__, s.c_str());
      dwarf_aux_info[s] = DwarfFrameParser::Ptr();
      return DwarfFrameParser::Ptr();
   }

   DwarfHandle::ptr dwarf = DwarfHandle::createDwarfHandle(s, orig_elf);
   assert(dwarf);
   sw_printf("[%s:%d] - Separate debug file used: %s\n",
           FILE__, __LINE__, dwarf->getDebugFilename().c_str());

   // MJMTODO - Need to check whether this is supposed to work or not
   // FIXME for ppc, if we ever support debug walking on ppc
   Architecture arch;
#if defined(arch_x86) || defined(arch_x86_64)
   if (orig_elf->wordSize() == 4)
      arch = Dyninst::Arch_x86;
   else
      arch = Dyninst::Arch_x86_64;
#elif defined(arch_aarch64)
    arch = Dyninst::Arch_aarch64;
#endif

   DwarfFrameParser::Ptr dresult = DwarfFrameParser::create(*dwarf->frame_dbg(), dwarf->origFile()->e_elfp(), arch);
   if(!dresult) return NULL;
   dwarf_aux_info[s] = dresult;
   return dresult;
}


DebugStepperImpl::DebugStepperImpl(Walker *w, DebugStepper *parent) :
   FrameStepper(w),
   last_addr_read(0),
   last_val_read(0),
   addr_width(0),
   parent_stepper(parent),
   cur_frame(NULL),
   depth_frame(NULL)
{
}

bool DebugStepperImpl::ReadMem(Address addr, void *buffer, unsigned size)
{
   bool result = getProcessState()->readMem(buffer, addr, size);

   last_addr_read = 0;
   if (!result)
      return false;
   if (size != addr_width)
      return false;

   last_addr_read = addr;
   if (addr_width == 4) {
      uint32_t v = *((uint32_t *) buffer);
      last_val_read = v;
   }
   else if (addr_width == 8) {
      uint64_t v = *((uint64_t *) buffer);
      last_val_read = v;
   }
   else {
      assert(0); //Unknown size
   }

   return true;
}

location_t DebugStepperImpl::getLastComputedLocation(unsigned long value)
{
   location_t loc;
   if (last_addr_read && last_val_read == value) {
      loc.val.addr = last_addr_read;
      loc.location = loc_address;
   }
   else {
      loc.val.addr = 0;
      loc.location = loc_unknown;
   }
   last_addr_read = 0;
   last_val_read = 0;
   return loc;
}

bool DebugStepperImpl::GetReg(MachRegister reg, MachRegisterVal &val)
{
   sw_printf("[%s:%d] Attempt to get value for reg %s\n", FILE__, __LINE__, reg.name().c_str());
   if (reg.isFramePointer()) {
      val = static_cast<MachRegisterVal>(depth_frame->getFP());
      return true;
   }

   if (reg.isStackPointer()) {
      val = static_cast<MachRegisterVal>(depth_frame->getSP());
      return true;
   }

   if (reg.isPC()) {
      val = static_cast<MachRegisterVal>(depth_frame->getRA());
      return true;
   }

   bool result = false;
   const Frame *prevDepthFrame = depth_frame;
   depth_frame = depth_frame->getPrevFrame();
   if (!depth_frame)
   {
      result = getProcessState()->getRegValue(reg, cur_frame->getThread(), val);
   }
#if defined(WITH_SYMTAB_API)
   else
   {
      Offset offset;
      void *symtab_v = NULL;
      std::string lib;
      depth_frame->getLibOffset(lib, offset, symtab_v);
      SymtabAPI::Symtab *symtab = (SymtabAPI::Symtab*) symtab_v;
      if (symtab)
      {
         result = symtab->getRegValueAtFrame(offset, reg, val, this);
      }
#if defined(arch_aarch64)
      if (!result) {
          sw_printf("Cast framestepper %p for frame %p to SigHandlerStepper at address %lx\n", (void*)prevDepthFrame->getStepper(), (const void*)prevDepthFrame, prevDepthFrame->getRA());

          SigHandlerStepper * ss = dynamic_cast<SigHandlerStepper*> (prevDepthFrame->getStepper());
	  if (ss != NULL) {
	      sw_printf("[%s:%d] - Not the first frame, cannot find dbg information, and previous frame is a signal trampoline frame. Try to get x30 from ucontext as RA\n",
                FILE__, __LINE__);
              static     ucontext_t dummy_context;
	      static int lr_offset = (char*)&(dummy_context.uc_mcontext.regs[30]) - (char*)&dummy_context;
	      // This assumes that a ucontext_t is at the following offset from the top of the signal handler's stack.
	      static int ucontext_offset = 128;
	      const Frame * signal_frame = depth_frame;
	      if (signal_frame != NULL) {
	          int addr_size = 8;
	          Address lr_addr = signal_frame->getSP() + ucontext_offset + lr_offset;
		  result = getProcessState()->readMem(&val, lr_addr, addr_size);
	      sw_printf("[%s:%d] - readMem results %d, get %lx as RA\n",
                FILE__, __LINE__, result, val);

	      } else {
	      sw_printf("[%s:%d] - the signal trampoline frame does not have a parent frame\n",
                FILE__, __LINE__);
	      }
	  } else {
	      sw_printf("Cannot cast framestepper to SigHandlerStepper\n");
	  }
       }
#endif      
   }
#endif

   depth_frame = prevDepthFrame;
   return result;
}

gcframe_ret_t DebugStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   LibAddrPair lib;
   bool result;

   if (lookupInCache(in, out)) {
       result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(out.getRA(), lib);
       if (result) {
           // Hit, and valid RA found
           return gcf_success;
       }
   }

   // This error check is duplicated in BottomOfStackStepper.
   // We should always call BOSStepper first; however, we need the
   // library for the debug stepper as well. If this becomes
   // a performance problem we can cache the library info in
   // the input frame.
   result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(in.getRA(), lib);
   if (!result) {
      sw_printf("[%s:%d] - Stackwalking with PC at %lx, which is not found in any known libraries\n",
                FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   Address pc = in.getRA() - lib.second;
   sw_printf("[%s:%d] Dwarf-based stackwalking, using local address 0x%lx from 0x%lx - 0x%lx\n",
             FILE__, __LINE__, pc, in.getRA(), lib.second);
   if (in.getRALocation().location != loc_register && !in.nonCall()) {
      /**
       * If we're here, then our in.getRA() should be pointed at the
       * instruction following a call.  We could either use the
       * call instruction's debug info (pc - 1) or the following
       * instruction's debug info (pc) to continue the stackwalk.
       *
       * In most cases it doesn't matter.  Because of how DWARF debug
       * info is defined, the stack doesn't change between these two points.
       *
       * However, if the call is a non-returning call (e.g, a call to exit)
       * then the next instruction may not exist or may be part of a separate
       * block with different debug info.  In these cases we want to use the
       * debug info associated with the call.  So, we subtract 1 from the
       * pc to get at the call instruction.
       **/
      pc = pc - 1;
   }

   /**
    * Some system libraries on some systems have their debug info split
    * into separate files, usually in /usr/lib/debug/.  Check these
    * for DWARF debug info
    **/
   DwarfFrameParser::Ptr dauxinfo = getAuxDwarfInfo(lib.first);
   if (!dauxinfo || !dauxinfo->hasFrameDebugInfo()) {
      sw_printf("[%s:%d] - Library %s does not have stackwalking debug info\n",
                 FILE__, __LINE__, lib.first.c_str());
      return gcf_not_me;
   }

   bool isVsyscallPage = false;
#if defined(os_linux)
   sw_printf("ARM-debug: dump lib========================\n");
   sw_printf("%s\n", lib.first.c_str());
   sw_printf("ARM-debug: dump lib========================\n");
   isVsyscallPage = (strstr(lib.first.c_str(), "[vsyscall-") != NULL);
#endif

   sw_printf("[%s:%d] - Using DWARF debug file info for %s\n",
                   FILE__, __LINE__, lib.first.c_str());
   cur_frame = &in;
   gcframe_ret_t gcresult = getCallerFrameArch(pc, in, out, dauxinfo, isVsyscallPage);
   cur_frame = NULL;

   result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(out.getRA(), lib);
   if (!result) return gcf_not_me;

   if (gcresult == gcf_success) {
      sw_printf("[%s:%d] - Success walking with DWARF aux file\n",
                FILE__, __LINE__);
      return gcf_success;
   }

   return gcresult;
}

void DebugStepperImpl::registerStepperGroup(StepperGroup *group)
{
   addr_width = group->getWalker()->getProcessState()->getAddressWidth();
   if (addr_width == 4)
      group->addStepper(parent_stepper, 0, 0xffffffff);
#if defined(arch_64bit)
   else if (addr_width == 8)
      group->addStepper(parent_stepper, 0, 0xffffffffffffffff);
#endif
   else
      assert(0 && "Unknown architecture word size");
}

unsigned DebugStepperImpl::getPriority() const
{
   return debugstepper_priority;
}

DebugStepperImpl::~DebugStepperImpl()
{
}

#if defined(arch_x86) || defined(arch_x86_64)
gcframe_ret_t DebugStepperImpl::getCallerFrameArch(Address pc, const Frame &in,
                                                   Frame &out, DwarfFrameParser::Ptr dinfo,
                                                   bool isVsyscallPage)
{
   MachRegisterVal frame_value, stack_value, ret_value;
   bool result;
   FrameErrors_t frame_error = FE_No_Error;

   addr_width = getProcessState()->getAddressWidth();

   depth_frame = cur_frame;

   result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                      ret_value, this, frame_error);

   if (!result && frame_error == FE_No_Frame_Entry && isVsyscallPage) {
      //Work-around kernel bug.  The vsyscall page location was randomized, but
      // the debug info still has addresses from the old, pre-randomized days.
      // See if we get any hits by assuming the address corresponds to the
      // old PC.
      pc += 0xffffe000;
      result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                         ret_value, this, frame_error);
   }
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get return debug info at %lx, error: %d\n",
                FILE__, __LINE__, in.getRA(), frame_error);
      return gcf_not_me;
   }
   location_t ra_loc = getLastComputedLocation(ret_value);

   Dyninst::MachRegister frame_reg;
   if (addr_width == 4)
      frame_reg = x86::ebp;
   else
      frame_reg = x86_64::rbp;

   result = dinfo->getRegValueAtFrame(pc, frame_reg,
                                      frame_value, this, frame_error);
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get frame debug info at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   location_t fp_loc = getLastComputedLocation(frame_value);

   result = dinfo->getRegValueAtFrame(pc, Dyninst::FrameBase,
                                      stack_value, this, frame_error);
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get stack debug info at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   location_t sp_loc = getLastComputedLocation(stack_value);

   if (isVsyscallPage) {
      // RHEL6 has broken DWARF in the vsyscallpage; it has
      // a double deref for the stack pointer. We detect this
      // (as much as we can...) and ignore it
      if (stack_value < in.getSP()) {
         stack_value = 0;
         sp_loc.location = loc_unknown;
      }
   }

   Address MAX_ADDR;
   if (addr_width == 4) {
       MAX_ADDR = 0xffffffff;
   }
#if defined(arch_64bit)
   else if (addr_width == 8){
       MAX_ADDR = 0xffffffffffffffff;
   }
#endif
   else {
       assert(0 && "Unknown architecture word size");
   }

   if(ra_loc.val.addr > MAX_ADDR || fp_loc.val.addr > MAX_ADDR || sp_loc.val.addr > MAX_ADDR) return gcf_not_me;

   out.setRA(ret_value);
   out.setFP(frame_value);
   out.setSP(stack_value);
   out.setRALocation(ra_loc);
   out.setFPLocation(fp_loc);
   out.setSPLocation(sp_loc);

   addToCache(in, out);

   return gcf_success;
}

void DebugStepperImpl::addToCache(const Frame &cur, const Frame &caller) {
  const location_t &calRA = caller.getRALocation();

  const location_t &calFP = caller.getFPLocation();

  unsigned raDelta = (unsigned) -1;
  unsigned fpDelta = (unsigned) -1;
  unsigned spDelta = (unsigned) -1;

  if (calRA.location == loc_address) {
    raDelta = calRA.val.addr - cur.getSP();
  }

  if (calFP.location == loc_address) {
    fpDelta = calFP.val.addr - cur.getSP();
  }

  spDelta = caller.getSP() - cur.getSP();

  cache_[cur.getRA()] = cache_t(raDelta, fpDelta, spDelta);
}

bool DebugStepperImpl::lookupInCache(const Frame &cur, Frame &caller) {
  dyn_hash_map<Address,cache_t>::iterator iter = cache_.find(cur.getRA());
  if (iter == cache_.end()) {
      return false;
  }

  addr_width = getProcessState()->getAddressWidth();

  if (iter->second.ra_delta == (unsigned) -1) {
      return false;
  }
  if (iter->second.fp_delta == (unsigned) -1) {
    return false;
  }
  assert(iter->second.sp_delta != (unsigned) -1);

  Address MAX_ADDR;
   if (addr_width == 4) {
       MAX_ADDR = 0xffffffff;
   }
#if defined(arch_64bit)
   else if (addr_width == 8){
       MAX_ADDR = 0xffffffffffffffff;
   }
#endif
   else {
       assert(0 && "Unknown architecture word size");
       return false;
   }

  location_t RA;
  RA.location = loc_address;
  RA.val.addr = cur.getSP() + iter->second.ra_delta;
  RA.val.addr %= MAX_ADDR;

  location_t FP;
  FP.location = loc_address;
  FP.val.addr = cur.getSP() + iter->second.fp_delta;

  FP.val.addr %= MAX_ADDR;
  int buffer[10];

  caller.setRALocation(RA);
  ReadMem(RA.val.addr, buffer, addr_width);
  caller.setRA(last_val_read);

  caller.setFPLocation(FP);
  ReadMem(FP.val.addr, buffer, addr_width);
  caller.setFP(last_val_read);

  caller.setSP(cur.getSP() + iter->second.sp_delta);

  return true;
}

#endif

// for aarch64 architecure specifically
#if defined(arch_aarch64)
gcframe_ret_t DebugStepperImpl::getCallerFrameArch(Address pc, const Frame &in,
                                                   Frame &out, DwarfFrameParser::Ptr dinfo,
                                                   bool isVsyscallPage)
{
   MachRegisterVal frame_value, stack_value, ret_value;
   bool result;
   FrameErrors_t frame_error = FE_No_Error;

   addr_width = getProcessState()->getAddressWidth();

   depth_frame = cur_frame;

   sw_printf("\nDebugStepperImpl::getCallerFrameArch() calls getRegValueAtFrame()\n");
   result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
   //result = dinfo->getRegValueAtFrame(pc, Dyninst::aarch64::x30,
                                      ret_value, this, frame_error);

   if (!result && frame_error == FE_No_Frame_Entry && isVsyscallPage) {
      //Work-around kernel bug.  The vsyscall page location was randomized, but
      // the debug info still has addresses from the old, pre-randomized days.
      // See if we get any hits by assuming the address corresponds to the
      // old PC.
      pc += 0xffffe000;
      result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                         ret_value, this, frame_error);
   }
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get return debug info at %lx, error: %d\n",
                FILE__, __LINE__, in.getRA(), frame_error);
      return gcf_not_me;
   }
   location_t ra_loc = getLastComputedLocation(ret_value);

   Dyninst::MachRegister frame_reg;
   frame_reg = Dyninst::aarch64::x29;

   sw_printf("\nDebugStepperImpl::getCallerFrameArch() calls getRegValueAtFrame()\n");
   result = dinfo->getRegValueAtFrame(pc, frame_reg,
                                      frame_value, this, frame_error);
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get frame debug info at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   location_t fp_loc = getLastComputedLocation(frame_value);

   sw_printf("\nDebugStepperImpl::getCallerFrameArch() calls getRegValueAtFrame()\n");
   result = dinfo->getRegValueAtFrame(pc, Dyninst::FrameBase,
                                      stack_value, this, frame_error);
   if (!result) {
      sw_printf("[%s:%d] - Couldn't get stack debug info at %lx\n",
                 FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   location_t sp_loc = getLastComputedLocation(stack_value);

   if (isVsyscallPage) {
      // RHEL6 has broken DWARF in the vsyscallpage; it has
      // a double deref for the stack pointer. We detect this
      // (as much as we can...) and ignore it
      if (stack_value < in.getSP()) {
         stack_value = 0;
         sp_loc.location = loc_unknown;
      }
   }

   Address MAX_ADDR;
   if (addr_width == 4) {
       MAX_ADDR = 0xffffffff;
   }
   else if (addr_width == 8){
       MAX_ADDR = 0xffffffffffffffff;
   }
   else {
       assert(0 && "Unknown architecture word size");
   }

   if(ra_loc.val.addr > MAX_ADDR || fp_loc.val.addr > MAX_ADDR || sp_loc.val.addr > MAX_ADDR) return gcf_not_me;

   out.setRA(ret_value);
   out.setFP(frame_value);
   out.setSP(stack_value);
   out.setRALocation(ra_loc);
   out.setFPLocation(fp_loc);
   out.setSPLocation(sp_loc);

   addToCache(in, out);

   return gcf_success;
}

void DebugStepperImpl::addToCache(const Frame &cur, const Frame &caller) {
  const location_t &calRA = caller.getRALocation();

  const location_t &calFP = caller.getFPLocation();

  unsigned raDelta = (unsigned) -1;
  unsigned fpDelta = (unsigned) -1;
  unsigned spDelta = (unsigned) -1;

  if (calRA.location == loc_address) {
    raDelta = calRA.val.addr - cur.getSP();
  }

  if (calFP.location == loc_address) {
    fpDelta = calFP.val.addr - cur.getSP();
  }

  spDelta = caller.getSP() - cur.getSP();

  cache_[cur.getRA()] = cache_t(raDelta, fpDelta, spDelta);
}

bool DebugStepperImpl::lookupInCache(const Frame &cur, Frame &caller) {
  dyn_hash_map<Address,cache_t>::iterator iter = cache_.find(cur.getRA());
  if (iter == cache_.end()) {
      return false;
  }

  addr_width = getProcessState()->getAddressWidth();

  if (iter->second.ra_delta == (unsigned) -1) {
      return false;
  }
  if (iter->second.fp_delta == (unsigned) -1) {
    return false;
  }
  assert(iter->second.sp_delta != (unsigned) -1);

  Address MAX_ADDR;
   if (addr_width == 4) {
       assert(0);
       MAX_ADDR = 0xffffffff;
   }
   else if (addr_width == 8){
       MAX_ADDR = 0xffffffffffffffff;
   }
   else {
       assert(0 && "Unknown architecture word size");
       return false;
   }

  location_t RA;
  RA.location = loc_address;
  RA.val.addr = cur.getSP() + iter->second.ra_delta;
  RA.val.addr %= MAX_ADDR;

  location_t FP;
  FP.location = loc_address;
  FP.val.addr = cur.getSP() + iter->second.fp_delta;

  FP.val.addr %= MAX_ADDR;
  int buffer[10];

  caller.setRALocation(RA);
  ReadMem(RA.val.addr, buffer, addr_width);
  caller.setRA(last_val_read);

  caller.setFPLocation(FP);
  ReadMem(FP.val.addr, buffer, addr_width);
  caller.setFP(last_val_read);

  caller.setSP(cur.getSP() + iter->second.sp_delta);

  return true;
}
#endif
//end if defined aarch64


