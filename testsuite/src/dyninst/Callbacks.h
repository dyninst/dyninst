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

// $Id: Callbacks.h,v 1.1 2008/10/30 19:17:04 legendre Exp $
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "BPatch.h"
#include "test_lib_dll.h"

TESTLIB_DLL_EXPORT void setExpectErrors(bool er);
TESTLIB_DLL_EXPORT void clearError();
TESTLIB_DLL_EXPORT bool getError();
TESTLIB_DLL_EXPORT void setErrorPrint(int error);
TESTLIB_DLL_EXPORT void setExpectError(int error);
TESTLIB_DLL_EXPORT void setBPatch(BPatch *bpatch);

TESTLIB_DLL_EXPORT void errorFunc(BPatchErrorLevel level, int num, const char * const *params);

TESTLIB_DLL_EXPORT void createInstPointError(BPatchErrorLevel level, int num, const char * const *params);

#endif /* CALLBACKS_H */
