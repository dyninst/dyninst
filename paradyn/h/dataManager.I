/*
 * dataManager.h - exported services of the Data Mgr thread of paradyn.
 *
 * $Log: dataManager.I,v $
 * Revision 1.46  1995/08/18 21:40:49  mjrg
 * uncommented defineDaemon
 *
 * Revision 1.45  1995/08/12  22:26:47  newhall
 * added DM_sequential_init() & DM_post_thread_create_init() members
 * moved struct init_struct from paradyn.h to dataManager.I
 *
 * Revision 1.44  1995/08/11  21:50:03  newhall
 * removed kludge()
 *
 * Revision 1.43  1995/08/08  03:10:35  newhall
 * changed newPerfData and sampleDataCallbackFunc definitions
 *
 * Revision 1.42  1995/08/05  17:07:40  krisna
 * `res' is used as a variable name in igen-generated code, YUCK
 *
 * Revision 1.41  1995/08/01 02:17:02  newhall
 * *** empty log message ***
 *
 * Revision 1.40  1995/08/01  02:15:29  newhall
 * changes to support phase interface  
 *
 * Revision 1.39  1995/06/02  20:57:40  newhall
 * * removed all pointers to datamanager class objects from datamanager
 *    interface functions and from client threads, objects are now
 *    refered to by handles or by passing copies of DM internal data
 * * removed applicationContext class from datamanager
 * * replaced List and HTable container classes with STL containers
 * * removed global variables from datamanager
 * * remove redundant lists of class objects from datamanager
 * * removed all stringPools and stringHandles
 *
 * Revision 1.37  1995/02/26  02:11:53  newhall
 * added some phase interface routines
 *
 * Revision 1.36  1995/02/16  08:01:41  markc
 * Modified interfaces:
 *   1) XDR interfaces use strings and vectors instead of igen arrays and char*'s
 *   2) No sync upcalls are allowed by the server in xdr interfaces
 *   3) Redefined igen arrays that were used in thread code to use manually defined
 *      structures rather than igen-defined, since igen did not need to know about
 *      these structures.
 *   4) Upped all of the version numbers since all new interface code is generated
 *   5) Async calls are no longer handled while waiting for sync responses, these
 *      requests are buffered.
 *
 * Revision 1.35  1995/01/26  17:59:30  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.34  1994/11/09  18:38:33  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.33  1994/11/07  14:57:44  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.32  1994/11/04  16:28:24  rbi
 * added getAvailableDaemons()
 *
 * Revision 1.31  1994/09/30  19:16:49  rbi
 * Abstraction interface change.
 *
 * Revision 1.30  1994/09/22  00:43:30  markc
 * Added "const" for char* args to addExecutable()
 * Made String_Array declaration explicit
 * Incremented version number
 *
 * Revision 1.29  1994/08/22  16:01:21  markc
 * Remove my_pid which duplicated a class member
 * Add daemon dictionary calls to dataManager interface.
 *
 * Revision 1.28  1994/08/17  17:55:26  markc
 * Incremented version numbers because a new version of igen has been
 * committed.
 *
 * Revision 1.27  1994/08/11  02:22:06  newhall
 * increment version number
 *
 * Revision 1.26  1994/08/11  02:18:05  newhall
 * added dataManager interface routine destroyPerformanceStream
 *
 * Revision 1.25  1994/08/08  20:14:54  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.24  1994/08/05  16:03:46  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.23  1994/07/25  14:56:19  hollings
 * added suppress resource option.
 *
 * Revision 1.22  1994/07/14  23:44:28  hollings
 * added getCurrentHybridCost
 *
 * Revision 1.21  1994/07/07  03:22:28  markc
 * Added addDaemon function to start daemon prior to start process.
 *
 * Revision 1.20  1994/07/02  01:45:22  markc
 * Removed aggregation operator from enableDataCollection call.
 * Remove aggregation operator definitions, which are provided by
 * util/h/aggregation.H
 *
 * Revision 1.19  1994/06/28  00:15:45  rbi
 * Increment of version numbers
 *
 * Revision 1.18  1994/06/27  21:21:39  rbi
 * Abstraction-specific resources and mapping information
 *
 * Revision 1.17  1994/06/17  22:08:11  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.16  1994/06/14  15:17:25  markc
 * Added new argument (aggregationOperator) to enableDataCollection.  This
 * tells the data manager how to aggregate a metric (sum, max, min, avg).
 *
 * Revision 1.15  1994/05/31  19:07:46  hollings
 * Changes to make access to resources and resourceLists direct not via
 * the dataManager thread.
 *
 * Revision 1.14  1994/05/18  00:49:46  hollings
 * Bad Jeff didn't advance the version number on the last commit.
 *
 * Revision 1.13  1994/05/10  03:57:28  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.12  1994/05/09  20:53:35  hollings
 * added changeState to dataManager, and clearSHG to performanceConsultant.
 *
 * Revision 1.11  1994/04/21  23:40:28  hollings
 * Remoced setParadynd name function.
 *
 * Revision 1.10  1994/04/20  15:30:58  hollings
 * added data manager function to get histogram buckets.
 *
 * Revision 1.9  1994/04/19  22:08:06  rbi
 * Added GetTotValue method to get non-normalized metric data.
 *
 * Revision 1.8  1994/04/01  20:44:59  hollings
 * Added calls to query bucketWidth and max number of bins.
 *
 * Revision 1.7  1994/03/31  01:37:21  markc
 * added pause/continue Process.
 *
 * Revision 1.6  1994/03/20  01:48:51  markc
 * Added support for paradyn daemon name.  igen function to store name.
 *
 * Revision 1.5  1994/03/08  17:39:23  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.4  1994/02/24  04:34:14  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 *
 * Revision 1.3  1994/02/03  23:26:50  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.2  1994/02/01  18:47:33  hollings
 * Upgrades for performanceConsultant thread.
 *
 * Revision 1.1  1994/01/27  00:41:37  hollings
 * First version of data manager interfaces.
 *
 */

