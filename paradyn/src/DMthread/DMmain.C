/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.85  1995/12/28 23:35:26  zhichen
 * Commented out sampleDataCallbackFunc; replaced with batchSampleDataCallbackFunc
 *
 * Revision 1.84  1995/12/18  23:21:24  newhall
 * changed metric units type so that it can have one of 3 values (normalized,
 * unnormalized or sampled)
 *
 * Revision 1.83  1995/12/03  21:31:37  newhall
 * added buffering of data values between DM and client threads based on
 * the number of metric/focus pairs a client thread has enabled
 * DM allocs buffers and the client threads dealloc them
 *
 * Revision 1.82  1995/11/22  00:07:30  mjrg
 * Removed -f and -t arguments to paradyn
 *
 * Revision 1.81  1995/11/17 17:18:02  newhall
 * added normalized member to metric class, support for MDL unitsType option
 *
 * Revision 1.80  1995/11/13  14:48:59  naim
 * Fixing return value for function DM_sequential_init - naim
 *
 * Revision 1.79  1995/11/11  00:45:58  tamches
 * added a callback func for the tunable constant printSampleArrival (DMdaemon.C)
 *
 * Revision 1.78  1995/11/03 00:05:21  newhall
 * second part of sampling rate change
 *
 * Revision 1.77  1995/10/19  22:41:57  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.76  1995/10/13  22:06:38  newhall
 * Added code to change sampling rate as bucket width changes (this is not
 * completely implemented in daemon code yet, so now it has no effect).
 * Purify fixes.  Added phaseType parameter to sampleDataCallbackFunc
 * Added 2 new DM interface routines: getResourceName, getResourceLabelName
 *
 * Revision 1.75  1995/09/26  20:22:11  naim
 * New function defintion: showErrorCallback. This function allows error msgs
 * from paradynd
 *
 * Revision 1.74  1995/08/24  17:23:53  newhall
 * fixed compile error
 *
 * Revision 1.73  1995/08/24  15:02:26  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.72  1995/08/20  03:51:27  newhall
 * *** empty log message ***
 *
 * Revision 1.71  1995/08/20 03:37:07  newhall
 * changed parameters to DM_sequential_init
 * added persistent data and persistent collection flags
 *
 * Revision 1.70  1995/08/18  21:47:46  mjrg
 * uncommented defineDaemon
 *
 * Removed calls to metDoTunable, metDoDaemon, and metDoProcess from
 * DM_post_thread_create_init.
 * Fixed dataManager::defineDaemon.
 *
 * Revision 1.69  1995/08/12  22:28:22  newhall
 * Added DM_post_thread_create_init, DM_sequential_init. Changes to DMmain
 *
 * Revision 1.68  1995/08/11  21:50:31  newhall
 * Removed DM kludge method function.  Added calls to metDoDaemon,
 * metDoProcess and metDoTunable that were moved out of metMain
 *
 * Revision 1.67  1995/08/01  02:11:08  newhall
 * complete implementation of phase interface:
 *   - additions and changes to DM interface functions
 *   - changes to DM classes to support data collection at current or
 *     global phase granularity
 * added alphabetical ordering to foci name creation
 *
 * Revision 1.66  1995/07/06  01:52:49  newhall
 * update for new version of Histogram library, removed compiler warnings
 *
 * Revision 1.65  1995/06/02  20:48:19  newhall
 * * removed all pointers to datamanager class objects from datamanager
 *    interface functions and from client threads, objects are now
 *    refered to by handles or by passing copies of DM internal data
 * * removed applicationContext class from datamanager
 * * replaced List and HTable container classes with STL containers
 * * removed global variables from datamanager
 * * remove redundant lists of class objects from datamanager
 * * some reorginization and clean-up of data manager classes
 * * removed all stringPools and stringHandles
 * * KLUDGE: there are PC friend members of DM classes that should be
 *    removed when the PC is re-written
 *
 * Revision 1.63  1995/03/02  04:23:19  krisna
 * warning and bug fixes.
 *
 * Revision 1.62  1995/03/01  00:12:27  newhall
 * added static members to phaseInfo class
 *
 * Revision 1.61  1995/02/27  18:43:03  tamches
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
double   quiet_nan();
#include <malloc.h>
#include <stdio.h>
}

