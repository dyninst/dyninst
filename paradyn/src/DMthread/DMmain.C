/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.61  1995/02/27 18:43:03  tamches
 * Changes to reflect the new TCthread; syntax for creating/declaring
 * tunable constants, as well as syntax for obtaining current
 * value of tunable constants has changed.
 *
 * Revision 1.60  1995/02/26  02:14:03  newhall
 * added some of the phase interface support
 *
 * Revision 1.59  1995/02/16  19:10:41  markc
 * Removed start slash from comments
 *
 * Revision 1.58  1995/02/16  08:15:53  markc
 * Changed Boolean to bool
 * Changed interfaces for igen-xdr to use string/vectors rather than char igen-arrays
 * Check for buffered igen calls.
 *
 * Revision 1.57  1995/01/26  17:58:18  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.56  1994/12/21  00:36:43  tamches
 * Minor change to tunable constant declaration to reflect new tc constructors.
 * Fewer compiler warnings.
 *
 * Revision 1.55  1994/12/15  07:38:22  markc
 * Initialized count used to track resourceBatch requests.
 *
 * Revision 1.54  1994/11/11  23:06:49  markc
 * Check to see if status is non-null
 *
 * Revision 1.53  1994/11/11  07:08:51  markc
 * Added extra arg to RPC_make_arg_list to tell paradyndPVM that it should
 * start other paradyndPVMs
 *
 * Revision 1.52  1994/11/09  18:39:34  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.51  1994/11/03  21:58:49  karavan
 * Allow blank string for parent resource name so paradyndSIM will work
 *
 * Revision 1.50  1994/11/03  20:54:13  karavan
 * Changed error printfs to calls to UIM::showError
 *
 * Revision 1.49  1994/11/02  11:45:58  markc
 * Pass NULL rather than "" in resourceInfoCallback
 *
 * Revision 1.48  1994/09/30  19:17:45  rbi
 * Abstraction interface change.
 *
 * Revision 1.47  1994/09/22  00:55:37  markc
 * Changed "String" to "char*"
 * Changed arg types for DMsetupSocket
 * Made createResource() take a const char* rather than char*
 *
 * Revision 1.46  1994/09/05  20:03:07  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.45  1994/08/22  15:58:36  markc
 * Add code for class daemonEntry
 *
 * Revision 1.44  1994/08/17  17:56:24  markc
 * Added flavor paramater to paradyn daemon data structure.
 * Added flavor parameter to reportSelf function call.
 *
 * Revision 1.43  1994/08/05  16:03:57  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.42  1994/08/03  19:06:24  hollings
 * Added tunableConstant to print enable/disable pairs.
 *
 * Fixed fold to report fold info to all perfStreams even if they have
 * not active data being displayed.
 *
 * Revision 1.41  1994/07/28  22:31:08  krisna
 * include <rpc/types.h>
 * stringCompare to match qsort prototype
 * proper prorotypes for starting DMmain
 *
 * Revision 1.40  1994/07/26  20:03:05  hollings
 * added suppressSearch.
 *
 * Revision 1.39  1994/07/25  14:55:36  hollings
 * added suppress resource option.
 *
 * Revision 1.38  1994/07/20  18:59:24  hollings
 * added resource batch mode.
 *
 * Revision 1.37  1994/07/07  03:30:16  markc
 * Changed expected return types for appContext functions from integer to Boolean
 *
 * Revision 1.36  1994/07/05  03:27:17  hollings
 * added observed cost model.
 *
 * Revision 1.35  1994/07/02  01:43:11  markc
 * Removed all uses of type aggregation from enableDataCollection.
 * The metricInfo structure now contains the aggregation operator.
 *
 * Revision 1.34  1994/06/29  02:55:59  hollings
 * fixed code to remove instrumenation when done with it.
 *
 * Revision 1.33  1994/06/27  21:23:25  rbi
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
double   quiet_nan(int unused);
#include <malloc.h>
#include <stdio.h>
}

