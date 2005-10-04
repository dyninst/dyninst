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

// $Id: linux.C,v 1.177 2005/10/04 18:10:09 legendre Exp $

#include <fstream>

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

#include <sys/ptrace.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/wait.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/signalhandler.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "mapped_object.h"
#include "mapped_module.h"

#include "dynamiclinking.h"

#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#include "Elf_X.h"
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 

#ifdef PAPI
#include "papi.h"
#endif

//These values can be different on different platforms, and don't seem to be
//consistently defined in the system's include files.
#if !defined(PTRACE_SETOPTIONS)
#if defined(arch_x86)
#define PTRACE_SETOPTIONS 21
#elif defined(arch_ia64)
#define PTRACE_SETOPTIONS 0x4200
#endif
#endif

#if !defined(PT_TRACE_EXIT)
#define PTRACE_O_TRACEEXIT 0x40
#endif

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

bool dyn_lwp::deliverPtrace(int request, Address addr, Address data) {
   bool needToCont = false;
   int len = proc_->getAddressWidth();

   if(request != PT_DETACH  &&  status() == running) {
      cerr << "  potential performance problem with use of dyn_lwp::deliverPtrace\n";
      if(pauseLWP() == true)
         needToCont = true;
   }

   bool ret = (P_ptrace(request, get_lwp_id(), addr, data, len) != -1);
   if (!ret) 
   {
      fprintf(stderr, "%d - ", get_lwp_id());
      perror("Internal ptrace");
   }
   
   if(request != PTRACE_DETACH  &&  needToCont == true)
      continueLWP();
   return ret;
}


// Some ptrace requests in Linux return the value rather than storing at the address in data
// (especially PEEK requests)
// - nash
long dyn_lwp::deliverPtraceReturn(int request, Address addr, Address data) {
   bool needToCont = false;
   int len = proc_->getAddressWidth();

   if(request != PT_DETACH  &&  status() == running) {
      cerr << "  potential performance problem with use of "
           << "dyn_lwp::deliverPtraceReturn\n";
      if(pauseLWP() == true)
         needToCont = true;
   }

   long ret = P_ptrace(request, get_lwp_id(), addr, data, len);

   if(request != PTRACE_DETACH  &&  needToCont == true)
      continueLWP();
   return ret;
}

/* ********************************************************************** */

void printStackWalk( process *p ) {
  dyn_lwp *lwp_to_use = NULL;
  if(process::IndependentLwpControl() && p->getRepresentativeLWP() ==NULL)
     lwp_to_use = p->getInitialThread()->get_lwp();
  else
     lwp_to_use = p->getRepresentativeLWP();

  Frame theFrame = lwp_to_use->getActiveFrame();
  while (true) {
    // do we have a match?
    const Address framePC = theFrame.getPC();
    inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
    
    if (theFrame.isLastFrame())
      // well, we've gone as far as we can, with no match.
      break;
    
    // else, backtrace 1 more level
    theFrame = theFrame.getCallerFrame();
  }
}
 
// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL);
  P_close (ttyfd);
}

// may be needed in the future
#if defined(DECODEANDHANDLE_EVENT_ON_LWP)
// returns true if it handled an event
bool checkForAndHandleProcessEventOnLwp(bool block, dyn_lwp *lwp) {
   pdvector<procevent *> foundEvents;
   getSH()->checkForProcessEvents(&foundEvents, lwp->get_lwp_id(), block);
   getSH()->handleProcessEvents(foundEvents);
   //assert(selectedLWP == lwp);
   
   return true;
}
#endif

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

// Wait for a process event to occur, then map it into
// the why/what space (a la /proc status variables)

int decodeRTSignal(process *proc,
                   procSignalWhy_t &why,
                   procSignalWhat_t &what,
                   procSignalInfo_t &info)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   if (!proc) return 0;

   pdstring status_str = pdstring("DYNINST_synch_event_id");
   pdstring arg_str = pdstring("DYNINST_synch_event_arg1");

   int status;
   Address arg;

   pdvector<int_variable *> vars;
   if (!proc->findVarsByAll(status_str, 
                            vars)) {
       // Can restrict to a particular library... might be worth it to streamline.
       // Hell... why not make this a process member and keep the addresses around?
       // We might not have the RT lib yet...
       return 0;
   }
   assert(vars.size() == 1); // findVarsByAll should return false if we didn't find anything

   Address status_addr = vars[0]->getAddress();

   if (!proc->readDataSpace((void *)status_addr, sizeof(int), 
                            &status, true)) {
       bperr("%s[%d]:  readDataSpace, status = %d\n", __FILE__, __LINE__,  status);
      return 0;
   }

   if (status == DSE_undefined) {
      return 0; // Nothing to see here
   }

   vars.clear();
   if (!proc->findVarsByAll(arg_str,
                            vars)) {
       assert(0);
       return 0;
   }
   assert(vars.size() == 1);

   Address arg_addr = vars[0]->getAddress();
    
   if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                            &arg, true))
      assert(0);
   info = (procSignalInfo_t)arg;
   switch(status) {
     case DSE_forkEntry:
        /* Entry to fork */
        why = procSyscallEntry;
        what = SYS_fork;
        break;
     case DSE_forkExit:
        why = procSyscallExit;
        what = SYS_fork;
        break;
     case DSE_execEntry:
        /* Entry to exec */
        why = procSyscallEntry;
        what = SYS_exec;
        break;
     case DSE_execExit:
        /* Exit of exec, unused */
        break;
     case DSE_exitEntry:
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        why = procSyscallEntry;
        what = SYS_exit;
        break;
   case DSE_loadLibrary:
     /* Unused */
     break;
     default:
        assert(0);
        break;
   }
   return 1;
}

static void get_linux_version(int &major, int &minor)
{
   static int maj = 0, min = 0;
   int result;
   FILE *f;
   if (maj)
   {
      major = maj;
      minor = min;
      return;
   }

   f = fopen("/proc/version", "r");
   if (!f)
   {
      //Assume 2.4, which is the earliest version we support
      major = maj = 2;
      minor = min = 4;
      return;
   }
   result = fscanf(f, "Linux version %d.%d", &major, &minor);
   fclose(f);

   if (result != 2)
   {
      major = maj = 2;
      minor = min = 4;
      return;
   }

   maj = major;
   min = minor;
   return;
}

static pdvector<pdstring> attached_lwp_ids;
static void add_lwp_to_poll_list(dyn_lwp *lwp)
{
   char filename[64];
   int lwpid, major, minor;
   struct stat buf;

   get_linux_version(major, minor);   
   if ((major == 2 && minor > 4) || (major >= 3))
      return;
   if (!lwp->proc()->multithread_capable(true))
      return;
   
   lwpid = lwp->get_lwp_id();
   snprintf(filename, 64, "/proc/%d", lwpid);
   if (stat(filename, &buf) == 0)
   {
      attached_lwp_ids.push_back(pdstring(lwpid));
      return;
   }

   snprintf(filename, 64, "/proc/.%d", lwpid);
   if (stat(filename, &buf) == 0)
   {
      attached_lwp_ids.push_back(pdstring(".") + pdstring(lwpid));
      return;
   }
   
   fprintf(stderr, "[%s:%u] - Internal Error.  Could not find new process %d"
           " in /proc area.  Thread deletion callbacks may not work\n", 
           __FILE__, __LINE__, lwpid);
}

