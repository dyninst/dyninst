/*
 * DMappConext.C: application context class for the data manager thread.
 *
 * $Log: DMappContext.C,v $
 * Revision 1.6  1994/02/24 04:36:29  markc
 * Added an upcall to dyninstRPC.I to allow paradynd's to report information at
 * startup.  Added a data member to the class that igen generates.
 * Make depend differences due to new header files that igen produces.
 * Added support to allow asynchronous starts of paradynd's.  The dataManager has
 * an advertised port that new paradynd's can connect to.
 *
 * Revision 1.5  1994/02/09  22:35:29  hollings
 * added debugging code to print Hash table.
 *
 * Revision 1.4  1994/02/08  21:05:54  hollings
 * Found a few pointer problems.
 *
 * Revision 1.3  1994/02/05  23:14:00  hollings
 * Made sure we didn't return an mi when the enable failed.
 *
 * Revision 1.2  1994/02/02  00:42:31  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 * Revision 1.1  1994/01/28  01:34:15  hollings
 * The initial version of the Data Management thread.
 *
 *
 */
#include <assert.h>
extern "C" {
#include <math.h>
double   quiet_nan(int unused);
#include <malloc.h>
#include "thread/h/thread.h"
}

#include "dataManager.h"
#include "dyninstRPC.CLNT.h"
#include "DMinternals.h"

List<performanceStream*> applicationContext::streams;

void paradynDdebug(int pid)
{
}

String_Array convertResourceList(resourceList *rl)
{
    String_Array ret;

    ret.count = rl->getCount();
    ret.data = rl->convertToStringList();
    return(ret);
}

//
// add a new paradyn daemon
// called when a new paradynd contacts the advertised socket
//
int applicationContext::addDaemon (int new_fd)
{
  paradynDaemon *new_daemon;

  new_daemon = new paradynDaemon (new_fd, NULL, NULL);
  // TODO - setup machine, program, login?

  msg_bind (new_daemon->fd, TRUE);

  // TODO - do I need the pid for this ?
  // The pid is reported later in an upcall
  // paradynDdebug (new_daemon->pid);

  return (0);
}


//
// add a new executable (binary) to a program.
//
int applicationContext::addExecutable(char  *machine,
				  char *login,
				  char *program,
				  int argc,
				  char **argv)
{
     int pid;
     executable *exec;
     paradynDaemon *daemon;
     String_Array programToRun;
     List<paradynDaemon*> curr;

    // find out if we have a paradynd on this machine+login+paradynd
    for (curr=daemons, daemon = NULL; *curr; curr++) {
	if (!strcmp((*curr)->machine, machine) &&
	    !strcmp((*curr)->login, login) &&
	    !strcmp((*curr)->program, program)) {
	    daemon = *curr;
	}
    }

    // nope start one.
    if (!daemon) {
	daemon = new paradynDaemon(machine, login, program, NULL, NULL);
	if (daemon->fd < 0) {
	    printf("unable to start paradynd: %s\n", program);
	    exit(-1);
	}
	daemons.add(daemon);
	msg_bind(daemon->fd, TRUE);
	paradynDdebug(daemon->pid);
    }

    programToRun.count = argc;
    programToRun.data = argv;
    pid = daemon->addExecutable(argc, programToRun);
    exec = new executable(pid, argc, argv, daemon);
    programs.add(exec);

    return(0);
}

//
// Indicate if at least one application has been defined.
//
Boolean applicationContext::applicationDefined() {
    return(programs.count() != 0);
}

//
// start the programs running.
//
Boolean applicationContext::startApplication()
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->startProgram(exec->pid);
    }
    return(TRUE);
}

//
// pause all processes.
//
Boolean applicationContext::pauseApplication()
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->pauseProgram(exec->pid);
    }
    return(TRUE);
}

//
// continue all processes.
//
Boolean applicationContext::continueApplication()
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->continueProgram(exec->pid);
    }
    return(TRUE);
}

//
// detach the paradyn tool from a running program.  This should clean all
//   of the dynamic instrumentation that has been inserted.
//
Boolean applicationContext::detachApplication(Boolean)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->continueProgram(exec->pid);
    }
    return(TRUE);
}

//
// print the status of each process.  This is used mostly for debugging.
//
void applicationContext::printStatus()
{
    String status;
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	status = exec->controlPath->getStatus(exec->pid);
	printf("%s\n", status);
    }
}

//
// Cause the passed process id to dump a core file.  This is also used for
//    debugging.
//
void applicationContext::coreProcess(int pid)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	if (exec->pid == pid) {
	    exec->controlPath->coreProcess(exec->pid);
	    printf("found process and coreing it\n");
	}
    }
}

//
// Find out what metrics are available.  This just returns their names.
//
String_Array applicationContext::getAvailableMetrics()
{
    int i;
    String_Array names;
    HTable<metric*> cm;

    names.count = metric::allMetrics.count();
    names.data = (String *) malloc(sizeof(String) * names.count);
    for (cm=metric::allMetrics,i=0; *cm; cm++,i++) {
       names.data[i] = (*cm)->getName();
       assert(names.data[i]);
    }
    metric::allMetrics.print();
    assert(i==names.count);
    return(names);
}

//
// look up the metric info about the passed metric.
//
metric *applicationContext::findMetric(char *name)
{
    name = metric::names.findAndAdd(name);
    return(metric::allMetrics.find(name));
    return(NULL);
}

//
// Get the expected delay (as a fraction of the running program) for the passed
//   resource list (focus) and metric.
//
float applicationContext::getPredictedDataCost(resourceList *rl, metric *m)
{
    double val, max;
    String_Array ra;
    List<paradynDaemon*> curr;

    ra = convertResourceList(rl);
    max = 0.0;
    for (curr = daemons; *curr; curr++) {
	val = (*curr)->getPredictedDataCost(ra, m->getName());
	if (val > max) val = max;
    }
    return(max);
}

//
// Start collecting data about the passed resource list (focus) and metric.
//    The returned metricInstance is used to provide a unique handle for this
//    metric/focus pair.
//
metricInstance *applicationContext::enableDataCollection(resourceList *rl, 
							 metric *m)
{
    int id;
    String_Array ra;
    Boolean foundOne;
    metricInstance *mi;
    List<paradynDaemon*> curr;

    ra = convertResourceList(rl);

    // 
    // for each daemon request the data to be enabled.
    //
    mi = new metricInstance(rl, m);
    foundOne = FALSE;
    for (curr = daemons; *curr; curr++) {
	id = (*curr)->enableDataCollection(ra, m->getName());
	if (id > 0) {
	    mi->components.add(new component(*curr, id, mi));
	    foundOne = TRUE;
	}
    }
    if (foundOne) {
	mi->data = new Histogram(m->getStyle());
	m->enabledCombos.add(mi, (void*) rl);
	return(mi);
    } else {
	delete(mi);
	return(NULL);
    }
}

//
// This actuals stops the data from being collected.
//
void applicationContext::disableDataCollection(metricInstance *mi)
{
    metric *m;
    component *c;
    List<component*> curr;

    m = mi->met;
    m->enabledCombos.remove(mi->focus);
    for (curr = mi->components; c = *curr; curr++) {
	delete(c);
    }
    delete(mi);
}
