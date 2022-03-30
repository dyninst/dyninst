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

#ifndef _BPatch_dll_h_
#define _BPatch_dll_h_

// TEMPORARY PARADYND FLOWGRAPH KLUGE
// If we are building BPatch classes into paradynd we want BPATCH_DLL_EXPORT 
// to be defined as the empty string (for all platforms). This currently tests
// SHM_SAMPLING because it is defined for paradynd and not for the dyninst
// dll or dyninst clients, read '#if PARADYND'. 
#ifdef SHM_SAMPLING
#define	BPATCH_DLL_EXPORT
#else
#if defined(_MSC_VER)

#ifdef BPATCH_DLL_BUILD
// we are building the dyninstAPI DLL
#define	BPATCH_DLL_EXPORT	__declspec(dllexport)

#else

// we are not building the dyninstAPI DLL
#define	BPATCH_DLL_EXPORT	__declspec(dllimport)
#if _MSC_VER >= 1300
#define	BPATCH_DLL_IMPORT   1
#endif
#endif	// BPATCH_DLL_BUILD

#else

// we are not building for a Windows target 
#define	BPATCH_DLL_EXPORT  __attribute__((visibility ("default")))

#endif

#endif // TEMPORARY PARADYND FLOWGRAPH KLUGE


// declare our version string
extern "C" BPATCH_DLL_EXPORT const char V_libdyninstAPI[];

#endif /* _BPatch_dll_h_ */
