/*
 * dataManager.h - exported services of the Data Mgr thread of paradyn.
 *
 * $Log: dataManager.I,v $
 * Revision 1.4  1994/02/24 04:34:14  markc
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

/*
 * error handler call back.
 *
 */
typedef (*errorHandler)(int errno, char *message);

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
typedef (*sampleDataCallbackFunc)(performanceStream *ps, 
				  metricInstance *mi,
				  timeStamp startTimeStamp, 
				  timeStamp endTimeStamp, 
				  sampleValue value);

/*
 * Handler that gets called when new trace data is available.
 *   
 *   ps - a performanceStream from createPerformanceStream
 *   mi - a metricInstance returned by enableDataCollection
 *   time - time of the event.
 *   eventSize - size of the event specific data.
 *   eventData - event specific data.
 */
typedef (*traceDataCallbackFunc)(performanceStream *ps, metricInstance *mi,
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
    resource *newResource, char *name);

struct controlCallback {
    metricInfoCallback mFunc;
    resourceInfoCallback rFunc;
};

typedef struct controlCallback controlCallback;


$remote dataManager {
    $base 2000;
    $version 1;

    //
    // Create an applicationContext (an application to be studied)
    //
    applicationContext *createApplicationContext(errorHandler foo);

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
    // Continue a paused application.
    //    app - an application context from createPerformanceConext.
    //
    Boolean continueApplication(applicationContext *app);

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
    // dataCallback 	- Function to call when new data arrives
    // controlCallback	- Function to call when new structure info arrives
    // sampleInterval	- nominal interval between samples (sample only)
    //
    performanceStream *createPerformanceStream(applicationContext*, 
					       dataType, 
					       dataCallback, 
					       controlCallback);
	

    void setSampleRate(performanceStream *stream, timeStamp sampleInterval);

    //
    // Routines to control data collection on a performanceStream.
    //
    // performanceStream 	- a stream returned by createPerformanceStream
    // resourceList		- a list of resources
    // metric		- what metric to collect data for
    //
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
    // Resource utility functions.
    //
    resource *getRootResource();

    resourceList *getRootResources();

    char *getResourceName(resource*);

    resource *getResourceParent(resource*);

    resource *newResource(applicationContext *app,
			  resource *parent, 
			  char *name);

    resourceList *getResourceChildren(resource*);

    Boolean isResourceDescendent(resource *parent, resource *child);

    resource *findChildResource(resource *parent, char *name);

    int getResourceCount(resourceList*);

    resource *getNthResource(resourceList*, int n);

    resourceList *createResourceList();

    void addResourceList(resourceList*, resource*);

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
    char *getMetricName(metric*);

    //
    // get the supporting info about a metric.
    metricInfo *getMetricInfo(metric*);

    //
    // Get metric out of a metric instance.
    //
    metric *getMetric(metricInstance*);
    char *getMetricNameFromMI(metricInstance*);

    //
    // Get the value of a metric.
    //
    sampleValue getMetricValue(metricInstance*);

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
					   char *name);

    $upcall $async void newPerfData(sampleDataCallbackFunc, 
				    performanceStream *ps,
				    metricInstance *mi,
				    timeStamp startTimeStamp, 
				    timeStamp endTimeStamp, 
				    sampleValue value);
    $smember applicationContext *appContext;
};

#endif
