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

// $Id: solarisMT.C,v 1.8 2002/09/17 20:08:00 bernat Exp $

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/metricFocusNode.h"

// As of solaris 8, threads are bound 1:1 with lwps. So this will always
// end up querying an lwp.

Frame pdThread::getActiveFrame() {

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

  Frame newFrame;

  process* proc = get_proc();

  int lwp = proc->findLWPbyPOS(pos);
  if (lwp > 0) {
    Frame lwpFrame = proc->getActiveFrame(lwp);
    newFrame = Frame(lwpFrame.getPC(), lwpFrame.getFP(),
		     lwpFrame.getPID(), this, lwp, true);
  }
  else {
    resumestate_t rs ;
    if (get_start_pc() &&
	proc->readDataSpace((caddr_t) get_resumestate_p(),
			    sizeof(resumestate_t), (caddr_t) &rs, false)) {
      fp = rs.sp;
      pc = rs.pc;
    } 
    newFrame = Frame(pc, fp, proc->getPid(), this, 0, true);
  }
  return newFrame;
}  


