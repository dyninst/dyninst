/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.33  1994/06/27 21:23:25  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.32  1994/06/27  18:54:48  hollings
 * changed stdio printf for paradynd.
 *
 * Revision 1.31  1994/06/17  22:07:59  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.30  1994/06/14  15:22:46  markc
 * Added arg to enableDataCollection call to support aggregation.
 *
 * Revision 1.29  1994/06/02  23:25:19  markc
 * Added virtual function 'handle_error' to pardynDaemon class which uses the
 * error handling features that igen provides.
 *
 * Revision 1.28  1994/05/31  18:26:15  markc
 * strdup'd a string passed into createResource, since igen will free the memory
 * for the string on return from the function.
 *
 * Revision 1.27  1994/05/18  02:51:04  hollings
 * fixed cast one return of malloc.
 *
 * Revision 1.26  1994/05/18  00:43:28  hollings
 * added routine to print output of stdout.
 *
 * Revision 1.25  1994/05/17  00:16:38  hollings
 * Changed process id speperator from [] to {} to get around braindead tcl.
 *
 * Revision 1.24  1994/05/16  22:31:38  hollings
 * added way to request unique resource name.
 *
 * Revision 1.23  1994/05/12  23:34:00  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.22  1994/05/10  03:57:37  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.21  1994/05/09  20:56:20  hollings
 * added changeState callback.
 *
 * Revision 1.20  1994/05/02  20:37:45  hollings
 * Fixed compiler warning.
 *
 * Revision 1.19  1994/04/21  23:24:26  hollings
 * removed process name from calls to RPC_make_arg_list.
 *
 * Revision 1.18  1994/04/20  15:30:10  hollings
 * Added error numbers.
 * Added data manager function to get histogram buckets.
 *
 * Revision 1.17  1994/04/18  22:28:31  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.16  1994/04/12  22:33:34  hollings
 * Fixed casts back to time64 which were dropping off the fraction of seconds
 * in the timestamps of samples.
 *
 * Revision 1.15  1994/04/12  15:32:00  hollings
 * added tunable constant samplingRate to control the frequency of sampling.
 *
 * Revision 1.14  1994/04/11  23:18:49  hollings
 * added checks to make sure time moves forward.
 *
 * Revision 1.13  1994/04/04  21:36:12  newhall
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

#include "util/h/tunableConst.h"
#include "dataManager.SRVR.h"
#include "dyninstRPC.CLNT.h"
#include "DMinternals.h"
#include "../pdMain/paradyn.h"

static dataManager *dm;
stringPool metric::names;
HTable<metric *> metric::allMetrics;
List<paradynDaemon*> paradynDaemon::allDaemons;

void newSampleRate(float rate);

tunableConstant samplingRate(0.5, 0.0, 1000.0, newSampleRate, "samplingRate",
   "how often to sample intermediate performance data (in seconds)");