#include "thread/h/thread.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMdaemon.h"
#include "DMmetric.h"
#include "DMperfstream.h"
#include "DMabstractions.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "paradyn/src/UIthread/Status.h"

#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "DMphase.h"

// bool parse_metrics(string metric_file);
bool metMain(string &userFile);

// this has to be declared before baseAbstr, cmfAbstr, and rootResource 
int dataManager::sock_fd;  
int dataManager::socket;  
dataManager *dataManager::dm = NULL;  

dictionary_hash<string,abstraction*> abstraction::allAbstractions(string::hash);
abstraction *baseAbstr = new abstraction("BASE");
abstraction *cmfAbstr = new abstraction("CMF");

dictionary_hash<string,metric*> metric::allMetrics(string::hash);
dictionary_hash<metricInstanceHandle,metricInstance *> 
		metricInstance::allMetricInstances(metricInstance::mhash);
dictionary_hash<perfStreamHandle,performanceStream*>  
		performanceStream::allStreams(performanceStream::pshash);
dictionary_hash<string, resource*> resource::allResources(string::hash);
dictionary_hash<string,resourceList *> resourceList::allFoci(string::hash);

vector<resource*> resource::resources;
vector<metric*> metric::metrics;
vector<paradynDaemon*> paradynDaemon::allDaemons;
vector<daemonEntry*> paradynDaemon::allEntries;
vector<executable*> paradynDaemon::programs;
unsigned paradynDaemon::procRunning;
vector<resourceList *> resourceList::foci;
vector<phaseInfo *> phaseInfo::dm_phases;
vector<bool> metricInstance::nextId;
vector<bool> performanceStream::nextId;

resource *resource::rootResource = new resource();
timeStamp metricInstance::curr_bucket_width;
timeStamp metricInstance::global_bucket_width;
phaseHandle metricInstance::curr_phase_id;
u_int metricInstance::num_curr_hists = 0;
u_int metricInstance::num_global_hists = 0;

double paradynDaemon::earliestFirstTime = 0;

// TODO: remove
// void DMchangeSampleRate(float rate); 
void newSampleRate(float rate);

//
// IO from application processes.
//
void dynRPCUser::applicationIO(int pid, int len, string data)
{

    // NOTE: this fixes a purify error with the commented out code (a memory
    // segment error occurs occasionally with the line "cout << rest << endl") 
    // this is problably not the best fix,  but I can't figure out why 
    // the error is occuring (rest is always '\0' terminated when this
    // error occurs)---tn 
    fprintf(stdout,data.string_of());

#ifdef n_def
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
	    printf("paradynd: ");
	}
	if (extra.length()) {
	    cout << extra;
	    extra = (char*) NULL;
	}
	cout << rest << endl;
	rest = ptr+1;
	if(rest)
	    ptr = P_strchr(rest, '\n');
        else
	    ptr = 0;
    }
    extra += rest;
    delete tp;
    rest = 0;
#endif

}

extern status_line *DMstatus;

