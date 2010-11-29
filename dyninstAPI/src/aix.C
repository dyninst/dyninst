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

// $Id: aix.C,v 1.247 2008/09/03 06:08:44 jaw Exp $

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include <termio.h>

#include <pthread.h>

#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/Dictionary.h"
#include "common/h/pathName.h"
#include "common/h/debugOstream.h"

#include "symtabAPI/h/Symtab.h"

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/symtab.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/inst-power.h" // Tramp constants
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_process.h"
#include "dyninstAPI/h/BPatch_function.h"

// FIXME remove and remove InstrucIter references
#include "parseAPI/src/InstrucIter.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "ast.h"

#if defined(cap_proc)
#include <sys/procfs.h>
#endif

#include "writeBackXCOFF.h"

#include "dyninstAPI/src/debug.h"

extern "C" {
extern int ioctl(int, int, ...);
};

bool PCProcess::skipHeap(const heapDescriptor &heap) {
    // MT: I've seen problems writing into a "found" heap that
    // is in the application heap (IE a dlopen'ed
    // library). Since we don't have any problems getting
    // memory there, I'm skipping any heap that is in 0x2.....

    if ((infHeaps[j].addr() > 0x20000000) &&
        (infHeaps[j].addr() < 0xd0000000) &&
        (infHeaps[j].type() == uncopiedHeap)) {
        infmalloc_printf("... never mind, AIX skipped heap\n");
        return true;
    }
    return false;
}

bool Frame::setPC(Address newpc) 
{
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
        if (!getProc()->writeDataSpace((void*)pcAddr_, sizeof(Address), 
                                  &newpc))
                  fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    }
    
    pc_ = newpc;
    range_ = NULL;
    return true;
}

#ifdef DEBUG 
void decodeInstr(unsigned instr_raw) 
{
  // Decode an instruction. Fun, eh?
  instruction i(instr_raw);

  switch(GENERIC_OP(i)) {
  case Bop:
    bperr( "Branch (abs=%d, link=%d) to 0x%x\n",
            IFORM_AA(i), IFORM_LK(i), IFORM_LI(i));
    break;
  case CMPIop:
    bperr( "CMPI reg(%d), 0x%x\n",
            DFORM_RA(i), DFORM_SI(i));
    break;
  case SIop:
    bperr( "SI src(%d), tgt(%d), 0x%x\n",
            DFORM_RA(i), DFORM_RT(i), DFORM_SI(i));
    break;
  case CALop:
    bperr( "CAL src(%d), tgt(%d), 0x%x\n",
            DFORM_RA(i), DFORM_RT(i), DFORM_SI(i));
    break;
  case CAUop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
            DFORM_RA(i), DFORM_RT(i), DFORM_SI(i));
    break;
  case ORILop:
    bperr( "ORIL src(%d), tgt(%d), 0x%x\n",
            DFORM_RT(i), DFORM_RA(i), DFORM_SI(i));
    break;
  case ANDILop:
    bperr( "CAU src(%d), tgt(%d), 0x%x\n",
            DFORM_RT(i), DFORM_RA(i), DFORM_SI(i));
    break;
  case Lop:
    bperr( "L src(%d)+0x%x, tgt(%d)\n",
            DFORM_RA(i), DFORM_SI(i), DFORM_RT(i));
    break;
  case STop:
    bperr( "L src(%d), tgt(%d)+0x%x\n",
            DFORM_RT(i), DFORM_RA(i), DFORM_SI(i));
    break;
  case BCop:
    bperr( "BC op(0x%x), CR bit(0x%x), abs(%d), link(%d), tgt(0x%x)\n",
            BFORM_BO(i), BFORM_BI(i), BFORM_AA(i), BFORM_LK(i), BFORM_BD(i));
    break;
  case BCLRop:
    switch (XFORM_XO(i)) {
    case BCLRxop:
      bperr( "BCLR op(0x%x), bit(0x%x), link(%d)\n",
              XFORM_RT(i), XFORM_RA(i), XFORM_RC(i));
      break;
    default:
      bperr( "%x\n", instr.asInt());
      break;
    }
    break;
  case 0:
    bperr( "NULL INSTRUCTION\n");
    break;
  default:
    bperr( "Unknown instr with opcode %d\n",
	    GENERIC_OP(i));

    break;
  }
  return;
}      

