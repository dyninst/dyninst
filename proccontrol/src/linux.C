/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/dyntypes.h"
#include "common/h/SymLite-elf.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Mailbox.h"

#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/linux.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/response.h"
#include "proccontrol/src/int_event.h"

#include "proccontrol/src/snippets.h"

#include "common/h/linuxKludges.h"
#include "common/h/parseauxv.h"

using namespace Dyninst;
using namespace std;

static GeneratorLinux *gen = NULL;

Generator *Generator::getDefaultGenerator()
{
   if (!gen) {
      gen = new GeneratorLinux();
      assert(gen);
      gen->launch();
   }
   return static_cast<Generator *>(gen);
}

void Generator::stopDefaultGenerator()
{
    if(gen) delete gen;
}

bool GeneratorLinux::initialize()
{
   return true;
}

bool GeneratorLinux::canFastHandle()
{
   return false;
}

ArchEvent *GeneratorLinux::getEvent(bool block)
{
   int status, options;

   //Block (or not block) in waitpid to receive a OS event
   options = __WALL;
   options |= block ? 0 : WNOHANG; 
   pthrd_printf("%s in waitpid\n", block ? "blocking" : "polling");
   int pid = waitpid(-1, &status, options);

   ArchEventLinux *newevent = NULL;
   if (pid == -1) {
      int errsv = errno;
      if (errsv == EINTR) {
         pthrd_printf("waitpid interrupted\n");
         newevent = new ArchEventLinux(true);
         return newevent;
      }
      perr_printf("Error. waitpid recieved error %s\n", strerror(errsv));
      newevent = new ArchEventLinux(errsv);
      return newevent;
   }

   if (dyninst_debug_proccontrol)
   {
      pthrd_printf("Waitpid return status %d for pid %d:\n", status, pid);
      if (WIFEXITED(status))
         pthrd_printf("Exited with %d\n", WEXITSTATUS(status));
      else if (WIFSIGNALED(status))
         pthrd_printf("Exited with signal %d\n", WTERMSIG(status));
      else if (WIFSTOPPED(status))
         pthrd_printf("Stopped with signal %d\n", WSTOPSIG(status));
#if defined(WIFCONTINUED)
      else if (WIFCONTINUED(status))
         perr_printf("Continued with signal SIGCONT (Unexpected)\n");
#endif
      else 
         pthrd_printf("Unable to interpret waitpid return.\n");
   }

   newevent = new ArchEventLinux(pid, status);
   return newevent;
}

GeneratorLinux::GeneratorLinux() :
   GeneratorMT(std::string("Linux Generator"))
{
   decoders.insert(new DecoderLinux());
}

GeneratorLinux::~GeneratorLinux()
{
}

DecoderLinux::DecoderLinux()
{
}

DecoderLinux::~DecoderLinux()
{
}

unsigned DecoderLinux::getPriority() const 
{
   return Decoder::default_priority;
}

Dyninst::Address DecoderLinux::adjustTrapAddr(Dyninst::Address addr, Dyninst::Architecture arch)
{
  if (arch == Dyninst::Arch_x86 || arch == Dyninst::Arch_x86_64) {
    return addr-1;
  }
  return addr;
}

bool DecoderLinux::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
   bool result;
   ArchEventLinux *archevent = static_cast<ArchEventLinux *>(ae);

   int_process *proc = NULL;
   linux_process *lproc = NULL;
   int_thread *thread = ProcPool()->findThread(archevent->pid);
   linux_thread *lthread = NULL;
   if (thread) {
      proc = thread->llproc();
      lthread = static_cast<linux_thread *>(thread);
   }
   if (proc) {
      lproc = dynamic_cast<linux_process *>(proc);
   }

   Event::ptr event = Event::ptr();
   ArchEventLinux *child = NULL;
   ArchEventLinux *parent = NULL;

   pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
                thread ? thread->getLWP() : -1);

   const int status = archevent->status;
   if (WIFSTOPPED(status))
   {
      const int stopsig = WSTOPSIG(status);
      int ext;
      pthrd_printf("Decoded to signal %d\n", stopsig);
      switch (stopsig)
      {
         case SIGSTOP:
            if (!proc) {               
               //The child half of an event pair.  Find the parent or postpone it.
               if (!archevent->findPairedEvent(parent, child)) {
                  pthrd_printf("Child half of paired event, postponing decode " 
                               "until parent arrives\n");
                  archevent->postponePairedEvent();
                  return true;
               }
               break;
            }
            if (lthread->hasPendingStop()) {
               pthrd_printf("Recieved pending SIGSTOP on %d/%d\n", 
                            thread->llproc()->getPid(), thread->getLWP());
               event = Event::ptr(new EventStop());
               break;
            }
         case SIGTRAP: {
            ext = status >> 16;
            if (ext) {
               switch (ext) {
                  case PTRACE_EVENT_EXIT:
                     if (!proc || !thread) {
                        //Legacy event on old process. 
                        return true;
                     }
                     pthrd_printf("Decoded event to pre-exit on %d/%d\n",
                                  proc->getPid(), thread->getLWP());
                     if (thread->getLWP() == proc->getPid())
                        event = Event::ptr(new EventExit(EventType::Pre, 0));
                     else {
                        EventLWPDestroy::ptr lwp_ev = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Pre));
                        event = lwp_ev;
                        event->setThread(thread->thread());
                        lproc->decodeTdbLWPExit(lwp_ev);
                     }
                     thread->setExitingInGenerator(true);
                     break;
                  case PTRACE_EVENT_FORK: 
                  case PTRACE_EVENT_CLONE: {
                     pthrd_printf("Decoded event to %s on %d/%d\n",
                                  ext == PTRACE_EVENT_FORK ? "fork" : "clone",
                                  proc->getPid(), thread->getLWP());
                     if (!proc || !thread) {
                        //Legacy event on old process. 
                        return true;
                     }
                     unsigned long cpid_l = 0x0;
                     do_ptrace((pt_req) PTRACE_GETEVENTMSG, (pid_t) thread->getLWP(), 
                               NULL, &cpid_l);
                     pid_t cpid = (pid_t) cpid_l;                     
                     archevent->child_pid = cpid;
                     archevent->event_ext = ext;
                     if (!archevent->findPairedEvent(parent, child)) {
                        pthrd_printf("Parent half of paired event, postponing decode "
                                     "until child arrives\n");
                        archevent->postponePairedEvent();
                        return true;
                     }
                     break;
                  }
                  case PTRACE_EVENT_EXEC: {
                     pthrd_printf("Decoded event to exec on %d/%d\n",
                                  proc->getPid(), thread->getLWP());
                     if (!proc || !thread) {
                        //Legacy event on old process. 
                        return true;
                     }
                     event = Event::ptr(new EventExec(EventType::Post));
                     event->setSyncType(Event::sync_process);
                     break;
                  }
               }
               break;
            }
            if (proc->getState() == int_process::neonatal_intermediate) {
               pthrd_printf("Decoded event to bootstrap on %d/%d\n",
                            proc->getPid(), thread->getLWP());
               event = Event::ptr(new EventBootstrap());
               break;
            }
            Dyninst::MachRegisterVal addr;
            Dyninst::Address adjusted_addr;
            result = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
            if (!result) {
               perr_printf("Failed to read PC address upon SIGTRAP\n");
               return false;
            }
            adjusted_addr = adjustTrapAddr(addr, proc->getTargetArch());

            if (rpcMgr()->isRPCTrap(thread, adjusted_addr)) {
               pthrd_printf("Decoded event to rpc completion on %d/%d at %lx\n",
                            proc->getPid(), thread->getLWP(), adjusted_addr);
               event = Event::ptr(new EventRPC(thread->runningRPC()->getWrapperForDecode()));
               break;
            }

            installed_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);
            if (ibp && ibp != thread->isClearingBreakpoint()) {
               pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(), 
                            thread->getLWP(), adjusted_addr);
               EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(adjusted_addr, ibp));
               event = event_bp;
               event->setThread(thread->thread());

               if (adjusted_addr == lproc->getLibBreakpointAddr()) {
                  pthrd_printf("Breakpoint is library load/unload\n");
                  EventLibrary::ptr lib_event = EventLibrary::ptr(new EventLibrary());
                  lib_event->setThread(thread->thread());
                  lib_event->setProcess(proc->proc());
                  lproc->decodeTdbLibLoad(lib_event);
                  event->addSubservientEvent(lib_event);
                  
                  break;
               }
               if (lproc->decodeTdbBreakpoint(event_bp)) {
                  pthrd_printf("Breakpoint was thread event\n");
                  break;
               }
               break;
            }
            if (thread->singleStep())
            {
               installed_breakpoint *ibp = thread->isClearingBreakpoint();
               if (ibp) {
                  pthrd_printf("Decoded event to breakpoint cleanup\n");
                  event = Event::ptr(new EventBreakpointClear(ibp));
                  break;
               } 
               else {
                  pthrd_printf("Decoded event to single step on %d/%d\n",
                               proc->getPid(), thread->getLWP());
                  event = Event::ptr(new EventSingleStep());
                  break;
               }
            }
         }
         default:
            pthrd_printf("Decoded event to signal %d on %d/%d\n",
                         stopsig, proc->getPid(), thread->getLWP());