static void remove_lwp_from_poll_list(int lwp_id)
{
   for (unsigned i=0; i<attached_lwp_ids.size(); i++)
   {
      const char *lname = attached_lwp_ids[i].c_str();
      if (*lname == '.') lname++;
      if (atoi(lname) == lwp_id)
      {
         attached_lwp_ids.erase(i, i);
      }
   }
}

static int find_dead_lwps()
{
   struct stat buf;
   char filename[64];
   int lwpid;

   for (unsigned i=0; i<attached_lwp_ids.size(); i++)
   {
      snprintf(filename, 64, "/proc/.%s", attached_lwp_ids[i].c_str());
      if (stat(filename, &buf) != 0)
      {
         const char *s = attached_lwp_ids[i].c_str();
         if (*s == '.') s++;
         lwpid = atoi(s);
         remove_lwp_from_poll_list(lwpid);
         return lwpid;
      }
   }
   return 0;
}

/**
 * This is a bad hack.  On Linux 2.4 waitpid doesn't return for dead threads,
 * so we poll for any dead threads before calling waitpid, and if they exist
 * we simulate the result as if waitpid had returned the desired value.
 **/
static pid_t linux_waitpid(pid_t pid, int *status, int options, bool *lwp_died)
{
   if (pid == -1)
   {
      int dead_lwp = find_dead_lwps();
      if (dead_lwp)
      {
         *status = 0;
         *lwp_died = true;
         return dead_lwp;
      }
   }
   *lwp_died = false;
   return waitpid(pid, status, options);
}

// returns true if got event, false otherwise
bool checkForEventLinux(procevent *new_event, int wait_arg, 
                        bool block, int wait_options)
{
   int result = 0, status = 0;
   bool dead_lwp = 0;
   //  wait (check) for process events
   if (!block) {
      wait_options |= WNOHANG;
   }

   //   result = waitpid( wait_arg, &status, wait_options );
   result = linux_waitpid( wait_arg, &status, wait_options, &dead_lwp );
   if (result < 0 && errno == ECHILD) {
      return false;
   } else if (result < 0) {
      perror("checkForEventLinux: waitpid failure");
   } else if(result == 0) {
      return false;
   }

   int pertinentPid = result;

   // Translate the signal into a why/what combo.
   // We can fake results here as well: translate a stop in fork
   // to a (SYSEXIT,fork) pair. Don't do that yet.
   process *pertinentProc = process::findProcess(pertinentPid);
   dyn_lwp *pertinentLWP  = NULL;
   dyn_thread *pertinentThread = NULL;
   if(pertinentProc) {
      // Got a signal, process is stopped.
      if(process::IndependentLwpControl() &&
         pertinentProc->getRepresentativeLWP() == NULL) {
         if (!pertinentProc->getInitialThread())
            return false; //We must be shutting down
         pertinentLWP = pertinentProc->getInitialThread()->get_lwp();
      } else {
         pertinentLWP = pertinentProc->getRepresentativeLWP();
      }
      pertinentProc->set_lwp_status(pertinentLWP, stopped);
   } else {
      extern pdvector<process*> processVec;
      for (unsigned u = 0; u < processVec.size(); u++) {
         process *curproc = processVec[u];
         if(! curproc)
            continue;
         dyn_lwp *curlwp = NULL;
         if( (curlwp = curproc->lookupLWP(pertinentPid)) ) {
            pertinentProc = curproc;
            pertinentLWP  = curlwp;
            pertinentProc->set_lwp_status(curlwp, stopped);
            break;
         }
      }
   }
   
   procSignalWhy_t  why  = procUndefined;
   procSignalWhat_t what = 0;
   procSignalInfo_t info = 0;
   bool process_exited = WIFEXITED(status) || dead_lwp;

   if(! pertinentProc)
      return false;

   if (process_exited && pertinentPid == pertinentProc->getPid())
   {
      // Main process exited via signal
      signal_printf("[%s:%u] - Main process exited\n", __FILE__, __LINE__);
      why = procExitedNormally;
      what = WEXITSTATUS(status);
   }
   else if (process_exited)
   {
      proccontrol_cerr << "[checkForEventLinux] - Recieved a thread deletion event for "
                       << pertinentLWP->get_lwp_id() << endl;
      signal_cerr << "[checkForEventLinux] - Recieved a thread deletion event for "
                  << pertinentLWP->get_lwp_id() << endl;
      // Thread exited via signal
      why = procSyscallEntry;
      what = SYS_lwp_exit;
   }
   else if (WIFSIGNALED(status)) {
      why = procExitedViaSignal;
      what = WTERMSIG(status);
      signal_printf("[%s:%u] - %d exited with signal %d\n", __FILE__, __LINE__, 
                    pertinentPid, what);
   }
   else if (WIFSTOPPED(status)) {
      // More interesting (and common) case.  This is where return value
      // faking would occur as well. procSignalled is a generic return.  For
      // example, we translate SIGILL to SIGTRAP in a few cases

      why = procSignalled;
      what = WSTOPSIG(status);

      signal_printf("[%s:%u] - %d was signaled with %d\n", __FILE__, __LINE__, 
                    pertinentPid, what);

      switch(what) {
        case SIGSTOP:
           errno = 0;
           if (!decodeRTSignal(pertinentProc, why, what, info)) {
              if (ESRCH == errno) {
                fprintf(stderr,"%s[%d]:  decodeRTSignal, errno = %d: %s\n", __FILE__, __LINE__,errno, strerror(errno));
                //why = procExitedViaSignal;
                why = procExitedNormally;
              }
           }
           break;
        case SIGTRAP:
        {
           Frame sigframe = pertinentLWP->getActiveFrame();

           if (pertinentProc->trampTrapMapping.defines(sigframe.getPC())) {
              why = procInstPointTrap;
              info = pertinentProc->trampTrapMapping[sigframe.getPC()];
              /*
              fprintf(stderr, "Trapping, address 0x%lx to 0x%lx\n",
                      sigframe.getPC(), info);
              */
           }

           // Trap on x86 gets reported as PC+sizeof(trap) == pc+1...
           // However, we handle that when we insert a trap (we add the expected PC)
           // So we never check -1
#if 0
           if (pertinentProc->trampTrapMapping.defines(sigframe.getPC()-1)) 
           {
              why = procInstPointTrap;
              info = pertinentProc->trampTrapMapping[sigframe.getPC()-1];
           }
#endif
           break;
        }
        case SIGCHLD:
           // Linux fork() sends a SIGCHLD once the fork has been created
           why = procForkSigChild;
           break;
           // Since we're not using detach-on-the-fly or any special trap handling,
           // we can nuke this and go back to using traps everywhere
        case SIGILL:
#if defined(arch_ia64)
            // IA64 still uses illegals; we want to "fake" a trap occasionally
            Address pc = getPC(pertinentPid);
            if ((pc == pertinentProc->dyninstlib_brk_addr) ||
                (pc == pertinentProc->main_brk_addr) ||
                pertinentProc->getDyn()->reachedLibHook(pc)) 
                what = SIGTRAP;
#endif
            break;

      }
   }

   (*new_event).proc = pertinentProc;
   (*new_event).lwp  = pertinentLWP;
   (*new_event).thr  = pertinentThread;
   (*new_event).why  = why;
   (*new_event).what = what;
   (*new_event).info = info;

   //   fprintf(stderr, "EVENT: Proc = %d, LWP = %d, Why = %d, What = %d, Wait_arg = %d\n",
   //           new_event->proc->getPid(), new_event->lwp->get_lwp_id(), 
	//   new_event->why, new_event->what, wait_arg);
   return true;
}

