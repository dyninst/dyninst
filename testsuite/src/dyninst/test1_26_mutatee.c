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

void test1_26_call1();

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

/*  struct26_2 test1_26_globalVariable1;  */
struct struct26_2 test1_26_globalVariable1;
int test1_26_globalVariable2 = 26000000;
int test1_26_globalVariable3 = 26000000;
int test1_26_globalVariable4 = 26000000;
int test1_26_globalVariable5 = 26000000;
int test1_26_globalVariable6 = 26000000;
int test1_26_globalVariable7 = 26000000;

int test1_26_globalVariable8 = 26000000;
int test1_26_globalVariable9 = 26000000;
int test1_26_globalVariable10 = 26000000;
int test1_26_globalVariable11 = 26000000;
int test1_26_globalVariable12 = 26000000;
int test1_26_globalVariable13 = 26000000;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

static void verifyScalarValue26(const char *name, int a, int value);
static void call26_2();

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int failed_test = FALSE;

/* Function definitions follow */

/*
 * Test #26 - field operators
 */

int test1_26_mutatee() {
  int retval;
    int i;

    test1_26_globalVariable1.field1 = 26001001;
    test1_26_globalVariable1.field2 = 26001002;
    for (i=0; i < 10; i++) test1_26_globalVariable1.field3[i] = 26001003 + i;
    test1_26_globalVariable1.field4.field1 = 26000013;
    test1_26_globalVariable1.field4.field2 = 26000014;

    test1_26_call1();

    verifyScalarValue26("test1_26_globalVariable2", test1_26_globalVariable2, 26001001);
    verifyScalarValue26("test1_26_globalVariable3", test1_26_globalVariable3, 26001002);
    verifyScalarValue26("test1_26_globalVariable4", test1_26_globalVariable4, 26001003);
    verifyScalarValue26("test1_26_globalVariable5", test1_26_globalVariable5, 26001003+5);
    verifyScalarValue26("test1_26_globalVariable6", test1_26_globalVariable6, 26000013);
    verifyScalarValue26("test1_26_globalVariable7", test1_26_globalVariable7, 26000014);

    /* local variables */
    verifyScalarValue26("test1_26_globalVariable8", test1_26_globalVariable8, 26002001);
    verifyScalarValue26("test1_26_globalVariable9", test1_26_globalVariable9, 26002002);
    verifyScalarValue26("test1_26_globalVariable10", test1_26_globalVariable10, 26002003);
    verifyScalarValue26("test1_26_globalVariable11", test1_26_globalVariable11, 26002003+5);
    verifyScalarValue26("test1_26_globalVariable12", test1_26_globalVariable12, 26002013);
    verifyScalarValue26("test1_26_globalVariable13", test1_26_globalVariable13, 26002014);

    if (!failed_test) {
      logerror("Passed test #26 (field operators)\n");
      test_passes(testname);
      retval = 0; /* Test passed */
    } else {
      retval = -1; /* Test failed */
    }

    return retval;
}

void verifyScalarValue26(const char *name, int a, int value)
{
  if (!verifyScalarValue(name, a, value, "test1_26", "field operators")) {
    failed_test = TRUE;
  }
}

void call26_2()
{
}

void test1_26_call1()
{
    int i;
    /*    struct26_2 localVariable26_1;  */
    struct struct26_2 localVariable26_1;

    localVariable26_1.field1 = 26002001;
    localVariable26_1.field2 = 26002002;
    for (i=0; i < 10; i++) localVariable26_1.field3[i] = 26002003 + i;
    localVariable26_1.field4.field1 = 26002013;
    localVariable26_1.field4.field2 = 26002014;

    /* check local variables at this point (since we known locals are still
       on the stack here. */
    call26_2();

}
