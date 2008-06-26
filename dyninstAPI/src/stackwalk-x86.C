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

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"

#if defined(CAP_INSTRUCTION_API)
#include "dyninstAPI/src/frameChecker.h"
#else
#include "dyninstAPI/src/InstrucIter.h"
#endif // defined(CAP_INSTRUCTION_API)
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/h/headers.h"

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
    frame_tramp,                 //Trampoline
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

static frameStatus_t getFrameStatus(process *p, unsigned long pc)
{
   codeRange *range;

   int_function *func = NULL;
   miniTrampInstance *mini = NULL;
   multiTramp *multi = NULL;
   baseTrampInstance *base = NULL;

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

   range = p->findOrigByAddr(pc);
   func = range->is_function();
   multi = range->is_multitramp();
   mini = range->is_minitramp();
   if (multi)
       base = multi->getBaseTrampInstanceByAddr(pc);

   if (base) {
       if (base->isInInstru(pc))
           return frame_tramp;
       else
           func = base->multiT->func();
   }
   else if (multi) {
       // Not in base tramp instrumented... we're effectively in the func
       func = multi->func();
   }
   else if (mini) {
       return frame_tramp;
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

static bool isPrevInstrACall(Address addr, process *p, int_function **callee)
{
   codeRange *range = p->findOrigByAddr(addr);
   pdvector<instPoint *> callsites;

   if (range == NULL)
     return false;
   
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
     if (site->match(addr - site->insn().size())) {
	 *callee = site->findCallee();
	 return true;
     }
   }   
   
   return false; 
}



/**
 * Sometimes a function that uses a stack frame may not yet have put up its
 * frame or have already taken it down by the time we reach it during a stack
 * walk.  This function returns true in this case.  offset is the distance
 * from the top of the stack to the return value for the caller.
 **/
static bool hasAllocatedFrame(Address addr, process *proc, int &offset)
{
    int frameSizeDontCare;
    codeRange *range = proc->findOrigByAddr(addr);

    if (range &&
        range->is_basicBlockInstance()) {
#if !defined(CAP_INSTRUCTION_API)
        InstrucIter ii(range->get_address(),
                       range->get_size(),
                       proc);
        ii.setCurrentAddress(addr);
        if (ii.isAReturnInstruction() ||
            ii.isStackFramePreamble(frameSizeDontCare))
            {
                offset = 0;
                return false;
            }
        if (ii.isFrameSetup())
            {
                offset = proc->getAddressWidth();
                return false;
            }
    }
    
#else
      frameChecker fc((const unsigned char*)(proc->getPtrToInstruction(addr)), range->get_size() - (addr - range->get_address()));
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
#endif
    return true;       
}

/**
 * If frame 'f' is a trampoline frame, this function returns true
 * if the trampoline was called by function entry or exit
 * instrumentation.
 **/
static bool isInEntryExitInstrumentation(Frame f)
{
   codeRange *range = f.getRange();
   miniTrampInstance *miniTI = range->is_minitramp();
   multiTramp *multi = range->is_multitramp();
   baseTrampInstance *baseTI = NULL;
   if (multi) baseTI = multi->getBaseTrampInstanceByAddr(f.getPC());

   if (baseTI == NULL)
   {
      if (miniTI == NULL)
         return false;
      baseTI = miniTI->baseTI;
   }
   const instPoint *instP = baseTI->baseT->instP();
   if (!instP) return false; // Could be iRPC

   if (instP->getPointType() == functionEntry ||
       instP->getPointType() == functionExit)
       // Not sure yet if function exit will be preInst or postInst...
       return true;

   return false;
}

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
    int_function *cur_func = getProc()->findFuncByAddr(pc_);
    int addr_size = getProc()->getAddressWidth();

#if defined(os_linux)
    // assume _start is never called, so just return if we're there
    if (cur_func &&
       cur_func->getAddress() == getProc()->getAOut()->parse_img()->getObject()->getEntryOffset()) {
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

   status = getFrameStatus(getProc(), getPC());

   if (status == frame_vsyscall)
   {
#if defined(os_linux)
      void *vsys_data;

      if ((vsys_data = getProc()->getVsyscallData()) == NULL)
      {
        /**
         * No vsyscall stack walking data present (we're probably 
         * on Linux 2.4) we'll go ahead and treat the vsyscall page 
         * as a leaf
         **/
        if (!getProc()->readDataSpace((void *) sp_, addr_size, 
                             (void *) &addrs.rtn, true))
          return Frame();
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
        Address reg_map[MAX_DW_VALUE+1];
        bool error;

        //Set up the register values array for getRegValueAtFrame
        memset(reg_map, 0, sizeof(reg_map));
        reg_map[DW_EBP] = fp_;
        reg_map[DW_ESP] = sp_;
        reg_map[DW_PC] = pc_;

        //Calc frame start
        reg_map[DW_CFA] = getRegValueAtFrame(vsys_data, pc_, DW_CFA, 
                                             reg_map, getProc(), &error);
        if (error) return Frame();

        //Calc registers values.
        newPC = getRegValueAtFrame(vsys_data, pc_, DW_PC, reg_map, 
                                   getProc(), &error);
        if (error) return Frame();
        newFP = getRegValueAtFrame(vsys_data, pc_, DW_EBP, reg_map, 
                                   getProc(), &error);
        if (error) return Frame();
        newSP = reg_map[DW_CFA];	
        goto done;
      }
#endif
   }
   else if (status == frame_sighandler)
   {
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
         return Frame();
      }
      if (!getProc()->readDataSpace((caddr_t)(sp_+pc_offset), addr_size,
                                    &addrs.rtn, true)) {
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
   else if ((status == frame_allocates_frame || status == frame_tramp))
   {
      /**
       * The function that created this frame uses the standard 
       * prolog: push %ebp; mov %esp->ebp .  We can read the 
       * appropriate data from the frame pointer.
       **/
      int offset = 0;
      // FIXME: for tramps, we need to check if we've saved the FP yet
      if ((status != frame_tramp && 
           !hasAllocatedFrame(pc_, getProc(), offset)) || 
          (prevFrameValid && isInEntryExitInstrumentation(prevFrame)))
      {
         addrs.fp = offset + sp_;
         if (!getProc()->readDataSpace((caddr_t) addrs.fp, addr_size, 
                               &addrs.rtn, true))
            return Frame();
         newPC = addrs.rtn;
         newFP = fp_;
         newSP = addrs.fp + getProc()->getAddressWidth();
         pcLoc = addrs.fp;
      }
      else
      {
         if (!fp_)
            return Frame();
         if (!getProc()->readDataSpace((caddr_t) fp_, addr_size, 
                                       &addrs.fp, false))
            return Frame();
         if (!getProc()->readDataSpace((caddr_t) (fp_ + addr_size), addr_size, 
                                       &addrs.rtn, false))
            return Frame();
         newFP = addrs.fp;
         newPC = addrs.rtn;
         newSP = fp_+ (2 * addr_size);
         pcLoc = fp_ + addr_size;
      }
      if (status == frame_tramp)
         newSP += getProc()->getAddressWidth() == 8 ? 
            tramp_pre_frame_size_64 : tramp_pre_frame_size_32;
      goto done;
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
             getFrameStatus(getProc(), estimated_ip) == frame_allocates_frame &&
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

         newPC = estimated_ip;
         newFP = estimated_fp;
         newSP = estimated_sp + getProc()->getAddressWidth();
         goto done;
      }
   }

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

