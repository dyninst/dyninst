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

/* $Id: context.C,v 1.119 2005/03/14 22:17:52 legendre Exp $ */

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/context.h"
#include "processMgr.h"
#include "paradynd/src/pd_image.h"


#if !defined(i386_unknown_nt4_0)
extern int termWin_port; //defined in main.C
extern pdstring pd_machine;
#endif

extern pdRPC *tp;

/*
 * find out if we have an application defined
 */
bool applicationDefined()
{
   if (getProcMgr().size()) {
      return(true);
   } else {
      return(false);
   }
}

//timeStamp getCurrentTime(bool);

extern void CallGraphSetEntryFuncCallback(pdstring exe_name, pdstring r, int tid);

void createThread(traceThread *fr) {
    assert(fr);
    process *dyn_proc = process::findProcess(fr->ppid);
    
    if(fr->tid == 0) {
        // we see these at times from a multi-threaded forked off child process
        dyn_proc->receivedInvalidThrCreateMsg();
        return;
    }
    
    dyn_thread *foundThr = dyn_proc->getThread(fr->tid);
    if(foundThr != NULL) {
        // received a duplicate thread create, can happen if rpcs launched on
        // lwps of a MT forked off child process get run on same threads, since
        // lwps changed between threads
        dyn_proc->receivedInvalidThrCreateMsg();
        return;
    }
    
    // creating new thread
    dyn_thread *thr =
    dyn_proc->createThread(fr->tid, fr->index, fr->lwp, fr->stack_addr, fr->start_pc,
                           fr->resumestate_p, fr->context==FLAG_SELF);
    assert(thr);
    
    pd_process *proc=NULL;
    proc = getProcMgr().find_pd_process(fr->ppid);

   if(! proc) {
      // child pd_process not defined so returning, can happen when handling
      // forks
      return;
   }
   pd_thread *pd_thr = new pd_thread(thr, proc);
   proc->addThread(pd_thr);
   metricFocusNode::handleNewThread(proc, pd_thr);

   // computing resource id
   pdstring buffer;
   pdstring pretty_name = pdstring(thr->get_start_func()->prettyName().c_str()) ;
   buffer = pdstring("thr_")+pdstring((unsigned)fr->tid)+pdstring("{")+pretty_name+pdstring("}");
   resource *rid;
   rid = resource::newResource(proc->get_rid(),
                                (void *)thr,
                                nullString, 
                                buffer,
                                timeStamp::ts1970(),
                                "",
                                ThreadResourceType,
                                MDL_T_STRING,
                                true);
   pd_thr->update_rid(rid);

   // tell front-end about thread start function for newly created threads
   // We need the module, which could be anywhere (including a library)
   int_function *func = thr->get_start_func();
   pdmodule *foundMod = func->pdmod();
   assert(foundMod != NULL);
   resource *modRes = foundMod->getResource();
   pdstring start_func_str = thr->get_start_func()->prettyName();
   pdstring res_string = modRes->full_name() + "/" + start_func_str;

   pd_image *im = proc->getImage();
   pdstring fl = im->get_file();
   
   CallGraphSetEntryFuncCallback(fl, res_string, thr->get_tid());
}

