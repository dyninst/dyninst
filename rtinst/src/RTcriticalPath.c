
#include <malloc.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "rtinst/h/critPath.h"
#include "piggyback.h"

extern time64 DYNINSTgetCPUtime(void);

/* tags for piggyback messages.  This tag space is independent of PVM tags */
#define END_PIGGY_MSG    0      /* no more messages */
#define CP_MSG          10      /* Critical Path message */
#define CP_ZERO_MSG 	11	/* Critical Path with zeroing message */

struct cpClock {
    double total;		/* C.P. Length */
    double lastUpdate;		/* C.P. Length */
};

struct cpInfo {
    int id;			/* what function/process etc. */
    int active;			/* is this id currently active (accumulating) */
    int inUpdate;		/* used for mutex to update samples */
    struct cpClock cp;		/* C.P. Length */
    struct cpClock ids;		/* amount of C.P. due to id unit */
    struct cpClock cpCOPY;	/* C.P. Length */
    struct cpClock idsCOPY;	/* amount of C.P. due to id unit */
    int activeCOPY;	 	/* is this id currently active (accumulating) */
    struct cpInfo *next;	/* keep a list of these around */
};

struct cpInfo *allCPData;

struct cpInfo *DYNINSTaddCPId(int id)
{
    struct cpInfo *curr;

    curr = (struct cpInfo *) calloc(sizeof(struct cpInfo), 1);
    curr->id = id;
    curr->next = allCPData;
    allCPData = curr;
    return(curr);
}

void DYNINST_CP_SetActive(int id)
{
    double now;
    struct cpInfo *curr;

    now = (double) DYNINSTgetCPUtime();

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }

    if (!curr) {
	 curr = DYNINSTaddCPId(id);
	 curr->cp.lastUpdate = now;
	 curr->ids.lastUpdate = now;
	 /* set the counter to active */
    }
    curr->active = True;
}

/*
 * Start the Timer for the id (i.e. function call)
 */
void DYNINST_CP_StartTimer(int id)
{
    double now;
    struct cpInfo *curr;

    now = (double) DYNINSTgetCPUtime();

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }

    if (!curr) {
	 curr = DYNINSTaddCPId(id);
	 curr->cp.lastUpdate = now;
	 /* default the counter to on */
	 curr->ids.lastUpdate = now;
	 curr->active = False;
    }

    /* copy for mutex */
    curr->idsCOPY = curr->ids;
    curr->cpCOPY = curr->cp;
    curr->activeCOPY = curr->active;
    curr->inUpdate = 1;

    curr->active = 1;

    curr->ids.lastUpdate = now;

    curr->cp.total += now - curr->cp.lastUpdate;
    curr->cp.lastUpdate = now;

    curr->inUpdate = 0;
}

/*
 * Stop the Timer for the id (i.e. function return)
 */
void DYNINST_CP_StopTimer(int id)
{
    double now;
    struct cpInfo *curr;

    now = (double) DYNINSTgetCPUtime();

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }
    if (!curr) {
	 curr = DYNINSTaddCPId(id);
	 curr->cp.lastUpdate = now;
	 /* default the counter to on */
	 curr->ids.lastUpdate = now;
	 curr->active = False;
    }

    /* copy for mutex */
    curr->idsCOPY = curr->ids;
    curr->cpCOPY = curr->cp;
    curr->activeCOPY = curr->active;
    curr->inUpdate = 1;

    curr->active = 0;

    curr->ids.total += now - curr->ids.lastUpdate;
    curr->ids.lastUpdate = now;

    curr->cp.total += now - curr->cp.lastUpdate;
    curr->cp.lastUpdate = now;

    curr->inUpdate = 0;
}

/*
 * Handle a CP send event.
 *    - internal helper function to combine CP and CP Zero sends.
 */
static void DYNINST_CP_Do_Send(int id, int tag)
{
    double now;
    double temp;
    struct cpInfo *curr;

    now = (double) DYNINSTgetCPUtime();

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }
    if (!curr) {
	 curr = DYNINSTaddCPId(id);
	 curr->cp.lastUpdate = now;
	 /* default the counter to on */
	 curr->ids.lastUpdate = now;
	 curr->active = False;
    }

    /* copy for mutex */
    curr->idsCOPY = curr->ids;
    curr->cpCOPY = curr->cp;
    curr->activeCOPY = curr->active;
    curr->inUpdate = 1;

    curr->cp.total += (now - curr->cp.lastUpdate);
    curr->cp.lastUpdate = now;
    if (curr->active) {
	curr->ids.total += now - curr->ids.lastUpdate;
	curr->ids.lastUpdate = now;
    }

    DYNINSTpiggyPackInt(&tag);

    DYNINSTpiggyPackInt(&id);

    temp = (double) curr->cp.total;
    DYNINSTpiggyPackDouble(&temp);

    temp = (double) curr->ids.total;
    DYNINSTpiggyPackDouble(&temp);

    /* don't invoke send that is done by DYNINSTpvmPiggySend */