void dynRPCUser::resourceBatchMode(bool onNow)
{
    static int count=0;

    int prev = count;
    if (onNow) {
	count++;
    } else {
	count--;
    }

    if (count == 0) {
	performanceStream::ResourceBatchMode(batchEnd);
    } else if (!prev) {
	performanceStream::ResourceBatchMode(batchStart);
    }
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(int program,
				      vector<string> resource_name,
				      string abstr) {
    resourceHandle r = createResource(resource_name, abstr);
    resourceInfoResponse(resource_name, r);
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
    ptr = dataManager::names.findAndAdd(newName);

    ret = allUniqueNames.find(ptr);

    if (!ret) {
	ret = new uniqueName(ptr);
	allUniqueNames.add(ret, ptr);
    }
    // changed from [] to {} due to TCL braindeadness.
    sprintf(newName, "%s{%d}", newResource.string_of(), ret->nextId++);
    ptr = dataManager::names.findAndAdd(newName);

    return((char*)ptr);
}
#endif

//
// Display errors using showError function from the UIM class
// This function allows to display error messages from paradynd
// using the "upcall" or "call back" mechanism.
// Parameters: 	errCode = Error code
//		errString = Error message
//		hostName = Host name where the error occur
// Call: there is a macro defined in "showerror.h". This macro must be
//       used when calling this function. A typical call is:
//       showErrorCallback(99, "Erro message test"). This macro will
//       automatically insert the additional host info required.
//
void dynRPCUser::showErrorCallback(int errCode, 
				   string errString,
				   string hostName)
{
    string msg;

    if (errString.length() > 0) {
	if (hostName.length() > 0) {
    	    msg = string("<Msg from daemon on host ") + hostName + 
	          string("> ") + errString;
        }
	else { 
	    msg = string("<Msg from daemon on host ?> ") + errString; 
        }
        uiMgr->showError(errCode, P_strdup(msg.string_of()));
    }
    else {
        uiMgr->showError(errCode, ""); 
    }

    //
    // hostName.length() should always be > 0, otherwise
    // hostName is not defined (i.e. "?" will be used instead).
    // if errString.length()==0, (i.e. errString.string_of()==""),
    // then we will use the default error message in errorList.tcl
    // This message, however, will not include any info about the current
    // host name.
    //
}

//
// used when a new program gets forked.
//
void dynRPCUser::newProgramCallbackFunc(int pid,
					vector<string> argvString,
					string machine_name)
{
    // there better be a paradynd running on this machine!
    paradynDaemon *pd, *daemon = NULL;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	if(pd->machine.length() && (pd->machine == machine_name)){
	    daemon = pd;
	}
    }
    // for now, abort if there is no paradynd, this should not happen
    if (!daemon) {
	printf("process started on %s, can't find paradynd there\n",
		machine_name.string_of());
	printf("paradyn error #1 encountered\n");
	exit(-1);
    }
      
   assert (paradynDaemon::addRunningProgram(pid, argvString, daemon));
}

void dynRPCUser::newMetricCallback(T_dyninstRPC::metricInfo info)
{
    addMetric(info);
}

void dynRPCUser::firstSampleCallback (int program,
                                      double firstTime) {

  assert(0 && "Invalid virtual function");
}

void dynRPCUser::cpDataCallbackFunc(int program,
                                         double timeStamp,
                                         int context,
                                         double total,
                                         double share)
{
    assert(0 && "Invalid virtual function");
}

//void dynRPCUser::sampleDataCallbackFunc(int program,
//					   int mid,
//					   double startTimeStamp,
//					   double endTimeStamp,
//					   double value)
//{
//    assert(0 && "Invalid virtual function");
//}

