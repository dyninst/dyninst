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

// $Id: aixMT.C,v 1.21 2005/02/17 21:10:34 bernat Exp $

//#include <sys/pthdebug.h> // Pthread debug library, disabled
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

/* Necessary functions:
  bool getLWPIDs(int **IDs_p); //caller should do a "delete [] *IDs_p"
  bool getLWPFrame(int lwp_id, Address *fp, Address *pc);

*/

/* 
 * Get the stack frame, given a (p)thread ID 
 */

Frame dyn_thread::getActiveFrameMT() {
  Frame newFrame;


  updateLWP();

  if (lwp) {
    // We have a kernel thread
    Frame lwpFrame = lwp->getActiveFrame();
    newFrame = Frame(lwpFrame.getPC(), lwpFrame.getFP(),
                     0, // no SP
                     lwpFrame.getPID(), lwpFrame.getProc(), this, lwp, true);
  }
  else {
#if 0 // Unsupported for now
    // process object holds a pointer to the appropriate thread session
    session_ptr = proc->get_pthdb_session();
    
    bperr( "pthread debug session update for getActiveFrame\n");
    ret = pthdb_session_update(*session_ptr);
    if (ret) bperr( "update: returned %d\n", ret);
    
    pthdb_pthread_t pthreadp;
    pthread_t pthread = (unsigned) -1;
    
    ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_FIRST);
    if (ret) bperr( "Getting first pthread data structure failed: %d\n", ret);
    
    ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
    if (ret) bperr( "pthread translation failed: %d\n", ret); 
    
    while (pthread != (unsigned) get_tid()) {
      ret = pthdb_pthread(*session_ptr, &pthreadp, PTHDB_LIST_NEXT);
      if (ret) bperr( "next pthread: returned %d\n", ret);
      ret = pthdb_pthread_ptid(*session_ptr, pthreadp, &pthread);
      if (ret) {
	bperr("check pthread: returned %d\n", ret);
	return Frame();
      }
    }
    ret = pthdb_pthread_context(*session_ptr,
				pthreadp,
				&context);
    if (!ret)
      {
	// Succeeded in call
	bperr("Pthread is suspended at IAR 0x%x and SP 0x%x\n",
		(unsigned) context.iar, (unsigned) context.gpr[1]);
	newFrame = Frame(context.iar, context.gpr[1], 
			 proc->getPid(), proc, this, 0, true);
      }
    else
      {
	// Process is running... but we didn't update the virtualTimer?
	// What?
	assert(0 && "Process running but virtualTimer incorrect");
      }
#endif
    bperr( "Error: attempt to get frame info for non-scheduled thread\n");
  }
  return newFrame;
}
