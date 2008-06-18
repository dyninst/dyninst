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
int groupable_mutatee = GROUPABLE;

/* The macro SOLO_MUTATEE(<testname>) (from solo_mutatee.h) defines a few
 * variables that are required by the mutatee driver.  This macro needs to be
 * called *after* the declaration of the main mutatee function.
 */

/* SOLO_MUTATEE(@<testname>@); */
/* Why am I using a macro here?  I'm just going to include the body of the
 * macro directly.
 */
mutatee_call_info_t mutatee_funcs[] = {
  {QUOTE(TEST_NAME), CONCAT(TEST_NAME, _mutatee), SOLO, "@<label>@"}
};
int runTest[1];
int passedTest[1];
unsigned int MAX_TEST = 1;

#ifdef __cplusplus
}
#endif

/* ******************************************************************** */
/* *** Everything above this line should be automatically generated *** */
/* ******************************************************************** */
#include QUOTE(MUTATEE_SRC)
