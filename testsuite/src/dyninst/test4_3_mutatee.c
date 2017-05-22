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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test4_3_func1();

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

unsigned int globalVariable3_1 = 0xdeadbeef;

void test4_3_func1() {
  DUMMY_FN_BODY;
}

int test4_3_mutatee() {
    int i;
/*    char *ch;*/
    char **newArgv;
    /* gargc and gargv are declared as extern in solo_mutatee_boilerplate.c and
     * defined in mutatee_driver.c
     */
    int argc = gargc;
    char **argv = gargv;
    size_t len, prefix, suffix; /* For building new strings */
    char *start; /* The start of the mutatee executable filename */

    newArgv = (char **) calloc(sizeof(char *), argc +1);
    for (i = 0; i < argc; i++) newArgv[i] = argv[i];

    /* Needs to find the name of the mutatee to run..  It needs to match the
     * current mutatee in compiler and optimization level...
     */
    /* replace 'test4_3' in copy of myName by 'test4_3b' */
    /* Verify that argv[0][0..6] matches 'test4_3' */
    /* FIXME This check won't work..  There may be a prefix specifying the path
     * that the executable is at
     */
    start = strstr(argv[0], "test4_3");
    if (NULL == start) {
      /* TODO Mismatch, this is an error */
    }
    /* I want to set newArgv to "test4_3b" + argv[7..] */
    len = strlen(argv[0]) + 2; /* + 2 for extra character and NULL */
    prefix = start - argv[0];
    suffix = len - prefix - 8;
    newArgv[0] = (char *) malloc(len * sizeof (char));
    strncpy(newArgv[0], argv[0], prefix);
    strncpy(newArgv[0] + prefix, "test4_3b", 8);
    /* Copy the rest of argv[0] */
    strncpy(newArgv[0] + prefix + 8, argv[0] + prefix + 7, suffix);

/*     newArgv[0] = strdup(argv[0]); */
/*     for (ch=newArgv[0]; *ch; ch++) { */
/* 	if (!strncmp(ch, "4a", 2)) *(ch+1) = 'b'; */
/*     } */

    /* Now I need to replace 'test4_3' on the command line (argument to -run)
     * with 'test4_3b'
     */
    for (i = 1; i < argc; i++) {
      /* Aw hell, I'm allowing comma-separated lists of tests to be specified
       * as the argument to -run...  Is there any way I can just make sure that
       * the right function gets called for the test4b mutatee no matter what?
       * Yes there is: just replace any arguments to -run with test4_3b.  Duh
       */
      /* FIXME this may screw up if there's more than one '-run' specified */
      if (strcmp(newArgv[i], "-run") == 0) {
	newArgv[i + 1] = strdup("test4_3b");
      }
    }

    globalVariable3_1 = 3000001; 
    dprintf("Starting \"%s\"\n", newArgv[0]);
    errno = 0;
    dprintf("Going into exec for %s...\n", newArgv[0]);
    execvp(newArgv[0], newArgv);
    perror("execvp");
    return -1; /* Never reached */
}
