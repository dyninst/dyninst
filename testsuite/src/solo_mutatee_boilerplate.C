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
 * Start of test_skeleton_solo
 */

/*
 * mutatee_utils.h and solo_mutatee.h are required for all solo test mutatees
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "mutatee_util.h"
#include "solo_mutatee.h"

/*
 * These macros are needed for when the skeleton is filled in using #defines
 * as the makefiles are being generated (two macros are defined to force
 * macro expansion)
 */
#define _CONCAT(A, B)	A ## B
#define CONCAT(A, B)	_CONCAT(A, B)
#define _QUOTE(S)		#S
#define QUOTE(S)		_QUOTE(S)

/* This is kind of a hack:  test4_3 needs to have argc and argv available.  So
 * I'm making them global variables in the mutatee_driver and adding extern
 * references to them here.
 */
extern int gargc;
extern char **gargv;

/* The main function for running the mutatee part of this test.
 * Should follow the pattern:
 * int <testname>_mutateeTest()
 */

int CONCAT(TEST_NAME, _mutatee());

/* Also declare a global that holds the name of the test */

static const char *testname = QUOTE(TEST_NAME);

/* The variable groupable flags whether or not this test can be run as part
 * of a group mutatee.  The mutatee driver uses this flag to tell whether it
 * is supposed to print the results of the test or not.
 */
static int groupable_mutatee = GROUPABLE;

const char * CONCAT(kill_compiler_warnings_, TEST_NAME) ()
{
   int foo = groupable_mutatee;
   const char *bar = testname;
   return (bar + foo);
}

/* The macro SOLO_MUTATEE(<testname>) (from solo_mutatee.h) defines a few
 * variables that are required by the mutatee driver.  This macro needs to be
 * called *after* the declaration of the main mutatee function.
 */

#if (GROUPABLE==0)
/* SOLO_MUTATEE(@<testname>@); */
/* Why am I using a macro here?  I'm just going to include the body of the
 * macro directly.
 */
mutatee_call_info_t mutatee_funcs[] = {
   {(char *) QUOTE(TEST_NAME), CONCAT(TEST_NAME, _mutatee), SOLO, (char *) "@<label>@"}
};
int runTest[1];
int passedTest[1];
int max_tests = 1;
#endif

#ifdef __cplusplus
}
#endif

/* ******************************************************************** */
/* *** Everything above this line should be automatically generated *** */
/* ******************************************************************** */
#include QUOTE(MUTATEE_SRC)