#ifdef notdef
    fprintf(stdout, "Send sent (id %d) %f, %f\n", id, curr->cp.total, curr->ids.total);
    fflush(stdout);
#endif

    curr->inUpdate = 0;
}

/*
 * Handle a CP send event.
 *
 */
void DYNINST_CP_Send(int id)
{
    DYNINST_CP_Do_Send(id, CP_MSG);
}

/*
 * Handle a CP send event.
 *
 */
void DYNINST_CP_Zero_Send(int id)
{
    DYNINST_CP_Do_Send(id, CP_ZERO_MSG);
}

/*
 * Receive a CP message.
 *
 */
void DYNINST_CP_Recv(int cpZero)
{
    int id;
    time64 now;
    double temp;
    time64 CPLength;
    time64 funcCPLength;
    struct cpInfo *curr;

    DYNINSTpiggyUnpackInt(&id);

    now = (double) DYNINSTgetCPUtime();

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }
    if (!curr) {
	curr = DYNINSTaddCPId(id);
	curr->cp.lastUpdate = now;
	/* default the counter to on */
	curr->ids.lastUpdate = now;
	curr->active = False;
    }

    /* copy for mutex */
    curr->idsCOPY = curr->ids;
    curr->cpCOPY = curr->cp;
    curr->activeCOPY = curr->active;
    curr->inUpdate = 1;

    DYNINSTpiggyUnpackDouble(&temp);
    CPLength = temp;

    DYNINSTpiggyUnpackDouble(&temp);
    funcCPLength = temp;

    curr->cp.total += (now - curr->cp.lastUpdate);
    curr->cp.lastUpdate = now;

    if (curr->active) {
	curr->ids.total += now - curr->ids.lastUpdate;
	curr->ids.lastUpdate = now;
    }

#ifdef notdef
    if (cpZero) fprintf(stdout, "CP Zero: ");
    fprintf(stdout, "Recv got (id %d) %f, %f\n", id, (double) CPLength, (double) funcCPLength);
    fprintf(stdout, "  had %f\n", (double) curr->cp.total);
    fflush(stdout);
#endif
    if (((CPLength > curr->cp.total) && !cpZero) ||
         ((CPLength - funcCPLength > curr->cp.total - curr->ids.total) && 
	 cpZero)) {
	curr->cp.total = CPLength;
	curr->ids.total = funcCPLength;
#ifdef notdef
	fprintf(stdout, " update total %f\n", (double) curr->cp.total);
	fflush(stdout);
#endif
    }

    curr->inUpdate = 0;
}

/*
 * Internal helper function.
 * 
 */
static void DYNINSTCP_Sample(int cpZero, int id)
{
    time64 now;
    int active;
    cpSample sample;
    struct cpClock cp;
    struct cpClock ids;
    struct cpInfo *curr;
    time64 wall_time,process_time;

#ifdef notdef
    fprintf(stdout, "DYNINSTCP_Sample called with flag = %d, id = %d\n", 
	 flag, id);
    fflush(stdout);
#endif

    process_time = DYNINSTgetCPUtime();
    now = process_time;
    wall_time = DYNINSTgetWalltime(); 

    for (curr= allCPData; curr; curr=curr->next) {
	if (curr->id == id) break;
    }
    if (!curr) {
	return;
    }

    /* make a copy of the values to use */
    if (curr->inUpdate) {
	cp = curr->cpCOPY;
	ids = curr->idsCOPY;
	active = curr->activeCOPY;
    } else {
	cp = curr->cp;
	ids = curr->ids;
	active = curr->active;
    }

    sample.id = id;
    sample.length = cp.total + (now - cp.lastUpdate);
    sample.share = ids.total;
    if (active) {
	sample.share += now - ids.lastUpdate;
    }
    if (cpZero) {
	/* subtract additional time in zeroed procedure from total and sample */
	sample.length = sample.length - sample.share;
	sample.share = sample.length;
    }

    DYNINSTgenerateTraceRecord(0, TR_CP_SAMPLE, sizeof(cpSample), &sample,
				wall_time,process_time);
}

/*
 * Invoke the correct form of DYNINST_Sample.  This is really an ugly hack
 *    to pass to boolen variable to DYNINST_Sample, but the first version of
 *    the MDL desn't support more than one argument to a function.
 *
 */
void DYNINST_CP_Sample(int id) { DYNINSTCP_Sample(0, id); }
void DYNINST_CP_Zero_Sample(int id) { DYNINSTCP_Sample(1, id); }

/*
 * Process and dispatch Piggy Backed messages.
 *
 */
void DYNINSTProcessPiggyMessage()
{
    int code;
    int done =0;

    while (!done) {
	 DYNINSTpiggyUnpackInt(&code);
	 switch (code) {
	     case END_PIGGY_MSG:
		 done = 1;
		 break;

	     case CP_MSG:
		 DYNINST_CP_Recv(0);
		 break;

	     case CP_ZERO_MSG:
		 DYNINST_CP_Recv(1);
		 break;

	     default:
	 }
    }
}