/**
 * We sometimes receive an event from checkForEventLinux that we 
 * don't want to deal with yet.  This function will insert the 
 * event into a vector that 
 **/

bool signalHandler::checkForProcessEvents(pdvector<procevent *> *events,
                                          int wait_arg, int &timeout)
{
   bool wait_on_initial_lwp = false;
   bool wait_on_spawned_lwp = false;

   if(wait_arg != -1) {
      if(process::findProcess(wait_arg))
         wait_on_initial_lwp = true;
      else wait_on_spawned_lwp = true;
   } else {
      wait_on_initial_lwp = true;
      wait_on_spawned_lwp = true;
   }

   /* If we're blocking, check to make sure we don't do so forever. */
   if( -1 == timeout ) {
      /* If we're waiting on just one process, only check it. */
      if( wait_arg > 0 ) { 
         process * proc = process::findProcess( wait_arg );
         assert( proc != NULL );
         
         /* Having to enumerate everything is broken. */
         if( proc->status() == exited 
             || proc->status() == stopped 
             || proc->status() == detached ) { return false; }
      }
      else {
         /* Iterate over all the processes.  Prove progress because the processVec
            may be empty. */
         bool noneLeft = true;

         for( unsigned i = 0; i < processVec.size(); i++ ) {
            if( processVec[i] != NULL ) {
               process * proc = processVec[i];
               
               /* Enumeration is broken, but I'm also wondering why we keep 
                  processes around that are 'exited.' */
               if( proc->status() != exited 
                   && proc->status() != stopped 
                   && proc->status() != detached ) { noneLeft = false;   }
             }
         }
         if( noneLeft ) { return false; }
      } /* end multiple-process wait */
   } /* end if we're blocking */
   	
   procevent *new_event = new procevent;
   bool gotevent = false;
   while(1) {
      if(wait_on_initial_lwp) {
         gotevent = checkForEventLinux(new_event, wait_arg, false, 0);
         if(gotevent)
            break;
      }
      if(wait_on_spawned_lwp) {
         gotevent = checkForEventLinux(new_event, wait_arg, false, 
                                             __WCLONE);
         if(gotevent)
            break;
      }
      if(! timeout) {
         // no event found
         delete new_event;
         break;
      } else {
         // a slight delay to lesson impact of spinning.  this is
         // particularly noticable when traps are hit at instrumentation
         // points (seems to occur frequently in test1).
         // *** important for performance ***

         int wait_time = timeout;
         if (timeout == -1) wait_time = 1; /*ms*/

#ifdef NOTDEF
         struct timeval slp;
         if (wait_time > 1000) {
           slp.tv_sec = wait_time / 1000;
           slp.tv_usec = (wait_time % 1000) /*ms*/ * 1000 /*us*/ ;
         }
         else {
           slp.tv_sec = 0;
           slp.tv_usec = wait_time /*ms*/ * 1000 /*us*/ ;
         }
         select(0,NULL,NULL,NULL, &slp);
#endif
         struct timespec slp, rem;
         if (wait_time > 1000) {
           slp.tv_sec = wait_time / 1000;
           slp.tv_nsec = (wait_time % 1000) /*ms*/ * 1000 /*us*/ * 1000 /*ns*/;
         }
         else {
           slp.tv_sec = 0;
           slp.tv_nsec = wait_time /*ms*/ * 1000 /*us*/ * 1000 /*ns*/;
         }
          //fprintf(stderr, "%s[%d]:  before nanosleep\n", __FILE__, __LINE__);
         nanosleep(&slp, &rem);
         //  can check remaining time to see if we have _really_ timed out
         //  (but do we really care?)

         if (timeout != -1) // if we're not blocking indefinitely
           timeout = 0; // we have timed out
      }
   }

   if(gotevent) {
      (*events).push_back(new_event);
      return true;
   } else
      return false;
}

void process::independentLwpControlInit() {
   if(multithread_capable()) {
      // On linux, if process found to be MT, there will be no
      // representativeLWP since there is no lwp which controls the entire
      // process for MT linux.
      real_lwps[representativeLWP->get_lwp_id()] = representativeLWP;
      representativeLWP = NULL;
   }
}

dyn_lwp *process::createRepresentativeLWP() {
   // the initial lwp has a lwp_id with the value of the pid

   // if we identify this linux process as multi-threaded, then later we will
   // adjust this lwp to be identified as a real lwp.
   dyn_lwp *initialLWP = createFictionalLWP(getPid());
   representativeLWP = initialLWP;
   // Though on linux, if process found to be MT, there will be no
   // representativeLWP since there is no lwp which controls the entire
   // process for MT linux.

   return initialLWP;
}

bool process::trapAtEntryPointOfMain(dyn_lwp *trappingLWP, Address)
{
    if (main_brk_addr == 0x0) return false;
    assert(trappingLWP);
    Frame active = trappingLWP->getActiveFrame();
    if (active.getPC() == main_brk_addr ||
        (active.getPC()-1) == main_brk_addr)
        return true;
    return false;
}

bool process::trapDueToDyninstLib(dyn_lwp *trappingLWP)
{
    // is the trap instr at dyninstlib_brk_addr?
    if (dyninstlib_brk_addr == 0) return false;
    assert(trappingLWP);
    Frame active = trappingLWP->getActiveFrame();
    if (active.getPC() == dyninstlib_brk_addr ||
        (active.getPC()-1) == dyninstlib_brk_addr)
        return true;
    return false;
}

bool process::setProcessFlags() {
    // None that I'm aware of -- bernat, 24FEB04
    return true;
}

bool process::unsetProcessFlags(){
    // As above
    return true;
}


void emitCallRel32(unsigned disp32, unsigned char *&insn);

static int lwp_kill(int pid, int sig)
{
  int result = P_tkill(pid, sig);
  if (result == -1 && errno == ENOSYS)
  {
     result = P_kill(pid, sig);
     proccontrol_cerr << "Sent " << sig << " to " << pid << " via kill\n";
  }
  else
    proccontrol_cerr << "Sent " << sig << " to " << pid << " via tkill\n";

  return result;
}


