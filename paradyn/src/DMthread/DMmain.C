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
u_int metricInstance::next_id = 0;
u_int performanceStream::next_id = 0;

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
void dynRPCUser::applicationIO(int,int,string data)
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
void dynRPCUser::resourceInfoCallback(int,
				      vector<string> resource_name,
				      string abstr, u_int type) {
    resourceHandle r = createResource(resource_name, abstr, type);
    resourceInfoResponse(resource_name, r);
}

void dynRPCUser::mappingInfoCallback(int,
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

//
// Upcall to tell paradyn that all daemons are ready after a metric
// enable request
//
void dynRPCUser::enableDataCallback(int, int)
{
}

void dynRPCUser::enableDataCallbackBatch(int daemon_id, vector<int> return_id)
{
}

//
// Upcall from daemon in response to getPredictedDataCost call
// id - perfStreamHandle assoc. with the call
// req_id - an identifier assoc. with the request 
// val - the cost of enabling the metric/focus pair
//
void dynRPCUser::getPredictedDataCostCallback(u_int id,
					      u_int req_id,
					      float val)
{
    // find the assoc. perfStream and update it's pred data cost value
    dictionary_hash_iter<perfStreamHandle,performanceStream*> 
		allS(performanceStream::allStreams);
    perfStreamHandle h; performanceStream *ps;
    while(allS.next(h,ps)){
	if(h == (perfStreamHandle)id){
            ps->predictedDataCostCallback(req_id,val);
	    return;
    } }
    // TODO: call correct routine
    assert(0);
}

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

void dynRPCUser::firstSampleCallback (int,double) {

  assert(0 && "Invalid virtual function");
}

void dynRPCUser::cpDataCallbackFunc(int,double,int,double,double)
{
    assert(0 && "Invalid virtual function");
}

// batch the sample delivery
void dynRPCUser::batchSampleDataCallbackFunc(int,
		    vector<T_dyninstRPC::batch_buffer_entry>)
{
    assert(0 && "Invalid virtual function");
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
void 
dynRPCUser::reportSelf (string , string , int , string)
{
  assert(0);
  return;
}

void 
dynRPCUser::reportStatus (string)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::processStatus(int, u_int)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::nodeDaemonReadyCallback(void)
{
    assert(0 && "Invalid virtual function");
}

void
dynRPCUser::endOfDataCollection(int)
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
  if (new_fd < 0)
    uiMgr->showError(4, "");

  // add new daemon to dictionary of all deamons
  paradynDaemon::addDaemon(new_fd); 
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
    phaseInfo::startPhase(0.0,dm_phase0,false,false);

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
    unsigned fd_first = 0;
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
	// ret = msg_poll(&tag, true);
	ret = msg_poll_preference(&tag, true,fd_first);
	fd_first = !fd_first;
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
resourceHandle createResource(vector<string>& resource_name, string& abstr, unsigned type) {
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
				  myName,abstr, type);

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


