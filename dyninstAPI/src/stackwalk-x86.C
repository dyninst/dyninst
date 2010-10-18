/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#include "dyninstAPI/src/frameChecker.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/h/headers.h"
#include "instructionAPI/h/InstructionDecoder.h"

#if defined(os_windows)
#define caddr_t unsigned* //Fixes very odd windows compilation issue
#endif

//Possible return values for getFrameStatus
typedef enum frameStatus_t {
    frame_unknown,               //Don't know
    frame_sighandler,            //Current function is a signal handler
    frame_allocates_frame,       //Function allocates a frame
    frame_saves_fp_noframe,      //Function doesn't allocate a frame but does
                                 // save the FP (result of gcc's 
                                 // -fomit-frame-pointer)
    frame_no_use_fp,             //Function doesn't allocate a frame and doesn't
                                 // use the frame pointer (also result of gcc's
                                 // -fomit-frame-pointer) 
    frame_tramp,                 //Trampoline with stack frame
    frameless_tramp,             //Trampoline without a stack frame
    frame_vsyscall               //PC is in the vsyscall page (linux only)
} frameStatus_t;

// constants for walking out of a signal handler
// these are offsets from the stack pointer in the signal
// tramp (__restore or __restore_rt) into the sigcontext
// struct that is placed on the sigtramp's frame
// TODO: obtain these from the appropriate header files

#define SIG_HANDLER_FP_OFFSET_32 28
#define SIG_HANDLER_PC_OFFSET_32 60
#define SIG_HANDLER_FRAME_SIZE_32 64
#define SIG_HANDLER_FP_OFFSET_64 120
#define SIG_HANDLER_PC_OFFSET_64 168
#define SIG_HANDLER_FRAME_SIZE_64 576

static frameStatus_t getFrameStatus(process *p, unsigned long pc, int &extra_height)
{
   codeRange *range;

   int_function *func = NULL;
   extra_height = 0;

   mapped_object *mobj = p->findObject(pc);
   if (mobj) {
     bool result = mobj->analyze();
     assert(result);
   }

   if (p->isInSignalHandler(pc))
      return frame_sighandler;
#if defined(os_linux)
   calcVSyscallFrame(p);
   if ((pc >= p->getVsyscallStart() && pc < p->getVsyscallEnd()) || /* RH9 Hack */ (pc >= 0xffffe000 && pc < 0xfffff000))
      return frame_vsyscall;
#endif

   // See if we're in instrumentation
   Address origAddr = pc;

   baseTrampInstance *bti = NULL;
   if (p->getRelocInfo(pc, 
                       origAddr,
                       func,
                       bti)) {
      // Find out whether we've got a saved
      // state or not
      if (bti) {
         extra_height = bti->trampStackHeight();
         if (bti->baseT->createFrame()) {
            return frame_tramp;
         }
         else {
            return frameless_tramp;
         }
      }
   }
   else {
      range = p->findOrigByAddr(origAddr);
      func = range->is_function();
   }

   if (func == NULL) {
      return frame_unknown;
   }
   else if (!func->hasNoStackFrame()) 
      return frame_allocates_frame;   
   else if (func->savesFramePointer())
     return frame_saves_fp_noframe;
   else
     return frame_no_use_fp;
}


