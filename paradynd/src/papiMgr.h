/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef papiMgr_h
#define papiMgr_h


#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"
#include "common/h/String.h"

#ifdef PAPI
#include "papi.h"
#endif

class process;
class papiMgr;


class HwEvent {

private:

  int eventCode;
  int papiIndex;
  papiMgr* papi;

public:

  HwEvent(int event, int index, papiMgr* p) : eventCode(event), papiIndex(index),
                             papi(p)  { }
  int getIndex() { return papiIndex; }

  bool enable();

  ~HwEvent();

};


typedef struct {
  int referenceCnt;
  int eventCode; 
  bool isActive;  /* pending flag */
} papiEvent_t;


class papiMgr {

 friend class HwEvent;

#ifdef PAPI

 public:

  void enableSampling();

  papiMgr(process *proc);
  ~papiMgr(); 

  unsigned int getNumHwEvents();

  unsigned int getNumHwEventsAvailable();

  bool doesHwEventExist(int eventCode);
  bool doesHwEventExist(pdstring eventName);

  int64_t getCurrentHwSample(int index);
  uint64_t getCurrentVirtCycles();

  bool enablePendingHwEvents();
  bool enableEvent(int event);

  static int getHwEventCode(pdstring& hwcntr_str);
  static bool isHwStrValid(pdstring& hwcntr_str);

  HwEvent* createHwEvent(pdstring& hwcntr_str);

 private: 

  int addHwEvent(int EventCode);
  bool removeHwEvent(int EventCode);
   
  process* proc_;
  
  long_long* values_; 

  /* each process has a PAPI eventset to keep track of state for process */
  int papiEventSet_;
  bool papiRunning_;
  bool papiAttached_;

  unsigned int numHwCntrs_;
  unsigned int numAddedEvents_;

  /*pdvector<papiEvent_t*> eventList_; */
  papiEvent_t** eventArray_;

  bool inferiorPapiAddEvent(int EventCode);
  bool inferiorPapiRemoveEvent(int EventCode);
  bool inferiorPapiStart();
  bool inferiorPapiStop();

#endif /* PAPI */

};




#endif /*papiMgr_h*/