// batch the sample delivery
void dynRPCUser::batchSampleDataCallbackFunc(int program,
					    vector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer)
{
    assert(0 && "Invalid virtual function");
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
dynRPCUser::reportSelf (string m, string p, int pd, string flavor)
{
  assert(0);
  return;
}

void 
dynRPCUser::reportStatus (string line)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::processStatus(int pid, u_int stat)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::nodeDaemonReadyCallback(void)
{
    assert(0 && "Invalid virtual function");
}


// 
// establish socket that will be advertised to paradynd's
// this socket will allow paradynd's to connect to paradyn for pvm
//
static void
DMsetupSocket (int &sockfd)
{
  // setup "well known" socket for pvm paradynd's to connect to
  assert ((dataManager::dm->socket =
       RPC_setup_socket (sockfd, AF_INET, SOCK_STREAM)) >= 0);

  // bind fd for this thread
  msg_bind (sockfd, true);
}

static void
DMnewParadynd ()
{
  // accept the connection
  int new_fd = RPC_getConnect(dataManager::dm->sock_fd);
  if (new_fd < 0) {
    uiMgr->showError(4, "unable to connect to new paradynd");
  }
  // add new daemon to dictionary of all deamons
  paradynDaemon::addDaemon(new_fd); 
}

void DM_eFunction(int errno, char *message)
{
    printf("error: %s\b", message);
    abort();
}


bool dataManager::DM_sequential_init(const char* met_file){
   string mfile = met_file;
   return(metMain(mfile)); 
}

int dataManager::DM_post_thread_create_init(int tid) {


    thr_name("Data Manager");
    dataManager::dm = new dataManager(tid);

    // supports argv passed to paradynDaemon
    // new paradynd's may try to connect to well known port
    DMsetupSocket (dataManager::dm->sock_fd);

    assert(RPC_make_arg_list(paradynDaemon::args,
  	 		     dataManager::dm->socket, 1, 1, "", false));

    // start initial phase
    string dm_phase0 = "phase_0";
    phaseInfo::startPhase(0.0,dm_phase0);

    char DMbuff[64];
    unsigned int msgSize = 64;
    msg_send (MAINtid, MSG_TAG_DM_READY, (char *) NULL, 0);
    unsigned int tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, DMbuff, &msgSize);
    return 1;
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
    extern bool our_print_sample_arrival;
    our_print_sample_arrival = false;
    extern void printSampleArrivalCallback(bool);
    tunableBooleanConstantDeclarator printSA("printSampleArrival", 
              "Print out status lines to show the arrival of samples",
	      our_print_sample_arrival, // init val
	      printSampleArrivalCallback,
	      developerConstant);

    int tid; memcpy((void*)&tid,varg, sizeof(int));
    dataManager::DM_post_thread_create_init(tid);

    int ret;
    unsigned int tag;
    paradynDaemon *pd = NULL;
    while (1) {
        for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
	    pd = paradynDaemon::allDaemons[i]; 
	    // handle up to max async requests that may have been buffered
	    // while blocking on a sync request
	    while (pd->buffered_requests()){
	        if(pd->process_buffered() == T_dyninstRPC::error) {
		    cout << "error on paradyn daemon\n";
		    paradynDaemon::removeDaemon(pd, true);
	} } }

	tag = MSG_TAG_ANY;
	ret = msg_poll(&tag, true);
	assert(ret != THR_ERR);

	if (tag == MSG_TAG_FILE) {
	    // must be an upcall on something speaking the dynRPC protocol.
	    if (ret == dataManager::dm->sock_fd){
	        DMnewParadynd(); // set up a new daemon
            }
	    else {
                for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
	            pd = paradynDaemon::allDaemons[i]; 
		    if(pd->get_fd() == ret){
		        if(pd->waitLoop() == T_dyninstRPC::error) {
			    cout << "error on paradyn daemon\n";
			    paradynDaemon::removeDaemon(pd, true);
	            }}

	            // handle async requests that may have been buffered
	            // while blocking on a sync request
                    while(pd->buffered_requests()){
		        if(pd->process_buffered() == T_dyninstRPC::error) {
			    cout << "error on paradyn daemon\n";
			    paradynDaemon::removeDaemon(pd, true);
		    }}
	        }
	    }
	} else if (dataManager::dm->isValidTag
		  ((T_dataManager::message_tags)tag)) {
	    if (dataManager::dm->waitLoop(true, 
	       (T_dataManager::message_tags)tag) == T_dataManager::error) {
	        // handle error
	        assert(0);
	    }
	} else {
	    cerr << "Unrecognized message in DMmain.C\n";
	    assert(0);
	}
   }
}


