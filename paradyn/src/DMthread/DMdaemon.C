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
 *  method functions for paradynDaemon and daemonEntry classes
 */
#include <assert.h>
extern "C" {
double   quiet_nan();
#include <malloc.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <stdio.h>
}

#include "thread/h/thread.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMdaemon.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "paradyn/src/UIthread/Status.h"
#include "DMmetric.h"
#include "paradyn/src/met/metricExt.h"


// TEMP this should be part of a class def.
status_line *DMstatus=NULL;
status_line *PROCstatus=NULL;

// change a char* that points to "" to point to NULL
// NULL is used to signify "NO ARGUMENT"
// NULL is easier to detect than "" (which needs a strlen to detect)
/*
static void fixArg(char *&argToFix)
{
  if (argToFix && !strlen(argToFix))
    argToFix = (char*) 0;
}
*/

// called when a program is started external to paradyn and
// must inform paradyn that it exists
bool paradynDaemon::addRunningProgram (int pid,
				       const vector<string> &argv,
				       paradynDaemon *daemon)
{
    executable *exec = new executable (pid, argv, daemon);
    programs += exec;
    ++procRunning;
    return true;
}


//
// add a new paradyn daemon
// called when a new paradynd contacts the advertised socket
//
bool paradynDaemon::addDaemon (int new_fd)
{
  // constructor adds new daemon to dictionary allDaemons
  paradynDaemon *new_daemon = new paradynDaemon (new_fd);

  if (new_daemon->errorConditionFound) {
    //TODO: "something" has to be done for a proper clean up - naim
    uiMgr->showError(6,"");
    return(false);
  }

  msg_bind_buffered (new_daemon->get_fd(), true, (int(*)(void*)) xdrrec_eof,
		     (void*)new_daemon->net_obj());
  assert(new_daemon);
  // The pid is reported later in an upcall
  return (true);
}

//  TODO : fix this so that it really does clean up state
// Dispose of daemon state.
//    This is called because someone wants to kill a daemon, or the daemon
//       died and we need to cleanup after it.
//
void paradynDaemon::removeDaemon(paradynDaemon *d, bool informUser)
{

    if (informUser) {
        uiMgr->showError (5, "paradynd has died");
    }

    d->dead = true;

#ifdef notdef
    executable *e;
    List<executable*> progs;
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
paradynDaemon *paradynDaemon::getDaemonHelper(const string &machine, 
					      const string &login, 
					      const string &name) {

    // find out if we have a paradynd on this machine+login+paradynd
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        if ((!machine.string_of() || (pd->machine == machine)) && 
           (!login.string_of() || (pd->login == login))  &&
	   (name.string_of() && (pd->name == name))) {
	    return (pd);     
        }
    }
  
    // find a matching entry in the dicitionary, and start it
    daemonEntry *def = findEntry(machine, name);
    if (!def) 
	return ((paradynDaemon*) 0);

    string m = machine; 
    // fill in machine name if emtpy
    if (!m.string_of()) {
        struct utsname un;
        P_uname(&un);
        m = un.nodename;
    }

    char statusLine[256];
    sprintf(statusLine, "Starting daemon on %s",m.string_of());
    (*DMstatus) << statusLine;

    string flav_arg(string("-z") + def->getFlavor());
    unsigned asize = paradynDaemon::args.size();
    paradynDaemon::args += flav_arg;

    pd = new paradynDaemon(m, login, def->getCommandString(), 
			       def->getNameString(), def->getFlavorString());

    if (pd->errorConditionFound) {
      //TODO: "something" has to be done for a proper clean up - naim
      string msg;
      msg=string("Cannot create daemon process on host \"") + m + string("\"");
      uiMgr->showError(84,P_strdup(msg.string_of())); 
      return((paradynDaemon*) 0);
    }

    paradynDaemon::args.resize(asize);
    (*DMstatus) << "ready";

    if (pd->get_fd() < 0) {
        uiMgr->showError (6, "unable to start paradynd");
        return((paradynDaemon*) 0);
    }

   // Send the initial metrics, constraints, and other neato things
   mdl_send(pd);
   // Send the initial metrics, constraints, and other neato things
   vector<T_dyninstRPC::metricInfo> info = pd->getAvailableMetrics();
   unsigned size = info.size();
   for (unsigned u=0; u<size; u++)
	addMetric(info[u]);

    msg_bind_buffered(pd->get_fd(), true, (int(*)(void*))xdrrec_eof,
		     (void*) pd->net_obj());
    return (pd);
}