#if 0
            //Debugging code
            if (stopsig == 11) {
               Dyninst::MachRegisterVal addr;
               result = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
               if (!result) {
                  fprintf(stderr, "Failed to read PC address upon crash\n");
               }
               fprintf(stderr, "Got crash at %lx\n", addr);               
               while (1) sleep(1);
            }
#endif
            event = Event::ptr(new EventSignal(stopsig));
      }
      if (event && event->getSyncType() == Event::unset)
         event->setSyncType(Event::sync_thread);
   }
   else if ((WIFEXITED(status) || WIFSIGNALED(status)) && 
            (!proc || !thread || thread->getGeneratorState() == int_thread::exited)) 
   {
      //This can happen if the debugger process spawned the 
      // child, but then detached.  We recieve the child process
      // exit (because we're the parent), but are no longer debugging it.
      // We'll just drop this event on the ground.
      //Also seen when multiple termination signals hit a multi-threaded process
      // we're debugging.  We'll keep pulling termination signals from the
      // a defunct process.  Similar to above, we'll drop this event
      // on the ground.
      return true;
   }
   else if (WIFEXITED(status) && proc->getPid() != thread->getLWP())
   {
      int exitcode = WEXITSTATUS(status);
      pthrd_printf("Decoded exit of thread %d/%d with code %d\n",
                   proc->getPid(), thread->getLWP(), exitcode);
      EventLWPDestroy::ptr lwp_ev = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Post));
      event = lwp_ev;
      event->setSyncType(Event::async);
      event->setThread(thread->thread());
      lproc->decodeTdbLWPExit(lwp_ev);
      thread->setGeneratorState(int_thread::exited);
   }
   else if (WIFEXITED(status) || WIFSIGNALED(status)) {
      if (WIFEXITED(status)) {
         int exitcode = WEXITSTATUS(status);
         pthrd_printf("Decoded event to exit of process %d/%d with code %d\n",
                      proc->getPid(), thread->getLWP(), exitcode);
         event = Event::ptr(new EventExit(EventType::Post, exitcode));
      }
      else {
         int termsig = WTERMSIG(status);
         if( proc->wasForcedTerminated() ) {
             pthrd_printf("Decoded event to force terminate of %d/%d\n",
                     proc->getPid(), thread->getLWP());
             event = Event::ptr(new EventForceTerminate(termsig));
         }else{
             pthrd_printf("Decoded event to crash of %d/%d with signal %d\n",
                          proc->getPid(), thread->getLWP(), termsig);
             event = Event::ptr(new EventCrash(termsig));
         }
      }
      event->setSyncType(Event::sync_process);
      int_threadPool::iterator i = proc->threadPool()->begin();
      for (; i != proc->threadPool()->end(); i++) {
         (*i)->setGeneratorState(int_thread::exited);
      }
   }

   if (parent && child)
   {
      //Paired event decoded
      assert(!event);
      thread = ProcPool()->findThread(parent->pid);
      assert(thread);
      proc = thread->llproc();
      if (parent->event_ext == PTRACE_EVENT_FORK)
         event = Event::ptr(new EventFork(child->pid));
      else if (parent->event_ext == PTRACE_EVENT_CLONE)
         event = Event::ptr(new EventNewLWP(child->pid));
      else 
         assert(0);
      event->setSyncType(Event::sync_thread);
      delete parent;
      delete child;
   }
   else {
      //Single event decoded
      assert(event);
      assert(!parent);
      assert(!child);
      assert(proc->proc());
      assert(thread->thread());
      delete archevent;
   }
   event->setThread(thread->thread());
   event->setProcess(proc->proc());
   events.push_back(event);

   return true;
}

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
   std::vector<std::string> a;
   std::map<int,int> f;
   std::vector<std::string> envp;
   LinuxPtrace::getPtracer(); //Make sure ptracer thread is initialized
   linux_process *newproc = new linux_process(p, e, a, envp, f);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, std::vector<std::string> envp, 
        std::map<int,int> f)
{
   LinuxPtrace::getPtracer(); //Make sure ptracer thread is initialized
   linux_process *newproc = new linux_process(0, e, a, envp, f);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *p)
{
   linux_process *newproc = new linux_process(pid_, p);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

static int computeAddrWidth(int pid)
{
#if defined(arch_64bit)
   /**
    * It's surprisingly difficult to figure out the word size of a process
    * without looking at the files it loads (we want to avoid disk accesses).
    *
    * /proc/PID/auxv offers a hackish opportunity to do this.  auxv contains
    * a list of name value pairs.  On 64 bit processes these name values are
    * a uint64/uint64 combo and on 32 bit processes they're uint32/uint32.
    *
    * The names are from a set of small integers (ranging from 0 to 37 at
    * the time of this writing).  Since these are small numbers, the top half
    * of name word will be 0x0 on 64 bit processes.  On 32-bit process this
    * word will contain a value, of which some should be non-zero.  
    *
    * We'll thus check every word that is 1 mod 4.  If all are 0x0 we assume we're
    * looking at a 64-bit process.
    **/
   uint32_t buffer[256];
   char auxv_name[64];
   
   snprintf(auxv_name, 64, "/proc/%d/auxv", pid);
   int fd = open(auxv_name, O_RDONLY);
   if (fd == -1) { 
      pthrd_printf("Couldn't open %s to determine address width: %s",
                   auxv_name, strerror(errno));
      return -1;
   }

   long int result = read(fd, buffer, sizeof(buffer));
   long int words_read = result / sizeof(uint32_t);
   int word_size = 8;
   for (long int i=1; i<words_read; i+= 4)
   {
      if (buffer[i] != 0) {
         word_size = 4;
         break;
      }
   }
   close(fd);
   return word_size;
#else
   return sizeof(void*);
#endif
}

Dyninst::Architecture linux_process::getTargetArch()
{
   if (arch != Dyninst::Arch_none) {
      return arch;
   }
   int addr_width = computeAddrWidth(getPid());
   
#if defined(arch_x86) || defined(arch_x86_64)
   assert(addr_width == 4 || addr_width == 8);
   arch = (addr_width == 4) ? Dyninst::Arch_x86 : Dyninst::Arch_x86_64;
#elif defined(arch_power)
   assert(addr_width == 4 || addr_width == 8);   
   arch = (addr_width == 4) ? Dyninst::Arch_ppc32 : Dyninst::Arch_ppc64;
#else
   assert(0);
#endif
   return arch;
}

linux_process::linux_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                             std::vector<std::string> envp,  std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   sysv_process(p, e, a, envp, f),
   unix_process(p, e, a, envp, f),
   x86_process(p, e, a, envp, f),
   thread_db_process(p, e, a, envp, f)
{
}

linux_process::linux_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   sysv_process(pid_, p),
   unix_process(pid_, p),
   x86_process(pid_, p),
   thread_db_process(pid_, p)
{
}

linux_process::~linux_process()
{
}

bool linux_process::plat_create()
{
   //Triggers plat_create_int on ptracer thread.
   return LinuxPtrace::getPtracer()->plat_create(this);
}

bool linux_process::plat_create_int()
{
   pid = fork();
   if (pid == -1)
   {
      int errnum = errno;
      pthrd_printf("Could not fork new process for %s: %s\n", 
                   executable.c_str(), strerror(errnum));
      setLastError(err_internal, "Unable to fork new process");
      return false;
   }

   if (!pid)
   {
      // Make sure cleanup on failure goes smoothly
      ProcPool()->condvar()->unlock();

      //Child
      long int result = ptrace((pt_req) PTRACE_TRACEME, 0, 0, 0);
      if (result == -1)
      {
         pthrd_printf("Failed to execute a PTRACE_TRACME.  Odd.\n");
         setLastError(err_internal, "Unable to debug trace new process");
         exit(-1);
      }

      // Never returns
      plat_execv();
   }
   return true;
}

bool linux_process::plat_attach()
{
   pthrd_printf("Attaching to pid %d\n", pid);
   int result = do_ptrace((pt_req) PTRACE_ATTACH, pid, NULL, NULL);
   if (result != 0) {
      int errnum = errno;
      pthrd_printf("Unable to attach to process %d: %s\n", pid, strerror(errnum));
      if (errnum == EPERM)
         setLastError(err_prem, "Do not have correct premissions to attach to pid");
      else if (errnum == ESRCH)
         setLastError(err_noproc, "The specified process was not found");
      else {
         setLastError(err_internal, "Unable to attach to the specified process");
      }
      return false;
   }
   
   return true;
}

static std::string deref_link(const char *path)
{
   char *p = realpath(path, NULL);
   if (p == NULL) {
      return std::string();
   }
   std::string sp = p;
   free(p);
   return sp;
}

bool linux_process::plat_execed()
{
   bool result = sysv_process::plat_execed();
   if (!result)
      return false;

   char proc_exec_name[128];
   snprintf(proc_exec_name, 128, "/proc/%d/exe", getPid());
   executable = deref_link(proc_exec_name);
   return true;
}

bool linux_process::plat_forked()
{
   return true;
}

bool linux_process::plat_readMem(int_thread *thr, void *local, 
                                 Dyninst::Address remote, size_t size)
{
   return LinuxPtrace::getPtracer()->ptrace_read(remote, size, local, thr->getLWP());
}

bool linux_process::plat_writeMem(int_thread *thr, const void *local, 
                                  Dyninst::Address remote, size_t size)
{
   return LinuxPtrace::getPtracer()->ptrace_write(remote, size, local, thr->getLWP());
}

static std::vector<unsigned int> fake_async_msgs;
void linux_thread::fake_async_main(void *)
{
   for (;;) {
      //Sleep for a small amount of time.
      struct timespec sleep_time;
      sleep_time.tv_sec = 0;
      sleep_time.tv_nsec = 1000000; //One milisecond
      nanosleep(&sleep_time, NULL);

      if (fake_async_msgs.empty())
         continue;

      getResponses().lock();
      
      //Pick a random async response to fill.
      int size = fake_async_msgs.size();
      int elem = rand() % size;
      unsigned int id = fake_async_msgs[elem];
      fake_async_msgs[elem] = fake_async_msgs[size-1];
      fake_async_msgs.pop_back();
      
      pthrd_printf("Faking response for event %d\n", id);
      //Pull the response from the list
      response::ptr resp = getResponses().rmResponse(id);
      assert(resp != response::ptr());
      
      //Add data to the response.
      reg_response::ptr regr = resp->getRegResponse();
      allreg_response::ptr allr = resp->getAllRegResponse();
      result_response::ptr resr = resp->getResultResponse();
      mem_response::ptr memr = resp->getMemResponse();
      if (regr)
         regr->postResponse(regr->val);
      else if (allr)
         allr->postResponse();
      else if (resr)
         resr->postResponse(resr->b);
      else if (memr)
         memr->postResponse();
      else
         assert(0);
      
      Event::ptr ev = resp->getEvent();
      if (ev == Event::ptr()) {
         //Someone is blocking for this response, mark it ready
         pthrd_printf("Marking response %s ready\n", resp->name().c_str());
         resp->markReady();
      }
      else {
         //An event triggered this async, create a new Async event
         // with the original event as subservient.
         int_eventAsync *internal = new int_eventAsync(resp);
         EventAsync::ptr async_ev(new EventAsync(internal));
         async_ev->setProcess(ev->getProcess());
         async_ev->setThread(ev->getThread());
         async_ev->setSyncType(Event::async);
         async_ev->addSubservientEvent(ev);
         
         pthrd_printf("Enqueueing Async event with subservient %s to mailbox\n", ev->name().c_str());
         mbox()->enqueue(async_ev, true);
         MTManager::eventqueue_cb_wrapper();
      }
      
      getResponses().signal();
      getResponses().unlock();
   }
}

bool linux_process::plat_needsAsyncIO() const
{
#if !defined(debug_async_simulate)
   return false;
#endif
   static DThread *fake_async_thread = NULL;
   if (!fake_async_thread) {
      fake_async_thread = new DThread();
      bool result = fake_async_thread->spawn(linux_thread::fake_async_main, NULL);
      assert(result);
   }
   return true;
}

bool linux_process::plat_readMemAsync(int_thread *thr, Dyninst::Address addr, mem_response::ptr result)
{
   bool b = plat_readMem(thr, result->getBuffer(), addr, result->getSize());
   if (!b) {
      result->markError(getLastError());      
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_process::plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr, size_t size, 
                                       result_response::ptr result)
{
   bool b = plat_writeMem(thr, local, addr, size);
   if (!b) {
      result->markError(getLastError());
      result->b = false;
   }
   else {
      result->b = true;
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_process::needIndividualThreadAttach()
{
   return true;
}

bool linux_process::plat_supportLWPEvents() const
{
   return true;
}

bool linux_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
   return findProcLWPs(pid, lwps);
}

int_process::ThreadControlMode int_process::getThreadControlMode() {
    return int_process::IndependentLWPControl;
}

bool linux_thread::plat_cont()
{
   pthrd_printf("Continuing thread %d\n", lwp);
   switch (handler_state) {
      case neonatal:
      case running:
      case exited:
      case errorstate:
         perr_printf("Continue attempted on thread in invalid state %s\n", 
                     int_thread::stateStr(handler_state));
         return false;
      case neonatal_intermediate:
      case stopped:
         //OK
         break;
   }

   // The following case poses a problem:
   // 1) This thread has received a signal, but the event hasn't been handled yet
   // 2) An event that precedes the signal event triggers a callback where
   //    the user requests that the whole process stop. This in turn causes
   //    the thread to be sent a SIGSTOP because the Handler hasn't seen the
   //    signal event yet.
   // 3) Before handling the pending signal event, this thread is continued to
   //    clear out the pending stop and consequently, it is delivered the signal
   //    which can cause the whole process to crash
   //
   // The solution:
   // Don't continue the thread with the pending signal if there is a pending stop.
   // Wait until the user sees the signal event to deliver the signal to the process.
   //
   // This also applies to iRPCs
   
   int tmpSignal = continueSig_;
   if( hasPendingStop() || runningRPC() ) {
       tmpSignal = 0;
   }

   void *data = (tmpSignal == 0) ? NULL : (void *) tmpSignal;
   int result;
   if (singleStep())
   {
      pthrd_printf("Calling PTRACE_SINGLESTEP with signal %d\n", tmpSignal);
      result = do_ptrace((pt_req) PTRACE_SINGLESTEP, lwp, NULL, data);
   }
   else 
   {
      pthrd_printf("Calling PTRACE_CONT with signal %d\n", tmpSignal);
      result = do_ptrace((pt_req) PTRACE_CONT, lwp, NULL, data);
   }
   if (result == -1) {
      int error = errno;
      perr_printf("low-level continue failed: %s\n", strerror(error));
      setLastError(err_internal, "Low-level continue failed\n");
      return false;
   }

   return true;
}

SymbolReaderFactory *linux_process::plat_defaultSymReader()
{
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
  return symreader_factory;
}


#ifndef SYS_tkill
#define SYS_tkill 238
#endif

pid_t P_gettid()
{
  static int gettid_not_valid = 0;
  long int result;

  if (gettid_not_valid)
    return getpid();

  result = syscall(SYS_gettid);
  if (result == -1 && errno == ENOSYS)
  {
    gettid_not_valid = 1;
    return getpid();
  }
  return (int) result;
}

static bool t_kill(int pid, int sig)
{
  static bool has_tkill = true;
  long int result = 0;
  pthrd_printf("Sending %d to %d\n", sig, pid);
  if (has_tkill) {
     result = syscall(SYS_tkill, pid, sig);
     if (result == -1 && errno == ENOSYS)
     {
        pthrd_printf("Using kill instead of tkill on this system\n");
        has_tkill = false;
     }
  }
  if (!has_tkill) {
     result = kill(pid, sig);
  }

  return (result == 0);
}

int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool initial_thrd)
{
   if (initial_thrd) {
      lwp_id = proc->getPid();
   }
   linux_thread *lthrd = new linux_thread(proc, thr_id, lwp_id);
   assert(lthrd);
   return static_cast<int_thread *>(lthrd);
}

linux_thread::linux_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   thread_db_thread(p, t, l)
{
}

linux_thread::~linux_thread()
{
}

bool linux_thread::plat_stop()
{
   bool result;

   assert(pending_stop);
   result = t_kill(lwp, SIGSTOP);
   if (!result) {
      int err = errno;
      if (err == ESRCH) {
         pthrd_printf("t_kill failed on %d, thread doesn't exist\n", lwp);
         setLastError(err_noproc, "Thread no longer exists");
         return false;
      }
      pthrd_printf("t_kill failed on %d: %s\n", lwp, strerror(err));
      setLastError(err_internal, "Could not send signal to process while stopping");
      return false;
   }

   return true;
}

void linux_thread::setOptions()
{
   long options = 0;
   options |= PTRACE_O_TRACECLONE;
   options |= PTRACE_O_TRACEEXIT;
   options |= PTRACE_O_TRACEFORK;
   options |= PTRACE_O_TRACECLONE;
   options |= PTRACE_O_TRACEEXEC;
   options |= PTRACE_O_TRACEFORK;

   if (options) {
      int result = do_ptrace((pt_req) PTRACE_SETOPTIONS, lwp, NULL, 
                          (void *) options);
      if (result == -1) {
         pthrd_printf("Failed to set options for %d: %s\n", tid, strerror(errno));
      }
   }   
}

bool linux_thread::getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
   switch (llproc()->getTargetArch())
   {
      case Arch_x86_64:
         // TODO
         // use ptrace_arch_prctl     
         pthrd_printf("Segment bases on x86_64 not implemented\n");
         return false;
      case Arch_x86: {
         MachRegister segmentSelectorReg;
         MachRegisterVal segmentSelectorVal;
         unsigned long entryNumber;
         struct user_desc entryDesc;

         switch (reg.val())
         {
            case x86::ifsbase: segmentSelectorReg = x86::fs; break;
            case x86::igsbase: segmentSelectorReg = x86::gs; break;
            default: {
               pthrd_printf("Failed to get unrecognized segment base\n");
               return false;
            }
         }

         if (!plat_getRegister(segmentSelectorReg, segmentSelectorVal))
         {
           pthrd_printf("Failed to get segment base with selector %s\n", segmentSelectorReg.name());
           return false;
         }
         entryNumber = segmentSelectorVal / 8;

         pthrd_printf("Get segment base doing PTRACE with entry %lu\n", entryNumber);
         long result = do_ptrace((pt_req) PTRACE_GET_THREAD_AREA, 
                                 lwp, (void *) entryNumber, (void *) &entryDesc);
         if (result == -1 && errno != 0) {
            pthrd_printf("PTRACE to get segment base failed: %s\n", strerror(errno));
            return false;
         }

         val = entryDesc.base_addr;
         pthrd_printf("Got segment base: 0x%lx\n", val);
         return true;
      }
      default:
         assert(0);
   }
}

bool linux_process::plat_individualRegAccess()
{
   return true;
}

bool linux_process::plat_detach()
{
   //ProcPool lock should be held.
   int_threadPool *tp = threadPool();
   bool had_error = false;
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      pthrd_printf("PTRACE_DETACH on %d\n", thr->getLWP());
      long result = do_ptrace((pt_req) PTRACE_DETACH, thr->getLWP(), NULL, (void *) 0);
      if (result == -1) {
         had_error = true;
         perr_printf("Failed to PTRACE_DETACH on %d/%d\n", getPid(), thr->getLWP());
         setLastError(err_internal, "PTRACE_DETACH operation failed\n");
      }
   }
   return !had_error;
}

