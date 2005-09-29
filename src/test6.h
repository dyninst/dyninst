/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test6.h,v 1.1 2005/09/29 20:39:41 bpellin Exp $
#ifndef TEST6_H
#define TEST6_H

#define skiptest(i,d) { \
    printf("Skipping test #%d (%s)\n", (i), (d)); \
    printf("    not implemented on this platform\n"); }

#define failtest(i,d,r) { fprintf(stderr, "**Failed** test #%d (%s)\n", (i), (d)); \
                          fprintf(stderr, "    %s\n", (r)); \
                          return -1; \
                         }

#define MK_LD(imm, rs1, rs2, bytes) (new BPatch_memoryAccess("", 0, \
							     true, false, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_ST(imm, rs1, rs2, bytes) (new BPatch_memoryAccess("", 0, \
							     false, true, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_LS(imm, rs1, rs2, bytes) (new BPatch_memoryAccess("", 0, \
							     true, true, \
                                                             (bytes), (imm), (rs1), (rs2)))
#define MK_PF(imm, rs1, rs2, f) (new BPatch_memoryAccess("", 0, \
							 false, false, true, \
                                                         (imm), (rs1), (rs2), \
                                                         0, -1, -1, (f)))

#define MK_LDsc(imm, rs1, rs2, scale, bytes) (new BPatch_memoryAccess("", 0, \
								      true, false, \
                                                                      (bytes), \
                                                                      (imm), (rs1), (rs2), \
                                                                      (scale)))

#define MK_LDsccnd(imm, rs1, rs2, scale, bytes, cond) (new BPatch_memoryAccess("", 0, true, false, (bytes), (imm), (rs1), (rs2), (scale), (cond), false))


#define MK_LD2(imm, rs1, rs2, bytes, imm_2, rs1_2, rs2_2, bytes_2) (new BPatch_memoryAccess("", 0, true, false, (bytes), (imm), (rs1), (rs2), 0, true, false, (bytes_2), (imm_2), (rs1_2), (rs2_2), 0))
#define MK_SL2(imm, rs1, rs2, bytes, imm_2, rs1_2, rs2_2, bytes_2) (new BPatch_memoryAccess("", 0, false, true, (bytes), (imm), (rs1), (rs2), 0, true, false, (bytes_2), (imm_2), (rs1_2), (rs2_2), 0))

#define MK_SL2vECX(imm, rs1, rs2, imm_2, rs1_2, rs2_2, bop) (new BPatch_memoryAccess("", 0, false, true, (imm), (rs1), (rs2), 0, 0, -1, 1, (bop),  true, false, (imm_2), (rs1_2), (rs2_2), 0, 0, -1, 1, (bop)))

#define MK_STnt(imm, rs1, rs2, bytes) (new BPatch_memoryAccess("", 0, \
							       false, true, \
                                                               (bytes), (imm), (rs1), (rs2), 0, \
                                                               -1, true))

// naxses
#ifdef sparc_sun_solaris2_4
const unsigned int naxses = 26;
#endif

#ifdef rs6000_ibm_aix4_1
const unsigned int naxses = 73;
#endif

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_nt4_0)
const unsigned int naxses = 85;
#endif

#ifdef ia64_unknown_linux2_4
const unsigned int naxses = 12;
#endif


#endif /* TEST6_H */

