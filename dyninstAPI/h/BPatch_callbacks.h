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

#ifndef _BPATCH_ERROR_H_
#define _BPATCH_ERROR_H_

class BPatch_process;
class BPatch_thread;
class BPatch_module;
class BPatch_point;
class BPatch_function;

typedef enum {
    BPatchFatal, BPatchSerious, BPatchWarning, BPatchInfo
} BPatchErrorLevel;

typedef enum {
   NoExit,
   ExitedNormally,
   ExitedViaSignal
} BPatch_exitType;

typedef void (*BPatchOneTimeCodeCallback)(BPatch_thread *proc,
                                          void *userData, void *returnValue);


typedef void (*BPatchErrorCallback)(BPatchErrorLevel severity,
				    int number,
				    const char * const *params);
typedef void (*BPatchAsyncThreadEventCallback)(BPatch_process *proc, BPatch_thread *thr);
typedef void (*BPatchUserEventCallback)(BPatch_process *proc, void *buf, 
                                        unsigned int bufsize);

typedef void (*BPatchDynLibraryCallback)(BPatch_thread *proc,
					 BPatch_module *mod,
					 bool load);

typedef void (*BPatchForkCallback)(BPatch_thread *parent, 
                                   BPatch_thread *child);

typedef void (*BPatchExecCallback)(BPatch_thread *proc);

typedef void (*BPatchExitCallback)(BPatch_thread *proc,
                                   BPatch_exitType exit_type);

typedef void (*BPatchSignalCallback)(BPatch_thread *proc, int sigNum);

typedef void (*BPatchDynamicCallSiteCallback)(BPatch_point *at_point,
                                              BPatch_function *called_function);

typedef void (*BPatchStopThreadCallback)(BPatch_point *at_point, 
                                         void *returnValue);

typedef void (*BPatchSignalHandlerCallback)(BPatch_point *at_point, 
                         long signum, BPatch_Vector<Dyninst::Address> *handlers);

#endif
