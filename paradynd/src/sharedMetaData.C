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

#include <iostream>
#include "paradynd/src/sharedMetaData.h"
#include "paradynd/src/shmMgr.h"

sharedMetaData::~sharedMetaData() {
  if(malloced) {
    theShmMgr.free(reinterpret_cast<Address>(cookie));
    theShmMgr.free(reinterpret_cast<Address>(inferior_pid));
    theShmMgr.free(reinterpret_cast<Address>(daemon_pid));
    theShmMgr.free(reinterpret_cast<Address>(observed_cost));
    theShmMgr.free(reinterpret_cast<Address>(virtualTimers));
    theShmMgr.free(reinterpret_cast<Address>(indexToThread));
    for(unsigned i = 0; i < maxThreads; i++) {
      theShmMgr.free(reinterpret_cast<Address>(pendingIRPCs[i]));
    }
  }
  free(pendingIRPCs);  // this is supposed to be the normal free
}

sharedMetaData::sharedMetaData(const sharedMetaData &par, shmMgr &shmMgrToUse)
   : theShmMgr(shmMgrToUse), maxThreads(par.maxThreads),
     rpcPtrBufferSize(rpcPtrSize * maxThreads)
{
   pendingIRPCs = reinterpret_cast<rpcToDo **>(malloc(rpcPtrBufferSize *
						      maxThreads));
   if(par.malloced == false) {
      malloced = false;
      return;
   }

   // cookie
   Address offset;
   offset = reinterpret_cast<Address>(par.cookie) -
            par.theShmMgr.getBaseAddrInDaemon2();
   cookie = reinterpret_cast<unsigned *>(theShmMgr.getBaseAddrInDaemon2() + 
					 offset);

   // inferior_pid
   offset = reinterpret_cast<Address>(par.inferior_pid) -
            par.theShmMgr.getBaseAddrInDaemon2();
   inferior_pid = reinterpret_cast<unsigned *>(theShmMgr.getBaseAddrInDaemon2()
					       + offset);

   // daemon_pid
   offset = reinterpret_cast<Address>(par.daemon_pid) -
            par.theShmMgr.getBaseAddrInDaemon2();
   daemon_pid = reinterpret_cast<unsigned *>(theShmMgr.getBaseAddrInDaemon2()
					     + offset);   

   // observed_cost
   offset = reinterpret_cast<Address>(par.observed_cost) - 
            par.theShmMgr.getBaseAddrInDaemon2();
   observed_cost = reinterpret_cast<unsigned *>(
			     theShmMgr.getBaseAddrInDaemon2() + offset);

   // virtualTimers
   offset = reinterpret_cast<Address>(par.virtualTimers) -
            par.theShmMgr.getBaseAddrInDaemon2();
   virtualTimers = reinterpret_cast<virtualTimer *>(
			     theShmMgr.getBaseAddrInDaemon2() + offset);

   // indexToThread
   offset = reinterpret_cast<Address>(par.indexToThread) -
            par.theShmMgr.getBaseAddrInDaemon2();
   indexToThread = reinterpret_cast<unsigned *>(
			     theShmMgr.getBaseAddrInDaemon2() + offset);

   for(unsigned i=0; i<par.maxThreads; i++) {
      // indexToThread
      offset = reinterpret_cast<Address>(par.pendingIRPCs[i]) -
	       par.theShmMgr.getBaseAddrInDaemon2();
      pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
			     theShmMgr.getBaseAddrInDaemon2() + offset);
   }
   curBaseAddr  = reinterpret_cast<Address>(theShmMgr.getBaseAddrInDaemon());
   malloced = true;
}