/**
 * Return the state of the process from /proc/pid/stat.
 * File format is:
 *   pid (executablename) state ...
 * where state is a character.  Returns '\0' on error.
 **/
static char getState(int pid)
{
  char procName[64];
  char sstat[256];
  char *status;
  int paren_level = 1;

  sprintf(procName,"/proc/%d/stat", pid);
  FILE *sfile = P_fopen(procName, "r");
  if (sfile == NULL) return '\0';
  fread( sstat, 1, 256, sfile );
  fclose( sfile );
  sstat[255] = '\0';
  status = sstat;
  
  while (*status != '\0' && *(status++) != '(');
  while (*status != '\0' && paren_level != 0)
  {
    if (*status == '(') paren_level++;
    if (*status == ')') paren_level--;
    status++;
  }
  while (*status == ' ') status++;
  return *status;
}

bool process::isRunning_() const {
  char result = getState(getpid());
  assert(result != '\0');
  return (result != 'T');
}

bool dyn_lwp::isRunning() const {
  char result = getState(get_lwp_id());
  assert(result != '\0');
  return (result != 'T');
}

/**
 * Reads events through a waitpid until a SIGSTOP blocks a LWP.
 *  p - The process to work with
 *  pid - If -1, then take a SIGSTOP from any lwp.  If non-zero then 
 *        only take SIGSTOPs from that pid.
 *  shouldBlock - Should we block until we get a SIGSTOP?
 **/
static dyn_lwp *doWaitUntilStopped(process *p, int pid, bool shouldBlock)
{
  procevent new_event;
  bool gotevent, suppress_conts;
  int result;
  dyn_lwp *stopped_lwp = NULL;
  pdvector<int> other_sigs;
  pdvector<int> other_lwps;
  unsigned i;

 /*
  fprintf(stderr, "%s[%d]:  doWaitUntilStopped, pid = %d, block = %s\n",
           __FILE__, __LINE__, pid, shouldBlock ? "true" : "false");
  proccontrol_cerr << "doWaitUntilStopped called on " << pid 
        << " (shouldBlock = " << shouldBlock << ")\n"; 
  */

  while (true)
  {
    gotevent = checkForEventLinux(&new_event, pid, shouldBlock, __WALL);
    if (!gotevent)
    {
       proccontrol_cerr << "\tDidn't get an event\n";
       break;
    }

    proccontrol_cerr << "\twhy = " << new_event.why 
          << ", what = " << new_event.what 
          << ", lwp = " << new_event.lwp->get_lwp_id() << endl;
   
    if (didProcReceiveSignal(new_event.why) && (new_event.what != SIGSTOP) || 
        didProcReceiveInstTrap(new_event.why))
    {
      /**
       * We caught a non-SIGTOP signal, let's throw it back.
       **/
       if (didProcReceiveSignal(new_event.why) && 
           new_event.what != SIGILL && new_event.what != SIGTRAP &&
           new_event.what != SIGFPE && new_event.what != SIGSEGV &&
           new_event.what != SIGBUS)
       {
          //We don't actually throw back signals that are caused by 
          // executing an instruction.  We can just drop these and
          // let the continueLWP_ re-execute the instruction and cause
          // it to be rethrown.
          other_sigs.push_back(new_event.what);
          other_lwps.push_back(new_event.lwp->get_lwp_id());
          proccontrol_cerr << "\tpostponing " << new_event.what << endl;
       }
       else if (didProcReceiveInstTrap(new_event.why) ||
                (didProcReceiveSignal(new_event.why) && 
                 new_event.what == SIGTRAP)) 
       {
          proccontrol_cerr << "\tReceived trap\n";
          new_event.lwp->changePC(new_event.lwp->getActiveFrame().getPC() - 1, 
                                  NULL);
       }
       else
       {
          proccontrol_cerr << "\tDropped " << new_event.what << endl;
       }

       new_event.lwp->continueLWP_(0);
       new_event.proc->set_lwp_status(new_event.lwp, running);
       continue;
    }

    suppress_conts =  (new_event.proc->getPid() != p->getPid() ||
                       didProcEnterSyscall(new_event.why) ||
                       didProcExitSyscall(new_event.why));
    if (suppress_conts)
    { 
      proccontrol_cerr << "\tHandled, no suppression\n";
      result = getSH()->handleProcessEvent(new_event);
      continue;
    }
    else
    {
      p->setSuppressEventConts(true);    
      result = getSH()->handleProcessEvent(new_event);
      p->setSuppressEventConts(false);
      proccontrol_cerr << "\tHandled, with suppression\n";
    }

    if (p->status() == exited)
    {
      proccontrol_cerr << "\tApp exited\n";
      return NULL;
    }

    if (didProcReceiveSignal(new_event.why) && (new_event.what == SIGSTOP))
    {
      proccontrol_cerr << "\tGot my SIGSTOP\n";
      stopped_lwp = new_event.lwp;
      break;
    }
  }

  assert(other_sigs.size() == other_lwps.size());
  for (i = 0; i < other_sigs.size(); i++)
  {
    //Throw back the extra signals we caught.
     proccontrol_cerr << "\tResending " << other_sigs[i] 
                      << "to " << other_lwps[i] << endl;
     lwp_kill(other_lwps[i], other_sigs[i]);
  }

  if (stopped_lwp)
    return stopped_lwp->status() == stopped ? stopped_lwp : NULL;
  else 
    return NULL;
}

bool dyn_lwp::removeSigStop()
{
  return (lwp_kill(get_lwp_id(), SIGCONT) == 0);
}

bool dyn_lwp::continueLWP_(int signalToContinueWith) {
   proccontrol_cerr << "Continuing LWP " << get_lwp_id() << " with " 
         << signalToContinueWith << endl;
   // we don't want to operate on the process in this state
   int arg3 = 0;
   int arg4 = 0;
   if(signalToContinueWith != dyn_lwp::NoSignal) {
      arg3 = 1;
      arg4 = signalToContinueWith;
   }

   if (proc()->suppressEventConts())
   {
     return false;
   }
   if (status() == exited)
   {
      return true;
   }

   ptraceOps++; 
   ptraceOtherOps++;

   int ret = P_ptrace(PTRACE_CONT, get_lwp_id(), arg3, arg4);
   if (ret == 0)
     return true;

   /**
    * A stopped ptrace'd process can be in two states on Linux.
    * If it blocked on a signal, but never received a PTRACE_CONT,
    * then it should be continued with a PTRACE_CONT.
    *
    * If the kernel couldn't find an awake thread to deliver the signal to,
    * it'll grab anyone (usually the last) and wake them up for long enough
    * to deliver the signal.  This leaves that thread is a different state
    * than the others, and it won't actually respond to the PTRACE_CONT.  
    **/
   ret = P_kill(get_lwp_id(), SIGCONT);
   if (ret == -1)
     return false;
   return true;
}

bool dyn_lwp::waitUntilStopped()
{
  if (status() == stopped)
  {
    return true;
  }

  return (doWaitUntilStopped(proc(), get_lwp_id(), true) != NULL);
}

