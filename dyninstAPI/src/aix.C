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
 * excluded
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

// $Id: aix.C,v 1.206 2005/10/04 18:10:04 legendre Exp $

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include <termio.h>

#include <pthread.h>
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/Object.h"
#include "common/h/pathName.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/inst-power.h" // Tramp constants
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/InstrucIter.h"

#include "mapped_module.h"
#include "mapped_object.h"

#if defined(AIX_PROC)
#include <sys/procfs.h>
#include <poll.h>
#endif

#include "writeBackXCOFF.h"

#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"

extern "C" {
extern int ioctl(int, int, ...);
};

// The frame threesome: normal (singlethreaded), thread (given a pthread ID),
// and LWP (given an LWP/kernel thread).
// The behavior is identical unless we're in a leaf node where
// the LR is in a register, then it's different.

Frame Frame::getCallerFrame()
{
  typedef struct {
    unsigned oldFp;
    unsigned savedCR;
    unsigned savedLR;
    unsigned compilerInfo;
    unsigned binderInfo;
    unsigned savedTOC;
  } linkArea_t;
  
  const int savedLROffset=8;
  //const int compilerInfoOffset=12;
  
  linkArea_t thisStackFrame;
  linkArea_t lastStackFrame;
  linkArea_t stackFrame;
  Address basePCAddr;

  Address newPC=0;
  Address newFP=0;
  Address newpcAddr=0;

  // Are we in a leaf function?
  bool isLeaf = false;
  bool noFrame = false;

  codeRange *range = getRange();
  int_function *func = range->is_function();

  if (uppermost_) {
    if (func) {
      isLeaf = func->makesNoCalls();
      noFrame = func->hasNoStackFrame();
    }
  }

  // Get current stack frame link area
  if (!getProc()->readDataSpace((caddr_t)fp_, sizeof(linkArea_t),
                        (caddr_t)&thisStackFrame, false))
    return Frame();
  getProc()->readDataSpace((caddr_t) thisStackFrame.oldFp, sizeof(linkArea_t),
			   (caddr_t) &lastStackFrame, false);
  
  if (noFrame) {
    stackFrame = thisStackFrame;
    basePCAddr = fp_;
  }
  else {
    stackFrame = lastStackFrame;
    basePCAddr = thisStackFrame.oldFp;
  }

  // See if we're in instrumentation
  baseTrampInstance *bti = NULL;

  if (range->is_multitramp()) {
      bti = range->is_multitramp()->getBaseTrampInstanceByAddr(getPC());
      if (bti) {
          // If we're not in instru, then re-set this to NULL
          if (!bti->isInInstru(getPC()))
              bti = NULL;
      }
  }
  else if (range->is_minitramp()) {
      bti = range->is_minitramp()->baseTI;
  }
  if (bti) {
      // Oy. We saved the LR in the middle of the tramp; so pull it out
      // by hand.
      newpcAddr = fp_ + TRAMP_SPR_OFFSET + STK_LR;         
      newFP = thisStackFrame.oldFp;

      if (!getProc()->readDataSpace((caddr_t) newpcAddr,
                                    sizeof(Address),
                                    (caddr_t) &newPC, false))
          return Frame();

      // Instrumentation makes its own frame; we want to skip the
      // function frame if there is one as well.
      instPoint *point = bti->baseT->point();
      assert(point); // Will only be null if we're in an inferior RPC, which can't be.
      // If we're inside the function (callSite or arbitrary; bad assumption about
      // arbitrary but we don't know exactly where the frame was constructed) and the
      // function has a frame, tear it down as well.
      if ((point->getPointType() == callSite ||
          point->getPointType() == otherPoint) &&
          !point->func()->hasNoStackFrame()) {
          if (!getProc()->readDataSpace((caddr_t) thisStackFrame.oldFp,
                                        sizeof(unsigned),
                                        (caddr_t) &newFP, false))
              return Frame();
      }
      // Otherwise must be at a reloc insn
  }
  else if (isLeaf) {
      // isLeaf: get the LR from the register instead of saved location on the stack
      if (lwp_ && lwp_->get_lwp_id()) {
          dyn_saved_regs regs;
          bool status = lwp_->getRegisters(&regs);
          if (! status) {
              return Frame();
          }
          newPC = regs.theIntRegs.__lr;
          newpcAddr = (Address) 1; 
          /* I'm using an address to signify a register */
      }
      else if (thread_ && thread_->get_tid()) {
          cerr << "NOT IMPLEMENTED YET" << endl;
      }
      else { // normal
          dyn_saved_regs regs;
          bool status = getProc()->getRepresentativeLWP()->getRegisters(&regs);
          if (!status) {
              return Frame();
          }
          newPC = regs.theIntRegs.__lr;
	  newpcAddr = (Address) 1;
      }
      

      if (noFrame)
          newFP = fp_;
      else
          newFP = thisStackFrame.oldFp;
  }
  else {
      // Common case.
      newPC = stackFrame.savedLR;
      newpcAddr = basePCAddr + savedLROffset;
      if (noFrame)
	newFP = fp_;
      else
	newFP = thisStackFrame.oldFp;
  }

#ifdef DEBUG_STACKWALK
  fprintf(stderr, "PC %x, FP %x\n", newPC, newFP);
#endif
  return Frame(newPC, newFP, 0, newpcAddr, this);
}

bool Frame::setPC(Address newpc) {
    
    // Encapsulate all the logic necessary to set a new PC in a frame
    // If the function is a leaf function, we need to overwrite the LR directly.
    // The return addr for a frame is basically the parent's pc...
    
    if (pc_ == newpc) return true;
    
    if (isUppermost()) {
        if (getLWP())
            getLWP()->changePC(newpc, NULL);
        else if (getThread())
            getThread()->get_lwp()->changePC(newpc, NULL);
        else 
            getProc()->getRepresentativeLWP()->changePC(newpc, NULL);
    }
    else if (pcAddr_ == 1) {
        // Stomp the LR
        dyn_lwp *lwp = getLWP();
        if (lwp && lwp->get_lwp_id()) {
            // Get the current LR and reset it to our new version
            dyn_saved_regs regs;
            bool status = lwp->getRegisters(&regs);
            if (!status) {
                bperr( "Failure to get registers in catchupSideEffect\n");
                return false;
            }
            regs.theIntRegs.__lr = newpc;
            if (!lwp->restoreRegisters(regs)) {
                bperr( "Failure to restore registers in catchupSideEffect\n");
                return false;
            }
        }
        else {
            // Process-wide
            dyn_saved_regs regs;
            bool status = getProc()->getRepresentativeLWP()->getRegisters(&regs);
            if (!status) {
                bperr("Failure to get registers in catchupSideEffect\n");
                return false;
            }
            regs.theIntRegs.__lr = newpc;
            getProc()->getRepresentativeLWP()->restoreRegisters(regs);
        }
    }
    else {    
        // The LR is stored at pcAddr
        getProc()->writeDataSpace((void*)pcAddr_, sizeof(Address), 
                                  &newpc);
    }
    
    pc_ = newpc;
    range_ = NULL;
    return true;
}