bool linux_process::plat_terminate(bool &needs_sync)
{
   //ProcPool lock should be held.
   //I had been using PTRACE_KILL here, but that was proving to be inconsistent.
   
   pthrd_printf("Terminating process %d\n", getPid());
   int result = kill(getPid(), SIGKILL);
   if (result == -1) {
      if (errno == ESRCH) {
         perr_printf("Process %d no longer exists\n", getPid());
         setLastError(err_noproc, "Process no longer exists");
      }
      else {
         perr_printf("Failed to kill(%d, SIGKILL) process\n", getPid());
         setLastError(err_internal, "Unexpected failure of kill\n");
         return false;
      }
   }

   needs_sync = true;
   return true;
}

Dyninst::Address linux_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) {
    Dyninst::Address result = 0x0;
    bool found_result = false;
    unsigned maps_size;
    map_entries *maps = getVMMaps(getPid(), maps_size);
    assert(maps); //TODO, Perhaps go to libraries for address map if no /proc/
    for (unsigned i=0; i<maps_size; i++) {
        if (!(maps[i].prems & PREMS_EXEC))
            continue;
        if (min + size > maps[i].end)
            continue;
        if (maps[i].end - maps[i].start < size)
            continue;

        if (maps[i].start > min)
            result = maps[i].start;
        else
            result = min;
        found_result = true;
        break;
    }
    assert(found_result);
    free(maps);
    return result;
}

