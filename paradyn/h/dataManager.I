/*
 * dataManager.h - exported services of the Data Mgr thread of paradyn.
 * $Id: dataManager.I,v 1.117 2005/01/28 18:12:02 legendre Exp $
 */

#ifndef MMTHREAD_H
#define MMTHREAD_H

#include "common/h/List.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"
#include "dyninstRPC.xdr.h"
#include "visi.xdr.h"
#include "pdutil/h/stringPool.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/DVbufferpool.h"

typedef enum {Global, Phase, All} SaveRequestType;

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
typedef void (*sampleDataCallbackFunc)(pdvector<dataValueType> *data,
				       u_int num_data_values);

/*
 * Handler that gets called when new trace data is available.
 *   
 *   handle - a performance stream handle from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   time - time of the event.
 *   eventSize - size of the event specific data.
 *   eventData - event specific data.
 *   Note: traceDataCallbackFunc needs to delete memory at timeStPtr
 */
typedef void (*traceDataCallbackFunc)(perfStreamHandle handle, 
				      metricInstanceHandle mi, 
				      timeStamp *timeStPtr, 
				      int eventSize, 
				      void *eventData);

/*
 * union to hold two types of data callback.
 *
 */
union dataCallback {
    sampleDataCallbackFunc 	sample;
    // Note: traceDataCallbackFunc needs to delete memory at timeStPtr
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
				   metricHandle handle,
				   dm_MetUnitsType units_type);

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

typedef void (*resourceRetireCallback)(perfStreamHandle handle, 
				       resourceHandle uniqueID, 
				       const char *name);

typedef void (*resourceUpdateCallback)(perfStreamHandle handle,
                                    resourceHandle theResource,
                                    const char * displayname,
                                    const char * abstraction);
typedef enum { batchStart, batchEnd } batchMode;

typedef void (*resourceBatchModeCallback)(perfStreamHandle, batchMode mode); 

// histFoldCallback must delete memory at widthPtr
typedef void (*histFoldCallback)(perfStreamHandle, timeLength *width,
                                 phaseType);

// initActValCallback must delete memory at initActValPtr
typedef void (*initActValCallback)(perfStreamHandle, 
				   metricHandle mi,
				   pdSample *initActValPtr,
                                   phaseType phaseID);

typedef enum { appRunning, appPaused, appExited } appState;

typedef void (*appStateChangeCallback)(perfStreamHandle, appState state);

// newPhaseCallback must delete memory at beginPtr, endPtr, bucketWidthPtr
typedef void (*newPhaseCallback)(perfStreamHandle handle, 
				 const char *name, 
				 phaseHandle phase,
				 relTimeStamp *beginPtr, 
				 relTimeStamp *endPtr, 
				 timeLength *bucketWidthPtr,
				 bool with_new_pc,
				 bool with_visis); 

typedef void (*forceFlushCallbackFunc)();

/*
 * Handler that gets called when there is a response to a 
 * getPredictedDataCost call 
 *   
 *   m_handle - the metricHandle assoc. with the request
 *   rl_handle - the resourceListHandle assoc. with the request 
 *   cost     - the cost of enabling the metric/focus pair
 *   clientId - the identifier passed by the calling thread
 */
typedef void (*predDataCostCallbackFunc)(u_int clientId,
					 float cost);

/*
 * Handler that gets called when there is a response to an enableData request 
 *   
 * response -   vector of responses to enable request one element for each   
 *		element in the vector passed by client to enableData request
 * request_id - identifier that client thread associated with this call
 *              (passed to DM on enableDataRequest)
 */
typedef void (*enableDataCallbackFunc)(pdvector<metricInstInfo> *response,
				       u_int request_id, u_int last_cb_for_request);

struct controlCallback {
    metricInfoCallback		mFunc;
    resourceInfoCallback	rFunc;
    resourceRetireCallback	retireFunc;
    resourceUpdateCallback      uFunc;
    resourceBatchModeCallback	bFunc;
    // histFoldCallback must delete memory at widthPtr
    histFoldCallback		fFunc;
    // initActValCallback must delete memory at 
    initActValCallback          avFunc;
    appStateChangeCallback	sFunc;
    // newPhaseCallback must delete memory at beginPtr, endPtr, bucketWidthPtr
    newPhaseCallback	        pFunc;
    predDataCostCallbackFunc    cFunc;
    enableDataCallbackFunc	eFunc;
    forceFlushCallbackFunc      flFunc;
};

typedef struct controlCallback controlCallback;

