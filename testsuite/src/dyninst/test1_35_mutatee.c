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
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* group_mutatee_boilerplate.c is prepended to this file by the make system */

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

#if defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(os_freebsd_test)

#ifndef Fortran
#ifdef __cplusplus
extern "C" int test1_35_call1();
#else
extern int test1_35_call1();
#endif
#endif
#endif

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int test1_35_mutatee() {
#if defined(i386_unknown_solaris2_5_test) \
 || defined(i386_unknown_linux2_0_test) \
 || defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4_test)  

#if !defined Fortran

    int value;
    value = test1_35_call1();

    if( value != 35 )
    {
      logerror( "**Failed** test #35 (function relocation)\n" );

      logerror( "    total = %d, should be 35.\n", value );

      switch(value)
      {
        case 1: logerror( "    Entry instrumentation did not work.\n" ); break;
        case 2: logerror( "    Exit instrumentation did not work.\n" ); break;
        default: logerror("    Take a look at the call to call35_2.\n" ); break;
      }
      return -1; /* Test failed */
    }

    test_passes(testname);
    logerror( "Passed test #35 (function relocation)\n" );
    return 0; /* Test passed */
#endif
#else
    test_passes(testname);
    logerror( "Skipped test #35 (function relocation)\n" );
    logerror( "\t- test not implemented for this platform\n" );
    return 0; /* Test "passed" */
#endif
}
