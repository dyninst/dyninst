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


#include <iostream>
#ifdef PAPI
#include "papi.h"
#endif
#include "papiMgr.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/process.h"


bool HwEvent::enable() {
#ifdef PAPI
  return papi->enableEvent(papiIndex);
#else
  return false;
#endif
}

HwEvent::~HwEvent() {
#ifdef PAPI
  papi->removeHwEvent(eventCode);
#endif
}

#ifdef PAPI
typedef struct {
  bool ready;
  void *result;
} papi_rpc_ret;

void inferiorPapiCallback(process* /*p*/, unsigned /* rpc_id */, void* data,
                          void* result)
{
  papi_rpc_ret *ret = (papi_rpc_ret *)data;
  ret->result = result;
  ret->ready = true;
}


papiMgr::papiMgr(process *proc)
{
  int retval;

  retval = PAPI_create_eventset(&papiEventSet_);
  assert(retval == PAPI_OK);

  numHwCntrs_ = PAPI_num_hw_counters();
  numAddedEvents_ = 0;

  values_ = new long_long[numHwCntrs_];
  proc_ = proc;

  eventArray_ = new papiEvent_t*[numHwCntrs_];
  for (int i = 0; i < numHwCntrs_; i++) {
    eventArray_[i] = NULL;
  }

  papiRunning_ = false;
  papiAttached_ = false;
  assert(retval == PAPI_OK);
}

papiMgr::~papiMgr() 
{
  int retval;
  retval = PAPI_close_remote(papiEventSet_);

  assert(retval == PAPI_OK);

  delete[] values_;

  for (unsigned int i = 0; i < numHwCntrs_; i++) {
    if (eventArray_[i] != NULL) {
      delete eventArray_[i];
    }
  }

  delete[] eventArray_;
}


unsigned int papiMgr::getNumHwEvents()
{
  return numHwCntrs_;
}

void papiMgr::enableSampling() 
{
  int retval;

  if (!papiAttached_) {

    processState stat = proc_->status();
    if (stat == running) {
      proc_->pause();
    }
    retval = PAPI_attach_remote(papiEventSet_, proc_->getPid());
    assert(retval == PAPI_OK);
    papiAttached_ = true;

    if (stat == running) {
      proc_->continueProc();
    }
  }
}

bool papiMgr::enableEvent(int index) 
{
  bool retval;

  if (!papiAttached_) {
    enableSampling();
  }

  assert(index < numHwCntrs_);

  if (eventArray_[index]->referenceCnt > 0 && !eventArray_[index]->isActive) {

    if (papiRunning_) {
      retval = inferiorPapiStop();
      assert(retval);
    }

    retval = inferiorPapiAddEvent(eventArray_[index]->eventCode);
    assert(retval);
    eventArray_[index]->isActive = true;
  } 

  if (numAddedEvents_ > 0 && !papiRunning_) {
    retval = inferiorPapiStart();
    assert(retval);
  }

  return true;
}


bool papiMgr::enablePendingHwEvents()
{
  unsigned int i;
  bool retval;


  if (!papiAttached_) {
     enableSampling();
  }

  for (i = 0; i < numHwCntrs_; i++) {
    if (eventArray_[i] != NULL) {
      if (eventArray_[i]->referenceCnt > 0 && !eventArray_[i]->isActive) {

        if (papiRunning_) {
          retval = inferiorPapiStop();
          assert(retval);
        }

        retval = inferiorPapiAddEvent(eventArray_[i]->eventCode);
        assert(retval);
        eventArray_[i]->isActive = true;
      } 
    }
  }

  if (numAddedEvents_ > 0 && !papiRunning_) {
    retval = inferiorPapiStart();
    assert(retval);
  }
  return true;
}


