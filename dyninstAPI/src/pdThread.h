/*
 * Copyright (c) 1996-2001 Barton P. Miller
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

// $Id: pdThread.h,v 1.10 2001/03/06 15:45:16 pcroth Exp $

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
    pdThread(process *pproc) : 
      tid(0), 
      fd(-1), 
      pos(0),
      pd_pos(0),
      stack_addr(0),
      start_pc(0),
      start_func(NULL),
      rid(NULL),
#ifndef BPATCH_LIBRARY
      previous(0),
#endif
      pending_tramp_addr( NULL )
    { 
      proc = pproc; 
      ppid = pproc->getPid();
    }
    pdThread(process *proc_, int tid_, unsigned pos_) :
      tid(tid_),
      fd(-1),
      pos(pos_),
      pd_pos(0),
      stack_addr(0),
      start_pc(0),
      start_func(NULL),
      rid(NULL),
#ifndef BPATCH_LIBRARY
      previous(0),
#endif
      pending_tramp_addr( NULL )
    {
      proc = proc_;
      ppid = proc_->getPid();
    }
    pdThread(process *pproc, int tid_, handleT handle_) :
      tid(tid_),
      fd(-1),
      pos(0),
      pd_pos(0),
      stack_addr(0),
      start_pc(0),
      start_func(NULL),
      rid(NULL),
#ifndef BPATCH_LIBRARY
      previous(0),
#endif
      pending_tramp_addr( NULL )
    {
      assert(pproc);
      proc = pproc;
      ppid = pproc->getPid();
      handle = handle_ ;
    }
    pdThread(process *parent, pdThread *src) {
      assert(src && parent);
      tid = src->tid;
      ppid = parent->getPid();
      pos = src->pos;
      pd_pos = src->pd_pos;
      stack_addr = src->stack_addr;
      start_pc = src->start_pc;
      start_func = src->start_func;
      rid = src->rid;
      proc = parent;
#ifndef BPATCH_LIBRARY
      previous = 0;
#endif
      pending_tramp_addr = NULL;
    }
    ~pdThread() {
#if defined(MT_THREAD)
      proc->threadMap->remove(tid);
#endif
      //delete rid; //deletion of resources is not yet implemented! - naim 1/21/98
      close(fd);
    }
    int            get_tid()           { return(tid); }
    unsigned       get_pos()           { return(pos); }
    unsigned       get_pd_pos()        { return(pd_pos); }
    unsigned       get_stack_addr()    { return(stack_addr); }
    int            get_fd()            { return(fd); }
    int            get_ppid()          { return(ppid); }
    resource*      get_rid()           { return(rid); }
    process*       get_proc()          { return(proc); }
    handleT        get_handle()        { return(handle); }
    function_base* get_start_func()    { return(start_func); }
    unsigned       get_start_pc()      { return(start_pc); }
    void*          get_resumestate_p() { return resumestate_p; }
#if defined(MT_THREAD)
    static rawTime64  getInferiorVtime(tTimer* , process*, bool&);
#endif
    void update_tid          (int id, unsigned p)   { tid = id; pos = p; }
    void update_handle       (int id, handleT h)    { tid = id; handle = h; }
    void update_rid          (resource *rid_)       { rid = rid_; } 
    void update_pd_pos       (unsigned p)           { pd_pos = p; }
    void update_fd           (int fd_)              { fd = fd_; }
    void update_stack_addr   (unsigned stack_addr_) { stack_addr=stack_addr_; }
    void update_start_pc     (unsigned start_pc_)   { start_pc=start_pc_; }
    void update_start_func   (function_base *pdf)   { start_func=pdf; }
    void update_resumestate_p(void* resumestate_p_) { resumestate_p=resumestate_p_; }

	Address get_pending_tramp_addr( void ) const	{ return pending_tramp_addr; }
	void set_pending_tramp_addr( Address a )	{ pending_tramp_addr = a; }

///
  private:
    int tid;
    int ppid;
    int fd;
    unsigned pos;
    unsigned pd_pos;
    unsigned stack_addr;
    unsigned start_pc ;
    void*    resumestate_p; //platform specific
    function_base *start_func ;
    resource *rid;
    process *proc;
    handleT handle; // the thread handle (/proc file descriptor or NT handle)
#ifndef BPATCH_LIBRARY
    rawTime64 previous;
#endif
    Address pending_tramp_addr;	// address of pending instrumentation
								// currently used on NT only

};

#endif
