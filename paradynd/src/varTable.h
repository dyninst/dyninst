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

// $Id: varTable.h,v 1.7 2002/12/20 07:50:07 jaw Exp $

// The varTable class consists of an array of superVectors. The varTable
// class is a template class. It has a levelMap vector that keeps track of
// the levels (rows) that has been allocated (remember that we need not only
// an index but also a level in order to determine the position of a counter
// or timer) - naim 3/28/97

#ifndef _VAR_TABLE_H_
#define _VAR_TABLE_H_

#include "common/h/Vector.h"
#include "paradynd/src/variableMgrTypes.h"

class baseVarInstance;
class threadMetFocusNode_Val;
class variableMgr;
class Frame;
class HwEvent;

class baseVarTable { 
 public:
  virtual ~baseVarTable() { }

  virtual inst_var_index allocateVar(HwEvent* hw) = 0;
    
  virtual void markVarAsSampled(inst_var_index varIndex, unsigned thrPos,
				threadMetFocusNode_Val *thrNval) = 0;
  
  virtual void markVarAsNotSampled(inst_var_index varIndex,
				   unsigned thrPos) = 0;
  
  virtual void free(inst_var_index varIndex) = 0;
			
  virtual bool doMajorSample() = 0;
  virtual bool doMinorSample() = 0;
  virtual void *shmVarDaemonAddr(inst_var_index varIndex) const = 0;
  virtual void *shmVarApplicAddr(inst_var_index varIndex) const = 0;
    
  virtual void handleExec() = 0;
  virtual void forkHasCompleted() = 0;
  virtual void deleteThread(unsigned thrPos) = 0;
  
  virtual unsigned getVarSize() = 0;
  virtual void initializeVarsAfterFork() = 0;
};

template <class HK>
class varTable : public baseVarTable {
 private:
  pdvector<baseVarInstance *> varInstanceBuf;
  variableMgr &varMgr;
  pdvector<inst_var_index> freeIndexes;

  inst_var_index getFreeIndex();

  // --- disallow these -------------------
  varTable(const varTable<HK> &parent)  : baseVarTable(parent),
    varMgr(parent.varMgr) { }
  varTable &operator=(const varTable &) { return *this; }
  // --------------------------------------
 public:
  varTable(variableMgr &varMgr_) : varMgr(varMgr_) { }
  varTable(const varTable<HK> &parent, variableMgr &vMgr);
  
  ~varTable();
  
  inst_var_index allocateVar(HwEvent* hw);
  
  void markVarAsSampled(inst_var_index varIndex, unsigned thrPos, 
			threadMetFocusNode_Val *thrNval);
  void markVarAsNotSampled(inst_var_index varIndex, unsigned thrPos);

  void *shmVarDaemonAddr(inst_var_index varIndex) const;
  void *shmVarApplicAddr(inst_var_index varIndex) const;

  void free(inst_var_index varIndex);
		
  bool doMajorSample();
  bool doMinorSample();
    
  void handleExec();
  void forkHasCompleted();
  void deleteThread(unsigned thrPos);

  unsigned getVarSize() { return sizeof(HK::initValue); }
  void initializeVarsAfterFork();
};

#endif
