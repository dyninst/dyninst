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


#ifndef __PROCESS_MGR__
#define __PROCESS_MGR__

#include "common/h/Vector.h"

class pd_process;
class process;

class processMgr {
   pdvector<pd_process *> procBuf;
   
 public:
   class procIter {
      int index;
      pdvector<pd_process *> *procVector;
    public:
      procIter(unsigned index_, processMgr *procMgr) :
         index(index_), procVector(&procMgr->procBuf) 
      { }
      procIter(const procIter &iter) {
         index = iter.index;
         procVector = iter.procVector;
      }
      pd_process *operator*() {
         return (*procVector)[index];
      }
      procIter operator++(int) {
         procIter result = *this;
         index++;
         return result;
      }
      procIter operator++() { // prefix
         index++;
         return *this;
      }
      procIter operator--(int) {
         procIter result = *this;
         index--;
         return result;
      }
      procIter operator--() { // prefix
         index--;
         return *this;
      }
      bool operator==(const procIter &p) {
         return (index == p.index);
      }
      bool operator!=(const procIter &p) {
         return (index != p.index);
      }
      int getIndex() { return index; }
   };
   friend class procIter;

   void addProcess(pd_process *pd_proc) {
      assert(pd_proc != NULL);
      procBuf.push_back(pd_proc);
   }

   procIter begin() { return procIter(0, this); }
   procIter end() { return procIter(procBuf.size(), this); }
   unsigned size() { return procBuf.size(); }


   void removeProcess(pd_process *p) {
      for(unsigned i=0; i<procBuf.size(); i++) {
         if(procBuf[i] == p) {
  	    procBuf.erase(i, i);
	    break;
         }
      }
   }
   
   pd_process *find_pd_process(process *dyn_proc);
   pd_process *find_pd_process(int pid);

};

// --- DON'T USE THIS (private of sorts) -----------------
extern processMgr *theProcMgr;
// -------------------------------------------------------

void initProcMgr();

inline processMgr &getProcMgr() {
   return *theProcMgr;
}



#endif

