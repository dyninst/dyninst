/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * task_support.C : code that enables paradyndPVM to function as tasker process
 *     in the pvm environment.  In pvm, a pvm daemon runs on each node of the
 *     virtual machine, where each node is a workstation.  This daemon is
 *     responsible for forking/spawning new processes, and setting up the 
 *     necessary fd's for communication.  However, paradyn must be able to 
 *     control these processes via ptrace, and paradyn must also establish
 *     a communication channel with this process for reporting data.
 *
 * The major functions of task_support are: 
 *     PDYN_startProcess(): start a process for pvm
 *     PDYN_register_as_starter(): tell pvm that this process wants to be
 *        responsible for starting processes.
 *
 */

// $Id: task_support.C,v 1.12 2003/07/15 22:47:21 schendel Exp $

extern "C" {
#include <malloc.h>
extern int pvmendtask(void);
extern int pvmputenv(char*);

}

#include "dyninstAPI/src/dyninstP.h"
#include <string.h>
#include <unistd.h>
#include "paradynd/src/pvm_support.h"
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pvm3.h>
#include <pvmsdpro.h>
#include "dyninstAPI/src/dyninst.h"
#include <assert.h>
#include "common/h/List.h"

//#define PDYN_DEBUG

static bool i_am_the_tasker = false; // true if this process is the pvm tasker.
static int pvmgs_tid; // the tid and pid of pvmgs (group server), in case we start it.
static int pvmgs_pid;


typedef struct task {
  int t_tid;
  int t_pid;
} task;

// The list of tasks that have been started by this procedure, but have
// yet to exit.
List<task*> mytasks;

static int dofork(char *path, int argc, char **argv, int nenv, char **envp);
static void task_new(int tid, int pid);

// Update mytasks to include new task information.
void task_new (int tid, int pid)
{
  task *tp;

  tp = new task;
  tp->t_tid = tid;
  tp->t_pid = pid;
  mytasks.add(tp, (void*) pid);
}

// Inform pvm that this process wants to be responsible for starting
// processes on this pvm node.
bool 
PDYN_register_as_starter()
{
  if (pvm_reg_tasker() == PvmOk) { /* register as tasker */
    i_am_the_tasker = true;
    return true;
  }
  else {
    return false;
  }
}

// Called by paradyn when a child has exited.  Paradyn will be ptracing
// this process so it will receive SIGCHLD for the process.
// This is not using any rusage information currently.
void 
PDYN_reportSIGCHLD (int pid, int exit_status)
{
  task *tp;

#ifdef PDYN_DEBUG
  pvm_perror( "in PDYN_reportSIGCHILD for pid=\n");
#endif

  if (!(tp = mytasks.find((void*) pid))) {
    // This is OK. The first process started is not in mytasks.
    //pvm_perror("PDYN_reportSIGCHILD: could not find pid.\n");
    return;
  }

  pvm_packf("%+ %d %d %d %d %d %d",
	    PvmDataFoo,
	    tp->t_tid,
	    exit_status,
	    0,  // user time (secs) 
	    0,  // user time (usecs)
	    0,  // system time (secs)
	    0); // system time (usecs)

  mytasks.remove(tp);

  // sends this message to the local pvmd
  pvm_send (0x80000000, SM_TASKX);
}

/*
 *  PDYN_startProcess handles requests to start processes for processes that
 *     will run locally.  There should be a one starter process per node on 
 *     the pvm machine.  As a result of calling addProcess, file descriptor
 *     3 on the started task will be used for communication with the paradynd.
 *     The paradynd will use some file descriptor greater than 3.
 *
 *  The start task message from pvm has the following format.
 *	SM_STTASK() {
 *		uint tid
 *		int flags
 *		string path
 *		int argc
 *		string argv[argc]
 *		int nenv
 *		string env[nenv]
 *	}
 */

