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

#ifndef dmdaemon_H
#define dmdaemon_H
#include <string.h>
#include <stdlib.h>
#include "util/h/sys.h"
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "util/h/machineType.h"
#include "dataManager.thread.h"
#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "../UIthread/Status.h"
#include "DMinclude.h"
#include "DMresource.h"
#include "DMperfstream.h"

class metricInstance;
class metric;

//
// used to store info. about an enable request that has not received
// a response from the daemons yet
//
class DM_enableType{
    friend class paradynDaemon;
    friend class dynRPCUser;
    friend class phaseInfo;
    friend void DMenableResponse(DM_enableType&,vector<bool>&);
 public: 
    DM_enableType(perfStreamHandle ph,phaseType t,phaseHandle ph_h,
		  u_int rI,u_int cI,vector <metricInstance *> *r,
		  vector <bool> *d,vector <bool> *e,u_int h,u_int pd,u_int pc,
		  u_int ppd): ps_handle(ph),ph_type(t), ph_handle(ph_h),
		  request_id(rI), client_id(cI), request(r),done(d),enabled(e),
		  how_many(h), persistent_data(pd), persistent_collection(pc),
		  phase_persistent_data(ppd), not_all_done(0) { 
			   for(u_int i=0; i < done->size(); i++){
			       if(!(*done)[i]) not_all_done++;
			   }
    }
    DM_enableType(){ ps_handle = 0; ph_type = GlobalPhase; ph_handle= 0; 
		request_id = 0; client_id = 0; request = 0; done = 0; 
		enabled = 0; how_many =0; persistent_data = 0; 
		persistent_collection = 0; phase_persistent_data = 0; 
    }
    ~DM_enableType(){ delete request; delete done; delete enabled;}

    metricInstance *findMI(metricInstanceHandle mh);
    void setDone(metricInstanceHandle mh);

    void updateAny(vector<metricInstance *> &completed_mis,
		   vector<bool> successful);

 private:
    perfStreamHandle ps_handle;  // client thread
    phaseType ph_type;           // phase type assoc. with enable request
    phaseHandle ph_handle;       // phase id, used if request is for curr phase
    u_int request_id;            // DM assigned enable request identifier
    u_int client_id;             // enable request id sent by calling thread
    vector <metricInstance *> *request;  // MI's assoc. w/ enable request
    vector <bool> *done;         // which elements are waiting for replies
    vector <bool> *enabled;      // which elements were already enabled
    u_int how_many;              // number of daemons 
    u_int persistent_data;
    u_int persistent_collection;
    u_int phase_persistent_data;
    u_int not_all_done;
};


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
	       const string &l, const string &, const string &f) : 
	       machine(m), command(c), name(n), login(l), dir(0), 
	       flavor(f) { }
  ~daemonEntry() { }
  bool setAll(const string &m, const string &c, const string &n,
	      const string &l, const string &d, const string &f);
  void print();
  const char *getCommand() const { return command.string_of();}
  const char *getName() const { return name.string_of();}
  const char *getLogin() const { return login.string_of();}
  const char *getDir() const { return dir.string_of();}
  const char *getMachine() const { return machine.string_of();}
  const char *getFlavor() const { return flavor.string_of();}
  const string &getNameString() const { return name;}
  const string &getMachineString() const { return machine;}
  const string &getCommandString() const { return command;}
  const string &getFlavorString() const { return flavor;}

private:
  string machine;
  string command;
  string name;
  string login;
  string dir;
  string flavor;
};

//
// a binary running somewhere under the control of a paradynd*.
//
class executable {
    public:
	executable(unsigned id, const vector<string> &av, paradynDaemon *p)
		 : pid(id), argv(av), controlPath(p) { exited = false; }
	unsigned pid;
        vector<string> argv;
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
	friend void *DMmain(void* varg);
	friend void newSampleRate(float rate);
	friend bool metDoDaemon();
	friend int dataManager::DM_post_thread_create_init(int tid);
	friend void DMdoEnableData(perfStreamHandle,vector<metricRLType>*,
				 u_int,phaseType,phaseHandle,u_int,u_int,u_int);
    public:
	paradynDaemon(const string &m, const string &u, const string &c,
		      const string &n, const string &flav);
	paradynDaemon(int f); // remaining values are set via a callback
	~paradynDaemon();
	