#ifndef MMTHREAD_H
#define MMTHREAD_H

#include "util/h/list.h"
#include "util/h/sys.h"
#include "dyninstRPC.xdr.h"
#include "visi.xdr.h"
#include "util/h/aggregation.h"
#include "util/h/stringPool.h"
#include "paradyn/src/DMthread/DMinclude.h"

typedef enum { Trace, Sample } dataType;

/*
 * error handler call back.
 *
 */
typedef void (*errorHandler)(int errno, char *message);

/*
 * Handler that gets called when new sample data is delivered.
 *
 *   handle - a performance stream handle from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   buckets - array of sampleValue filled by the DM 
 *   first - the first bucket value
 *   count - number of bucket values in buckets
 *    
 */
typedef void (*sampleDataCallbackFunc)(perfStreamHandle handle, 
				       metricInstanceHandle mi,
				       int bucketNum,
				       sampleValue value);

/*
 * Handler that gets called when new trace data is available.
 *   
 *   handle - a performance stream handle from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   time - time of the event.
 *   eventSize - size of the event specific data.
 *   eventData - event specific data.
 */
typedef void (*traceDataCallbackFunc)(perfStreamHandle handle, 
				      metricInstanceHandle mi, 
				      timeStamp time, 
				      int eventSize, 
				      void *eventData);

/*
 * union to hold two types of data callback.
 *
 */
union dataCallback {
    sampleDataCallbackFunc 	sample;
    traceDataCallbackFunc	trace;
};

typedef union dataCallback dataCallback;

/*
 * Handler that gets called when a new metric is defined.
 *
 * handle - a performance stream handle from createPerformanceStream
 * metricInfo - info about metric, client is resp. for deleting this struct
 *
 */
typedef void (*metricInfoCallback)(perfStreamHandle, 
				   const char *name,
				   int style,
				   int aggregate,
				   const char *units,
				   metricHandle handle);

/*
 * Handler that gets called when a new resource is defined.
 *
 * handle - a performance stream handle from createPerformanceStream
 * parent		- parent of new resource
 * newResource		- new resource being created
 * name			- name of the new resource, client must delete
 *
 */
typedef void (*resourceInfoCallback)(perfStreamHandle handle, 
				     resourceHandle parent, 
				     resourceHandle newResource, 
				     const char *name,
				     const char *abstraction);