// Returns true if it's a call, and returns the callee function.
// 
// The code is very different version for defensiveMode as the parsing
// often assumes calls not to return, which makes determining whether the 
// previous instruction is a call much more difficult.
static bool isPrevInstrACall(Address addr, process *proc, int_function **callee)
{
    if (BPatch_defensiveMode != proc->getHybridMode()) {
        codeRange *range = proc->findOrigByAddr(addr);
        pdvector<instPoint *> callsites;

        if (range == NULL) {
            baseTrampInstance *bti = NULL;
            Address origAddr = 0;
            int_function *tmp=NULL;
            bool success = proc->getRelocInfo(addr, origAddr, tmp, bti);
            if (success) {
                addr = origAddr;
                range = proc->findOrigByAddr(origAddr);
            }
        }
        if (range == NULL) {
            return false;
        }
   
        int_function *func_ptr = range->is_function();

        if (func_ptr != NULL)
            callsites = func_ptr->funcCalls();
        else
            return false;
      
        for (unsigned i = 0; i < callsites.size(); i++)
            {
                instPoint *site = callsites[i];

                // Argh. We need to check for each call site in each
                // instantiation of the function.
                if (site->match(addr - site->insn()->size())) {
                    *callee = site->findCallee();
                    return true;
                }
            }   
   
        return false; 
    }
    else { // in defensive mode 

        /*In defensive mode calls may be deemed to be non-returning at
          parse time and there is an additional complication, the block 
          may have been split, meaning that block->containsCall()
          is not a reliable test.

          General approach: 
          find block at addr-1, then translate back to original
          address and see if the block's last instruction is a call
        */

        codeRange *callRange = proc->findOrigByAddr(addr-1);
        bblInstance *callBBI = callRange->is_basicBlockInstance();
        Address callAddr = 0; //address of call in original block instance

        if (callRange && callRange->is_mapped_object()) 
        {
            callRange->is_mapped_object()->analyze();
            callRange = proc->findOrigByAddr(addr-1);
            callBBI = callRange->is_basicBlockInstance();
        }
        if (!callRange) {
            // see if we're in instrumentation
            baseTrampInstance *bti = NULL;
            Address origAddr = 0;
            int_function *tmp = NULL;
            bool success = proc->getRelocInfo(addr, origAddr, tmp, bti);
            if (success) {
               
               callBBI = tmp->findBlockInstanceByAddr(origAddr);
               //// this is possible if we're searching for the
               //// return address of a frameless function and happen to 
               //// run over an instrumentation address
               //mal_printf("Stackwalked into relocated code "
               //           "[%lx from origAddr %lx], which should "
               //           "not be possible as we're in defensive mode, "
               //           "where we disable frameless tramps %s[%d]\n",
               //           addr, origAddr, FILE__,__LINE__);
            }
            return false;
        }


        // if the range is a bbi, and it contains a call, and the call instruction
        // matches the address, set callee and return true
        if (callBBI && addr == callBBI->endAddr()) {

            // if the block has no call, make sure it's not due to block splitting
            if ( !callBBI->block()->containsCall() ) {
                Address origAddr = callBBI->equivAddr(0,callBBI->lastInsnAddr());
                callBBI = proc->findOrigByAddr(origAddr)->is_basicBlockInstance();
                if (callBBI && callBBI->block()->containsCall()) {
                    callAddr = origAddr;//addr of orig call instruction
                }
            }
            // if the block does contain a call, set callAddr if we haven't yet
            if ( !callAddr && callBBI->block()->containsCall() ) {
                callAddr = callBBI->equivAddr(0, callBBI->lastInsnAddr());
            }
        }

        if (callAddr) {
            instPoint *callPoint = callBBI->func()->findInstPByAddr( callAddr );
            if (!callPoint) { // this is necessary, at least the first time
                callBBI->func()->funcCalls();
                callPoint = callBBI->func()->findInstPByAddr( callAddr );
            }
            if (!callPoint || callSite != callPoint->getPointType()) {
                assert(callBBI->func()->obj()->parse_img()->codeObject()->
                       defensiveMode());
                mal_printf("Warning, call at %lx, found while "
                           "stackwalking, has no callpoint attached, does "
                           "the target tamper with the call stack? %s[%d]\n", 
                           callAddr, FILE__,__LINE__);
                *callee = callBBI->func(); // wrong function here, but what can we do?
            } else {
                *callee = callPoint->findCallee();
            }
            if (NULL == *callee && 
                string::npos == callBBI->func()->get_name().find("DYNINSTbreakPoint"))
                {
                    mal_printf("WARNING: didn't find a callee for callPoint at "
                               "%lx when stackwalking %s[%d]\n", callAddr, 
                               FILE__,__LINE__);
                }
            return true;
        }
        return false;
    }
}


/**
 * Sometimes a function that uses a stack frame may not yet have put up its
 * frame or have already taken it down by the time we reach it during a stack
 * walk.  This function returns true in this case.  offset is the distance
 * from the top of the stack to the return value for the caller.
 **/
