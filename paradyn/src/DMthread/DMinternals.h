
/*
 * Define the classes used in the implementation of the data manager.
 *
 * $Log: DMinternals.h,v $
 * Revision 1.3  1994/02/03 23:26:57  hollings
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
#include "DMresource.h"

//
// A handle to a running paradynd* somewhere.
//
class paradynDaemon: public dynRPCUser {
    public:
	paradynDaemon(char *m, char *u, char *p, xdrIOFunc r, xdrIOFunc w):
	    dynRPCUser(m, u, p, r, w) {
		machine = m;
		login = u;
		program = p;

		allDaemons.add(this);
	}
        char *machine;
        char *login;
 	char *program;
	static List<paradynDaemon*> allDaemons;
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
	applicationContext(errorHandler ef)	{
 	    errorFunc = ef;
	}
	addExecutable(char  *machine,
                      char *login,
                      char *name,
                      int argc,
                      char **argv);
	Boolean applicationDefined();
	Boolean startApplication();
  	Boolean pauseApplication();	
	Boolean continueApplication();
	Boolean detachApplication(Boolean);
	void printStatus();
	void coreProcess(int pid);
	String_Array getAvailableMetrics();
	metric *findMetric(char *name);
	metricInstance *enableDataCollection(resourceList*, metric*); 
	float getPredictedDataCost(resourceList*, metric*);
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
			  dataCallback dc,
			  controlCallback cc,
			  int tid) {
	    appl = a;
	    type = t;
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
	void callSampleFunc(metricInstance *, double, double, double);
	void callResourceFunc(resource *p, resource *c, char *name);
    private:
	applicationContext      *appl;
	dataType                type;
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
	    allComponents.add(mi, (void *) id);
	}
	~component() {
	    daemon->disableDataCollection(id);
	}
	static HTable<metricInstance*> allComponents;
    private:
	paradynDaemon *daemon;
	int id;
};

class metric {
    public:
	metric(metricInfo i) { info = i; }
	metricInfo *getInfo() { return(&info); }
	String getName() { return(info.name); }
	metricStyle getStyle() { return((metricStyle) info.style); }
	List<metricInstance*> enabledCombos;
	static stringPool names;
	static HTable<metric*> allMetrics;
    private:
	metricInfo info;
};


class metricInstance {
    public:
	metricInstance(resourceList *rl, metric *m) {
	    met = m;
	    focus = rl;
	    count = 0;
	    enabledTime = 0.0;
	}
	float getValue() {
	    float ret;

	    if (!data) abort();
	    ret = data->getValue();
	    if (met->getStyle() != MetStyleSampledFunction) {
		ret /= enabledTime;
	    }
	    return(ret);
	}
	int count;		// active users (perfStreams)
	resourceList *focus;
	metric *met;
	List<component*> components;
	List<performanceStream*> users;
	Histogram *data;
	float enabledTime;
    private:
};