bool dyn_lwp::stop_() 
{
  return (lwp_kill(get_lwp_id(), SIGSTOP) == 0);
} 

bool process::continueProc_(int sig)
{
  bool no_err = true;
  dyn_lwp *lwp;
  unsigned index = 0; 

  //Continue all real LWPs
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  while (lwp_iter.next(index, lwp))
  {
    if (lwp->status() == running || lwp->status() == exited)
      continue;
    bool result = lwp->continueLWP(sig);
    if (result)
      set_lwp_status(lwp, running);
    else
      no_err = false;
  }

  //Continue any representative LWP
  if (representativeLWP && representativeLWP->status() != running)
  {
    bool result = representativeLWP->continueLWP(sig);
    if (result)
      set_lwp_status(representativeLWP, running);
    else
      no_err = false;
  }

  return no_err;
}

bool process::stop_()
{
  int result;
  
  //Stop the main process
  result = P_kill(getPid(), SIGSTOP);
  if (result == -1) 
  {
    perror("Couldn't send SIGSTOP\n");
    return false;
  }

  dyn_lwp *lwp = waitUntilLWPStops();
  if (!lwp)
  {
    return false;
  }
  if (status() == exited) {
      return false;
  }
  //Stop all other LWPs
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  unsigned index = 0;
  
  while(lwp_iter.next(index, lwp))
  {
    lwp->pauseLWP(true);
  }

  return true;
}

bool process::waitUntilStopped()
{
  dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(real_lwps);
  dyn_lwp *lwp;
  unsigned index = 0;
  bool result = true;

  while(lwp_iter.next(index, lwp))
  {
    result &= lwp->waitUntilStopped();
  }

  return result;
}

dyn_lwp *process::waitUntilLWPStops()
{
  return doWaitUntilStopped(this, -1, true);
}

terminateProcStatus_t process::terminateProc_()
{
  if (kill( getPid(), SIGKILL )) {
    // Kill failed... 
    if (errno == ESRCH)
      return alreadyTerminated;
    else
      return terminateFailed;
  }
  else
    return terminateSucceeded;
}

void dyn_lwp::realLWP_detach_() 
{
   int result;
   if(! proc_->isAttached()) {
      if (! proc_->hasExited())
         cerr << "Detaching, but not attached" << endl;
      return;
   }
    
   if (status() != exited)
   {
      //An exited lwp has already auto-detached
      ptraceOps++;
      ptraceOtherOps++;
      result = deliverPtrace(PTRACE_DETACH, 1, SIGCONT);
   }
   remove_lwp_from_poll_list(get_lwp_id());
   return;
}

void dyn_lwp::representativeLWP_detach_() {
    // If the process is already exited, then don't call ptrace
    if(! proc_->isAttached())
        return;
    
    if (fd_) close(fd_);
    
    ptraceOps++;
    ptraceOtherOps++;
    deliverPtrace(PTRACE_DETACH, 1, SIGCONT);

    remove_lwp_from_poll_list(get_lwp_id());
    return;
}

bool process::dumpCore_(const pdstring/* coreFile*/) { return false; }

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   //  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
   return writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
  //    cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced
  //     << " len=" << amount << endl; cerr.flush();
   return writeDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
   return readDataSpace(inTraced, amount, const_cast<void*>( inSelf ));
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;
   Address w = 0x0;               /* ptrace I/O buffer */
   int len = sizeof(Address); /* address alignment of ptrace I/O requests */
   unsigned cnt;

   //cerr << "writeDataSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //    << " len=" << nbytes << endl; cerr.flush();

#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
   if (proc_->collectSaveWorldData) {
       codeRange *range = NULL;
       proc_->codeRangesByAddr_.precessor((Address)inTraced, range); //findCodeRangeByAddress((Address)inTraced);
       if(range){
	 mapped_object *mappedobj_ptr = range->is_mapped_object();
	 if (mappedobj_ptr) {
	   // If we're writing into a shared object, mark it as dirty.
	   // _Unless_ we're writing "__libc_sigaction"
	   int_function *func = range->is_function();
	   if ((! func) || (func->prettyName() != "__libc_sigaction")){
	     mappedobj_ptr->setDirty();
	   }
	 }
       }
   }
#endif
#endif

   ptraceOps++; ptraceBytes += nbytes;

   if (0 == nbytes)
      return true;

   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;	  

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) (ap-cnt), 0);

      if (errno)
         return false;

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) (ap-cnt), w))
         return false;

      if (len-cnt >= nbytes) 
         return true; /* done */
	  
      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }	  
	  
   /* Copy aligned portion */
   while (nbytes >= (u_int)len) {
      assert(0 == ((Address)ap) % len);
      memcpy(&w, dp, len);
      int retval =  P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w);
      if (retval < 0)
         return false;

      // Check...
      /*
      Address test;
      fprintf(stderr, "Writing %x... ", w);
      test = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);
      fprintf(stderr, "... got %x, lwp %d\n", test, get_lwp_id());
      */      
      dp += len;
      ap += len;
      nbytes -= len;
   }

   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write it back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);

      if (errno)
         return false;

      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w))
         return false;

   }

   return true;
}

#if 0
bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;
   Address w;               /* ptrace I/O buffer */
   int len = proc_->getAddressWidth(); /* address alignment of ptrace I/O requests */
   unsigned cnt;

   //cerr << "writeDataSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //    << " len=" << nbytes << endl; cerr.flush();

#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
   if (proc_->collectSaveWorldData) {
       codeRange *range = NULL;
	proc_->codeRangesByAddr_->precessor((Address)inTraced, range); //findCodeRangeByAddress((Address)inTraced);
	if(range){
	        shared_object *sharedobj_ptr = range->is_shared_object();
	       if (sharedobj_ptr) {
        	   // If we're writing into a shared object, mark it as dirty.
	           // _Unless_ we're writing "__libc_sigaction"
        	   int_function *func = range->is_function();
	           if ((! func) || (func->prettyName() != "__libc_sigaction")){
        	      sharedobj_ptr->setDirty();
		   }
	       }
	}
   }
