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

// $Id: sharedMetaData.h,v

#ifndef SHARED_META_DATA
#define SHARED_META_DATA

#include <stdlib.h>
#include <string.h>
#include "rtinst/h/rtinst.h"

class shmMgr;
class sharedMetaOffsetData;

class sharedMetaData {
 private:
  shmMgr &theShmMgr;
   
  unsigned maxThreads;
  unsigned *cookie;
  unsigned *inferior_pid;
  unsigned *daemon_pid;
  unsigned *observed_cost;
  /* MT */
  virtualTimer *virtualTimers;
  unsigned *posToThread;

  enum { rpcSize = sizeof(rpcToDo), rpcPtrSize = sizeof(rpcToDo*),
         numRPCs = MAX_PENDING_RPC, 
	 // the size of the (per thread) buffer that contains the rpcs
	 perThrRpcBufferSize = rpcSize * numRPCs };
  // the size of the array that contains all of the pointers to the
  // buffers, where there is a buffer for each thread that stores the
  // rpcs
  const unsigned rpcPtrBufferSize;  
  rpcToDo **pendingIRPCs; /* By POS, then by index */

  bool malloced;
  Address curBaseAddr;
  sharedMetaData& operator=(const sharedMetaData &from);

 public:

  sharedMetaData(shmMgr &shmMgrToUse, unsigned maxNumberOfThreads) : 
    theShmMgr(shmMgrToUse), maxThreads(maxNumberOfThreads),
    rpcPtrBufferSize(rpcPtrSize * maxThreads), malloced(false), 
    curBaseAddr(0)
  { 
    pendingIRPCs = reinterpret_cast<rpcToDo **>(malloc(rpcPtrBufferSize *
						       maxThreads));
  }
  sharedMetaData(const sharedMetaData &par, shmMgr &shmMgrToUse);

  ~sharedMetaData();  // free the memory from shmMgr we allocated

  unsigned *getObservedCost() { return observed_cost; }
  virtualTimer *getVirtualTimers() { return virtualTimers; }
  virtualTimer &getVirtualTimer(int pos) { return virtualTimers[pos]; }
  unsigned getPosToThread(int pos) const { return posToThread[pos]; }
  void setPosToThread(int pos, unsigned v) { posToThread[pos] = v; }

  // malloc some memory for the sharedMetaData in shared memory
  void mallocInShm();
  void initialize(unsigned cookie_a, int dmnPid, int appPid);
  void initializeForkedProc(unsigned cookie_a, int appPid);

  // adjust the addresses of shmMem malloced memory to be relative to
  // a new, given base address
  void adjustToNewBaseAddr(Address newBaseAddr);

  // save the offsets of the shmMem malloced memory into a structure
  // which can be given to the run time library
  void saveOffsetsIntoRTstructure(sharedMetaOffsetData *offsetData);
};


class sharedMetaOffsetData {
  shmMgr &theShmMgr;
  RTsharedData_t *rtData;

 public:
  sharedMetaOffsetData(shmMgr &shmMgrToUse, int maxNumThreads);
  sharedMetaOffsetData(shmMgr &shmMgrToUse, sharedMetaOffsetData &parentData);

  ~sharedMetaOffsetData();

  Address getAddrInDaemon() { 
    return reinterpret_cast<Address>(rtData);
  }
};



#endif   // SHARED_META_DATA


