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

/* $Id: context.C,v 1.83 2002/12/20 07:50:06 jaw Exp $ */

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/perfStream.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/init.h"
#include "processMgr.h"


// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

#if !defined(i386_unknown_nt4_0)
extern int termWin_port; //defined in main.C
extern string pd_machine;
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

#if defined(MT_THREAD)

void createThread(traceThread *fr) {
    pd_process *proc=NULL;
    assert(fr);
    proc = getProcMgr().find_pd_process(fr->ppid);
    if (!(proc && proc->get_rid())) // 6/2/99 zhichen, a weird situation
      return ;                // when a threaded-process forks
    // creating new thread
    dyn_thread *thr = 
       proc->get_dyn_process()->createThread(fr->tid, fr->pos, fr->stack_addr,
		      fr->start_pc, fr->resumestate_p, fr->context==FLAG_SELF);
    assert(thr);
    pd_thread *pd_thr = new pd_thread(thr);
    proc->addThread(pd_thr);

    metricFocusNode::handleNewThread(proc, pd_thr);

    // computing resource id
    string buffer;
    string pretty_name = string(thr->get_start_func()->prettyName().c_str()) ;
    buffer = string("thr_")+string(fr->tid)+string("{")+pretty_name+string("}");
    resource *rid;
    rid = resource::newResource(proc->get_rid(), (void *)thr, nullString, 
                                buffer, timeStamp::ts1970(), "", MDL_T_STRING,
                                true);
    pd_thr->get_dyn_thread()->update_rid(rid);

    // tell front-end about thread start function for newly created threads
    image *img = proc->getImage();
    pdmodule *foundMod = img->findModule(thr->get_start_func());
    assert(foundMod != NULL);
    resource *modRes = foundMod->getResource();
    string start_func_str = thr->get_start_func()->prettyName();
    string res_string = modRes->full_name() + "/" + start_func_str;
    CallGraphSetEntryFuncCallback(img->file(), res_string, thr->get_tid());
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
   string buffer;
   resource *rid;
   process *dynproc = pdproc->get_dyn_process();
   if(fr->context == FLAG_ATTACH) {
      dynproc->updateThread(thr, fr->tid, fr->pos, 
			    fr->stack_addr, fr->start_pc, fr->resumestate_p);
      // computing resource id
      string pretty_name = string(thr->get_start_func()->prettyName().c_str());
      buffer = string("thr_") + string(fr->tid) + string("{") + 
	       pretty_name + string("}");
      rid = resource::newResource(dynproc->rid, (void *)thr, nullString, 
			  buffer, timeStamp::ts1970(), "", MDL_T_STRING, true);
      thr->update_rid(rid);        
   } else {
      buffer = string("thr_") + string(fr->tid) + string("{main}") ;
      rid = resource::newResource(dynproc->rid, (void *)thr, nullString,
			  buffer, timeStamp::ts1970(), "", MDL_T_STRING, true);
      
      // updating main thread
      dynproc->updateThread(thr, fr->tid, fr->pos, fr->resumestate_p, rid);
   }
   //sprintf(errorLine, "*****updateThreadId, tid=%d, pos=%d, stack=0x%x, startpc=0x%x, resumestat=0x%x\n", fr->tid, fr->pos, fr->stack_addr, fr->start_pc, fr->resumestate_p) ;
   //logLine(errorLine) ;
}

void deleteThread(traceThread *fr)
{
    pd_process *pdproc = NULL;

    assert(fr);
    pdproc = getProcMgr().find_pd_process(fr->ppid);
    assert(pdproc && pdproc->get_dyn_process()->rid);

    pd_thread *thr = pdproc->thrMgr().find_pd_thread(fr->tid);
    tp->retiredResource(thr->get_dyn_thread()->get_rid()->full_name());

    // take a final sample when thread is noticed as exited, but before it's
    // meta-data is deleted;
    pdproc->doMajorShmSample();  // take a final sample
    metricFocusNode::handleDeletedThread(pdproc, thr);
    pdproc->getVariableMgr().deleteThread(thr);

    // deleting thread
    pdproc->removeThread(fr->tid);
    pdproc->get_dyn_process()->deleteThread(fr->tid);
    // deleting resource id
    // how do we delete a resource id? - naim
}

#endif

unsigned instInstancePtrHash(instInstance * const &ptr) {
   // would be a static fn but for now aix.C needs it.
   unsigned addr = (unsigned)(Address)ptr;
   return addrHash16(addr); // util.h
}

