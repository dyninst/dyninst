#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include "pvm3.h"
#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "piggyback.h"

/* really should ensure this tag is unique */
#define PIGGY_MSG		0x0000beef

void DYNINSTrecv_Piggy_Hook() {}
void DYNINSTsend_Piggy_Hook() {}
static int DYNINSTinSendInst;
static int DYNINSTinRecvInst;
static int DYNINST_Piggy_Init_Done;

struct skipTid {
    int tid;
    struct skipTid *next;
};

static struct skipTid *skipList;

static int exchangesPiggyMsgs(int tid)
{
    struct skipTid *item;

    for (item = skipList; item; item = item->next) {
	if (item->tid == tid) return(0);
    }

    return(1);
}

static void setSkipPiggyMsg(int tid)
{
    struct skipTid *item;

    if (!exchangesPiggyMsgs(tid)) return;

    item = (struct skipTid *) malloc(sizeof(struct skipTid));
    item->next = skipList;
    item->tid = tid;
    skipList = item;
}

/*
 * Special version of message comparison routine to verify that if we are
 *   not in inside instrumentation we don't receive the Piggy message by 
 *   accident.
 */
static int DYNINSTpvmCompare(int buf, int tid, int tag)
{
    int cc;
    int dummy, id, tg;

    if ((cc = pvm_bufinfo(buf, &dummy, &tg, &id)) < 0) {
	return cc;
    }
    /* check for PIGGY_MSG out of turn */
    if (tg == PIGGY_MSG) {
	if (!DYNINSTinSendInst && !DYNINSTinRecvInst) {
	    return 0;
	}
    }
    return ((tid == -1 || tid == id) &&  (tag == -1 || tag == tg)) ? 1 : 0;
}

void DYNINSTpvmPiggyInit()
{
    /* enroll in pvm */
    (void) pvm_mytid();

    DYNINST_Piggy_Init_Done = 1;

    pvm_recvf(DYNINSTpvmCompare);
}

void DYNINSTpvmPiggySend(int tid, int tag)
{
    int appBuffer;
    int cpMsgBuffer;

    if (!DYNINST_Piggy_Init_Done) DYNINSTpvmPiggyInit();

    if (!DYNINSTinSendInst) {
	DYNINSTinSendInst = 1;

	if (exchangesPiggyMsgs(tid) && (tid != pvm_mytid())) {
	    int count = 0;
	    /* setup temp buffer for Piggy message */
	    cpMsgBuffer = pvm_mkbuf(PvmDataDefault);
	    if (cpMsgBuffer < 0) {
		pvm_perror("Paradyn: unable to create Piggy send buffer");
		pvm_exit();
		abort();
	    }

	    appBuffer = pvm_setsbuf(cpMsgBuffer);
	    if (appBuffer <= 0) {
		pvm_perror("Paradyn: unable to set C.P. send Buffer");
		pvm_exit();
		abort();
	    }

	    /* hook to send Piggy messages */
	    DYNINSTsend_Piggy_Hook();

	    pvm_pkint(&count, 1, 1);
	    pvm_send(tid, PIGGY_MSG);

	    pvm_setsbuf(appBuffer);
	    pvm_freebuf(cpMsgBuffer);
	}

	DYNINSTinSendInst = 0;
    }
}


void DYNINSTpvmPiggyMcast(int *tids, int count)
{
    int i;
    int appBuffer;
    int cpMsgBuffer;
    int endPiggyMessage = 0;


    if (!DYNINST_Piggy_Init_Done) DYNINSTpvmPiggyInit();

    if (!DYNINSTinSendInst) {
	DYNINSTinSendInst = 1;

	/* setup temp buffer for Piggy message */
	cpMsgBuffer = pvm_mkbuf(PvmDataDefault);
	if (cpMsgBuffer < 0) {
	    pvm_perror("Paradyn: unable to create Piggy send buffer");
	    pvm_exit();
	    abort();
	}

	appBuffer = pvm_setsbuf(cpMsgBuffer);
	if (appBuffer <= 0) {
	    pvm_perror("Paradyn: unable to set C.P. send Buffer");
	    pvm_exit();
	    abort();
	}

	/* hook to send Piggy messages */
	DYNINSTsend_Piggy_Hook();

	pvm_pkint(&endPiggyMessage, 1, 1);

	for (i=0; i < count; i++) {
	    if (exchangesPiggyMsgs(tids[i])) {
		pvm_send(tids[i], PIGGY_MSG);
	    }
	}

	pvm_setsbuf(appBuffer);
	pvm_freebuf(cpMsgBuffer);

	DYNINSTinSendInst = 0;
    }
}