#endif
#endif

   ptraceOps++; ptraceBytes += nbytes;

   if (0 == nbytes)
      return true;

   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;	  

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) (ap-cnt), 0, len);

      if (errno)
         return false;

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) (ap-cnt), w, len))
         return false;

      if (len-cnt >= nbytes) 
         return true; /* done */
	  
      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }	  
	  
   /* Copy aligned portion */
   while (nbytes >= (u_int)len) {
      assert(0 == ((Address)ap) % len);
      memcpy(&w, dp, len);
      int retval =  P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w, len);
      if (retval < 0)
         return false;

      // Check...
      /*
      Address test;
      fprintf(stderr, "Writing %x... ", w);
      test = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0, len);
      fprintf(stderr, "... got %x, lwp %d\n", test, get_lwp_id());
      */      
      dp += len;
      ap += len;
      nbytes -= len;
   }

   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write it back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0, len);

      if (errno)
         return false;

      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w, len))
         return false;

   }

   return true;
}
#endif

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     Address w = 0x0;               /* ptrace I/O buffer */
     int len = proc_->getAddressWidth(); /* address alignment of ptrace I/O requests */
     unsigned cnt;

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = ((Address)ap) % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;

	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) (ap-cnt), w, len);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
	       dp[i] = p[cnt+i];

	  if (len-cnt >= nbytes)
	       return true; /* done */

	  dp += len-cnt;
	  ap += len-cnt;
	  nbytes -= len-cnt;
     }

     /* Copy aligned portion */
     while (nbytes >= (u_int)len) {
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0, len);
	  if (errno)
	      return false;
	  memcpy(dp, &w, len);
	  dp += len;
	  ap += len;
	  nbytes -= len;
     }

     if (nbytes > 0) {
	  /* Some unaligned data remains */
	  unsigned char *p = (unsigned char *) &w;
	  
	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0, len);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < nbytes; i++)
	       dp[i] = p[i];
     }
     return true;
}

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
pdstring process::tryToFindExecutable(const pdstring & /* iprogpath */, int pid) {
  // We need to dereference the /proc link.
  // Case 1: multiple copies of the same file opened with multiple
  // pids will not match (and should)
  // Case 2: an exec'ed program will have the same /proc path,
  // but different program paths
  pdstring procpath = pdstring("/proc/") + pdstring(pid) + pdstring("/exe");
  char buf[1024];
  int chars_read = readlink(procpath.c_str(), buf, 1024);
  if (chars_read == -1) {
    // Note: the name could be too long. Not handling yet.
    perror("Reading file name from /proc entry");
    return procpath;
  }
  buf[chars_read] = 0;
  return pdstring(buf);
}


bool process::determineLWPs(pdvector<unsigned> &lwp_ids)
{
  char name[128];
  struct dirent *direntry;
  
  /**
   * Linux 2.6:
   **/
  sprintf(name, "/proc/%d/task", getPid());
  DIR *dirhandle = opendir(name);
  if (dirhandle)
  {
     //Only works on Linux 2.6
     while((direntry = readdir(dirhandle)) != NULL) {
        unsigned lwp_id = atoi(direntry->d_name);
        if (lwp_id) 
           lwp_ids.push_back(lwp_id);
     }
     closedir(dirhandle);
     return true;
  }
  /**
   * Linux 2.4:
   *
   * PIDs that are created by pthreads have a '.' prepending their name
   * in /proc.  We'll check all of those for the ones that have this lwp
   * as a parent pid.
   **/
  dirhandle = opendir("/proc");
  if (!dirhandle)
  {
     //No /proc directory.  I give up.  No threads for you.
     return false;
  } 
  while ((direntry = readdir(dirhandle)) != NULL)
  {
     if (direntry->d_name[0] != '.')
        continue;
     unsigned lwp_id = atoi(direntry->d_name+1);
     int lwp_ppid;
     if (!lwp_id) 
        continue;
     sprintf(name, "/proc/%d/stat", lwp_id);
     FILE *fd = fopen(name, "r");
     if (!fd)
        continue;
     fscanf(fd, "%*d %*s %*c %d", &lwp_ppid);
     fclose(fd);
     if (lwp_ppid != getPid())
        continue;
     lwp_ids.push_back(lwp_id);
  }
  closedir(dirhandle);
  lwp_ids.push_back(getPid());
  
  return true;
}

#if !defined(BPATCH_LIBRARY)
#ifdef PAPI
papiMgr* dyn_lwp::papi() {

  return proc()->getPapiMgr();

}
#endif
#endif


procSyscall_t decodeSyscall(process * /*p*/, procSignalWhat_t what)
{
   switch (what) {
      case SYS_fork:
         return procSysFork;
         break;
      case SYS_exec:
         return procSysExec;
         break;
      case SYS_exit:
         return procSysExit;
         break;
      case SYS_lwp_exit:
         return procLwpExit;
         break;
      default:
         return procSysOther;
         break;
   }
   assert(0);
   return procSysOther;
}

bool process::dumpImage( pdstring imageFileName ) {
	/* What we do is duplicate the original file,
	   and replace the copy's .text section with
	   the (presumably instrumented) in-memory
	   executable image.  Note that we don't seem
	   to be concerned with making sure that we
	   actually grab the instrumentation code itself... */
	
	/* Acquire the filename. */
   if (!mapped_objects.size()) {
      return false;
   }

   pdstring originalFileName = mapped_objects[0]->fullName();
	
	/* Use system() to execute the copy. */
	pdstring copyCommand = "cp " + originalFileName + " " + imageFileName;
   system( copyCommand.c_str() );

   /* Open the copy so we can use libelf to find the .text section. */
   int copyFD = open( imageFileName.c_str(), O_RDWR, 0 );
   if( copyFD < 0 ) { return false; }
   
   /* Start up the elven widgetry. */
   Elf_X elf( copyFD, ELF_C_READ );
   if (!elf.isValid()) return false;
   
   /* Acquire the shared names pointer. */
   Elf_X_Shdr elfSection = elf.get_shdr( elf.e_shstrndx() );
   Elf_X_Data elfData = elfSection.get_data();
   const char *sharedNames = elfData.get_string();

   /* Iterate over the sections to find the text section's
      offset, length, and base address. */
   Address offset = 0;
   Address length = 0;
   Address baseAddr = 0;
   
   for( int i = 0; i < elf.e_shnum(); ++i ) {
      elfSection = elf.get_shdr( i );
      const char * name = (const char *) &sharedNames[ elfSection.sh_name() ];
      
      if( P_strcmp( name, ".text" ) == 0 ) {
         offset = elfSection.sh_offset();
         length = elfSection.sh_size();
         baseAddr = elfSection.sh_addr();
         break;
      } /* end if we've found the text section */
   } /* end iteration over sections */

   /* Copy the code out of the mutatee. */
   char * codeBuffer = (char *)malloc( length );
   assert( codeBuffer != NULL );
   
   if( ! readTextSpace( (void *) baseAddr, length, codeBuffer ) ) {
      free( codeBuffer );
      elf.end();
      P_close( copyFD );
      return false;
   }

   /* Write that code to the image file. */
   lseek( copyFD, offset, SEEK_SET );
   write( copyFD, codeBuffer, length );

   /* Clean up. */
   free( codeBuffer );
   elf.end();
   P_close( copyFD );
   return true;
}

int getNumberOfCPUs()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}

//Returns true if the function is part of the PLT table
bool isPLT(int_function *f)
{
    const Object &obj = f->mod()->obj()->parse_img()->getObject();
    return obj.is_offset_in_plt(f->ifunc()->getOffset());
}

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).