metricInstance *performanceStream::enableDataCollection(resourceList *rl, 
							metric *m,
							aggregation aggOp)
{
    char *name;
    metricInstance *mi;

    if (!m || !rl) return(NULL);

    name = rl->getCanonicalName();
    mi = m->enabledCombos.find(name);
    if (mi) {
        mi->count++;
	mi->users.add(this);
    } else {
	mi = appl->enableDataCollection(rl, m, aggOp);
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
				       sampleValue *buckets,
				       int count,
				       int first)
{
    if (dataFunc.sample) {
	dm->setTid(threadId);
	dm->newPerfData(dataFunc.sample, this, mi, buckets, count, first);
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

void performanceStream::callResourceBatchFunc(batchMode mode)
{
    if (controlFunc.bFunc) {
	dm->setTid(threadId);
	dm->changeResourceBatchMode(controlFunc.bFunc, this, mode);
    }
}

void performanceStream::callFoldFunc(timeStamp width)
{
    if (controlFunc.fFunc) {
	dm->setTid(threadId);
	dm->histFold(controlFunc.fFunc, this, width);
    }
}


void performanceStream::callStateFunc(appState state)
{
    if (controlFunc.sFunc) {
	dm->setTid(threadId);
	dm->changeState(controlFunc.sFunc, this, state);
    }
}

//
// IO from application processes.
//
void dynRPCUser::applicationIO(int pid, int len, String data)
{
    char *ptr;
    char *rest;
    // extra should really be per process.
    static char *extra;

    rest = data;
    ptr = strchr(rest, '\n');
    while (ptr) {
	*ptr = '\0';
	if (pid) {
	    printf("pid %d:", pid);
	} else {
	    printf("paradynd:", pid);
	}
	if (extra) {
	    printf(extra);
	    free(extra);
	    extra = NULL;
	}
	printf("%s\n", rest);
	rest = ptr+1;
	ptr = strchr(rest, '\n');
    }
    extra = (char *) malloc(strlen(rest)+1);
    strcpy(extra, rest);
}

abstractionType parseAbstractionName(String abstraction)
{
  if (!abstraction) return BASE;
  if (strcmp(abstraction,"TCL") == 0) return TCL;
  if (strcmp(abstraction,"CMF") == 0) return CMF;
  if (strcmp(abstraction,"BASE") == 0) return BASE;
  printf("DATAMANGER: bad abstraction '%s'\n",abstraction);
  return BASE;
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(int program,
				      String parentString,
				      String newResource,
				      String name,
				      String abstraction)
{
    resource *parent;
    abstractionType at;

    // create the resource.
    if (*parentString != '\0') {
	// non-null string.
	parentString = resource::names.findAndAdd(parentString);
	parent = resource::allResources.find(parentString);
	if (!parent) abort();
    } else {
	parent = resource::rootResource;
    }

// rbi
    at = parseAbstractionName(abstraction);
// rbi

    createResource(parent, name, at);
}

void dynRPCUser::mappingInfoCallback(int program,
				     String abstraction, 
				     String type, 
				     String key,
				     String value)

{
//  List<performanceStream *> curr;
//  performanceStream *stream;

/*  printf("DATAMANAGER: '%s' '%s' map '%s -> %s'\n", abstraction, 
	 type, key, value); */
/*  for (curr = applicationContext::streams; stream = *curr; curr++) {
     Make sure stream is of right abstraction 
    stream->callMappingFunc(abstraction, type, key, value);
  }
*/
  
}

class uniqueName {
  public:
    uniqueName(char *base) { name = base; nextId = 0; }
    int nextId;
    char *name;
};


String dynRPCUser::getUniqueResource(int program, 
				     String parentString, 
				     String newResource)
{
    char *ptr;
    uniqueName *ret;
    char newName[80];
    static List<uniqueName*> allUniqueNames;

    sprintf(newName, "%s/%s", parentString, newResource);
    ptr = resource::names.findAndAdd(newName);

    ret = allUniqueNames.find(ptr);

    if (!ret) {
	ret = new uniqueName(ptr);
	allUniqueNames.add(ret, ptr);
    }
    // changed from [] to {} due to TCL braindeadness.
    sprintf(newName, "%s{%d}", newResource, ret->nextId++);
    ptr = resource::names.findAndAdd(newName);

    return(ptr);
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
	printf("paradyn error #1 encountered\n");
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

    // printf("mid %d %f from %f to %f\n", mid, value, startTimeStamp, endTimeStamp);
    mi = activeMids.find((void*) mid);
    if (!mi) {
	printf("ERROR: data for unknown mid: %d\n", mid);
	printf("paradyn Error #2\n");
	exit(-1);
    }

    if (mi->components.count() != 1) {
	// find the right component.
	part = mi->components.find(this);

	if (!part) {
	    printf("Unable to find component!!!\n");
	    printf("paradyn Error #3\n");
	    exit(-1);
	}
	ret = part->sample.newValue(endTimeStamp, value);
    }
    ret = mi->sample.newValue(mi->parts, endTimeStamp, value);

    if (ret.valid) {
	assert(ret.end >= 0.0);
	assert(ret.start >= 0.0);
	assert(ret.end >= ret.start);
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
// When an error is determined on an igen call, this function is
// called, since the default error handler will exit, and we don't
// want paradyn to exit.
//
void paradynDaemon::handle_error()
{
   dm->appContext->removeDaemon(this, TRUE);
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
    printf("paradyn Error #4\n");
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

    paradynDaemon::args =
	      RPC_make_arg_list(AF_INET, SOCK_STREAM, known_sock, 1);

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


resource *createResource(resource *p, char *newResource, abstractionType at)
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
    ret = new resource(p, strdup(newResource), at);
    fullName = ret->getFullName();

    /* inform others about it if they need to know */
    for (curr = applicationContext::streams; stream = *curr; curr++) {
      if (stream->getAbstraction() == at) {
	stream->callResourceFunc(p, ret, fullName);
      }
    }

    return(ret);
}

void newSampleRate(float rate)
{
    List<paradynDaemon*> curr;

    for (curr = paradynDaemon::allDaemons; *curr; curr++) {
	(*curr)->setSampleRate(rate);
    }
}
