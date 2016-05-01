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

void test4_4_func2();
void test4_4_func3();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

unsigned int test4_4_global1 = 0xdeadbeef;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

void test4_4_func3() {
    dprintf("in test4_4_func3\n");
    test4_4_global1 = 4000002;
}

void test4_4_func2()
{
    dprintf("in test4_4_func2\n");
    /* call to test4_4_func3 should be inserted here */
}

int test4_4_mutatee()
{
#ifndef i386_unknown_nt4_0_test
    int i;
    int pid;

    pid = fork();
    if (pid == 0) {
      /* gargc and gargv are declared as extern in solo_mutatee_boilerplate.c and
       * defined in mutatee_driver.c
       */
      int argc = gargc;
      char **argv = gargv;
      char **newArgv;
      char *start;
      size_t len, prefix, suffix;

      newArgv = (char**) calloc(sizeof(char *), argc +1);
      for (i = 0; i < argc; i++) newArgv[i] = argv[i];
        
      /* replace 4a in copy of myName by 4b */
      /* newArgv[0] = strdup(argv[0]); */
      len = strlen(argv[0]) + 2; /* + 2 for extra character and NULL byte */
      start = strstr(argv[0], "test4_4");
      if (NULL == start) {
	/* TODO Mismatch; this is an error */
      }
      prefix = start - argv[0];
      suffix = len - prefix - 8;
      newArgv[0] = (char *) malloc(len * sizeof (char));
      strncpy(newArgv[0], argv[0], prefix);
      strncpy(newArgv[0] + prefix, "test4_4b", 8);
      strncpy(newArgv[0] + prefix + 8, argv[0] + prefix + 7, suffix);

      /*         for (ch=newArgv[0]; *ch; ch++) { */
      /*             if (!strncmp(ch, "4a", 2)) *(ch+1) = 'b'; */
      /*         } */

      /* Now I need to replace 'test4_4' on the command line (argument to -run)
       * with 'test4_4b'
       */
      for (i = 1; i < argc; i++) {
	if (strcmp(newArgv[i], "-run") == 0) {
	  newArgv[i + 1] = strdup("test4_4b");
	}
      }
        
      dprintf("Starting \"%s\"\n", newArgv[0]);
      execvp(newArgv[0], newArgv);
      perror("execvp");
    } else {
      test4_4_func2();
#if defined(rs6000_ibm_aix4_1_test)
      /* On AIX the child dies when the parent exits, so wait */
      /* and the parent needs to wake up occasionally to keep dyninst happy*/
      dprintf("%d SLEEPING\n",getpid());
      sleep(10);
      dprintf("%d SLEEP MORE\n",getpid());
      sleep(2);
      dprintf("%d SLEEP MORE\n",getpid());
      sleep(5);
      dprintf("%d DONE SLEEPING\n",getpid());

#endif
      exit(getpid());
    }
#endif
    return 0;
}