#ifdef DEBUG 
void decodeInstr(unsigned instr_raw) {
  // Decode an instruction. Fun, eh?
  union instructUnion instr;
  instr.raw = instr_raw;

  switch(instr.generic.op) {
  case Bop:
    bperr( "Branch (abs=%d, link=%d) to 0x%x\n",
	    instr.iform.aa, instr.iform.lk, instr.iform.li);
    break;
  case CMPIop:
    bperr( "CMPI reg(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.d_or_si);
    break;
  case SIop:
    bperr( "SI src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CALop:
    bperr( "CAL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case CAUop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.ra, instr.dform.rt, instr.dform.d_or_si);
    break;
  case ORILop:
    bperr( "ORIL src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case ANDILop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case Lop:
    bperr( "L src(%d)+0x%x, tgt(%d)\n",
	    instr.dform.ra, instr.dform.d_or_si, instr.dform.rt);
    break;
  case STop:
    bperr( "L src(%d), tgt(%d)+0x%x\n",
	    instr.dform.rt, instr.dform.ra, instr.dform.d_or_si);
    break;
  case BCop:
    bperr( "BC op(0x%x), CR bit(0x%x), abs(%d), link(%d), tgt(0x%x)\n",
	    instr.bform.bo, instr.bform.bi, instr.bform.aa, instr.bform.lk, instr.bform.bd);
    break;
  case BCLRop:
    switch (instr.xform.xo) {
    case BCLRxop:
      bperr( "BCLR op(0x%x), bit(0x%x), link(%d)\n",
	      instr.xform.rt, instr.xform.ra, instr.xform.rc);
      break;
    default:
      bperr( "%x\n", instr.raw);
      break;
    }
    break;
  case 0:
    bperr( "NULL INSTRUCTION\n");
    break;
  default:
    bperr( "Unknown instr with opcode %d\n",
	    instr.generic.op);

    break;
  }
  return;
}      

#endif

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}


int getNumberOfCPUs()
{
  return(1);
}

// #include "paradynd/src/costmetrics.h"

#if defined(duplicated_in_process_c_because_linux_ia64_needs_it)
Address process::getTOCoffsetInfo(Address dest)
{
  // We have an address, and want to find the module the addr is
  // contained in. Given the probabilities, we (probably) want
  // the module dyninst_rt is contained in. 
  // I think this is the right func to use

  if (symbols->findFuncByAddr(dest, this))
    return (Address) (symbols->getObject()).getTOCoffset();

  if (shared_objects)
    for(u_int j=0; j < shared_objects->size(); j++)
      if (((*shared_objects)[j])->getImage()->findFuncByAddr(dest, this))
	return (Address) (((*shared_objects)[j])->getImage()->getObject()).getTOCoffset();
  // Serious error! Assert?
  return 0;
}
#endif

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address branch_range = 0x01fffffc;
static const Address lowest_addr = 0x10000000;
static const Address highest_addr = 0xe0000000;
Address data_low_addr;
static const Address data_hi_addr = 0xcfffff00;
// Segment 0 is kernel space, and off-limits
// Segment 1 is text space, and OK
// Segment 2-12 (c) is data space
// Segment 13 (d) is shared library text, and scavenged
// Segment 14 (e) is kernel space, and off-limits
// Segment 15 (f) is shared library data, and we don't care about it.
// However, we can scavenge some space in with the shared libraries.

void process::inferiorMallocConstraints(Address near, Address &lo, 
					Address &hi, inferiorHeapType type)
{
  lo = lowest_addr;
  hi = highest_addr;
  if (near)
  {
      if (near < (lowest_addr + branch_range))
          lo = lowest_addr;
      else
          lo = near - branch_range;
      if (near > (highest_addr - branch_range))
          hi = highest_addr;
      else
          hi = near + branch_range;
  }
  switch (type)
  {
 case dataHeap:
     // mmap, preexisting dataheap constraints
     // so shift down lo and hi accordingly
     if (lo < data_low_addr) {
         lo = data_low_addr;
         // Keep within branch range so that we know we can
         // reach anywhere inside.
         if (hi < (lo + branch_range))
             hi = lo + branch_range;
     }
     if (hi > data_hi_addr) {
         hi = data_hi_addr;
/*
         if (lo > (hi - branch_range))
             lo = hi - branch_range;
*/
     }
     break;
 default:
     // no change
     break;
  }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif


#define DEBUG_MSG 0 
#define _DEBUG_MSG 0
void compactLoadableSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){
	int startPage, stopPage;
	imageUpdate *patch;
	//this function now returns only ONE section that is loadable.
	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}


	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- imagePatches[i]->address%pageSize;
			imagePatches[i]->stopPage = imagePatches[i]->address + imagePatches[i]->size- 
					(imagePatches[i]->address + imagePatches[i]->size )%pageSize;

		}
	}

	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		imagePatches.erase(0,j-1);
		j=0;
		for(;j<imagePatches.size()-1;j++){
			if(imagePatches[j]->stopPage > imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{

					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					imagePatches[j]->stopPage = imagePatches[j]->address + imagePatches[j]->size-
                                        	(imagePatches[j]->address + imagePatches[j]->size )%pageSize;		
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[imagePatches.size()-1]->stopPage;
	int startIndex=k, stopIndex=imagePatches.size()-1;
	/*if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	patch = new imageUpdate;
        patch->address = imagePatches[startIndex]->address;
        patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
        newPatches.push_back(patch);
	if(DEBUG_MSG){
		bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
	}*/
	bool finished = false;
	if(_DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(_DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int)stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(_DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(_DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
			fflush(stdout);
		}
	}	

	
}

void compactSections(pdvector <imageUpdate*> imagePatches, pdvector<imageUpdate*> &newPatches){

	unsigned startPage, stopPage;
	imageUpdate *patch;

	int pageSize = getpagesize();

	imageUpdate *curr, *next;
	bool foundDup=true;
	unsigned int j;

	VECTOR_SORT(imagePatches, imageUpdateSort);

	while(foundDup){
		foundDup = false;
		j =0;
	        while(imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

		for(j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
			if(curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
				if(curr->size > next->size){
					next->address=0;
				}else{
					curr->address=0;
					curr=next;
				}
				foundDup =true;
			}else{
				curr=next;
			}

		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}
	if(DEBUG_MSG){
		bperr(" SORT 1 %d \n", imagePatches.size());
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int endAddr;
	for(unsigned int i=0;i<imagePatches.size();i++){
		if(imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			endAddr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  endAddr - (endAddr % pageSize);

			if(DEBUG_MSG){
				bperr("%d address %x end addr %x : start page %x stop page %x \n",
					i,imagePatches[i]->address ,imagePatches[i]->address + imagePatches[i]->size,
					imagePatches[i]->startPage, imagePatches[i]->stopPage);
			}
		}
	
	}
	foundDup = true;

	while(foundDup){
		foundDup = false;
                j =0;
                while(imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }
		//imagePatches.erase(0,j-1); //is it correct to erase here? 
		//j = 0;
		for(;j<imagePatches.size()-1;j++){ 
			if(imagePatches[j]->address!=0 && imagePatches[j]->stopPage >= imagePatches[j+1]->startPage){
				foundDup = true;
				if(imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
					imagePatches[j+1]->address = 0;	
				}else{
					imagePatches[j]->size = (imagePatches[j+1]->address + imagePatches[j+1]->size) -
						imagePatches[j]->address;
					imagePatches[j+1]->address = 0; 
					endAddr = imagePatches[j]->address + imagePatches[j]->size;
					imagePatches[j]->stopPage =  endAddr - (endAddr % pageSize);
				}
			}  
		}
		VECTOR_SORT(imagePatches, imageUpdateSort);
	}

	unsigned int k=0;

	if(DEBUG_MSG){
		bperr(" SORT 3 %d \n", imagePatches.size());

		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}
	while(imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[k]->stopPage;
	int startIndex=k, stopIndex=k;
	bool finished = false;
	if(DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}
	for(;k<imagePatches.size();k++){
		if(imagePatches[k]->address!=0){
			if(DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
			if(imagePatches[k]->startPage <= (unsigned int) stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
			}else{

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);
				if(DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;
				//was k+1	
				if(k < imagePatches.size()){
					while(imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
					if(k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

	if(!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);
		if(DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
		}
	}	
	
}


void process::addLib(char* lname){

	BPatch_process *appThread = BPatch::bpatch->getProcessByPid(getPid());
	BPatch_image *appImage = appThread->getImage();

   BPatch_Vector<BPatch_point *> *mainFunc;

	bool isTrampRecursive = BPatch::bpatch->isTrampRecursive();
    BPatch::bpatch->setTrampRecursive( true ); //ccw 31 jan 2003
    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction("main", bpfv) || !bpfv.size()) { 
      bperr("Unable to find function \"main\". Save the world will fail.\n");
      return;
   }

   BPatch_function *mainFuncPtr =bpfv[0];
   mainFunc = mainFuncPtr->findPoint(BPatch_entry);
    
   if (!mainFunc || ((*mainFunc).size() == 0)) {
      bperr( "    Unable to find entry point to \"main.\"\n");
      exit(1);
   }

   bpfv.clear();
   if (NULL == appImage->findFunction("dlopen", bpfv) || !bpfv.size()) {
      bperr("Unable to find function \"dlopen\". Save the world will fail.\n");
      return;
   }
   BPatch_function *dlopen_func = bpfv[0];
   
   BPatch_Vector<BPatch_snippet *> dlopen_args;
   BPatch_constExpr nameArg(lname);
   BPatch_constExpr rtldArg(4);
   
   dlopen_args.push_back(&nameArg);
   dlopen_args.push_back(&rtldArg);
   
   BPatch_funcCallExpr dlopenExpr(*dlopen_func, dlopen_args);
   
	//bperr(" inserting DLOPEN(%s)\n",lname);
	requestTextMiniTramp = 1;
   
   appThread->insertSnippet(dlopenExpr, *mainFunc, BPatch_callBefore,
                            BPatch_firstSnippet);
	requestTextMiniTramp = 0;
   
	BPatch::bpatch->setTrampRecursive( isTrampRecursive ); //ccw 31 jan 2003
}


//save world
char* process::dumpPatchedImage(pdstring imageFileName){ //ccw 28 oct 2001

	writeBackXCOFF *newXCOFF;
	//addLibrary *addLibraryXCOFF;
	//char name[50];	
	pdvector<imageUpdate*> compactedUpdates;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	void *data;//, *paddedData;
	//Address guardFlagAddr;
	char *directoryName = 0;

	if(!collectSaveWorldData){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::enableDumpPatchedImage() not called.  No mutated binary saved\n");
		return NULL;
	}

	directoryName = saveWorldFindDirectory();
	if(!directoryName){
		return NULL;
	}
	strcat(directoryName, "/");


	//at this point build an ast to call dlopen("libdyninstAPI_RT.so.1",);
	//and insert it at the entry point of main.

	addLib("libdyninstAPI_RT.so.1");

	
	imageUpdates.sort(imageUpdateSort);// imageUpdate::mysort ); 

	compactLoadableSections(imageUpdates,compactedUpdates);

	highmemUpdates.sort( imageUpdateSort);
	if(highmemUpdates.size() > 0){
		compactSections(highmemUpdates, compactedHighmemUpdates);
	}

	imageFileName = "dyninst_mutatedBinary";
	char* fullName =
      new char[strlen(directoryName) + strlen(imageFileName.c_str())+1];
   strcpy(fullName, directoryName);
   strcat(fullName, imageFileName.c_str());

	bool openFileError;

	newXCOFF = new writeBackXCOFF((const char *)(getAOut()->fullName().c_str()), fullName /*"/tmp/dyninstMutatee"*/ , openFileError);

	if( openFileError ){
		delete [] fullName;
		return NULL;
	}
	newXCOFF->registerProcess(this);
	//int sectionsAdded = 0;
	//unsigned int newSize, nextPage, paddedDiff;
	//unsigned int pageSize = getpagesize();


	//This adds the LOADABLE HEAP TRAMP sections
	//AIX/XCOFF NOTES:
	//On AIX we allocate the heap tramps in two locations: on the heap
	//(0x20000000) and around the text section (0x10000000) The os loader will
	//ONLY load ONE text section, ONE data section and ONE bss section. We
	//cannot (from within the mutated binary) muck with addresses in the range
	//0x10000000 - 0x1fffffff so to reload these tramps we MUST expand the
	//text section and tack these on the end.  THIS WILL INCREASE THE FILE
	//SIZE BY A HUGE AMOUNT.  The file size will increase by (sizeof(text
	//section) + sizeof(tramps) + (gap between text section and tramps)) the
	//gap may be quite large

	//SO we do NOT do what we do on the other platforms, ie work around the
	//heap with the compactedUpdates. we just expand the text section and 
	//tack 'em on the end.

	assert(compactedUpdates.size() < 2);
#if defined(__XLC__) || defined(__xlC__)
      // XLC does not like typecasts on the left hand side of "="
        char *data_c = new char[compactedUpdates[0]->size];
        data = (void *) data_c;
#else
        (char*) data = new char[compactedUpdates[0]->size];
#endif

	readDataSpace((void*) compactedUpdates[0]->address,
                 compactedUpdates[0]->size, data, true);	

	newXCOFF->attachToText(compactedUpdates[0]->address,
                          compactedUpdates[0]->size, (char*)data);

	if(compactedHighmemUpdates.size() > 0){
		saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates,
                                     (void*) newXCOFF);
	}
	saveWorldAddSharedLibs((void*) newXCOFF);

   saveWorldCreateDataSections((void*)newXCOFF);

	newXCOFF->createXCOFF();
	newXCOFF->outputXCOFF();
/*
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
   strcpy(fullName, directoryName);
   strcat(fullName, (char*)imageFileName.c_str());
   
   addLibraryXCOFF= new addLibrary(fullName, "/tmp/dyninstMutatee",
                                   "libdyninstAPI_RT.so.1");

	addLibraryXCOFF->outputXCOFF();
*/
   delete [] fullName;

	delete newXCOFF;

	return directoryName;
}

// should use demangle.h here, but header is badly broken on AIX 5.1
typedef void *Name;
typedef enum { VirtualName, MemberVar, Function, MemberFunction, Class,
               Special, Long } NameKind;
typedef enum { RegularNames = 0x1, ClassNames = 0x2, SpecialNames = 0x4,
               ParameterText = 0x8, QualifierText = 0x10 } DemanglingOptions;

Name *(*P_native_demangle)(char *, char **, unsigned long) = NULL;
char *(*P_functionName)(Name *) = NULL;
char *(*P_varName)(Name *) = NULL;
char *(*P_text)(Name *) = NULL;
NameKind (*P_kind)(Name *) = NULL;

void loadNativeDemangler() 
{
   P_native_demangle = NULL;
   
   void *hDemangler = dlopen("libdemangle.so.1", RTLD_LAZY|RTLD_MEMBER);
   if (hDemangler != NULL) {
      P_native_demangle = (Name*(*)(char*, char**, long unsigned int)) dlsym(hDemangler, "demangle");
      if (!P_native_demangle) 
         BPatch_reportError(BPatchSerious,122,
                   "unable to locate function demangle in libdemangle.so.1\n");

      P_functionName = (char*(*)(Name*)) dlsym(hDemangler, "functionName");
      if (!P_functionName) 
         BPatch_reportError(BPatchSerious,122,
               "unable to locate function functionName in libdemangle.so.1\n");
      
      P_varName = (char*(*)(Name*)) dlsym(hDemangler, "varName");
      if (!P_varName) 
         BPatch_reportError(BPatchSerious,122,
                    "unable to locate function varName in libdemangle.so.1\n");

      P_kind = (NameKind(*)(Name*)) dlsym(hDemangler, "kind");
      if (!P_kind) 
         BPatch_reportError(BPatchSerious,122,
                            "unable to locate function kind in libdemangle.so.1\n");
      
      P_text = (char*(*)(Name*)) dlsym(hDemangler, "text");
      if (!P_text) 
         BPatch_reportError(BPatchSerious,122,
                       "unable to locate function text in libdemangle.so.1\n");
   } 
}


extern "C" char *cplus_demangle(char *, int);
extern void dedemangle( const char * demangled, char * dedemangled );

#define DMGL_PARAMS      (1 << 0)       /* Include function args */
#define DMGL_ANSI        (1 << 1)       /* Include const, volatile, etc */

char * P_cplus_demangle( const char * symbol, bool nativeCompiler, bool includeTypes ) {
	/* If the symbol isn't from the native compiler, or the native demangler
	   isn't available, use the built-in. */
	bool nativeDemanglerAvailable =	P_cplus_demangle != NULL &&
									P_text != NULL &&
									P_varName != NULL &&
									P_functionName != NULL;
	if( !nativeCompiler || ! nativeDemanglerAvailable ) {
		char * demangled = cplus_demangle( const_cast<char *>(symbol),
					includeTypes ? DMGL_PARAMS | DMGL_ANSI : 0 );
		if( demangled == NULL ) { return NULL; }

		if( ! includeTypes ) {
			/* De-demangling never makes a string longer. */
			char * dedemangled = strdup( demangled );
			assert( dedemangled != NULL );

			dedemangle( demangled, dedemangled );
			assert( dedemangled != NULL );

			free( demangled );
			return dedemangled;
			}

		return demangled;
   } /* end if not using native demangler. */
   else if( nativeDemanglerAvailable ) {
		/* Use the native demangler, which apparently behaves funny. */
		Name * name;
		char * rest;
		
		/* P_native_demangle() won't actually demangled 'symbol'.
		   Find out what P_kind() of symbol it is and demangle from there. */
		name = (P_native_demangle)( const_cast<char*>(symbol), (char **) & rest,
			RegularNames | ClassNames | SpecialNames | ParameterText | QualifierText );
		if( name == NULL ) { return NULL; }

		char * demangled = NULL;
		switch( P_kind( name ) ) {
			case Function:
				demangled = (P_functionName)( name );			
				break;
			
			case MemberFunction:
				/* Doing it this way preserves the leading classnames. */
				demangled = (P_text)( name );
				break;

			case MemberVar:
				demangled = (P_varName)( name );
				break;

			case VirtualName:
			case Class:
			case Special:
			case Long:
				demangled = (P_text)( name );
				break;
			default: assert( 0 );
			} /* end P_kind() switch */

		/* Potential memory leak: no P_erase( name ) call.  Also, the
		   char *'s returned from a particular Name will be freed
		   when that name is erase()d or destroyed,	so strdup if we're
		   fond of them. */
   
		if( ! includeTypes ) {
			/* De-demangling never makes a string longer. */
			char * dedemangled = strdup( demangled );
			assert( dedemangled != NULL );

			dedemangle( demangled, dedemangled );
			assert( dedemangled != NULL );

			return dedemangled;
			}

		return demangled;
		} /* end if using native demangler. */
	else {
		/* We're trying to demangle a native binary but the native demangler isn't available.  Punt. */	
		return NULL;
		}
	} /* end P_cplus_demangle() */



#include <dlfcn.h> // dlopen constants

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

/*************************************************************************/
/***  Code to handle dlopen()ing the runtime library                   ***/
/***                                                                   ***/
/***  get_dlopen_addr() -- return the address of the dlopen function   ***/
/***  Address dyninstlib_brk_addr -- address of the breakpoint at the  ***/
/***                                 end of the RT init function       ***/
/***  Address main_brk_addr -- address when we switch to dlopen()ing   ***/
/***                           the RT lib                              ***/
/***  loadDYNINSTlib() -- Write the (string) name of the RT lib,     ***/
/***                        set up and execute a call to dlopen()      ***/
/***  trapDueToDyninstLib() -- returns true if trap addr is at         ***/
/***                          dyninstlib_brk_addr                      ***/
/***  trapAtEntryPointOfMain() -- see above                            ***/
/***  handleIfDueToDyninstLib -- cleanup function                      ***/
/***  handleTrapAtEntryPointOfMain -- cleanup function                 ***/
/***  insertTrapAtEntryPointOfMain -- insert a breakpoint at the start ***/
/***                                  of main                          ***/
/*************************************************************************/


/* Auxiliary function */

bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
    startup_printf("[%d] Checking all process threads for breakpoint 0x%x\n",
                   proc->getPid(), break_addr);
  pdvector<Frame> activeFrames;
  if (!proc->getAllActiveFrames(activeFrames)) return false;
  for (unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++) {
      if (activeFrames[frame_iter].getPC() == break_addr) {
          return true;
      }
  }
  return false;
}

bool process::trapDueToDyninstLib(dyn_lwp *lwp)
{
  // Since this call requires a PTRACE, optimize it slightly
  if (dyninstlib_brk_addr == 0x0) return false;

  Frame active = lwp->getActiveFrame();
  
  if (active.getPC() == dyninstlib_brk_addr)
      return true;

  return false;
}

bool process::trapAtEntryPointOfMain(dyn_lwp *lwp, Address)
{
  if (main_brk_addr == 0x0) return false;

  Frame active = lwp->getActiveFrame();
  if (active.getPC() == main_brk_addr)
      return true;
  return false;
}

/*
 * Restore "the original instruction" written into main so that
 * we can proceed after the trap. Saved in "savedCodeBuffer",
 * which is a chunk of space we use for dlopening the RT library.
 */

bool process::handleTrapAtEntryPointOfMain(dyn_lwp *)
{
    
    if (!main_brk_addr) return false;
    // Put back the original insn
    if (!writeDataSpace((void *)main_brk_addr, 
                        instruction::size(), (char *)savedCodeBuffer))
        return false;
    
    // And zero out the main_brk_addr so we don't accidentally
    // trigger on it.
    main_brk_addr = 0x0;
    return true;
}

/*
 * Stick a trap at the entry point of main. At this point,
 * libraries are mapped into the proc's address space, and
 * we can dlopen the RT library.
 */

bool process::insertTrapAtEntryPointOfMain()
{
    int_function *f_main = NULL;
    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("main", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"main() uninstrumentable");
        return false;
    }

    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one main! using the first" << endl;
    }
    f_main = funcs[0];
    assert(f_main);

    Address addr = f_main->getAddress();
    
    startup_printf("[%d]: inserting trap at 0x%x\n",
                   getPid(), addr);

    // save original instruction first
    readDataSpace((void *)addr, instruction::size(), savedCodeBuffer, true);
    // and now, insert trap
    codeGen gen(instruction::size());
    instruction::generateTrap(gen);

    writeDataSpace((void *)addr, gen.used(), gen.start_ptr());  
    main_brk_addr = addr;
    
    return true;
}

bool process::getDyninstRTLibName() {
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           pdstring msg = pdstring("Environment variable ")
              + pdstring("DYNINSTAPI_RT_LIB")
              + pdstring(" has not been defined for process ")
              + pdstring(pid);
           showErrorCallback(101, msg);
           return false;
        }
    }
    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        pdstring msg = pdstring("Runtime library ") + dyninstRT_name
        + pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }
    return true;
}


  

/*
 * loadDYNINSTlib()
 *
 * The evil black magic function. What we want: for the runtime
 * library to show up in the process' address space. Magically.
 * No such luck. What we do: patch in a call to dlopen(DYNINSTRT_NAME)
 * at the entry of main, then restore the original instruction
 * and continue.
 */

bool process::loadDYNINSTlib()
{
    // We use the address of main() to load the library, saving what's
    // there and restoring later
    
    // However, if we can get code_len_ + code_off_ from the object file,
    // then we can use the area above that point freely.
    
    // Steps: Get the library name (command line or ENV)
    //        Get the address for dlopen()
    //        Write in a call to dlopen()
    //        Write in a trap after the call
    //        Write the library name somewhere where dlopen can find it.
    // Actually, why not write the library name first?

    int_function *scratch = findOnlyOneFunction("main");
    if (!scratch) return false;
    // Testing...
    // Round it up to the nearest instruction. 
    Address codeBase = scratch->getAddress();

    startup_printf("[%d]: using address of 0x%x for call to dlopen\n",
                   getPid(), codeBase);

    codeGen scratchCodeBuffer(BYTES_TO_SAVE);
    Address dyninstlib_addr = 0;
    Address dlopencall_addr = 0;
    
    // Do we want to save whatever is there? Can't see a reason why...
    
    // write library name...
    dyninstlib_addr = codeBase;

    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);
    
    // Need a register space
    // make sure this syncs with inst-power.C
    Register liveRegList[] = {10, 9, 8, 7, 6, 5, 4, 3};
    Register deadRegList[] = {11, 12};
    unsigned liveRegListSize = sizeof(liveRegList)/sizeof(Register);
    unsigned deadRegListSize = sizeof(deadRegList)/sizeof(Register);
    
    registerSpace *dlopenRegSpace = new registerSpace(deadRegListSize, deadRegList, 
                                                      liveRegListSize, liveRegList);
    dlopenRegSpace->resetSpace();
    
    pdvector<AstNode*> dlopenAstArgs(2);
    AstNode *dlopenAst;
    
    dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
    dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
    dlopenAst = new AstNode("dlopen", dlopenAstArgs);
    removeAst(dlopenAstArgs[0]);
    removeAst(dlopenAstArgs[1]);

    dlopencall_addr = codeBase + scratchCodeBuffer.used();

    startup_printf("[%d]: call to dlopen starts at 0x%x\n", getPid(), dlopencall_addr);

    // We need to push down the stack before we call this
    pushStack(scratchCodeBuffer);

    dlopenAst->generateCode(this, dlopenRegSpace, scratchCodeBuffer,
                            true, true);
    removeAst(dlopenAst);

    popStack(scratchCodeBuffer);

    dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
    instruction::generateTrap(scratchCodeBuffer);

    startup_printf("[%d]: call to dlopen breaks at 0x%x\n", getPid(),
                   dyninstlib_brk_addr);

    readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    writeDataSpace((void *)codeBase, scratchCodeBuffer.used(), 
                   scratchCodeBuffer.start_ptr());
    
    // save registers
    assert(savedRegs == NULL);
    savedRegs = new dyn_saved_regs;
    bool status = getRepresentativeLWP()->getRegisters(savedRegs);
    assert((status!=false) && (savedRegs!=(void *)-1));

    assert(dlopencall_addr);
    assert(dyninstlib_brk_addr);

    if (!getRepresentativeLWP()->changePC(dlopencall_addr, NULL)) {
        logLine("WARNING: changePC failed in loadDYNINSTlib\n");
        assert(0);
    }
    
    setBootstrapState(loadingRT_bs);
    return true;
}

/*
 * Cleanup after dlopen()ing the runtime library. Since we don't overwrite
 * any existing functions, just restore saved registers. Cool, eh?
 */

bool process::loadDYNINSTlibCleanup(dyn_lwp *lwp)
{
    lwp->restoreRegisters(*savedRegs);
    delete savedRegs;
    savedRegs = NULL;
    // We was never here.... 

    int_function *scratch = findOnlyOneFunction("main");
    if (!scratch) return false;
    // Testing...
    // Round it up to the nearest instruction. 
    Address codeBase = scratch->getAddress();
    
    writeDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer);

    // But before we go, reset the dyninstlib_brk_addr so we don't
    // accidentally trigger it, eh?
    dyninstlib_brk_addr = 0x0;
    return true;
}


//////////////////////////////////////////////////////////
// AIX /proc compatibility section
/////////////////////////////////////////////////////////

#define GETREG_GPR(regs,reg)   (regs.__gpr[reg])
// AIX system calls can vary in name and number. We need a way
// to decode this mess. So what we do is #define the syscalls we 
// want to numbers and use those to index into a mapping array.
// The first time we try and map a syscall we fill the array in.

int SYSSET_MAP(int syscall, int pid)
{
    static int syscall_mapping[NUM_SYSCALLS];
    static bool mapping_valid = false;
    
    if (mapping_valid)
        return syscall_mapping[syscall];
    
    for (int i = 0; i < NUM_SYSCALLS; i++)
        syscall_mapping[i] = -1;
    
    // Open and read the sysent file to find exit, fork, and exec.
    prsysent_t sysent;
    prsyscall_t *syscalls;
    int fd;
    char filename[256];
    char syscallname[256];
    sprintf(filename, "/proc/%d/sysent", pid);
    fd = open(filename, O_RDONLY, 0);
    if (read(fd, &sysent,
             sizeof(sysent) - sizeof(prsyscall_t))
        != sizeof(sysent) - sizeof(prsyscall_t))
        perror("AIX syscall_map: read");
    syscalls = (prsyscall_t *)malloc(sizeof(prsyscall_t)*sysent.pr_nsyscalls);
    if (read(fd, syscalls, sizeof(prsyscall_t)*sysent.pr_nsyscalls) !=
        (int) (sizeof(prsyscall_t) * sysent.pr_nsyscalls))
        perror("AIX syscall_map: read2");
    for (unsigned int j = 0; j < (unsigned) sysent.pr_nsyscalls; j++) {
        lseek(fd, syscalls[j].pr_nameoff, SEEK_SET);
        read(fd, syscallname, 256);
        // Now comes the interesting part. We're interested in a list of
        // system calls. Compare the freshly read name to the list, and if
        // there is a match then set the syscall mapping.
        if (!strcmp(syscallname, "_exit")) {
            syscall_mapping[SYS_exit] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "kfork")) {
            syscall_mapping[SYS_fork] = syscalls[j].pr_number;
        }
        else if (!strcmp(syscallname, "execve")) {    
            syscall_mapping[SYS_exec] = syscalls[j].pr_number;
        }
        if (!strcmp(syscallname, "thread_terminate_unlock")) {
           syscall_mapping[SYS_lwp_exit] = syscalls[j].pr_number;
        }
    }
    P_close(fd);
    free(syscalls);
    mapping_valid = true;
    return syscall_mapping[syscall];
}

// Bleah...
unsigned SYSSET_SIZE(sysset_t *x)
{
    // (pr_size - 1) because sysset_t is one uint64_t too large
    return sizeof(sysset_t) + (sizeof (uint64_t) * (x->pr_size-1));
}

sysset_t *SYSSET_ALLOC(int pid)
{
    static bool init = false;
    static int num_calls = 0;
    if (!init) {
        prsysent_t sysent;
        int fd;
        char filename[256];
        sprintf(filename, "/proc/%d/sysent", pid);
        fd = open(filename, O_RDONLY, 0);
        if (read(fd, &sysent,
                 sizeof(sysent) - sizeof(prsyscall_t))
            != (int) (sizeof(sysent) - sizeof(prsyscall_t)))
            perror("AIX syscall_alloc: read");
        num_calls = sysent.pr_nsyscalls;
        init = true;
        P_close(fd);
    }
    int size = 0; // Number of 64-bit ints we use for the bitmap
    // array size (*8 because we're bitmapping)
    size = ((num_calls / (8*sizeof(uint64_t))) + 1);
    sysset_t *ret = (sysset_t *)malloc(sizeof(sysset_t) 
                                       - sizeof(uint64_t) 
                                       + size*sizeof(uint64_t));

    ret->pr_size = size;
    
    return ret;
}

/*
 * The set operations (set_entry_syscalls and set_exit_syscalls) are defined
 * in sol_proc.C
 */

bool process::get_entry_syscalls(sysset_t *entry)
{
    pstatus_t status;
    if (!get_status(&status)) return false;
    
    // If the offset is 0, no syscalls are being traced
    if (status.pr_sysentry_offset == 0) {
        premptysysset(entry);
    }
    else {
        // The entry member of the status vrble is a pointer
        // to the sysset_t array.
        if(pread(getRepresentativeLWP()->status_fd(), entry, 
                 SYSSET_SIZE(entry), status.pr_sysentry_offset)
           != (int) SYSSET_SIZE(entry)) {
            perror("get_entry_syscalls: read");
            return false;
        }
    }
    return true;
}

bool process::get_exit_syscalls(sysset_t *exit)
{
    pstatus_t status;
    if (!get_status(&status)) return false;

    // If the offset is 0, no syscalls are being traced
    if(status.pr_sysexit_offset == 0) {
        premptysysset(exit);
    }
    else {
        if(pread(getRepresentativeLWP()->status_fd(), exit, 
                 SYSSET_SIZE(exit), status.pr_sysexit_offset)
           != (int) SYSSET_SIZE(exit)) {
            perror("get_exit_syscalls: read");
            return false;
        }
    }
    return true;
}



bool process::dumpCore_(const pdstring coreFile)
{
    pause();
    
    if (!dumpImage(coreFile))
        return false;
    
    continueProc();
    return true;
}

bool process::dumpImage(const pdstring outFile)
{
    // formerly OS::osDumpImage()
    const pdstring &imageFileName = getAOut()->fileName();
    // const Address codeOff = symbols->codeOffset();
    int i;
    int rd;
    int ifd;
    int ofd;
    int cnt;
    int total;
    int length;

    char buffer[4096];
    struct filehdr hdr;
    struct stat statBuf;
    struct aouthdr aout;
    struct scnhdr *sectHdr;
    bool needsCont = false;

    ifd = open(imageFileName.c_str(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", imageFileName.c_str());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;
    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    cnt = read(ifd, &hdr, sizeof(struct filehdr));
    if (cnt != sizeof(struct filehdr)) {
	sprintf(errorLine, "Error reading header\n");
	logLine(errorLine);
	showErrorCallback(44, (const char *) errorLine);
	return false;
    }

    cnt = read(ifd, &aout, sizeof(struct aouthdr));

    sectHdr = (struct scnhdr *) calloc(sizeof(struct scnhdr), hdr.f_nscns);
    cnt = read(ifd, sectHdr, sizeof(struct scnhdr) * hdr.f_nscns);
    if ((unsigned) cnt != sizeof(struct scnhdr)* hdr.f_nscns) {
	sprintf(errorLine, "Section headers\n");
	logLine(errorLine);
	return false;
    }

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += 4096) {
        rd = read(ifd, buffer, 4096);
        write(ofd, buffer, rd);
        total += rd;
    }

    if (!stopped) {
        // make sure it is stopped.
        pause();
        needsCont = true;
    }
    
    Address baseAddr = getAOut()->codeAbs();

    sprintf(errorLine, "seeking to 0x%lx as the offset of the text segment \n",
            baseAddr);
    logLine(errorLine);
    sprintf(errorLine, "Code offset = 0x%lx\n", baseAddr);
    logLine(errorLine);
    

    lseek(ofd, aout.text_start, SEEK_SET);
    
    /* Copy the text segment over */
    for (i = 0; i < aout.tsize; i += 4096) {
        length = ((i + 4096) < aout.tsize) ? 4096 : aout.tsize-i;
        readDataSpace((void *) (baseAddr+i), length, (void *)buffer, false);

        
        write(ofd, buffer, length);
    }
    
    if (needsCont) {
        continueProc();
    }
    
    P_close(ofd);
    P_close(ifd);
    
    return true;

}

bool process::getExecFileDescriptor(pdstring /*filename*/,
                                    int pid,
                                    bool waitForTrap,
                                    int &status,
                                    fileDescriptor &desc) {
    // AIX's /proc has a map file which contains data about
    // files loaded into the address space. The first two appear
    // to be the text and data from the process. We'll take it,
    // making sure that the filenames match.

    // Amusingly, we don't use the filename parameter; we need to match with
    // later /proc reads, and little things like pathnames kinda get in the way.
    

    char tempstr[256];

    // Can get rid of this; actually, move to attach.

    if (waitForTrap) {
        
        pstatus_t pstatus;
        int trapped = 0;
        int timeout = 0;
        int stat_fd = 0;
        sprintf(tempstr, "/proc/%d/status", pid);
        
        while (!trapped &&
               (timeout < 10000) // 10 seconds, _should_ be enough
               ) {
            
            // On slower machines (sp3-cw.cs.wisc.edu) we can enter
            // this code before the forked child has actually been created.
            // We attempt to re-open the FD if it failed the first time.
            if (stat_fd <= 0) 
                stat_fd = P_open(tempstr, O_RDONLY, 0);
            if (stat_fd > 0) {
                if (pread(stat_fd, &pstatus, sizeof(pstatus), 0) != sizeof(pstatus))
                    perror("pread failed while waiting for initial trap\n");
                
                if ((pstatus.pr_lwp.pr_why == PR_SYSEXIT))
                    trapped = 1;
            }
            timeout++;
            usleep(1000);
        }
        if (!trapped) {
            // Hit the timeout, assume failure
            fprintf(stderr, "Failed to open application /proc status FD\n");
            return false;
        }
        status = SIGTRAP;
        // Explicitly don't close the FD
    }
    
    int map_fd;
    sprintf(tempstr, "/proc/%d/map", pid);
    map_fd = P_open(tempstr, O_RDONLY, 0);

    if (map_fd <= 0)
        return false;

    prmap_t text_map;
    char text_name[512];
    prmap_t data_map;
    
    pread(map_fd, &text_map, sizeof(prmap_t), 0);
    pread(map_fd, text_name, 512, text_map.pr_pathoff);
    //assert(text_map.pr_mflags & MA_MAINEXEC);
    
    pread(map_fd, &data_map, sizeof(prmap_t), sizeof(prmap_t));
    if (!(data_map.pr_mflags & MA_MAINEXEC))
        data_map.pr_vaddr = 0;
    
    // We assume text = entry 0, data = entry 1
    // so they'll have the same names

    Address textOrg = (Address) text_map.pr_vaddr;
    Address dataOrg = (Address) data_map.pr_vaddr;

    // Round up to next multiple of wordsize;
    
    if (textOrg % sizeof(unsigned))
        textOrg += sizeof(unsigned) - (textOrg % sizeof(unsigned));
    if (dataOrg % sizeof(unsigned))
        dataOrg += sizeof(unsigned) - (dataOrg % sizeof(unsigned));

   // Here's a fun one. For the a.out file, we normally have the data in
   // segment 2 (0x200...). This is not always the case. Programs compiled
   // with the -bmaxdata flag have the heap in segment 3. In this case, change
   // the lower bound for the allocation constants in aix.C.
   extern Address data_low_addr;
   if (dataOrg >= 0x30000000)
       data_low_addr = 0x30000000;
   else
       data_low_addr = 0x20000000;

    // Check if text_name substring matches filename?

    desc = fileDescriptor(text_name,
                          textOrg,
                          dataOrg,
                          false); // Not a shared object
    // Try and track the types of descriptors created in aixDL.C...
    desc.setMember("");
    //desc.setPid(pid);
    
    return true;
}


void process::copyDanglingMemory(process *parent) {
    assert(parent);
    assert(parent->status() == stopped);
    assert(status() == stopped);

    // Copy everything in a heap marked "uncopied" over by hand
    pdvector<heapItem *> items = heap.heapActive.values();
    for (unsigned i = 0; i < items.size(); i++) {
        if (items[i]->type == uncopiedHeap) {
            char *buffer = new char[items[i]->length];
            parent->readDataSpace((void *)items[i]->addr, items[i]->length,
                          buffer, true);
            writeDataSpace((void *)items[i]->addr, 
                                  items[i]->length,
                                  buffer);
            delete [] buffer;
        }
    }
    // Odd... some changes _aren't_ copied.

    // Get all of the multiTramps and recopy the jumps
    // (copied from parent space)

    pdvector<codeRange *> mods;
    modifiedAreas_.elements(mods);

    for (unsigned j = 0; j < mods.size(); j++) {
        unsigned char buffer[mods[j]->get_size_cr()];

        parent->readDataSpace((void *)mods[j]->get_address_cr(),
                              mods[j]->get_size_cr(),
                              (void *)buffer, true);
        writeDataSpace((void *)mods[j]->get_address_cr(),
                       mods[j]->get_size_cr(),
                       (void *)buffer);
    }
}


// findCallee
int_function *instPoint::findCallee() {
    if (callee_) {
        return callee_;
    }

    if (ipType_ != callSite) {
        return NULL;
    }

    if (isDynamicCall()) { 
        return NULL;
    }

    // Check if we parsed an intra-module static call
    assert(img_p_);
    image_func *icallee = img_p_->getCallee();
    if (icallee) {
      // Now we have to look up our specialized version
      // Can't do module lookup because of DEFAULT_MODULE...
      const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName());
      if (!possibles) {
          return NULL;
      }
      for (unsigned i = 0; i < possibles->size(); i++) {
          if ((*possibles)[i]->ifunc() == icallee) {
              callee_ = (*possibles)[i];
              return callee_;
          }
      }
      // No match... very odd
      assert(0);
      return NULL;
  }


    // Other possibilities: call through a function pointer,
    // or a inter-module call. We handle inter-module calls as
    // a static function call, since they're bound at load time.

    // Or module == glink.s == "Global_Linkage"
    if (func()->prettyName().suffixed_by("_linkage")) {
        // Make sure we're not mistaking a function named
        // *_linkage for global linkage code. 
        
        if ((*insn_).raw != 0x4e800420) // BCTR
            return NULL;
        Address TOC_addr = (func()->obj()->parse_img()->getObject()).getTOCoffset();

        // Love the fixed-length platforms; we can go backwards.
        InstrucIter linkIter(addr(), proc());
        
        // Linkage code looks like a bunch of loads, a move, then a jump to CTR.
        // Find the load.

        Address toc_offset = 0;
        // Don't run off the function
        while (*linkIter > func()->getAddress()) {
            instruction inst = linkIter.getInstruction();
            if (((*inst).dform.op == Lop) &&
                ((*inst).dform.rt == 12) &&
                ((*inst).dform.ra == 2)) {
                if ((*linkIter) != (addr() - 20)) {
                    fprintf(stderr, "Odd addr for linkage load: 0x%x, for call at 0x%x, in func %s\n",
                            *linkIter, addr(), func()->prettyName().c_str());
                }
                toc_offset = (*inst).dform.d_or_si;
                break;
            }
            linkIter--;
        }

        if (toc_offset) {
            // This should be the contents of R12 in the linkage function

            void *callee_TOC_ptr = func()->obj()->getPtrToData(TOC_addr + toc_offset);
            Address callee_TOC_entry = *((Address *)callee_TOC_ptr);

            // We need to find what object the callee TOC entry is defined in. This will be the
            // same place we find the function, later.
            Address callee_addr = 0;
            
            const pdvector<mapped_object *> &m_objs = proc()->mappedObjects();
            
            for (unsigned i = 0; i < m_objs.size(); i++) {
                if ((callee_TOC_entry <= m_objs[i]->dataAbs()) &&
                    (callee_TOC_entry < (m_objs[i]->dataAbs() + m_objs[i]->dataSize()))) {
                    // Found it. So grab the target
                    void *toc_ptr = m_objs[i]->getPtrToData(callee_TOC_entry);
                    callee_addr = *((Address *)toc_ptr);
                    break;
                }
            }
            
            if (!callee_addr) return NULL;
            // callee_addr: address of function called, contained in image callee_img
            // Sanity check on callee_addr
            if ((callee_addr < 0x20000000) ||
                (callee_addr > 0xdfffffff)) {
                if (callee_addr != 0) { // unexpected -- where is this function call? Print it out.
                    bperr( "Skipping illegal address 0x%x in function %s\n",
                           (unsigned) callee_addr, func()->prettyName().c_str());
                }
                return NULL;
            }
            
            // Again, by definition, the function is not in owner.
            // So look it up.
            int_function *pdf = 0;
            codeRange *range = proc()->findCodeRangeByAddress(callee_addr);
            pdf = range->is_function();
            
            if (pdf)
                {
                    callee_ = pdf;
                    return callee_;
                }
            else
                bperr( "Couldn't find target function for address 0x%x, jump at 0x%x\n",
                       (unsigned) callee_addr,
                       (unsigned) addr());
        }
    }
    return NULL;
}

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, pdstring func, 
                            pdvector<int_function *> &result)
{
   bool found = false;
   mapped_module *lpthread = p->findModule("libpthread.a", true);
   if (lpthread)
      found = lpthread->findFuncVectorByPretty(func, result);
   if (found)
      return;

   mapped_module *lc = p->findModule("libc.a", true);
   if (lc)
      found = lc->findFuncVectorByPretty(func, result);
   if (found)
      return;
   
   p->findFuncsByPretty(func, result);
}

static bool initWrapperFunction(process *p, pdstring symbol_name, pdstring func_name)
{
   //Find symbol_name
   bool res;
   pdvector<int_variable *> dyn_syms;
   res = p->findVarsByAll(symbol_name, dyn_syms);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't find any %s, expected 1\n", 
              __FILE__, __LINE__, symbol_name.c_str());
      return false;
   }
   Address sym_addr = dyn_syms[0]->getAddress();

   //Find func_name
   pdvector<int_function *> funcs;
   findThreadFuncs(p, func_name, funcs);   
   if (funcs.size() != 1)
   {
      fprintf(stderr, "[%s:%d] - Found %d %s functions, expected 1\n",
              __FILE__, __LINE__, funcs.size(), func_name.c_str());
      return false;
   }   
   //Replace
   res = writeFunctionPtr(p, sym_addr, funcs[0]);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't update %s\n",
              __FILE__, __LINE__, symbol_name.c_str());
      return false;
   }
   return true;
}

