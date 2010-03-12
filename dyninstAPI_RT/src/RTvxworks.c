#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTcommon.h"

#include <unistd.h>

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
    rtdebug_printf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
}

int dyn_pid_self()
{
    return getpid();
}

int dyn_lwp_self()
{
    return getpid();
}

void DYNINSTbreakPoint()
{  
    /* We set a global flag here so that we can tell
      if we're ever in a call to this when we get a
      SIGBUS */
    if (DYNINSTstaticMode)
        return;

    DYNINST_break_point_event = 1;
    while (DYNINST_break_point_event) {
        kill(dyn_lwp_self(), 7);
    }
    /* Mutator resets to 0... */
}