#include "thread/h/thread.h"
#include "../TCthread/tunableConst.h"
#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMinternals.h"
#include "../pdMain/paradyn.h"
#include "../UIthread/Status.h"

#include "util/h/Vector.h"
#include "DMphase.h"

static int wellKnownPortFd;
static dataManager *dm;
stringPool metric::names;
HTable<metric *> metric::allMetrics;
List<paradynDaemon*> paradynDaemon::allDaemons;

void newSampleRate(float rate); // callback routine for float TC "samplingRate"

// TEMPORARY
vector<performanceStream *> phase_notify_list;
vector<phaseInfo *> dm_phases; 


metricInstance *performanceStream::enableDataCollection(resourceList *rl, 
							metric *m)
{
    stringHandle name;
    metricInstance *mi;

    if (!m || !rl) return(NULL);

    name = rl->getCanonicalName();
    mi = m->enabledCombos.find(name);
    printf("%s enabled\n",(char *)name);
    if (mi) {
        mi->count++;
	mi->users.add(this);
    } else {
	mi = appl->enableDataCollection(rl, m);
	if (mi) {
	    mi->users.add(this);
	}
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
				         stringHandle name)
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

void performanceStream::callPhaseFunc(phaseInfo *phase)
{
    if (controlFunc.pFunc) {
	dm->setTid(threadId);
	dm->newPhaseInfo(controlFunc.pFunc,this,phase);
    }
}

//
// IO from application processes.
//
void dynRPCUser::applicationIO(int pid, int len, string data)
{
    char *ptr;
    char *rest;
    // extra should really be per process.
    static string extra;

    rest = P_strdup(data.string_of());
    char *tp = rest;
    ptr = P_strchr(rest, '\n');
    while (ptr) {
	*ptr = '\0';
	if (pid) {
	    printf("pid %d:", pid);
	} else {
	    printf("paradynd: %d", pid);
	}
	if (extra.length()) {
	    cout << extra;
	    extra = (char*) NULL;
	}
	cout << rest << endl;
	rest = ptr+1;
	ptr = P_strchr(rest, '\n');
    }
    delete tp;
    extra = rest;
}

extern status_line *DMstatus;

void dynRPCUser::resourceBatchMode(bool onNow)
{
    int prev;
    static int count=0;
    List<performanceStream*> curr;

    prev = count;
    if (onNow) {
	count++;
    } else {
	count--;
    }

    if (count == 0) {
	for (curr = applicationContext::streams; *curr; curr++) {
	    (*curr)->callResourceBatchFunc(batchEnd);
	}
    } else if (!prev) {
	for (curr = applicationContext::streams; *curr; curr++) {
	    (*curr)->callResourceBatchFunc(batchStart);
	}
    }
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(int program,
				      string parentString,
				      string newResource,
				      string name,
				      string abstr)
{
    resource *parent;
    stringHandle iName;

//
//  Commented out because it slows resource 
//  movement from paradynd to paradyn
//
//    (*DMstatus) << "receiving resources";

    // create the resource.
    if (parentString.length()) {
	// non-null string.
	iName = resource::names.findAndAdd(parentString.string_of());
	parent = resource::allResources.find(iName);
	if (!parent) abort();
    } else {
	parent = resource::rootResource;
    }
    createResource(parent, name.string_of(), abstr.string_of());

//
//  Commented out because it slows resource 
//  movement from paradynd to paradyn
//
//    (*DMstatus) << "ready";
}

void dynRPCUser::mappingInfoCallback(int program,
				     string abstraction, 
				     string type, 
				     string key,
				     string value)

{
  AMnewMapping(abstraction.string_of(),type.string_of(),key.string_of(),
	       value.string_of());    
}

class uniqueName {
  public:
    uniqueName(stringHandle base) { name = base; nextId = 0; }
    int nextId;
    stringHandle name;
};

// commented out since igen no longer supports sync upcalls
#ifdef notdef
char *dynRPCUser::getUniqueResource(int program, 
				    string parentString, 
				    string newResource)
{
    uniqueName *ret;
    char newName[80];
    stringHandle ptr;
    static List<uniqueName*> allUniqueNames;

    sprintf(newName, "%s/%s", parentString.string_of(), newResource.string_of());
    ptr = resource::names.findAndAdd(newName);

    ret = allUniqueNames.find(ptr);

    if (!ret) {
	ret = new uniqueName(ptr);
	allUniqueNames.add(ret, ptr);
    }
    // changed from [] to {} due to TCL braindeadness.
    sprintf(newName, "%s{%d}", newResource.string_of(), ret->nextId++);
    ptr = resource::names.findAndAdd(newName);

    return((char*)ptr);
}
#endif

//
// used when a new program gets forked.
//
void dynRPCUser::newProgramCallbackFunc(int pid,
					vector<string> argvString,
					string machine_name)
{
     paradynDaemon *daemon;
     List<paradynDaemon*> curr;

    // there better be a paradynd running on this machine!
    for (curr=paradynDaemon::allDaemons, daemon = NULL; *curr; curr++) {
	if ((*curr)->machine.length() &&
	    ((*curr)->machine == machine_name))
	    daemon = *curr;
    }
    // for now, abort if there is no paradynd, this should not happen
    if (!daemon) {
	printf("process started on %s, can't find paradynd there\n",
		machine_name.string_of());
	printf("paradyn error #1 encountered\n");
	exit(-1);
    }
      
   assert (dm->appContext);
   assert (dm->appContext->addRunningProgram(pid, argvString, daemon));
}

void dynRPCUser::newMetricCallback(T_dyninstRPC::metricInfo info)
{
    addMetric(info);
}

void dynRPCUser::firstSampleCallback (int program,
                                      double firstTime) {

  assert(0 && "Invalid virtual function");
}

void paradynDaemon::firstSampleCallback(int program,
					double firstTime) {
  assert(dm);
  setEarliestFirstTime(dm->firstSampleTime(program, firstTime));
}

void dynRPCUser::sampleDataCallbackFunc(int program,
					   int mid,
					   double startTimeStamp,
					   double endTimeStamp,
					   double value)
{
    assert(0 && "Invalid virtual function");
}

paradynDaemon::paradynDaemon(const string m, const string u, const string c,
			     const string n, int f)
: dynRPCUser(m, u, c, NULL, NULL, args, false, wellKnownPortFd),
  machine(m), login(u), command(c), name(n), flavor(f),
  activeMids(uiHash), earliestFirstTime(0)
{
  assert(m.length());
  assert(c.length());
  assert(n.length());

  // if c includes a pathname, lose the pathname
  char *loc = P_strrchr(c.string_of(), '/');
  if (loc) {
    loc = loc + 1;
    command = loc;
  }
  
  status = new status_line(machine.string_of());
  allDaemons.add(this);
}

// machine, name, command, flavor and login are set via a callback
paradynDaemon::paradynDaemon(int f)
: dynRPCUser(f, NULL, NULL, false), flavor(0), activeMids(uiHash),
  earliestFirstTime(0) {
  allDaemons.add(this);
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

    // get the earliest first time that had been reported by any paradyn daemon
    // to use as the base (0) time
    
    assert(getEarliestFirstTime());
    startTimeStamp -= getEarliestFirstTime();
    endTimeStamp -= getEarliestFirstTime();

    // get a fresh version of the TC "printSampleArrival" for use in this routine
    tunableBooleanConstant printSampleArrival = tunableConstantRegistry::findBoolTunableConstant("printSampleArrival");

    if (printSampleArrival.getValue()) {
      cout << "mid " << mid << " " << value << " from " 
	<< startTimeStamp << " to " << endTimeStamp << "\n";
    }
    assert(mid >= 0);
    if (!activeMids.defines((unsigned)mid)) {
      bool found = false;
      for (unsigned ve=0; ve<disabledMids.size(); ve++) {
	if (disabledMids[ve] == (unsigned) mid) {
	  found = true;
	  break;
	}
      }
      if (found) {
	char msg[80];
	// TODO -- data may be in transit when the disable request is made
	sprintf(msg, "ERROR?:data for disabled mid: %d", mid);
	DMstatus->message(msg);
	return;
      } else {
	printf("ERROR: data for unknown mid: %d\n", mid);
	uiMgr->showError (2, "");
	exit(-1);
      }
    }
    mi = activeMids[(unsigned)mid];
    assert(mi);
    if (mi->components.count() != 1) {
	// find the right component.
	part = mi->components.find(this);

	if (!part) {
	  uiMgr->showError(3, "Unable to find component!!!");
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
	mi->data->addInterval(ret.start, ret.end, ret.value, false);
    }
}

//
// paradyn daemon should never go away.  This represents an error state
//    due to a paradynd being killed for some reason.
//
// TODO -- handle this better
paradynDaemon::~paradynDaemon() {

#ifdef notdef
    metricInstance *mi;
    HTable<metricInstance*> curr;

    allDaemons.remove(this);

    // remove the metric ID as required.
    for (curr = activeMids; mi = *curr; curr++) {
	mi->parts.remove(this);
	mi->components.remove(this);
    }
#endif
    printf("Inconsistant state\n");
    abort();
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
dynRPCUser::reportSelf (string m, string p, int pd, int flavor)
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
   dm->appContext->removeDaemon(this, true);
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
// This must set command, name, machine and flavor fields
//
void 
paradynDaemon::reportSelf (string m, string p, int pd, int flav)
{
  setPid(pd);
  flavor = flav;
  if (!m.length() || !p.length()) {
    dm->appContext->removeDaemon(this, true);
    printf("paradyn daemon reported bad info, removed\n");
    // error
  } else {
    machine = strdup(m.string_of());
    command = strdup(p.string_of());
    status = new status_line(machine.string_of());

    switch (flavor) {
    case metPVM:
      name = strdup("pvmd"); 
      break;
    case metCM5:
      name = strdup("cm5d");
      break;
    case metUNIX:
      name = strdup("defd");
      break;
    default:
      dm->appContext->removeDaemon(this, true);
      printf("paradyn daemon reported bad flavor, removed\n");
    }
  }
  return;
}

void 
dynRPCUser::reportStatus (string line)
{
    assert(0 && "Invalid virtual function");
}

//
// When a paradynd reports status, send the status to the user
//
void 
paradynDaemon::reportStatus (string line)
{
  if (status)
    status->message(line.string_of());
}

// 
// establish socket that will be advertised to paradynd's
// this socket will allow paradynd's to connect to paradyn for pvm
//
static void
DMsetupSocket (int &sockfd, int &known_sock)
{
  // setup "well known" socket for pvm paradynd's to connect to
  assert ((known_sock =
	   RPC_setup_socket (sockfd, AF_INET, SOCK_STREAM)) >= 0);

  // this info is needed to create argument list for other paradynds
  dm->socket = known_sock;
  dm->sock_fd = sockfd;

  dm->firstTime = 0;
  // bind fd for this thread
  msg_bind (sockfd, true);
}

static void
DMnewParadynd (int sockfd, dataManager *dm)
{
  int new_fd;

  // accept the connection
  new_fd = RPC_getConnect(sockfd);
  if (new_fd < 0) {
    uiMgr->showError(4, "unable to connect to new paradynd");
  }

  assert (dm->appContext);
  assert (dm->appContext->addDaemon(new_fd));
}


// TEMPORARY until new version of igen
void DMstartPhase(timeStamp start_Time, string *name){

    // update the histogram data structs assoc with each MI
    // return a start time for the phase

    // create a new phaseInfo object and add it to the list of phases
    phaseHandle handle = (phaseHandle)phaseInfo::numPhases;

    string *n;
    if (name){
	n = new string(name->string_of());
    }
    else {
	char s[20];
        n = new string(sprintf(s,"%s%d","phase_",handle));
    }

    phaseInfo *p = NULL;
    timeStamp bin_width = (Histogram::bucketSize);
    timeStamp start_time = bin_width*(Histogram::lastGlobalBin);
    p = new phaseInfo(start_time,
		      (timeStamp)-1.0,
            	      bin_width,
            	      handle,
            	      n);
    
    dm_phases+=p;

    // invoke newPhaseCallback for all perf. streams
    performanceStream *ps;
    for(unsigned i = 0; i < phase_notify_list.size() ; i++){
        ps = phase_notify_list[i]; 
	ps->callPhaseFunc(p);
    }
    p = 0;

}

void DMaddPhaseNotify(performanceStream *p){

   phase_notify_list +=p; 
   p = 0;

}

void DMremovePhaseNotify(performanceStream *p){
   unsigned size = phase_notify_list.size();
   for(unsigned i = 0; i < size; i++){
      if( phase_notify_list[i] == p){
	  phase_notify_list[i] = phase_notify_list[size-1];
          phase_notify_list.resize(size-1); 
	  return;
      }
   }
}



//
// Main loop for the dataManager thread.
//
void *DMmain(void* varg)
{
    // We declare the "printChangeCollection" tunable constant here; it will
    // last for the lifetime of this function, which is pretty much forever.
    // (used to be declared as global in DMappContext.C.  Globally declared
    //  tunables are now a no-no).  Note that the variable name (printCC) is
    // unimportant.   -AT
    tunableBooleanConstantDeclarator printCC("printChangeCollection", 
			   "Print the name of metric/focus when enabled or disabled",
			   false, // initial value
			   NULL, // callback
			   developerConstant);

    // Now the same for "printSampleArrival"
    tunableBooleanConstantDeclarator printSA("printSampleArrival", 
                                             "Print out status lines to show the arrival of samples",
					     false, // initial value
					     NULL, // callback
					     developerConstant);

    // Now for the float TC "samplingRate"
    tunableFloatConstantDeclarator sampR ("samplingRate",
					  "how often to sample intermediate performance data (in seconds)",
					  0.5, // initial value
					  0.0, // min
					  1000.0, // max
					  newSampleRate, // callback
					  userConstant);

    int arg; memcpy((void *) &arg, varg, sizeof arg);

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
    DMsetupSocket (sockfd, known_sock);
    wellKnownPortFd = sockfd;

    assert(RPC_make_arg_list(paradynDaemon::args, AF_INET, SOCK_STREAM,
			     known_sock, 1, 1, "", false));

    msg_send (MAINtid, MSG_TAG_DM_READY, (char *) NULL, 0);
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, DMbuff, &msgSize);

    while (1) {
        for (curr = paradynDaemon::allDaemons; *curr; curr++) {
	  // handle up to max async requests that may have been buffered
	  // while blocking on a sync request
	  while ((*curr)->buffered_requests()) {
	    if ((*curr)->process_buffered() == T_dyninstRPC::error) {
	      // cleanup code here ?
	      cout << "error on paradyn daemon\n";
	      dm->appContext->removeDaemon(*curr, true);
	    }
	  }
	}

	tag = MSG_TAG_ANY;
	ret = msg_poll(&tag, true);
	assert(ret != THR_ERR);

	if (tag == MSG_TAG_FILE) {
	  // must be an upcall on something speaking the dynRPC protocol.
	  if (ret == sockfd)
	    DMnewParadynd(sockfd, dm);        // set up a new paradynDaemon
	  else {
	    for (curr = paradynDaemon::allDaemons; *curr; curr++) {
	      if ((*curr)->get_fd() == ret) {
		if ((*curr)->waitLoop() == T_dyninstRPC::error) {
		  // cleanup code here ?
		  cout << "error on paradyn daemon\n";
		  dm->appContext->removeDaemon(*curr, true);
		}
	      }

	      // handle async requests that may have been buffered
	      // while blocking on a sync request
	      while ((*curr)->buffered_requests()) {
		if ((*curr)->process_buffered() == T_dyninstRPC::error) {
		  // cleanup code here ?
		  cout << "error on paradyn daemon\n";
		  dm->appContext->removeDaemon(*curr, true);
		}
	      }
	    }
	  }
	} else if (dm->isValidTag((T_dataManager::message_tags)tag)) {
	  if (dm->waitLoop(true, (T_dataManager::message_tags)tag) ==
	      T_dataManager::error) {
	    // handle error
	    assert(0);
	  }
	} else {
	  cerr << "Unrecognized message in DMmain.C\n";
	  assert(0);
	}
   }
}


void addMetric(T_dyninstRPC::metricInfo info)
{
    metric *met;
    stringHandle iName;
    performanceStream *stream;
    List<performanceStream *> curr;

    iName = metric::names.findAndAdd(info.name.string_of());
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


resource *createResource(resource *p, const char *newResource, const char *abstr)
{
    resource *ret;
    resource *temp;
    stringHandle fullName;
    performanceStream *stream;
    List<performanceStream *> curr;

    /* first check to see if the resource has already been defined */
    temp = p->children.find(newResource);
    if (temp) return(temp);

    /* then create it */
    ret = new resource(p, strdup(newResource), (abstr ? abstr : "BASE"));
    fullName = ret->getFullName();

    /* inform others about it if they need to know */
    for (curr = applicationContext::streams; stream = *curr; curr++) {
	stream->callResourceFunc(p, ret, fullName);
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

bool daemonEntry::setAll (const string m, const string c, const string n,
			  const string l, const string d, int f)
{
  if (!n.length() || !c.length())
    return false;

  if (m.length()) machine = m;
  if (c.length()) command = c;
  if (n.length()) name = n;
  if (l.length()) login = l;
  if (d.length()) dir = d;
  flavor = f;

  return true;
}
void daemonEntry::print() 
{
  cout << "DAEMON ENTRY\n";
  cout << "  name: " << name << endl;
  cout << "  command: " << command << endl;
  cout << "  dir: " << dir << endl;
  cout << "  login: " << login << endl;
  cout << "  machine: " << machine << endl;
  cout << "  flavor: ";
  switch (flavor) {
  case metPVM:
    cout << " metPVM " << endl;
    break;
  case metUNIX:
    cout << " metUNIX " << endl;
    break;
  case metCM5:
    cout << " metCM5 " << endl;
    break;
  default:
    cout << flavor << " is UNKNOWN!" << endl;
    break;
  }
}

int paradynDaemon::read(const void* handle, char *buf, const int len) {
  assert(0);
  int ret, ready_fd;
  assert(len > 0);
  assert((int)handle<200);
  assert((int)handle >= 0);
  static vector<unsigned> fd_vect(200);

  // must handle the msg_bind_buffered call here because xdr_read will be
  // called in the constructor for paradynDaemon, before the previous call
  // to msg_bind_buffered had been called

  if (!fd_vect[(unsigned)handle]) {
    List<paradynDaemon*> alld;
    for (alld=paradynDaemon::allDaemons; *alld; alld++)
      if ((*alld)->get_fd() == (int)handle)
	break;
    if (!(*alld))
      return -1;
    msg_bind_buffered((int)handle, true, (int(*)(void*))xdrrec_eof,
		      (void*)(*alld)->net_obj());
    fd_vect[(unsigned)handle] = 1;
  }

  do {
    unsigned tag = MSG_TAG_FILE;

    do 
      ready_fd = msg_poll(&tag, true);
    while ((ready_fd != (int) handle) && (ready_fd != THR_ERR));
    
    if (ready_fd == (int) handle) {
      errno = 0;
      ret = P_read((int)handle, buf, len);
    } else 
      return -1;
  } while (ret < 0 && errno == EINTR);

  if (ret <= 0)
    return (-1);
  else
    return ret;
}
