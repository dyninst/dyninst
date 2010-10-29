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
#include <errno.h>
//#include <unistd.h>

/* Code to assist in remapping memory operations that were affected
 * by our instrumentation */

struct MemoryMapper RTmemoryMapper = {0, 0, 0, 0};

unsigned long __attribute__((fastcall)) RTtranslateMemory(unsigned long input) {
   /* Standard nonblocking synchronization construct */
   int index;
   int min;
   int max;
   volatile int guard2;
   //fprintf(stderr, "RTtranslateMemory(0x%lx)\n", input);
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
         //fprintf(stderr, "... returning (should be) segv!\n");
         return 0;
      }
      else {
         //fprintf(stderr, "... returning shift\n");
         return input + RTmemoryMapper.elements[index].shift;
      }
   }
   else {
      //fprintf(stderr, "\t min %d, max %d, index %d, returning no change\n", min, max, index);
      return input;
   }
}

