/*
 * dataManager.h - exported services of the Data Mgr thread of paradyn.
 *
 * $Log: dataManager.I,v $
 * Revision 1.27  1994/08/11 02:22:06  newhall
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
#include "rtinst/h/trace.h"
#include "util/h/hist.h"
#include "dyninstRPC.h"
#include "util/h/aggregation.h"
#include "util/h/stringPool.h"

/* one or more program working together */
class applicationContext;	

/* sequence of peformance data (trace|sample) */
class performanceStream;	

/* time */
// typedef double timeStamp;		

/* something that data can be collected for */
class resource;		

/* descriptive information about a resource */
typedef struct {
    char *name;			/* name of actual resource */
    char *fullName;		/* full path name of resource */
    timeStamp creation;		/* when did it get created */
} resourceInfo;		

/* list of resources */
class resourceList;		

/* a metric */
class metric;			

/* a list of metrics */
typedef metric *metricList;		

/* a metric/resourceList pair */
class metricInstance;		

typedef enum { Trace, Sample } dataType;
typedef enum { BASE, CMF, TCL } abstractionType;

/*
 * error handler call back.
 *
 */
typedef void (*errorHandler)(int errno, char *message);

/*
 * Handler that gets called when new sample data is delivered.
 *
 *   ps - a performanceStream from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   startTimeStamp - starting time of the interval covered by this sample.
 *   endTimeStamp - ending time of the interval covered by this sample.
 *   value - the value of this sample
 *    
 */
typedef void (*sampleDataCallbackFunc)(performanceStream *ps, 
				       metricInstance *mi,
				       sampleValue *buckets,
				       int count,
				       int total);

/*
 * Handler that gets called when new trace data is available.
 *   
 *   ps - a performanceStream from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   time - time of the event.
 *   eventSize - size of the event specific data.
 *   eventData - event specific data.
 */
typedef void (*traceDataCallbackFunc)(performanceStream *ps, metricInstance *mi,
    timeStamp time, int eventSize, void *eventData);

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
 * performanceStream	- a stream returned by createPerformanceStream
 * metric		- a metric
 *
 */
typedef void (*metricInfoCallback)(performanceStream*, metric*);

/*
 * Handler that gets called when a new resource is defined.
 *
 * performanceStream	- a stream returned by createPerformanceStream
 * parent		- parent of new resource
 * newResource		- new resource being created
 * name			- name of the new resource
 *
 */
typedef void (*resourceInfoCallback)(performanceStream*, resource *parent, 
    resource *newResource, stringHandle name);

typedef enum batchMode { batchStart, batchEnd };

typedef void (*resourceBatchModeCallback)(performanceStream*, batchMode mode); 

typedef void (*histFoldCallback)(performanceStream*, timeStamp width);

typedef enum appState { appRunning, appPaused };

typedef void (*appStateChangeCallback)(performanceStream*, appState state);

struct controlCallback {
    metricInfoCallback 		mFunc;
    resourceInfoCallback 	rFunc;
    resourceBatchModeCallback	bFunc;
    histFoldCallback		fFunc;
    appStateChangeCallback	sFunc;
};

typedef struct controlCallback controlCallback;


