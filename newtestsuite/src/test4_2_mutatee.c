#include <stdlib.h>

#include "mutatee_util.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test4_2_func2();
void test4_2_func3();
void test4_2_func4();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

unsigned int test4_2_global1 = 0xdeadbeef;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/* should be called by parent */
void test4_2_func3() {
    dprintf("pid %d in test4_2_func3\n", (int) getpid());
    test4_2_global1 = 2000002;
}

/* should be called by child */
void test4_2_func4() {
    dprintf("pid %d in test4_2_func4\n", (int) getpid());
    test4_2_global1 = 2000003;
}

void test4_2_func2() {
    /* if we get this value at exit, no function was called */
    test4_2_global1 = 2000001;
}

int test4_2_mutatee() {
#ifndef i386_unknown_nt4_0
    int pid;

    pid = fork();
    dprintf("fork result: %d\n", pid);
    if (pid >= 0) {
        /* both parent and child exit here */
        test4_2_func2();
        dprintf("at exit of %d, test4_2_global1 = %d\n", (int) getpid(),
                test4_2_global1);

#if defined(rs6000_ibm_aix4_1)
	if( pid == 0){
	        /* On AIX the child dies when the parent exits, so wait */
		/* apparently the parent needs to wake up occasionally to keep Dyninst happy */
		dprintf("%d SLEEPING\n",getpid());
        	sleep(5);
		dprintf("%d SLEEP MORE\n",getpid());
		sleep(1);
		dprintf("%d SLEEP MORE\n",getpid());
		sleep(5);
		dprintf("%d DONE SLEEPING\n",getpid());
	}
#endif
        
        /* Make the parent exit first (again, testing) */
        if (pid == 0) {
		dprintf("%d SLEEPING\n",getpid());
            sleep(1);
		dprintf("%d SLEEPING\n",getpid());
            sleep(1);
		dprintf("%d SLEEPING\n",getpid());
            sleep(1);
		dprintf("%d SLEEPING\n",getpid());
            sleep(1);
		dprintf("%d SLEEPING\n",getpid());
            sleep(1);
        }

	dprintf("Mutatee %d exiting...\n", getpid());
        exit(getpid());
    } else if (pid < 0) {
        /* error case */
        exit(pid);
    }
#endif
    return 0; /* No error, technically.  Shouldn't get here */
}