//  
// add a new daemon, unless a daemon is already running on that machine
// with the same machine, login, and program
//
bool paradynDaemon::getDaemon (const string &machine, 
			       const string &login,
			       const string &name){

    if (!getDaemonHelper(machine, login, name)){
        return false;
    }
    return true;
}

//
// define a new entry for the daemon dictionary, or change an existing entry
//
bool paradynDaemon::defineDaemon (const char *c, const char *d,
				  const char *l, const char *n,
				  const char *m, const char *f) {

  if(!n || !c)
      return false;
  string command = c;
  string dir = d;
  string login = l;
  string name = n;
  string machine = m;
  string flavor = f;

  daemonEntry *newE;
  for(unsigned i=0; i < allEntries.size(); i++){
      newE = allEntries[i];
      if(newE->getNameString() == name){
          if (newE->setAll(machine, command, name, login, dir, flavor))
              return true;
          else 
              return false;
      }
  }

  newE = new daemonEntry(machine, command, name, login, dir, flavor);
  if (!newE)
      return false;
  allEntries += newE;
  return true;
}


daemonEntry *paradynDaemon::findEntry(const string &, 
				      const string &n) {

    // if (!n) return ((daemonEntry*) 0);
    for(unsigned i=0; i < allEntries.size(); i++){
        daemonEntry *newE = allEntries[i];
        if(newE->getNameString() == n){
	   return(newE);
        }
    }
    return ((daemonEntry*) 0);

}

void paradynDaemon::tellDaemonsOfResource(u_int parent, u_int my_id, 
					  const char *name) {
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->addResource(parent,my_id,name);
    }

}

void paradynDaemon::printEntries()
{
    daemonEntry *entry;
    for(unsigned i=0; i < allEntries.size(); i++){
        entry = allEntries[i];
	entry->print();
    }
}

void paradynDaemon::print() 
{
  cout << "DAEMON\n";
  cout << "  name: " << name << endl;
  cout << "  command: " << command << endl;
  cout << "  login: " << login << endl;
  cout << "  machine: " << machine << endl;
  cout << "  flavor: " << flavor << endl;
}

void paradynDaemon::printDaemons()
{
    paradynDaemon *pd;
  cout << "ACTIVE DAEMONS\n";
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->print();
    }
}

void paradynDaemon::printPrograms()
{
    executable *entry;
    for(unsigned i=0; i < programs.size(); i++){
	entry = programs[i];
	cout << "PROGRAM ENTRY\n";
	cout << "pid : " << entry->pid << endl;
	entry->controlPath->print();
    }
}

//
// Return list of names of defined daemons.  
//
vector<string> *paradynDaemon::getAvailableDaemons()
{
    vector<string> *names = new vector<string>;

    daemonEntry *entry;
    for(unsigned i=0; i < allEntries.size(); i++){
        entry = allEntries[i];
	*names += entry->getName();
    }
    return(names);
    names = 0;
}

// TODO: fix this
//
// add a new executable (binary) to a program.
//
bool paradynDaemon::newExecutable(const string &machine,
				  const string &login,
				  const string &name, 
				  const string &dir, 
				  const vector<string> &argv){

  static char tmp_buf[256];

  if (! DMstatus) {
      DMstatus = new status_line("Data Manager");
  }
  if (! PROCstatus) {
      PROCstatus = new status_line("Processes");
  }

  paradynDaemon *daemon;
  if ((daemon=getDaemonHelper(machine, login, name)) == (paradynDaemon*) NULL)
      return false;

  performanceStream::ResourceBatchMode(batchStart);
  int pid = daemon->addExecutable(argv, dir, true);
  performanceStream::ResourceBatchMode(batchEnd);

  // did the application get started ok?
  if (pid > 0 && !daemon->did_error_occur()) {
      // TODO
      sprintf (tmp_buf, "%sPID=%d ", tmp_buf, pid);
      PROCstatus->message(tmp_buf);

      executable *exec = new executable(pid, argv, daemon);
      paradynDaemon::programs += exec;
      ++procRunning;
      return (true);
  } else {
      return(false);
  }
}

//
// start the programs running.
//
bool paradynDaemon::startApplication()
{
    executable *prog;
    for(unsigned i=0; i < programs.size(); i++){
	prog = programs[i];
        prog->controlPath->startProgram(prog->pid);   
    }
    return(true);
}

//
// pause all processes.
//
bool paradynDaemon::pauseAll()
{
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->pauseApplication();
    }
    // tell perf streams about change.
    performanceStream::notifyAllChange(appPaused);
    return(true);
}

