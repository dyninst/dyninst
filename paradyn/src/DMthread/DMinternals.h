
/*
 * Define the classes used in the implementation of the data manager.
 *
 * $Log: DMinternals.h,v $
 * Revision 1.25  1994/08/08 20:15:19  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.24  1994/08/05  16:03:56  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.23  1994/07/14  23:45:53  hollings
 * added hybrid cost model.
 *
 * Revision 1.22  1994/07/07  03:31:20  markc
 * Changed return types for public functions to Boolean to agree with exported
 * functions.
 * Added code to handle relative path names and extract the tail when new
 * daemons are started.
 *
 * Revision 1.21  1994/07/05  03:27:16  hollings
 * added observed cost model.
 *
 * Revision 1.20  1994/07/02  01:43:09  markc
 * Removed all uses of type aggregation from enableDataCollection.
 * The metricInfo structure now contains the aggregation operator.
 *
 * Revision 1.19  1994/06/29  02:55:57  hollings
 * fixed code to remove instrumenation when done with it.
 *
 * Revision 1.18  1994/06/27  21:23:22  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.17  1994/06/17  22:07:58  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.16  1994/06/14  15:22:19  markc
 * Added support for aggregation.
 *
 * Revision 1.15  1994/06/02  23:25:18  markc
 * Added virtual function 'handle_error' to pardynDaemon class which uses the
 * error handling features that igen provides.
 *
 * Revision 1.14  1994/05/10  03:57:36  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.13  1994/05/09  20:56:19  hollings
 * added changeState callback.
 *
 * Revision 1.12  1994/04/19  22:08:37  rbi
 * Added getTotValue method to get non-normalized metric data.
 *
 * Revision 1.11  1994/03/31  01:40:37  markc
 * Added pauseProcess, continueProcess member functions.
 *
 * Revision 1.10  1994/03/25  22:59:31  hollings
 * Made the data manager tolerate paraynd's dying.
 *
 * Revision 1.9  1994/03/24  16:41:19  hollings
 * Added support for multiple paradynd's at once.
 *
 * Revision 1.8  1994/03/22  21:02:54  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.7  1994/03/21  20:32:47  hollings
 * Changed the mid to mi mapping to be per paradyn daemon.  This is required
 * because mids are asigned by the paradynd's, and are not globally unique.
 *
 * Revision 1.6  1994/03/20  01:49:47  markc
 * Gave process structure a buffer to allow multiple writers.  Added support
 * to register name of paradyn daemon.  Changed addProcess to return type int.
 *
 * Revision 1.5  1994/03/08  17:39:32  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.4  1994/02/24  04:36:30  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 *
 * Revision 1.3  1994/02/03  23:26:57  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.2  1994/02/02  00:42:32  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 *
 */
#include "util/h/list.h"
#include "util/h/hist.h"
#include "util/h/aggregateSample.h"
#include "DMresource.h"
#include "dataManager.h"
#include <string.h>

//
// A handle to a running paradynd* somewhere.
//
class paradynDaemon: public dynRPCUser {
    public:
	paradynDaemon(char *m, char *u, char *p, xdrIOFunc r, xdrIOFunc w):
	    dynRPCUser(m, u, p, r, w, args) {
	        char *loc;
		char *newm;

		if (!m) m = "";
		if (!u) u = "";
		
		// if p includes a pathname, lose the pathname
		loc = strrchr(p, '/');
		if (loc)
		  {
		    loc = loc + 1;
		    newm = strdup (loc);
		    free (m);
		    m = newm;
		  }
		
		machine = m;
		login = u;
		program = p;

		allDaemons.add(this);
	}
	// machine, program, and login are set via a callback
	paradynDaemon(int f, xdrIOFunc r, xdrIOFunc w):
	    dynRPCUser(f, r, w) {
		allDaemons.add(this);
	}

	~paradynDaemon();
	
	Boolean dead;			// has there been an error on the link.
	void reportSelf (String m, String p, int pd);
        char *machine;
        char *login;
 	char *program;
	// these args are passed to the paradynd when started
        // for paradyndPVM these args contain the info to connect to the
        // "well known" socket for new paradynd's
	static char **args;
	static List<paradynDaemon*> allDaemons;

	void paradynDaemon::sampleDataCallbackFunc(int program,
						   int mid,
						   double startTimeStamp,
						   double endTimeStamp,
						   double value);

	// all active metrics ids for this daemon.
	HTable<metricInstance*> activeMids;

	// replace the igen provided error handler
	virtual void handle_error();
};


//
// a binary running somewhere under the control of a paradynd*.
//
class executable {
    public:
	executable(int id, int c, char **av, paradynDaemon *p) {
	    pid = id;
	    argc = c;
	    argv = av;
	    controlPath = p;
	}
	int pid;
        int argc;
        char **argv;
        paradynDaemon *controlPath;
};