void forkProcess(int pid, int ppid, int trace_fd, key_t theKey,
		 void *applAttachedAtPtr) 
{
   pd_process *parentProc = getProcMgr().find_pd_process(ppid);
   if (!parentProc) {
      logLine("Error in forkProcess: could not find parent process\n");
      return;
   }

#ifdef FORK_EXEC_DEBUG
   timeStamp forkTime = getWallTime();
#endif
   process *childDynProc = 
      process::forkProcess(parentProc->get_dyn_process(), (pid_t)pid, trace_fd,
			   theKey, applAttachedAtPtr);
   pd_process *childProc = new pd_process(*parentProc, childDynProc);
   getProcMgr().addProcess(childProc);
   // For each mi with a component in parentProc, copy it to the child
   // process --- if the mi isn't refined to a specific process (i.e. is for
   // 'any process') NOTE: It's easy to not copy data items (timers, ctrs)
   // that don't belong in the child.  But for trampolines, it's tricky,
   // since the fork() syscall will copy all code whether we like it or not
   // (except on AIX).  Since the meta-data for the conventional inferior
   // heap has already been copied by the fork-ctor, when we detect code that
   // shouldn't have been copied, we manually delete it with deleteInst().
   // "map" is helpful in this context.
   metricFocusNode::handleFork(parentProc, childProc);
   // The following routines perform some assertion checks.
   //   childProc->getTable().forkHasCompleted();

#ifdef FORK_EXEC_DEBUG
   cerr << "Fork process took " << (getWallTime()-forkTime) << endl;
#endif

   // Here is where we (used to) continue the parent process...who has been
   // waiting patiently at a DYNINSTbreakPoint() since the beginning of
   // DYNINSTfork() while all this hubbub was going on.  But we can't issue
   // the continueProc().  Why not?  Because it's quite possible that the
   // signal delivered to paradynd by the DYNINSTbreakPoint() hasn't yet been
   // processed (yes, this happens in practice), so paradynd still thinks
   // that parentProc's status is running.  What's the solution?  We create a
   // stupid new field in the process structure that, when true, tells
   // paradynd that when the next SIGSTOP is delivered, to continue the
   // process.  On the other hand, if parentProc's status is stopped, then we
   // go ahead and issue the continueProc now.  --ari

//   if (parentProc->status() == running)
//      parentProc->continueAfterNextStop();
//   else
//      if (!parentProc->continueProc())
//         assert(false);
}

void pd_execCallback(process *proc) {
   pd_process *pd_proc = getProcMgr().find_pd_process(proc);
   if (!pd_proc) {
      logLine("Error in pd_execCallback: could not find pd_process\n");
      return;
   }

   metricFocusNode::handleExec(pd_proc);
}

#if !defined(i386_unknown_nt4_0)

PDSOCKET connect_Svr(string machine,int port)
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

int pd_createProcess(pdvector<string> &argv, pdvector<string> &envp, string dir) {

#if !defined(i386_unknown_nt4_0)
   if (termWin_port == -1)
      return -1;
  
   PDSOCKET stdout_fd = INVALID_PDSOCKET;
   if ((stdout_fd = connect_Svr(pd_machine,termWin_port)) == INVALID_PDSOCKET)
      return -1;
   if (write(stdout_fd,"from_app\n",strlen("from_app\n")) <= 0)
   {
      CLOSEPDSOCKET(stdout_fd);
      return -1;
   }
#endif

	// NEW: We bump up batch mode here; the matching bump-down occurs after
	// shared objects are processed (after receiving the SIGSTOP indicating
	// the end of running DYNINSTinit; more specifically,
	// procStopFromDYNINSTinit().  Prevents a diabolical w/w deadlock on
	// solaris --ari
	tp->resourceBatchMode(true);

#if !defined(i386_unknown_nt4_0)
   process *proc = createProcess(argv[0], argv, envp, dir, 0, stdout_fd, 2);
#else 
   process *proc = createProcess(argv[0], argv, envp, dir, 0, 1, 2);
#endif

	if (!costMetric::addProcessToAll(proc))
      assert(false);

   pd_process *new_pd_proc = new pd_process(proc);
   getProcMgr().addProcess(new_pd_proc);

   if(proc) {
      return(proc->getPid());
   } else {
#if !defined(i386_unknown_nt4_0)
      CLOSEPDSOCKET(stdout_fd);
#endif
      return(-1);
   }
}