dynreg_to_user_t dynreg_to_user;
static void init_dynreg_to_user()
{
   static volatile bool initialized = false;
   static Mutex init_lock;
   if (initialized)
      return;
      
   init_lock.lock();
   if (initialized) {
      init_lock.unlock();
      return;
   }
    
   //Match the order of the 'user' structure to map registers correctly.
   int cur = 0;
   if (sizeof(void*) == 8) {
      /**
       * This is annoying, struct user is different for 64-bit processes debugging 
       * 32-bit processes.
       **/
      //r15
      cur+= 8; //r14
      cur+= 8; //r13
      cur+= 8; //r12
      dynreg_to_user[x86::ebp]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::ebx]   = make_pair(cur+=8, 4);
      cur+= 8; //r11
      cur+= 8; //r10
      cur+= 8; //r9
      cur+= 8; //r8
      dynreg_to_user[x86::eax]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::ecx]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::edx]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::esi]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::edi]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::oeax]  = make_pair(cur+=8, 4);
      dynreg_to_user[x86::eip]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::cs]    = make_pair(cur+=8, 4);
      dynreg_to_user[x86::flags] = make_pair(cur+=8, 4);
      dynreg_to_user[x86::esp]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::ss]    = make_pair(cur+=8, 4);
      dynreg_to_user[x86::fsbase]= make_pair(cur+=8, 4);
      dynreg_to_user[x86::gsbase]= make_pair(cur+=8, 4);
      dynreg_to_user[x86::ds]    = make_pair(cur+=8, 4);
      dynreg_to_user[x86::es]    = make_pair(cur+=8, 4);
      dynreg_to_user[x86::fs]    = make_pair(cur+=8, 4);
      dynreg_to_user[x86::gs]    = make_pair(cur+=8, 4);
   }
   else {
      dynreg_to_user[x86::ebx]   = make_pair(cur, 4);
      dynreg_to_user[x86::ecx]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::edx]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::esi]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::edi]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::ebp]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::eax]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::ds]    = make_pair(cur+=4, 4);
      dynreg_to_user[x86::es]    = make_pair(cur+=4, 4);
      dynreg_to_user[x86::fs]    = make_pair(cur+=4, 4);
      dynreg_to_user[x86::gs]    = make_pair(cur+=4, 4);
      dynreg_to_user[x86::oeax]  = make_pair(cur+=4, 4);
      dynreg_to_user[x86::eip]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::cs]    = make_pair(cur+=4, 4);
      dynreg_to_user[x86::flags] = make_pair(cur+=4, 4);
      dynreg_to_user[x86::esp]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::ss]    = make_pair(cur+=4, 4);
   }
   cur = 0;
   dynreg_to_user[x86_64::r15]    = make_pair(cur, 8);
   dynreg_to_user[x86_64::r14]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r13]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r12]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rbp]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rbx]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r11]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r10]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r9]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::r8]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rax]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rcx]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rdx]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rsi]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rdi]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::orax]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rip]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::cs]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::flags]  = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::rsp]    = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::ss]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::fsbase] = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::gsbase] = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::ds]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::es]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::fs]     = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::gs]     = make_pair(cur+=8, 8);

   initialized = true;

   init_lock.unlock();
}

