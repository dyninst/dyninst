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
 * excluded
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

// $Id: aixMT.C,v 1.3 2002/05/09 21:43:05 schendel Exp $

#include <sys/pthdebug.h> // Pthread debug library
#include "dyninstAPI/src/pdThread.h"
/* Necessary functions:
  bool getLWPIDs(int **IDs_p); //caller should do a "delete [] *IDs_p"
  bool getLWPFrame(int lwp_id, Address *fp, Address *pc);

  // Notify daemon of threads
  pdThread *createThread(
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p, 
    bool);
  void updateThread(pdThread *thr, int tid, unsigned pos, void* resumestate_p,
                    resource* rid) ;
  void updateThread(
    pdThread *thr, 
    int tid, 
    unsigned pos, 
    unsigned stack_addr, 
    unsigned start_pc, 
    void* resumestate_p);
  void deleteThread(int tid);

*/

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
    /*
    if (getTable().increaseMaxNumberOfThreads()) {
      if (!threadMap->add(tid,pd_pos)) {
	// we should be able to add more threads! - naim
	assert(0 && "Could not add thread");
      }
    } else {
      // we completely run out of space! - naim
      assert(0 && "Could not create more space for threads");
    }
    */
  }
  thr->update_pd_pos(pd_pos);
  thr->update_resumestate_p(resumestate_p);
  function_base *pdf ;

  if (startpc) {
    /* AIX: "start"pc is actually current pc. We could also grab and return
       the initial function (__pi_func) */
    thr->update_stack_addr(stackbase) ;
    thr->update_start_pc(startpc) ;
    pdf = findFuncByAddr(startpc) ;
    thr->update_start_func(pdf) ;
  } else {
    cerr << "createThread: zero startPC found!" << endl;
    pdf = findOneFunction("main");
    assert(pdf);
    //thr->update_start_pc(pdf->addr()) ;
    thr->update_start_pc(0);
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
  }

  cerr << "aix.C: adding thread...";
  getVariableMgr().addThread(thr);
  cerr << " done." << endl;

  sprintf(errorLine,"+++++ creating new thread{%s}, pd_pos=%u, pos=%u, tid=%d, stack=0x%x, resumestate=0x%x, by[%s]\n",
	  pdf->prettyName().string_of(), pd_pos,pos,tid,stackbase,(unsigned)resumestate_p, bySelf?"Self":"Parent");
  logLine(errorLine);

  return(thr);
}

//
// CALLED for mainThread
//
void process::updateThread(pdThread *thr, int tid, 
			   unsigned pos, void* resumestate_p, 
			   resource *rid)
{
  unsigned pd_pos;
  assert(thr);
  thr->update_tid(tid, pos);
  assert(threadMap);
  if(!threadMap->add(tid,pd_pos)) {
    // we run out of space, so we should try to get some more - naim
    /*
    if (getTable().increaseMaxNumberOfThreads()) {
      if (!threadMap->add(tid,pd_pos)) {
        // we should be able to add more threads! - naim
        assert(0 && "Could not add thread!");
      }
    } else {
      // we completely run out of space! - naim
      assert(0 && "Could not add space for new thread");
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

  /* Need stack. Got pthread debug library. Any questions? */
  /* Yeah... how do we get a stack base addr? :) */

  sprintf(errorLine,"+++++ updateThread--> creating new thread{main}, pd_pos=%u, pos=%u, tid=%d, resumestate=0x%x\n",pd_pos,pos,tid, (unsigned) resumestate_p);
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
	 tid, pos, stackbase, startpc);
  logLine(errorLine);

  thr->update_tid(tid, pos);
  assert(threadMap);
  if(!threadMap->add(tid,pd_pos)) {
    // we run out of space, so we should try to get some more - naim
    /*
    if (getTable().increaseMaxNumberOfThreads()) {
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
    thr->update_start_func(pdf);
    thr->update_stack_addr(stackbase);
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
/* 
 * Get the stack frame, given a (p)thread ID 
 */

Frame pdThread::getActiveFrame() {
  pthdb_context_t context;

  //static int init = 0;
  pthdb_session_t *session_ptr;
  //static pthdb_callbacks_t callbacks;
  unsigned ret;

  process *proc = get_proc();
  // process object holds a pointer to the appropriate thread session
  session_ptr = proc->get_pthdb_session();

  Frame newFrame;

  fprintf(stderr, "pthread debug session update for getActiveFrame\n");
  ret = pthdb_session_update(*session_ptr);
  if (ret) fprintf(stderr, "update: returned %d\n", ret);

  pthdb_pthread_t pthreadp;
  pthread_t pthread = (unsigned) -1;

  ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_FIRST);
  if (ret) fprintf(stderr, "Getting first pthread data structure failed: %d\n", ret);

  ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
  if (ret) { 
    fprintf(stderr, "pthread translation failed: %d\n", ret);
  }

  fprintf(stderr, "Looking for thread ID %d\n", get_tid());
 
  while (pthread != (unsigned) get_tid()) {
    ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_NEXT);
    if (ret) fprintf(stderr, "next pthread: returned %d\n", ret);
    ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
    if (ret) fprintf(stderr, "check pthread: returned %d\n", ret);
  }
  ret = pthdb_pthread_context(*session_ptr,
			      pthreadp,
			      &context);
  if (!ret)
    {
      // Succeeded in call
      fprintf(stderr, "Returned context data: instruction pointer = 0x%llx, stack pointer = 0x%llx, link register = 0x%llx\n",
	      context.iar, context.gpr[1], context.lr);
      newFrame = Frame(context.iar, context.gpr[1], 
		       proc->getPid(), this, 0, true);
    }
  else
    {
      // Thread is currently scheduled. Find out to which kernel lwp,
      // and pull the data from there
      fprintf(stderr, "pdThread::getActiveFrame, got ret of %d from pthread_context\n", ret);
      int lwp; 
      ret = pthdb_pthread_tid(*session_ptr, pthreadp, &lwp);
      if (ret) fprintf(stderr, "Translating pthread to lwp failed: %d\n", ret);
      fprintf(stderr, "Getting kernel thread data for thread %d, lwp %d\n",
	      get_tid(), lwp); 
      Frame lwpFrame = proc->getActiveFrame(lwp);
      newFrame = Frame(lwpFrame.getPC(), lwpFrame.getFP(),
		       lwpFrame.getPID(), this, lwp, true);
    }
  return newFrame;
}