class applicationContext {
    public:
	void tellDaemonsOfResource(char *parent, char *name) {
	    List<paradynDaemon*> curr;
	    for (curr = daemons; *curr; curr++) {
		(*curr)->addResource(parent, name);
	    }
	}

	void startResourceBatchMode();
	void endResourceBatchMode();
	Boolean setInstSuppress(resource *, Boolean);

	applicationContext(errorHandler ef)	{
 	    errorFunc = ef;
	}
        Boolean addDaemon (int new_fd);
        void removeDaemon(paradynDaemon *d, Boolean informUser);

	// start a daemon on a specific machine, if the daemon
	// is not currently running on that machine
	paradynDaemon *getDaemonHelper (char *machine,
					char *login,
					char *program);

	// start a daemon on a specific machine, if the daemon
	// is not currently running on that machine
        Boolean getDaemon (char *machine,
			   char *login,
			   char *program);
	Boolean addExecutable(char  *machine,
			      char *login,
			      char *name,
			      int argc,
			      char **argv);
	Boolean addRunningProgram(int pid,
				  int argc,
				  char **argv, 
				  paradynDaemon *daemon);	
	Boolean applicationDefined();
	Boolean startApplication();
  	Boolean pauseApplication();	
	Boolean continueApplication();
  	Boolean pauseProcess(int pid);	
	Boolean continueProcess(int pid);
	Boolean detachApplication(Boolean);
	void printStatus();
	void coreProcess(int pid);
	String_Array getAvailableMetrics();
	metric *findMetric(char *name);
	metricInstance *enableDataCollection(resourceList*, metric*);
	float getPredictedDataCost(resourceList*, metric*);
	float getCurrentHybridCost();
	void disableDataCollection(metricInstance*);

	static List<performanceStream*> streams;
    private:
	List<executable*>	 programs;
	List<paradynDaemon*>	 daemons;
	errorHandler             errorFunc;
};

//
// A consumer of performance data.
//
class performanceStream {
	friend void addMetric(metricInfo info);
    public:
	performanceStream(applicationContext *a, 
			  dataType t,
			  abstractionType ab,
			  dataCallback dc,
			  controlCallback cc,
			  int tid) {
	    appl = a;
	    type = t;
	    abstraction = ab;
	    dataFunc = dc;
	    controlFunc = cc;
	    sampleRate = 10000;
	    threadId = tid;
        }
	void setSampleRate(timeStamp rate) { sampleRate = rate; }

	metricInstance *enableDataCollection(resourceList*, metric*);
	void disableDataCollection(metricInstance*);
	void enableResourceCreationNotification(resource*);
	void disableResourceCreationNotification(resource*);
	void callSampleFunc(metricInstance *, sampleValue*, int, int);
	void callResourceFunc(resource *p, resource *c, stringHandle name);
	void callResourceBatchFunc(batchMode mode);
	void callFoldFunc(timeStamp width);
	void callStateFunc(appState state);
        abstractionType getAbstraction() { return abstraction; }
    private:
	applicationContext      *appl;
	dataType                type;
        abstractionType         abstraction;
	dataCallback            dataFunc;
	controlCallback         controlFunc;
	int threadId;
	// List<metricInstance*>   enabledMetrics;
	timeStamp               sampleRate;     /* sample sampleRate usec */
};

//
// a part of an mi.
//
class component {
    public:
	component(paradynDaemon *d, int i, metricInstance *mi) {
	    daemon = d;
	    id = i;
	    d->activeMids.add(mi, (void *) id);
	}
	~component() {
	    daemon->disableDataCollection(id);
	}
	sampleInfo sample;
    private:
	paradynDaemon *daemon;
	int id;
};

class metric {
    public:
	metric(metricInfo i) {
	  info.style = i.style;
	  info.units = strdup (i.units);
	  info.name = strdup (i.name);
          info.aggregate = i.aggregate;
	}
	metricInfo *getInfo() { return(&info); }
	String getName() { return(info.name); }
	metricStyle getStyle() { return((metricStyle) info.style); }
        int getAggregate() { return info.aggregate;}
	List<metricInstance*> enabledCombos;
	static stringPool names;
	static HTable<metric*> allMetrics;
    private:
	metricInfo info;
};


class metricInstance {
    public:
	~metricInstance() {
	    if (data) delete(data);
	}
	metricInstance(resourceList *rl, metric *m) {
	    met = m;
	    focus = rl;
	    count = 0;
	    enabledTime = 0.0;
            sample.aggOp = m->getAggregate();
	    data = NULL;
	}
	float getValue() {
	    float ret;

	    if (!data) abort();
	    ret = data->getValue();
	    ret /= enabledTime;
	    return(ret);
	}
	float getTotValue() {
	    float ret;

	    if (!data) abort();
	    ret = data->getValue();
	    return(ret);
	}
	int count;		// active users (perfStreams)
	resourceList *focus;
	metric *met;
	sampleInfo sample;
	List<sampleInfo*> parts;
	List<component*> components;
	List<performanceStream*> users;
	Histogram *data;
	float enabledTime;
    private:
};

