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

// $Id: baseTable.h,v 1.7 2002/04/05 19:38:59 schendel Exp $
// The baseTable class consists of an array of superVectors. The baseTable class is
// a template class. It has a levelMap vector that keeps track of the levels (rows)
// that has been allocated (remember that we need not only an index but also a level
// in order to determine the position of a counter or timer) - naim 3/28/97

#ifndef _BASE_TABLE_H_
#define _BASE_TABLE_H_

#if !defined(i386_unknown_nt4_0)    // interface is a keyword
#pragma interface
#endif // !defined(i386_unknown_nt4_0)

#include "common/h/Vector.h"
#include "paradynd/src/superVector.h"

class process;

template <class HK, class RAW>
class baseTable {
  private:
    unsigned numberOfColumns;
    unsigned numberOfRows;
    unsigned heapNumElems;
    unsigned subHeapIndex;
    vector < superVector<HK, RAW> * > theBaseTable;
    vector < unsigned > levelMap;
    process *inferiorProcess;

    // since we don't define them, make sure they're not used:
    // (VC++ requires template members to be defined if they are declared)
    baseTable(const baseTable &)                {}
    baseTable &operator=(const baseTable &)     { return *this; }

  public:
    baseTable() : inferiorProcess(NULL) {};
    baseTable(process *proc, unsigned nColumns, unsigned nRows,
	      unsigned level, unsigned iheapNumElems, 
	      unsigned subHeapIndex);
    baseTable(const baseTable<HK,RAW> *parentSuperTable, process *proc);

    ~baseTable();

#if defined(MT_THREAD)
    void addRows(unsigned level, unsigned nRows,
		 bool calledFromBaseTableConst=false);
#else
    void addRows(unsigned level, unsigned nRows);
#endif

    bool alloc(
#if defined(MT_THREAD)
               unsigned thr_pos, const RAW &iRawValue,
#else
               const RAW &iRawValue,
#endif
	       const HK &iHouseKeepingValue,
	       unsigned &allocatedIndex,
	       unsigned &allocatedLevel,
	       bool doNotSample=false);

    void makePendingFree(
#if defined(MT_THREAD)
                         unsigned pd_pos, unsigned allocatedIndex,
#else
                         unsigned allocatedIndex,
#endif
			 unsigned allocatedLevel, 
			 const vector<Address> &trampsUsing);

    RAW *index2LocalAddr(unsigned position,
			 unsigned allocatedIndex,
			 unsigned allocatedLevel) const;

    RAW *index2InferiorAddr(unsigned position,
			    unsigned allocatedIndex,
			    unsigned allocatedLevel) const;

    HK *getHouseKeeping(unsigned position, unsigned allocatedIndex,
			unsigned allocatedLevel) const;

    void initializeHKAfterFork(unsigned allocatedIndex, 
			       unsigned allocatedLevel,
			       const HK &iHouseKeepingValue);

    void setBaseAddrInApplic(RAW *addr);

    bool doMajorSample();
    bool doMinorSample();
    
    void handleExec();
    void forkHasCompleted();
#if defined(MT_THREAD)
    void addColumns(unsigned from, unsigned to);
    void addThread(unsigned pos, unsigned pd_pos);
    void deleteThread(unsigned pos, unsigned pd_pos);
#endif
};

#endif
