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

#include <sys/types.h>
#include <sys/socket.h>
#include "thread/h/thread.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMdaemon.h"
#include "paradyn/src/TCthread/tunableConst.h"
#include "paradyn/src/UIthread/Status.h"
#include "DMmetric.h"
#include "paradyn/src/met/metricExt.h"
#include "util/h/Time.h"

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


static appState applicationState = appPaused; // status of the application.


metricInstance *DM_enableType::findMI(metricInstanceHandle mh){
    for(u_int i=0; i < request->size(); i++){
         if(mh == ((*request)[i])->getHandle()){
             return ((*request)[i]);
    } }
    return 0;
}

void DM_enableType::setDone(metricInstanceHandle mh){
    for(u_int i=0; i < request->size(); i++){
        if(mh == ((*request)[i])->getHandle()){
	    (*done)[i] = true;
            return;
        } 
    }
}

// find any matching completed mids and update the done values 
// if a matching value is successfully enabled done=true, else done= false
void DM_enableType::updateAny(vector<metricInstance *> &completed_mis,
			      vector<bool> successful){

   for(u_int i=0; i < done->size(); i++){
       if(!(*done)[i]){  // try to update element
           assert(not_all_done);
           metricInstanceHandle mh = ((*request)[i])->getHandle();
           for(u_int j=0; j < completed_mis.size(); j++){
	       if(completed_mis[j]){
                   if(completed_mis[j]->getHandle() == mh){
                       if(successful[j]) (*done)[i] = true;
                       not_all_done--;
               } } 
	   }
   } }
}



