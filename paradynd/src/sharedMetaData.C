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
 * update services, notics of latent defects, or correction of
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

// $Id: sharedMetaData.C,v

#include <iostream.h>
#include "paradynd/src/sharedMetaData.h"
#include "paradynd/src/shmMgr.h"


void sharedMetaData::mallocInShm(shmMgr &theShmMgr) {
  cookie = reinterpret_cast<unsigned *>(theShmMgr.malloc(sizeof(unsigned)));
  inferior_pid = reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  daemon_pid   = reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  observed_cost= reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  /* MT */
  virtualTimers= reinterpret_cast<tTimer *>(
			    theShmMgr.malloc(sizeof(tTimer) * maxThreads));
  posToThread  = reinterpret_cast<unsigned *>(
			    theShmMgr.malloc(sizeof(unsigned) * maxThreads));
  for(unsigned i = 0; i < maxThreads; i++) {
    pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
				    theShmMgr.malloc(perThrRpcBufferSize));
  }
  
  curBaseAddr  = reinterpret_cast<Address>(theShmMgr.getBaseAddrInDaemon());
  malloced     = true;
}

void sharedMetaData::initialize(unsigned cookie_a, int dmnPid, int appPid) {
  assert(malloced == true);
  *cookie = cookie_a;
  *inferior_pid = appPid;
  *daemon_pid = dmnPid;
  *observed_cost = 0;
}

void sharedMetaData::initializeForkedProc(unsigned cookie_a, int appPid) {
  *cookie = cookie_a;
  *inferior_pid = appPid;
  // *daemon_pid ... value copied from parent is fine
  *observed_cost = 0;
}				  

void sharedMetaData::adjustToNewBaseAddr(Address newBaseAddr) {
  assert(malloced == true);
  cookie = reinterpret_cast<unsigned *>(
               (reinterpret_cast<Address>(cookie) - curBaseAddr) + 
                newBaseAddr);
  inferior_pid = reinterpret_cast<unsigned *>(
	         (reinterpret_cast<Address>(inferior_pid) - curBaseAddr) + 
                  newBaseAddr);
  daemon_pid = reinterpret_cast<unsigned *>(
	         (reinterpret_cast<Address>(daemon_pid) - curBaseAddr) + 
                  newBaseAddr);
  observed_cost = reinterpret_cast<unsigned *>(
	         (reinterpret_cast<Address>(observed_cost) - curBaseAddr) + 
                  newBaseAddr);
  virtualTimers = reinterpret_cast<tTimer *>(
	         (reinterpret_cast<Address>(virtualTimers) - curBaseAddr) + 
                  newBaseAddr);
  posToThread = reinterpret_cast<unsigned *>(
	         (reinterpret_cast<Address>(posToThread) - curBaseAddr) + 
                  newBaseAddr);
  for (unsigned i = 0; i < maxThreads; i++) {
    pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
		 (reinterpret_cast<Address>(pendingIRPCs[i]) - curBaseAddr) +
                  newBaseAddr);      
  }
  curBaseAddr = newBaseAddr;
}

void sharedMetaData::saveOffsetsIntoRTstructure(RTsharedData_t *RTdata) {
  assert(malloced == true);
  RTdata->cookie = reinterpret_cast<unsigned *>(
	             reinterpret_cast<Address>(cookie) - curBaseAddr);
  RTdata->inferior_pid = reinterpret_cast<unsigned *>(
                     reinterpret_cast<Address>(inferior_pid) - curBaseAddr);
  RTdata->daemon_pid = reinterpret_cast<unsigned *>(
		     reinterpret_cast<Address>(daemon_pid) - curBaseAddr);
  RTdata->observed_cost = reinterpret_cast<unsigned *>(
                     reinterpret_cast<Address>(observed_cost) - curBaseAddr);
  cerr << "saving obsCost offset = " << RTdata->observed_cost << "\n";
  RTdata->virtualTimers = reinterpret_cast<tTimer *>(
                     reinterpret_cast<Address>(virtualTimers) - curBaseAddr);
  cerr << "saving vTimer offset = " << RTdata->virtualTimers << "\n";
  RTdata->posToThread = reinterpret_cast<unsigned *>(
		     reinterpret_cast<Address>(posToThread) - curBaseAddr);
  for (unsigned i = 0; i < maxThreads; i++) {
    RTdata->pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
                  reinterpret_cast<Address>(pendingIRPCs[i]) - curBaseAddr);
  }
  // We want this to be an offset from within the shared memory segment
  RTdata->pendingIRPCs = reinterpret_cast<rpcToDo **>(
		  (unsigned) RTdata->pendingIRPCs - (unsigned)curBaseAddr);

}