#endif

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) 
{
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL); 
  P_close (ttyfd);
}

#if defined(cap_dynamic_heap)

// 32-bit Address Space
// --------------------
// Segment 0 is kernel space, and off-limits
// Segment 1 is text space, and OK
// Segment 2-12 (c) is data space
// Segment 13 (d) is shared library text, and scavenged
// Segment 14 (e) is kernel space, and off-limits
// Segment 15 (f) is shared library data, and we don't care about it.
// However, we can scavenge some space in with the shared libraries.

static const Address lowest_addr32  = 0x10000000;
static const Address highest_addr32 = 0xe0000000;
static const Address data_hi_addr32 = 0xcfffff00;

// 64-bit Address Space
// --------------------
// 0x0000000000000000 -> 0x000000000fffffff Contains the kernel
// 0x00000000d0000000 -> 0x00000000dfffffff Contains 32-bit shared library text (inaccessable to 64-bit apps)
// 0x00000000e0000000 -> 0x00000000efffffff Shared memory segment available to 32-bit apps
// 0x00000000f0000000 -> 0x00000000ffffffff Contains 32-bit shared library data (inaccessable to 64-bit apps)
// 0x0000000100000000 -> 0x07ffffffffffffff Contains program data & text, plus shared memory or mmap segments
// 0x0800000000000000 -> 0x08ffffffffffffff Privately loaded 64-bit modules
// 0x0900000000000000 -> 0x09ffffffffffffff 64-bit shared library text & data
// 0x0f00000000000000 -> 0x0fffffffffffffff 64-bit application stack

#if defined(rs6000_ibm_aix64)
static const Address lowest_addr64  = 0x0000000100000000;
static const Address highest_addr64 = 0x0f00000000000000;
static const Address data_hi_addr64 = 0xffffffffffffffff;
// XXX
// According to the map above, data_hi_addr64 should probably be 0x0800000000000000.
// But, that causes us to write instrumentation into an invalid region of mutatee
// memory, though.  This should probably be fixed.
#endif

static const Address branch_range   = 0x01ff0000;
Address data_low_addr;