static bool hasAllocatedFrame(Address addr, process *proc, int &offset)
{
    codeRange *range = proc->findOrigByAddr(addr);

    if (range &&
        range->is_basicBlockInstance()) {
      frameChecker fc((const unsigned char*)(proc->getPtrToInstruction(addr)),
		      range->get_size() - (addr - range->get_address()),
		      proc->getAOut()->parse_img()->codeObject()->cs()->getArch());
      if(fc.isReturn() || fc.isStackPreamble())
      {
	offset = 0;
	return false;
      }
      if(fc.isStackFrameSetup())
      {
	offset = proc->getAddressWidth();
	return false;
      }
    }
    return true;
}

/**
 * If frame 'f' is a trampoline frame, this function returns true
 * if the trampoline was called by function entry or exit
 * instrumentation.
 **/
static bool isInEntryExitInstrumentation(Frame f)
{
  instPoint *p = f.getPoint();
  if (!p) return false;
  
  if (p->getPointType() == functionEntry ||
      p->getPointType() == functionExit)
    // Not sure yet if function exit will be preInst or postInst...
    return true;
  return false;
}

class DyninstMemRegReader : public Dyninst::SymtabAPI::MemRegReader
{
 private:
   process *proc;
   Frame *orig_frame;
 public:
   DyninstMemRegReader(process *p, Frame *f) { 
      proc = p; 
      orig_frame = f;
   }
   virtual bool ReadMem(Address addr, void *buffer, unsigned size) {
      return proc->readDataSpace((void *) addr, size, buffer, false);
   }

   virtual bool GetReg(MachRegister reg, MachRegisterVal &val) {
      if (proc->getAddressWidth() == 4) {
          switch (reg.val()) {
            case x86::ieip:
              case x86_64::ieip:
                  val = orig_frame->getPC();
               break;
            case x86::iesp:
              case x86_64::iesp:
                  val = orig_frame->getSP();
               break;
            case x86::iebp:
              case x86_64::iebp:
                  val = orig_frame->getFP();
               break;
            default:
               return false;
         }
      }
      else {
          switch (reg.val()) {
            case x86_64::irip:
            case x86_64::ieip:
                  val = orig_frame->getPC();
               break;
            case x86_64::irsp:
            case x86_64::iesp:
               val = orig_frame->getSP();
               break;
            case x86_64::irbp:
            case x86_64::iebp:
                val = orig_frame->getFP();
               break;
            default:
               return false;
         }
      }
      return true;
   }

   bool start() {
      return true;
   }

   bool done() {
      return true;
   }
   
   virtual ~DyninstMemRegReader() {};
};

extern int tramp_pre_frame_size_32;
extern int tramp_pre_frame_size_64;

//The estimated maximum frame size when having to do an 
// exhaustive search for a frame.
#define MAX_STACK_FRAME_SIZE 8192

// x86_64 uses a different stack address than standard x86.
#define MAX_STACK_FRAME_ADDR_64 0x0000007fbfffffff
#define MAX_STACK_FRAME_ADDR_32 0xbfffffff

