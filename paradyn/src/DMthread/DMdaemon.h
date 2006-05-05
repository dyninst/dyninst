/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: DMdaemon.h,v 1.76 2006/05/05 18:22:40 mjbrim Exp $

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
#include "dyninstRPC.mrnet.CLNT.h"
#include "../UIthread/Status.h"
#include "DMinclude.h"
#include "DMresource.h"
#include "DMperfstream.h"

#include "mrnet/MRNet.h"

class metricInstance;
class metric;


// hash functions for dictionary members
inline unsigned uiHash(const unsigned &ptr) {
    return (ptr >> 4);
}

// an entry in the daemon dictionary
// initial entries are obtained from Paradyn config files
    //	       machine(m),
class daemonEntry {

public:
  daemonEntry (){ }

  daemonEntry (const pdstring &m, const pdstring &c, const pdstring &n,
               const pdstring &l, const pdstring &, const pdstring &r,
               const pdstring &f, const pdstring &t, const pdstring &MPIt) : 
      machine(m), command(c), name(n), login(l),
      dir(""), remote_shell(r), flavor(f), mrnet_topology(t),
      MPItype(MPIt), network(NULL), leafInfo(NULL), nLeaves(0) { }

  ~daemonEntry() { }

  bool setAll(const pdstring &m, const pdstring &c, const pdstring &n,
              const pdstring &l, const pdstring &d, const pdstring &r,
              const pdstring &f, const pdstring &t,
              const pdstring & MPIt);

  void print();
  const char *getCommand() const { return command.c_str();}
  const char *getName() const { return name.c_str();}
  const char *getLogin() const { return login.c_str();}
  const char *getDir() const { return dir.c_str();}
  const char *getMachine() const { return machine.c_str();}
  const char *getFlavor() const { return flavor.c_str();}
  const char *getMRNetTopology() const { return mrnet_topology.c_str();}
  const char * getMPItype() const {return MPItype.c_str();}
  MRN::Network * getMRNetNetwork() const { return network;}
  MRN::Network::LeafInfo ** getMRNetLeafInfo() const { return leafInfo; }
  unsigned int getMRNetNumLeaves() const { return nLeaves;}

  void setMRNetNetwork(MRN::Network * n) { network=n;}
  void setMRNetLeafInfo(MRN::Network::LeafInfo ** li) { leafInfo=li; }
  void setMRNetNumLeaves(unsigned int c) { nLeaves=c;}


  const pdstring &getNameString() const { return name;}
  const pdstring &getLoginString() const { return login;}
  const pdstring &getDirString() const { return dir;}
  const pdstring &getMachineString() const { return machine;}
  const pdstring &getCommandString() const { return command;}
  const pdstring getRemoteShellString() const;
  const pdstring getMPItypeString() const {
		return MPItype;
	}
  const pdstring &getFlavorString() const { return flavor;}
  const pdstring &getMRNetTopologyString() const { return mrnet_topology;}

  void setMPItype(pdstring type){ MPItype = type;}

private:
  pdstring machine;
  pdstring command;
  pdstring name;
  pdstring login;
  pdstring dir;
  pdstring remote_shell;
  pdstring flavor;
  pdstring mrnet_topology;
  pdstring MPItype;
  MRN::Network * network;
  MRN::Network::LeafInfo **leafInfo;
  unsigned int nLeaves;
};

//
// a binary running somewhere under the control of a paradynd*.
//
class paradynDaemon;

class executable {
    public:
	executable(unsigned id, const pdvector<pdstring> &av, paradynDaemon *p)
        : pid(id), argv(av), controlPath(p) { exited = false; }
	unsigned pid;
    pdvector<pdstring> argv;
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
class processMet;
class paradynDaemon: public dynRPCUser {
   friend class dynRPCUser;
   friend class processMet;
   friend class component;
   friend class phaseInfo;
   friend class dataManager;
   friend void *DMmain(void* varg);
   friend void newSampleRate(timeLength rate);
   friend bool metDoDaemon();
   friend int dataManager::DM_post_thread_create_init( DMthreadArgs* );
   friend void DMdoEnableData(perfStreamHandle, perfStreamHandle,
			      pdvector<metricRLType>*, u_int, phaseType,
			      phaseHandle,u_int,u_int,u_int);
   friend class metricFocusReq_Val; // access to disabledMids
   