$remote dataManager {
    $base 2500;
    $version 60;


    // 
    // Define a daemon in the daemon dictionary
    //     command = command line to exec
    //     dir = directory to use as working directory
    //     login = user name to execute as
    //     name = name of dictionary entry, for matching
    //     flavor = programming environment (metCM5, metUNIX)
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
                        const char *remote_shell,
                        const char *MPI_type,
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
	               const char *MPItype,
		       const pdvector<pdstring> *argv);

    // Attach is the other way to start a process...newly implemented with v1.2
    bool attach(const char *machine,
	        const char *user,
	        const char *cmd,
	        const char *pidStr,
	        const char *daemonName,
	        int afterAttach // 1 --> pause, 2 --> run, 0 --> leave as is
		);
	        
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

    void createCallGraph();

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
    // Disconnect the tool from the process.
    //    pause - leave the process in a stopped state.
    //
    bool detachApplication(bool pause);

    // 
    // write all global data to files created in the dirname directory
    // 
    $async void saveAllData (const char *dirname, SaveRequestType optionFlag);

    // 
    // write all resource information to a file created in dirname
    //
    $async void saveAllResources (const char *dirname);

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

    //
    // Routines to control data collection on a performance Stream.
    //

    //
    // Request to enable a set of metric/focus pairs  
    // ps_handle - the perfStreamHandle of the calling thread 
    // pt_handle - the perfTraceStreamHandle of the calling thread 
    // request   - vector of metic/focus pairs to enable
    // request_Id - identifier passed by calling thread  
    // type - which phase type to enable data for
    // phaseId - the identifier of the phase for which data is requested
    // persistent_data - if set data is not distroyed on last disable 
    // persistent_collection - if set data collection isn't stoped on disable 
    // phase_persistent_data - like persistent_data, but only valid for curr
    //                         phase
    //
    $async void enableDataRequest(perfStreamHandle ps_handle,
                                  perfStreamHandle pt_handle,
                                  pdvector<metric_focus_pair> *request,
				  u_int request_Id,
				  phaseType type,
				  u_int phaseId,
				  u_int persistent_data,
				  u_int persistent_collection,
				  u_int phase_persistent_data);

    //
    // same as enableDataRequest, but with diff type request
    //
    $async void enableDataRequest2(perfStreamHandle ps_handle,
                                   pdvector<metricRLType> *request,
				   u_int request_Id,
				   phaseType type,
				   u_int phaseId,
				   u_int persistent_data,
				   u_int persistent_collection,
				   u_int phase_persistent_data);

    //
    // stop collecting data for the named metricInstance.
    // perfStreamHandle	- a handle returned by createPerformanceStream
    // metricInstance	- a metricInstance returned by enableDataCollection.
    // phaseType - specifies either global or current phase data
    //
    // If you are going to add something to a list using pt_handle on enable
    // it would probably be a good idea to remove it useing pt_handle on
    // disable rather than ps_handle...otherwise all those asserts you added
    // to check the size of the list are bound to fail when the ps_handle
    // and pt_handle are not the same.
    $async void disableDataCollection(perfStreamHandle ps_handle, 
				      perfStreamHandle pt_handle,
			              metricInstanceHandle,
			              phaseType);

    //
    // stop collecting data for the named metricInstance and clear the
    // persistent data flag(s) before disabling
    // ps_handle - a handle returned by createPerformanceStream
    // mi_handle - a metricInstance returned by enableDataCollection.
    // p_type - specifies either global or current phase data
    // clear_persistent_data - if true, clear persistent_data flag
    // clear_phase_persistent_data - if true, clear phase_persistent_data flag
    //
    $async void disableDataAndClearPersistentData(perfStreamHandle ps_handle,
					 perfStreamHandle pt_handle,
					 metricInstanceHandle mi_handle,
					 phaseType p_type,
					 bool clear_persistent_data,
					 bool clear_phase_persistent_data);
    //
    // This routine returns a list of foci which are the result of combining
    // each child of resource rh with the remaining resources that make up rlh
    // that are in the currentSearchPath if the resource rh is a 
    // component of the focus rlh, otherwise returns 0
    //
    pdvector<rlNameId> *magnify(resourceHandle rh, 
			      resourceListHandle rlh,
                              magnifyType m,
			      resourceHandle currentSearchPath);

    //
    // if resource rh is a decendent of a component of the focus, return a new
    // focus consisting of rh replaced with it's corresponding entry in rlh,
    // otherwise return the focus rlh
    //
    resourceListHandle *constrain(resourceHandle rh, resourceListHandle rlh);

    //
    // like constrain, except it returns 0 on failure
    //
    resourceListHandle *morespecific(resourceHandle rh, resourceListHandle rlh);

    //
    // returns true if seppressSearch is true for this focus
    //
    bool isSuppressed(resourceListHandle rlh);

    //
    // returns the name for the focus associated with this MI
    // returns 0 on error
    //
    const char *getFocusNameFromMI(metricInstanceHandle mi);

    //
    // returns the name for the focus associated with this resourceHandle
    //
    const char *getFocusName(const pdvector<resourceHandle>*);

    //
    // returns the name for the focus associated with this resourceListHandle
    // returns 0 on error
    //
    const char *getFocusNameFromHandle(resourceListHandle rlh);

    //
    // change the persistent_collection flag value for the assoc. MI
    //
    $async void setPersistentCollection(metricInstanceHandle);
    $async void clearPersistentCollection(metricInstanceHandle);

    //
    // change the persistent_data flag value for the assoc. MI
    //
    $async void setPersistentData(metricInstanceHandle);

    // 
    // this clears both persistent_data and phase_persistent_data flags of MI
    // 
    $async void clearPersistentData(metricInstanceHandle);

    //
    // Return the expected cost of collecting performance data for a single
    //    metric at a given focus.  The value returned is the fraction of
    //    perturbation expected (i.e. 0.10 == 10% slow down expected).
    //    perfStreamHandle and u_int are idetifiers for the request
    //
    $async void getPredictedDataCost(perfStreamHandle,
				     metricHandle,
				     resourceListHandle,
				     u_int);

    // 
    // Histogram/timebase information
    //
    timeLength* getBucketWidth( metricInstanceHandle mih, phaseType ptype );
    void getCurrentBucketWidth(timeLength *widthVal);
    void getGlobalBucketWidth(timeLength *widthVal);

    //
    // phase info
    //
    u_int getCurrentPhaseId();
    void getCurrentStartTime(relTimeStamp *getTimeVal);

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
    pdvector<resourceHandle> *getRootResources();


    //
    // converts from a vector of resourceHandles to a resourceListHandle
    // 
    resourceListHandle getResourceList(const pdvector<resourceHandle>*);

    //
    // converts from a resourceListHandle to a vector of resourceHandles
    // client is responsible for deleting vector<resourceHandle>*
    // 
    pdvector<resourceHandle> *getResourceHandles(resourceListHandle);


    //query for matched metric/focus.
    //the current match implemetation is that the code hierarchy 
    //resource name in reponse focus should be same as that included in 
    //parameter. no wildcard expression is taken into consideration
    pdvector<metric_focus_pair> *matchMetFocus(metric_focus_pair *);

    //
    // converts from a resource name to a resourceHandle 
    // the resource name must be the full name ie. "/Code/blah.c/foo"
    // returns NULL if conversion fails 
    //
    resourceHandle *findResource(const char *name);

    //updates a resource's display name in the case that the user
    //defines a new name
    void updateResource(resourceHandle theResource,
                         const char * name,
                         const char * displayname);
    //
    // returns name of resource (this is not a unique representation of 
    // the name instead it is the unique name trunctated)
    // so for "/Code/blah.c/foo" this routine will return "foo"
    //
    const char *getResourceLabelName(resourceHandle);


    //
    // returns full name of a resource  ie. "/Code/blah.c/foo" 
    //
    const char *getResourceName(resourceHandle);

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
    void setResourceSearchChildrenSuppress(resourceHandle res_handle, 
					   bool newValue);

    //
    // consider this resource a black box (applies to modules/procedures now).
    //
    void setResourceInstSuppress(resourceHandle res_handle, bool newValue);

    // 
    // returns a vector of metric names 
    // client is responsible for deleting these
    //
    pdvector<pdstring> *getAvailableMetrics(bool all);

    //
    // returns a vector of metric name and id pairs
    // client is responsible for deleting these
    //
    pdvector<met_name_id> *getAvailableMetInfo(bool all);

    //
    // looks for a specifc metric instance
    // returns NULL if search is unsuccessful
    // 
    metricHandle *findMetric(const char *name);

    //
    // Metric utility functions.
    //
    const char *getMetricName(metricHandle);

    //
    // get the supporting info about a metric.
    //
    T_dyninstRPC::metricInfo *getMetricInfo(metricHandle);

    // TODO: these shouldn't be necessary
    // Get metric out of a metric instance.
    //
    metricHandle *getMetric(metricInstanceHandle);
    const char *getMetricNameFromMI(metricInstanceHandle);

    //
    // Get the value of a metric.
    // Note: can't return a pdSample since igen can't handle class return
    // types
    void getMetricValue(metricInstanceHandle, pdSample *retSample);

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
			pdSample *buckets,
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
			pdSample *buckets,
			int numberOfBuckets,
			int first,
			phaseHandle phase_id);

    //
    // Get the total value of a metric. 
    //  (ie. do not divide by time enabled)
    //
    void getTotValue(metricInstanceHandle, pdSample *retSample);


    //
    //  start_Time: would allow a phase to be specified for some time
    //              in the future, this would be useful for using phase
    //              information from a config. file
    //              passing a NULL causes the current time in the Histogram
    //              to be used
    //  name: would be an optional user defined identifier for the phase
    //        each phase would still have a system defined unique id
    //  with_new_pc: flag that indicates if new instance of PC should be
    //		     started with this phase (DM just takes value of flag
    //		     and passes it back on newPhaseCallbacks)
    //  with_visis:  flag that indicates if new instance of curr. phase
    //		     visis should be started with the new phase (this 
    //		     option is not currently supported....the DM would
    //		     have to keep curr. phase MI's enabled for the new
    // 		     phase by setting collection flags for the MI's
    //               and calling a VM routine to start new versions of
    //		     all curr. phase visis with a pre-defined list of
    //		     MI's for each one)
    //
    $async void StartPhase(const relTimeStamp *startTimePtr, const char *name,
			   bool with_new_pc, bool with_visis);


    //
    // returns a list of all currently defined phases in the system
    // the caller is responsible for freeing the space associated with
    // the list
    //
    pdvector<T_visi::phase_info> *getAllPhaseInfo();


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
    // daemonStartInfo: provides information about the command line to start a 
    // daemon manually (needed if a rshd is not available on a remote machine)
    //
    void displayDaemonStartInfo();
    void printDaemonStartInfo(const char *filename);

    //
    // display information about Paradyn
    //
    void displayParadynGeneralInfo();
    void displayParadynLicenseInfo();
    void displayParadynReleaseInfo();
    void displayParadynVersionInfo();

    //
    // return a list of daemon names
    //
    pdvector<pdstring> *getAvailableDaemons();

    //
    // upcalls for indications of events.
    //
    $upcall $async void newMetricDefined(metricInfoCallback, 
					 perfStreamHandle, 
				         const char *name,
				         int style,
				         int aggregate,
				         const char *units,
				         metricHandle handle,
					 dm_MetUnitsType units_type);

    $upcall $async void newResourceDefined(resourceInfoCallback,
					   perfStreamHandle, 
					   resourceHandle parent,
					   resourceHandle newResource, 
					   const char *name,
					   const char *abstr);

    $upcall $async void retireResource_(resourceRetireCallback,
				        perfStreamHandle, 
                                        resourceHandle uniqueID, 
					const char *name);

    $upcall $async void updateResource_(resourceUpdateCallback,
                                        perfStreamHandle,
                                        resourceHandle theResource,
                                        const char * displayname,
                                        const char * abstr);

    $upcall $async void changeResourceBatchMode(resourceBatchModeCallback,
					        perfStreamHandle, 
						batchMode mode);

    // $upcall $async void newPerfData(sampleDataCallbackFunc, 
    $upcall $async void newPerfData(sampleDataCallbackFunc sampleFunc, 
				    pdvector<dataValueType> *data,
				    u_int num_data_values);

    // trace data streams
    $upcall $async void newTracePerfData(traceDataCallbackFunc traceFunc,
                                    pdvector<traceDataValueType> *traceData,
                                    u_int num_traceData_values);

    $upcall $async void predictedDataCost(predDataCostCallbackFunc func,
				     metricHandle m_handle,
				     resourceListHandle rl_handle, 
				     float cost,
				     u_int clientId);
    
    //
    // response to enableDataRequest call
    // func - callback function registered by client thread on createPerfStream
    // response - vector of enable reponses to enable
    // request_id - identifier passed by client to enableDataRequest
    //
    $upcall $async void enableDataCallback(enableDataCallbackFunc func,
    				           pdvector<metricInstInfo> *response,
   					   u_int request_id, u_int last_cb_for_request);

    $upcall $async void forceFlush(forceFlushCallbackFunc func);

    $upcall $async void histFold(histFoldCallback, 
				 perfStreamHandle, 
				 timeLength *widthPtr,
				 phaseType);

    $upcall $async void setInitialActualValue(initActValCallback, 
					      perfStreamHandle,
					      metricHandle mi, 
					      pdSample *initActValPtr,
					      phaseType phaseID);

    $upcall $async void changeState(appStateChangeCallback, 
				    perfStreamHandle, 
				    appState);

    //
    // Info. about a new phase that was created
    //
    $upcall $async void newPhaseInfo(newPhaseCallback, 
				     perfStreamHandle,
				     const char *,      // name
				     phaseHandle,       // unique id 
				     relTimeStamp *startPtr,
				     relTimeStamp *endPtr,
				     timeLength *bucketWidthPtr,
				     bool,		// with_new_pc
				     bool);		// with_visis

    $signore
    static int sock_port;
    static PDSOCKET sock_desc;
    static bool DM_sequential_init(const char*);
    static int DM_post_thread_create_init(DMthreadArgs*);
    static dataManager *dm;  // need this so can call setTid which is
			     // not a static function, otherwise could
			     // just define all DM functions as static
    static int termWin_port;
    static PDSOCKET termWin_sock;
    $signore
};

#endif