//
// pause one processes.
//
bool paradynDaemon::pauseProcess(unsigned pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	if(exec->pid == pid)
	    break;
        exec = 0;
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
bool paradynDaemon::continueAll()
{
    paradynDaemon *pd;

    if (programs.size() == 0 || procRunning == 0)
	// no program to continue
       return false;

    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->continueApplication();
    }
    // tell perf streams about change.
    performanceStream::notifyAllChange(appRunning);
    return(true);
}

//
// continue one processes.
//
bool paradynDaemon::continueProcess(unsigned pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	if(exec->pid == pid)
	    break;
        exec = 0;
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
bool paradynDaemon::detachApplication(bool pause)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
	exec->controlPath->detachProgram(exec->pid,pause);
    }
    return(true);
}

//
// print the status of each process.  This is used mostly for debugging.
//
void paradynDaemon::printStatus()
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
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
void paradynDaemon::dumpCore(int pid)
{
    executable *exec = 0;
    for(unsigned i=0; i < programs.size(); i++){
	exec = programs[i];
        if ((exec->pid == (unsigned)pid) || (pid == -1)) {
	    exec->controlPath->coreProcess(exec->pid);
	    printf("found process and coreing it\n");
        }
    }
}


bool paradynDaemon::setInstSuppress(resource *res, bool newValue)
{
    bool ret = false;
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        ret |= pd->setTracking(res->getHandle(), newValue);
    }
    return(ret);
}

//
// Get the expected delay (as a fraction of the running program) for the passed
//   resource list (focus) and metric.
//
float paradynDaemon::predictedDataCost(resourceList *rl, metric *m)
{
    if (!rl || !m) return(0.0);

    vector<u_int> focus;
    assert(rl->convertToIDList(focus));

    double max = 0.0;

    const char *metName = m->getName();
    assert(metName);

    double val;
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	val = pd->getPredictedDataCost(focus, metName);
	if(val > max) max = val;
    }
    return(max);
}

float paradynDaemon::currentSmoothObsCost()
{
    double val, max = 0.0;
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        val = pd->getCurrentSmoothObsCost();
        if (val > max) max = val;
    }
    return(max);
}

//
// Start collecting data about the passed resource list (focus) and metric.
//    The returned metricInstance is used to provide a unique handle for this
//    metric/focus pair.
//
bool paradynDaemon::enableData(resourceListHandle r_handle, 
			       metricHandle m_handle,
			       metricInstance *mi) {

    resourceList *rl = resourceList::getFocus(r_handle);
    metric *m = metric::getMetric(m_handle);
    if(!rl || !m) 
	return false;

    vector<u_int> vs;
    if(!(rl->convertToIDList(vs)))
	return false;

    // 
    // for each daemon request the data to be enabled.
    //
    bool foundOne = false;
    // get a fresh copy of the TC "printChangeCollection"
    tunableBooleanConstant printChangeCollection = 
			tunableConstantRegistry::findBoolTunableConstant(
			"printChangeCollection");
        
    int id;
    paradynDaemon *pd;

    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        id = pd->enableDataCollection(vs, (const char*) m->getName(), mi->id);
        if (printChangeCollection.getValue()) {
            cout << "EDC:  " << m->getName()
   	            << rl->getName() << " " << id <<"\n";
        }
	if (id > 0 && !pd->did_error_occur()) {
	    component *comp = new component(pd, id, mi);
	    if(mi->addComponent(comp)){
		mi->addPart(&comp->sample);
            }
	    else {
               cout << "internal error in paradynDaemon::enableData" << endl;
	       abort();
	    } 
	    foundOne = true;
	}
    }

    if (foundOne) {
        mi->setEnabled();	
    }
    return foundOne;
}


bool paradynDaemon::setDefaultArgs(char *&name)
{
  if (!name)
    name = strdup("defd");
  if (name)
    return true;
  else 
    return false;
}


bool daemonEntry::setAll (const string &m, const string &c, const string &n,
			  const string &l, const string &d, const string &f)
{
  if(!n.string_of() || !c.string_of())
      return false;

  if (m.string_of()) machine = m;
  if (c.string_of()) command = c;
  if (n.string_of()) name = n;
  if (l.string_of()) login = l;
  if (d.string_of()) dir = d;
  if (d.string_of()) flavor = f;

  return true;
}
void daemonEntry::print() 
{
  cout << "DAEMON ENTRY\n";
  cout << "  name: " << name << endl;
  cout << "  command: " << command << endl;
  cout << "  dir: " << dir << endl;
  cout << "  login: " << login << endl;
  cout << "  machine: " << machine << endl;
  cout << "  flavor: " << flavor << endl;
}