bool pd_attachProcess(const string &progpath, int pid, int afterAttach) { 
	// matching bump-down occurs in procStopFromDYNINSTinit().
	tp->resourceBatchMode(true);

	process *new_proc;
	bool res = attachProcess(progpath, pid, afterAttach, &new_proc);

	if (!costMetric::addProcessToAll(new_proc))
		assert(false);

	pd_process *new_pd_proc = new pd_process(new_proc);
	getProcMgr().addProcess(new_pd_proc);
	
	return res;
}

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
      if(p != NULL && p->status() != running) {
#ifdef DETACH_ON_THE_FLY
         if(! p->detachAndContinue())
#else
	 if(! p->continueProc())
#endif
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

      if (p != NULL && p->status() == running) {
#ifdef DETACH_ON_THE_FLY
         p->reattachAndPause();
#else
         p->pause();
#endif
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
   if (sizeof(cookie) != read(fd, &cookie, sizeof(cookie)))
      assert(false);

   const unsigned cookie_fork   = 0x11111111;
   const unsigned cookie_attach = 0x22222222;

   bool calledFromFork   = (cookie == cookie_fork);
   bool calledFromAttach = (cookie == cookie_attach);

   string str;
   if (calledFromFork)
      str = string("getting new connection from forked process");
   else if (calledFromAttach)
      str = string("getting new connection from attached process");
   else
      str = string("getting unexpected process connection");
   statusLine(str.c_str());

   int pid;
   if (sizeof(pid) != read(fd, &pid, sizeof(pid)))
      assert(false);

   int ppid;
   if (sizeof(ppid) != read(fd, &ppid, sizeof(ppid)))
      assert(false);

   key_t theKey;
   if (sizeof(theKey) != read(fd, &theKey, sizeof(theKey)))
      assert(false);

   int32_t ptr_size;
   if (sizeof(ptr_size) != read(fd, &ptr_size, sizeof(ptr_size)))
      assert(false);

   void *applAttachedAtPtr = NULL;
   char *ptr_dst = (char *)&applAttachedAtPtr;
   if (sizeof(void *) > (uint32_t)ptr_size) {
      // adjust for pointer size mismatch
      ptr_dst += sizeof(void *) - sizeof(int32_t);
   }
   if (ptr_size != read(fd, ptr_dst, ptr_size))
      assert(false);

   pd_process *curr = NULL;

   if (calledFromFork) {
      // the following will (1) call fork ctor (2) call
      // metricFocusNode::handleFork (3) continue the parent process, who has
      // been waiting to avoid race conditions.
      forkProcess(pid, ppid, fd, theKey, applAttachedAtPtr);
      curr = getProcMgr().find_pd_process(pid);
      assert(curr);
      // continue process...the next thing the process will do is call
      // DYNINSTinit(-1, -1, -1)
      string str = string("running DYNINSTinit() for fork child pid ") + string(pid);
	  forkexec_cerr << str << endl;
      statusLine(str.c_str());
	  if( curr->status() == running )
	  {
//#if defined(i386_unknown_linux2_0)
		  curr->continueAfterNextStop();
//#endif
	  }
	  else {
		  if (!curr->continueProc()) {
		    assert(false);
		  }
	  }
#if defined(rs6000_ibm_aix4_1)
      // HACK to compensate for AIX goofiness: as soon as we call
      // continueProc() above (and not before!), a SIGTRAP appears to
      // materialize out of thin air, stopping the child process.  Thus,
      // DYNINSTinit() won't run unless we issue an explicit continue.
      // (Actually, there may be a semi-legit explanation.  It seems that on
      // non-solaris platforms, including sunos and aix, if a sigstop is sent
      // and not handled -- i.e. if we just leave the application in a paused
      // state, without continuing -- then the sigstop will be sent over and
      // over again, nonstop, until the application is continued.  So perhaps
      // the sigtrap we see now was present all along, but we never knew it
      // because waitpid in the main loop kept returning sigstops.  --ari)

      int wait_status;
      int wait_result = waitpid(curr->getPid(), &wait_status, WNOHANG);
      if (wait_result > 0) {
	 bool was_stopped = WIFSTOPPED(wait_status);
	 if (was_stopped) {
	    int sig = WSTOPSIG(wait_status);
	    if (sig == 5)  { // sigtrap
	       curr->get_dyn_process()->status_ = stopped;
	       if (!curr->continueProc())
		  assert(false);
 	    }
        }
      }

#elif defined(i386_unknown_linux2_0)
	  int wait_status;
	  int wait_result = waitpid( curr->getPid(), &wait_status, WUNTRACED );
	  if( wait_result > 0 && WIFSTOPPED(wait_status) )
	  {
		  int sig = WSTOPSIG(wait_status);
		  forkexec_cerr << "Extra check: stopped on sig " << sig << endl;
		  if (sig == SIGTRAP || sig == SIGSTOP)
		  {
			  curr->get_dyn_process()->status_ = stopped;
			  if (!curr->continueProc())
				  assert(false);
		  }
	  }
#endif      
   }

   else {
      // This routine gets called when the attached process is in
      // the middle of running DYNINSTinit.
      curr = getProcMgr().find_pd_process(pid);
      assert(curr);
      curr->get_dyn_process()->traceLink = fd;
      statusLine("ready");
   }
}

void paradyn_handleProcessExit(process *proc, int exitStatus) {
   pd_process *pd_proc = getProcMgr().find_pd_process(proc);
   assert(pd_proc != NULL);
   pd_proc->handleExit(exitStatus);
}