int papiMgr::addHwEvent(int eventCode) 
{
  unsigned int i;
  papiEvent_t *tmp;
  int retval;
  bool restartPapi = false;

  if ( numAddedEvents_ >= numHwCntrs_ ) {
    fprintf(stderr, "paradynd error:  Only %d hardware counters available\n",
            numHwCntrs_);

    return -1;
  }

  /* check if eventCode exists.  If so, increment reference count and return */  
  for (i = 0; i < numHwCntrs_; i++) {
    if (eventArray_[i] != NULL) {
      if(eventArray_[i]->eventCode == eventCode) {
        eventArray_[i]->referenceCnt++;
        return i;
      }
    } 
  }

  /* check for available slot */
  for (i = 0; i < numHwCntrs_; i++) {
    if (eventArray_[i] == NULL)
      break;
  }

  /* PAPI must be stopped in the inferior process */
  if (papiRunning_) {
    retval = inferiorPapiStop();
    assert(retval);
    restartPapi = true;
  }

  retval = PAPI_add_event(papiEventSet_, eventCode);

  if (retval == PAPI_OK) {
    tmp = new papiEvent_t;
    tmp->referenceCnt = 1;
    tmp->eventCode = eventCode;
    tmp->isActive = false;

    eventArray_[i] = tmp;
    numAddedEvents_++;
  }
  else {
    fprintf(stderr, "paradynd error:  PAPI_add_event() failed with event code 0x%x \n",
            eventCode);
  
    i = -1;
  }

  if (restartPapi) {
    retval = inferiorPapiStart();
    assert(retval);
  }

  return i;
}


bool papiMgr::removeHwEvent(int eventCode) 
{
  unsigned int i;
  int retval;

  for (i = 0; i < numHwCntrs_; i++) {
    if (eventArray_[i] != NULL) {
      if(eventArray_[i]->eventCode == eventCode) {
        eventArray_[i]->referenceCnt--;
        if (eventArray_[i]->referenceCnt == 0) {
          if (eventArray_[i]->isActive) {
            if (papiRunning_) {
              retval = inferiorPapiStop();
              assert(retval);
            }
            /* do inferior RPC to remove event */
            if (!inferiorPapiRemoveEvent(eventCode)) {
              return false;
            }
            numAddedEvents_--;
          }

          retval = PAPI_remove_event(papiEventSet_, eventCode);
          assert (retval == PAPI_OK);
          
          delete eventArray_[i];
          eventArray_[i] = NULL;

        }
        goto success;
      }
    } 
  }

  return false;

success:

  if (numAddedEvents_ > 0 && !papiRunning_) {
    retval = inferiorPapiStart();
    assert(retval);
  }
  return true;
}



bool papiMgr::inferiorPapiAddEvent(int eventCode)
{

  /* fprintf(stderr, "MRM_DEBUG: inferiorPapiAddEvent() \n"); */

  // build AstNode for "DYNINSTpapi_add_event" call
  pdstring callee = "DYNINSTpapi_add_event";
  pdvector<AstNode*> args(1);
  args[0] = new AstNode(AstNode::Constant, (void *)eventCode);
  AstNode *code = new AstNode(callee, args);
  removeAst(args[0]);

  // issue RPC and wait for result
  papi_rpc_ret ret = { false, NULL };

  /* set lowmem to ensure there is space for inferior malloc */
  proc_->postRPCtoDo(code, true, // noCost
                     &inferiorPapiCallback, &ret,
                     true); // But use reserved memory

  extern void checkProcStatus();

  do {
    proc_->launchRPCs(proc_->status()==running);
    checkProcStatus();
  } while (!ret.ready);

  switch ((int)ret.result) {
  case 0:
#ifdef DEBUG
    sprintf(errorLine, "DYNINSTpapi_add_event() failed\n");
    logLine(errorLine);
#endif
    return false;
    break;
   case 1:
    return true;
  default:
    return false;
  }
}


bool papiMgr::inferiorPapiRemoveEvent(int eventCode)
{

  /* fprintf(stderr, "MRM_DEBUG: inferiorPapiRemoveEvent() \n"); */

  // build AstNode for "DYNINSTpapi_remove_event" call
  pdstring callee = "DYNINSTpapi_remove_event";
  pdvector<AstNode*> args(1);
  args[0] = new AstNode(AstNode::Constant, (void *)eventCode);
  AstNode *code = new AstNode(callee, args);
  removeAst(args[0]);

  // issue RPC and wait for result
  papi_rpc_ret ret = { false, NULL };

  /* set lowmem to ensure there is space for inferior malloc */
  proc_->postRPCtoDo(code, true, // noCost
                     &inferiorPapiCallback, &ret,
                     true); // But use reserved memory

  extern void checkProcStatus();

  do {
    proc_->launchRPCs(proc_->status()==running);
    checkProcStatus();
  } while (!ret.ready);

  switch ((int)ret.result) {
  case 0:
#ifdef DEBUG
    sprintf(errorLine, "DYNINSTpapi_remove_event() failed\n");
    logLine(errorLine);
#endif
    return false;
    break;
   case 1:
    return true;
  default:
    return false;
  }
}