// Called whenever a program is ready to run (both programs started by paradyn
// and programs forked by other programs).
// The new program inherits all enabled metrics, and if the application is 
// running the new program must run too.
// 
bool paradynDaemon::addRunningProgram (int pid,
				       const vector<string> &argv,
				       paradynDaemon *daemon)
{
    executable *exec = new executable (pid, argv, daemon);
    programs += exec;
    ++procRunning;

    daemon->propagateMetrics(daemon);

    if (applicationState == appRunning) {
      daemon->continueProcess(pid);
    }

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

//
// KLUDGE:  set socket buffer size to 64k to avoid write-write deadlock
//              between paradyn and paradynd
//
#if defined(sparc_sun_sunos4_1_3) || defined(hppa1_1_hp_hpux)
   int num_bytes =0;
   int size = sizeof(num_bytes);
   num_bytes = 32768;
   if(setsockopt(new_daemon->get_fd(),SOL_SOCKET,SO_SNDBUF,(char *)&num_bytes ,size) < 0){
      perror("setsocketopt SND_BUF error");
   }
#endif

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
      string msg;
      msg = string("paradynd has died on host <") + d->getDaemonMachineName()
	    + string(">");
      uiMgr->showError(5, P_strdup(msg.string_of()));
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

timeStamp getCurrentTime(void) {
    static double previousTime=0.0;
    struct timeval tv;
  retry:
    assert(gettimeofday(&tv, NULL) == 0); // 0 --> success; -1 --> error

    double seconds_dbl = tv.tv_sec * 1.0;
    assert(tv.tv_usec < 1000000);
    double useconds_dbl = tv.tv_usec * 1.0;

    seconds_dbl += useconds_dbl / 1000000.0;

    if (seconds_dbl < previousTime) goto retry;
    previousTime = seconds_dbl;

    return seconds_dbl;
}

// get the current time of a daemon, to adjust for clock differences.
void getDaemonTime(paradynDaemon *pd) {
  timeStamp t1 = getCurrentTime();
  timeStamp dt = pd->getTime(); // daemon time
  timeStamp t2 = getCurrentTime();
  timeStamp delay = (t2 - t1) / 2.0;
  pd->setTimeFactor(t1 - dt + delay);
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
    if (!def) {
	if (name.length()) {
	  string msg;
	  msg = string("Paradyn daemon \"") + name + string("\" not defined.");
	  uiMgr->showError(90,P_strdup(msg.string_of()));
        }
	else {
	  uiMgr->showError(91,"");
        }
	return ((paradynDaemon*) 0);
    }

    string m = machine; 
    // fill in machine name if emtpy
    if (!m.string_of()) {
        m = default_host;
    }

    char statusLine[256];
    sprintf(statusLine, "Starting daemon on %s",m.string_of());
    uiMgr->updateStatus(DMstatus,P_strdup(statusLine));

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

#if defined(sparc_sun_sunos4_1_3) || defined(hppa1_1_hp_hpux)
   int num_bytes =0;
   int nb_size = sizeof(num_bytes);
   num_bytes = 32768;
   if(setsockopt(pd->get_fd(),SOL_SOCKET,SO_SNDBUF,(char *)&num_bytes ,nb_size) < 0){
      perror("setsocketopt SND_BUF error");
   }
#endif

    paradynDaemon::args.resize(asize);
    if (def->getFlavorString() == "cm5") {
      // if the daemon flavor is cm5, we have to wait until the node
      // daemon starts 
      uiMgr->updateStatus(DMstatus,P_strdup("Waiting for CM5 node daemon ..."));
    }
     else  
       uiMgr->updateStatus(DMstatus,P_strdup("ready"));

    if (pd->get_fd() < 0) {
        uiMgr->showError (6, "");
        return((paradynDaemon*) 0);
    }

   // Send the initial metrics, constraints, and other neato things
   mdl_send(pd);
   // Send the initial metrics, constraints, and other neato things
   vector<T_dyninstRPC::metricInfo> info = pd->getAvailableMetrics();
   unsigned size = info.size();
   for (unsigned u=0; u<size; u++)
	addMetric(info[u]);

    getDaemonTime(pd);

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
					  const char *name, unsigned type) {
    paradynDaemon *pd;
    for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
        pd = paradynDaemon::allDaemons[i];
	pd->addResource(parent,my_id,name, type);
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

// For a given machine name, find the appropriate paradynd structure.
// Returns NULL if an appropriate matching entry can't be found.
paradynDaemon *paradynDaemon::machineName2Daemon(const string &theMachineName) {
   for (unsigned i=0; i < allDaemons.size(); i++) {
      paradynDaemon *theDaemon = allDaemons[i];
      if (theDaemon->getDaemonMachineName() == theMachineName)
	 return theDaemon;
   }
   return 0; // failure; this machine name isn't known!
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
      uiMgr->updateStatus(PROCstatus,P_strdup(tmp_buf));
#ifdef notdef
      executable *exec = new executable(pid, argv, daemon);
      paradynDaemon::programs += exec;
      ++procRunning;
#endif
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
    applicationState = appPaused;
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
    applicationState = appRunning;
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
void paradynDaemon::getPredictedDataCostCall(perfStreamHandle ps_handle,
				      metricHandle m_handle,
				      resourceListHandle rl_handle,
				      resourceList *rl, 
				      metric *m,
				      u_int clientID)
{
    if(rl && m){
        vector<u_int> focus;
        assert(rl->convertToIDList(focus));
        const char *metName = m->getName();
        assert(metName);
        u_int requestId;
        if(performanceStream::addPredCostRequest(ps_handle,requestId,m_handle,
				rl_handle, paradynDaemon::allDaemons.size())){
            paradynDaemon *pd;
            for(unsigned i = 0; i < paradynDaemon::allDaemons.size(); i++){
                pd = paradynDaemon::allDaemons[i];
	        pd->getPredictedDataCost(ps_handle,requestId,focus, 
					 metName,clientID);
            }
	    return;
        }
    }
    // TODO: change this to do the right thing
    // this should make the response upcall to the correct calling thread
    // perfConsult->getPredictedDataCostCallbackPC(0,0.0);
    assert(0);
}

//
// make data enable request to paradynds, and add request entry to
// list of outstanding enable requests
//
void paradynDaemon::enableData(vector<metricInstance *> *miVec,
 			       vector<bool> *done,
			       vector<bool> *enabled,
			       DM_enableType *new_entry,
	                       bool need_to_enable){

    // make enable request, pass only pairs that need to be enabled to daemons
    if(need_to_enable){  
	bool whole_prog_focus = false;
	vector<paradynDaemon*> daemon_subset; // which daemons to send request
        vector<T_dyninstRPC::focusStruct> foci; 
	vector<string> metrics; 
	vector<u_int> mi_ids;  

        for(u_int i=0; i < miVec->size(); i++){
	    if(!(*enabled)[i] && !(*done)[i]){
		// create foci, metrics, and mi_ids entries for this mi
		T_dyninstRPC::focusStruct focus;
		string met_name;
		assert((*miVec)[i]->convertToIDList(focus.focus));
		met_name = (*miVec)[i]->getMetricName();
		foci += focus;
		metrics += met_name;
		mi_ids += (*miVec)[i]->getHandle();
	        // set curretly enabling flag on mi 
		(*miVec)[i]->setCurrentlyEnabling();

		// check to see if this focus is refined on the machine
		// or process heirarcy, if so then add the approp. daemon
		// to the daemon_subset, else set whole_prog_focus to true
		if(!whole_prog_focus){
		    string machine_name;
		    resourceList *rl = (*miVec)[i]->getresourceList(); 
		    assert(rl);
		    // focus is refined on machine or process heirarchy 
		    if(rl->getMachineNameReferredTo(machine_name)){
			// get the daemon corr. to this focus and add it
			// to the list of daemons
			paradynDaemon *pd = 
				paradynDaemon::machineName2Daemon(machine_name);
                        assert(pd);
			bool found = false;
			for(u_int k=0; k< daemon_subset.size(); k++){
			  if(pd->id == daemon_subset[k]->id){
			      found = true;    
			} }
			if(!found){ // add new daemon to subset list
			    daemon_subset += pd;    
			}
			pd = 0;
		    }
		    else {  // foucs is not refined on process or machine 
			whole_prog_focus = true;
		    }
		}
	} }
	assert(foci.size() == metrics.size());
	assert(metrics.size() == mi_ids.size());
	assert(daemon_subset.size() <= paradynDaemon::allDaemons.size());

        //
        // kludge to make a cm5 application request to whole program focus
	//
	string cm5_string = string("cm5");
	for(u_int k=0; k < paradynDaemon::allDaemons.size(); k++){
            if((paradynDaemon::allDaemons[k])->flavor == cm5_string){
		whole_prog_focus = true;
		break;
	} }

	// if there is a whole_prog_focus then make the request to all 
	// the daemons, else make the request to the daemon subset
	// make enable requests to all daemons
	if(whole_prog_focus) {
	    for(u_int j=0; j < paradynDaemon::allDaemons.size(); j++){
	       paradynDaemon *pd = paradynDaemon::allDaemons[j]; 
	       pd->enableDataCollection(foci,metrics,mi_ids,j,
				     new_entry->request_id);
	    }
	}
	else {  
	    // change the enable number in the entry 
	    new_entry->how_many = daemon_subset.size();
	    for(u_int j=0; j < daemon_subset.size(); j++){
	       daemon_subset[j]->enableDataCollection(foci,metrics,mi_ids,
				 daemon_subset[j]->id,new_entry->request_id);
	    }
        }
    }
    // add entry to outstanding_enables list
    paradynDaemon::outstanding_enables += new_entry;
    new_entry = 0; 
    miVec = 0;
    done = 0; 
    enabled = 0;
}


// propagateMetrics:
// called when a new process is started, to propagate all enabled metrics to
// the new process.
// Metrics are propagated only if the new process is the only process running 
// on a daemon (this is why we don't need the pid here). If there are already
// other processes running on a daemon, than it is up to the daemon to do the
// propagation (we can't do it here because the daemon has to do the aggregation).
// Calling this function has no effect if there are no metrics enabled.
void paradynDaemon::propagateMetrics(paradynDaemon *daemon) {

    vector<metricInstanceHandle> allMIHs = metricInstance::allMetricInstances.keys();

    for (unsigned i = 0; i < allMIHs.size(); i++) {

      metricInstance *mi = metricInstance::getMI(allMIHs[i]);
     
      if (mi->isEnabled()) {

	// first we must find if the daemon already has this metric enabled for
	// some process. In this case, we don't need to do anything, the
	// daemon will do the propagation by itself.
	bool found = false;
	for (unsigned j = 0; j < mi->components.size(); j++) {
	  if (mi->components[j]->getDaemon() == daemon) {
	    found = true;
	    break;
	  }
	}
	if (!found) {
	  resourceListHandle r_handle = mi->getFocusHandle();
	  metricHandle m_handle = mi->getMetricHandle();
	  resourceList *rl = resourceList::getFocus(r_handle);
	  metric *m = metric::getMetric(m_handle);

	  vector<u_int> vs;
	  assert(rl->convertToIDList(vs));

	  int id = daemon->enableDataCollection2(vs, (const char *) m->getName(), mi->id);

	  if (id > 0 && !daemon->did_error_occur()) {
	    component *comp = new component(daemon, id, mi);
	    if (mi->addComponent(comp)) {
	      //mi->addPart(&comp->sample);
	    }
	    else {
	      cout << "internal error in paradynDaemon::addRunningProgram" << endl;
	      abort();
	    }
	  }
	}
      }
    }
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
  static bool done = false;
  if (!done) {
    setEarliestFirstTime(getAdjustedTime(firstTime));
  }
  done = true;
}


paradynDaemon::paradynDaemon(const string &m, const string &u, const string &c,
			     const string &n, const string &f)
: dynRPCUser(m, u, c, NULL, NULL, args, false, dataManager::sock_fd),
  machine(m), login(u), command(c), name(n), flavor(f), activeMids(uiHash)
{
  if (!this->errorConditionFound) {
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
    id = paradynDaemon::allDaemons.size()-1;
    assert(paradynDaemon::allDaemons.size() > id); 
  }
  // else...we leave "errorConditionFound" for the caller to check...
  //        don't forget to check!
}

// machine, name, command, flavor and login are set via a callback
paradynDaemon::paradynDaemon(int f)
: dynRPCUser(f, NULL, NULL, false), flavor(0), activeMids(uiHash){
  if (!this->errorConditionFound) {
    // No problems found in order to create this new daemon process - naim 
    paradynDaemon *pd = this;
    paradynDaemon::allDaemons += pd;
    id = paradynDaemon::allDaemons.size()-1;
  }
  // else...we leave "errorConditionFound" for the caller to check...
  //        don't forget to check!
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

	unsigned mid          = entry.mid ;
	double startTimeStamp = entry.startTimeStamp ;
	double endTimeStamp   = entry.endTimeStamp ;
	double value          = entry.value ;
	u_int  weight	      = entry.weight;
	//bool   internal_metric = entry.internal_met;
	startTimeStamp = 
	    this->getAdjustedTime(startTimeStamp) - getEarliestFirstTime();
	endTimeStamp = 
	    this->getAdjustedTime(endTimeStamp) - getEarliestFirstTime();

	if (our_print_sample_arrival) {
	    cout << "mid " << mid << " " << value << " from "
	         << startTimeStamp << " to " << endTimeStamp 
		 << " weight " << weight 
		 << " machine " << machine.string_of() << "\n";
	}

        // Okay, the sample is not an error; let's process it.
	metricInstance *mi;
        bool found = activeMids.find(mid, mi);
	if (!found) {
	   // this can occur due to asynchrony of enable or disable requests
	   // so just ignore the data
	  continue;
        }
       	assert(mi);

	// Any sample sent by a daemon should not have the start time
	// less than lastSampleEnd for the aggregate sample. When a new
	// component is added to a metric, the first sample could have
	// the startTime less than lastSampleEnd. If this happens,
	// the daemon clock must be late (or the time adjustment
	// factor is not good enough), and so we must update
	// the time adjustment factor for this daemon.
	if (startTimeStamp < mi->aggSample.currentTime()) {
	  timeStamp diff = mi->aggSample.currentTime() - startTimeStamp;
	  startTimeStamp += diff;
	  endTimeStamp += diff;
	  this->setTimeFactor(this->getTimeFactor() + diff);
	  //printf("*** Adjusting time for %s: diff = %f\n", this->machine.string_of(), diff);
	}

	struct sampleInterval ret;
	if (mi->components.size()){
	   // find the right component.
	   component *part = 0;
	   for(unsigned i=0; i < mi->components.size(); i++) {
	      if((unsigned)mi->components[i]->daemon == (unsigned)this){
		 part = mi->components[i];
                 // update the weight associated with this component
		 // this does not necessarily need to be updated with
		 // each new value as long as we can distinguish between
		 // internal and non-internal metric values in some way
		 // (internal metrics weight is 1 and regular metrics 
		 // weight is the number of processes for this daemon),
		 // and the weight is changed when the number of processes 
		 // changes (we are not currently doing this part)
		 //if(!internal_metric){
	         //    mi->num_procs_per_part[i] = weight;
		 //}
              }
	   }
	   if (!part) {
	      uiMgr->showError(3, "");
	      return;
	      //exit(-1);
	   }

	   // update the sampleInfo value associated with 
	   // the daemon that sent the value 
	   //
	   if (!part->sample->firstValueReceived())
	     part->sample->startTime(startTimeStamp);
	   part->sample->newValue(endTimeStamp, value, weight);
	}

	// don't aggregate if this metric is still being enabled (we may 
	// not have received replies for the enable requests from all the daemons)
	if (mi->isCurrentlyEnabling())
	  continue;

	//
	// update the metric instance sample value if there is a new
	// interval with data for all parts, otherwise this routine
	// returns false for ret.valid and the data cannot be bucketed
	// by the histograms yet (not all components have sent data for
	// this interval)
	// newValue will aggregate the parts according to mi's aggOp
	//
	ret = mi->aggSample.aggregateValues();
	
	if (ret.valid) {  // there is new data from all components 
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
// (pid no longer used --ari)
//
void 
paradynDaemon::reportSelf (string m, string p, int pd, string flav)
{
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

  getDaemonTime(this);

  if (machine == "CM5 node daemon") {
    uiMgr->updateStatus(DMstatus,P_strdup("ready"));
  }

  return;
}

//
// When a paradynd reports status, send the status to the user
//
void 
paradynDaemon::reportStatus (string line)
{
  if (status)
    uiMgr->updateStatus(status, P_strdup(line.string_of()));
}

/***
 This call is used by a daemon to report a change in the status of a process
 such as when the process exits.
 When one process exits, we just decrement procRunning, a counter of the number
 of processes running. If procRunning is zero, there are no more processes running,
 and the status of the application is set to appExited.
***/
void
paradynDaemon::processStatus(int pid, u_int stat) {
  if (stat == procExited) { // process exited
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


// Called by a daemon when there is no more data to be sent for a metric
// instance (because the processes have exited).
void
paradynDaemon::endOfDataCollection(int mid) {

    if(activeMids.defines(mid)){
        metricInstance *mi = activeMids[mid];
	assert(mi);
        assert(mi->removeComponent(this));
    }
    else{  // check if this mid is for a disabled metric 
        bool found = false;
        for (unsigned ve=0; ve<disabledMids.size(); ve++) {
            if (disabledMids[ve] == mid) {
 	        found = true;
	        break;
 	    }
        }
	if (!found) {
	    cout << "Ending data collection for unknown metric" << endl;
	    uiMgr->showError (2, "Ending data collection for unknown metric");
	}
    }
}