 public:
   static pdvector<paradynDaemon*>  allDaemons;
   static dictionary_hash<pdstring, pdvector<paradynDaemon*> > daemonsByHost;
   static dictionary_hash<unsigned, paradynDaemon* > daemonsById;

   MRN::Network * getNetwork() const { return network; }
   MRN::EndPoint * getEndPoint() const { return endpoint; }
   MRN::Communicator * getCommunicator() const { return communicator; }
   void setCommunicator(MRN::Communicator * com)
		 {
			 communicator = com;
		 }
   u_int get_id() { return id; }
   void setDaemonId(unsigned id);
   unsigned getDaemonId(unsigned id);

   const pdstring &getMachineName() const {return machine;}

   static paradynDaemon *getDaemonById(unsigned id) {
      assert(id < allDaemons.size());
      return allDaemons[id];
   }

   
   struct MPICHWrapperInfo
   {
      pdstring	filename;
      bool	isLocal;
      pdstring	remoteMachine;
      pdstring	remoteShell;
            
      MPICHWrapperInfo(void) : isLocal( true ) {}
      MPICHWrapperInfo(const pdstring& fname) : 
	 filename(fname), isLocal(true) { }
      MPICHWrapperInfo(const pdstring& fname, const pdstring& machine,
		       const pdstring& rsh)
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
   
   paradynDaemon(const pdstring &m, const pdstring &u, const pdstring &c,
		 const pdstring &r, const pdstring &n, const pdstring &flav);
   paradynDaemon(PDSOCKET use_sock); // remaining values are set via a callback
   paradynDaemon(MRN::Network *, MRN::EndPoint *, pdstring &m, pdstring &l,
                 pdstring &n, pdstring &f);
   ~paradynDaemon();
   
   // replace the igen provided error handler
   virtual void handle_error();
   virtual void setDaemonStartTime(MRN::Stream *, int, double startTime);
   virtual void setInitialActualValueFE(MRN::Stream *, int mid, double initActualVal);
   virtual void reportStatus(MRN::Stream *, pdstring);
   virtual void processStatus(MRN::Stream *, int pid, u_int stat);
   virtual void reportSelf (MRN::Stream *, pdstring m, pdstring p, int pd, pdstring flav);
   virtual void batchSampleDataCallbackFunc(MRN::Stream *, int program,
				    pdvector<T_dyninstRPC::batch_buffer_entry>);
   // trace data streams
   virtual void batchTraceDataCallbackFunc(MRN::Stream *, int program,
			      pdvector<T_dyninstRPC::trace_batch_buffer_entry>);
   
   virtual void cpDataCallbackFunc(MRN::Stream *, int, double, int, double, double);
   
   virtual void endOfDataCollection(MRN::Stream *, int);

   virtual void resourceReportsDone( MRN::Stream *, int );

   virtual void retiredResource(MRN::Stream *, pdstring res);

   virtual void resourceEquivClassReportCallback(MRN::Stream *s, 
						 pdvector<T_dyninstRPC::equiv_class_entry> );
   virtual void callGraphEquivClassReportCallback(MRN::Stream *s, 
						 pdvector<T_dyninstRPC::equiv_class_entry> );


   virtual void resourceInfoCallback(MRN::Stream *, unsigned int,
                                     pdvector<pdstring> resource_name,
                                     pdstring abstr,
                                     u_int type,
                                     u_int mdlType);
   virtual void severalResourceInfoCallback(MRN::Stream *,pdvector<T_dyninstRPC::resourceInfoCallbackStruct>);
   virtual void resourceUpdateCallback(MRN::Stream *      , pdvector<pdstring> resource_name, pdvector<pdstring> display_name, pdstring abstraction);
   virtual void resourceBatchMode(MRN::Stream *,bool onNow);  

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
      //assert(time_factor>relTimeStamp::Zero());
   }
   timeLength getTimeFactor() { return time_factor; }

