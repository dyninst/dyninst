/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.13  1994/04/04 21:36:12  newhall
 * added synchronization code to DM thread startup
 *
 * Revision 1.12  1994/04/01  20:17:22  hollings
 * Added init of well known socket fd global.
 *
 * Revision 1.11  1994/03/25  22:59:33  hollings
 * Made the data manager tolerate paraynd's dying.
 *
 * Revision 1.10  1994/03/24  16:41:20  hollings
 * Added support for multiple paradynd's at once.
 *
 * Revision 1.9  1994/03/21  20:32:48  hollings
 * Changed the mid to mi mapping to be per paradyn daemon.  This is required
 * because mids are asigned by the paradynd's, and are not globally unique.
 *
 * Revision 1.8  1994/03/20  01:49:48  markc
 * Gave process structure a buffer to allow multiple writers.  Added support
 * to register name of paradyn daemon.  Changed addProcess to return type int.
 *
 * Revision 1.7  1994/03/08  17:39:33  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.6  1994/02/25  20:58:11  markc
 * Added support for storing paradynd's pids.
 *
 * Revision 1.5  1994/02/24  04:36:31  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 *
 * Revision 1.4  1994/02/08  21:05:55  hollings
 * Found a few pointer problems.
 *
 * Revision 1.3  1994/02/03  23:26:58  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.2  1994/02/02  00:42:33  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 * Revision 1.1  1994/01/28  01:34:17  hollings
 * The initial version of the Data Management thread.
 *
 *
 */
#include <assert.h>
extern "C" {
#include <math.h>
double   quiet_nan(int unused);
#include <malloc.h>
#include "thread/h/thread.h"
}

#include "dataManager.SRVR.h"
#include "dyninstRPC.CLNT.h"
#include "DMinternals.h"
#include "paradyn.h"

static dataManager *dm;
stringPool metric::names;
HTable<metric *> metric::allMetrics;
List<paradynDaemon*> paradynDaemon::allDaemons;

metricInstance *performanceStream::enableDataCollection(resourceList *rl, 
							metric *m)
{
    metricInstance *mi;

    if (!m || !rl) return(NULL);

    mi = m->enabledCombos.find(rl);
    if (mi) {
        mi->count++;
	mi->users.add(this);
    } else {
	mi = appl->enableDataCollection(rl, m);
	if (mi) mi->users.add(this);
    }
    return(mi);
}

//
// Turn off data collection for this perf stream.  Other streams may still
//    get the data.
//
void performanceStream::disableDataCollection(metricInstance *mi)
{
    mi->count--;
    mi->users.remove(this);
    if (!mi->count) {
	appl->disableDataCollection(mi);
    }
}


void performanceStream::enableResourceCreationNotification(resource *r)
{
    r->notify.add(this);
}

void performanceStream::disableResourceCreationNotification(resource *r)
{
    r->notify.remove(this);
}

void performanceStream::callSampleFunc(metricInstance *mi,
				       double startTimeStamp,
				       double endTimeStamp,
				       double value) 
{
    if (dataFunc.sample) {
	dm->setTid(threadId);
	dm->newPerfData(dataFunc.sample, this, mi, startTimeStamp, 
		    endTimeStamp, value);
    }
}

void performanceStream::callResourceFunc(resource *p,
				         resource *c,
				         char *name)
{
    if (controlFunc.rFunc) {
	dm->setTid(threadId);
	dm->newResourceDefined(controlFunc.rFunc, this, p, c, name);
    }
}