int_function *instPoint::findCallee() {

  // Already been bound
  if (callee_) {
      return callee_;
  }  
  if (ipType_ != callSite) {
      // Assert?
      return NULL; 
  }
  if (isDynamicCall()) {
      return NULL;
  }

  assert(img_p_);
  image_func *icallee = img_p_->getCallee(); 
  if (icallee) {
      // Now we have to look up our specialized version
      // Can't do module lookup because of DEFAULT_MODULE...
      const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName());
      if (!possibles) {
          return NULL;
      }
      for (unsigned i = 0; i < possibles->size(); i++) {
          if ((*possibles)[i]->ifunc() == icallee) {
              callee_ = (*possibles)[i];
              return callee_;
          }
      }
      // No match... very odd
      assert(0);
      return NULL;
  }
  // Do this the hard way - an inter-module jump
  // get the target address of this function
  Address target_addr = img_p_->callTarget();
  if(!target_addr) {
      // this is either not a call instruction or an indirect call instr
      // that we can't get the target address
      return NULL;
  }

  // get the relocation information for this image
  const Object &obj = func()->obj()->parse_img()->getObject();
  const pdvector<relocationEntry> *fbt;
  if(!obj.get_func_binding_table_ptr(fbt)) {
      return false; // target cannot be found...it is an indirect call.
  }
  
  Address base_addr = func()->obj()->codeBase();
   // find the target address in the list of relocationEntries
   for(u_int i=0; i < fbt->size(); i++) {
      if((*fbt)[i].target_addr() == target_addr) {
         // check to see if this function has been bound yet...if the
         // PLT entry for this function has been modified by the runtime
         // linker
         int_function *target_pdf = 0;
         if(proc()->hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
	   callee_ = target_pdf;
	   return callee_;  // target has been bound
         } 
         else {
	   pdvector<int_function *> pdfv;
	   bool found = proc()->findFuncsByMangled((*fbt)[i].name(), pdfv);
	   if(found) {
	     assert(pdfv.size());
#ifdef BPATCH_LIBRARY
	     if(pdfv.size() > 1)
	       cerr << __FILE__ << ":" << __LINE__ 
		    << ": WARNING:  findAllFuncsByName found " 
		    << pdfv.size() << " references to function " 
		    << (*fbt)[i].name() << ".  Using the first.\n";
#endif
	     callee_ = pdfv[0];
	     return callee_;
	   }
         }
         return NULL;
      }
   }
   return NULL;  
}

bool process::getExecFileDescriptor(pdstring filename,
                                    int /*pid*/,
                                    bool /*whocares*/,
                                    int &,
                                    fileDescriptor &desc)
{
    desc = fileDescriptor(filename, 
                          0, // code
                          0, // data
                          false); // a.out
    return true;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
#if !defined(arch_x86_64)
      lo = region_lo(near);
      hi = region_hi(near);  
#else
      if (getAddressWidth() == 8) {
	  lo = region_lo_64(near);
	  hi = region_hi_64(near);
      }
      else {
	  lo = region_lo(near);
	  hi = region_hi(near);  
      }
#endif
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif

bool dyn_lwp::realLWP_attach_() {
   char procName[128];
   sprintf(procName, "/proc/%d/mem", get_lwp_id());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) 
     fd_ = INVALID_HANDLE_VALUE;

   startup_cerr << "process::attach() doing PTRACE_ATTACH" << endl;

   if( 0 != P_ptrace(PTRACE_ATTACH, get_lwp_id(), 0, 0) )
   {
      perror( "process::attach - PTRACE_ATTACH" );
      return false;
   }
   
   add_lwp_to_poll_list(this);
   if (0 > waitpid(get_lwp_id(), NULL, __WCLONE)) {
      perror("process::attach - waitpid");
      exit(1);
   }

   if (proc_->status() == running)
      continueLWP();
   return true;
}

bool dyn_lwp::representativeLWP_attach_() {

   // step 1) /proc open: attach to the inferior process memory file
   char procName[128];
   sprintf(procName, "/proc/%d/mem", (int) proc_->getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) 
     fd_ = INVALID_HANDLE_VALUE;
   
   bool running = false;
   if( proc_->wasCreatedViaAttach() )
      running = proc_->isRunning_();
   
   // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
   // to paradynd in all cases?  How about when we are attaching to an
   // already-running process?  (Seems that in the latter case, no SIGTRAP
   // is automatically generated)
   
   // Only if we are really attaching rather than spawning the inferior
   // process ourselves do we need to call PTRACE_ATTACH
   if(proc_->wasCreatedViaAttach() || 
      proc_->wasCreatedViaFork() || 
      proc_->wasCreatedViaAttachToCreated())
   {
      startup_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
      if( 0 != P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) )
      {
         perror( "process::attach - PTRACE_ATTACH" );
         return false;
      }
      add_lwp_to_poll_list(this);

      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach - waitpid");
         exit(1);
      }
   }

   if(proc_->wasCreatedViaAttach() )
   {
      // If the process was running, it will need to be restarted, as
      // PTRACE_ATTACH kills it
      // Actually, the attach process contructor assumes that the process is
      // running.  While this is foolish, let's play along for now.
      if( proc_->status() != running || !proc_->isRunning_() ) {
         if( 0 != P_ptrace(PTRACE_CONT, getPid(), 0, 0) )
         {
            perror( "process::attach - continue 1" );
         }
      }
   }

   if(proc_->wasCreatedViaAttachToCreated())
   {
      // This case is a special situation. The process is stopped
      // in the exec() system call but we have not received the first 
      // TRAP because it has been caught by another process.
      
      /* lose race */
      sleep(1);
      
      /* continue, clearing pending stop */
      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
         perror("process::attach: PTRACE_CONT 1");
         return false;
      }
     
      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach: WAITPID");
         return false;
      }

      /* continue, resending the TRAP to emulate the normal situation*/
      if ( 0 > kill(getPid(), SIGTRAP)){
         perror("process::attach: KILL");
         return false;
      }
      
      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
         perror("process::attach: PTRACE_CONT 2");
         return false;
      }

      proc_->set_status(neonatal);
   } // end - if createdViaAttachToCreated

   return true;
}

// These constants are not defined in all versions of elf.h
#ifndef AT_NULL
#define AT_NULL 0
#endif
#ifndef AT_SYSINFO
#define AT_SYSINFO 32
#endif
#ifndef AT_SYSINFO_EHDR
#define AT_SYSINFO_EHDR 33
#endif

