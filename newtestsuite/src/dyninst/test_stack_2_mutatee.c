#include <signal.h>
#include <unistd.h>

#include "mutatee_util.h"

#define do_dyninst_breakpoint() stop_process_()

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

void test_stack_2_func4();
void test_stack_2_func3();
void test_stack_2_func2();
void test_stack_2_func1();

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

static int globalVariable2_1 = 0;
static volatile int globalVariable2_2 = 0;

/* Function definitions follow */

#if !defined(i386_unknown_nt4_0)
void test_stack_2_func4()
{
    globalVariable2_1++;
    do_dyninst_breakpoint();
}

void sigalrm_handler(int signum)
{
    globalVariable2_1++;
    globalVariable2_2 = 1;
    test_stack_2_func4();
}

void test_stack_2_func3()
{
    globalVariable2_1++;

    /* Cause a SIGALRM */
    alarm(1);
    while (globalVariable2_2 == 0) ;
}

void test_stack_2_func2()
{
    globalVariable2_1++;
    test_stack_2_func3();
}
#endif /* !i386_unknown_nt4_0 */

void test_stack_2_func1()
{
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4) \
 || defined(ia64_unknown_linux2_4)
    void (*old_handler)(int) = signal(SIGALRM, sigalrm_handler);

    globalVariable2_1++;
    test_stack_2_func2();

    signal(SIGALRM, old_handler);
#endif
}

/* skeleton test doesn't do anything besides say that it passed */
int test_stack_2_mutatee() {
#ifndef os_windows
  /* Windows doesn't support signals */
  test_stack_2_func1();
#endif
  return 0; /* Return code for this mutatee is not checked */
}
