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

#ifndef _PDTHREAD_H_
#define _PDTHREAD_H_

#include "dyninstAPI/src/process.h"

class pdThread {
  public:
    // This definition must be completed later when we get the result of a call
    // to thr_self in the application. We are assuming that 
    // 0 <= thr_self,tid < MAX_NUMBER_OF_THREADS - naim
    // We are also assuming that the position in the paradynd super table
    // for this thread is initially 0, until it gets updated with the record
    // sent in DYNINSTinit - naim 4/15/97
    pdThread(process *pproc) : tid(0), pos(0), pd_pos(0), rid(NULL)
    { 
      proc = pproc; 
      ppid = pproc->getPid();
    }
    pdThread(process *proc_, int tid_, unsigned pos_, resource *rid_ )
    { 
      proc = proc_; 
      ppid = proc_->getPid();
      tid = tid_;
      pos = pos_;
      rid = rid_;
    }
    pdThread(process *pproc, int tid_, handleT handle_) : tid(tid_), pos(0), rid(NULL), handle(handle_)
    {
      assert(pproc);
      proc = pproc;
      ppid = pproc->getPid();
    }
    pdThread(process *parent, pdThread *src) {
      assert(src && parent);
      tid = src->tid;
      ppid = parent->getPid();
      pos = src->pos;
      pd_pos = src->pd_pos;
      rid = src->rid;
      proc = parent;
    }
    ~pdThread() {}
    int get_tid() { return(tid); }
    unsigned get_pos() { return(pos); }
    unsigned get_pd_pos() { return(pd_pos); }
    void update_tid(int id, unsigned p) { tid = id; pos = p; }
    void update_handle(int id, handleT h) { tid = id; handle = h; }
    void update_pd_pos(unsigned p) { pd_pos = p; }
    int get_ppid() { return(ppid); }
    resource *get_rid() { return(rid); }
    process *get_proc() { return(proc); }
    handleT get_handle() { return(handle); }
  private:
    int tid;
    int ppid;
    unsigned pos;
    unsigned pd_pos;
    resource *rid;
    process *proc;
    handleT handle; // the thread handle (/proc file descriptor or NT handle)
};

#endif
