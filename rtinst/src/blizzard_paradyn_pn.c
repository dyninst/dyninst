#include <signal.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocol.h"
#include "model.h"
#include "align.h"



typedef struct {
	void (*handler)();
	int ticks;            /* a tick is 100us in default,     zxu                        */
			      /* see /p/wwt/tempest/lib.src/cm5/blizzard/traps.c init_traps */
			      /* -guard_interval- */
	int NumberOfExpires;
} TimerBlock;


extern void *user_timers; 
extern int CMNA_self_address;  /* ZXU changed node_number to CMNA_self_address*/


/*
 * SEE /p/wwt/tempest/lib.src/cm5/blizzard
 * for TPPI_schedule_timer code.
 */
typedef void (*TPPI_TimerHandlerPtr)(void *);
void TPPI_schedule_timer(int ticks, TPPI_TimerHandlerPtr handler, void *user_ptr);
/* make sure to initiatlze user_timers to NULL in traps.c (blizzard) -zxu */


static int in_alarm_expire_handler = 0;

int gcd(int a, int b) /* find the greates common devisor */
{
	while(a != b)
	{	 if(a>b)
		{        a = a%b ;
			if(a==0) return b ;
		} else
		{        b = b%a ;
			if(b==0) return a ;
		}
	}
	return a ;
}
#include <sys/time.h>
void resolve_iticks(int ticks)
/* This rountine is not used right now. I just try to use the routine to
   express the idea what we may need to do if the tick is coarse than
   our sampleInterval, The parameter ticks in the it_value and it_interval
   set by blizzard --zxu 
*/
{
	struct itimerval interval;
	int iticks ;

	CMOS_getitimer (1, &interval);
	iticks =  interval.it_interval.tv_sec * 1000000+interval.it_interval.tv_usec ;
	if(iticks>ticks)
	{/* need to resolve */
		printf("resolve minimal interval ------------\n") ;
		iticks = gcd(ticks, iticks) ;
		interval.it_interval.tv_sec = ((int) (iticks) / 1000000);
		interval.it_interval.tv_usec = iticks % 1000000;
		interval.it_value.tv_sec = ((int) (iticks) / 1000000);
		interval.it_value.tv_usec = iticks % 1000000;
		CMOS_setitimer (1, &interval, NULL) ;
	}
}

#include <cm/timers.h> /* know the actual interval only */
void runHandler_setNextAlarm(TimerBlock *tb) 
/* 
 */
{
	/*
	int me ;
	CM_TIME cmtv ;
	static double  last_alarm=0.0 ;
	static double  this_alarm=0.0 ;
	*/
	
	if(!in_alarm_expire_handler)         /* need atomic */
	{	/* block out possible nested call -ZXU                      */
		in_alarm_expire_handler = 1 ;
		(tb->handler)();            /* run the handler              */
		tb->NumberOfExpires++;      /* keep track the total expires */

		/*
                me = blizzard_identify()  ;                                            
		if(me == 2)
		{	CMOS_get_time(cmtv) ;
			this_alarm = CM_ni_time_in_sec(cmtv) ;
		 	printf("alarm_interval = %g (seconds)\n", (this_alarm - last_alarm)) ; 
			last_alarm = this_alarm ;
		}
		*/
		TPPI_schedule_timer(tb->ticks, runHandler_setNextAlarm, (void *)tb);
      	}		
	in_alarm_expire_handler = 0;
}

void RecurringBlizzardAlarm(int interval, void (*handler)()) 
{
    TimerBlock *tb;

    tb = (TimerBlock *) malloc(sizeof(TimerBlock));
    tb->NumberOfExpires = 0;
    if (interval > 100)               /* why? because guard_interval = 100                       */
    	interval /= 50000;              /* was originaly 2000, The asynchronousness (50000 worked) */
					/* guard_interval = 100, however it seems that it is round */
					/* to 10000us by the system                                */
    tb->ticks = interval;
    tb->handler = handler;
    TPPI_schedule_timer(interval, runHandler_setNextAlarm, (void *) tb);
}

void clear_out_timers() {
	/* clear out all remaining timers */
	/* i can not see why?             */
	user_timers = 0;
}

extern volatile int DYNINSTtotalAlaramExpires;
static int lastAlarmExpires = 0;
void PrintAlarmInfo() 
{
	if (lastAlarmExpires != DYNINSTtotalAlaramExpires) {
		printf("DYNINSTtotalAlaramExpires at %d on node %d\n", 
			DYNINSTtotalAlaramExpires, CMNA_self_address);
		lastAlarmExpires = DYNINSTtotalAlaramExpires;
	}
}


/*
	SIGNAL STUFF
*/

void Sig15Handler(int sig, int code, struct sigcontext scp, char *addr) 
{
 	printf("signal %d caught on node %d.  Address %x\n", sig, code, (int) addr);
}

void SetupSig15() 
{
	signal(SIGTERM, Sig15Handler);
}


int blizzard_identify() 
{
	return (CMNA_self_address);
}