typedef enum batchMode { batchStart, batchEnd };

typedef void (*resourceBatchModeCallback)(perfStreamHandle, batchMode mode); 

typedef void (*histFoldCallback)(perfStreamHandle, timeStamp width, phaseType);

typedef enum appState { appRunning, appPaused };

typedef void (*appStateChangeCallback)(perfStreamHandle, appState state);

typedef void (*newPhaseCallback)(perfStreamHandle handle, 
				 const char *name, 
				 phaseHandle phase,
				 timeStamp begin, 
				 timeStamp end, 
				 float bucketwidth); 

struct controlCallback {
    metricInfoCallback 		mFunc;
    resourceInfoCallback 	rFunc;
    resourceBatchModeCallback	bFunc;
    histFoldCallback		fFunc;
    appStateChangeCallback	sFunc;
    newPhaseCallback	        pFunc;
};

typedef struct controlCallback controlCallback;

// Structure used to pass initial arguments to data manager
struct init_struct{
  int tid;
  char *met_file;
}; 

typedef struct init_struct init_struct;



$remote dataManager {
    $base 2000;
    $version 23;

    //
    // Start a paradyn daemon
    //
    bool addDaemon (const char *machine,
		    const char *login,
		    const char *name);

    // 
    // Define a daemon in the daemon dictionary
    //     command = command line to exec
    //     dir = directory to use as working directory
    //     login = user name to execute as
    //     name = name of dictionary entry, for matching
    //     flavor = programming environment (metPVM, metCM5, metUNIX)
    //     machine = machine to run this daemon on
    //
    //     Note - arguments that are not used should be passed in
    //            as null, anything else will be assumed to be a
    //            valid string.
     bool defineDaemon (const char *command,
     	                const char *dir,
     	                const char *login,
                        const char *name,
     	                const char *machine,
                        const char *flavor);

    //
    // Define a program to run. 
    //     machine = name of machine to run on
    //     login = user to run as
    //     name = name of daemon to run with, matches dictionary entry
    //     dir = working directory to use
    //
    //     Note - arguments that are not used should be passed in
    //            as null, anything else will be assumed to be a
    //            valid string.
    bool addExecutable(const char *machine, 
		       const char *login, 
		       const char *name, 
	               const char *dir,
		       const vector<string> *argv);

    //
    // Find out if an application has been defined yet.
    //
    bool applicationDefined();

    //
    // Start an application running (This starts the actual execution).
    // 
    bool startApplication();

    //
    //   Stop all processes associted with the application.
    //
    // Pause an application (I am not sure about this but I think we want it).
    //      - Does this force buffered data to be delivered?
    //	- Does a paused application respond to enable/disable commands?
    //
    bool pauseApplication();

    //
    // Pause a process in an application 
    //      - Does this force buffered data to be delivered?
    //	- Does a paused application respond to enable/disable commands?
    //
    bool pauseProcess(int pid);

    //
    // Continue a paused application.
    //
    bool continueApplication();

    //
    // Continue a paused process in an application.
    //
    bool continueProcess(int pid);

    //
    // Disconnect the tool from the process.
    //    pause - leave the process in a stopped state.
    //
    bool detachApplication(bool pause);

    // 
    // Create a performanceStream.  This a flow of performance information
    //   to a destination. 
    //
    // dataType 	- Sample or Trace
    // dataCallback 	- Function to call when new data arrives
    // controlCallback	- Function to call when new structure info arrives
    //
    perfStreamHandle createPerformanceStream(dataType, 
					     dataCallback, 
					     controlCallback);

    // 
    // Destroy a performance Stream.  
    // 
    // perfStreamHandle   - a handle returned by createPerformanceStream
    //
    int  destroyPerformanceStream(perfStreamHandle);

    // TODO: this should go:  use a tunable const to get initial rate
    //       and use phase mechanism as way to change rate after that 
    //
    void setSampleRate(perfStreamHandle handle, timeStamp sampleInterval);


    //
    // Routines to control data collection on a performance Stream.
    //
    // perfStreamHandle	- a handle returned by createPerformanceStream
    // resourceList	- a focus : DM deletes this 
    // metric		- what metric to collect data for
    // metricInstInfo   - info. about enabled metric/focus
    //                    client must delete this struct
    // phaseType        - enable collection for either the current phase
    //                    or for global data
    // pers_data        - if set, archive old data
    // pers_collection  - if set, keep MI enabled on new phase definition  
    //
    metricInstInfo *enableDataCollection(perfStreamHandle,
					 const vector<resourceHandle>*,
					 metricHandle,
					 phaseType,
					 unsigned persistent_data,
					 unsigned persistent_collection);
    //
    // stop collecting data for the named metricInstance.
    // perfStreamHandle	- a handle returned by createPerformanceStream
    // metricInstance	- a metricInstance returned by enableDataCollection.
    // phaseType - specifies either global or current phase data
    //
    void disableDataCollection(perfStreamHandle, 
			       metricInstanceHandle,
			       phaseType);

    //
    // change the persistent_collection flag value for the assoc. MI
    //
    // $async void setPersistentCollection(metricInstanceHandle);
    // $async void clearPersistentCollection(metricInstanceHandle);

    //
    // change the persistent_data flag value for the assoc. MI
    //
    // $async void setPersistentData(metricInstanceHandle);
    // $async void clearPersistentData(metricInstanceHandle);

    //
    // Return the expected cost of collecting performance data for a single
    //    metric at a given focus.  The value returned is the fraction of
    //    perturbation expected (i.e. 0.10 == 10% slow down expected).
    //
    float getPredictedDataCost(resourceListHandle, metricHandle);

    //
    // Current best guess of the cost of inst in the system.
    //
    float getCurrentHybridCost();

    // 
    // Histogram/timebase information
    //
    timeStamp getCurrentBucketWidth();

    // 
    // Histogram/timebase information
    //
    timeStamp getGlobalBucketWidth();

    //
    // phase info
    //
    u_int getCurrentPhaseId();
    timeStamp getCurrentStartTime();

    int getMaxBins();

    //
    // Resource utility functions.
    //

    //
    // returns handle for root resource, returns NULL on error
    //
    resourceHandle *getRootResource();

    //
    // returns handles of root resources, returns NULL on error
    //
    vector<resourceHandle> *getRootResources();

    //
    // create a new resource and return it's handle
    //
    resourceHandle newResource(resourceHandle parent, const char *name);

    //
    // converts from a vector of resourceHandles to a resourceListHandle
    // 
    resourceListHandle getResourceList(const vector<resourceHandle>*);

    //
    // converts from a resourceListHandle to a vector of resourceHandles
    // client is responsible for deleting vector<resourceHandle>*
    // 
    vector<resourceHandle> *getResourceHandles(resourceListHandle);

    //
    // converts from a resource name to a resourceHandle 
    // returns NULL if convertion fails 
    //
    resourceHandle *findResource(const char *name);

    //
    // returns name of resource
    //
    string *getResourceName(resourceHandle);

    //
    // converts from a focus name to a resourceListHandle 
    // returns NULL if convertion fails 
    //
    resourceListHandle *findResourceList(const char *name);

    //
    // don't search the SHG for this node.
    //
    void setResourceSearchSuppress(resourceHandle res_handle, bool newValue);

    //
    // don't search the SHG for children of this node.
    //
    void setResourceSearchChildrenSuppress(resourceHandle res_handle, bool newValue);

    //
    // consider this resource a black box (applies to modules/procedures now).
    //
    void setResourceInstSuppress(resourceHandle res_handle, bool newValue);

    // 
    // returns a vector of metric names 
    // client is responsible for deleting these
    //
    vector<string> *getAvailableMetrics();

    //
    // returns a vector of metric name and id pairs
    // client is responsible for deleting these
    //
    vector<met_name_id> *getAvailableMetInfo();

    //
    // looks for a specifc metric instance
    // returns NULL if search is unsuccessful
    // 
    metricHandle *findMetric(const char *name);

    //
    // Metric utility functions.
    //
    string *getMetricName(metricHandle);

    //
    // get the supporting info about a metric.
    //
    T_dyninstRPC::metricInfo *getMetricInfo(metricHandle);

    // TODO: these shouldn't be necessary
    // Get metric out of a metric instance.
    //
    metricHandle *getMetric(metricInstanceHandle);
    string *getMetricNameFromMI(metricInstanceHandle);

    //
    // Get the value of a metric.
    //
    sampleValue getMetricValue(metricInstanceHandle);

    //
    // fill the passed array of buckets with the histogram sample from the
    //   passed metricInstance.  
    //		mi		metric instance to use
    //		buckets		array of buckets to fill
    //    	numberOfBuckets max number of buckets to fill
    //		first		first bin to fill (i.e. what bin goes into
    //				buckets[0]
    //          phase           specifies either curr. or global phase data
    //
    int getSampleValues(metricInstanceHandle mi, 
			sampleValue *buckets,
			int numberOfBuckets,
			int first,
			phaseType phase);

    //
    // fill the passed array of buckets with the archived histogram values
    // of the passed metricInstance
    //      mi        which metric instance
    //      phase_id  which phase
    //
    int getArchiveValues(metricInstanceHandle mi, 
			sampleValue *buckets,
			int numberOfBuckets,
			int first,
			phaseHandle phase_id);

    //
    // Get the total value of a metric. 
    //  (ie. do not divide by time enabled)
    //
    sampleValue getTotValue(metricInstanceHandle);


    //
    //  start_Time: would allow a phase to be specified for some time
    //              in the future, this would be useful for using phase
    //              information from a config. file
    //              a default value of -1.0 would be equivalent to now
    //  name: would be an optional user defined identifier for the phase
    //        each phase would still have a system defined unique id
    //
    $async void StartPhase(timeStamp start_Time, const char *name);


    //
    // returns a list of all currently defined phases in the system
    // the caller is responsible for freeing the space associated with
    // the list
    //
    vector<T_visi::phase_info> *getAllPhaseInfo();


    //
    // Debugger style calls.
    //

    void printResources();
    void printStatus();
    void coreProcess(int pid);

    //
    // probably temporary call, show defined daemons
    //
    void printDaemons();

    //
    // return a list of daemon names
    //
    vector<string> *getAvailableDaemons();

    //
    // upcalls for indications of events.
    //
    $upcall $async void newMetricDefined(metricInfoCallback, 
					 perfStreamHandle, 
				         const char *name,
				         int style,
				         int aggregate,
				         const char *units,
				         metricHandle handle);

    $upcall $async void newResourceDefined(resourceInfoCallback,
					   perfStreamHandle, 
					   resourceHandle parent,
					   resourceHandle newResource, 
					   const char *name,
					   const char *abstr);

    $upcall $async void changeResourceBatchMode(resourceBatchModeCallback,
					        perfStreamHandle, 
						batchMode mode);

    $upcall $async void newPerfData(sampleDataCallbackFunc, 
				    perfStreamHandle,
				    metricInstanceHandle mi,
				    int bucketNum,
				    sampleValue value);

//    $upcall $async void newPerfData(sampleDataCallbackFunc, 
//				    perfStreamHandle,
//				    metricInstanceHandle mi,
//				    sampleValue *buckets,
//				    int total,
//				    int first);

    $upcall $async void histFold(histFoldCallback, 
				 perfStreamHandle, 
				 timeStamp width,
				 phaseType);

    $upcall $async void changeState(appStateChangeCallback, 
				    perfStreamHandle, 
				    appState);

    //
    // Info. about a new phase that was created
    //
    $upcall $async void newPhaseInfo(newPhaseCallback, 
				     perfStreamHandle,
				     const char *,       // name
				     phaseHandle,       // unique id 
				     timeStamp,         // start time 
				     timeStamp,         // end time 
				     float);            // bucketwidth


    // Called to inform the data manager that it is ok to parse the mdl files
    // This informs the data manager that all igen interfaces have been created
    // and that it is ok to use them -- the start up of all the thread is
    // inelegant
    // $async void kludge(char *metric_file);


    $signore
    static int socket;
    static int sock_fd;
    static int DM_sequential_init(init_struct*);
    static int DM_post_thread_create_init(int);
    static dataManager *dm;  // need this so can call setTid which is
			     // not a static function, otherwise could
			     // just define all DM functions as static
    $signore
};

#endif