//912 is currently the x86_64 size, 128 bytes for just-because padding
#define MAX_USER_SIZE (912+128)
bool linux_thread::plat_getAllRegisters(int_registerPool &regpool)
{
   volatile unsigned int sentinel1 = 0xfeedface;
   unsigned char user_area[MAX_USER_SIZE];
   volatile unsigned int sentinel2 = 0xfeedface;
   memset(user_area, 0, MAX_USER_SIZE);

   //If a sentinel assert fails, then someone forgot to increase MAX_USER_SIZE
   // for a new platform.
   assert(sentinel1 == 0xfeedface);
   int result = do_ptrace((pt_req) PTRACE_GETREGS, lwp, NULL, user_area);
   if (result != 0) {
      perr_printf("Error reading registers from %d\n", lwp);
      setLastError(err_internal, "Could not read user area from thread");
      return false;
   }
   assert(sentinel2 == 0xfeedface);

   init_dynreg_to_user();
   Dyninst::Architecture curplat = llproc()->getTargetArch();
   regpool.regs.clear();
   dynreg_to_user_t::iterator i;
   for (i = dynreg_to_user.begin(); i != dynreg_to_user.end(); i++)
   {
      const MachRegister reg = i->first;
      MachRegisterVal val;
      if (reg.getArchitecture() != curplat)
         continue;
      const unsigned int offset = i->second.first;
      const unsigned int size = i->second.second;
      if (size == 4) {
         val = *((uint32_t *) (user_area+offset));
      }
      else if (size == 8) {
         val = *((uint64_t *) (user_area+offset));
      }
      else {
         assert(0);
      }
      pthrd_printf("Register %s has value %lx, offset %d\n", reg.name(), val, offset);
      regpool.regs[reg] = val;
   }
   return true;
}