int paradynDaemon::read(const void* handle, char *buf, const int len) {
  assert(0);
  int ret, ready_fd;
  assert(len > 0);
  assert((int)handle<200);
  assert((int)handle >= 0);
  static vector<unsigned> fd_vect(200);

  // must handle the msg_bind_buffered call here because xdr_read will be
  // called in the constructor for paradynDaemon, before the previous call
  // to msg_bind_buffered had been called

  if (!fd_vect[(unsigned)handle]) {
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
        if(pd->get_fd() == (int)handle)
	    break;
    }
    if (!(pd))
      return -1;

    msg_bind_buffered((int)handle, true, (int(*)(void*))xdrrec_eof,
		      (void*)(pd)->net_obj());
    fd_vect[(unsigned)handle] = 1;
  }

  do {
    unsigned tag = MSG_TAG_FILE;

    do 
      ready_fd = msg_poll(&tag, true);
    while ((ready_fd != (int) handle) && (ready_fd != THR_ERR));
    
    if (ready_fd == (int) handle) {
      errno = 0;
      ret = P_read((int)handle, buf, len);
    } else 
      return -1;
  } while (ret < 0 && errno == EINTR);

  if (ret <= 0)
    return (-1);
  else
    return ret;
}

void paradynDaemon::firstSampleCallback(int, double firstTime) {
  setEarliestFirstTime(firstTime);
}


paradynDaemon::paradynDaemon(const string &m, const string &u, const string &c,
			     const string &n, const string &f)
: dynRPCUser(m, u, c, NULL, NULL, args, false, dataManager::sock_fd),
  machine(m), login(u), command(c), name(n), flavor(f), activeMids(uiHash)
{
  if (!(this->errorConditionFound)) {
    // No problems found in order to create this new daemon process - naim
    assert(m.length());
    assert(c.length());
    assert(n.length());
    assert(f.length());

    // if c includes a pathname, lose the pathname
    const char *loc = P_strrchr(c.string_of(), '/');
    if (loc) {
      loc = loc + 1;
      command = loc;
    }
  
    status = new status_line(machine.string_of());
    paradynDaemon *pd = this;
    paradynDaemon::allDaemons+=pd;
  }
}

// machine, name, command, flavor and login are set via a callback
paradynDaemon::paradynDaemon(int f)
: dynRPCUser(f, NULL, NULL, false), flavor(0), activeMids(uiHash){
  if (!(this->errorConditionFound)) {
    // No problems found in order to create this new daemon process - naim 
    paradynDaemon *pd = this;
    paradynDaemon::allDaemons += pd;
  }
}

bool our_print_sample_arrival = false;
void printSampleArrivalCallback(bool newVal) {
   our_print_sample_arrival = newVal;
}

// batched version of sampleCallbackFunc
void paradynDaemon::batchSampleDataCallbackFunc(int ,
		vector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer)
{
    // get the earliest first time that had been reported by any paradyn
    // daemon to use as the base (0) time
    assert(getEarliestFirstTime());

    // Just for debugging:
    //fprintf(stderr, "in DMdaemon.C, burst size = %d\n", theBatchBuffer.size()) ;

    // Go through every item in the batch buffer we've just received and
    // process it.
    for (unsigned index =0; index < theBatchBuffer.size(); index++) {
	T_dyninstRPC::batch_buffer_entry &entry = theBatchBuffer[index] ; 

	unsigned mid               = entry.mid ;
	double startTimeStamp = entry.startTimeStamp ;
	double endTimeStamp   = entry.endTimeStamp ;
	double value          = entry.value ;

	startTimeStamp -= getEarliestFirstTime();
	endTimeStamp -= getEarliestFirstTime();
	if (our_print_sample_arrival) {
		cout << "mid " << mid << " " << value << " from " 
                     << startTimeStamp << " to " << endTimeStamp << "\n";
	}

	if (!activeMids.defines(mid)) {
	   // we're either gonna print a "data for disabled mid" error (in status line),
	   // or "data for unknown mid".  The latter is a fatal error.
	   bool found = false;
	   for (unsigned ve=0; ve<disabledMids.size(); ve++) {
	      if (disabledMids[ve] == mid) {
		  found = true;
		  break;
	      }
	   }

	   if (found) {
	      tunableBooleanConstant developerMode = tunableConstantRegistry::findBoolTunableConstant("developerMode");
	      bool developerModeActive = developerMode.getValue();
	      if (developerModeActive) {
		 string msg;
		 msg = string("ERROR?:data for disabled mid: ") + string(mid);
		 cout << msg.string_of() << endl;
		 DMstatus->message(P_strdup(msg.string_of()));
	      }
	      return;
	   } else {
	      cout << "ERROR: data for unknown mid: " << mid << endl;
	      uiMgr->showError (2, "");
	      exit(-1);
	   }
        }

        // Okay, the sample is not an error; let's process it.
	metricInstance *mi = activeMids[mid];
	assert(mi);

	struct sampleInterval ret;
	if (mi->components.size()){
	   // find the right component.
	   component *part = 0;

	   for(unsigned i=0; i < mi->components.size(); i++) {
	      if((unsigned)mi->components[i]->daemon == (unsigned)this)
		 part = mi->components[i];
	   }
	   if (!part) {
	      uiMgr->showError(3, "Unable to find component!!!");
	      exit(-1);
	   }

	   ret = part->sample.newValue(endTimeStamp, value);
	}

    	ret = mi->sample.newValue(mi->parts, endTimeStamp, value);

	if (ret.valid) {
	   assert(ret.end >= 0.0);
	   assert(ret.start >= 0.0);
	   assert(ret.end >= ret.start);
	   mi->enabledTime += ret.end - ret.start;

	   mi->addInterval(ret.start, ret.end, ret.value, false);
	}
    } // the main for loop
}

