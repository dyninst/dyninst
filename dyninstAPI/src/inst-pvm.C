/*
 * PVM Specific Instrumentation.
 *
 * $Log: inst-pvm.C,v $
 * Revision 1.1  1994/01/27 20:31:21  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/12/15  21:03:07  hollings
 * Initial revision
 *
 *
 */
#include <assert.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <string.h>
#include <errno.h>

#include "rtinst/h/trace.h"
#include "dyninst.h"
#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "dyninstPVM.h"
#include "pvm3/include/pvm3.h"

libraryList msgFilterFunctions;
libraryList msgByteSentFunctions;
libraryList msgByteRecvFunctions;
libraryList msgByteFunctions;
libraryList fileByteFunctions;
libraryList libraryFunctions;

process *nodePseudoProcess;
resource machineResource;

static int *slaves;
struct hostinfo *hinfo;
static int nhosts, narch;
static AstNode tagArg(Param, (void *) 1);

instMaping defaultInst[] = {
    { "main", "DYNINSTinit", FUNC_ENTRY },
    { "main", "DYNINSTsampleValues", FUNC_EXIT },
    { "exit", "DYNINSTsampleValues", FUNC_ENTRY },
    { "pvm_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "pvm_recv", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY },
    { NULL, NULL, 0},
};

void addLibFunc(libraryList *list, char *name, int arg)
{
    libraryFunc *temp = new libraryFunc(name, arg);
    list->add(temp, (void *) temp->name);
}

char *getProcessStatus(process *proc)
{
     return("print status not supported for PVM");
}

/*
 * Define the various classes of library functions to inst.
 *
 */
void initLibraryFunctions()
{
    extern void machineInit();

    machineInit();

    addLibFunc(&msgByteSentFunctions, "pvm_send", TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "pvm_recv", TAG_LIB_FUNC);

    addLibFunc(&fileByteFunctions, "write",
	    TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);
    addLibFunc(&fileByteFunctions, "read",
	    TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);

    addLibFunc(&libraryFunctions, "DYNINSTsampleValues", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "exit", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "fork", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "main", 0);
    addLibFunc(&libraryFunctions, "pvm_bufinfo", TAG_LIB_FUNC);

    addLibFunc(&msgFilterFunctions, "pvm_barrier", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgFilterFunctions, "pvm_mcast", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgFilterFunctions, "pvm_send", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgFilterFunctions, "pvm_recv", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
	
    libraryFunctions += fileByteFunctions;
    // libraryFunctions += msgByteSentFunctions;
    // libraryFunctions += msgByteRecvFunctions;
    libraryFunctions += msgFilterFunctions;

    msgByteFunctions += msgByteSentFunctions;
    msgByteFunctions += msgByteRecvFunctions;
}

process *createGlobalPseudoProcess(process *sampleNodeProc)
{
    return(NULL);
}

int flushPtrace()
{
    return(0);
}

/*
 * The performance consultant's ptrace, it calls PVM routines as needed.
 *
 */
int PCptrace(int request, process *proc, void *addr, int data, void *addr2)
{
    int ret;
    int bid;
    int count;
    int paradynd;
    int allDone = 0;

    paradynd = proc->thread;

    pvm_initsend(0);
    pvm_pkint(&proc->pid, 1,1);
    if (request == PTRACE_WRITEDATA) {
	pvm_pkint(&request, 1, 1);
	pvm_pkint(&data, 1, 1);
	pvm_pkint((int *) &addr, 1, 1);
	pvm_pkbyte((char *) addr2, data, 1);
    } else if (request == PTRACE_DUMPCORE) {
	pvm_pkint(&request, 1, 1);
    } else if (request == PTRACE_CONT) {
	pvm_pkint(&request, 1, 1);
	pvm_pkint(&data, 1,1);
    } else if (request == PTRACE_POKETEXT) {
	pvm_pkint(&request, 1, 1);
	pvm_pkint((int *) &addr, 1, 1);
	pvm_pkint((int *) &data, 1, 1);
    } else if (request == PTRACE_INTERRUPT) {
	pvm_pkint(&request, 1, 1);
    } else if (request == PTRACE_READDATA) {
	pvm_pkint(&request, 1, 1);
	pvm_pkint(&data, 1, 1);
	pvm_pkint((int *) &addr, 1, 1);

	pvm_pkint(&allDone, 1, 1);
	pvm_send(paradynd, PTRACE_REQUEST);

	bid = pvm_recv(paradynd, PTRACE_RESPONCE);
	pvm_upkint(&count, 1, 1);
	if (count >= 0) {
	    pvm_upkbyte((char *) addr2, count, 1);
	} else {
	    errno = -count;
	}
	return(0);
    } else {
	abort();
    }
    pvm_pkint(&allDone, 1, 1);
    pvm_send(paradynd, PTRACE_REQUEST);
    pvm_recv(paradynd, PTRACE_RESPONCE);
    pvm_upkint(&ret, 1, 1);
    return(ret);
}

#define NS_TO_SEC       1000000000.0

StringList<int> primitiveCosts;

void initPrimitiveCost()
{
    /* based on measured values for the CM-5. */
    /* Need to add code here to collect values for other machines */
    primitiveCosts.add(728, (void *) "DYNINSTincrementCounter");
    primitiveCosts.add(728, (void *) "DYNINSTdecrementCounter");
    primitiveCosts.add(1159, (void *) "DYNINSTstartWallTimer");
    primitiveCosts.add(1939, (void *) "DYNINSTstopWallTimer");
    primitiveCosts.add(1296, (void *) "DYNINSTstartProcessTimer");
    primitiveCosts.add(2365, (void *) "DYNINSTstopProcessTimer");
}

/*
 * return the time required to execute the passed primitive.
 *
 */
float getPrimitiveCost(char *name)
{
    float ret;

    ret = primitiveCosts.find(name)/NS_TO_SEC;
    if (ret == 0.0) {
        printf("no cost value for primitive %s, using 10 usec\n", name);
        ret = 10000/NS_TO_SEC;
    }
    ret = 0.0;
    return(ret);
}

/*
 * dummy routines to make the rest of the system link.  We should remove refs
 *   to these ASAP.
 *
 */
int findNodeOffset(char *file, int offset)
{
    // only one executable per image for this model.
    assert(offset == 0);
    return(0);
}

int forkNodeProcesses(process *proc, traceHeader *header, traceFork *rec)
{
    abort();
    return(0);
}

int processPtraceAck(traceHeader* header, ptraceAck *recordData)
{
    abort();
    return(0);
}

int sendPtraceBuffer(process *curr)
{
    abort();
    return(0);
}

struct names {
    char *base;
    int count;
    struct names *next;
} names;


char *generateName(char *np)
{
    char *ret;
    struct names *curr;
    static struct names *nameList;

    for (curr = nameList; curr; curr=curr->next) {
	if (!strcmp(np, curr->base)) break;
    }
    if (curr) {
	ret = (char *) malloc(strlen(curr->base)+6);
	sprintf(ret, "%s[%d]", curr->base, curr->count++);
    } else {
	curr = (struct names *) calloc(sizeof(struct names), 1);
	curr->base = (char *) malloc(strlen(np)+1);
	ret = curr->base;
	strcpy(curr->base, np);
	curr->count = 0;
	curr->next = nameList;
	nameList = curr;
    }
    return(ret);
}

/*
 * somewhere in the VM a process has been created.
 *
 */
void handlePVMspawn(int tid, int pid, char *command)
{
    int i;
    char *np;
    process *ret;
    char *name;
    applicationContext app;
    struct executableRec *newExec;
    applicationContext context;
    extern Boolean applicationPaused;
    extern List<applicationContext> applicationContexts;

    context = *applicationContexts;
    np = strrchr(command,  '/');
    if (np) 
        np++;
    else
        np = command;

    name = generateName(np);

    /* ??? - this seems like a hack */
    /* if this is the first process, mark the app paused */
    applicationPaused = True;

    ret = allocateProcess(pid, name);
    ret->pid = pid;
    ret->thread = tid;
    ret->symbols = parseImage(command, 0);
    ret->traceLink = -1;
    ret->parent = NULL;
    if (ret->symbols) {
	initInferiorHeap(ret, False);
	installDefaultInst(ret, defaultInst);
	ret->status = stopped;
    } else {
	ret->status = exited;
    }

    newExec = (struct executableRec *) calloc(sizeof(struct executableRec), 1);
    ret->appl = context;
    app = ret->appl;

    newExec->name = name;
    newExec->type = selfTermination;
    newExec->state = stopped;
    newExec->proc = ret;

    /* add to applicationContext's list */
    newExec->next = app->programs;
    app->programs = newExec;

    PCptrace(PTRACE_CONT, ret, (void *) 1, 0, NULL);

    for (i=0; i < nhosts; i++) {
	if (tid == slaves[i]) {
	    //
	    // We have now discovered a machine, create it.
	    //
	    (void) newResource(machineResource, (void *) slaves[i], 
		hinfo[i].hi_name, 0.0);
	}
    }

#ifdef notdef
    if (!firstProcess) {
	newExec->state = running;
	ret->status = running;
	PCptrace(PTRACE_CONT, ret, (void *) 1, 0, NULL);
    }
#endif
}

extern time64 firstRecordTime;

extern Boolean firstSampleReceived;
extern void createResource(traceHeader *header, struct _newresource *r);
extern void processSample(traceHeader *h, traceSample *s);

void processTraceRecord()
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;
    int bufStart;               /* current starting point */

    static char buffer[2048];   /* buffer for data */
    static int bufEnd = 0;      /* last valid data in buffer */

    pvm_upkint(&ret, 1, 1);
    pvm_upkbyte(&buffer[bufEnd], ret, 1);

    bufEnd += ret;
    bufStart = 0;

    while (bufStart < bufEnd) {
	if (bufEnd - bufStart < (sizeof(traceStream) + sizeof(header))) {
            break;
        }

	memcpy(&sid, &buffer[bufStart], sizeof(traceStream));
	bufStart += sizeof(traceStream);

	memcpy(&header, &buffer[bufStart], sizeof(header));
	bufStart += sizeof(header);

	recordData = &buffer[bufStart];
	bufStart +=  header.length;
	assert((header.length & 0x3) == 0);

	/*
	 * convert header to time based on first record.
	 *
	 */
	if (!firstRecordTime) {
	    double st;

	    firstRecordTime = header.wall;
	    st = firstRecordTime/1000000.0;
	    printf("started at %f\n", st);
	}
	header.wall -= firstRecordTime;

	switch (header.type) {
	    case TR_NEW_RESOURCE:
		createResource(&header, (struct _newresource *) recordData);
		break;

	    case TR_SAMPLE:
		processSample(&header, (traceSample *) recordData);
		firstSampleReceived = True;
		break;

	    case TR_EXIT:
		// curr->status = exited;
		break;

	    default:
		printf("got record type %d on sid %d\n", header.type, sid);
	}
    }

    /* copy those bits we have to the base */
    memcpy(buffer, &buffer[bufStart], bufEnd - bufStart);
    bufEnd = bufEnd - bufStart;
}

void processPVMEvent(int fd)
{
    int bid;
    int tid;
    int pid;
    int bytes;
    int request;
    char command[256];

    bid = pvm_recv(-1, -1);
    pvm_bufinfo(bid, &bytes, &request, &tid);
    switch (request) {
	 case PROCESS_STARTED:
	     pvm_upkstr(command);
	     pvm_upkint(&pid, 1, 1);
	     printf("process %s(%d) started by tid %d\n", command, pid, tid);
	     handlePVMspawn(tid, pid, command);
	     break;

	 case TRACE_RECORD:
	     processTraceRecord();
	     break;

	 case PROCESS_EXITED:
	     pvm_upkint(&pid, 1, 1);
	     printf("process %d on tid %d exited\n", pid, tid);
	     break;

	 case PRINT_MESSAGE:
	     char message[256];
	     pvm_upkstr(message);
	     printf("%d: %s", tid, message);
	     fflush(stdout);
	     break;

	 default:
	    abort();

    }
}

void startParadnHelper()
{
    int i;
    int info;
    int val;
    int mytid;
    int version;

    /* enroll in pvm */
    mytid = pvm_mytid();

    info = pvm_config(&nhosts, &narch, &hinfo);
    if (info) {
	printf("error getting pvm config\n");
	exit(-1);
    }

    slaves = (int *) calloc(sizeof(int), nhosts);
    if (!machineResource) {
	machineResource = newResource(rootResource, NULL, "Machine", 0.0);
    }

    for (i=0; i < nhosts; i++) {
	printf("starting paradynd on %s\n", hinfo[i].hi_name);
	val = pvm_spawn("paradynd", NULL, PvmTaskHost,
	   hinfo[i].hi_name, 1, &slaves[i]);
	if (val != 1) {
	    printf("error starting paradynd on %s\n", hinfo[i].hi_name);
	}
	pvm_recv(slaves[i], DAEMON_READY);
	pvm_upkint(&version, 1, 1);
	if (version != PARADYN_PVM_VERSION) {
	    printf("paradynd version is not compatible (expected %d, got %d)\n",
		PARADYN_PVM_VERSION, version);
	    exit(-1);
	}
	printf("tid = %d\n", slaves[i]);
    }
}

/*
 * machine specific init for PVM.
 *
 */
void machineInit()
{
    int i;
    int nfds;
    int *fds;

    /* make sure our paradn helper is running on every node. */
    startParadnHelper();

    nfds = pvm_getfds(&fds);
    for (i=0; i < nfds; i++) {
	addIoHandler(processPVMEvent, fds[i]);
    }
}

void instCleanup()
{
    pvm_exit();
}