void process::inferiorMallocConstraints(Address near, Address &lo, 
					Address &hi, inferiorHeapType type)
{
   int addrWidth = getAddressWidth();
   Address lowest_addr = 0, highest_addr = 0, data_hi_addr = 0;

   switch (addrWidth) {
      case 4:	lowest_addr = lowest_addr32;
               highest_addr = highest_addr32;
               data_hi_addr = data_hi_addr32;
               break;
#if defined(rs6000_ibm_aix64)
      case 8:	lowest_addr = lowest_addr64;
               highest_addr = highest_addr64;
               data_hi_addr = data_hi_addr64;
               break;
#endif
      default:	assert(0 && "Unknown address width");
   }

   // The notion of "near" only works on 32-bit processes. (For now?)
   if (addrWidth == 4 && near) {
      if (near < (lowest_addr + branch_range))
         lo = lowest_addr;
      else
         lo = near - branch_range;

      if (near > (highest_addr - branch_range))
         hi = highest_addr;
      else
         hi = near + branch_range;
   }

   switch (type) {
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
            // Not sure why this is commented out.
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

   while (foundDup){
		foundDup = false;

                j =0;

      while (imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }

		VECTOR_ERASE(imagePatches,0,j-1);
		j=0;

      for (;j<imagePatches.size()-1;j++){
         if (imagePatches[j]->stopPage > imagePatches[j+1]->startPage){
				foundDup = true;

            if (imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
               imagePatches[j+1]->address = 0;	
            } else {
               imagePatches[j]->size = 
                  (imagePatches[j+1]->address + imagePatches[j+1]->size) -
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

   while (imagePatches[k]->address==0 && k < imagePatches.size()){
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

   for (;k<imagePatches.size();k++){
      if (imagePatches[k]->address!=0){
         if (_DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
         if (imagePatches[k]->startPage <= (unsigned int)stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
         } else {

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);

            if (_DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}
				finished = true;

				//was k+1	
            if (k < imagePatches.size()){
               while (imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}

					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;

               if (k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

   if (!finished) {
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);

      if (_DEBUG_MSG){
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

   while (foundDup){
		foundDup = false;
		j =0;
      while (imagePatches[j]->address==0 && j < imagePatches.size()){
       	        	j++;
        	}
		curr = imagePatches[j];

      for (j++;j<imagePatches.size();j++){
			next = imagePatches[j];		
         if (curr->address == next->address){
				//duplicate
				//find which is bigger and save that one.
            if (curr->size > next->size){
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

   if (DEBUG_MSG){
		bperr(" SORT 1 %d \n", imagePatches.size());
	
		for(unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}

	unsigned int endAddr;
   for (unsigned int i=0;i<imagePatches.size();i++){
      if (imagePatches[i]->address!=0){
			imagePatches[i]->startPage = imagePatches[i]->address- (imagePatches[i]->address%pageSize);
				
			endAddr = imagePatches[i]->address + imagePatches[i]->size;
			imagePatches[i]->stopPage =  endAddr - (endAddr % pageSize);

         if (DEBUG_MSG){
				bperr("%d address %x end addr %x : start page %x stop page %x \n",
					i,imagePatches[i]->address ,imagePatches[i]->address + imagePatches[i]->size,
					imagePatches[i]->startPage, imagePatches[i]->stopPage);
			}
		}
	
	}
	foundDup = true;

   while (foundDup){
		foundDup = false;
                j =0;
      while (imagePatches[j]->address==0 && j < imagePatches.size()){
                        j++;
                }

		//imagePatches.erase(0,j-1); //is it correct to erase here? 
		//j = 0;
      for (;j<imagePatches.size()-1;j++){ 
         if (imagePatches[j]->address!=0 && imagePatches[j]->stopPage >= imagePatches[j+1]->startPage){
				foundDup = true;
            if (imagePatches[j]->stopPage > imagePatches[j+1]->stopPage){
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

   if (DEBUG_MSG){
		bperr(" SORT 3 %d \n", imagePatches.size());

      for (unsigned int kk=0;kk<imagePatches.size();kk++){
			bperr("%d address 0x%x  size 0x%x \n",kk, imagePatches[kk]->address, imagePatches[kk]->size);
		}
		fflush(stdout);
	}
   while (imagePatches[k]->address==0 && k < imagePatches.size()){
	        k++;
        }

	startPage = imagePatches[k]->startPage;
	stopPage = imagePatches[k]->stopPage;
	int startIndex=k, stopIndex=k;
	bool finished = false;
   if (DEBUG_MSG){
		bperr("COMPACTING....\n");	
		bperr("COMPACTING %x %x %x\n", imagePatches[0]->startPage, stopPage, imagePatches[0]->address);
	}

   for (;k<imagePatches.size();k++){
      if (imagePatches[k]->address!=0){
         if (DEBUG_MSG){
				bperr("COMPACTING k[start] %x k[stop] %x stop %x addr %x size %x\n", imagePatches[k]->startPage, 
					imagePatches[k]->stopPage,stopPage, imagePatches[k]->address, imagePatches[k]->size);
			}
         if (imagePatches[k]->startPage <= (unsigned int) stopPage){
				stopIndex = k;
				stopPage = imagePatches[k]->stopPage;
         } else {

				patch = new imageUpdate;
				patch->address = imagePatches[startIndex]->address;
				patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address + 
						imagePatches[stopIndex]->size;
				newPatches.push_back(patch);

            if (DEBUG_MSG){
					bperr(" COMPACTED: address %x --> %x    start %x  stop %x\n", 
						patch->address, patch->size, startPage,  stopPage);
				}

				finished = true;

				//was k+1	
            if (k < imagePatches.size()){
               while (imagePatches[k]->address==0 && k < imagePatches.size()){
						k++;
					}
					startIndex = k;
					stopIndex = k;
					startPage = imagePatches[k]->startPage;
					stopPage  = imagePatches[k]->stopPage;
					finished = false;
               if (k == imagePatches.size()){
						finished = true;
					}
				} 
			}
		}

	}

   if (!finished){
		patch = new imageUpdate;
                patch->address = imagePatches[startIndex]->address;
                patch->size = imagePatches[stopIndex]->address - imagePatches[startIndex]->address +
                                   imagePatches[stopIndex]->size;
                newPatches.push_back(patch);

      if (DEBUG_MSG){
			bperr(" COMPACTED: %x --> %x \n", patch->address, patch->size);
		}
	}	
	
}

bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); return false; }
bool process::instrumentLibcStartMain() { assert(0); return false; }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); return false; }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); return false; }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); return 0; }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); return 0; }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); return 0; }
bool process::isMmapSysCall(int) { assert(0); return false; }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); return 0;}
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); return 0;}



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
                        instruction::size(), (char *)savedCodeBuffer)) {
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
        return false;
    }
    
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
    insnCodeGen::generateTrap(gen);

    if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    main_brk_addr = addr;
    
    return true;
}

bool AddressSpace::getDyninstRTLibName() 
{
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           std::string msg = std::string("Environment variable ")
              + std::string("DYNINSTAPI_RT_LIB")
              + std::string(" has not been defined");
           showErrorCallback(101, msg);
           return false;
        }
    }

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "_32";
    const char *name = dyninstRT_name.c_str();

    if (getAddressWidth() != sizeof(void *) && !P_strstr(name, modifier)) {
        const char *split = P_strrchr(name, '/');

        if (!split) split = name;
        split = P_strchr(split, '.');
        if (!split) {
            // We should probably print some error here.
            // Then, of course, the user will find out soon enough.
            return false;
        }

        dyninstRT_name = std::string(name, split - name) +
                         std::string(modifier) +
                         std::string(split);
    }

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
        + std::string(" does not exist or cannot be accessed!");
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

    startup_printf("[%d]: using address of 0x%lx for call to dlopen\n",
                   getPid(), codeBase);

    codeGen scratchCodeBuffer(BYTES_TO_SAVE);
    scratchCodeBuffer.setAddrSpace(this);
    scratchCodeBuffer.setAddr(codeBase);
    scratchCodeBuffer.setFunction(scratch);

    Address dyninstlib_addr = 0;
    Address dlopencall_addr = 0;
    
    // Do we want to save whatever is there? Can't see a reason why...
    
    // write library name...
    dyninstlib_addr = codeBase;

    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);
    
    // Need a register space
    // make sure this syncs with inst-power.C
    
    registerSpace *dlopenRegSpace = registerSpace::savedRegSpace(this);

    scratchCodeBuffer.setRegisterSpace(dlopenRegSpace);

    int_function *dlopen_func = findOnlyOneFunction("dlopen");
    if (!dlopen_func) {
        fprintf(stderr, "%s[%d]: ERROR: unable to find dlopen!\n",
                __FILE__, __LINE__);
        return false;
    }
    
    pdvector<AstNodePtr> dlopenAstArgs(2);
    AstNodePtr dlopenAst;
    
    dlopenAstArgs[0] = AstNode::operandNode(AstNode::Constant, (void *)(dyninstlib_addr));
    dlopenAstArgs[1] = AstNode::operandNode(AstNode::Constant, (void*)DLOPEN_MODE);

    dlopenAst = AstNode::funcCallNode(dlopen_func, dlopenAstArgs);


    dlopencall_addr = codeBase + scratchCodeBuffer.used();

    startup_printf("[%d]: call to dlopen starts at 0x%lx\n", getPid(), dlopencall_addr);

    // We need to push down the stack before we call this
    pushStack(scratchCodeBuffer);

    dlopenAst->generateCode(scratchCodeBuffer,
                            true);

    popStack(scratchCodeBuffer);

    dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
    insnCodeGen::generateTrap(scratchCodeBuffer);

    startup_printf("[%d]: call to dlopen breaks at 0x%lx\n", getPid(),
                   dyninstlib_brk_addr);

    readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    if (!writeDataSpace((void *)codeBase, scratchCodeBuffer.used(), 
                   scratchCodeBuffer.start_ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    
    // save registers
    assert(savedRegs == NULL);
    savedRegs = new dyn_saved_regs;
   assert(savedRegs != NULL);
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
    
    if (!writeDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);

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
           fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
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
           fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
            perror("get_exit_syscalls: read");
            return false;
        }
    }
    return true;
}



