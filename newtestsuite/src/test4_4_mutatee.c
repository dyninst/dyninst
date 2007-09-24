#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mutatee_util.h"

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
#ifndef i386_unknown_nt4_0
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
	  newArgv[i + 1] = "test4_4b";
	}
      }
        
      dprintf("Starting \"%s\"\n", newArgv[0]);
      execvp(newArgv[0], newArgv);
      perror("execvp");
    } else {
        test4_4_func2();
#if defined(rs6000_ibm_aix4_1)
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