Frame Frame::getCallerFrame()
{
  stackwalk_printf("Entry to getCallerFrame, cur pc 0x%lx\n", pc_);

    int_function *cur_func = getProc()->findFuncByAddr(pc_);
    int addr_size = getProc()->getAddressWidth();
    int extra_height = 0;
#if defined(os_linux)
    // assume _start is never called, so just return if we're there
    if (cur_func &&
       cur_func->getAddress() == getProc()->getAOut()->parse_img()->getObject()->getEntryOffset()) {
      stackwalk_printf("%s[%d]: stack walk at entry of a.out (_start), returning null frame\n", FILE__, __LINE__);
       return Frame();
    }
#endif
   /**
    * These two variables are only valid when this function is
    * called recursively.
    **/
   static Frame prevFrame;
   static bool prevFrameValid = false;

   /**
    * for the x86, the frame-pointer (EBP) points to the previous 
    * frame-pointer, and the saved return address is in EBP-4.
    **/
   struct {
      Address fp;
      Address rtn;
   } addrs;

   frameStatus_t status;

   Address newPC=0;
   Address newFP=0;
   Address newSP=0;
   Address newpcAddr=0;
   Address pcLoc=0;
   addrs.fp = 0;
   addrs.rtn = 0;

   status = getFrameStatus(getProc(), getPC(), extra_height);

   stackwalk_printf("%s[%d]: frame status is %d\n",
		    FILE__, __LINE__, status);

   if (status == frame_vsyscall)
   {
#if defined(os_linux)
      SymtabAPI::Symtab *vsys_obj;

      if ((vsys_obj = getProc()->getVsyscallObject()) == NULL ||
          !vsys_obj->hasStackwalkDebugInfo())
      {
        /**
         * No vsyscall stack walking data present (we're probably 
         * on Linux 2.4) we'll go ahead and treat the vsyscall page 
         * as a leaf
         **/
         stackwalk_printf("%s[%d]: no vsyscall data present, treating as leaf frame\n", FILE__, __LINE__);
         if (!getProc()->readDataSpace((void *) sp_, addr_size, 
                                       (void *) &addrs.rtn, true)) {
            stackwalk_printf("%s[%d]: Failed to access memory at 0x%x, returning null frame \n", FILE__, __LINE__, sp_);
            return Frame();
         }
         
         newFP = fp_;
         newPC = addrs.rtn;
         newSP = sp_+addr_size;
         goto done;
      }
      else
      {
         /**
          * We have vsyscall stack walking data, which came from
          * the .eh_frame section of the vsyscall DSO.  We'll use
          * getRegValueAtFrame to parse the data and get the correct
          * values for %esp, %ebp, and %eip
          **/
         Address vsys_base = 0x0;
         DyninstMemRegReader reader(getProc(), this);
         stackwalk_printf("%s[%d]: vsyscall data is present, analyzing\n", 
                          FILE__, __LINE__);
         bool result;
         result = vsys_obj->getRegValueAtFrame(pc_, ReturnAddr,
                                               newPC, &reader);
         if (!result) {
            //Linux in inconsistent about whether we should subtract the
            // vys_start before using.  So we try both, stick with what works
            vsys_base = getProc()->getVsyscallStart();
            result = vsys_obj->getRegValueAtFrame(pc_ - vsys_base, ReturnAddr,
                                                  newPC, &reader);
         }
         if (!result) {
            //It gets worse, sometimes the vsyscall data is just plain wrong.
            //FC-9 randomized the location of the vsyscall page, but didn't update
            //the debug info.  We'll try the non-updated address.
            vsys_base = getProc()->getVsyscallStart() - 0xffffe000;
            result = vsys_obj->getRegValueAtFrame(pc_ - vsys_base, ReturnAddr,
                                                  newPC, &reader);
         }
         if (!result) {
            stackwalk_printf("[%s:%u] - Error getting PC value of vsyscall\n",
                             __FILE__, __LINE__);
            return Frame();                             
         }         
         Dyninst::MachRegister frame_reg;
         if (getProc()->getAddressWidth() == 4)
            frame_reg = x86::ebp;
         else
            frame_reg = x86_64::rbp;

         result = vsys_obj->getRegValueAtFrame(pc_ - vsys_base, frame_reg,
                                               newFP, &reader);
         if (!result) {
            stackwalk_printf("[%s:%u] - Couldn't get frame debug info at %lx\n",
                              __FILE__, __LINE__, pc_);
            return Frame();
         }
         
         result = vsys_obj->getRegValueAtFrame(pc_ - vsys_base, FrameBase,
                                               newSP, &reader);
         if (!result) {
            stackwalk_printf("[%s:%u] - Couldn't get stack debug info at %lx\n",
                      __FILE__, __LINE__, pc_);
            return Frame();
         }
         goto done;
      }
#endif
   }
   else if (status == frame_sighandler)
   {
     stackwalk_printf("%s[%d]: parsing signal handler...\n", FILE__, __LINE__);
      int fp_offset, pc_offset, frame_size;
      if (addr_size == 4) {
         fp_offset = SIG_HANDLER_FP_OFFSET_32;
         pc_offset = SIG_HANDLER_PC_OFFSET_32;
         frame_size = SIG_HANDLER_FRAME_SIZE_32;
      }
      else {
         fp_offset = SIG_HANDLER_FP_OFFSET_64;
         pc_offset = SIG_HANDLER_PC_OFFSET_64;
         frame_size = SIG_HANDLER_FRAME_SIZE_64;
      }
      
      if (!getProc()->readDataSpace((caddr_t)(sp_+fp_offset), addr_size,
                                    &addrs.fp, true)) {
	stackwalk_printf("%s[%d]: Failed to read memory at sp_+fp_offset 0x%lx\n", FILE__, __LINE__,sp_+fp_offset);
         return Frame();
      }
      if (!getProc()->readDataSpace((caddr_t)(sp_+pc_offset), addr_size,
                                    &addrs.rtn, true)) {
	stackwalk_printf("%s[%d]: Failed to read memory at sp_+pc_offset 0x%lx\n", FILE__, __LINE__,sp_+pc_offset);
         return Frame();
      }
      
      
      /**
       * If the current frame is for the signal handler function, then we need 
       * to read the information about the next frame from the data saved by 
       * the signal handling mechanism.
       **/
      newFP = addrs.fp;
      newPC = addrs.rtn;
      newSP = sp_ + frame_size;
      pcLoc = sp_ + pc_offset;
      goto done;
   }   
   else if (status == frame_allocates_frame || status == frame_tramp)
   {
     int offset = 0;
     // FIXME: for tramps, we need to check if we've saved the FP yet

     /**
      * The function that created this frame uses the standard 
      * prolog: push %ebp; mov %esp->ebp .  We can read the 
      * appropriate data from the frame pointer.
      **/
    
     if ((status != frame_tramp && 
          !hasAllocatedFrame(pc_, getProc(), offset)) || 
         (prevFrameValid && isInEntryExitInstrumentation(prevFrame)))
     {
        addrs.fp = offset + sp_;
        if (!getProc()->readDataSpace((caddr_t) addrs.fp, addr_size, 
                                      &addrs.rtn, true)) {
           stackwalk_printf("%s[%d]: Failed to read memory at addrs.fp 0x%lx\n", 
                            FILE__, __LINE__, addrs.fp);
           return Frame();
        }
        newPC = addrs.rtn;
        newFP = fp_;
        newSP = addrs.fp + getProc()->getAddressWidth();
        pcLoc = addrs.fp;
     }
     else
     {
        if (!fp_) {
           stackwalk_printf("%s[%d]: No frame pointer!\n", FILE__, __LINE__);	  
           return Frame();
        }
        if (!getProc()->readDataSpace((caddr_t) fp_, addr_size, 
                                      &addrs.fp, false)) {
           stackwalk_printf("%s[%d]: Failed to read memory at fp_ 0x%lx\n", 
                            FILE__, __LINE__, fp_);	   
           return Frame();
        }
        if (!getProc()->readDataSpace((caddr_t) (fp_ + addr_size), addr_size, 
                                      &addrs.rtn, false)) {
           stackwalk_printf("%s[%d]: Failed to read memory at pc (fp_+addr_size) 0x%lx\n",
                            FILE__, __LINE__, fp_+addr_size);
           return Frame();
        }
        newFP = addrs.fp;
        newPC = addrs.rtn;
        newSP = fp_+ (2 * addr_size);
        pcLoc = fp_ + addr_size;
     }
     if (status == frame_tramp)
        newSP += extra_height;
     goto done;
   }
   else if (status == frameless_tramp)
   {
       baseTrampInstance *bti = NULL;
       Address origAddr = 0;
       int_function *tmp = NULL;
       bool success = getProc()->getRelocInfo(pc_, origAddr, tmp, bti);
       assert(success);
       newPC = origAddr;
       newFP = fp_;
       newSP = sp_; //Not really correct, but difficult to compute and unlikely to matter
       pcLoc = 0x0;
   }
   else if (status ==  frame_saves_fp_noframe || status == frame_no_use_fp ||
            status == frame_unknown)
   {
      /**
       * The evil case.  We don't have a valid frame pointer.  We'll
       * start a search up the stack from the sp, looking for an address
       * that could qualify as the result of a return.  We'll do a few
       * things to try and keep ourselves from accidently following a 
       * constant value that looks like a return:
       *  - Make sure the address in the return follows call instruction.
       *  - See if the resulting frame pointer is part of the stack.
       *  - Peek ahead.  If the stack trace from following the address doesn't
       *     end with the top of the stack, we probably shouldn't follow it.
       **/

     stackwalk_printf("%s[%d]: Going into heuristic stack walker...\n", FILE__, __LINE__);

      Address estimated_sp;
      Address estimated_ip;
      Address estimated_fp;
      Address stack_top;
      int_function *callee = NULL;
      bool result;

      /**
       * Calculate the top of the stack.
       **/
      Address max_stack_frame_addr =
#if defined(arch_x86_64)	  
      addr_size == 8 ? MAX_STACK_FRAME_ADDR_64 : MAX_STACK_FRAME_ADDR_32;
#else
          MAX_STACK_FRAME_ADDR_32;
#endif

      stack_top = 0;
      if (sp_ < max_stack_frame_addr && sp_ > max_stack_frame_addr - 0x200000)
      {
          //If we're within two megs of the linux x86 default stack, we'll
	      // assume that's the one in use.
          // Points to first possible integer
          stack_top = max_stack_frame_addr - (addr_size - 1);
      }
      else if (getProc()->multithread_capable() && 
               thread_ != NULL &&
               thread_->get_stack_addr() != 0)
      {
         int stack_diff = thread_->get_stack_addr() - sp_;
         if (stack_diff < MAX_STACK_FRAME_SIZE && stack_diff > 0)
            stack_top = thread_->get_stack_addr();
      }
      if (stack_top == 0)
         stack_top = sp_ + MAX_STACK_FRAME_SIZE;
      assert(sp_ < stack_top);

      /**
       * Search for the correct return value.
       **/
      estimated_sp = sp_;
      for (; estimated_sp <= stack_top; estimated_sp++)
      {
         estimated_ip = 0;
         result = getProc()->readDataSpace((caddr_t) estimated_sp, addr_size, 
                                           &estimated_ip, false);
         
         if (!result) break;

         //If the instruction that preceeds this address isn't a call
         // instruction, then we'll go ahead and look for another address.
         if (!isPrevInstrACall(estimated_ip, getProc(), &callee))
            continue;

         //Given this point for the top of our stack frame, calculate the 
         // frame pointer         
         if (status == frame_saves_fp_noframe)
         {
            result = getProc()->readDataSpace((caddr_t) estimated_sp-addr_size,
                              sizeof(int), (caddr_t) &estimated_fp, false);
            if (!result) break;
         }
         else //status == NO_USE_FP
         {
            estimated_fp = fp_;
         }

         //If the call instruction calls into the current function, then we'll
         // just skip everything else and assume we've got the correct return
         // value (fingers crossed).
         if (cur_func != NULL && cur_func == callee)
         {
            pcLoc = estimated_sp;
            newPC = estimated_ip;
            newFP = estimated_fp;
            newSP = estimated_sp+addr_size;
            goto done;
         }
         
         //Check the validity of the frame pointer.  It's possible the
         // previous frame doesn't have a valid fp, so we won't be able
         // to rely on the check in this case.
         int_function *next_func = getProc()->findFuncByAddr(estimated_ip);
         if (next_func != NULL && 
             getFrameStatus(getProc(), estimated_ip, extra_height) == frame_allocates_frame &&
             (estimated_fp < fp_ || estimated_fp > stack_top))
         {
            continue;
         }

         //BAD HACK: The initial value of %esi when main starts sometimes
         // points to an area in the guard_setup function that may look
         // like a valid return value in some versions of libc.  Since it's
         // easy for the value of %esi to get saved on the stack somewhere,
         // we'll special case this.
         if (callee == NULL && next_func != NULL &&
             !strcmp(next_func->prettyName().c_str(), "__guard_setup"))
         {
           continue;
         }

         pcLoc = estimated_sp;
         newPC = estimated_ip;
         newFP = estimated_fp;
         newSP = estimated_sp + getProc()->getAddressWidth();
         goto done;
      }
   }
   stackwalk_printf("%s[%d]: Heuristic stack walker failed, returning null frame\n", FILE__, __LINE__);

   return Frame();

 done:

   Frame ret = Frame(newPC, newFP, newSP, newpcAddr, this);
   ret.pcAddr_ = pcLoc;

   if (status == frame_tramp)
   {
      /**
       * Instrumentation has its own stack frame (created in the
       * base tramp), but is really an extension of the function. 
       * We skip the function. Platform-independent code can
       * always regenerate it if desired.
       **/
      prevFrameValid = true;
      prevFrame = *this;
      ret = ret.getCallerFrame();
      prevFrameValid = false;
   }
   return ret;
}

