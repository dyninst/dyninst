/*
 * DMappConext.C: application context class for the data manager thread.
 *
 * $Log: DMappContext.C,v $
 * Revision 1.51  1995/02/27 18:41:26  tamches
 * Minor changes to reflect the new TCthread; tunable const header file
 * has moved and call syntax has changed.
 *
 * Revision 1.50  1995/02/16  19:10:39  markc
 * Removed start slash from comments
 *
 * Revision 1.49  1995/02/16  08:11:16  markc
 * Changed Boolean to bool
 * Changed interfaces that call igen-xdr calls to use strings and vectors rather
 *    than char*'s and igen-arrays
 *
 * Revision 1.48  1995/01/26  17:58:11  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.47  1994/12/21  00:36:41  tamches
 * Minor change to tunable constant declaration to reflect new tc constructors.
 * Fewer compiler warnings.
 *
 * Revision 1.46  1994/11/09  18:39:30  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.45  1994/11/04  20:13:32  karavan
 * added a status line.
 *
 * Revision 1.44  1994/11/04  16:30:38  rbi
 * added getAvailableDaemons()
 *
 * Revision 1.43  1994/11/03  20:54:01  karavan
 * Changed error printfs to calls to UIM::showError
 *
 * Revision 1.42  1994/11/03  16:10:11  rbi
 * Updated addExecutable
 *
 * Revision 1.41  1994/11/02  11:45:12  markc
 * Put a hack into addExecutable to handle incorrect parameters passed in.
 *
 * Revision 1.40  1994/09/22  00:52:29  markc
 * Changed "String" to "char*"
 * Used access methods for private member functions from igen classes
 * Removed purify error (new char[len] --> char[len+1] on line 303
 * Typecast args passed to msg_bind_buffered
 * Added "const" to applicationContext::defineDaemon(), ::findEntry() args
 *
 * Revision 1.39  1994/09/05  20:02:59  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.38  1994/08/22  15:57:26  markc
 * Added support for config language version 2.
 * Check for "" parms and convert them to NULL since NULL signifies undefined,
 * not "".
 * Support daemon dictionary.
 *
 * Revision 1.37  1994/08/08  20:15:17  hollings
 * added suppress instrumentation command.
 *
 * Revision 1.36  1994/08/05  16:03:55  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.35  1994/08/03  19:06:23  hollings
 * Added tunableConstant to print enable/disable pairs.
 *
 * Fixed fold to report fold info to all perfStreams even if they have
 * not active data being displayed.
 *
 * Revision 1.34  1994/07/28  22:31:06  krisna
 * include <rpc/types.h>
 * stringCompare to match qsort prototype
 * proper prorotypes for starting DMmain
 *
 * Revision 1.33  1994/07/25  14:55:34  hollings
 * added suppress resource option.
 *
 * Revision 1.32  1994/07/14  23:46:23  hollings
 * added getCurrentHybridCost, and fixed max for predicted cost.
 *
 * Revision 1.31  1994/07/08  21:56:25  jcargill
 * Changed XDR connection to paradynd to a buffered bind
 *
 * Revision 1.30  1994/07/07  03:28:59  markc
 * Added routine to start a paradyn daemon.
 * Changed int returns to type Boolean to agree with expected return values from
 * interface functions.
 *
 * Revision 1.29  1994/07/05  03:27:15  hollings
 * added observed cost model.
 *
 * Revision 1.28  1994/07/02  01:43:08  markc
 * Removed all uses of type aggregation from enableDataCollection.
 * The metricInfo structure now contains the aggregation operator.
 *
 * Revision 1.27  1994/06/29  02:55:55  hollings
 * fixed code to remove instrumenation when done with it.
 *
 * Revision 1.26  1994/06/23  19:26:20  karavan
 * added option for core dump of all processes with pid=-1 in coreProcess
 * command.
 *
 * Revision 1.25  1994/06/17  22:07:57  hollings
 * Added code to provide upcall for resource batch mode when a large number
 * of resources is about to be added.
 *
 * Revision 1.24  1994/06/14  15:21:34  markc
 * Set the aggOp field in metricInstance so the metric can choose from one of
 * four types of aggregation (max, min, sum, avg).  The aggregation is done in
 * aggregateSample.C
 *
 * Revision 1.23  1994/06/02  23:25:17  markc
 * Added virtual function 'handle_error' to pardynDaemon class which uses the
 * error handling features that igen provides.
 *
 * Revision 1.22  1994/05/30  19:23:58  hollings
 * Corrected call to change state for continue to be appRunning not appPaused.
 *
 * Revision 1.21  1994/05/23  20:28:04  karavan
 * fixed return values for addExecutable
 *
 * Revision 1.20  1994/05/17  00:17:06  hollings
 * Made sure we did the correct thing on a callErrr.
 *
 * Revision 1.19  1994/05/11  18:45:37  markc
 * Put code in addExecutable to assign the machine name for paradynDaemons
 * that are started on the local host.
 *
 * Revision 1.18  1994/05/10  03:57:35  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.17  1994/05/09  20:56:18  hollings
 * added changeState callback.
 *
 * Revision 1.16  1994/04/20  15:30:09  hollings
 * Added error numbers.
 * Added data manager function to get histogram buckets.
 *
 * Revision 1.15  1994/04/18  22:28:30  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.14  1994/03/31  01:42:19  markc
 * Added pauseProcess, continueProcess member functions.
 *
 * Revision 1.13  1994/03/25  22:59:28  hollings
 * Made the data manager tolerate paraynd's dying.
 *
 * Revision 1.12  1994/03/24  16:41:18  hollings
 * Added support for multiple paradynd's at once.
 *
 * Revision 1.11  1994/03/22  21:02:53  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.10  1994/03/20  01:49:46  markc
 * Gave process structure a buffer to allow multiple writers.  Added support
 * to register name of paradyn daemon.  Changed addProcess to return type int.
 *
 * Revision 1.9  1994/03/08  17:39:31  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.8  1994/03/01  21:24:50  hollings
 * removed call to print all metrics.
 *
 * Revision 1.7  1994/02/25  20:58:10  markc
 * Added support for storing paradynd's pids.
 *
 * Revision 1.6  1994/02/24  04:36:29  markc
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
double   quiet_nan(int unused);
#include <malloc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <stdio.h>
}

#include "thread/h/thread.h"
#include "../pdMain/paradyn.h"
#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMinternals.h"
#include "../TCthread/tunableConst.h"
#include "../UIthread/Status.h"

List<performanceStream*> applicationContext::streams;

status_line *DMstatus;

void paradynDdebug(int pid)
{
}

// change a char* that points to "" to point to NULL
// NULL is used to signify "NO ARGUMENT"
// NULL is easier to detect than "" (which needs a strlen to detect)
static void fixArg(char *&argToFix)
{
  if (argToFix && !strlen(argToFix))
    argToFix = (char*) 0;
}

String_Array convertResourceList(resourceList *rl)
{
    String_Array ret;

    ret.count = rl->getCount();
    ret.data = (char **) rl->convertToStringList();
    return(ret);
}

// called when a program is started external to paradyn and
// must inform paradyn that it exists
bool applicationContext::addRunningProgram (int pid,
					    const vector<string> &argv,
					    paradynDaemon *daemon)
{
	executable *exec = new executable (pid, argv, daemon);
	programs.add(exec);
	return true;
}


//
// add a new paradyn daemon
// called when a new paradynd contacts the advertised socket
//
bool applicationContext::addDaemon (int new_fd)
{
  paradynDaemon *new_daemon;

  new_daemon = new paradynDaemon (new_fd);

  msg_bind_buffered (new_daemon->get_fd(), true, (int(*)(void*)) xdrrec_eof,
		     (void*)new_daemon->net_obj());

  // TODO - do I need the pid for this ?
  // The pid is reported later in an upcall
  // paradynDdebug (new_daemon->pid);

  daemons.add(new_daemon);

  return (true);
}

//
// Dispose of daemon state.
//    This is called because someone wants to kill a daemon, or the daemon
//       died and we need to cleanup after it.
//
void applicationContext::removeDaemon(paradynDaemon *d, bool informUser)
{
#ifdef notdef
    executable *e;
    List<executable*> progs;
#endif

    if (informUser) {
	//printf("paradynd (pid %d) had died\n", d->getPid());
	//printf("paradyn Error #5\n");
      uiMgr->showError (5, "paradynd has died");
    }

    d->dead = true;
#ifdef notdef
    daemons.remove(d);

    //
    // Delete executables running on the dead paradyn daemon.
    //
    for (progs = programs; e = *progs; progs++) {
       if (e->controlPath == d) {
	   programs.remove(e);
	   delete(e);
       }
    }

#endif

    // tell the thread package to ignore the fd to the daemon.
    msg_unbind(d->get_fd());
}

//
// add a new daemon
// check to see if a daemon that matches the function args exists
// if it does exist, return a pointer to it
// otherwise, create a new daemon
//
paradynDaemon *applicationContext::getDaemonHelper (char *machine, 
						    char *login,
						    char *name)
{
  paradynDaemon *daemon;
  List<paradynDaemon*> curr;
  daemonEntry *def;
  char statusLine[256];

  // if name is null, use the default daemon name
  if (!name) 
    if (!setDefaultArgs(name))
      return NULL;
  
  // find out if we have a paradynd on this machine+login+paradynd
  for (curr=daemons, daemon = NULL; *curr; curr++) {
    if ((!machine || ((*curr)->machine == machine)) &&
	(!login || ((*curr)->login == login)) &&
	(name && ((*curr)->name == name))) {
      return (*curr);
    }
  }

  // find a matching entry in the dicitionary, and start it
  def = findEntry(machine, name);
  if (!def)
    return ((paradynDaemon*) 0);

  // fill in machine name if emtpy
  if (!machine) {
    struct utsname un;
    P_uname(&un);
    machine = P_strdup(un.nodename);
  }

  sprintf(statusLine, "Starting daemon on %s",machine);
  (*DMstatus) << statusLine;

  daemon = new paradynDaemon(machine, login, def->getCommand(), def->getName(),
			     def->getFlavor());

  (*DMstatus) << "ready";

  if (daemon->get_fd() < 0) {
    //printf("unable to start paradynd: %s\n", def->getCommand());
    //printf("paradyn Error #6\n");
    uiMgr->showError (6, "unable to start paradynd");
    return((paradynDaemon*) NULL);
  }
  daemons.add(daemon);
  msg_bind_buffered (daemon->get_fd(), true, (int(*)(void*))xdrrec_eof,
		     (void*) daemon->net_obj());

  paradynDdebug(daemon->getPid());
  return daemon;
}

// 
// add a new daemon, unless a daemon is already running on that machine
// with the same machine, login, and program
//
bool applicationContext::getDaemon (char *machine,
				       char *login,
				       char *name)
{
  // change all "" to NULL
  fixArg(machine); fixArg(login); fixArg(name);

  if (!getDaemonHelper(machine, login, name))
    return false;
  else
    return true;
}

bool applicationContext::defineDaemon (const char *command,
					  const char *dir,
					  const char *login,
					  const char *name,
					  const char *machine,
					  int flavor)
{
  List<daemonEntry*> walk;
  daemonEntry *newE;

  if (!name || !command)
    return false;

  for (walk=allEntries; *walk; walk++)
    if ((*walk)->getName() == name) {
      if ((*walk)->setAll(machine, command, name, login, dir, flavor))
	return true;
      else
	return false;
    }
  newE = new daemonEntry(machine, command, name, login, dir, flavor);
  if (!newE)
    return false;
  allEntries.add(newE);
  return true;
}

daemonEntry *applicationContext::findEntry(const char *m, const char *n)
{
  List<daemonEntry*> walk;

  if (!n)
    return ((daemonEntry*) 0);
  // TODO -- casting
  for (walk=allEntries; *walk; walk++) {
    if ((*walk)->getName() == n)
      return (*walk);
  }
  return ((daemonEntry*) 0);
}

void applicationContext::printDaemons()
{
  List<daemonEntry*> walk;
  for (walk=allEntries; *walk; walk++) {
    (*walk)->print();
  }
}

//
// Return list of names of defined daemons.  
//
String_Array applicationContext::getAvailableDaemons()
{
    int i;
    String_Array names;
    List<daemonEntry*> walk;

    walk=allEntries;
    names.count = walk.count();
    names.data = (char **) malloc(sizeof(char*) * names.count);
    // TODO -- casting stringHandle to char*
    // TODO -- memory leak ?
    for (i=0; *walk; walk++,i++) {
       names.data[i] = P_strdup((*walk)->getName().string_of());
       assert(names.data[i]);
    }
    assert(i==names.count);
    return(names);
}

//
// add a new executable (binary) to a program.
//
bool applicationContext::addExecutable(char  *machine,
					  char *login,
					  char *name,
					  char *dir,
				          const vector<string> &argv)
{
  int pid;
  executable *exec;
  paradynDaemon *daemon;
  static status_line pidnum("Processes");
  static char tmp_buf[256];

  if (! DMstatus) {
    DMstatus = new status_line("Data Manager");
  }

  if ((daemon = getDaemonHelper(machine, login, name)) ==
      (paradynDaemon*) NULL)
    return false;

  startResourceBatchMode();
  pid = daemon->addExecutable(argv);
  endResourceBatchMode();

  // did the application get started ok?
  if (pid > 0 && !daemon->did_error_occur()) {
    // TODO

    sprintf (tmp_buf, "%sPID=%d ", tmp_buf, pid);
    pidnum.message(tmp_buf);

    exec = new executable(pid, argv, daemon);
    programs.add(exec);
    return (true);
  } else {
    return(false);
  }
}

//
// Indicate if at least one application has been defined.
//
bool applicationContext::applicationDefined() {
    return(programs.count() != 0);
}

//
// start the programs running.
//
bool applicationContext::startApplication()
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->startProgram(exec->pid);
    }
    return(true);
}

//
// pause all processes.
//
bool applicationContext::pauseApplication()
{
    paradynDaemon *dm;
    performanceStream* s;
    List<paradynDaemon*> curr;
    List<performanceStream*> currStreams;

    for (curr = daemons; dm = *curr; curr++) {
	dm->pauseApplication();
    }

    // tell perf streams about change.
    for (currStreams = streams; s = *currStreams; currStreams++) {
	s->callStateFunc(appPaused);
    }

    return(true);
}

//
// pause one processes.
//
bool applicationContext::pauseProcess(int pid)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
        if (exec->pid == pid) break; 
    }
    if (exec) {
        exec->controlPath->pauseProgram(exec->pid);
        return(true); 
    } else
	return (false);
}

//
// continue all processes.
//
bool applicationContext::continueApplication()
{
    paradynDaemon *dm;
    performanceStream* s;
    List<paradynDaemon*> curr;
    List<performanceStream*> currStreams;

    for (curr = daemons; dm = *curr; curr++) {
	dm->continueApplication();
    }

    // tell perf streams about change.
    for (currStreams = streams; s = *currStreams; currStreams++) {
	s->callStateFunc(appRunning);
    }

    return(true);
}

//
// continue one processes.
//
bool applicationContext::continueProcess(int pid)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	if (exec->pid == pid) break;
    }
    if (exec) {
        exec->controlPath->continueProgram(exec->pid);
        return(true); 
    } else
	return (false);
}

//
// detach the paradyn tool from a running program.  This should clean all
//   of the dynamic instrumentation that has been inserted.
//
bool applicationContext::detachApplication(bool pause)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	exec->controlPath->detachProgram(exec->pid, pause);
    }
    return(true);
}

//
// print the status of each process.  This is used mostly for debugging.
//
void applicationContext::printStatus()
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
      string status = exec->controlPath->getStatus(exec->pid);
	if (!exec->controlPath->did_error_occur()) {
	  cout << status << endl;
	}
    }
}

//
// Cause the passed process id to dump a core file.  This is also used for
//    debugging.
// If pid = -1, all processes will dump core files.
//
void applicationContext::coreProcess(int pid)
{
    executable *exec;
    List<executable*> curr;

    for (curr = programs; exec = *curr; curr++) {
	if ((exec->pid == pid) || (pid == -1)) {
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
    names.data = (char **) malloc(sizeof(char*) * names.count);
    // TODO -- storing stringHandles into char* ?
    for (cm=metric::allMetrics,i=0; *cm; cm++,i++) {
       names.data[i] = (char*) (*cm)->getName();
       assert(names.data[i]);
    }
    assert(i==names.count);
    return(names);
}

//
// look up the metric info about the passed metric.
//
metric *applicationContext::findMetric(char *name)
{
    stringHandle iName;

    iName = metric::names.findAndAdd(name);
    return(metric::allMetrics.find(iName));
}

bool applicationContext::setInstSuppress(resource *res, bool newValue)
{
    bool ret;
    paradynDaemon *daemon;
    List<paradynDaemon*> curr;

    ret = false;
    for (curr = daemons; daemon = *curr; curr++) {
	ret |= daemon->setTracking((char*)res->getFullName(), newValue);
    }
    return(ret);
}

//
// Get the expected delay (as a fraction of the running program) for the passed
//   resource list (focus) and metric.
//
float applicationContext::getPredictedDataCost(resourceList *rl, metric *m)
{
    char *metName;
    double val, max;
    paradynDaemon *daemon;
    List<paradynDaemon*> curr;

    if (!rl || !m) return(0.0);

    vector<string> vs;
    assert(rl->convertToStringList(vs));

    max = 0.0;

    // TODO -- stringHandle or char*
    metName = (char*) m->getName();
    assert(metName);
    for (curr = daemons; *curr; curr++) {
	daemon = *curr;
	val = daemon->getPredictedDataCost(vs, metName);
	if (val > max) max = val;
    }
    return(max);
}

float applicationContext::getCurrentHybridCost()
{
    double val, max;
    paradynDaemon *daemon;
    List<paradynDaemon*> curr;

    max = 0.0;
    for (curr = daemons; *curr; curr++) {
	daemon = *curr;
	val = daemon->getCurrentHybridCost();
	if (val > max) max = val;
    }
    return(max);
}

void histDataCallBack(sampleValue *buckets,
		      int count,
		      int first,
		      void *arg)
{
    metricInstance *mi;
    performanceStream *ps;
    List<performanceStream*> curr;

    mi = (metricInstance *) arg;
    for (curr = mi->users; ps = *curr; curr++) {
	ps->callSampleFunc(mi, buckets, count, first);
    }
}

void histFoldCallBack(timeStamp width, void *arg)
{
    performanceStream *ps;
    static timeStamp oldWidth;
    List<performanceStream*> curr;

    if (oldWidth == width) return;
    oldWidth = width;

    for (curr = applicationContext::streams; ps = *curr; curr++) {
	ps->callFoldFunc(width);
    }
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
    component *comp;
    bool foundOne;
    stringHandle name;
    metricInstance *mi;
    paradynDaemon *daemon;
    List<paradynDaemon*> curr;

    vector<string> vs;
    assert(rl->convertToStringList(vs));

    // 
    // for each daemon request the data to be enabled.
    //
    mi = new metricInstance(rl, m);
    foundOne = false;

    // get a fresh copy of the TC "printChangeCollection"
    tunableBooleanConstant printChangeCollection = tunableConstantRegistry::findBoolTunableConstant("printChangeCollection");

    for (curr = daemons; daemon = *curr; curr++) {
	id = daemon->enableDataCollection(vs, (const char*) m->getName());
	if (printChangeCollection.getValue()) {
	  cout << "EDC:  " << (char*)m->getName()
	    << (char *) rl->getCanonicalName() << " " << id <<"\n";
	}
	if (id > 0 && !daemon->did_error_occur()) {
	    comp = new component(*curr, id, mi);
	    mi->components.add(comp, (void *) *curr);
	    mi->parts.add(&comp->sample, (void *) *curr);
	    foundOne = true;
	}
    }
    if (foundOne) {
	mi->data = new Histogram(m->getStyle(), 
				 histDataCallBack, 
				 histFoldCallBack, 
				 (void *) mi);
	name = rl->getCanonicalName();
	m->enabledCombos.add(mi, name);
	mi->count = 1;

	if (printChangeCollection.getValue()) {
	  cout << "EN: " << m->getName() 
	    << ((char *)rl->getCanonicalName()) << "\n";
	}
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
    stringHandle name;
    List<component*> curr;

    m = mi->met;
    name = mi->focus->getCanonicalName();

    // get a fresh copy of the TC "printChangeCollection"
    tunableBooleanConstant printChangeCollection = tunableConstantRegistry::findBoolTunableConstant("printChangeCollection");

    if (printChangeCollection.getValue()) {
	cout << "DI: " << m->getName() << ((char *) name) << "\n";
    }

    m->enabledCombos.remove(name);
    for (curr = mi->components; c = *curr; curr++) {
	delete(c);
    }
    // TODO -- is this safe, what if data is reported for this ?
    delete(mi);
}

void applicationContext::startResourceBatchMode() 
{
    List<performanceStream*> curr;
    for (curr = streams; *curr; curr++) {
	(*curr)->callResourceBatchFunc(batchStart);
    }
}

void applicationContext::endResourceBatchMode() 
{
    List<performanceStream*> curr;
    for (curr = streams; *curr; curr++) {
	(*curr)->callResourceBatchFunc(batchEnd);
    }
}

bool applicationContext::setDefaultArgs(char *&name)
{
  if (!name)
    name = strdup("defd");
  if (name)
    return true;
  else 
    return false;
}
