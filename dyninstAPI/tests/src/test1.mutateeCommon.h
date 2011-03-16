/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

/* Test application (Mutatee) */

/* $Id: test1.mutateeCommon.h,v 1.13 2005/07/06 18:28:00 rchen Exp $ */

/* Empty functions are sometimes compiled too tight for entry and exit
   points.  The following macro is used to flesh out these
   functions. (expanded to use on all platforms for non-gcc compilers jkh 10/99)
 */
static volatile int dummy3__;

#define DUMMY_FN_BODY \
  int dummy1__ = 1; \
  int dummy2__ = 2; \
  dummy3__ = dummy1__ + dummy2__

/* control debug printf statements */
#define dprintf	if (debugPrint) printf

#define MAX_TEST 40 

extern int kludge;
extern int debugPrint;
extern int runTest[MAX_TEST+1];
extern int passedTest[MAX_TEST+1];

extern void verifyScalarValue(const char *name, int a, int value, int testNum,
                              const char *testName);
extern void verifyValue(const char *name, int *a, int index, int value, 
                        int tst, const char *tn);

#define TRUE	1
#define FALSE	0

#define RET13_1 1300100

#define RAN17_1 1701000

#define RET17_1 1700100
#define RET17_2 1700200

#define MAGIC19_1 1900100
#define MAGIC19_2 1900200

/* These are copied in libtestA.c and libtestB.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

#define TEST20_A 3
#define TEST20_B 4.3
#define TEST20_C 7
#define TEST20_D 6.4
#define TEST20_TIMES 41


#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(ia64_unknown_linux2_4) \
 || defined(ppc64_linux)
#if defined(x86_64_unknown_linux2_4) && (__WORDSIZE == 32)
static const char *libNameA = "libtestA_m32.so";
#else
static const char *libNameA = "libtestA.so";
#endif
#endif

