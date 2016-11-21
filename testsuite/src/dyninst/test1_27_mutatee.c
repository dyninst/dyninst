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

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

struct struct26_1 {
    int field1;
    int field2;
};

struct struct26_2 {
    int field1;
    int field2;
    int field3[10];
    struct struct26_1 field4;
};
#if !defined(os_windows_test) 
/* make GCC emit type information correctly */
struct test1_27_type1_t {
    /* void *field27_11; */
    int field27_11;
    float field27_12;
};

typedef struct test1_27_type1_t test1_27_type1;

struct test1_27_type2_t {
    /* void *field27_21; */
    int field27_21;
    float field27_22;
} ;
typedef struct test1_27_type2_t test1_27_type2;

struct test1_27_type3_t {
    int field3[10];
    struct struct26_2 field4;
};

typedef struct test1_27_type3_t test1_27_type3;

struct test1_27_type4_t {
    int field3[10];
    struct struct26_2 field4;
} ;

typedef struct test1_27_type4_t test1_27_type4;
#else 
/* Make VC2003 emit type information correctly */
typedef struct {
    /* void *field27_11; */
    int field27_11;
    float field27_12;
} test1_27_type1;

typedef struct {
    /* void *field27_21; */
    int field27_21;
    float field27_22;
} test1_27_type2;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} test1_27_type3;

typedef struct {
    int field3[10];
    struct struct26_2 field4;
} test1_27_type4;
#endif
/* need this variables or some compilers (AIX xlc) will removed unused
   typedefs - jkh 10/13/99 */
test1_27_type1 test1_27_dummy1;
test1_27_type2 test1_27_dummy2;
test1_27_type3 test1_27_dummy3;
test1_27_type4 test1_27_dummy4;

int test1_27_globalVariable1 = -1;

/* Note for future reference: -Wl,-bgcbypass:3 is NECESSARY for
   compilation (gcc) on AIX. Damn efficient linkers. */
int test1_27_globalVariable5[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int test1_27_globalVariable6[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float test1_27_globalVariable7[10] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0,
				6.0, 7.0, 8.0, 9.0};
float test1_27_globalVariable8[12];

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/*
 * Test #27 - type compatibility
 */
int test1_27_mutatee() {
  int retval, passed;

#if !defined(rs6000_ibm_aix4_1_test) \
 && !defined(i386_unknown_linux2_0_test) \
 && !defined(x86_64_unknown_linux2_4_test) /* Blind duplication - Ray */ \
 && !defined(i386_unknown_nt4_0_test) \
 && !defined(os_freebsd_test)

    logerror("Skipped test #27 (type compatibility)\n");
    logerror("\t- not implemented on this platform\n");
    test_passes(testname);
    retval = 0; /* Test "passed" */
#else
    passed = (test1_27_globalVariable1 == 1);
   
    if (passed) {
      logerror("Passed test #27 (type compatibility)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }
#endif
    return retval;
}
