/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * $Log: dyninstRPC.I,v $
 * Revision 1.22  1994/11/02 11:41:27  markc
 * Incremented the version.
 *
 * Revision 1.21  1994/09/22  00:44:26  markc
 * Incremented version number
 * Changed "String" to "char*"
 * Made array declarations explicit
 *
 * Revision 1.20  1994/08/22  16:01:31  markc
 * Remove my_pid which duplicated a class member
 * Add daemon dictionary calls to dataManager interface.
 *
 * Revision 1.19  1994/08/17  17:55:27  markc
 * Incremented version numbers because a new version of igen has been
 * committed.
 *
 * Revision 1.18  1994/08/08  20:14:56  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.17  1994/07/20  16:30:40  hollings
 * added resourceBatchMode
 *
 * Revision 1.16  1994/07/14  23:44:29  hollings
 * added getCurrentHybridCost
 *
 * Revision 1.15  1994/07/05  03:26:54  hollings
 * added stat printing command.
 *
 * Revision 1.14  1994/07/02  01:45:24  markc
 * Removed aggregation operator from enableDataCollection call.
 * Remove aggregation operator definitions, which are provided by
 * util/h/aggregation.H
 *
 * Revision 1.13  1994/06/29  02:55:34  hollings
 * Added copyright and log line.
 *
 *
 */

#define MetStyleEventCounter	0
#define MetStyleSampledFunction	1

#include "util/h/aggregation.h"

$remote array char * String_Array;

/* descriptive information about a metric */
$remote typedef struct { 
    char *name;
    int style;
    int aggregate;
    char *units;
} metricInfo;		

$remote array metricInfo metricInfo_Array;

/*
 * Create a paradynd process.
 *
 */
int createDaemon(char *machine,
		 char *login,
		 char *name,
		 int argc,
		 char *argv[]);


$remote dynRPC {
    $base 4000;
    $version 19;

    //
    // Define a program to run. 
    int addExecutable(int argc, String_Array argv);

    //
    // Connect up with a running program (not possible on all platforms).
    //
    Boolean attachProgram(int pid);

    //
    // Start an application running (This starts the actual execution).
    //
    Boolean startProgram(int program);

    //
    //   Stop a process
    //
    Boolean pauseProgram(int program);

    //
    //   Stop all processes associted with the application.
    //
    Boolean pauseApplication();

    //
    // Continue a paused process
    //
    void continueProgram(int program);

    //
    // Continue all of the processes
    //
    void continueApplication();

    //
    // Disconnect the tool from the process.
    //    pause - leave the process in a stopped state.
    //
    //
    Boolean detachProgram(int program, Boolean pause);

    //
    // Gets called when a new application is started that is not visible to
    //   paradyn (e.g. pvm_spawn).
    //
    $upcall $async void newProgramCallbackFunc( int pid, 
						int argc, 
						String_Array argv, 
						char *machine);
				

    //
    // Handler that gets called when new sample data is delivered.
    //
    //   p - a program from addProgram
    //   mi - a metricInstance returned by enableDataCollection
    //   startTimeStamp - starting time of the interval covered by this sample.
    //   endTimeStamp - ending time of the interval covered by this sample.
    //   value - the value of this sample
    //
   $virtual $upcall $async void sampleDataCallbackFunc(int program, 
						       int mi,
						       double startTimeStamp, 
						       double endTimeStamp, 
						       double value);

    //
    // Handler that gets called when a new resource is defined.
    //
    // parent		- parent of new resource
    // newResource	- new resource being created (full name)
    // name		- short name of the new resource
    // abstraction      - abstraction to which it belongs     
    //
    $upcall $async void resourceInfoCallback(int program, 
					      char *parent, 
					      char *newResource, 
					      char *name,
					      char *abstraction);

    $upcall $async void resourceBatchMode(Boolean);

    //
    // Handler that gets called when new mapping data is collected.
    //
    $upcall $async void mappingInfoCallback(int program, 
					      char *abstraction, 
					      char *type, 
					      char *key,
					      char *value);

    //
    // Request a unique resource name
    //
    $upcall char *getUniqueResource(int program, 
				     char *parent, 
				     char *newResource);


    // Upcall for a new metric being defined.
    //
    $upcall $async void newMetricCallback(metricInfo info); 

    void setSampleRate(double sampleInterval);

    //
    // tell dyninst to ignore this resource for instrumentation.
    //    Currently this only applies to programs, modules, and procedures.
    //    False - suppress
    //    True - enable
    //
    Boolean setTracking(char *resource, Boolean);

    //
    // Routines to control data collection on a performanceStream.
    //
    // resourceList	- a list of resources
    // metric	- what metric to collect data for
    //
    int enableDataCollection(String_Array resourceList, char *metric);

    //
    // stop collecting data for the named mi.
    //     mi		- a metricInstance returned by enableDataCollection.
    //
    void disableDataCollection(int mi);

    //
    // Return the expected cost of collecting performance data for a single
    //    metric at a given focus.  The value returned is the fraction of
    //    perturbation expected (i.e. 0.10 == 10% slow down expected).
    //
    double getPredictedDataCost(String_Array resourceList, char *metric);

    //
    // Current best guess about the cost of instrumentation.
    //
    double getCurrentHybridCost();

    //
    // Get the static configuration information.
    //
    metricInfo_Array getAvailableMetrics();

    //
    // Create a new resource.
    //
    void addResource(const char *parent, const char *name);

    //
    // stdio or stderr from an application.
    //
    $upcall $async void applicationIO(int pid, int len, char *line);

    //
    // Debugger style calls.
    //
    //
    char *getStatus(int pid);
    void coreProcess(int pid);

    //
    // For paradynd's that may get started, but not by paradyn
    // This allows them to report system information
    // flavor = programming environment {metPVM, metCM5}
    $virtual $upcall $async void reportSelf(char *machine, 
					    char *program, 
					    int pid,
					    int flavor);

    //
    // For timing and other measurements of paradynd
    //
    void printStats();
};
