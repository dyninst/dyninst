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

// $Id: superTable.h,v 1.10 2002/04/23 18:58:46 schendel Exp $
// The superTable class consists of an array of baseTable elements
// (superVectors) and it represents the ThreadTable in paradynd. The
// superTable class is the class that has contact with the outside
// world. Rows on this table are represented by superVectors. Each
// superVector has one entry for each thread (although we could have more
// entries than threads, in which case we call these entries "reserved"). In
// order to know what entry correspond to what thread, we use a hash table
// (which has a counter part in the application). The superTable has also
// different "levels". Each level is used to store a different kind of
// dataReqNode (i.e. counters, wall timers and process timers). When we add a
// new counter, we need to find first on which level or levels are we storing
// counters. Then we need to find an empty position in the
// fastInferiorHeap. Each entry of the superVector has a fastInferiorHeap
// associated with it. This structure has the mapping of data elements to
// locations in the shared memory segment. This mapping is the same for every
// entry in the superVector (i.e. for every thread), which means that when we
// add a new data element, we are adding it to every thread. The reason for
// this is that having the same offset for counters of different threads will
// make the instrumentation code a lot easier and faster.  - naim 3/26/97

// superTable
//                                      FIH-thr1   FIH-thr2   FIH-thr3      
// theCounterBaseTable   superVector    XXXXXXXXXX XXXXXXXXXX XXXXXXXXXX
// theWallTimerBaseTable superVector    XXXXXXXXXX XXXXXXXXXX XXXXXXXXXX
// theProcTimerBaseTable superVector    XXXXXXXXXX XXXXXXXXXX XXXXXXXXXX

//    note: FIH = fastInferiorHeap, X = a timer or counter variable

// The superTable contains three baseTables, one for counters, wall timers,
//    and proc timers.
// Each baseTable currently contains one superVector.  It appears possible to
//    have multiple superVectors (of the same type) per baseTable, but I'm
//    not sure of the purpose for this.  For example, it would be possible
//    for theCounterBaseTable to have two "counter" superVectors.  Or
//    for theWallTimerBaseTable to have two "timer" superVectors.
// Each superVector contains an array of fastInferiorHeaps (FIH).  In the
//    ST case, there's just one FIH per superVector.  In the MT case,
//    there's one FIH for each thread in the superVector.
// Each fastInferiorHeap points to a section of the shared memory.  This
//    section of shared is where an array of the timers/counters is stored.

#ifndef _SUPER_TABLE_H_
#define _SUPER_TABLE_H_

#include "common/h/Vector.h"
#include "paradynd/src/fastInferiorHeapHKs.h"
#include "paradynd/src/baseTable.h"
#include "paradynd/src/superTableTypes.h"

class process;
class pdThread;

class superTable {
  private:
    unsigned maxNumberOfThreads;
    unsigned maxNumberOfLevels;
    unsigned numberOfCurrentLevels;
    unsigned numberOfCounterLevels;
    unsigned numberOfWallTimerLevels;
    unsigned numberOfProcTimerLevels;
    unsigned numberOfCurrentThreads;
    unsigned currMaxNumberOfThreads;

    baseTable<intCounterHK, intCounter>  *theIntCounterBaseTable;
    baseTable<wallTimerHK, tTimer> *theWallTimerBaseTable;
    baseTable<processTimerHK, tTimer> *theProcTimerBaseTable;
    process *inferiorProcess;

    // since we don't define them, make sure they're not used:
    superTable(const superTable &);
    superTable &operator=(const superTable &);

  public:
    superTable() { 
      theIntCounterBaseTable = NULL;
      theWallTimerBaseTable = NULL;
      theProcTimerBaseTable = NULL;
      inferiorProcess = NULL;	           
    };
    superTable(process *proc,
	       unsigned maxNumberOfIntCounters,
	       unsigned maxNumberOfWallTimers,
	       unsigned maxNumberOfProcTimers,
	       unsigned i_currMaxNumberOfThreads);
    superTable(const superTable &parentSuperTable, process *proc);

    ~superTable();

    inst_var_index allocateForInstVar(inst_var_type varType);

    void createCounterVar(inst_var_type varType, inst_var_index varIndex, 
			  unsigned thrPos, const intCounter &iValue, 
			  const intCounterHK &iHKValue);
    void createWallTimerVar(inst_var_type varType, inst_var_index varIndex, 
			    unsigned thrPos, const tTimer &iValue, 
			    const wallTimerHK &iHKValue);
    void createProcTimerVar(inst_var_type varType, inst_var_index varIndex, 
			    unsigned thrPos, const tTimer &iValue, 
			    const processTimerHK &iHKValue);

    void markVarAsSampled(inst_var_type varType, inst_var_index varIndex,
			  unsigned thrPos) const;

    void markVarAsNotSampled(inst_var_type varType, inst_var_index varIndex,
			     unsigned thrPos) const;

    void makePendingFree(inst_var_type varType, inst_var_index varIndex,
			 unsigned thrPos, const vector<Address> &trampsUsing);

    void setBaseAddrInApplic(unsigned type, void *addr);

    void *index2LocalAddr(inst_var_type varType, inst_var_index varIndex,
			  unsigned thrPos) const;

    void *index2InferiorAddr(inst_var_type varType, inst_var_index varIndex,
			     unsigned thrPos) const;

    void *getHouseKeeping(inst_var_type varType, inst_var_index varIndex,
			  unsigned thrPos);

    void initializeHKAfterForkIntCounter(unsigned level,
					 unsigned varIndex,
					 const intCounterHK &iHouseKeepingValue);

    void initializeHKAfterForkWallTimer(unsigned level,
					unsigned varIndex,
					const wallTimerHK &iHouseKeepingValue);

    void initializeHKAfterForkProcTimer(unsigned level,
					unsigned varIndex,
					const processTimerHK &iHouseKeepingValue);

    // it does doMajorSample for the three types (intCounter, wallTimer and procTimer)
    bool doMajorSample();
    
    // it does doMinorSample for the three types (intCounter, wallTimer and procTimer)
    bool doMinorSample();

    void handleExec();
    void forkHasCompleted();

    unsigned getCurrentNumberOfThreads();
    unsigned getMaxSize() { return maxNumberOfThreads; }
    // only used by a MT process
    bool increaseMaxNumberOfThreads();
    void addThread(pdThread *thr);
    void deleteThread(pdThread *thr);
};

#endif
