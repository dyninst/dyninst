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

// $Id: aixMT.C,v 1.13 2002/10/08 22:49:45 bernat Exp $

#include <sys/pthdebug.h> // Pthread debug library
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/metricFocusNode.h"
#include "dyninstAPI/src/dyn_lwp.h"

/* Necessary functions:
  bool getLWPIDs(int **IDs_p); //caller should do a "delete [] *IDs_p"
  bool getLWPFrame(int lwp_id, Address *fp, Address *pc);

*/

/* 
 * Get the stack frame, given a (p)thread ID 
 */

Frame pdThread::getActiveFrame() {
  Frame newFrame;

  pthdb_context_t context;

  //static int init = 0;
  pthdb_session_t *session_ptr;
  //static pthdb_callbacks_t callbacks;
  unsigned ret;

  process *proc = get_proc();

  // First, see if we're on a running thread. That's much simpler
  // than doing lookup through the pthread debug library

  updateLWP();

  if (lwp) {
    // We have a kernel thread
    Frame lwpFrame = lwp->getActiveFrame();
    newFrame = Frame(lwpFrame.getPC(), lwpFrame.getFP(),
		     lwpFrame.getPID(), this, lwp, true);
  }
  else {
    // process object holds a pointer to the appropriate thread session
    session_ptr = proc->get_pthdb_session();
    
    fprintf(stderr, "pthread debug session update for getActiveFrame\n");
    ret = pthdb_session_update(*session_ptr);
    if (ret) fprintf(stderr, "update: returned %d\n", ret);
    
    pthdb_pthread_t pthreadp;
    pthread_t pthread = (unsigned) -1;
    
    ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_FIRST);
    if (ret) fprintf(stderr, "Getting first pthread data structure failed: %d\n", ret);
    
    ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
    if (ret) fprintf(stderr, "pthread translation failed: %d\n", ret); 
    
    while (pthread != (unsigned) get_tid()) {
      ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_NEXT);
      if (ret) fprintf(stderr, "next pthread: returned %d\n", ret);
      ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
      if (ret) {
	fprintf(stderr, "check pthread: returned %d\n", ret);
	return Frame();
      }
    }
    ret = pthdb_pthread_context(*session_ptr,
				pthreadp,
				&context);
    if (!ret)
      {
	// Succeeded in call
	fprintf(stderr, "Pthread is suspended at IAR 0x%x and SP 0x%x\n",
		(unsigned) context.iar, (unsigned) context.gpr[1]);
	newFrame = Frame(context.iar, context.gpr[1], 
			 proc->getPid(), this, 0, true);
      }
    else
      {
	// Process is running... but we didn't update the virtualTimer?
	// What?
	assert(0 && "Process running but virtualTimer incorrect");
      }
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

int PTHDB_read_regs(pthdb_user_t /*user*/,
		    tid_t tid,
		    unsigned long long flags,
		    pthdb_context_t * /*context*/)
{
  fprintf(stderr, "UNWRITTEN read regs, tid %d, flags %lld\n", tid, flags);
  return 1;
}

int PTHDB_write_regs(pthdb_user_t /*user*/,
		     tid_t tid,
		     unsigned long long flags,
		     pthdb_context_t */*context*/)
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