bool linux_thread::plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
   if (x86::fsbase == reg || x86::gsbase == reg 
       || x86_64::fsbase == reg || x86_64::gsbase == reg) {
      return getSegmentBase(reg, val);
   }

   init_dynreg_to_user();
   dynreg_to_user_t::iterator i = dynreg_to_user.find(reg);
   if (i == dynreg_to_user.end() || reg.getArchitecture() != llproc()->getTargetArch()) {
      perr_printf("Recieved unexpected register %s on thread %d\n", reg.name(), lwp);
      setLastError(err_badparam, "Invalid register");
      return false;
   }

   const unsigned offset = i->second.first;
   const unsigned size = i->second.second;
   assert(sizeof(val) >= size);
   val = 0;
   unsigned long result = do_ptrace((pt_req) PTRACE_PEEKUSR, lwp, (void *) offset, NULL);
   if (errno != 0) {
      perr_printf("Error reading registers from %d: %s\n", lwp, strerror(errno));
      setLastError(err_internal, "Could not read register from thread");
      return false;
   }
   val = result;

   pthrd_printf("Register %s has value 0x%lx\n", reg.name(), val);
   return true;
}

bool linux_thread::plat_setAllRegisters(int_registerPool &regpool) {
   unsigned char user_area[MAX_USER_SIZE];

   //Fill in 'user_area' with the contents of regpool.
   if( !plat_convertToSystemRegs(regpool, user_area) ) return false;

   int result = do_ptrace((pt_req) PTRACE_SETREGS, lwp, NULL, user_area);
   if (result != 0) {
      perr_printf("Error setting registers for %d\n", lwp);
      setLastError(err_internal, "Could not read user area from thread");
      return false;
   }
   pthrd_printf("Successfully set the values of all registers for %d\n", lwp);
   return true;
}

bool linux_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area) {
   init_dynreg_to_user();

   Architecture curplat = llproc()->getTargetArch();
   unsigned num_found = 0;
   for (dynreg_to_user_t::const_iterator i = dynreg_to_user.begin(); i != dynreg_to_user.end(); i++)
   {
      const MachRegister reg = i->first;
      MachRegisterVal val;
      if (reg.getArchitecture() != curplat)
         continue;
      num_found++;
      const unsigned int offset = i->second.first;
      const unsigned int size = i->second.second;
      assert(offset+size < MAX_USER_SIZE);
      
      int_registerPool::reg_map_t::const_iterator j = regpool.regs.find(reg);
      assert(j != regpool.regs.end());
      val = j->second;
      
      if (size == 4) {
          if( sizeof(void *) == 8 ) {
              // Zero unused memory
              *((uint64_t *) (user_area+offset)) = (uint64_t) 0;
          }
         *((uint32_t *) (user_area+offset)) = (uint32_t) val;
      }
      else if (size == 8) {
         *((uint64_t *) (user_area+offset)) = (uint64_t) val;
      }
      else {
         assert(0);
      }
      pthrd_printf("Register %s gets value %lx, offset %d\n", reg.name(), val, offset);
   }

   if (num_found != regpool.regs.size())
   {
      setLastError(err_badparam, "Invalid register set passed to setAllRegisters");
      perr_printf("Couldn't find all registers in the register set %u/%u\n", num_found,
                  (unsigned int) regpool.regs.size());
      return false;
   }
   assert(num_found == regpool.regs.size());

   return true;
}

bool linux_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
   init_dynreg_to_user();
   dynreg_to_user_t::iterator i = dynreg_to_user.find(reg);
   if (reg.getArchitecture() != llproc()->getTargetArch() ||
       i == dynreg_to_user.end()) 
   {
      setLastError(err_badparam, "Invalid register passed to setRegister");
      perr_printf("User passed invalid register %s to plat_setRegister, arch is %x\n",
                  reg.name(), (unsigned int) reg.getArchitecture());
      return false;
   }
   
   const unsigned int offset = i->second.first;
   const unsigned int size = i->second.second;
   int result;
   if (size == 4) {
      uint32_t value = (uint32_t) val;
      result = do_ptrace((pt_req) PTRACE_POKEUSR, lwp, (void *) offset, (void *) value);
   }
   else if (size == 8) {
      uint64_t value = (uint64_t) val;
      result = do_ptrace((pt_req) PTRACE_POKEUSR, lwp, (void *) offset, (void *) value);
   }
   else {
      assert(0);
   }
   pthrd_printf("Set register %s (size %u, offset %u) to value %lx\n", reg.name(), size, offset, val);
   if (result != 0) {
      int error = errno;
      setLastError(err_internal, "Could not set register value");
      perr_printf("Unable to set value of register %s in thread %d: %s\n",
                  reg.name(), lwp, strerror(error));
      return false;
   }
   
   return true;
}

