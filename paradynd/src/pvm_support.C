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
 * $Id: pvm_support.C,v 1.8 2000/07/28 17:22:33 pcroth Exp $
 *      Supports the paradyn daemon in the PVM environment.  There are
 *      two main functions that must be supported, task management and host
 *      management.  task_support.C provides the specific task support.  
 *      host_support.C provides specific host support.  pvm_support is the 
 *      interface between that support and paradyn.  This support is provided
 *      by two functions:
 *          PDYN_handle_pvmd_message handles messages sent from the pvm daemon
 *              the two main types of messages are: task start, host add/delete
 *          PDYN_initForPVM initializes the paradyn daemon to work in a pvm
 *              environment
 */

#include "common/h/String.h"
#include "pdutil/h/rpcUtil.h"
#include "pvm_support.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <pvm3.h>
#include <pvmsdpro.h>
#define TIDPVMD 0x80000000

static char *PDYN_pvm_daemon_name;
static char **PDYN_pvm_argv = 0;
static int pvmid;
static bool start_other_paradynds (char **);
int PDYN_get_pvmd_tid();

void PDYN_goodbye(const char *msg)
{
  pvm_perror(const_cast<char*>(msg));
  pvm_exit();
}

// PDYN_initForPVM can be called in two separate cases.  The first case is
// when the process (paradyndPVM) is the first paradyn daemon started on the
// parallel virtual machine.  In this case, this process must query pvm to
// determine the location of other pvm nodes and start paradynd's on those
// nodes.  The second case is when the process has been started by the first
// paradynd from the first case.  This process does not need to attempt to
// start other paradynd's.  These two cases are distinguished by the flag
// value passed as a parameter.  This flag is 1 when it is passed from
// paradyn.  
//
// flag == 1 --> this is the local paradynd, and it must try to
//               start the other paradynd's
// flag != 1 --> this has been pvm_spawn'd and must attempt to connect
//               to the advertised socket
// argv[0] -->  the name of the daemon to start: paradynd, paradyndPVM
// machine -->  the machine that is running paradyn
// sock -->     socket number that is listening for new paradyn daemons
//
// in either case this process must register as the tasker process and hoster
bool
PDYN_initForPVM (char **argv, const string machine, int /*sock*/,
		 int flag)
{
  pvmid = pvm_mytid();
  if (!pvmid) {
      fprintf(stdout, "Unable to connect to PVM daemon, is PVM running?");
      fflush(stdout);
      return false;
  }
  pvm_setopt(PvmResvTids, 1);		/* allow reserved messages */
  pvm_setopt(PvmRoute, PvmDontRoute);	/* no freaks talking to me */

  if (flag == 1)
    {
      // change the flags in argv to 0 
      int i=0;
      while (argv[i])
	{
	  if (!strncmp(argv[i], "-l", 2)) {
	    /* this is non-heap memory */
	    /* free(argv[i]); */
	    argv[i] = strdup("-l0");
	    assert(argv[i]);
	  } else if (!strncmp(argv[i], "-v", 2)) {
	    argv[i] = strdup("-v0");
	  }
	  i++;
	}

      if (!start_other_paradynds(argv)) {
	PDYN_goodbye("Start other paradynd's failed\n");
	return false;
      } else if (!PDYN_reg_as_hoster()) {
	PDYN_goodbye("Register as hoster failed\n");
	return false;
      }

      // look for host deletes, host adds are caught by reg_as_hoster
      // TODO - do I need to do this 
      if (pvm_notify (PvmHostDelete, PDYN_HOST_DELETE, 1, &pvmid) < 0) {
	PDYN_goodbye("pvm host delete failed\n");
	return false;
      }
	
    }

  // register as starter process
  if (!PDYN_register_as_starter()) {
    PDYN_goodbye("Register as starter failed\n");
    return false;
  }

  return true;
}

// starts paradyndPVM's on all of the pvm daemons that are running on
// the parallel virtual machine.  There is no need to start one locally,
// because the process executing this code is the local paradyndPVM.
// 
// The arguments are described in PDYN_initForPVM.
//
bool
start_other_paradynds (char **argv)
{
  char *command;
  struct pvmhostinfo *hostp;
  int nhost, loop, pvmd_tid;
  int kidid, narch;

  command = argv[0];

  if (pvm_config(&nhost, &narch, &hostp) < 0) {
    PDYN_goodbye("pvm_config() failed in pvm_support.C\n");
    return false;
  }

  pvmd_tid = PDYN_get_pvmd_tid();

  PDYN_pvm_daemon_name = command;
  PDYN_pvm_argv = argv;
  if (!PDYN_pvm_argv) return false;

  for (loop=0; loop<nhost; ++loop)
    {
      if (!hostp[loop].hi_name) return false;
      int res;
      // don't start a paradyn on this machine
      if (pvmd_tid != hostp[loop].hi_tid) {
	if ((res=pvm_spawn("paradynd", argv, 1, hostp[loop].hi_name, 1, &kidid)) != 1) {
	   PDYN_goodbye("Spawn failed in start_other_paradynd\n");
	   fprintf( stderr, "Error in pvm_spawn, %d tasks started: %d\n", res, kidid );
	   return false;
	 }
      }
    }
  return true;
}

char *PDYN_daemon()
{
  return PDYN_pvm_daemon_name;
}

char **PDYN_daemon_args()
{
  return PDYN_pvm_argv;
}

// returns the pvm thread id of the local pvm daemon
// pvm has a standard where a certain value signifies the local pvm daemon for
// identification purposes.  Unfortunately, the use of this default value will
// prevent messages from being delivered to the local pvm daemon.
int
PDYN_get_pvmd_tid()
{
  static int pvmd_tid = -1;
  struct pvmtaskinfo *taskp;
  int narch;

  if (pvmd_tid != -1)
    return pvmd_tid;
  
  if ((pvm_tasks(pvm_mytid(), &narch, &taskp) < 0) || (narch < 1)) {
    PDYN_goodbye("pvm_tasks() failed in pvm_support.C\n");
    return false;
  }

  pvmd_tid = taskp[0].ti_host;
  return pvmd_tid;
}

// Called from paradynd code when select returns ready on the pvm file
// file descriptors, which implies that a pvm message is available.
// This message had better be from the pvmd.  No other processes should
// be sending a message to the paradyn daemon via the pvm file descriptors.
//
// Since we can not use select on some platforms (pvm_getfds is not implemented),
// this function polls for a message from pvmd, and handles the message if there 
// is one.
// 
bool PDYN_handle_pvmd_message()
{
  int buf, msgtag, from_tid, bytes;

  if ((buf = pvm_nrecv (-1, -1)) <= 0) {
//    PDYN_goodbye("No message in PDYN_handle_pvmd_message, should be one\n");
    return false;
  } else if (pvm_bufinfo(buf, &bytes, &msgtag, &from_tid) < 0) {
    PDYN_goodbye("pvm_bufinfo failed in PDYN_handle_pvmd_message\n");
    return false;
  }

  switch (msgtag)
    {
    case SM_STHOST:             // request for new pvmd
#ifdef PDYN_DEBUG
      fprintf(stderr, "start new host\n");
#endif
      PDYN_hoster();
      break;
    case SM_STTASK:             // request to start process
      PDYN_startProcess();
      break;
    case PDYN_HOST_DELETE:      // a pvm host has been deleted
#ifdef PDYN_DEBUG
      fprintf(stderr, "delete host\n");
#endif
      PDYN_hostDelete();
      break;
    default:
#ifdef PDYN_DEBUG
      fprintf(stderr, "unknown pvm message to paradynd\n");
#endif
      return false;
    }
  return true;
}


