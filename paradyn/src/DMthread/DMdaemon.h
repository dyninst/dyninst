/*
 * Copyright (c) 1996-2003 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: DMdaemon.h,v 1.62 2003/05/29 19:24:55 schendel Exp $

#ifndef dmdaemon_H
#define dmdaemon_H

#include <string.h>
#include <stdlib.h>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
// trace data streams
#include "pdutil/h/ByteArray.h"
#include "common/h/machineType.h"
#include "common/h/Time.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "../UIthread/Status.h"
#include "DMinclude.h"
#include "DMresource.h"
#include "DMperfstream.h"

class metricInstance;
class metric;


// hash functions for dictionary members
inline unsigned uiHash(const unsigned &ptr) {
  return (ptr >> 4);
}

// an entry in the daemon dictionary
// initial entries are obtained from Paradyn config files
class daemonEntry {

public:
  daemonEntry (){ }
  daemonEntry (const string &m, const string &c, const string &n,
	       const string &l, const string &, const string &r,
		   const string &f) : 
	       machine(m), command(c), name(n), login(l),
	       dir(0), remote_shell(r), flavor(f) { }
  ~daemonEntry() { }
  bool setAll(const string &m, const string &c, const string &n,
	      const string &l, const string &d, const string &r,
		  const string &f);
  void print();
  const char *getCommand() const { return command.c_str();}
  const char *getName() const { return name.c_str();}
  const char *getLogin() const { return login.c_str();}
  const char *getDir() const { return dir.c_str();}
  const char *getMachine() const { return machine.c_str();}
  const char *getFlavor() const { return flavor.c_str();}
  const string &getNameString() const { return name;}
  const string &getMachineString() const { return machine;}
  const string &getCommandString() const { return command;}
  const string getRemoteShellString() const;
  const string &getFlavorString() const { return flavor;}

private:
  string machine;
  string command;
  string name;
  string login;
  string dir;
  string remote_shell;
  string flavor;
};

//
// a binary running somewhere under the control of a paradynd*.
//
class executable {
    public:
	executable(unsigned id, const pdvector<string> &av, paradynDaemon *p)
		 : pid(id), argv(av), controlPath(p) { exited = false; }
	unsigned pid;
        pdvector<string> argv;
        paradynDaemon *controlPath;
	bool exited; // true if this process has exited
};

//
// A handle to a running paradynd* somewhere.
//
// the machine field should have a value EXCEPT
// if the paradynDaemon has been started via the second
// constructor, and has not called reportSelf yet (pvm daemons)
// 
// method functions are for process and daemon control as well
// as for enabling and disabling data collection
//
// IMPORTANT NOTE: whenever the paradynDaemon constructor is used, it
// is the user's responsability to check whether the new object have been
// successfully created or not (i.e. by checking the public variable
// "bool errorConditionFound" in class dynRPCUser). In this way, we allow
// the user to take the appropriate error recovery actions instead of
// executing an "assert(0)" - naim
//
class paradynDaemon: public dynRPCUser {
   friend class dynRPCUser;
   friend class component;
   friend class phaseInfo;
   friend class dataManager;
   friend void *DMmain(void* varg);
   friend void newSampleRate(timeLength rate);
   friend bool metDoDaemon();
   friend int dataManager::DM_post_thread_create_init(thread_t tid);
   friend void DMdoEnableData(perfStreamHandle, perfStreamHandle,
			      pdvector<metricRLType>*, u_int, phaseType,
			      phaseHandle,u_int,u_int,u_int);
   friend class metricFocusReq_Val; // access to disabledMids
   
  public:
   struct MPICHWrapperInfo
   {
      string	filename;
      bool	isLocal;
      string	remoteMachine;
      string	remoteShell;
            
      MPICHWrapperInfo(void) : isLocal( true ) {}
      MPICHWrapperInfo(const string& fname) : 
	 filename(fname), isLocal(true) { }
      MPICHWrapperInfo(const string& fname, const string& machine,
		       const string& rsh)
	 : filename(fname), isLocal(false), remoteMachine(machine),
	   remoteShell(rsh)  { }
      MPICHWrapperInfo(const MPICHWrapperInfo& w)
	 : filename(w.filename), isLocal(w.isLocal),
	   remoteMachine(w.remoteMachine), remoteShell(w.remoteShell) { }

      MPICHWrapperInfo& operator=(const MPICHWrapperInfo& w) {
	 if( &w != this )
	 {
	    filename = w.filename;
	    isLocal = w.isLocal;
	    remoteMachine = w.remoteMachine;
	    remoteShell = w.remoteShell;
	 }
	 return *this;
      }
   };

   static pdvector<MPICHWrapperInfo> wrappers;
   
   paradynDaemon(const string &m, const string &u, const string &c,
		 const string &r, const string &n, const string &flav);
   paradynDaemon(PDSOCKET use_sock); // remaining values are set via a callback
   ~paradynDaemon();
   
   // replace the igen provided error handler
   virtual void handle_error();
   virtual void setDaemonStartTime(int, double startTime);
   virtual void setInitialActualValueFE(int mid, double initActualVal);
   virtual void reportStatus(string);
   virtual void processStatus(int pid, u_int stat);
   virtual void reportSelf (string m, string p, int pd, string flav);
   virtual void batchSampleDataCallbackFunc(int program,
				    pdvector<T_dyninstRPC::batch_buffer_entry>);
   // trace data streams
   virtual void batchTraceDataCallbackFunc(int program,
			      pdvector<T_dyninstRPC::trace_batch_buffer_entry>);
   
   virtual void cpDataCallbackFunc(int, double, int, double, double);
   
   virtual void endOfDataCollection(int);
   virtual void retiredResource(string res);
   virtual void resourceInfoCallback(unsigned int,
                                     pdvector<string> resource_name,
                                     string abstr, u_int type);
   virtual void severalResourceInfoCallback(pdvector<T_dyninstRPC::resourceInfoCallbackStruct>);
   virtual void resourceBatchMode(bool onNow);  
   void reportResources();
   
   void getProcStats(int *numProcsForDmn, int *numProcsExited);
   
   void setStartTime(timeStamp t) {
      startTime = t;
   }
   timeStamp getStartTime() const {
      return startTime;
   }
   
   static void setEarliestStartTime(timeStamp f);
   static timeStamp getEarliestStartTime() {
      return earliestStartTime;
   }
   void setTimeFactor(timeLength timef) {
      time_factor = timef;
   }
   timeLength getTimeFactor() { return time_factor; }
   timeStamp getAdjustedTime(timeStamp time) { 
      return time + time_factor; 
   }
   timeLength getMaxNetworkDelay() {
      return maxNetworkDelay;
   }
   void setMaxNetworkDelay(timeLength maxNetDelay) {
      maxNetworkDelay = maxNetDelay;
   }
   u_int get_id() {
      return id;
   }
   // calls attemptUpdateAggDelay(timeLength) after querying the
   // network delay
   //void attemptUpdateAggDelay();
   // will set the aggDelay based on pdNetworkDelay, but only if it's
   // larger than the currently stored value
   //void attemptUpdateAggDelay(timeLength pdNetworkDelay);
   
   void calc_FE_DMN_Times(timeLength *networkDelay,
			  timeLength *timeAdjustment);
   // the value 5 for samplesToTake is rather arbitrary
   timeLength updateTimeAdjustment(const int samplesToTake = 5);
   
   thread_t	getSocketTid( void ) const	{ return stid; }

   bool isMonitoringProcess(int pid);
   
#ifdef notdef
   // Not working -- would provide a read that didn't block other threads
   static int read(const void *handle, char *buf, const int len);
#endif // notdef
   
   // application and daemon definition functions
   static bool defineDaemon(const char *command, const char *dir,
			    const char *login, const char *name,
			    const char *machine, const char *remote_shell,
			    const char *flavor);

   // start a new program; propagate all enabled metrics to it   
   static bool addRunningProgram(int pid, const pdvector<string> &paradynd_argv, 
                                 paradynDaemon *daemon,
                                 bool calledFromExec, bool isInitiallyRunning);

   // launch new process   
   static bool newExecutable(const string &machineArg, 
			     const string &login, const string &name, 
			     const string &dir, 
			     const pdvector<string> &argv);

   // attach to an already-running process.  cmd gives the full path to the
   // executable, used just to parse the symbol table.  the word Stub was
   // appended to the name to avoid conflict with the igen call "attach"   
   static bool attachStub(const string &machine, const string &user,
			  const string &cmd, int pid,
			  const string &daemonName,
			  int afterAttach // 0 --> as is, 1 --> pause, 2 -->run
			  );
   
   static bool addDaemon(PDSOCKET sock);
   static bool getDaemon (const string &machine, const string &login, 
			  const string &name);
   static bool detachApplication(bool);
   static void removeDaemon(paradynDaemon *d, bool informUser);
   
   // application and daemon control functions
   static bool startApplication();
   static bool pauseProcess(unsigned pid);
   bool continueProcess(unsigned pid);
   static bool pauseAll();	
   static bool continueAll();

   //Sends message to each daemon telling it to instrument
   //the dynamic call sites in a certain function.
   static bool AllMonitorDynamicCallSites(string name);
   static bool setInstSuppress(resource *, bool);

   static void getMatchingDaemons(pdvector<metricInstance *> *miVec,
                                  pdvector<paradynDaemon *> *matching_daemons);

   static void tellDaemonsOfResource(u_int parent,
				     u_int res,const char *name, u_int type);
   // sets the name of the daemon to use
   static bool setDefaultArgs(char *&name);
   
   // debugging and daemon info. routines 
   static void dumpCore(int pid);

   // TODO: remove these
   static void printStatus();
   static void printDaemons();
   static void printEntries();
   static void printPrograms();
   void print();
   
   static bool applicationDefined(){return(programs.size() != 0);}
   static pdvector<string> *getAvailableDaemons();

   // returns the paradynDaemon(s) w/ this machine name.
   static pdvector<paradynDaemon*> machineName2Daemon(const string &mach);
   
   static void getPredictedDataCostCall(perfStreamHandle, metricHandle,
					resourceListHandle, resourceList*,
					metric*, u_int);
   static float currentSmoothObsCost();
   const string &getMachineName() const {return machine;}
   
   static paradynDaemon *getDaemonById(unsigned id) {
      assert(id < allDaemons.size());
      return allDaemons[id];
   }

   // list of all active daemons: one for each unique name/machine pair 
   static pdvector<paradynDaemon*>  allDaemons;
    static dictionary_hash<string, pdvector<paradynDaemon*> > daemonsByHost;
   
 private:
   bool   dead;	// has there been an error on the link.
   string machine;
   string login;
   string command;
   string name;
   string flavor;
   u_int id;

   // What to do with the process after an attach
   // 0: leave as is
   // 1: pause
   // 2: run
   // Note: the daemon handles the pause/resuming, we just
   // want to update the state in the frontend appropriately.
   int afterAttach_;

   thread_t	stid;	// tid assigned to our RPC socket
   
   string status;
   
   timeLength time_factor; // for adjusting the time to account for 
                               // clock differences between daemons.
   timeLength maxNetworkDelay; // used in setting aggregation waiting time
   timeStamp startTime;
   
   // all active metrics ids for this daemon.
   dictionary_hash<unsigned, metricInstance*> activeMids;
   pdvector<unsigned> disabledMids;
   
   // used to hold responses to resourceInfoCallback
   pdvector<u_int> newResourceTempIds;
   pdvector<resourceHandle> newResourceHandles;
   pdvector<int> pidsThatAreMonitored;
   static u_int count;
   
   static timeStamp earliestStartTime;
   
   // list of all possible daemons: currently one per unique name
   static pdvector<daemonEntry*> allEntries; 
   // list of all active programs
   static pdvector<executable*>  programs;

   // how many processes are running or ready to run.
   static unsigned procRunning;

   // these args are passed to the paradynd when started for paradyndPVM
   // these args contain the info to connect to the "well known" socket for
   // new paradynd's
   static pdvector<string> args;
   
   // start a daemon on a machine, if one not currently running there
   static paradynDaemon *getDaemonHelper(const string &machine,
					 const string &login,
					 const string &name);
   static daemonEntry *findEntry(const string &machine, const string &name);
   
   void propagateMetrics();
   void addProcessInfo(const pdvector<string> &resource_name);
};
#endif
