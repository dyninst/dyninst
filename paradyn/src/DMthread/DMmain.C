/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.6  1994/02/25 20:58:11  markc
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

static dataManager *dm;
stringPool metric::names;
HTable<metric *> metric::allMetrics;
HTable<metricInstance*> component::allComponents;
List<paradynDaemon*> paradynDaemon::allDaemons;
char **paradynDaemon::args = 0;

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
void dynRPCUser::newProgramCallbackFunc(int program)
{
    abort();
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
    metricInstance *mi;
    performanceStream *ps;
    List<performanceStream*> curr;

    mi = component::allComponents.find((void*) mid);
    if (!mi) {
	printf("ERROR: data for unknown mid: %d\n", mid);
	exit(-1);
    }

    if (mi->components.count() != 1) {
	printf("ERROR: multiple data sources for one mi, not supported yet\n");
	exit(-1);
    }

    mi->enabledTime += endTimeStamp - startTimeStamp;
    mi->data->addInterval(startTimeStamp, endTimeStamp, value, FALSE);

    //
    // call callbacks for perfstreams.
    // 
    for (curr = mi->users; ps = *curr; curr++) {
	ps->callSampleFunc(mi, startTimeStamp, endTimeStamp,value);
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
  
  // setup arg list to pass
  // to prevent memory leaks this list could be freed by the destructor
  // this list is null terminated
  assert (paradynDaemon::args =
	  RPC_make_arg_list ("paradyndPVM", AF_INET, SOCK_STREAM, *known_sock, 1));

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

    thr_name("Data Manager");
    printf("mm running\n");

    // supports argv passed to paradynDaemon
    DMsetupSocket (&sockfd, &known_sock);

    dm = new dataManager(arg);
    // this will be set on addExecutable
    dm->appContext = 0;

    // new paradynd's may try to connect to well known port
    // hash define around this code to avoid compiling in, except for PVM


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