bool papiMgr::inferiorPapiStart()
{

  /* fprintf(stderr, "MRM_DEBUG: inferiorPapiStart() \n"); */

  // build AstNode for "DYNINSTpapi_start" call
  pdstring callee = "DYNINSTpapi_start";
  pdvector<AstNode*> args(1);
  AstNode *code = new AstNode(callee, args);

  // issue RPC and wait for result
  papi_rpc_ret ret = { false, NULL };

  /* set lowmem to ensure there is space for inferior malloc */
  proc_->postRPCtoDo(code, true, // noCost
                     &inferiorPapiCallback, &ret,
                     true); // But use reserved memory

  extern void checkProcStatus();

  do {
    proc_->launchRPCs(proc_->status()==running);
    checkProcStatus();
  } while (!ret.ready);

  switch ((int)ret.result) {
  case 0:
#ifdef DEBUG
    sprintf(errorLine, "DYNINSTpapi_start() failed\n");
    logLine(errorLine);
#endif
    return false;
    break;
   case 1:
    papiRunning_ = true;
    return true;
  default:
    return false;
  }
}


bool papiMgr::inferiorPapiStop()
{

  /* fprintf(stderr, "MRM_DEBUG: inferiorPapiStop() \n");  */

  // build AstNode for "DYNINSTpapi_start" call
  pdstring callee = "DYNINSTpapi_stop";
  pdvector<AstNode*> args(1);
  AstNode *code = new AstNode(callee, args);

  // issue RPC and wait for result
  papi_rpc_ret ret = { false, NULL };

  /* set lowmem to ensure there is space for inferior malloc */
  proc_->postRPCtoDo(code, true, // noCost
                     &inferiorPapiCallback, &ret,
                     true); // But use reserved memory

  extern void checkProcStatus();

  do {
    proc_->launchRPCs(proc_->status()==running);
    checkProcStatus();
  } while (!ret.ready);

  switch ((int)ret.result) {
  case 0:
#ifdef DEBUG
    sprintf(errorLine, "DYNINSTpapi_stop() failed\n");
    logLine(errorLine);
#endif
    return false;
    break;
   case 1:
    papiRunning_ = false;
    return true;
  default:
    return false;
  }
}



int64_t papiMgr::getCurrentHwSample(int index) 
{
  int retval; 
  retval = PAPI_read_force(papiEventSet_, values_);
  assert(retval == PAPI_OK);
  return values_[index];
}

int papiMgr::getHwEventCode(pdstring& hwcntr_str) 
{
  int retval;
  int code;
  retval = PAPI_event_name_to_code(const_cast<char*>(hwcntr_str.c_str()), &code);
  
  if (retval != PAPI_OK)
    return -1;
  else
    return code;  
}

bool papiMgr::isHwStrValid(pdstring& hwcntr_str)
{
  int retval;
  int code;

  retval = PAPI_event_name_to_code(const_cast<char*>(hwcntr_str.c_str()), &code);

  if (retval != PAPI_OK)
    return false;
  else
    return true;
}
 
uint64_t papiMgr::getCurrentVirtCycles()
{
  uint64_t ret;

  if (!papiAttached_) {
     enableSampling();
  }
  ret = PAPI_get_remote_virt_cyc(papiEventSet_);
  //fprintf(stderr, "MRM_DEBUG: papiMgr::getCurrentVirtCycles()  %lld \n", ret);
  return ret;
}

HwEvent* papiMgr::createHwEvent(pdstring& hwcntr_str) 
{
  int retval;
  int code;

  /* fprintf(stderr, "MRM_DEBUG: createHwEvent() \n"); */

  retval = PAPI_event_name_to_code(const_cast<char*>(hwcntr_str.c_str()), &code);
  
  if (retval != PAPI_OK)
    return NULL;
  else {
    int index = addHwEvent(code);
    if (index < 0)
      return NULL;

    return new HwEvent(code, index, this);
  }
}

#endif