bool process::dumpCore_(const std::string coreFile)
{
    pause();
    
    if (!dumpImage(coreFile))
        return false;
    
    continueProc();
    return true;
}

bool process::dumpImage(const std::string outFile)
{
    // formerly OS::osDumpImage()
    const string &imageFileName = getAOut()->fullName();

	
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

bool SignalGenerator::decodeSignal_NP(EventRecord &ev)
{
  if (ev.type == evtSignalled && ev.what == SIGSTOP) {
    // On AIX we can't manipulate a process stopped on a
    // SIGSTOP... in any case, we clear it.
    // No other signal exhibits this behavior.
    ev.proc->getRepresentativeLWP()->clearSignal();
   }

  return false;  // signall needs further deccoding
}

bool SignalGeneratorCommon::getExecFileDescriptor(std::string filename,
                                                  int pid,
                                                  bool waitForTrap,
                                                  int &status,
                                                  fileDescriptor &desc) 
{
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
                if (pread(stat_fd, &pstatus, sizeof(pstatus), 0) != sizeof(pstatus)) {
                   fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
                    perror("pread failed while waiting for initial trap\n");
                }
                
                if ((pstatus.pr_lwp.pr_why == PR_SYSEXIT))
                    trapped = 1;
            }
            timeout++;
            usleep(1000);
        }
        if (!trapped) {
            // Hit the timeout, assume failure
            fprintf(stderr, "%s[%d][%s] Failed to open application /proc status FD:%s\n",
                    __FILE__, __LINE__, getThreadStr(getExecThreadID()),tempstr);
            return false;
        }
        status = SIGTRAP;
        // Explicitly don't close the FD
    }
    
    int map_fd;
    sprintf(tempstr, "/proc/%d/map", pid);
    map_fd = P_open(tempstr, O_RDONLY, 0);

    if (map_fd <= 0) {
        fprintf(stderr, "%s[%d]:  failed to open /proc/%d/map\n", __FILE__, __LINE__, pid);
        return false;
    }

    prmap_t text_map;
    char prog_name[512];
    prog_name[0] = '\0';

    prmap_t data_map;
    
    if (sizeof(prmap_t) != pread(map_fd, &text_map, sizeof(prmap_t), 0))
       fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
    pread(map_fd, prog_name, 512, text_map.pr_pathoff);
    if (prog_name[0] == '\0')
       fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
    //assert(text_map.pr_mflags & MA_MAINEXEC);
    
    if (sizeof(prmap_t) != pread(map_fd, &data_map, sizeof(prmap_t), sizeof(prmap_t)))
       fprintf(stderr, "%s[%d]:  pread: %s\n", FILE__, __LINE__, strerror(errno));
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

   /* name... We generate relative paths of exe with and without "./" at different places. 
      This creates multiple mapped file for the same file due to different paths. 
      We will simplify it by stripping the "./" */
   char text_name[256];
   char name[256];
   strcpy(text_name, filename.c_str());
   if (text_name[0] == '.')
    	strcpy(name,text_name+2);
   else
    	strcpy(name, text_name);
   desc = fileDescriptor(name,
                          textOrg,
                          dataOrg,
                          false); // Not a shared object
    // Try and track the types of descriptors created in aixDL.C...
	// We set this to the pathless file name (so that we can distinguish
	// exec'ed processes)
    desc.setMember(name);
    //desc.setPid(pid);
    
    return true;
}


