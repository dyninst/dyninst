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

// $Id: test6.h,v 1.1 2008/10/30 19:21:16 legendre Exp $
#ifndef TEST6_H
#define TEST6_H

#include "test_results.h"

#define skiptest(i,d) { \
    printf("Skipping test #%d (%s)\n", (i), (d)); \
    printf("    not implemented on this platform\n"); }

#define failtest(i,d,r) { fprintf(stderr, "**Failed** test #%d (%s)\n", (i), (d)); \
                          fprintf(stderr, "    %s\n", (r)); \
                          if(appProc) appProc->continueExecution(); \
                          return FAILED; \
                         }

#define MK_LD(imm, rs1, rs2, bytes) (new BPatch_memoryAccess(NULL, 0,\
							     true, false, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_ST(imm, rs1, rs2, bytes) (new BPatch_memoryAccess(NULL, 0,\
							     false, true, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_LS(imm, rs1, rs2, bytes) (new BPatch_memoryAccess(NULL, 0,\
							     true, true, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_PF(imm, rs1, rs2, f) (new BPatch_memoryAccess(NULL, 0,\
							 false, false, true, \
                                                         (imm), (rs1), (rs2), \
                                                         0, -1, -1, (f)))

#define MK_LDsc(imm, rs1, rs2, scale, bytes) (new BPatch_memoryAccess(NULL, 0,\
								      true, false, \
                                                                      (bytes), \
                                                                      (imm), (rs1), (rs2), \
                                                                      (scale)))

#define MK_LDsccnd(imm, rs1, rs2, scale, bytes, cond) (new BPatch_memoryAccess(NULL, 0,true, false, (bytes), (imm), (rs1), (rs2), (scale), (cond), false))


#define MK_LD2(imm, rs1, rs2, bytes, imm_2, rs1_2, rs2_2, bytes_2) (new BPatch_memoryAccess(NULL, 0,true, false, (bytes), (imm), (rs1), (rs2), 0, true, false, (bytes_2), (imm_2), (rs1_2), (rs2_2), 0))
#define MK_SL2(imm, rs1, rs2, bytes, imm_2, rs1_2, rs2_2, bytes_2) (new BPatch_memoryAccess(NULL, 0,false, true, (bytes), (imm), (rs1), (rs2), 0, true, false, (bytes_2), (imm_2), (rs1_2), (rs2_2), 0))

#define MK_SL2vECX(imm, rs1, rs2, imm_2, rs1_2, rs2_2, bop) (new BPatch_memoryAccess(NULL, 0,false, true, (imm), (rs1), (rs2), 0, 0, -1, 1, (bop),  true, false, (imm_2), (rs1_2), (rs2_2), 0, 0, -1, 1, (bop)))

#define MK_STnt(imm, rs1, rs2, bytes) (new BPatch_memoryAccess(NULL, 0,\
							       false, true, \
                                                               (bytes), (imm), (rs1), (rs2), 0, \
                                                               -1, true))

// naxses

#ifdef rs6000_ibm_aix4_1_test
const unsigned int naxses = 73;
#endif

#if defined(arch_x86_test)
#if defined(os_windows_test)
const unsigned int naxses = 95;
#elif defined(os_freebsd_test)
const unsigned int naxses = 107;
#else
const unsigned int naxses = 91;
#endif
#endif

#if defined(x86_64_unknown_linux2_4_test) || defined(amd64_unknown_freebsd7_0_test)
const unsigned int naxses = 100;
#endif

#endif /* TEST6_H */

