#include <stdio.h>
#include "rtinst/h/rtinst.h"

extern DYNINSTstartProcessTimer(tTimer *timer);
extern DYNINSTstartWallTimer(tTimer* timer);
extern DYNINSTstopProcessTimer(tTimer* timer);
extern DYNINSTstopWallTimer(tTimer* timer);

void
DYNINSTstartProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstartProcessTimer(timer); 
}




void
DYNINSTstartWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstartWallTimer(timer);
}


void
DYNINSTstopProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstopProcessTimer(timer);
}



void
DYNINSTstopWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    DYNINSTstopWallTimer(timer);
}