void process::copyDanglingMemory(process *parent) {
    assert(parent);
    assert(parent->status() == stopped);
    assert(status() == stopped);

    // Copy everything in a heap marked "uncopied" over by hand
    pdvector<heapItem *> items = heap_.heapActive.values();
    for (unsigned i = 0; i < items.size(); i++) {
        if (items[i]->type == uncopiedHeap) {
            char *buffer = new char[items[i]->length];
            parent->readDataSpace((void *)items[i]->addr, items[i]->length,
                          buffer, true);
            if (! writeDataSpace((void *)items[i]->addr, 
                                  items[i]->length,
                                  buffer))
              fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
            delete [] buffer;
        }
    }
    // Odd... some changes _aren't_ copied.

    // Get all of the multiTramps and recopy the jumps
    // (copied from parent space)

    pdvector<codeRange *> mods;
    modifiedRanges_.elements(mods);

    for (unsigned j = 0; j < mods.size(); j++) {
        unsigned char buffer[mods[j]->get_size()];

        parent->readDataSpace((void *)mods[j]->get_address(),
                              mods[j]->get_size(),
                              (void *)buffer, true);
        if (!writeDataSpace((void *)mods[j]->get_address(),
                       mods[j]->get_size(),
                       (void *)buffer))
            fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
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

    if (isDynamic()) { 
        return NULL;
    }

    // Check if we parsed an intra-module static call
    assert(img_p_);
    image_func *icallee = img_p_->getCallee();
    if (icallee) {
      // Now we have to look up our specialized version
      // Can't do module lookup because of DEFAULT_MODULE...
      const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName().c_str());
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

    // We figure out the linkage by running a quick instructiter over the target...
    // Code is similar to archCheckEntry in image-power.C

    // TODO: encapsulate this in the instPoint so we can handle absolute branches
    // acceptably.

    if (!proc()->isValidAddress(callTarget())) {
        return NULL;
    }
        
#if !defined(cap_instruction_api)
    InstrucIter targetIter(callTarget(), proc());
    if (!targetIter.getInstruction().valid()) {
        return NULL;
    }
    Address toc_offset = 0;

    if (targetIter.isInterModuleCallSnippet(toc_offset)) {
#else
    using namespace Dyninst::InstructionAPI;
    const unsigned char* buffer = (const unsigned char*)(proc()->getPtrToInstruction(callTarget()));
    parsing_printf("Checking for linkage at addr 0x%lx\n", callTarget());
    InstructionDecoder d(buffer, 24, proc()->getArch());
    std::vector<Instruction::Ptr> insns;
    Instruction::Ptr tmp;
    while(tmp = d.decode()) insns.push_back(tmp);
    if(insns.size() != 6) return NULL;
    static RegisterAST::Ptr r2(new RegisterAST(ppc32::r2));
    static RegisterAST::Ptr r12(new RegisterAST(ppc32::r12));
    Address toc_offset = 0;
    if(insns[0]->getOperation().getID() != power_op_lwz) return NULL;
    if(insns[2]->getOperation().getID() != power_op_lwz) return NULL;
    if(insns[5]->getOperation().getID() != power_op_bcctr) return NULL;
    if(insns[0]->isWritten(r12) && insns[0]->isRead(r2))
    {
        std::vector<Expression::Ptr> tmp;
        insns[0]->getOperand(1).getValue()->getChildren(tmp);
        assert(tmp.size() == 1);
        Expression::Ptr child_as_expr = tmp[0];
        assert(child_as_expr);
        child_as_expr->bind(r2.get(), Result(u32, 0));
        toc_offset = child_as_expr->eval().convert<Address>();
#endif
        Address TOC_addr = (func()->obj()->parse_img()->getObject())->getTOCoffset();
        
        // We need to read out of memory rather than disk... so this is a call to
        // readDataSpace. Yummy.

        Address linkageAddr = 0;
        Address linkageTarget = 0;
        // Basically, load r12, <x>(r2)
        if (!proc()->readDataSpace((void *)(TOC_addr + toc_offset),
                                   sizeof(Address),
                                   (void *)&linkageAddr, false))
            return NULL;
        // And load r0, 0(r12)
        if (!proc()->readDataSpace((void *)linkageAddr,
                                   sizeof(Address),
                                   (void *)&linkageTarget, false))
            return NULL;

        if (linkageTarget == 0) {
            // No error for this one... looks like unloaded libs
            // do it.
            return NULL;
        }

        // Again, by definition, the function is not in owner.
        // So look it up.
        int_function *pdf = proc()->findFuncByAddr(linkageTarget);

        if (pdf) {
            callee_ = pdf;
            return callee_;
        }
        else
            return NULL;
    }
    return NULL;
}

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, std::string func, 
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

static bool initWrapperFunction(process *p, std::string symbol_name, std::string func_name)
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

static bool instrumentThrdFunc(int_function *dummy_create, std::string func_name, 
                               process *proc) 
{
   //Find create_thread
   pdvector<int_function *> thread_init_funcs;
   findThreadFuncs(proc, func_name.c_str(), thread_init_funcs);
   //findThreadFuncs(this, "init_func", &thread_init_funcs);
   if (!thread_init_funcs.size()) {
      return false;
   }
   //Instrument
   for (unsigned i=0; i<thread_init_funcs.size(); i++)
   {
      pdvector<AstNodePtr> args;
      AstNodePtr call_dummy_create = AstNode::funcCallNode(dummy_create, args);
      const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(call_dummy_create, callPreInsn, orderFirstAtPoint, false, 
               false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                  __FILE__, __LINE__);
         }
      }
   }   
   return true;
}