	// replace the igen provided error handler
	virtual void handle_error();
	virtual void firstSampleCallback(int program, double firstTime);
        virtual void reportStatus(string);
	virtual void processStatus(int pid, u_int stat);
	virtual void reportSelf (string m, string p, int pd, string flav);
	virtual void batchSampleDataCallbackFunc(int program,
                               vector<T_dyninstRPC::batch_buffer_entry>);

	virtual void cpDataCallbackFunc(int, double, int, double, double);

        virtual void endOfDataCollection(int);

	double getEarliestFirstTime() const { return earliestFirstTime;}
	static void setEarliestFirstTime(double f){
            if(!earliestFirstTime) earliestFirstTime = f;
	}
        void setTimeFactor(timeStamp timef) {
            time_factor = timef;
        }
        timeStamp getTimeFactor() { return time_factor; }
        timeStamp getAdjustedTime(timeStamp time) { return time + time_factor; }

	// Not working -- would provide a read that didn't block other threads
	static int read(const void *handle, char *buf, const int len);

        // application and daemon definition functions
	static bool defineDaemon(const char *command, const char *dir,
				 const char *login, const char *name,
				 const char *machine, const char *flavor);
	static bool addRunningProgram(int pid, const vector<string> &argv, 
			              paradynDaemon *daemon);	
	static bool newExecutable(const string &machine, const string &login,
				  const string &name, const string &dir, 
				  const vector<string> &argv);
        static bool addDaemon(int new_fd);
        static bool getDaemon (const string &machine, 
			       const string &login, 
			       const string &name);
	static bool detachApplication(bool);
        static void removeDaemon(paradynDaemon *d, bool informUser);

        // application and daemon control functions
	static bool startApplication();
	static bool pauseProcess(unsigned pid);
	static bool continueProcess(unsigned pid);
  	static bool pauseAll();	
	static bool continueAll();
	static bool setInstSuppress(resource *, bool);
        static void enableData(vector<metricInstance *> *miVec, 
			       vector<bool> *done,
                               vector<bool> *enabled,
			       DM_enableType *new_entry,
			       bool need_to_enable);

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
	static vector<string> *getAvailableDaemons();
        static paradynDaemon  *machineName2Daemon(const string &theMachName);
           // returns the paradynDaemon w/ this machine name, NULL if not found.

	static void getPredictedDataCostCall(perfStreamHandle,metricHandle,
				      resourceListHandle,resourceList*,metric*,
				      u_int);
	static float currentSmoothObsCost();
        const string &getDaemonMachineName() const {return machine;}

    private:
        bool   dead;	// has there been an error on the link.
        string machine;
        string login;
        string command;
        string name;
        string flavor;
	u_int id;

        status_line *status;

        timeStamp time_factor; // for adjusting the time to account for 
                               // clock differences between daemons.

	// all active metrics ids for this daemon.
        dictionary_hash<unsigned, metricInstance*> activeMids;
        vector<unsigned> disabledMids;

        static double earliestFirstTime;

        // list of all possible daemons: currently one per unique name
        static vector<daemonEntry*> allEntries; 
	// list of all active programs
        static vector<executable*>  programs;
	// list of all active daemons: one for each unique name/machine pair 
        static vector<paradynDaemon*>  allDaemons;
        static unsigned procRunning; // how many processes are running or ready to run.
	static vector<DM_enableType*> outstanding_enables;
	static u_int next_enable_id;

        // these args are passed to the paradynd when started
        // for paradyndPVM these args contain the info to connect to the
        // "well known" socket for new paradynd's
        static vector<string> args;

	// start a daemon on a machine, if one not currently running there
        static paradynDaemon *getDaemonHelper (const string &machine,
				               const string &login,
				               const string &name);
        static daemonEntry *findEntry (const string &machine, 
				       const string &name);

        void propagateMetrics();

};
#endif
