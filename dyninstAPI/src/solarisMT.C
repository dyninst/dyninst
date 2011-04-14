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

// $Id: solarisMT.C,v 1.16 2005/03/02 23:31:10 bernat Exp $

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

// As of solaris 8, threads are bound 1:1 with lwps. So this will always
// end up querying an lwp.

Frame dyn_thread::getActiveFrameMT() {

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

  updateLWP();
  if (lwp) {
    newFrame = lwp->getActiveFrame();
    newFrame.thread_ = this;
  }
  else {
    resumestate_t rs ;
    if (get_start_pc() &&
	proc->readDataSpace((caddr_t) get_resumestate_p(),
			    sizeof(resumestate_t), (caddr_t) &rs, false)) {
      fp = rs.sp;
      pc = rs.pc;
    } 
    newFrame = Frame(pc, fp, 0, proc->getPid(), proc, this, 0, true);
  }
  return newFrame;
}  


