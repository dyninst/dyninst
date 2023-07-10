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

#ifndef _RTCOMMON_H_
#define _RTCOMMON_H_

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTthread.h"
#include <stddef.h>
#include <stdarg.h>
#include "common/h/compiler_annotations.h"

void DYNINSTtrapFunction(void);
void DYNINSTbreakPoint(void);
/* Use a signal that is safe if we're not attached. */
void DYNINSTsafeBreakPoint(void);
void DYNINSTinit(void);
int DYNINSTreturnZero(void);
int DYNINSTwriteEvent(void *ev, size_t sz);
int DYNINSTasyncConnect(int pid);

int DYNINSTinitializeTrapHandler(void);
void* dyninstTrapTranslate(void *source, 
                           volatile unsigned long *table_used,
                           volatile unsigned long *table_version,
                           volatile trapMapping_t **trap_table,
                           volatile unsigned long *is_sorted);

extern int DYNINST_mutatorPid;
extern int libdyninstAPI_RT_init_localCause;
extern int libdyninstAPI_RT_init_localPid;
extern int libdyninstAPI_RT_init_maxthreads;
extern int libdyninstAPI_RT_init_debug_flag;
extern int DYNINSTdebugPrintRT;
extern tc_lock_t DYNINST_trace_lock;

extern void *map_region(void *addr, int len, int fd);
extern int unmap_region(void *addr, int len);
extern void mark_heaps_exec(void);

extern int DYNINSTdebugRTlib;

DLLEXPORT extern int DYNINSTstaticMode;


int rtdebug_printf(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);
#endif
       
