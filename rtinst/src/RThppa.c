#include <stdio.h>
#include "rtinst/h/rtinst.h"

extern double MILLION;
extern int DYNINSTin_sample;
extern time64 DYNINSTgetCPUtime(void);
extern time64 DYNINSTgetWalltime(void);

void
DYNINSTstartProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT,endT;
#endif

    if (DYNINSTin_sample) return;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
        timer->start     = DYNINSTgetCPUtime();
        timer->normalize = MILLION;
    }
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[0]+=endT-startT;
    DYNINSTtestN[0]++;
#endif
}




void
DYNINSTstartWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
        timer->start     = DYNINSTgetWalltime();
        timer->normalize = MILLION;
    }
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[2]+=endT-startT;
    DYNINSTtestN[2]++;
#endif
}


void
DYNINSTstopProcessTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT,endT;
#endif

    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (!timer->counter) {
        return;
    }

    if (timer->counter == 1) {
        time64 now = DYNINSTgetCPUtime();

        timer->snapShot = now - timer->start + timer->total;
        timer->mutex    = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
        timer->total = DYNINSTgetCPUtime() - timer->start + timer->total; 
        timer->counter = 0;
        timer->mutex = 0;

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("process timer rollback\n"); fflush(stdout);
            abort();
        }
    }
    else {
      timer->counter--;
    }

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[1]+=endT-startT;
    DYNINSTtestN[1]++;
#endif 
}



void
DYNINSTstopWallTimer_hpux(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (!timer->counter) {
        return;
    }

    if (timer->counter == 1) {
        time64 now = DYNINSTgetWalltime();

        timer->snapShot = now - timer->start + timer->total;
        timer->mutex    = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
        timer->total    = DYNINSTgetWalltime() - timer->start + timer->total;
        timer->counter  = 0;
        timer->mutex    = 0;

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("wall timer rollback\n"); 
            fflush(stdout);
            abort();
        }
    }
    else {
        timer->counter--;
    }
#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[3]+=endT-startT;
    DYNINSTtestN[3]++;
#endif 
}