bool process::initMT()
{
   unsigned i;
   bool res;

   /**
    * Instrument thread_create with calls to DYNINST_dummy_create
    **/
   //Find create_thread
   pdvector<int_function *> thread_init_funcs;
   findThreadFuncs(this, "_pthread_body", thread_init_funcs);
   //findThreadFuncs(this, "init_func", &thread_init_funcs);
   if (thread_init_funcs.size() < 1)
   {
      fprintf(stderr, "[%s:%d] - Found no copies of pthread_body, expected 1\n",
              __FILE__, __LINE__);
      return false;
   }
   //Find DYNINST_dummy_create
   pdvector<int_function *> dummy_create_funcs;
   int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
   if (!dummy_create)
   {
      fprintf(stderr, "[%s:%d] - Could not find DYNINST_dummy_create\n",
              __FILE__, __LINE__);
      return false;
   }
   //Instrument
   for (i=0; i<thread_init_funcs.size(); i++)
   {
      pdvector<AstNode *> args;
      AstNode call_dummy_create(dummy_create, args);
      AstNode *ast = &call_dummy_create;
      const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false, 
                                 false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                    __FILE__, __LINE__);
         }
         //TODO: Save the mt objects for detach
      }
   }
   
   res = initWrapperFunction(this, "DYNINST_pthread_getthrds_np_record", 
                             "pthread_getthrds_np");
   if (!res) return false;
   res = initWrapperFunction(this, "DYNINST_pthread_self_record", 
                             "pthread_self");
   if (!res) return false;

   return true;
}
 
#include <sched.h>
void dyninst_yield()
{
   sched_yield();
}
