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

#ifndef _test1_h_
#define _test1_h_

#define TEST_PTR_32BIT	((void*)0x1234faceUL)
#define TEST_PTR_48BIT	((void *)0x4321abcd8967UL)
#define TEST_PTR_64BIT	((void *)0x5678bbcb9541dabaUL)
#define TEST_VAL_64BIT  (0x5678bbcb9541dabaUL)
#define TEST_VAL_32BIT  (0x1234faceUL)

#if defined(alpha_dec_osf4_0_test)	/* Always 64 bits, 48 bit addresses. */
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_48BIT

#elif defined(mips_sgi_irix6_4_test)	/* Can be 64 or 32 bits. */
#if (_MIPS_SZPTR == 64)
#define TEST_PTR_SIZE	8
#define TEST_PTR	TEST_PTR_64BIT
#else /* _MIPS_SZPTR == 32 */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#endif

#elif defined(i386_unknown_linux2_4)
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#define TEST_VAL     TEST_VAL_32BIT
#elif defined(x86_64_unknown_linux2_4_test)
#define	TEST_PTR_SIZE	8
#define	TEST_PTR		TEST_PTR_64BIT
#define TEST_VAL     TEST_VAL_64BIT
#elif defined (ppc64_linux_test)
#define	TEST_PTR_SIZE	8
#define	TEST_PTR		TEST_PTR_64BIT
#define TEST_VAL     TEST_VAL_64BIT
#elif defined(os_freebsd_test) && defined(arch_x86_64_test)
#define TEST_PTR_SIZE   8
#define TEST_PTR                TEST_PTR_64BIT
#define TEST_VAL     TEST_VAL_64BIT
#else /* Others are 32 bits. */
#define TEST_PTR_SIZE	4
#define TEST_PTR	TEST_PTR_32BIT
#define TEST_VAL  TEST_VAL_32BIT
#endif

#if defined(os_windows_test)
#endif

#endif /* _test1_h_ */
