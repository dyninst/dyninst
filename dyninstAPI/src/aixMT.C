/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: aixMT.C,v 1.24 2007/06/13 18:50:30 bernat Exp $

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
    lwpFrame.thread_ = this;
    return lwpFrame;
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
    
    while (pthread != (dynthread_t) get_tid()) {
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
  }
  return newFrame;
}