void sharedMetaData::mallocInShm() {
  assert(malloced == false);
  cookie = reinterpret_cast<unsigned *>(theShmMgr.malloc(sizeof(unsigned)));
  inferior_pid = reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  daemon_pid   = reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  observed_cost= reinterpret_cast<unsigned *>(
				    theShmMgr.malloc(sizeof(unsigned)));
  /* MT */
  virtualTimers= reinterpret_cast<virtualTimer *>(
			    theShmMgr.malloc(sizeof(virtualTimer) * maxThreads));
  indexToThread  = reinterpret_cast<unsigned *>(
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
  virtualTimers = reinterpret_cast<virtualTimer *>(
	         (reinterpret_cast<Address>(virtualTimers) - curBaseAddr) + 
                  newBaseAddr);
  indexToThread = reinterpret_cast<unsigned *>(
	         (reinterpret_cast<Address>(indexToThread) - curBaseAddr) + 
                  newBaseAddr);
  for (unsigned i = 0; i < maxThreads; i++) {
    pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
		 (reinterpret_cast<Address>(pendingIRPCs[i]) - curBaseAddr) +
                  newBaseAddr);      
  }
  curBaseAddr = newBaseAddr;
}

void sharedMetaData::saveOffsetsIntoRTstructure(sharedMetaOffsetData 
						*offsetData) {
  assert(malloced == true);
  RTsharedData_t *RTdata = 
    reinterpret_cast<RTsharedData_t*>(offsetData->getAddrInDaemon());

  RTdata->cookie = reinterpret_cast<unsigned *>(
	             reinterpret_cast<Address>(cookie) - curBaseAddr);
  RTdata->inferior_pid = reinterpret_cast<unsigned *>(
                     reinterpret_cast<Address>(inferior_pid) - curBaseAddr);
  RTdata->daemon_pid = reinterpret_cast<unsigned *>(
		     reinterpret_cast<Address>(daemon_pid) - curBaseAddr);
  RTdata->observed_cost = reinterpret_cast<unsigned *>(
                     reinterpret_cast<Address>(observed_cost) - curBaseAddr);
  RTdata->virtualTimers = reinterpret_cast<virtualTimer *>(
                     reinterpret_cast<Address>(virtualTimers) - curBaseAddr);
  RTdata->indexToThread = reinterpret_cast<unsigned *>(
		     reinterpret_cast<Address>(indexToThread) - curBaseAddr);
  for (unsigned i = 0; i < maxThreads; i++) {
    RTdata->pendingIRPCs[i] = reinterpret_cast<rpcToDo *>(
                  reinterpret_cast<Address>(pendingIRPCs[i]) - curBaseAddr);
  }
  // We want this to be an offset from within the shared memory segment
  RTdata->pendingIRPCs = reinterpret_cast<rpcToDo **>(
		  (unsigned) RTdata->pendingIRPCs - (unsigned)curBaseAddr);

}

sharedMetaOffsetData::sharedMetaOffsetData(shmMgr &shmMgrToUse, 
					   int maxNumThreads)
  : theShmMgr(shmMgrToUse)
{
   rtData = (RTsharedData_t *)theShmMgr.malloc(sizeof(RTsharedData_t));
   rtData->pendingIRPCs = (rpcToDo **)
      theShmMgr.malloc(sizeof(rpcToDo *) * maxNumThreads);
}

sharedMetaOffsetData::sharedMetaOffsetData(shmMgr &shmMgrToUse,
					  sharedMetaOffsetData &parentData) :
   theShmMgr(shmMgrToUse)
{
   Address rtDataOffset = reinterpret_cast<Address>(parentData.rtData) - 
                          parentData.theShmMgr.getBaseAddrInDaemon2();
   rtData = reinterpret_cast<RTsharedData_t*>(rtDataOffset + 
					   shmMgrToUse.getBaseAddrInDaemon2());

   Address irpcsOffset = 
      reinterpret_cast<Address>(parentData.rtData->pendingIRPCs) -
      parentData.theShmMgr.getBaseAddrInDaemon2();
   rtData->pendingIRPCs = reinterpret_cast<rpcToDo**>(irpcsOffset +
					  shmMgrToUse.getBaseAddrInDaemon2());
}

sharedMetaOffsetData::~sharedMetaOffsetData() {
   theShmMgr.free(reinterpret_cast<Address>(rtData->pendingIRPCs));
   theShmMgr.free(reinterpret_cast<Address>(rtData));
}