bool process::initMT()
{
   bool res;
   bool result = true;

   /** Check the magic environment variable.
    *  We want AIXTHREAD_SCOPE to be "S", which means map 1:1 pthread->kernel threads.
    *  If it isn't set, we can't track threads correctly.
    */
   char *thread_scope = getenv("AIXTHREAD_SCOPE");
   if ((thread_scope == NULL) ||
       (strcmp(thread_scope, "S") != 0)) {
       fprintf(stderr, "Error: multithread support requires the environment variable AIXTHREAD_SCOPE to be set to \"S\".\n");
       return false;
   }

   /**
    * Instrument thread_create with calls to DYNINST_dummy_create
    **/
   //Find DYNINST_dummy_create
   pdvector<int_function *> dummy_create_funcs;
   int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
   if (!dummy_create)
   {
      fprintf(stderr, "[%s:%d] - Could not find DYNINST_dummy_create\n",
              __FILE__, __LINE__);
      return false;
   }

   bool res1 = instrumentThrdFunc(dummy_create, "_pthread_body", this);
   bool res2 = instrumentThrdFunc(dummy_create, "_pthread_body_start", this);
   if (!res1 && !res2)
   {
      startup_printf("[%s:%u] - Error. Couldn't find any thread init functions " 
                     "to instrument\n", FILE__, __LINE__);
      result = false;
   }
   
   res = initWrapperFunction(this, "DYNINST_pthread_getthrds_np_record", 
                             "pthread_getthrds_np");
   if (!res) result = false;
   res = initWrapperFunction(this, "DYNINST_pthread_self_record", 
                             "pthread_self");
   if (!res) result = false;

   return result;
}
 
#include <sched.h>
void dyninst_yield()
{
   sched_yield();
}

bool SignalHandler::handleProcessExitPlat(EventRecord & /*ev*/, bool &) 
{
    return true;
}

bool process::hasPassedMain() 
{
   return true;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &/*remote*/)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &/*remote*/,
                   BPatch_Vector<unsigned int> &/*tlist*/)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &/*remote*/,
                   unsigned int /*pid*/, std::string &/*pidStr*/)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &/*remote*/)
{
    return true;
}