bool process::readAuxvInfo()
{
  /**
   * The location of the vsyscall is stored in /proc/PID/auxv in Linux 2.6
   * auxv consists of a list of name/value pairs, ending with the AT_NULL
   * name.  There isn't a direct way to get the vsyscall info on Linux 2.4
   **/
  char buffer[32];
  int fd;
  Address dso_start = 0x0, text_start = 0x0;
  unsigned page_size = 0x0;
  struct {
    int type;
    Address value;
  } auxv_entry;
  
  sprintf(buffer, "/proc/%d/auxv", pid);

  fd = open(buffer, O_RDONLY);
  if (fd == -1)
  {
    //This is expected on linux 2.4 systems
      return false;
  }

  do {
    read(fd, &auxv_entry, sizeof(auxv_entry));
    if (auxv_entry.type == AT_SYSINFO)
      text_start = auxv_entry.value;
    else if (auxv_entry.type == AT_SYSINFO_EHDR)
      dso_start = auxv_entry.value;
    else if (auxv_entry.type == AT_PAGESZ)
      page_size = auxv_entry.value;
  } while (auxv_entry.type != AT_NULL);

  P_close(fd);

  // FC3 hackage. If we didn't find the vsyscall page, guess
  if (text_start == 0x0 && dso_start == 0x0) {
#if defined( arch_x86 )
    cerr << "Warning: couldn't find vsyscall page, assuming addr of 0xffffe000" << endl;
    text_start = 0xffffe400;
    dso_start = 0xffffe000;
#else
	/* Untested; text_start unknown. */
	cerr << "Warning: couldn't find vsyscall page, assuming addr of 0xa000000000010000." << endl;
	dso_start = 0xa000000000010000;
	/* Garbage value to satisfy the assert; IA-64 doesn't care. */
	text_start = 0xa000000000010000;
#endif
  }
  
  assert(text_start != 0x0 && dso_start != 0x0);
  if (page_size == 0x0) page_size = getpagesize();
  
  vsyscall_start_ = dso_start;
  vsyscall_end_ = dso_start + page_size;
  vsyscall_text_ = text_start;

  return true;
}

void loadNativeDemangler() {}

const unsigned int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 4;
const char DYNINST_LOAD_HIJACK_FUNCTIONS[][20] = {
  "__libc_start_main",
  "_init",
  "_start",
  "main"
};

/**
 * Returns an address that we can use to write the code that executes
 * dlopen on the runtime library.
 *
 * Inserting the code into libc is a good thing, since _dl_open
 * will sometimes check it's caller and return with a 'invalid caller'
 * error if it's called from the application.
 **/
Address findFunctionToHijack(process *p) 
{
   Address codeBase;
   unsigned i;
   for(i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      const char *func_name = DYNINST_LOAD_HIJACK_FUNCTIONS[i];

      pdvector<int_function *> hijacks;
      if (!p->findFuncsByAll(func_name, hijacks))
          return 0;
      codeBase = hijacks[0]->getAddress();

      if (codeBase)
          break;
   }
   
  return codeBase;
} /* end findFunctionToHijack() */

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, pdstring func, 
                            pdvector<int_function *> &result)
{
   bool found = false;
   mapped_module *lpthread = p->findModule("libpthread*", true);
   if (lpthread)
      found = lpthread->findFuncVectorByPretty(func, result);
   if (found)
      return;

   mapped_module *lc = p->findModule("libc.so*", true);
   if (lc)
      found = lc->findFuncVectorByPretty(func, result);
   if (found)
      return;
   
   p->findFuncsByPretty(func, result);
}

bool process::initMT()
{
   unsigned i;
   bool res;

#if !defined(cap_threads)
   return true;
#endif

   /**
    * Instrument thread_create with calls to DYNINST_dummy_create
    **/
   //Find create_thread
   pdvector<int_function *> thread_init_funcs;
   findThreadFuncs(this, "create_thread", thread_init_funcs);
   findThreadFuncs(this, "start_thread", thread_init_funcs);
   if (thread_init_funcs.size() < 1)
   {
      fprintf(stderr, "[%s:%d] - Found no copies of create_thread, expected 1\n",
              __FILE__, __LINE__);
      return false;
   }
   //Find DYNINST_dummy_create
   int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
   if (!dummy_create)
   {
     fprintf(stderr, "[%s:%d] - Couldn't find DYNINST_dummy_create",
             __FILE__, __LINE__);
      return false;
   }
   //Instrument
   for (i=0; i<thread_init_funcs.size(); i++)
   {
      pdvector<AstNode *> args;
      AstNode call_dummy_create(dummy_create, args);
      AstNode *ast = &call_dummy_create;
      const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(ast, callPreInsn, orderFirstAtPoint, false, 
                                 false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                    __FILE__, __LINE__);
         }
         //TODO: Save the mt objects for detach
      }
   }
#if 0
      /*
   //Find functions that are run on pthread exit
   pdvector<int_function *> thread_dest_funcs;
   findThreadFuncs(this, "__pthread_do_exit", &thread_dest_funcs);
   findThreadFuncs(this, "pthread_exit", &thread_dest_funcs);
   findThreadFuncs(this, "deallocate_tsd", &thread_dest_funcs);
   if (!thread_dest_funcs.size())
   {
      fprintf(stderr,"[%s:%d] - Found 0 copies of pthread_exit, expected 1\n",
              __FILE__, __LINE__);
      return false;
   }
   //Find DYNINSTthreadDestroy
   int_function *threadDestroy = findOnlyOneFunction("DYNINSTthreadDestroy");
   if (!threadDestroy)
   {
      fprintf(stderr, "[%s:%d] - Couldn't find DYNINSTthreadDestroy",
              __FILE__, __LINE__);
      return false;
   }
   //Instrument
   for (i=0; i<thread_dest_funcs.size(); i++)
   {
      pdvector<AstNode *> args;
      AstNode call_thread_destroy(threadDestroy, args);
      AstNode *ast = &call_thread_destroy;
      miniTrampHandle *mthandle;
      instPoint *ip = thread_dest_funcs[i]->funcEntry(this);

      result = addInstFunc(this, mthandle, ip, ast, callPostInsn, 
                           orderFirstAtPoint, true, true, true);
      if (result != success_res)
      {
         fprintf(stderr, "[%s:%d] - Couldn't instrument thread_destroy\n",
                 __FILE__, __LINE__);
      }
   }
      */
#endif
   /**
    * Have dyn_pthread_self call the actual pthread_self
    **/
   //Find dyn_pthread_self
   pdvector<int_variable *> ptself_syms;
   res = findVarsByAll("DYNINST_pthread_self", ptself_syms);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't find any dyn_pthread_self, expected 1\n",
              __FILE__, __LINE__);
   }
   assert(ptself_syms.size() == 1);
   Address dyn_pthread_self = ptself_syms[0]->getAddress();
   //Find pthread_self
   pdvector<int_function *> pthread_self_funcs;
   findThreadFuncs(this, "pthread_self", pthread_self_funcs);   
   if (pthread_self_funcs.size() != 1)
   {
      fprintf(stderr, "[%s:%d] - Found %d pthread_self functions, expected 1\n",
              __FILE__, __LINE__, pthread_self_funcs.size());
      for (unsigned j=0; j<pthread_self_funcs.size(); j++)
      {
         int_function *ps = pthread_self_funcs[j];
         fprintf(stderr, "[%s:%u] - %s in module %s at %x\n", __FILE__, __LINE__,
                 ps->prettyName().c_str(), ps->mod()->fullName().c_str(), 
                 ps->getAddress());
      }
      return false;
   }   
   //Replace
   res = writeFunctionPtr(this, dyn_pthread_self, pthread_self_funcs[0]);
   if (!res)
   {
      fprintf(stderr, "[%s:%d] - Couldn't update dyn_pthread_self\n",
              __FILE__, __LINE__, pthread_self_funcs.size());
      return false;
   }
   return true;
}

void dyninst_yield()
{
   pthread_yield();
}
