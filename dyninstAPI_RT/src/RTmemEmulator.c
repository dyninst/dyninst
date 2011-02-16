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

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined (__GNUC__)
#include <unistd.h>
#define FAST_CALL __attribute__((fastcall)) 
#elif defined (os_windows)
#define FAST_CALL __fastcall
#endif

/* Code to assist in remapping memory operations that were affected
 * by our instrumentation */

struct MemoryMapper RTmemoryMapper = {0, 0, 0, 0};
extern FILE *stOut;

//#define DEBUG_MEM_EM

unsigned long RTtranslateMemory(unsigned long input, unsigned long origAddr, unsigned long curAddr) {
   /* Standard nonblocking synchronization construct */
   int index;
   int min;
   int max;
   volatile int guard2;
   int debug=0;
   if (input >= 0x409000 && input < 0x4009100) {
       fprintf(stOut, "RTtranslateMemory(ptr 0x%lx, origInsn 0x%lx, curAddr 0x%lx)\n", 
               input, origAddr, curAddr);
       debug = 1;
   }

#ifdef DEBUG_MEM_EM
   fprintf(stOut, "RTtranslateMemory(ptr 0x%lx, origInsn 0x%lx, curAddr 0x%lx)\n", 
           input, origAddr, curAddr);
#endif

   do {
      guard2 = RTmemoryMapper.guard2;
      min = 0;
      max = (RTmemoryMapper.size - 1);
      do {
         index = min + ((max - min) / 2);
         if (input >= RTmemoryMapper.elements[index].lo) {
            /* Either correct or too low */
            if (input < RTmemoryMapper.elements[index].hi) {
               break;
            }
            else {
               min = index + 1;
            }
         }
         else {
            /* Too high */
            max = index - 1;
         }
      } while (min <= max);
   } while (guard2 != RTmemoryMapper.guard1);

   if (min <= max) {
      if (RTmemoryMapper.elements[index].shift == -1) {
         //fprintf(stOut, "... returning (should be) segv!\n");
         return 0;
      }
      else {
#ifdef  DEBUG_MEM_EM
        fprintf(stOut, "... returning shadow copy as index is within range 0x%lx to 0x%lx, shift 0x%lx\n",
                RTmemoryMapper.elements[index].lo,
                RTmemoryMapper.elements[index].hi,
                RTmemoryMapper.elements[index].shift);
        fprintf(stOut, "Original 0x%lx, dereferenced 0x%x, now 0x%lx, deref 0x%x ", 
                input, * (int *) input, (input + RTmemoryMapper.elements[index].shift),
                * (int *)(input + RTmemoryMapper.elements[index].shift));
        fprintf(stOut, "equal=%d\n", (*(int*)input) == *(int*)(input + RTmemoryMapper.elements[index].shift));
#endif
        if (debug) {
            fprintf(stOut, "... returning shadow copy as index is within range 0x%lx to 0x%lx, shift 0x%lx\n",
                    RTmemoryMapper.elements[index].lo,
                    RTmemoryMapper.elements[index].hi,
                    RTmemoryMapper.elements[index].shift);
            fprintf(stOut, "Original 0x%lx, dereferenced 0x%x, now 0x%lx, deref 0x%x ", 
                    input, * (int *) input, (input + RTmemoryMapper.elements[index].shift),
                    * (int *)(input + RTmemoryMapper.elements[index].shift));
            fprintf(stOut, "equal=%d\n", (*(int*)input) == *(int*)(input + RTmemoryMapper.elements[index].shift));
        }
		return input + RTmemoryMapper.elements[index].shift;
      }
   }
   else {
#ifdef  DEBUG_MEM_EM
      fprintf(stOut, "\t min %d, max %d, index %d returning no change\n", min, max, index);
      //fprintf(stOut, "\t min %d, max %d, index %d returning no change ", min, max, index);
      //fprintf(stOut, "@deref 0x%x\n", *(int*)input);
#endif
	  if (debug) fprintf(stOut, "\t no shift\n");
      return input;
   }
}

unsigned long RTtranslateMemoryShift(unsigned long input, unsigned long origAddr, unsigned long curAddr) {
   /* Standard nonblocking synchronization construct */
	int debug=0;
	int index;
   int min;
   int max;
   volatile int guard2;

#ifdef  DEBUG_MEM_EM
   fprintf(stOut, "RTtranslateMemoryShift(ptr 0x%lx, origInsn 0x%lx, curAddr 0x%lx)\n", 
           input, origAddr, curAddr);
#endif
   if (input >= 0x409000 && input < 0x4009100) {
        fprintf(stOut, "RTtranslateMemoryShift(ptr 0x%lx, origInsn 0x%lx, curAddr 0x%lx)\n", 
               input, origAddr, curAddr);
       debug = 1;
   }

   do {
      guard2 = RTmemoryMapper.guard2;
      min = 0;
      max = (RTmemoryMapper.size - 1);
      do {
         index = min + ((max - min) / 2);
         if (input >= RTmemoryMapper.elements[index].lo) {
            /* Either correct or too low */
            if (input < RTmemoryMapper.elements[index].hi) {
               break;
            }
            else {
               min = index + 1;
            }
         }
         else {
            /* Too high */
            max = index - 1;
         }
      } while (min <= max);
   } while (guard2 != RTmemoryMapper.guard1);

   if (min <= max) {
      if (RTmemoryMapper.elements[index].shift == -1) {
         return -1 * input;
      }
      else {
#ifdef DEBUG_MEM_EM
         fprintf(stOut, "Original 0x%lx, dereferenced 0x%x, now 0x%lx, deref 0x%x ", 
                 input, * (int *) input, (input + RTmemoryMapper.elements[index].shift),
                 * (int *)(input + RTmemoryMapper.elements[index].shift));
         fprintf(stOut, "equal=%d\n", (*(int*)input) == *(int*)(input + RTmemoryMapper.elements[index].shift));
#endif
       if (debug) {
         fprintf(stOut, "Original 0x%lx, dereferenced 0x%x, now 0x%lx, deref 0x%x ", 
                 input, * (int *) input, (input + RTmemoryMapper.elements[index].shift),
                 * (int *)(input + RTmemoryMapper.elements[index].shift));
         fprintf(stOut, "equal=%d\n", (*(int*)input) == *(int*)(input + RTmemoryMapper.elements[index].shift));
       }
		 return RTmemoryMapper.elements[index].shift;
      }
   }
   else {
#ifdef DEBUG_MEM_EM
      fprintf(stOut, "\t min %d, max %d, index %d returning no change\n", min, max, index);
      //fprintf(stOut, "\t min %d, max %d, index %d returning no change ", min, max, index);
      //fprintf(stOut, "@deref 0x%x\n", *(int*)input);
#endif
	  if (debug) fprintf(stOut, "\t No shift\n");
	  return 0;
   }
}

int RTuntranslatedEntryCounter;
extern void DYNINST_stopThread (void * pointAddr, void *callBackID, void *flags, void *calculation);

void RThandleShadow(void *direction, void *pointAddr, void *callbackID, void *flags, void *calculation) {
    if ((int)direction == 1) {
        if (RTuntranslatedEntryCounter == 0) {
            // Entering a system call...
            DYNINST_stopThread(pointAddr, callbackID, flags, (void *)1);
        }
        RTuntranslatedEntryCounter++;
    }
    else {
        RTuntranslatedEntryCounter--;
        if (RTuntranslatedEntryCounter == 0) {
            DYNINST_stopThread(pointAddr, callbackID, flags, (void *)0);
        }
    }
}