   timeStamp getAdjustedTime(timeStamp time) { 
		 /*
     cerr << "IN getAdjustedTime time " << time << endl;
     cerr << "IN getAdjustedTime time_factor " << time_factor << endl;
     cerr << "IN getAdjustedTime time+time_factor " << (time+time_factor) << endl;
     cerr << "IN getAdjustedTime relTimeStamp::Zero() " << relTimeStamp::Zero() << endl;
		 */
     //assert((time + time_factor) > relTimeStamp::Zero());
      return time + time_factor; 
   }
   timeLength getMaxNetworkDelay() {
      return maxNetworkDelay;
   }
   void setMaxNetworkDelay(timeLength maxNetDelay) {
      maxNetworkDelay = maxNetDelay;
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
   timeLength updateTimeAdjustment2(const int samplesToTake = 5);
   static bool updateTimeAdjustment( );
   
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
                            const char *flavor, const char *mrnet_config,
                            const char * MPItype );
   static paradynDaemon * instantiateDaemon(pdstring machine,
                                     pdstring login,
                                     pdstring name,
                                     pdstring flavor,
                                     pdstring mrnet_topology );
   static bool instantiateMRNetforMPIDaemons(daemonEntry *,
                                                       unsigned int );
   static bool instantiateDefaultDaemon(daemonEntry *,
                                        const pdvector<pdstring> * host_list);
   static bool initializeDaemon(daemonEntry *);
   static bool startMPIDaemonsandApplication(daemonEntry *, processMet* );


   // start a new program; propagate all enabled metrics to it   
   static bool addRunningProgram(int pid, const pdvector<pdstring> &paradynd_argv, 
                                 paradynDaemon *daemon,
                                 bool calledFromExec, bool isInitiallyRunning);

   // launch new process   
   static bool newExecutable(const pdstring &machineArg, 
			     const pdstring &login, const pdstring &name, 
			     const pdstring &dir,
			     const pdvector<pdstring> &argv);


   // attach to an already-running process.  cmd gives the full path to the
   // executable, used just to parse the symbol table.  the word Stub was
   // appended to the name to avoid conflict with the igen call "attach"   
   static bool attachStub(const pdstring &machine, const pdstring &user,
			  const pdstring &cmd, int pid,
			  const pdstring &daemonName,
			  int afterAttach // 0 --> as is, 1 --> pause, 2 -->run
			  );
   
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
   static bool AllMonitorDynamicCallSites(pdstring name);
   static bool setInstSuppress(resource *, bool);

   static void findMatchingDaemons(metricInstance *mi, 
				   pdvector<paradynDaemon *> *matchingDaemons);

   static void getMatchingDaemons(pdvector<metricInstance *> *miVec,
                                  pdvector<paradynDaemon *> *matching_daemons);
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
   static pdvector<pdstring> *getAvailableDaemons();

   // returns the paradynDaemon(s) w/ this machine name.
   static pdvector<paradynDaemon*> machineName2Daemon(const pdstring &mach);
   
   static void getPredictedDataCostCall(perfStreamHandle, metricHandle,
					resourceListHandle, resourceList*,
					metric*, u_int);
   static float currentSmoothObsCost();
   static daemonEntry *findEntry(const pdstring &name);

   static unsigned int num_dmns_to_report_resources;
   
 private:
    MRN::Network *network; //mrnet network to which daemon belongs
    MRN::EndPoint *endpoint; //endpoint for daemon in mrnet network
    MRN::Communicator *communicator; //endpoint for daemon in mrnet network

   bool   dead;	// has there been an error on the link.
   pdstring machine;
   pdstring login;
   pdstring command;
   pdstring name;
   pdstring flavor;
   pdstring mrnet_topology;
   u_int id;

   unsigned daemonId;

   // What to do with the process after an attach
   // 0: leave as is
   // 1: pause
   // 2: run
   // Note: the daemon handles the pause/resuming, we just
   // want to update the state in the frontend appropriately.
   int afterAttach_;

   thread_t	stid;	// tid assigned to our RPC socket
   
   pdstring status;
   
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
   static u_int countSync;
   
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
   static pdvector<pdstring> args;
   
   // start a daemon on a machine, if one not currently running there
   static paradynDaemon *getDaemon(const pdstring &machine,
                                   const pdstring &login,
                                   const pdstring &name);
   
   void propagateMetrics();
   void addProcessInfo(const pdvector<pdstring> &resource_name);
   void SendMDLFiles( MRN::Stream * stream );
};
#endif