#ifdef notdef
void DYNINSTpvmPiggyBcast(char *group, int tag)
{
    int tid;
    int appBuffer;
    int cpMsgBuffer;

    if (!DYNINST_Piggy_Init_Done) DYNINSTpvmPiggyInit();

    if (!DYNINSTinSendInst) {
	DYNINSTinSendInst = 1;

	/* setup temp buffer for Piggy message */
	cpMsgBuffer = pvm_mkbuf(PvmDataDefault);
	if (cpMsgBuffer < 0) {
	    pvm_perror("Paradyn: unable to create Piggy send buffer");
	    pvm_exit();
	    abort();
	}

	appBuffer = pvm_setsbuf(cpMsgBuffer);
	if (appBuffer <= 0) {
	    pvm_perror("Paradyn: unable to set C.P. send Buffer");
	    pvm_exit();
	    abort();
	}

	if (exchangesPiggyMsgs(tid)) {
	    int count = 0;

	    /* hook to send Piggy messages */
	    DYNINSTsend_Piggy_Hook();

	    pvm_pkint(&count, 1, 1);
	    pvm_bcast(group, PIGGY_MSG);
	}

	pvm_setsbuf(appBuffer);
	pvm_freebuf(cpMsgBuffer);

	DYNINSTinSendInst = 0;
    }
}
#endif

void DYNINSTpvmPiggyRecv()
{
    int ret;
    int bytes;
    int tag, tid;
    int appBuffer;
    int cpMsgBuffer;

    if (!DYNINST_Piggy_Init_Done) DYNINSTpvmPiggyInit();

    if (!DYNINSTinRecvInst) {
	DYNINSTinRecvInst = 1;

	/* we want no active buffer at this point since recv sets one up */
	appBuffer = pvm_setrbuf(0);
	pvm_bufinfo(appBuffer, &bytes, &tag, &tid);

	if (exchangesPiggyMsgs(tid)) {
	    ret = pvm_probe(tid, PIGGY_MSG);
	    /* since PVM ensures fifo delivery and we send the Piggy message
	     * before the data message, we know this means that the other
	     * process is not playing along with us and sending Piggy messages.
	     *   - This is caused by having some processes that are not
	     *     instrumented (e.g. pvmgs).
	     */
	    if (ret > 0) {
		pvm_recv(tid, PIGGY_MSG);

		DYNINSTProcessPiggyMessage();

		cpMsgBuffer = pvm_getrbuf();
		if (cpMsgBuffer > 0) pvm_freebuf(cpMsgBuffer);
	    } else if (ret == 0) {
		setSkipPiggyMsg(tid); 
		fprintf(stdout, "%x: tid %x not participating in Piggy\n", 
		    pvm_mytid(), tid);
	    } else {
		pvm_perror("pvmprobe");
		exit(-1);
	    }
	}

	pvm_setrbuf(appBuffer);
	DYNINSTinRecvInst = 0;
    }
}

/* Bundle basic types for piggypacking onto messages. 
 *   These operations need to be done in the correct way for the given
 *   message passing library.
 *
 */
void DYNINSTpiggyPackInt(int *val)  { pvm_pkint(val, 1, 1); }

void DYNINSTpiggyPackDouble(double *val)  { pvm_pkdouble(val, 1, 1); }

void DYNINSTpiggyPackStr(char *val)  
{ 
    int length;

    length = strlen(val);
    pvm_pkint(&length, 1, 1); 
    pvm_pkbyte(val, length, 1); 
}


void DYNINSTpiggyUnpackInt(int *val)  { pvm_upkint(val, 1, 1); }

void DYNINSTpiggyUnpackDouble(double *val)  { pvm_upkdouble(val, 1, 1); }

void DYNINSTpiggyUnpackStr(char **val)  
{ 
    int length;

    pvm_upkint(&length, 1, 1); 
    *val = calloc(length+1, 1);
    pvm_upkbyte(*val, length, 1); 
}