//
// The thread reported from DYNINSTinit when using attaching 
//
void updateThreadId(traceThread *fr) {
   //static bool firstTime=true;
   //assert(firstTime); // this routine should only execute once! - naim
   //firstTime=false;
   pd_process *pdproc = NULL;
   pdproc = getProcMgr().find_pd_process(fr->ppid);
   assert(pdproc);
   assert(pdproc->thrMgr().size() > 0);
   pd_thread *pdthr = *(pdproc->thrMgr().begin());
   dyn_thread *thr = pdthr->get_dyn_thread();
   assert(thr);
   pdstring buffer;
   resource *rid;
   process *dynproc = pdproc->get_dyn_process()->lowlevel_process();
   if(fr->context == FLAG_ATTACH) {
       dynproc->updateThread(thr, fr->tid, fr->index, fr->lwp,
                            fr->stack_addr, fr->start_pc, fr->resumestate_p);
      // computing resource id
      pdstring pretty_name = pdstring(thr->get_start_func()->prettyName().c_str());
      buffer = pdstring("thr_") + pdstring((unsigned)fr->tid) + pdstring("{") + 
	       pretty_name + pdstring("}");
   } else {
      buffer = pdstring("thr_") + pdstring((unsigned)fr->tid) + pdstring("{main}") ;
      
      // updating main thread
      dynproc->updateThread(thr, fr->tid, fr->index, fr->lwp, fr->resumestate_p);
   }

   rid = resource::newResource(pdproc->get_rid(),
                                (void *)thr,
                                nullString,
                                buffer,
                                timeStamp::ts1970(),
                                "",
                                ThreadResourceType,
                                MDL_T_STRING, 
                                true);
   pdthr->update_rid(rid);
   pd_image *im = pdproc->getImage();
   pdstring fl = im->get_file();
   int_function *entry_pdf = thr->get_start_func();
   CallGraphSetEntryFuncCallback(fl, entry_pdf->ResourceFullName(), fr->tid);
   //sprintf(errorLine, "*****updateThreadId, tid=%d, index=%d, stack=0x%x, startpc=0x%x, resumestat=0x%x\n", fr->tid, fr->index, fr->stack_addr, fr->start_pc, fr->resumestate_p) ;
   //logLine(errorLine) ;
}

void deleteThread(traceThread *fr)
{
    pd_process *pdproc = NULL;

    assert(fr);
    pdproc = getProcMgr().find_pd_process(fr->ppid);
    
    if (!pdproc) return;

    assert(pdproc && pdproc->get_rid());

    pd_thread *thr = pdproc->thrMgr().find_pd_thread(fr->tid);
    tp->retiredResource(thr->get_rid()->full_name());

    // take a final sample when thread is noticed as exited, but before it's
    // meta-data is deleted;
    pdproc->doMajorShmSample();  // take a final sample
    metricFocusNode::handleExitedThread(pdproc, thr);
    pdproc->getVariableMgr().deleteThread(thr);

    // deleting thread
    pdproc->removeThread(fr->tid);
    pdproc->get_dyn_process()->lowlevel_process()->deleteThread(fr->tid);
    // deleting resource id
    // how do we delete a resource id? - naim
}

unsigned miniTrampHandlePtrHash(miniTrampHandle * const &ptr) {
   // would be a static fn but for now aix.C needs it.
   unsigned addr = (unsigned)(Address)ptr;
   return addrHash16(addr); // util.h
}

void pd_execCallback(pd_process *pd_proc) {
   if (!pd_proc) {
      logLine("Error in pd_execCallback: could not find pd_process\n");
      return;
   }

   metricFocusNode::handleExec(pd_proc);
}

#if !defined(i386_unknown_nt4_0)

