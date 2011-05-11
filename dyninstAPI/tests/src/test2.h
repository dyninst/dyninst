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

#ifndef _test2_h_
#define _test2_h_

#if defined(i386_unknown_linux2_0) \
 || defined(ia64_unknown_linux2_4)
#define	TEST_DYNAMIC_LIB	"libX11.so"
#define TEST_DYNAMIC_LIB2	"libXt.so"

#elif defined(x86_64_unknown_linux2_4)
#define	TEST_DYNAMIC_LIB	"libX11.so.6"
#define TEST_DYNAMIC_LIB2	"libXt.so.6"

#elif defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0)
#define	TEST_DYNAMIC_LIB	"libXaw.so"
#define TEST_DYNAMIC_LIB2	"libXt.so"

#elif defined(rs6000_ibm_aix4_1)
#define TEST_DYNAMIC_LIB        "./libtestA.so"
#define TEST_DYNAMIC_LIB_NOPATH "libtestA.so"
#define TEST_DYNAMIC_LIB2       "./libtestB.so"
#define TEST_DYNAMIC_LIB2_NOPATH "libtestB.so"

#else
#define	TEST_DYNAMIC_LIB	"libX11.so.4"
#define TEST_DYNAMIC_LIB2	"libXt.so.4"
#endif

#endif /* _test2_h_ */
