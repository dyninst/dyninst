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


/* 
 * $Id: asmExterns.h,v 1.3 2006/03/14 22:57:24 legendre Exp $
*/

//ccw 3 aug 2000 
//this file holds the externs for the assembly code. this allows
//the CE client easy access to it and the NT box as well.
//it is included in baseTrampTemplate.h when appropriate.

extern "C" void baseTramp();
extern "C" void baseTramp_savePreInsn();
extern "C" void baseTramp_skipPreInsn();
extern "C" void baseTramp_globalPreBranch();
extern "C" void baseTramp_localPreBranch();
extern "C" void baseTramp_localPreReturn();
extern "C" void baseTramp_updateCostInsn();
extern "C" void baseTramp_restorePreInsn();
extern "C" void baseTramp_emulateInsn();
extern "C" void baseTramp_skipPostInsn();
extern "C" void baseTramp_savePostInsn();
extern "C" void baseTramp_globalPostBranch();
extern "C" void baseTramp_localPostBranch();
extern "C" void baseTramp_localPostReturn();
extern "C" void baseTramp_restorePostInsn();
extern "C" void baseTramp_returnInsn();
extern "C" void baseTramp_endTramp();

extern "C" void baseNonRecursiveTramp();
extern "C" void baseNonRecursiveTramp_savePreInsn();
extern "C" void baseNonRecursiveTramp_skipPreInsn();
extern "C" void baseNonRecursiveTramp_guardOnPre_begin();
extern "C" void baseNonRecursiveTramp_guardOnPre_end();
extern "C" void baseNonRecursiveTramp_globalPreBranch();
extern "C" void baseNonRecursiveTramp_localPreBranch();
extern "C" void baseNonRecursiveTramp_localPreReturn();
extern "C" void baseNonRecursiveTramp_guardOffPre_begin();
extern "C" void baseNonRecursiveTramp_guardOffPre_end();
extern "C" void baseNonRecursiveTramp_updateCostInsn();
extern "C" void baseNonRecursiveTramp_restorePreInsn();
extern "C" void baseNonRecursiveTramp_emulateInsn();
extern "C" void baseNonRecursiveTramp_skipPostInsn();
extern "C" void baseNonRecursiveTramp_savePostInsn();
extern "C" void baseNonRecursiveTramp_guardOnPost_begin();
extern "C" void baseNonRecursiveTramp_guardOnPost_end();
extern "C" void baseNonRecursiveTramp_globalPostBranch();
extern "C" void baseNonRecursiveTramp_localPostBranch();
extern "C" void baseNonRecursiveTramp_localPostReturn();
extern "C" void baseNonRecursiveTramp_guardOffPost_begin();
extern "C" void baseNonRecursiveTramp_guardOffPost_end();
extern "C" void baseNonRecursiveTramp_restorePostInsn();
extern "C" void baseNonRecursiveTramp_returnInsn();
extern "C" void baseNonRecursiveTramp_endTramp();