bool linux_thread::plat_getAllRegistersAsync(allreg_response::ptr result)
{
   bool b = plat_getAllRegisters(*result->getRegPool());
   if (!b) {
      result->markError(getLastError());
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_thread::plat_getRegisterAsync(Dyninst::MachRegister reg, 
                                         reg_response::ptr result)
{
   Dyninst::MachRegisterVal val = 0;
   bool b = plat_getRegister(reg, val);
   result->val = val;
   if (!b) {
      result->markError(getLastError());
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_thread::plat_setAllRegistersAsync(int_registerPool &pool,
                                             result_response::ptr result)
{
   bool b = plat_setAllRegisters(pool);
   if (!b) {
      result->markError(getLastError());
      result->b = false;
   }
   else {
      result->b = true;
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_thread::plat_setRegisterAsync(Dyninst::MachRegister reg, 
                                         Dyninst::MachRegisterVal val,
                                         result_response::ptr result)
{
   bool b = plat_setRegister(reg, val);
   if (!b) {
      result->markError(getLastError());
      result->b = false;
   }
   else {
      result->b = true;
   }
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_thread::attach()
{
   if (llproc()->threadPool()->initialThread() == this) {
      return true;
   }

   if (llproc()->getState() != int_process::neonatal &&
       llproc()->getState() != int_process::neonatal_intermediate)
   {
      pthrd_printf("thread::attach called on running thread %d/%d, should " 
                   "be auto-attached.\n", llproc()->getPid(), lwp);
      return true;
   }
   assert(getInternalState() == neonatal);

   pthrd_printf("Calling PTRACE_ATTACH on thread %d/%d\n", 
                llproc()->getPid(), lwp);
   int result = do_ptrace((pt_req) PTRACE_ATTACH, lwp, NULL, NULL);
   if (result != 0) {
      perr_printf("Failed to attach to thread: %s\n", strerror(errno));
      setLastError(err_internal, "Failed to attach to thread");
      return false;
   }
   return true;
}

#if !defined(ARCH_GET_FS)
#define ARCH_GET_FS 0x1003
#endif
#if !defined(ARCH_GET_GS)
#define ARCH_GET_GS 0x1004
#endif
#if !defined(PTRACE_GET_THREAD_AREA)
#define PTRACE_GET_THREAD_AREA 25
#endif
#if !defined(PTRACE_ARCH_PRCTL)
#define PTRACE_ARCH_PRCTL 30
#endif
#define FS_REG_NUM 25
#define GS_REG_NUM 26

bool linux_thread::plat_getThreadArea(int val, Dyninst::Address &addr)
{
   Dyninst::Architecture arch = llproc()->getTargetArch();
   switch (arch) {
      case Arch_x86: {
         uint32_t addrv[4];
         int result = do_ptrace((pt_req) PTRACE_GET_THREAD_AREA, lwp, (void *) val, &addrv);
         if (result != 0) {
            int error = errno;
            perr_printf("Error doing PTRACE_GET_THREAD_AREA on %d/%d: %s\n", llproc()->getPid(), lwp, strerror(error));
            setLastError(err_internal, "Error doing PTRACE_GET_THREAD_AREA\n");
            return false;
         }
         addr = (Dyninst::Address) addrv[1];
         break;
      }
      case Arch_x86_64: {
         int op;
         if (val == FS_REG_NUM)
            op = ARCH_GET_FS;
         else if (val == GS_REG_NUM)
            op = ARCH_GET_GS;
         else {
            perr_printf("Bad value (%d) passed to plat_getThreadArea\n", val);
            return false;
         }
         uint64_t addrv;
         int result = do_ptrace((pt_req) PTRACE_ARCH_PRCTL, lwp, &addrv, (void *) op);
         if (result != 0) {
            int error = errno;
            perr_printf("Error doing PTRACE_ARCH_PRCTL on %d/%d: %s\n", llproc()->getPid(), lwp, strerror(error));
            setLastError(err_internal, "Error doing PTRACE_ARCH_PRCTL\n");
            return false;
         }
         addr = (Dyninst::Address) addrv;
         break;
      }
      default:
         assert(0); //Should not be needed on non-x86
   }
   return true;
}

ArchEventLinux::ArchEventLinux(bool inter_) : 
   status(0),
   pid(NULL_PID),
   interrupted(inter_),
   error(0),
   child_pid(NULL_PID),
   event_ext(0)
{
}

ArchEventLinux::ArchEventLinux(pid_t p, int s) : 
   status(s),
   pid(p), 
   interrupted(false), 
   error(0),
   child_pid(NULL_PID)
{
}

ArchEventLinux::ArchEventLinux(int e) : 
   status(0),
   pid(NULL_PID),
   interrupted(false),
   error(e),
   child_pid(NULL_PID)
{
}
      
ArchEventLinux::~ArchEventLinux()
{
}

std::vector<ArchEventLinux *> ArchEventLinux::pending_events;

bool ArchEventLinux::findPairedEvent(ArchEventLinux* &parent, ArchEventLinux* &child)
{
   bool is_parent;
   if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      //'this' event is a parent, search list for a child
      is_parent = true;
   }
   else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP) {
      //'this' event  is a child, search list for a parent
      is_parent = false;
   }
   else
      assert(0);

   vector<ArchEventLinux *>::iterator i;
   for (i = pending_events.begin(); i != pending_events.end(); i++) {
      parent = is_parent ? this : *i;
      child = is_parent ? *i : this;
      if (parent->child_pid == child->pid) {
         pending_events.erase(i);
         return true;
      }
   }
   return false;
}

void ArchEventLinux::postponePairedEvent()
{
   pending_events.push_back(this);
}

LinuxHandleNewThr::LinuxHandleNewThr() :
   Handler("Linux New Thread")
{
}

LinuxHandleNewThr::~LinuxHandleNewThr()
{
}

Handler::handler_ret_t LinuxHandleNewThr::handleEvent(Event::ptr ev)
{
   linux_thread *thr = NULL;
   if (ev->getEventType().code() == EventType::Bootstrap) {
      thr = static_cast<linux_thread *>(ev->getThread()->llthrd());
   }
   else if (ev->getEventType().code() == EventType::ThreadCreate) {
      Dyninst::LWP lwp = static_cast<EventNewThread *>(ev.get())->getLWP();
      ProcPool()->condvar()->lock();
      thr = static_cast<linux_thread *>(ProcPool()->findThread(lwp));
      ProcPool()->condvar()->unlock();
   }
   assert(thr);
                                        
   pthrd_printf("Setting ptrace options for new thread %d\n", thr->getLWP());
   thr->setOptions();
   return ret_success;
}

int LinuxHandleNewThr::getPriority() const
{
   return PostPlatformPriority;
}

void LinuxHandleNewThr::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::ThreadCreate));
   etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool initialized = false;
   static LinuxHandleNewThr *lbootstrap = NULL;
   if (!initialized) {
      lbootstrap = new LinuxHandleNewThr();
      initialized = true;
   }
   hpool->addHandler(lbootstrap);
   thread_db_process::addThreadDBHandlers(hpool);
   return hpool;
}

bool ProcessPool::LWPIDsAreUnique()
{
   return true;
}

LinuxPtrace *LinuxPtrace::linuxptrace = NULL;

long do_ptrace(pt_req request, pid_t pid, void *addr, void *data)
{
   return LinuxPtrace::getPtracer()->ptrace_int(request, pid, addr, data);
}

LinuxPtrace *LinuxPtrace::getPtracer()
{
   if (!linuxptrace) {
      linuxptrace = new LinuxPtrace();
      assert(linuxptrace);
      linuxptrace->start();
   }
   return linuxptrace;
}


LinuxPtrace::LinuxPtrace() :
   ptrace_request(unknown),
   request((pt_req) 0),
   pid(0),
   addr(NULL),
   data(NULL),
   size(0),
   ret(0)
{
}

LinuxPtrace::~LinuxPtrace()
{
}

static void start_ptrace(void *lp)
{
   LinuxPtrace *linuxptrace = (LinuxPtrace *) (lp);
   linuxptrace->main();
}

void LinuxPtrace::start()
{
   init.lock();
   thrd.spawn(start_ptrace, this);
   init.wait();
   init.unlock();
}

void LinuxPtrace::main()
{
   init.lock();
   cond.lock();
   init.signal();
   init.unlock();
   for (;;) {
      cond.wait();
      ret_lock.lock();
      switch(ptrace_request) {
         case create_req:
            bret = proc->plat_create_int();
	    break;
         case ptrace_req:
            ret = ptrace(request, pid, addr, data);
            break;
         case ptrace_bulkread:
            bret = PtraceBulkRead(remote_addr, size, data, pid);
            break;
         case ptrace_bulkwrite:
            bret = PtraceBulkWrite(remote_addr, size, data, pid);
            break;            
         case unknown:
            assert(0);
      }
      err = errno;
      ret_lock.signal();
      ret_lock.unlock();
   }
}

void LinuxPtrace::start_request()
{
   request_lock.lock();
   cond.lock();
   ret_lock.lock();
}

void LinuxPtrace::waitfor_ret()
{
   cond.signal();
   cond.unlock();
   ret_lock.wait();   
}

void LinuxPtrace::end_request()
{
   ret_lock.unlock();
   request_lock.unlock();
}

long LinuxPtrace::ptrace_int(pt_req request_, pid_t pid_, void *addr_, void *data_)
{
   start_request();

   ptrace_request = ptrace_req;
   request = request_;
   pid = pid_;
   addr = addr_;
   data = data_;

   waitfor_ret();
   
   long myret = ret;
   int my_errno = err;

   end_request();

   errno = my_errno;
   return myret;
}

bool LinuxPtrace::plat_create(linux_process *p)
{
   start_request();
   ptrace_request = create_req;
   proc = p;
   waitfor_ret();
   bool result = bret;
   end_request();
   return result;
}

bool LinuxPtrace::ptrace_read(Dyninst::Address inTrace, unsigned size_, 
                              void *inSelf, int pid_)
{
   start_request();
   ptrace_request = ptrace_bulkread;
   remote_addr = inTrace;
   data = inSelf;
   pid = pid_;
   size = size_;
   waitfor_ret();
   bool result = bret;
   end_request();
   return result;
}

bool LinuxPtrace::ptrace_write(Dyninst::Address inTrace, unsigned size_, 
                               const void *inSelf, int pid_)
{
   start_request();
   ptrace_request = ptrace_bulkwrite;
   remote_addr = inTrace;
   data = const_cast<void *>(inSelf);
   pid = pid_;
   size = size_;
   waitfor_ret();
   bool result = bret;
   end_request();
   return result;
}


const unsigned int x86_64_mmap_flags_position = 26;
const unsigned int x86_64_mmap_size_position = 43;
const unsigned int x86_64_mmap_addr_position = 49;
const unsigned int x86_64_mmap_start_position = 4;
const unsigned char x86_64_call_mmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop,nop,nop,nop
0x48, 0x8d, 0x64, 0x24, 0x80,                   //lea    -128(%rsp),%rsp
0x49, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r8
0x49, 0xc7, 0xc1, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r9
0x49, 0xc7, 0xc2, 0x22, 0x00, 0x00, 0x00,       //mov    $0x22,%r10
0x48, 0xc7, 0xc2, 0x07, 0x00, 0x00, 0x00,       //mov    $0x7,%rdx
0x48, 0x31, 0xf6,                               //xor    %rsi,%rsi
0x48, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00,       //mov    $<size>,%rsi
0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $<addr>,%rdi
0x00, 0x00, 0x00,                               //
0x48, 0xc7, 0xc0, 0x09, 0x00, 0x00, 0x00,       //mov    $0x9,%rax
0x0f, 0x05,                                     //syscall 
0x48, 0x8d, 0xa4, 0x24, 0x80, 0x00, 0x00, 0x00, //lea    128(%rsp),%rsp
0xcc,                                           //Trap
0x90                                            //nop
};
const unsigned int x86_64_call_mmap_size = sizeof(x86_64_call_mmap);

const unsigned int x86_64_munmap_size_position = 15;
const unsigned int x86_64_munmap_addr_position = 21;
const unsigned int x86_64_munmap_start_position = 4;
const unsigned char x86_64_call_munmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop,nop,nop,nop
0x48, 0x8d, 0x64, 0x24, 0x80,                   //lea    -128(%rsp),%rsp
0x48, 0x31, 0xf6,                               //xor    %rsi,%rsi
0x48, 0xc7, 0xc6, 0x00, 0x00, 0x00, 0x00,       //mov    $<size>,%rsi
0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $<addr>,%rdi
0x00, 0x00, 0x00,                               //
0x48, 0xc7, 0xc0, 0x0b, 0x00, 0x00, 0x00,       //mov    $0xb,%rax
0x0f, 0x05,                                     //syscall 
0x48, 0x8d, 0xa4, 0x24, 0x80, 0x00, 0x00, 0x00, //lea    128(%rsp),%rsp
0xcc,                                           //Trap
0x90                                            //nop
};
const unsigned int x86_64_call_munmap_size = sizeof(x86_64_call_munmap);