PDSOCKET connect_Svr(pdstring machine,int port)
{
  PDSOCKET stdout_fd = INVALID_PDSOCKET;

  struct sockaddr_in serv_addr;
  struct hostent *hostptr = 0;
  struct in_addr *inadr = 0;
  if (!(hostptr = P_gethostbyname(machine.c_str())))
    {
      cerr << "CRITICAL: Failed to find information for host " << pd_machine.c_str() << "." << endl;
      assert(0);
    }

  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  P_memset ((void*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *inadr;
  serv_addr.sin_port = htons(port);

  if ( (stdout_fd = P_socket(AF_INET,SOCK_STREAM , 0)) == PDSOCKET_ERROR)
  {
    stdout_fd = INVALID_PDSOCKET;
    return stdout_fd;
  }

  //connect() may timeout if lots of Paradynd's are trying to connect to
  //  Paradyn at the same time, so we keep retrying the connect().
  errno = 0;
  while (P_connect(stdout_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == PDSOCKET_ERROR) {
/*#if defined(i386_unknown_nt4_0)
    if (PDSOCKET_ERRNO != WSAETIMEDOUT)
#else */
    if (errno != ETIMEDOUT)
//#endif
    {
      stdout_fd = INVALID_PDSOCKET;
      return stdout_fd;
    } 
    CLOSEPDSOCKET(stdout_fd);
    if ((stdout_fd = P_socket(AF_INET,SOCK_STREAM, 0)) == PDSOCKET_ERROR)
    {
      stdout_fd = INVALID_PDSOCKET;
      return stdout_fd;
    }
    errno = 0;
  }

  return stdout_fd;
}
#endif


#ifdef notdef
bool addDataSource(char *name, char *machine,
    char *login, char *command, int argc, char *argv[])
{
    P_abort();
    return(false);
}
#endif

bool startApplication()
{
    continueAllProcesses();
    return(false);
}

// TODO use timers here
timeStamp startPause = timeStamp::ts1970();

// total processor time the application has been paused.
// so for a multi-processor system this should be processor * time.
timeLength elapsedPauseTime = timeLength::Zero();

static bool appPause = true;

bool markApplicationPaused()
{
  if (!appPause) {
    // get the time when we paused it.
    startPause = getWallTime();
    // sprintf(errorLine, "paused at %f\n", startPause);
    // logLine(errorLine);
    appPause = true;
    return true;
  } else 
    return false;
}

bool markApplicationRunning()
{
  if (!appPause) {
//    cerr << "WARNING: markApplicationRunning: the application IS running\n";
    return false;
  }
  appPause = false;

  if (!isInitFirstRecordTime()) {
    cerr << "WARNING: markApplicationRunning: !firstRecordTime\n";
    return false;
  }

  if (startPause > timeStamp::ts1970())
    elapsedPauseTime += (getWallTime() - startPause);

  return true;
}

bool isApplicationPaused()
{
  return appPause;
}

bool continueAllProcesses()
{
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *p = *itr++;
      if(p != NULL && p->isStopped()) {
          if(p->isTerminated() || ! p->continueProc())
          {
              sprintf(errorLine,"WARNING: cannot continue process %d\n",
                      p->getPid());
              cerr << errorLine << endl;
          }
      }
   }

   statusLine("application running");
   if (!markApplicationRunning()) {
      return false;
   }

   // sprintf(errorLine, "continued at %f\n", getCurrentTime(false));
   // logLine(errorLine);
   
   return(false); // Is this correct?
}

bool pauseAllProcesses()
{
   bool changed = markApplicationPaused();
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *p = *itr++;
      if (p != NULL && !p->isStopped() && !p->isTerminated()) {
         p->pause();
      }
   }

   if (changed)
      statusLine("application paused");
   
   return(changed);
}

void processNewTSConnection(int tracesocket_fd) {
   // either a forked process or one created via attach is trying to get a
   // new tracestream connection.  accept() the new connection, then do some
   // processing.  There is no need to restrict this for forked and attached
   // processes --mjrg
   int fd = RPC_getConnect(tracesocket_fd); // accept()
      // will become traceLink of new process
   assert(fd >= 0);

   unsigned cookie;
   //cerr << "processNewTSConnection\n";
   unsigned rRet = 0;
   if (sizeof(cookie) != (rRet=P_recv(fd, &cookie, sizeof(cookie), 0))) {
      cerr << "error, read return: " << rRet << ", cookieSize: "
           << sizeof(cookie) << endl;      
      assert(false);
   }

   //const unsigned cookie_fork   = 0x11111111;
   const unsigned cookie_attach = 0x22222222;

   bool calledFromAttach = (cookie == cookie_attach);

   pdstring str;

   if (calledFromAttach)
   {
      str = "getting new connection from attached process";
   }
   else
   {
#if !defined(i386_unknown_nt4_0)
      str = "getting unexpected process connection";
      statusLine(str.c_str());
#else
      // we expect a 0 cookie value on Windows where CreateProcess
      // is not the same semantics as fork.
#endif // !defined(i386_unknown_nt4_0)
   } 

   int pid;
   if (sizeof(pid) != P_recv(fd, &pid, sizeof(pid), 0))
      assert(false);

   int ppid;
   if (sizeof(ppid) != P_recv(fd, &ppid, sizeof(ppid), 0))
      assert(false);

   key_t theKey;
   if (sizeof(theKey) != P_recv(fd, &theKey, sizeof(theKey), 0))
      assert(false);

   int32_t ptr_size;
   if (sizeof(ptr_size) != P_recv(fd, &ptr_size, sizeof(ptr_size), 0))
      assert(false);

   void *applAttachedAtPtr = NULL;
   char *ptr_dst = (char *)&applAttachedAtPtr;
   if (sizeof(void *) > (uint32_t)ptr_size) {
      // adjust for pointer size mismatch
      ptr_dst += sizeof(void *) - sizeof(int32_t);
   }
   if (ptr_size != P_recv(fd, ptr_dst, ptr_size, 0))
      assert(false);

   process *curr = NULL;

   // This routine gets called when the attached process is in
   // the middle of running DYNINSTinit.
   curr = process::findProcess(pid);
   assert(curr);
   curr->traceLink = fd;
   statusLine("ready");
}

bool allThreadCreatesReceived(process *proc, unsigned num_expected) {
   unsigned num_thrs = proc->threads.size();
   unsigned invalid_creates = proc->numInvalidThrCreateMsgs();
   //cerr << "num_thrs: " << num_thrs << ", inv_creates: " << invalid_creates
   //     << ", total_received: " << num_thrs + invalid_creates 
   //     << "  [" << num_expected << "]\n";
   return (num_thrs + invalid_creates >= num_expected);
}

bool allThreadsCreated(process *proc, const pdvector<unsigned> &par_tids) {
   for(unsigned i=0; i<par_tids.size(); i++) {
      bool found_lwp = false;
      unsigned cur_tid = par_tids[i];
      //cerr << "looking for tid: " << cur_tid << endl;
      for(unsigned j=0; j<proc->threads.size(); j++) {
         dyn_thread *curthr = proc->threads[j];

         if(curthr->get_tid() == cur_tid) {
            found_lwp = true;
            break;
         }
      }
      if(found_lwp == false) {
         //cerr << "still waiting on thr w/ tid " << cur_tid 
         //     << " to be created\n";
         return false;
      }
   }
   return true;
}

extern PDSOCKET traceSocket_fd;

// returns true when all threads created, false if not all have been created
// yet.  the function will monitor the trace link so that createThread
// calls can be handled.
void wait_for_thread_creation(process *childDynProc,
                              unsigned num_expected) {
   // recognize and set up meta-data for threads in the child process
   fd_set readSet;
   fd_set errorSet;
   int ct;
   while(! allThreadCreatesReceived(childDynProc, num_expected)) {
       if(childDynProc->hasExited()) return;
       //getSH()->checkForAndHandleProcessEvent(false);

      int width = 0;
      struct timeval pollTimeStruct;
      timeLength pollTime(50, timeUnit::ms());
      pollTimeStruct.tv_sec  = 
         static_cast<long>(pollTime.getI(timeUnit::sec()));
      pollTimeStruct.tv_usec = 
         static_cast<long>(pollTime.getI(timeUnit::us()));
   
      FD_ZERO(&readSet);
      FD_ZERO(&errorSet);
      if(childDynProc->traceLink >= 0) {
         FD_SET(childDynProc->traceLink, &readSet);
      }
      if(childDynProc->traceLink > width)
         width = childDynProc->traceLink;

      // add traceSocket_fd, which accept()'s new connections (from processes
      // not launched via createProcess() [process.C], such as when a process
      // forks, or when we attach to an already-running process).
      if (traceSocket_fd != INVALID_PDSOCKET) FD_SET(traceSocket_fd, &readSet);
      if (traceSocket_fd > width) width = traceSocket_fd;

      // add our igen connection with the paradyn process.
      FD_SET(tp->get_sock(), &readSet);
      FD_SET(tp->get_sock(), &errorSet);

      // "width" is computed but ignored on Windows NT, where sockets 
      // are not represented by nice little file descriptors.
      if (tp->get_sock() > width) width = tp->get_sock();
      // TODO - move this into an os dependent area
      ct = P_select(width+1, &readSet, NULL, &errorSet, &pollTimeStruct);
      if (ct <= 0)  continue;

      if (traceSocket_fd >= 0 && FD_ISSET(traceSocket_fd, &readSet)) {
         // Either (1) a process we're measuring has forked, and the child
         // process is asking for a new connection, or (2) a process we've
         // attached to is asking for a new connection.
	processNewTSConnection(traceSocket_fd); // context.C
      }      
      
      if(childDynProc->traceLink >= 0 && 
         FD_ISSET(childDynProc->traceLink, &readSet)) {
         processTraceStream(childDynProc);
         /* in the meantime, the process may have died, setting
            curProc to NULL */
         
         /* clear it in case another process is sharing it */
         if(childDynProc->traceLink >= 0) {
            // may have been set to -1
            FD_CLR(childDynProc->traceLink, &readSet);
         }
      }
   }
}

void show_proc_threads(process *proc) {
   cerr << "showing proc threads, pid: " << proc->getPid() << endl;
   for(unsigned j=0; j<proc->threads.size(); j++) {
      dyn_thread *curthr = proc->threads[j];
      if(curthr->get_lwp() == NULL) {
         cerr << "    thr_id: " << curthr->get_tid() 
              << ", calling updateLWP\n";
         curthr->updateLWP();
         cerr << "    lwp now is " << curthr->get_lwp() << endl;
      }
      cerr << "  thr_id: " << curthr->get_tid();
      if(curthr->get_lwp())
         cerr << ", lwp_id: " << curthr->get_lwp()->get_lwp_id();
      cerr << ", index: " << curthr->get_index() << endl;
   }
}

// This function sets the variable DYNINST_initialize_done to 1 in the
// multi-threaded rtinst library.  This variable is a way to keep certain
// rtinst functions from being called, like DYNINSTthreadIndex.
void initMT_AfterFork(process *proc) {
   bool err = false;
   err = false;
   Address initAddr = proc->findInternalAddress("DYNINST_initialize_done",
                                            true, err);
   assert(err==false);
   unsigned newInitVal = 1;

   bool retv = proc->writeTextSpace((caddr_t)initAddr, sizeof(unsigned), 
                              (caddr_t)&newInitVal);
   if(retv == false) {
     cerr << "!!!  Couldn't write DYNINST_initialize_done variable into "
          << "rt library !!\n";
   }

}

void MT_lwp_setup(process *parentDynProc, process *childDynProc) {
   pdvector<unsigned> par_tids;
   for(unsigned i=0; i<parentDynProc->threads.size(); i++) {
      unsigned tid = parentDynProc->threads[i]->get_tid();
      if(tid == 1)
         continue;
      par_tids.push_back(tid);
   }

#if 0
   unsigned num_expected = 0;
   pdvector<unsigned> expected_thrs;
#if defined(sparc_sun_solaris2_4)
   expected_thrs = par_tids;
#endif
   pdvector<unsigned> completed_lwps;
   do {
      childDynProc->recognize_threads(&completed_lwps);
      for (unsigned foo = 0; foo < completed_lwps.size(); foo++)
      num_expected += completed_lwps.size();
      completed_lwps.clear();
      //wait_for_thread_creation(childDynProc, num_expected);
#if defined(rs6000_ibm_aix4_1)
      if(childDynProc->threads.size() > 0)
         expected_thrs.push_back(childDynProc->threads[0]->get_tid());
#endif
   } while(! allThreadsCreated(childDynProc, expected_thrs));
   initMT_AfterFork(childDynProc);
#endif

   unsigned num_expected = 0;
   pdvector<unsigned> expected_thrs;
#if defined(sparc_sun_solaris2_4)
   expected_thrs = par_tids;
#endif
   pdvector<unsigned> completed_lwps;

   childDynProc->recognize_threads(&completed_lwps);

   num_expected += completed_lwps.size();
   completed_lwps.clear();
   //wait_for_thread_creation(childDynProc, num_expected);

   // Idea: we'll catch the threads over the standard trace pipe

   initMT_AfterFork(childDynProc);
}