bool
PDYN_startProcess()
{
  int pid;
  int i;
  int ret = 0;
  unsigned int tid;
  int flags;
  int argc;
  char **argv;
  char path[1024];
  char buf[1024];
  int nenv;      
  char **envp;    // not used

  // pvm_upkstr wants null terminated strings
  // TODO - check pvm_upkstr to see if it may exceed string boundary
  path[1023] = '\0';
  buf[1023] = '\0';

  // TODO - should this be an unsigned or a signed int
  bool aflag;
  aflag=(pvm_upkuint (&tid, 1, 1) >= 0);
  assert(aflag);
  aflag=(pvm_upkint (&flags, 1, 1) >= 0);
  assert(aflag);
  aflag=(pvm_upkstr (path) >= 0);
  assert(aflag);
  aflag=(pvm_upkint (&argc, 1, 1) >= 0);
  assert(aflag);

#ifdef PDYN_DEBUG
  pvm_perror ("in start process\n");
#endif
  argv =  new char*[argc+1];
  argv[argc] = (char *) 0;
  for (i=0; i < argc; i++)
    {
      if (pvm_upkstr (buf) < 0)
	return false;
      argv[i] = new char[strlen(buf)+1];
      strcpy(argv[i], buf);
    }

  assert(argc >= 0);
  // this is a bit of a kludge
  // if the executable path is not specified in the hostfile for the
  // pvm daemon, the file name will be relative if the file is found
  // in $HOME/pvm3/bin/$PVM_ARCH, but the prefix for $HOME will not
  // be given, so we must append it here

  if (argv[0][0] != '/') 
    {
	char *hptr = 0, *newstr;
	int slen;

        hptr = getenv("HOME");
	assert(hptr);

	slen = strlen(hptr);
	if (slen <= 0) return false;
	slen += strlen(argv[0]);
	newstr = new char[slen+2];
	strcpy(newstr, hptr);
	strcat(newstr, "/");
	strcat(newstr, argv[0]);
	hptr = argv[0];
	argv[0] = newstr;
	delete hptr;
    }

  if (pvm_upkint(&nenv, 1, 1) < 0) return false;

  envp = new char*[nenv+1];
  envp[nenv] = (char *) 0;
  for (i=0; i<nenv; ++i) {
    if (pvm_upkstr(buf) < 0) return false;
    envp[i] = new char[strlen(buf) + 1];
    strcpy (envp[i], buf);
  }

  // a request to start the group server may be received here
  // the group server should not be instrumented
  // the group server is the file "pvmgs"
  int groupStart=0;
  char *loc;
  loc = strrchr(argv[0], '/');
  if (loc) 
    loc++;
  else
    loc = argv[0];
  if (!strcmp(loc, "pvmgs"))
    groupStart = 1;

  // it is very important that the environment args are passed to addProcess
  // nevn and envp must be used in addProcess for pvm
  pdvector<pdstring> av, ev;
  for (unsigned u=0; u<(unsigned)argc; u++)
    av += argv[u];
  for (unsigned v=0; v<(unsigned)nenv; v++)
    ev += envp[v];

  if (groupStart) {
    if ((pid = dofork(argv[0], argc, argv, nenv, envp)) == -1) {
      ret = -1;
      pvm_perror("Start group server failed\n");
      pvm_packf("%+ %d %d %d %d %d %d", PvmDataFoo, tid, 0, 0, 0, 0, 0);
      pvm_send(0x80000000, SM_TASKX);
    } else {
      task_new (tid, pid);
      pvmgs_tid = tid;
      pvmgs_pid = pid;
    }
  } else if ((pid = addProcess (av, ev)) == -1) {
    ret = -1;
    pvm_perror ("Start process failed\n");
    pvm_packf("%+ %d %d %d %d %d %d", PvmDataFoo, tid, 0, 0, 0, 0, 0);
    pvm_send(0x80000000, SM_TASKX);
  } else {
    task_new (tid, pid);
    // PDYND_report_to_paradyn (pid, argc, argv);
  }
  
  // TODO -- does this still hold ?
  // don't cleanup args, since pointers to them are used from addProcess
  // ditto for env args

  if (ret == -1)
    return false;
  else 
    return true;
}

int dofork(char *path, int /*argc*/, char **argv, int nenv, char **envp)
{
  int pid;

  pid = fork();
  if (pid > 0) {
    return pid;
  } else {
    pvmendtask();
    while (nenv-- > 0)
      pvmputenv(envp[nenv]);
    execv(path, argv);
    /* execve(path, argv, envp); */
    fprintf(stderr, "dofork() aaugh, bit it\n");
    _exit(1);
  }
  return pid;
}


//
//  Clean-up and exit from pvm.
//
void PDYN_exit_pvm(void) {

  if (pvmgs_pid) {
    // kill pvmgs
    pvm_kill(pvmgs_tid);
    PDYN_reportSIGCHLD(pvmgs_pid, 0);
  }

  // unregister as the tasker
  // If we are the tasker, we should send a second pvm_reg_tasker to unregister.
  if (i_am_the_tasker) {
    pvm_reg_tasker();
  }

  pvm_exit();

}
