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
#ifndef __DYNINSTCOMPAT_H__
#define __DYNINSTCOMPAT_H__

/*******************************************************************************
 * This file, and the accompaning dyninstCompat.vX.C files are simply to
 * get around incompatabilities between DyninstAPI versions.  Hopefully,
 * when DyninstAPI v4 support is dropped, we can merge these files into
 * dyninstCore.[Ch]
 */

#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_image.h"
#include "BPatch_Vector.h"

#if defined(HAVE_BPATCH_PROCESS_H)
#include "BPatch_process.h"
#else
#define BPatch_process BPatch_thread
#endif

struct dynHandle {
  BPatch *bpatch;
  BPatch_addressSpace *addSpace;
  BPatch_process *proc;
  BPatch_image *image;
};

dynHandle *mutatorInit(void);
bool dynStartTransaction(dynHandle *);
bool dynEndTransaction(dynHandle *);

#endif
