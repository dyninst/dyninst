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

// $Id: solarisMT.C,v 1.4 2002/05/10 18:36:54 schendel Exp $

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/metricFocusNode.h"

int process::findLWPbyPthread(int pthread) {
  // Unimplemented
  return -1;
}

pdThread *process::createThread(
  int tid, 
  unsigned pos, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p,  
  bool bySelf)
{
  pdThread *thr;

  // creating new thread
  thr = new pdThread(this, tid, pos);
  threads += thr;

  unsigned pd_pos;
  if(!threadMap->add(tid,pd_pos)) {
    // we run out of space, so we should try to get some more - naim

    /* is this still needed ? - bhs
    if (getVariableMgr().increaseMaxNumberOfThreads()) {
      if (!threadMap->add(tid,pd_pos)) {
	// we should be able to add more threads! - naim
	assert(0);
      }
    } else {
      // we completely run out of space! - naim
      assert(0);
    }
    */
  }
  thr->update_pd_pos(pd_pos);
  thr->update_resumestate_p(resumestate_p);
  function_base *pdf ;

  if (startpc) {
    thr->update_stack_addr(stackbase) ;
    thr->update_start_pc(startpc) ;
    pdf = findFuncByAddr(startpc) ;
    thr->update_start_func(pdf) ;
  } else {
    pdf = findOneFunction("main");
    assert(pdf);
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_pc(0);
    thr->update_start_func(pdf) ;

    prstatus_t theStatus;
    if (ioctl(proc_fd, PIOCSTATUS, &theStatus) != -1) {
      thr->update_stack_addr((stackbase=(unsigned)theStatus.pr_stkbase));
    } else {
      assert(0);
    }
  }

  metricFocusNode::handleNewThread(thr);

  //  
  sprintf(errorLine,"+++++ creating new thread{%s}, pd_pos=%u, pos=%u, tid=%d, stack=0x%x, resumestate=0x%x, by[%s]\n",
	  pdf->prettyName().string_of(),
	  pd_pos,pos,
	  tid,
	  stackbase,
	  (unsigned)resumestate_p,
	  bySelf?"Self":"Parent");
  logLine(errorLine);

  return(thr);
}

//
// CALLED for mainThread
//
void process::updateThread(pdThread *thr, int tid, unsigned pos, void* resumestate_p, resource *rid) {
  unsigned pd_pos;
  assert(thr);
  thr->update_tid(tid, pos);
  assert(threadMap);
  if(!threadMap->add(tid,pd_pos)) {
    // we run out of space, so we should try to get some more - naim
    /*  bhs
    if (getVariableMgr().increaseMaxNumberOfThreads()) {
      if (!threadMap->add(tid,pd_pos)) {
        // we should be able to add more threads! - naim
        assert(0);
      }
    } else {
      // we completely run out of space! - naim
      assert(0);
    }
    */
  }
  thr->update_pd_pos(pd_pos);
  thr->update_rid(rid);
  thr->update_resumestate_p(resumestate_p);
  function_base *f_main = findOneFunction("main");
  assert(f_main);

  //unsigned addr = f_main->addr();
  //thr->update_start_pc(addr) ;
  thr->update_start_pc(0) ;
  thr->update_start_func(f_main) ;

  prstatus_t theStatus;
  if (ioctl(proc_fd, PIOCSTATUS, &theStatus) != -1) {
    thr->update_stack_addr((unsigned)theStatus.pr_stkbase);
  } else {
    assert(0);
  }

  sprintf(errorLine,"+++++ updateThread--> creating new thread{main}, pd_pos=%u, pos=%u, tid=%d, stack=0x%x, resumestate=0x%x\n",
	  pd_pos,
	  pos,
	  tid,
	  theStatus.pr_stkbase, 
	  (unsigned) resumestate_p);
  logLine(errorLine);
}

//
// CALLED from Attach
//
void process::updateThread(
  pdThread *thr, 
  int tid, 
  unsigned pos, 
  unsigned stackbase, 
  unsigned startpc, 
  void* resumestate_p) 
{
  unsigned pd_pos;
  assert(thr);
  //  
  sprintf(errorLine," updateThread(tid=%d, pos=%d, stackaddr=0x%x, startpc=0x%x)\n",
	 tid, 
	  pos, 
	  stackbase, 
	  startpc);
  logLine(errorLine);

  thr->update_tid(tid, pos);
  assert(threadMap);
  if(!threadMap->add(tid,pd_pos)) {
    // we run out of space, so we should try to get some more - naim
    /*  bhs
    if (getVariableMgr().increaseMaxNumberOfThreads()) {
      if (!threadMap->add(tid,pd_pos)) {
	// we should be able to add more threads! - naim
	assert(0);
      }
    } else {
      // we completely run out of space! - naim
      assert(0);
    }
    */
  }

  thr->update_pd_pos(pd_pos);
  thr->update_resumestate_p(resumestate_p);

  function_base *pdf;

  if(startpc) {
    thr->update_start_pc(startpc) ;
    pdf = findFuncByAddr(startpc) ;
    thr->update_start_func(pdf) ;
    thr->update_stack_addr(stackbase) ;
  } else {
    pdf = findOneFunction("main");
    assert(pdf);
    thr->update_start_pc(startpc) ;
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_func(pdf) ;

    prstatus_t theStatus;
    if (ioctl(proc_fd, PIOCSTATUS, &theStatus) != -1) {
      thr->update_stack_addr((stackbase=(unsigned)theStatus.pr_stkbase));
    } else {
      assert(0);
    }
  } //else

  sprintf(errorLine,"+++++ creating new thread{%s}, pd_pos=%u, pos=%u, tid=%d, stack=0x%xs, resumestate=0x%x\n",
	  pdf->prettyName().string_of(), pd_pos, pos, tid, stackbase, (unsigned) resumestate_p);
  logLine(errorLine);
}

void process::deleteThread(int tid)
{
  pdThread *thr=NULL;
  unsigned i;

  for (i=0;i<threads.size();i++) {
    if (threads[i]->get_tid() == tid) {
      thr = threads[i];
      break;
    }   
  }
  if (thr != NULL) {
    getVariableMgr().deleteThread(thr);
    unsigned theSize = threads.size();
    threads[i] = threads[theSize-1];
    threads.resize(theSize-1);
    delete thr;    
    sprintf(errorLine,"----- deleting thread, tid=%d, threads.size()=%d\n",tid,threads.size());
    logLine(errorLine);
  }
}

Frame pdThread::getActiveFrame() {
  // FIXME: needs to find whether we're running on an active
  // thread and pop to process::getActiveFrame(lwp) in that case.
  typedef struct {
    long    sp;
    long    pc;
    long    l1; //fsr
    long    l2; //fpu_en;
    long    l3; //g2, g3, g4
    long    l4; 
    long    l5; 
  } resumestate_t;

  Address fp = 0, pc = 0;

  process* proc = get_proc();
  resumestate_t rs ;
  if (get_start_pc() &&
      proc->readDataSpace((caddr_t) get_resumestate_p(),
			  sizeof(resumestate_t), (caddr_t) &rs, false)) {
    fp = rs.sp;
    pc = rs.pc;
  } 
  return Frame(pc, fp, proc->getPid(), this, 0, true);
}