int PTHDB_read_data(pthdb_user_t user,
		    void *buf,
		    pthdb_addr_t addr,
		    size_t len)
{
  // We hide the process pointer in the user data. Heh.
  process *p = (process *)user;
  /*
  fprintf(stderr, "read_data(0x%x, 0x%x, 0x%x, %d)\n",
	  (int) user, (int) buf, (int) addr, (int) len);
  */
  if (!p->readDataSpace((void *)addr, len, buf, false))
    fprintf(stderr, "Error reading data space\n");
  return 0;
}

int PTHDB_write_data(pthdb_user_t user,
		    void *buf,
		    pthdb_addr_t addr,
		    size_t len)
{
  // We hide the process pointer in the user data. Heh.
  process *p = (process *)user;
  /*
  fprintf(stderr, "write_data(0x%x, 0x%x, 0x%x, %d)\n",
	  (int) user, (int) buf, (int) addr, (int) len);
  */
  if (!p->writeDataSpace((void *)addr, len, buf))
    fprintf(stderr, "Error writing data space\n");
  return 0;
}

int PTHDB_read_regs(pthdb_user_t user,
		    tid_t tid,
		    unsigned long long flags,
		    pthdb_context_t * context)
{
  fprintf(stderr, "UNWRITTEN read regs, tid %d, flags %lld\n", tid, flags);
  return 1;
}

int PTHDB_write_regs(pthdb_user_t user,
		     tid_t tid,
		     unsigned long long flags,
		     pthdb_context_t *context)
{
  fprintf(stderr, "UNWRITTEN write regs, tid %d, flags %lld\n", tid, flags);
  return 1;
}
  

int PTHDB_alloc(pthdb_user_t /*user*/,
		size_t len,
		void **bufp)
{
  *bufp = malloc(len);
  /*
  fprintf(stderr, "alloc(0x%x, %d), returning 0x%x\n",
	  (int) user, (int) len, (int) *bufp);
  */
  return 0;
}

int PTHDB_dealloc(pthdb_user_t /*user*/,
		  void *buf)
{
  /*  fprintf(stderr, "dealloc(0x%x, 0x%x)\n",
	  (int) user, (int) buf);
  */
  free(buf);
  return 0;
}

int PTHDB_realloc(pthdb_user_t user,
		  void *buf,
		  size_t len,
		  void **bufp)
{
  /*
  fprintf(stderr, "realloc(0x%x, 0x%x, %d)\n",
	  (int) user, (int) buf, (int) len);
  */
  PTHDB_dealloc(user, buf);
  PTHDB_alloc(user, len, bufp);
  return 0;
}

int PTHDB_print(pthdb_user_t user, char *str)
{
  fprintf(stderr, "print(0x%x, %s)\n",
	  (int) user, str);
  return 0;
}

int process::findLWPbyPthread(int tid)
{
  pthdb_session_t *session_ptr;
  tid_t lwp = 0;
  pthdb_pthread_t pthreadp;
  session_ptr = get_pthdb_session();
  fprintf(stderr, "findLWPbyPthread, looking for kernel thread for pthread %d\n", tid);
  int ret;
  ret = pthdb_session_update(*session_ptr);
  if (ret) {
    fprintf(stderr, "update: returned %d\n", ret);
    return -1;
  }
  pthread_t pthread = 0;
    
  ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_FIRST);
  if (ret) {
    fprintf(stderr, "first pthread returned %d\n", ret);
    return -1;
  }
  ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
  if (ret) { 
    fprintf(stderr, "pthread translation failed: %d\n", ret);
    return -1;
  }
    
  while (pthread != (unsigned) tid) {
    ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_NEXT);
    if (ret) fprintf(stderr, "next pthread: returned %d\n", ret);
    ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
    if (ret) fprintf(stderr, "check pthread: returned %d\n", ret);
  }
  fprintf(stderr, "Got pthread value of %d\n", pthread);
    
  ret = pthdb_pthread_tid(*session_ptr, pthreadp, &lwp);
  if (ret) fprintf(stderr, "pthread_tid returned %d\n", ret);
  fprintf(stderr, "find LWP by pthread: returning %d\n", lwp);
  return lwp;
}