void performanceStream::callFoldFunc(timeStamp width)
{
    if (controlFunc.fFunc) {
	dm->setTid(threadId);
	dm->histFold(controlFunc.fFunc, this, width);
    }
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(int program,
				      String parentString,
				      String newResource,
				      String name)
{
    resource *parent;

    // create the resource.
    if (*parentString != '\0') {
	// non-null string.
	parentString = resource::names.findAndAdd(parentString);
	parent = resource::allResources.find(parentString);
	if (!parent) abort();
    } else {
	parent = resource::rootResource;
    }

    createResource(parent, name);
}

//
// used when a new program gets forked.
//
void dynRPCUser::newProgramCallbackFunc(int pid,
					int argc, 
					String_Array argvString,
					String machine_name)
{
     char **argv;
     paradynDaemon *daemon;
     List<paradynDaemon*> curr;
     int i;

    // there better be a paradynd running on this machine!
    for (curr=paradynDaemon::allDaemons, daemon = NULL; *curr; curr++) {
	if (!strcmp((*curr)->machine, machine_name))
	    daemon = *curr;
    }
    // for now, abort if there is no paradynd, this should not happen
    if (!daemon) {
	printf("process started on %s, can't find paradynd there\n",
		machine_name);
	exit(-1);
    }
   argv = (char **) malloc (argvString.count);
   if (!argv) {
	printf(" cannot malloc memory in newProgramCallbackFunc\n");
	exit(-1);
   }
   for (i=0; i<argvString.count; ++i) {
	argv[i] = strdup(argvString.data[i]);
	if (!argv[i]) {
		printf(" cannot malloc memory in newProgramCallbackFunc\n");
		exit(-1);
	}
   }
      
   assert (dm->appContext);
   assert (!dm->appContext->addRunningProgram(pid, argc, argv, daemon));
}

void dynRPCUser::newMetricCallback(metricInfo info)
{
    addMetric(info);
}

void dynRPCUser::sampleDataCallbackFunc(int program,
					   int mid,
					   double startTimeStamp,
					   double endTimeStamp,
					   double value)
{
    assert(0 && "Invalid virtual function");
}

void paradynDaemon::sampleDataCallbackFunc(int program,
					   int mid,
					   double startTimeStamp,
					   double endTimeStamp,
					   double value)
{
    component *part;
    metricInstance *mi;
    struct sampleInterval ret;

    mi = activeMids.find((void*) mid);
    if (!mi) {
	printf("ERROR: data for unknown mid: %d\n", mid);
	exit(-1);
    }

    if (mi->components.count() != 1) {
	// find the right component.
	part = mi->components.find(this);

	if (!part) {
	    printf("Unable to find component!!!\n");
	    exit(-1);
	}
	ret = part->sample.newValue((time64) endTimeStamp, value);
    }
    ret = mi->sample.newValue(mi->parts, (time64) endTimeStamp, value);

    if (ret.valid) {
	mi->enabledTime += ret.end - ret.start;
	mi->data->addInterval(ret.start, ret.end, ret.value, FALSE);
    }
}

paradynDaemon::~paradynDaemon() {
    metricInstance *mi;
    HTable<metricInstance*> curr;

    allDaemons.remove(this);

    // remove the metric ID as required.
    for (curr = activeMids; mi = *curr; curr++) {
	mi->parts.remove(this);
	mi->components.remove(this);
    }
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
dynRPCUser::reportSelf (String m, String p, int pd)
{
  assert(0);
  return;
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
paradynDaemon::reportSelf (String m, String p, int pd)
{
  machine = strdup(m);
  program = strdup(p);
  my_pid = pd;
  return;
}

// 
// establish socket that will be advertised to paradynd's
// this socket will allow paradynd's to connect to paradyn for pvm
//
static void
DMsetupSocket (int *sockfd, int *known_sock)
{
  // setup "well known" socket for pvm paradynd's to connect to
  assert ((*known_sock =
	   RPC_setup_socket (sockfd, AF_INET, SOCK_STREAM)) >= 0);

  // this info is needed to create argument list for other paradynds
  dm->socket = *known_sock;
  dm->sock_fd = *sockfd;

  // bind fd for this thread
  msg_bind (*sockfd, TRUE);
}

static void
DMnewParadynd (int sockfd, dataManager *dm)
{
  int new_fd;

  // accept the connection
  new_fd = RPC_getConnect(sockfd);
  if (new_fd < 0) {
    printf ("unable to connect to new paradynd\n");
    exit(-1);
  }

  assert (dm->appContext);
  assert (!dm->appContext->addDaemon(new_fd));
}

//
// Main loop for the dataManager thread.
//
void *DMmain(int arg)
{
    int ret;
    unsigned int tag;
    List<paradynDaemon*> curr;
    int known_sock, sockfd;
    char DMbuff[64];
    unsigned int msgSize = 64;

    thr_name("Data Manager");

    dm = new dataManager(arg);
    // this will be set on addExecutable
    dm->appContext = 0;

    // supports argv passed to paradynDaemon
    // new paradynd's may try to connect to well known port
    DMsetupSocket (&sockfd, &known_sock);
    dynRPCUser::__wellKnownPortFd__ = sockfd;

    msg_send (MAINtid, MSG_TAG_DM_READY, (char *) NULL, 0);
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, DMbuff, &msgSize);

    while (1) {
	tag = MSG_TAG_ANY;
	ret = msg_poll(&tag, TRUE);
	assert(ret != THR_ERR);

	if (tag == MSG_TAG_FILE) {
	  // must be an upcall on something speaking the dynRPC protocol.
	  for (curr = paradynDaemon::allDaemons; *curr; curr++) {
	    if ((*curr)->fd == ret) {
	      (*curr)->awaitResponce(-1);
	    }
	  }
	  if (ret == sockfd)
	    DMnewParadynd(sockfd, dm);        // set up a new paradynDaemon
	} else {
	    dm->mainLoop(); 
	}
    }
}


void addMetric(metricInfo info)
{
    char *iName;
    metric *met;
    performanceStream *stream;
    List<performanceStream *> curr;

    iName = metric::names.findAndAdd(info.name);
    assert(iName);
    met = metric::allMetrics.find(iName);
    if (met) {
	// check that it is compatible ????
	return;
    }

    //
    // It's really new 
    //
    met = new metric(info);
    metric::allMetrics.add(met, iName);

    //
    // now tell all perfStreams
    //
    for (curr = applicationContext::streams; *curr; curr++) {
	stream = *curr;
	if (stream->controlFunc.mFunc) {
	    // set the correct destination thread.
	    dm->setTid(stream->threadId);
	    dm->newMetricDefined(stream->controlFunc.mFunc, stream, met);
	}
    }
}


resource *createResource(resource *p, char *newResource)
{
    resource *ret;
    char *fullName;
    resource *temp;
    performanceStream *stream;
    List<performanceStream *> curr;

    /* first check to see if the resource has already been defined */
    temp = p->children.find(newResource);
    if (temp) return(temp);

    /* then create it */
    ret = new resource(p, newResource);
    fullName = ret->getFullName();

    /* inform others about it */
    for (curr = applicationContext::streams; stream = *curr; curr++) {
	stream->callResourceFunc(p, ret, fullName);
    }

    return(ret);
}