$remote dataManager {
    $base 2000;
    $version 13;

    //
    // Create an applicationContext (an application to be studied)
    //
    applicationContext *createApplicationContext(errorHandler foo);

    //
    // Start a paradyn daemon
    //
    Boolean addDaemon (applicationContext *app,
	               char *machine,
		       char *login,
		       char *name);
    //
    // Define a program to run. 
    //
    Boolean addExecutable(applicationContext *app,
			  char  *machine, 
			  char *login, 
			  char *name, 
			  int argc,
			  char **argv);

    //
    // Find out if an application has been defined yet.
    //
    Boolean applicationDefined(applicationContext *app);

    //
    // Start an application running (This starts the actual execution).
    //   app - an application context from createPerformanceConext.
    // 
    Boolean startApplication(applicationContext *app);

    //
    //   Stop all processes associted with the application.
    //	    app - an application context from createPerformanceConext.
    //
    // Pause an application (I am not sure about this but I think we want it).
    //      - Does this force buffered data to be delivered?
    //	- Does a paused application respond to enable/disable commands?
    //
    Boolean pauseApplication(applicationContext *app);

    //
    //   Stop all processes associted with the application.
    //	    app - an application context from createPerformanceConext.
    //
    // Pause a process in an application 
    //      - Does this force buffered data to be delivered?
    //	- Does a paused application respond to enable/disable commands?
    //
    Boolean pauseProcess(applicationContext *app, int pid);

    //
    //
    // Continue a paused application.
    //    app - an application context from createPerformanceConext.
    //
    Boolean continueApplication(applicationContext *app);

    //
    // Continue a paused process in an application.
    //    app - an application context from createPerformanceConext.
    //
    Boolean continueProcess(applicationContext *app, int pid);

    //
    // Disconnect the tool from the process.
    //    app - an application context from createPerformanceConext.
    //    pause - leave the process in a stopped state.
    //
    Boolean detachApplication(applicationContext *app,Boolean pause);

    // 
    // Create a performanceStream.  This a flow of performance information
    //   to a destination. 
    //
    // applicationContext	- an application context.
    // dataType 		- Sample or Trace
    // abstractionType  - source language
    // dataCallback 	- Function to call when new data arrives
    // controlCallback	- Function to call when new structure info arrives
    // sampleInterval	- nominal interval between samples (sample only)
    //
    performanceStream *createPerformanceStream(applicationContext*, 
					       dataType, 
					       abstractionType, 
					       dataCallback, 
					       controlCallback);

    // 
    // Destroy a performanceStream.  
    // 
    // applicationContext  - an application context.
    // performanceStream   - a stream returned by createPerformanceStream
    //
    int  destroyPerformanceStream(applicationContext*,
				 performanceStream*);

    void setSampleRate(performanceStream *stream, timeStamp sampleInterval);

    //
    // Routines to control data collection on a performanceStream.
    //
    // performanceStream 	- a stream returned by createPerformanceStream
    // resourceList		- a list of resources
    // metric		- what metric to collect data for
    metricInstance *enableDataCollection(performanceStream*,
					 resourceList*,
					 metric*);


    //
    // stop collecting data for the named metricInstance.
    // performanceStream	- a stream returned by createPerformanceStream
    // metricInstance	- a metricInstance returned by enableDataCollection.
    //
    void disableDataCollection(performanceStream*, metricInstance*);

    //
    // Return the expected cost of collecting performance data for a single
    //    metric at a given focus.  The value returned is the fraction of
    //    perturbation expected (i.e. 0.10 == 10% slow down expected).
    //
    float getPredictedDataCost(applicationContext*, resourceList*, metric*);

    //
    // Current best guess of the cost of inst in the system.
    //
    float getCurrentHybridCost(applicationContext*);

    //
    // Control information arriving about a resource Classes
    //
    // performanceStream	- a stream returned by createPerformanceStream
    // resource		- enable notification of children of this resource
    //
    void enableResourceCreationNotification(performanceStream*, resource*);

    //
    // Turn off notification of creation of descendants of this resource.
    // 
    // performanceStream	- a stream returned by createPerformanceStream
    // resource		- disable notification of descendants of this resource
    //
    //
    void disableResourceCreationNotification(performanceStream*, resource*);

    // 
    // Histogram/timebase information
    //
    timeStamp getCurrentBucketWidth();

    int getMaxBins();

    //
    // Resource utility functions.
    //
    resource *getRootResource();

    resourceList *getRootResources();

    resource *newResource(applicationContext *app,
     			  resource *parent, 
     			  char *name);

    //
    // don't search the SHG for this node.
    //
    void setResourceSearchSuppress(applicationContext *app,
			     resource *res, Boolean newValue);

    //
    // consider this resource a black box (applies to modules/procedures now).
    //
    void setResourceInstSuppress(applicationContext *app,
			     resource *res, Boolean newValue);

    // Get the static config. information for the passed applicationContext.
    //
    $array String getAvailableMetrics(applicationContext*);

    //
    // looks for a specifc metric instance in an application context.
    //
    metric *findMetric(applicationContext *context, char *name);

    //
    // Metric utility functions.
    //
    stringHandle getMetricName(metric*);

    //
    // get the supporting info about a metric.
    metricInfo *getMetricInfo(metric*);

    //
    // Get metric out of a metric instance.
    //
    metric *getMetric(metricInstance*);
    stringHandle getMetricNameFromMI(metricInstance*);

    //
    // Get the value of a metric.
    //
    sampleValue getMetricValue(metricInstance*);

    //
    // fill the passed array of buckets with the histogram sample from the
    //   passed metricInstance.  
    //		mi		metric instance to use
    //		buckets		array of buckets to fill
    //    	numberOfBuckets max number of buckets to fill
    //		first		first bin to fill (i.e. what bin goes into
    //				buckets[0]
    //
    int getSampleValues(metricInstance *mi, 
			sampleValue *buckets,
			int numberOfBuckets,
			int first);

    //
    // Get the total value of a metric. 
    //  (ie. do not divide by time enabled)
    //
    sampleValue getTotValue(metricInstance*);

    //
    // Debugger style calls.
    //
    void printResources();
    void printStatus(applicationContext*);
    void coreProcess(applicationContext*, int pid);

    //
    // upcalls for indications of events.
    //
    $upcall $async void newMetricDefined(metricInfoCallback, 
					 performanceStream*, 
					 metric*);

    $upcall $async void newResourceDefined(resourceInfoCallback,
					   performanceStream *ps, 
					   resource *parent,
					   resource *newResource, 
					   stringHandle name);

    $upcall $async void changeResourceBatchMode(resourceBatchModeCallback,
					        performanceStream *ps, 
						batchMode mode);

    $upcall $async void newPerfData(sampleDataCallbackFunc, 
				    performanceStream *ps,
				    metricInstance *mi,
				    sampleValue *buckets,
				    int total,
				    int first);

    $upcall $async void histFold(histFoldCallback, 
				 performanceStream *ps, 
				 timeStamp width);

    $upcall $async void changeState(appStateChangeCallback, 
				    performanceStream *ps, 
				    appState state);

    $smember applicationContext *appContext;
    $smember int socket;
    $smember int sock_fd;
};

#endif
