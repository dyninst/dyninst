
#define MetStyleEventCounter	0
#define MetStyleSampledFunction	1

#define opSum	0
#define opAvg	1
#define opMin	2
#define opMax	3

/* descriptive information about a metric */
$remote typedef struct { 
    String name;
    int style;
    int aggregate;
    String units;
} metricInfo;		

/*
 * Create a paradynd process.
 *
 */
int createDaemon(String machine,
		 String login,
		 String name,
		 int argc,
		 char *argv[]);


$remote dynRPC {
    $base 4000;
    $version 10;

    //
    // Define a program to run. 
    int addExecutable(int argc, $array String argv);

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
						$array String argv, 
						String machine);
				

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
					      String parent, 
					      String newResource, 
					      String name,
					      String abstraction);


    //
    // Handler that gets called when new mapping data is collected.
    //
    $upcall $async void mappingInfoCallback(int program, 
					      String abstraction, 
					      String type, 
					      String key,
					      String value);

    //
    // Request a unique resource name
    //
    $upcall String getUniqueResource(int program, 
				     String parent, 
				     String newResource);


    // Upcall for a new metric being defined.
    //
    $upcall $async void newMetricCallback(metricInfo info); 

    void setSampleRate(double sampleInterval);

    //
    // Routines to control data collection on a performanceStream.
    //
    // resourceList	- a list of resources
    // metric	- what metric to collect data for
    //
    int enableDataCollection($array String resourceList, String metric);

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
    double getPredictedDataCost($array String resourceList, String metric);

    //
    // Get the static configuration information.
    //
    $array metricInfo getAvailableMetrics();

    //
    // Create a new resource.
    //
    void addResource(String parent, String name);

    //
    // stdio or stderr from an application.
    //
    $upcall $async void applicationIO(int pid, int len, String line);

    //
    // Debugger style calls.
    //
    //
    String getStatus(int pid);
    void coreProcess(int pid);

    //
    // For paradynd's that may get started, but not by paradyn
    // This allows them to report system information
    $virtual $upcall $async void reportSelf(String machine, 
					    String program, 
					    int pid);

    $cmember int my_pid;
};