void addMetric(T_dyninstRPC::metricInfo &info)
{
    // if metric already exists return
    if(metric::allMetrics.defines(info.name)){
        return;
    }
    metric *met = new metric(info);

    // now tell all perfStreams
    dictionary_hash_iter<perfStreamHandle,performanceStream*> 
		allS(performanceStream::allStreams);
    perfStreamHandle h;
    performanceStream *ps;
    while(allS.next(h,ps)){
	if(ps->controlFunc.mFunc){
	    // set the correct destination thread.
	    dataManager::dm->setTid(ps->threadId);
	    dataManager::dm->newMetricDefined(ps->controlFunc.mFunc, 
					      ps->Handle(),
					      met->getName(),
					      met->getStyle(),
					      met->getAggregate(),
					      met->getUnits(),
					      met->getHandle(),
					      met->getUnitsType());
	}
    }
}


#ifdef n_def
resourceHandle createResource(resourceHandle parent, vector<string>& res_name,
			 string& res_string,string& abstr)
{
    /* first check to see if the resource has already been defined */
    resource *p = resource::resources[parent];
    resourceHandle *child = p->findChild(res_string.string_of());
    if (child){
        return(*child); 
	delete child;
    }

    // if abstr is not defined then use default abstraction 
    if(!abstr.string_of()){
        abstr = string("BASE");
    }

    /* then create it */
    resource *ret =  new resource(parent,res_name,res_string,abstr);

    /* inform others about it if they need to know */
    dictionary_hash_iter<perfStreamHandle,performanceStream*> 
			allS(performanceStream::allStreams);
    perfStreamHandle h;
    performanceStream *ps;
    resourceHandle r_handle = ret->getHandle();
    string name = ret->getFullName(); 
    while(allS.next(h,ps)){
	ps->callResourceFunc(parent,r_handle,name,abstr);
    }
    return(r_handle);
}
#endif

// I don't want to parse for '/' more than once, thus the use of a string vector
resourceHandle createResource(vector<string>& resource_name, string& abstr) {
  resource *parent = NULL;
  unsigned r_size = resource_name.size();
  string p_name;


  switch (r_size) {
    case 0:
        // Should this case ever occur ?
        assert(0); break;
    case 1:
        parent = resource::rootResource; break;
    default:
        for (unsigned ri=0; ri<(r_size-1); ri++) 
            p_name += string("/") + resource_name[ri];
        parent = resource::string_to_resource(p_name);
        assert(parent);
        break;
    }
    if (!parent) assert(0);

    /* first check to see if the resource has already been defined */
    resource *p = resource::resources[parent->getHandle()];
    string myName = p_name;
    myName += "/";
    myName += resource_name[r_size - 1];
    resourceHandle *child = p->findChild(myName.string_of());
    if (child){
        return(*child); 
        delete child;
    }

    // if abstr is not defined then use default abstraction 
    if(!abstr.string_of()){
        abstr = string("BASE");
    }

    /* then create it */
    resource *ret =  new resource(parent->getHandle(),resource_name,
				  myName,abstr);

    /* inform others about it if they need to know */
    dictionary_hash_iter<perfStreamHandle,performanceStream*> 
			allS(performanceStream::allStreams);
    perfStreamHandle h;
    performanceStream *ps;
    resourceHandle r_handle = ret->getHandle();
    string name = ret->getFullName(); 
    while(allS.next(h,ps)){
	ps->callResourceFunc(parent->getHandle(),r_handle,ret->getFullName(),
	ret->getAbstractionName());
    }
    return(r_handle);
}

void newSampleRate(float rate)
{
    paradynDaemon *pd = NULL;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i]; 
	pd->setSampleRate(rate);
    }
}

#ifdef ndef
// Note - the metric parser has been moved into the dataManager
bool parse_metrics(string metric_file) {
     bool parseResult = metMain(metric_file);
    return parseResult;
}
#endif