const unsigned int x86_mmap_flags_position = 20;
const unsigned int x86_mmap_size_position = 10;
const unsigned int x86_mmap_addr_position = 5;
const unsigned int x86_mmap_start_position = 4;
const unsigned char x86_call_mmap[] = {
   0x90, 0x90, 0x90, 0x90,                //nop; nop; nop; nop
   0xbb, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebx  (addr)
   0xb9, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ecx  (size)
   0xba, 0x07, 0x00, 0x00, 0x00,          //mov    $0x7,%edx  (perms)
   0xbe, 0x22, 0x00, 0x00, 0x00,          //mov    $0x22,%esi (flags)
   0xbf, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%edi  (fd)
   0xbd, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebp  (offset)
   0xb8, 0xc0, 0x00, 0x00, 0x00,          //mov    $0xc0,%eax (syscall)
   0xcd, 0x80,                            //int    $0x80
   0xcc,                                  //Trap
   0x90                                   //nop
};
const unsigned int x86_call_mmap_size = sizeof(x86_64_call_mmap);

const unsigned int x86_munmap_size_position = 10;
const unsigned int x86_munmap_addr_position = 5;
const unsigned int x86_munmap_start_position = 4;
const unsigned char x86_call_munmap[] = {
   0x90, 0x90, 0x90, 0x90,                //nop; nop; nop; nop
   0xbb, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ebx  (addr)
   0xb9, 0x00, 0x00, 0x00, 0x00,          //mov    $0x0,%ecx  (size)
   0xb8, 0xc0, 0x00, 0x00, 0x00,          //mov    $0x5b,%eax (syscall)
   0xcd, 0x80,                            //int    $0x80
   0xcc,                                  //Trap
   0x90                                   //nop
};
const unsigned int x86_call_munmap_size = sizeof(x86_call_munmap);

