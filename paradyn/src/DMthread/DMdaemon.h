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

// hash functions for dictionary members
inline unsigned uiHash(const unsigned &ptr) {
  return (ptr >> 2);
}

// an entry in the daemon dictionary
// initial entries are obtained from Paradyn config files
class daemonEntry {

public:
  daemonEntry (){ }
  daemonEntry (const string &m, const string &c, const string &n,
	       const string &l, const string &d, const string &f) : 
	       machine(m), command(c), name(n), login(l), dir(0), 
	       flavor(f) { }
  ~daemonEntry() { }
  bool setAll(const string &m, const string &c, const string &n,
	      const string &l, const string &d, const string &f);
  void print();
  const char *getCommand() { return command.string_of();}
  const char *getName() { return name.string_of();}
  const char *getLogin() { return login.string_of();}
  const char *getDir() { return dir.string_of();}
  const char *getMachine() { return machine.string_of();}
  const char *getFlavor() { return flavor.string_of();}
  const string &getNameString() { return name;}
  const string &getMachineString() { return machine;}
  const string &getCommandString() { return command;}
  const string &getFlavorString() { return flavor;}

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
	friend void *DMmain(void* varg);
	friend void newSampleRate(float rate);
	friend bool metDoDaemon();
	friend int dataManager::DM_post_thread_create_init(int tid);
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
	virtual void nodeDaemonReadyCallback(void);
	
	virtual void reportSelf (string m, string p, int pd, string flav);
//	virtual void sampleDataCallbackFunc(int, int, double, 
//					    double, double);

	virtual void batchSampleDataCallbackFunc(int program,
                                                 vector<T_dyninstRPC::batch_buffer_entry>);

	virtual void cpDataCallbackFunc(int, double, int, double, double);
	double getEarliestFirstTime() const { return earliestFirstTime;}
	static void setEarliestFirstTime(double f){
            if(!earliestFirstTime) earliestFirstTime = f;
	}

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
	static bool enableData(resourceListHandle,metricHandle,metricInstance*);
	static void tellDaemonsOfResource(u_int parent,
					  u_int res,const char *name);
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
	static float predictedDataCost(resourceList*, metric*);
	static float currentSmoothObsCost();

    private:
        bool   dead;	// has there been an error on the link.
        string machine;
        string login;
        string command;
        string name;
        string flavor;

        status_line *status;

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
};
#endif
