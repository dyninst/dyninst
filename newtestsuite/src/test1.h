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

#ifndef _test1_h_
#define _test1_h_

#define TEST_PTR_32BIT	((void*)0x1234faceUL)
#define TEST_PTR_48BIT	((void *)0x4321abcd8967UL)
#define TEST_PTR_64BIT	((void *)0x5678bbcb9541dabaUL)

#if defined(alpha_dec_osf4_0)	/* Always 64 bits, 48 bit addresses. */
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_48BIT

#elif defined(mips_sgi_irix6_4)	/* Can be 64 or 32 bits. */
#if (_MIPS_SZPTR == 64)
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_64BIT
#else /* _MIPS_SZPTR == 32 */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#endif

#elif defined(ia64_unknown_linux2_4)
#define	TEST_PTR_SIZE	8
#define	TEST_PTR		TEST_PTR_64BIT

#elif defined(i386_unknown_linux2_4)
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#elif defined(x86_64_unknown_linux2_4)
#define	TEST_PTR_SIZE	8
#define	TEST_PTR		TEST_PTR_64BIT

#else /* Others are 32 bits. */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#endif

#if defined(os_windows)
#endif

#endif /* _test1_h_ */