//
// paradyn daemon should never go away.  This represents an error state
//    due to a paradynd being killed for some reason.
//
// TODO -- handle this better
paradynDaemon::~paradynDaemon() {

#ifdef notdef
    metricInstance *mi;
    HTable<metricInstance*> curr;

    allDaemons.remove(this);

    // remove the metric ID as required.
    for (curr = activeMids; mi = *curr; curr++) {
	mi->parts.remove(this);
	mi->components.remove(this);
    }
#endif
    printf("Inconsistant state\n");
    abort();
}

//
// When an error is determined on an igen call, this function is
// called, since the default error handler will exit, and we don't
// want paradyn to exit.
//
void paradynDaemon::handle_error()
{
   removeDaemon(this, true);
}

//
// When a paradynd is started remotely, ie not by paradyn, this upcall
// reports the information for that paradynd to paradyn
//
// This must set command, name, machine and flavor fields
//
void 
paradynDaemon::reportSelf (string m, string p, int pd, string flav)
{
  setPid(pd);
  flavor = flav;
  if (!m.length() || !p.length()) {
    removeDaemon(this, true);
    printf("paradyn daemon reported bad info, removed\n");
    // error
  } else {
    machine = m.string_of();
    command = p.string_of();
    status = new status_line(machine.string_of());

    if (flavor == "pvm")
	  name = "pvmd";
    else if (flavor == "cm5")
	  name = "cm5d";
    else if (flavor == "unix")
	  name = "defd";
    else
	  name = flavor;
    }

  // Send the initial metrics, constraints, and other neato things
  mdl_send(this);
  vector<T_dyninstRPC::metricInfo> info = this->getAvailableMetrics();
  unsigned size = info.size();
  for (unsigned u=0; u<size; u++)
      addMetric(info[u]);

  return;
}

//
// When a paradynd reports status, send the status to the user
//
void 
paradynDaemon::reportStatus (string line)
{
  if (status)
    status->message(line.string_of());
}

/***
 This call is used by a daemon to report a change in the status of a process
 such as when the process exits, or stops.
 If one process stops (due to a breakpoint, for example) we stop the application.
 When one process exits, we just decrement procRunning, a counter of the number
 of processes running. If procRunning is zero, there are no more processes running,
 and the status of the application is set to appExited.
***/
void
paradynDaemon::processStatus(int pid, u_int stat) {
  if (stat == procPaused) { // process stoped
    // if one process stops, we stop the application
    pauseAll();
  } else if (stat == procExited) { // process exited
    for(unsigned i=0; i < programs.size(); i++) {
        if ((programs[i]->pid == (unsigned)pid) && programs[i]->controlPath == this) {
	  programs[i]->exited = true;
	  if (--procRunning == 0)
	    performanceStream::notifyAllChange(appExited);
	  break;
        }
    }
  }
}

/*** 
 This call is made by paradyndCM5 when it is ready to run an
 application. ParadyndCM5 is started by paradynd, when the application
 writes a MULTI_FORK. The application must then stop until the daemon
 is ready. After paradyndCM5 has read the symbol table and installed
 the initial instrumentation, the daemon calls
 nodeDaemonReadyCallback, and paradyn notifies all other daemons so that
 they can resume a procss that was waiting for the node daemon.
***/
void
paradynDaemon::nodeDaemonReadyCallback(void) {
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++) {
      paradynDaemon *pd = paradynDaemon::allDaemons[i];
      if (pd != this) {
	pd->nodeDaemonReady();
      }
    }
}

