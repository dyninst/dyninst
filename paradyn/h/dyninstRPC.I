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
 * Revision 1.25  1995/02/16 08:01:42  markc
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
 * Revision 1.24  1994/11/09  18:38:36  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.23  1994/11/03  16:05:31  rbi
 * eliminated argc from addExecutable interface
 *
 * Revision 1.22  1994/11/02  11:41:27  markc
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

/* descriptive information about a metric */
$remote struct metricInfo { 
    string name;
    int style;
    int aggregate;
    string units;
};

/*
 * Create a paradynd process.
 *
 */
int createDaemon(string machine,
		 string login,
		 string name,
		 vector<string> argv);


$remote dynRPC {
    $base 4000;
    $version 21;

    //
    // Define a program to run. 
    int addExecutable(vector<string> argv);

    //
    // Connect up with a running program (not possible on all platforms).
    //
    bool attachProgram(int pid);

    //
    // Start an application running (This starts the actual execution).
    //
    bool startProgram(int program);

    //
    //   Stop a process
    //
    bool pauseProgram(int program);

    //
    //   Stop all processes associted with the application.
    //
    bool pauseApplication();

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
    bool detachProgram(int program, bool pause);

    //
    // Gets called when a new application is started that is not visible to
    //   paradyn (e.g. pvm_spawn).
    //
    $upcall $async void newProgramCallbackFunc( int pid, 
						vector<string> argv, 
						string machine);
				

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
   // Handler that reports the initial time for which a daemon reports a sample
   // The earliest (absolute) initial time will become the base time (0)
   //
   $virtual $upcall $async void firstSampleCallback(int program,
                                                    double time);

    //
    // Handler that gets called when a new resource is defined.
    //
    // parent		- parent of new resource
    // newResource	- new resource being created (full name)
    // name		- short name of the new resource
    // abstraction      - abstraction to which it belongs     
    //
    $upcall $async void resourceInfoCallback(int program, 
					     string parent, 
					     string newResource, 
					     string name,
					     string abstraction);

    $upcall $async void resourceBatchMode(bool);

    //
    // Handler that gets called when new mapping data is collected.
    //
    $upcall $async void mappingInfoCallback(int program, 
					    string abstraction, 
					    string type, 
					    string key,
					    string value);

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
    bool setTracking(string resource, bool);

    //
    // Routines to control data collection on a performanceStream.
    //
    // resourceList	- a list of resources
    // metric	- what metric to collect data for
    //
    int enableDataCollection(vector<string> resourceList, string metric);

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
    double getPredictedDataCost(vector<string> resourceList, string metric);

    //
    // Current best guess about the cost of instrumentation.
    //
    double getCurrentHybridCost();

    //
    // Get the static configuration information.
    //
    vector<metricInfo> getAvailableMetrics();

    //
    // Create a new resource.
    //
    void addResource(string parent, string name);

    //
    // stdio or stderr from an application.
    //
    $upcall $async void applicationIO(int pid, int len, string line);

    //
    // stdio or stderr from an application.
    //
    $virtual $upcall $async void reportStatus(string line);

    //
    // Debugger style calls.
    //
    //
    string getStatus(int pid);
    void coreProcess(int pid);

    //
    // For paradynd's that may get started, but not by paradyn
    // This allows them to report system information
    // flavor = programming environment {metPVM, metCM5}
    $virtual $upcall $async void reportSelf(string machine, 
					    string program, 
					    int pid,
					    int flavor);

    //
    // For timing and other measurements of paradynd
    //
    void printStats();
};
