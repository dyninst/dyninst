/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include <iostream>
#include <fstream>

#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include "registers/aarch64_regs.h"
#include "common/h/dyntypes.h"
#include "common/src/vm_maps.h"
#include "compiler_annotations.h"

#include "common/src/pathName.h"
#include "PCErrors.h"
#include "Generator.h"
#include "Event.h"
#include "Handler.h"
#include "Mailbox.h"
#include "PlatFeatures.h"

#include "procpool.h"
#include "irpc.h"
#include "int_thread_db.h"
#include "linux.h"
#include "int_handler.h"
#include "response.h"
#include "int_event.h"

#include "snippets.h"

#include "common/src/linuxKludges.h"
#include "common/src/parseauxv.h"

#include "boost/shared_ptr.hpp"

#include "unaligned_memory_access.h"

//needed by GETREGSET/SETREGSET
#if defined(arch_aarch64)
#include<sys/user.h>
#include<sys/procfs.h>
#include<sys/uio.h>
#if !defined(WITH_SYMLITE)
#include<linux/elf.h>
#endif
#endif

// Before glibc-2.7, sys/ptrace.h lacked PTRACE_O_* and PTRACE_EVENT_*, so we
// need them from linux/ptrace.h.  (Conditionally, as later glibc conflicts.)
#if !__GLIBC_PREREQ(2,7)
#include <linux/ptrace.h>
#endif

using namespace Dyninst;
using namespace ProcControlAPI;

#if defined(WITH_SYMLITE)
#include "symlite/h/SymLite-elf.h"
#elif defined(WITH_SYMTAB_API)
#include "symtabAPI/h/SymtabReader.h"
#else
#error "No defined symbol reader"
#endif

#if !defined(PTRACE_GETREGS) && defined(PPC_PTRACE_GETREGS)
#define PTRACE_GETREGS PPC_PTRACE_GETREGS
#endif

#if !defined(PTRACE_SETREGS) && defined(PPC_PTRACE_SETREGS)
#define PTRACE_SETREGS PPC_PTRACE_SETREGS
#endif


static pid_t P_gettid();
static bool t_kill(int pid, int sig);

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

bool GeneratorLinux::initialize()
{
    int result;

    sigset_t usr2_set;
    sigemptyset(&usr2_set);
    sigaddset(&usr2_set, SIGUSR2);
    result = pthread_sigmask(SIG_UNBLOCK, &usr2_set, NULL);
    if (result != 0) {
	perr_printf("Unable to unblock SIGUSR2: %s\n", strerror(result));
    }

    generator_lwp = P_gettid();
    generator_pid = P_getpid();
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

   if (isExitingState())
      return NULL;
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
   GeneratorMT(std::string("Linux Generator")),
   generator_lwp(0),
   generator_pid(0)
{
   decoders.insert(new DecoderLinux());
}

static volatile int on_sigusr2_hit;
static void on_sigusr2(int)
{
   on_sigusr2_hit = 1;
}

void GeneratorLinux::evictFromWaitpid()
{
   if (!generator_lwp)
      return;
   if (generator_pid != P_getpid())
      return;

   //Throw a SIGUSR2 at the generator thread.  This will kick it out of
   // a waitpid with EINTR, and allow it to exit.  Will do nothing if not
   // blocked in waitpid.
   //
   //There's a subtle race condition here, which we can't easily fix.
   // The generator thread could be just before waitpid, but after
   // it's exit test when the signal hits.  We won't throw EINTR because
   // we're not in waitpid yet, and we won't retest the exiting state.
   // This exact kind of race is why we have things like pselect, but
   // waitpid doesn't have a pwaitpid, so we're stuck.
   struct sigaction newact, oldact;
   memset(&newact, 0, sizeof(struct sigaction));
   memset(&oldact, 0, sizeof(struct sigaction));
   newact.sa_handler = on_sigusr2;

   int result = sigaction(SIGUSR2, &newact, &oldact);
   if (result == -1) {
      perr_printf("Error signaling generator thread: %s\n", strerror(errno));
      return;
   }
   on_sigusr2_hit = 0;
   bool bresult = t_kill(generator_lwp, SIGUSR2);
   while (bresult && !on_sigusr2_hit) {
      //Don't use a lock because pthread_mutex_unlock is not signal safe
      sched_yield();
   }

   result = sigaction(SIGUSR2, &oldact, NULL);
   if (result == -1) {
      perr_printf("Error signaling generator thread: %s\n", strerror(errno));
   }
}


GeneratorLinux::~GeneratorLinux()
{
   setState(exiting);
   evictFromWaitpid();
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
  if (arch == Dyninst::Arch_aarch64){
    return addr;
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
      lthread = dynamic_cast<linux_thread *>(thread);
   }
   if (proc) {
      lproc = dynamic_cast<linux_process *>(proc);
   }

   if (!proc) {
      pthrd_printf("Warning: could not find event for process %d\n", archevent->pid);
   }

   Event::ptr event = Event::ptr();
   ArchEventLinux *child = NULL;
   ArchEventLinux *parent = NULL;

   pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
                thread ? thread->getLWP() : -1);

   const int status = archevent->status;
   pthrd_printf("ARM-debug: status 0x%x\n",(unsigned int)status);
   if (WIFSTOPPED(status))
   {
      const int stopsig = WSTOPSIG(status);
      int ext;
      pthrd_printf("Decoded to signal %d\n", stopsig);
      switch (stopsig)
      {
         case (SIGTRAP | 0x80): //PTRACE_O_TRACESYSGOOD
            if (!proc || !thread) {
                //Legacy event on old process?
                return true;
            }
            pthrd_printf("Decoded event to syscall-stop on %d/%d\n",
                  proc->getPid(), thread->getLWP());
            if (lthread->hasPostponedSyscallEvent()) {
               delete archevent;
               archevent = lthread->getPostponedSyscallEvent();
               ext = archevent->event_ext;

               // in case of a fake syscall exit BP is inserted,
               // it should be cleared when the actual stop is received.
               Dyninst::MachRegisterVal addr;
               int r = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
               if (!r) {
                   perr_printf("Failed to read PC address upon SIGTRAP|0x80\n");
                   return false;
               }
               if( lthread->isSet_fakeSyscallExitBp &&
                   lthread->addr_fakeSyscallExitBp == addr){
                   // do not handle the bp and clear the bp.
                    bool rst = lthread->proc()->rmBreakpoint(addr, lthread->BPptr_fakeSyscallExitBp );
                    if( !rst){
                        perr_printf("ARM-error: Failed to remove inserted BP, addr %p.\n",
                                (void*)addr);
                    }
                    lthread->isSet_fakeSyscallExitBp = false;
               }

               switch (ext) {
                  case PTRACE_EVENT_FORK:
                  case PTRACE_EVENT_CLONE:
                     pthrd_printf("Resuming %s event after syscall exit on %d/%d\n",
                                  ext == PTRACE_EVENT_FORK ? "fork" : "clone",
                                  proc->getPid(), thread->getLWP());
                     if (!archevent->findPairedEvent(parent, child)) {
                        pthrd_printf("Parent half of paired event, postponing decode "
                                     "until child arrives\n");
                        archevent->postponePairedEvent();
                        return true;
                     }
                     break;
                  case PTRACE_EVENT_EXEC:
                     pthrd_printf("Resuming exec event after syscall exit on %d/%d\n",
                                  proc->getPid(), thread->getLWP());
                     event = Event::ptr(new EventExec(EventType::Post));
                     event->setSyncType(Event::sync_process);
                     break;
               }
               break;
            }
	        // If we're expecting syscall events other than postponed ones, fall through the rest of
	        // the event handling
	        if(!thread->syscallMode())
	        {
	          perr_printf("Received an unexpected syscall TRAP\n");
	          return false;
	        }

            DYNINST_FALLTHROUGH;

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
            if (lthread->getGeneratorState().getState() == int_thread::neonatal ||
                lthread->getGeneratorState().getState() == int_thread::neonatal_intermediate)
            {
               //Discovered thread from refresh
               pthrd_printf("Decoded event to thread bootstrap on %d/%d\n",
                            proc->getPid(), thread->getLWP());
               event = Event::ptr(new EventBootstrap());
               break;
            }

            DYNINST_FALLTHROUGH;

         case SIGTRAP: {
            {
#if 0
               //Debugging code
               Dyninst::MachRegisterVal addr;
               Dyninst::MachRegisterVal x30_val;

               result = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
               result = thread->plat_getRegister(Dyninst::aarch64::x30, x30_val);
               if (!result) {
                  fprintf(stderr, "Failed to read PC address upon crash\n");
               }
               fprintf(stderr, "Got SIGTRAP at %lx\n", addr);
	       fprintf(stderr, "X30 : %lx\n", x30_val);

               Dyninst::MachRegisterVal X0;
               result = thread->plat_getRegister(Dyninst::aarch64::x0 ,X0);
               pthrd_printf("ARM-debug: x0 is 0x%lx/%u\n", X0, X0);
#endif
            }
            ext = status >> 16;
            if (ext) {
               bool postpone = false;
               switch (ext) {
                  case PTRACE_EVENT_EXIT:
                    if (!proc || !thread) {
                        //Legacy event on old process.
                        return true;
                    }
                    pthrd_printf("Decoded event to pre-exit on %d/%d\n",
                                  proc->getPid(), thread->getLWP());
                    if (thread->getLWP() == proc->getPid())
		            {
		                unsigned long eventmsg = 0x0;
		                int r = do_ptrace((pt_req)PTRACE_GETEVENTMSG, (pid_t) thread->getLWP(),
					        NULL, &eventmsg);
		                if(r == -1)
		                {
                            int error = errno;
                            perr_printf("Error getting event message from exit\n");
                            if (error == ESRCH)
                                proc->setLastError(err_exited, "Process exited during operation");
                            return false;
		                }
		                int exitcode = (int)eventmsg;
                        if (WIFSIGNALED(exitcode)) {
                            int termsig = WTERMSIG(exitcode);
                            pthrd_printf("Decoded event to pre-exit due to crash/signal %d\n", termsig);
                            event = Event::ptr(new EventCrash(termsig));
                            break;
                        }

		                exitcode = WEXITSTATUS(exitcode);

		                pthrd_printf("Decoded event to pre-exit of process %d/%d with code %i\n",
				            proc->getPid(), thread->getLWP(), exitcode);
		                event = Event::ptr(new EventExit(EventType::Pre, exitcode));

                        for (int_threadPool::iterator j = proc->threadPool()->begin();
                             j != proc->threadPool()->end(); ++j)
                        {
                           dynamic_cast<linux_thread*>(*j)->setGeneratorExiting();
                        }
		            }
                    else {
                        EventLWPDestroy::ptr lwp_ev = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Pre));
                        event = lwp_ev;
                        event->setThread(thread->thread());
                        lproc->decodeTdbLWPExit(lwp_ev);
                        lthread->setGeneratorExiting();
                    }
                    thread->setExitingInGenerator(true);
                    break;
                  case PTRACE_EVENT_FORK:
                  case PTRACE_EVENT_CLONE: {
                     if (!proc || !thread) {
                        //Legacy event on old process.
                        return true;
                     }
                     pthrd_printf("Decoded event to %s on %d/%d\n",
                                  ext == PTRACE_EVENT_FORK ? "fork" : "clone",
                                  proc->getPid(), thread->getLWP());
                     unsigned long cpid_l = 0x0;
                     int r = do_ptrace((pt_req) PTRACE_GETEVENTMSG, (pid_t) thread->getLWP(),
                                            NULL, &cpid_l);
                     if (r == -1) {
                        int error = errno;
                        perr_printf("Error getting event message from fork/clone\n");
                        if (error == ESRCH)
                           proc->setLastError(err_exited, "Process exited during operation");
                        return false;
                     }
                     pid_t cpid = (pid_t) cpid_l;
                     archevent->child_pid = cpid;

                     postpone = true;

                     break;
                  }
                  case PTRACE_EVENT_EXEC: {
                     if (!proc || !thread) {
                        //Legacy event on old process.
                        return true;
                     }
                     pthrd_printf("Decoded event to exec on %d/%d\n",
                                  proc->getPid(), thread->getLWP());
                     postpone = true;
                     break;
                  }
               }

               if (postpone) {
                  archevent->event_ext = ext;

                  pthrd_printf("Postponing event until syscall exit on %d/%d\n",
                               proc->getPid(), thread->getLWP());
                  event = Event::ptr(new EventPostponedSyscall());
                  lthread->postponeSyscallEvent(archevent);
                  archevent = NULL;

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
#if 0
            //ARM-Debug
            pthrd_printf("ARM-debug: PC = 0x%lx\n", addr);
            char buffer_inst[4];
            proc->plat_readMem(thread, buffer_inst, addr, 4);
            printf("0x%8x\n", *((unsigned int*)buffer_inst) );
#endif

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

            bp_instance *clearingbp = thread->isClearingBreakpoint();
            if (thread->singleStep() && clearingbp) {
                pthrd_printf("Decoded event to breakpoint restore\n");
                event = Event::ptr(new EventBreakpointRestore(new int_eventBreakpointRestore(clearingbp)));
                if (thread->singleStepUserMode()) {
                   Event::ptr subservient_ss = EventSingleStep::ptr(new EventSingleStep());
                   subservient_ss->setProcess(proc->proc());
                   subservient_ss->setThread(thread->thread());
                   subservient_ss->setSyncType(Event::sync_thread);
                   event->addSubservientEvent(subservient_ss);
                }
                break;
            }

            // Need to distinguish case where the thread is single-stepped to a
            // breakpoint and when a single step hits a breakpoint.
            //
            // If no forward progress was made due to a single step, then a
            // breakpoint was hit
            sw_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);
            if (thread->singleStep() && !ibp) {
               pthrd_printf("Decoded event to single step on %d/%d\n",
                       proc->getPid(), thread->getLWP());
               event = Event::ptr(new EventSingleStep());
               break;
            }

            if (thread->syscallMode() && !ibp) {
                if (thread->preSyscall()) {
                    pthrd_printf("Decoded event to pre-syscall on %d/%d\n",
                            proc->getPid(), thread->getLWP());
                    event = Event::ptr(new EventPreSyscall());
                    break;
                } else {
                    pthrd_printf("Decoded event to post-syscall on %d/%d\n",
                            proc->getPid(), thread->getLWP());
                    event = Event::ptr(new EventPostSyscall());
                    break;
                }
            }

            if (ibp && ibp != clearingbp) {
               pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(),
                            thread->getLWP(), adjusted_addr);
               EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(new int_eventBreakpoint(adjusted_addr, ibp, thread)));
               event = event_bp;
               event->setThread(thread->thread());

               if (thread->singleStepUserMode() && !proc->plat_breakpointAdvancesPC()) {
                  Event::ptr subservient_ss = EventSingleStep::ptr(new EventSingleStep());
                  subservient_ss->setProcess(proc->proc());
                  subservient_ss->setThread(thread->thread());
                  subservient_ss->setSyncType(Event::sync_thread);
                  event->addSubservientEvent(subservient_ss);
               }

               if (adjusted_addr == lproc->getLibBreakpointAddr()) {
                  pthrd_printf("Breakpoint is library load/unload\n");
                  EventLibrary::ptr lib_event = EventLibrary::ptr(new EventLibrary());
                  lib_event->setThread(thread->thread());
                  lib_event->setProcess(proc->proc());
                  lib_event->setSyncType(Event::sync_thread);
                  event->addSubservientEvent(lib_event);
                  break;
               }
               for (;;) {
                  async_ret_t r = lproc->decodeTdbBreakpoint(event_bp);
                  if (r == aret_error) {
                     //Not really an error, just how we say that it isn't
                     // a breakpoint.
                     break;
                  }
                  if (r == aret_success) {
                     //decodeTdbBreakpoint added a subservient event if this hits
                     pthrd_printf("Breakpoint was thread event\n");
                     break;
                  }
                  if (r == aret_async) {
                     pthrd_printf("decodeTdbBreakpoint returned async\n");
                     set<response::ptr> resps;
                     lproc->getMemCache()->getPendingAsyncs(resps);
                     pthrd_printf("%d asyncs are pending\n", (int) resps.size());
                     int_process::waitForAsyncEvent(resps);
                     continue;
                  }
               }

               break;
            }
            response::ptr resp;
            EventBreakpoint::ptr evhwbp = thread->decodeHWBreakpoint(resp);
            if (evhwbp) {
               pthrd_printf("Decoded SIGTRAP as hardware breakpoint\n");
               event = evhwbp;
               break;
            }

         }

         DYNINST_FALLTHROUGH;

         default:
            pthrd_printf("Decoded event to signal %d on %d/%d\n",
                         stopsig, proc->getPid(), thread->getLWP());
#if 0
            //Debugging code
            if (stopsig == 11 || stopsig == 4) {
               Dyninst::MachRegisterVal addr;
               result = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
               if (!result) {
                  fprintf(stderr, "Failed to read PC address upon crash\n");
               }
               fprintf(stderr, "Got crash at %lx\n", addr);
	       fprintf(stderr, "ARM-debug: PC = 0x%lx\n", addr);
	       char buffer_inst[4];
	       proc->plat_readMem(thread, buffer_inst, addr, 4);
	       fprintf(stderr,"0x%8x\n", *((unsigned int*)buffer_inst) );

               //while (1) sleep(1);
            }
#endif
            Dyninst::MachRegisterVal addr;
            result = thread->plat_getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
            event = Event::ptr(new EventSignal(stopsig, addr, EventSignal::Unknown, false));
      }
      if (event && event->getSyncType() == Event::unset)
         event->setSyncType(Event::sync_thread);
   }
   else if ((WIFEXITED(status) || WIFSIGNALED(status)) &&
            (!proc || !thread || thread->getGeneratorState().getState() == int_thread::exited))
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
      thread->getGeneratorState().setState(int_thread::exited);
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
         (*i)->getGeneratorState().setState(int_thread::exited);
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
         event = Event::ptr(new EventFork(EventType::Post, child->pid));
      else if (parent->event_ext == PTRACE_EVENT_CLONE)
         event = Event::ptr(new EventNewLWP(child->pid, (int) int_thread::as_created_attached));
      else
         assert(0);
      event->setSyncType(Event::sync_thread);
      ProcPool()->removeDeadThread(child->pid);
      delete parent;
      delete child;
   }
   else {
       if (archevent && ProcPool()->deadThread(archevent->pid)) {
	      delete archevent;
	      return true;
       }

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

#if defined(arch_power)
#define DEFAULT_PROCESS_TYPE linux_ppc_process
#define DEFAULT_THREAD_TYPE linux_ppc_thread
#elif defined(arch_x86) || defined(arch_x86_64)
#define DEFAULT_PROCESS_TYPE linux_x86_process
#define DEFAULT_THREAD_TYPE linux_x86_thread
#elif defined(arch_aarch64) || defined(arch_aarch32)
#define DEFAULT_PROCESS_TYPE linux_arm_process
#define DEFAULT_THREAD_TYPE linux_arm_thread
#endif

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
   std::vector<std::string> a;
   std::map<int,int> f;
   std::vector<std::string> envp;
   LinuxPtrace::getPtracer(); //Make sure ptracer thread is initialized
   linux_process *newproc = new DEFAULT_PROCESS_TYPE(p, e, a, envp, f);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, std::vector<std::string> envp,
        std::map<int,int> f)
{
   LinuxPtrace::getPtracer(); //Make sure ptracer thread is initialized
   linux_process *newproc = new DEFAULT_PROCESS_TYPE(0, e, a, envp, f);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *p)
{
   linux_process *newproc = new DEFAULT_PROCESS_TYPE(pid_, p);
   assert(newproc);
   return static_cast<int_process *>(newproc);
}

int linux_process::computeAddrWidth()
{

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
    * We'll thus check every word that is 1 mod 4 for little-endian machines,
    * or 0 mod 4 for big-endian.  If all words of either stripe are 0x0, we
    * assume we're looking at a 64-bit process.
    **/
   uint32_t buffer[256];
   char auxv_name[64];

   snprintf(auxv_name, 64, "/proc/%d/auxv", getPid());
   int fd = open(auxv_name, O_RDONLY);
   if (fd == -1) {
      pthrd_printf("Couldn't open %s to determine address width: %s",
                   auxv_name, strerror(errno));
      return -1;
   }

   ssize_t nread = read(fd, buffer, sizeof(buffer));
   ssize_t words_read = (nread / sizeof(uint32_t)) & ~3;
   close(fd);

   // We want to check the highest 4 bytes of each integer
   // On big-endian systems, these come first in memory
   bool be_zero = true, le_zero = true;
   for (ssize_t i=0; i<words_read; i+= 4)
   {
     be_zero &= buffer[i] == 0;
     le_zero &= buffer[i+1] == 0;
   }

   int word_size = (be_zero || le_zero) ? 8 : 4;
   pthrd_printf("computeAddrWidth: word size is %d\n", word_size);
   return word_size;
}

linux_process::linux_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                             std::vector<std::string> envp,  std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   sysv_process(p, e, a, envp, f),
   unix_process(p, e, a, envp, f),
   thread_db_process(p, e, a, envp, f),
   indep_lwp_control_process(p, e, a, envp, f),
   mmap_alloc_process(p, e, a, envp, f),
   int_followFork(p, e, a, envp, f),
   int_signalMask(p, e, a, envp, f),
   int_LWPTracking(p, e, a, envp, f),
   int_memUsage(p, e, a, envp, f)
{
}

linux_process::linux_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   sysv_process(pid_, p),
   unix_process(pid_, p),
   thread_db_process(pid_, p),
   indep_lwp_control_process(pid_, p),
   mmap_alloc_process(pid_, p),
   int_followFork(pid_, p),
   int_signalMask(pid_, p),
   int_LWPTracking(pid_, p),
   int_memUsage(pid_, p)
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
      errno = 0;
      long int r = ptrace((pt_req) PTRACE_TRACEME, 0, 0, 0);
      if (r == -1)
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

bool linux_process::plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) {
    vector<Dyninst::LWP> lwps;
    if( !getThreadLWPs(lwps) ) {
        pthrd_printf("Failed to determine lwps for process %d\n", getPid());
        setLastError(err_noproc, "Failed to find /proc files for debuggee");
        return false;
    }

    for(vector<Dyninst::LWP>::iterator i = lwps.begin();
            i != lwps.end(); ++i)
    {
        const auto ignore_max = std::numeric_limits<std::streamsize>::max();
        char proc_stat_name[128];

        snprintf(proc_stat_name, 128, "/proc/%d/stat", *i);
        ifstream sfile(proc_stat_name);

        while (sfile.good()) {

            // The stat looks something like: 123 (command) R 456...
            // We'll just look for the ") R " part.
            if (sfile.ignore(ignore_max, ')').peek() == ' ') {
                char space, state_char;

                // Eat the space we peeked and grab the state char.
                if (sfile.get(space).get(state_char).peek() == ' ') {
                    // Found the state char -- 'T' means it's already stopped.
                    runningStates.insert(make_pair(*i, (state_char != 'T')));
                    break;
                }

                // Restore the state char and try again
                sfile.unget();
            }
        }

        if (!sfile.good() && (*i == getPid())) {
            // Only the main thread is treated as an error.  Other threads may
            // have exited between getThreadLWPs and /proc/pid/stat open or read.
            pthrd_printf("Failed to read /proc/%d/stat file\n", *i);
            setLastError(err_noproc, "Failed to find /proc files for debuggee");
            return false;
        }
    }

    return true;
}

// Ubuntu 10.10 and other hardened systems do not allow arbitrary ptrace_attaching; instead
// you may only attach to a child process (https://wiki.ubuntu.com/SecurityTeam/Roadmap/KernelHardening)
//
// We can detect this and warn the user; however, it takes root to disable it.

#include <fstream>

static void warn_user_ptrace_restrictions() {
  ifstream ptrace_scope("/proc/sys/kernel/yama/ptrace_scope");
  if (ptrace_scope.is_open()) {
    int val = 99;
    ptrace_scope >> val;
    if (val == 1) {
      cerr << "Warning: your Linux system provides limited ptrace functionality as a security" << endl
	   << "measure. This measure prevents ProcControl and Dyninst from attaching to binaries." << endl
	   << "To temporarily disable this measure (until a reboot), execute the following command:" << endl
	   << "\techo 0 > /proc/sys/kernel/yama/ptrace_scope" << endl;
      struct stat statbuf;
      if (!stat("/etc/sysctl.d/10-ptrace.conf", &statbuf)) {
	cerr << "To permanently disable this measure, edit the file \"/etc/sysctl.d/10-ptrace.conf\"" << endl
	     << "and follow the directions in that file." << endl;
      }
      cerr << "For more information, see https://wiki.ubuntu.com/SecurityTeam/Roadmap/KernelHardening" << endl;
      is_restricted_ptrace = true;
    }
  }
}


bool linux_process::plat_attach(bool, bool &)
{
   pthrd_printf("Attaching to pid %d\n", pid);

   bool attachWillTriggerStop = plat_attachWillTriggerStop();

   int result = do_ptrace((pt_req) PTRACE_ATTACH, pid, NULL, NULL);
   if (result != 0) {
      int errnum = errno;
      pthrd_printf("Unable to attach to process %d: %s\n", pid, strerror(errnum));
      switch(errnum) {
      case EPERM:
	warn_user_ptrace_restrictions();
	setLastError(err_prem, "Do not have correct premissions to attach to pid");
	break;
      case ESRCH:
         setLastError(err_noproc, "The specified process was not found");
	 break;
      default:
         setLastError(err_internal, "Unable to attach to the specified process");
	 break;
      }
      return false;
   }

   if ( !attachWillTriggerStop ) {
       // Force the SIGSTOP delivered by the attach to be handled
       pthrd_printf("Attach will not trigger stop, calling PTRACE_CONT to flush out stop\n");
       result = do_ptrace((pt_req) PTRACE_CONT, pid, NULL, NULL);
       if( result != 0 ) {
           int errnum = errno;
           pthrd_printf("Unable to continue process %d to flush out attach: %s\n",
                   pid, strerror(errnum));
           if (errnum == ESRCH)
              setLastError(err_exited, "Process exited during operation");
           return false;
       }
   }

   return true;
}

// Attach any new threads and synchronize, until there are no new threads
bool linux_process::plat_attachThreadsSync()
{
   while (true) {
      bool found_new_threads = false;

      ProcPool()->condvar()->lock();
      bool result = attachThreads(found_new_threads);
      if (found_new_threads)
         ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();

      if (!result) {
         pthrd_printf("Failed to attach to threads in %d\n", pid);
         setLastError(err_internal, "Could not get threads during attach\n");
         return false;
      }

      if (!found_new_threads)
         return true;

      while (Counter::processCount(Counter::NeonatalThreads, this) > 0) {
         bool proc_exited = false;
         pthrd_printf("Waiting for neonatal threads in process %d\n", pid);
         result = waitAndHandleForProc(true, this, proc_exited);
         if (!result) {
            perr_printf("Internal error calling waitAndHandleForProc on %d\n", getPid());
            return false;
         }
         if (proc_exited) {
            perr_printf("Process exited while waiting for user thread stop, erroring\n");
            setLastError(err_exited, "Process exited while thread being stopped.\n");
            return false;
         }
      }
   }
}

bool linux_process::plat_attachWillTriggerStop() {
    char procName[64];
    char cmd[256];
    pid_t tmpPid;
    char state_char;
    int ttyNumber;

    // Retrieve the state of the process and its controlling tty
    snprintf(procName, 64, "/proc/%d/stat", pid);

    boost::shared_ptr<FILE> sfile(fopen(procName, "r"), fclose);
    if (!sfile) {
        perr_printf("Failed to determine whether attach would trigger stop -- assuming it will\n");
        return true;
    }

    if(fscanf(sfile.get(), "%d %255s %c %d %d %d",
            &tmpPid, cmd, &state_char,
            &tmpPid, &tmpPid, &ttyNumber) < 0) {
        perr_printf("Failed to determine whether attach would trigger stop -- assuming it will\n");
        return true;
    }

    // If the process is stopped and it has a controlling tty, an attach
    // will not trigger a stop
    if ( state_char == 'T' && ttyNumber != 0 ) {
        return false;
    }

    return true;
}

bool linux_process::plat_execed()
{
   bool result = sysv_process::plat_execed();
   if (!result)
      return false;

   char proc_exec_name[128];
   snprintf(proc_exec_name, 128, "/proc/%d/exe", getPid());
   executable = resolve_file_path(proc_exec_name);
   return true;
}

bool linux_process::plat_forked()
{
   return true;
}

bool linux_process::plat_readMem(int_thread *thr, void *local,
                                 Dyninst::Address remote, size_t size)
{
   char file[128];
   snprintf(file, 64, "/proc/%d/mem", getPid());
   int fd = open(file, O_RDWR);
   ssize_t ret = pread(fd, local, size, remote);
   close(fd);
   if (static_cast<size_t>(ret) != size) {
      // Reads through procfs failed.
      // Fall back to use ptrace
      return LinuxPtrace::getPtracer()->ptrace_read(remote, size, local, thr->getLWP());
   }
   return true;
}

bool linux_process::plat_writeMem(int_thread *thr, const void *local,
                                  Dyninst::Address remote, size_t size, bp_write_t)
{

   char file[128];
   snprintf(file, 64, "/proc/%d/mem", getPid());
   int fd = open(file, O_RDWR);
   ssize_t ret = pwrite(fd, local, size, remote);
   close(fd);
   if (static_cast<size_t>(ret) != size) {
      // Writes through procfs failed.
      // Fall back to use ptrace
      return LinuxPtrace::getPtracer()->ptrace_write(remote, size, local, thr->getLWP());
   }
   return true;
}

linux_x86_process::linux_x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                     std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   linux_process(p, e, a, envp, f),
   x86_process(p, e, a, envp, f)
{
}

linux_x86_process::linux_x86_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   linux_process(pid_, p),
   x86_process(pid_, p)
{
}


linux_x86_process::~linux_x86_process()
{
}

Dyninst::Architecture linux_x86_process::getTargetArch()
{
   if (arch != Dyninst::Arch_none) {
      return arch;
   }
   int addr_width = computeAddrWidth();
   arch = (addr_width == 4) ? Dyninst::Arch_x86 : Dyninst::Arch_x86_64;
   return arch;
}

bool linux_x86_process::plat_supportHWBreakpoint()
{
   return true;
}

linux_ppc_process::linux_ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                     std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   linux_process(p, e, a, envp, f),
   ppc_process(p, e, a, envp, f)
{
}

linux_ppc_process::linux_ppc_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   linux_process(pid_, p),
   ppc_process(pid_, p)
{
}


linux_ppc_process::~linux_ppc_process()
{
}

Dyninst::Architecture linux_ppc_process::getTargetArch()
{
   if (arch != Dyninst::Arch_none) {
      return arch;
   }
   int addr_width = computeAddrWidth();
   arch = (addr_width == 4) ? Dyninst::Arch_ppc32 : Dyninst::Arch_ppc64;
   return arch;
}

//steve: added
linux_arm_process::linux_arm_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                     std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   linux_process(p, e, a, envp, f),
   arm_process(p, e, a, envp, f)
{
}

linux_arm_process::linux_arm_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   linux_process(pid_, p),
   arm_process(pid_, p)
{
}


linux_arm_process::~linux_arm_process()
{
}

Dyninst::Architecture linux_arm_process::getTargetArch()
{
   if (arch != Dyninst::Arch_none) {
      return arch;
   }
   int addr_width = computeAddrWidth();
   arch = (addr_width == 4) ? Dyninst::Arch_aarch32 : Dyninst::Arch_aarch64;
   assert(arch == Dyninst::Arch_aarch64); //should be aarch64 at this stage
   return arch;
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

      pthrd_printf("Faking response for event %u\n", id);
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
      if(!result) return false;
   }
   return true;
}

bool linux_process::plat_readMemAsync(int_thread *thr, Dyninst::Address addr, mem_response::ptr result)
{
   bool b = plat_readMem(thr, result->getBuffer(), addr, result->getSize());
   if (!b) {
      result->markError(getLastError());
   }
   result->setLastBase(addr);
   fake_async_msgs.push_back(result->getID());
   return true;
}

bool linux_process::plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr, size_t size,
                                       result_response::ptr result, bp_write_t bp_write)
{
   bool b = plat_writeMem(thr, local, addr, size, bp_write);
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

bool linux_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
   return findProcLWPs(pid, lwps);
}

bool linux_process::plat_supportLWPCreate()
{
   return true;
}

bool linux_process::plat_supportLWPPreDestroy()
{
   return true;
}

bool linux_process::plat_supportLWPPostDestroy()
{
   return true;
}

void linux_thread::postponeSyscallEvent(ArchEventLinux *event)
{
   assert(!postponed_syscall_event);
   postponed_syscall_event = event;
}

bool linux_thread::hasPostponedSyscallEvent()
{
   return postponed_syscall_event != NULL;
}

ArchEventLinux *linux_thread::getPostponedSyscallEvent()
{
   ArchEventLinux *ret = postponed_syscall_event;
   postponed_syscall_event = NULL;
   return ret;
}


bool linux_thread::plat_cont()
{
   switch (getHandlerState().getState()) {
      case neonatal:
      case running:
      case exited:
      case errorstate:
      case detached:
         perr_printf("Continue attempted on thread in invalid state %s\n",
                     int_thread::stateStr(handler_state.getState()));
         return false;
      case neonatal_intermediate:
      case stopped:
         //OK
         break;
      case none:
      case dontcare:
      case ditto:
         assert(0);
   }

   pthrd_printf("Continuing thread %d/%d from current handler state %s\n",
		   proc_->getPid(), lwp, int_thread::stateStr(handler_state.getState()));

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

   int tmpSignal = continueSig_;
   if( hasPendingStop()) {
       pthrd_printf("There are pending stops, tmpSignal is %d\n", tmpSignal);
       tmpSignal = 0;
   }

   void *data = (tmpSignal == 0) ? NULL : (void *) (long) tmpSignal;
   int result;
   if (hasPostponedSyscallEvent())
   {
      pthrd_printf("Calling PTRACE_SYSCALL on %d with signal %d\n", lwp, tmpSignal);
      result = do_ptrace((pt_req) PTRACE_SYSCALL, lwp, NULL, data);
   }
   else if (singleStep())
   {
      pthrd_printf("Calling PTRACE_SINGLESTEP on %d with signal %d\n", lwp, tmpSignal);
      result = do_ptrace((pt_req) PTRACE_SINGLESTEP, lwp, NULL, data);
   }
   else if (syscallMode())
   {
        pthrd_printf("Calling PTRACE_SYSCALL on %d with signal %d\n", lwp, tmpSignal);
        result = do_ptrace((pt_req) PTRACE_SYSCALL, lwp, NULL, data);
   }
   else
   {
      pthrd_printf("Calling PTRACE_CONT on %d with signal %d\n", lwp, tmpSignal);
      result = do_ptrace((pt_req) PTRACE_CONT, lwp, NULL, data);
   }
   if (result == -1) {
      int error = errno;
      if (error == ESRCH) {
         pthrd_printf("Continue attempted on exited thread %d\n", lwp);
         setLastError(err_exited, "Continue on exited thread");
         return false;
      }
      perr_printf("low-level continue failed: %s\n", strerror(error));
      setLastError(err_internal, "Low-level continue failed\n");
      return false;
   }
   if( tmpSignal == continueSig_ ) continueSig_ = 0;

   return true;
}

SymbolReaderFactory *getElfReader()
{
#if defined(WITH_SYMLITE)
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
  return symreader_factory;
#elif defined(WITH_SYMTAB_API)
  return SymtabAPI::getSymtabReaderFactory();
#else
#error "No defined symbol reader"
#endif
}

SymbolReaderFactory *linux_process::plat_defaultSymReader()
{
   return getElfReader();
}


#ifndef SYS_tkill
#define SYS_tkill 238
#endif

static pid_t P_gettid()
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
   linux_thread *lthrd = new DEFAULT_THREAD_TYPE(proc, thr_id, lwp_id);
   assert(lthrd);
   return static_cast<int_thread *>(lthrd);
}

linux_thread::linux_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   thread_db_thread(p, t, l),
   postponed_syscall_event(NULL),
   generator_started_exit_processing(false)
{
}

linux_thread::~linux_thread()
{
   delete postponed_syscall_event;
}

bool linux_thread::plat_stop()
{
   bool result;

   assert(pending_stop.local());
   result = t_kill(lwp, SIGSTOP);
   if (!result) {
      int err = errno;
      if (err == ESRCH) {
         pthrd_printf("t_kill failed on %d, thread doesn't exist\n", lwp);
         setLastError(err_exited, "Operation on exited thread");
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
   options |= PTRACE_O_TRACEEXIT;
   options |= PTRACE_O_TRACEEXEC;
   options |= PTRACE_O_TRACESYSGOOD;
   if (llproc()->getLWPTracking()->lwp_getTracking())
      options |= PTRACE_O_TRACECLONE;
   if (llproc()->getFollowFork()->fork_isTracking() != FollowFork::ImmediateDetach)
      options |= PTRACE_O_TRACEFORK;

   if (options) {
      int result = do_ptrace((pt_req) PTRACE_SETOPTIONS, lwp, NULL,
                          (void *) options);
      if (result == -1) {
         int error = errno;
         pthrd_printf("Failed to set options for %ld: %s\n", tid, strerror(errno));
         if (error == ESRCH)
            setLastError(err_exited, "Process exited during operation");
      }
   }
}

bool linux_thread::unsetOptions()
{
    long options = 0;

    int result = do_ptrace((pt_req) PTRACE_SETOPTIONS, lwp, NULL,
            (void *) options);
    if (result == -1) {
        int error = errno;
        pthrd_printf("Failed to set options for %ld: %s\n", tid, strerror(errno));
        if (error == ESRCH)
           setLastError(err_exited, "Process exited during operation");
        return false;
    }
    return true;
}

bool linux_process::plat_individualRegAccess()
{
   return true;
}

bool linux_process::plat_detach(result_response::ptr, bool leave_stopped)
{
   int_threadPool *tp = threadPool();
   bool had_error = false;
   bool first_thread_signaled = false;

   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      if (leave_stopped && !first_thread_signaled) {
         pthrd_printf("Signaling %d/%d with SIGSTOP during detach to leave stopped\n", getPid(), thr->getLWP());
         t_kill(thr->getLWP(), SIGSTOP);
         first_thread_signaled = true;
      }
      pthrd_printf("PTRACE_DETACH on %d\n", thr->getLWP());
      long result = do_ptrace((pt_req) PTRACE_DETACH, thr->getLWP(), NULL, (void *) 0);
      if (result == -1) {
         int error = errno;
         had_error = true;
         perr_printf("Failed to PTRACE_DETACH on %d/%d (%s)\n", getPid(), thr->getLWP(), strerror(errno));
         if (error == ESRCH)
            setLastError(err_exited, "Process exited during operation");
         else
            setLastError(err_internal, "PTRACE_DETACH operation failed\n");
      }
   }
   // Before we return from detach, make sure that we've gotten out of waitpid()
   // so that we don't steal events on that process.
   GeneratorLinux* g = dynamic_cast<GeneratorLinux*>(Generator::getDefaultGenerator());
   assert(g);
   g->evictFromWaitpid();

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

bool linux_process::preTerminate() {

   pthrd_printf("Stopping process %d for pre-terminate handling\n", getPid());
   threadPool()->initialThread()->getInternalState().desyncStateProc(int_thread::stopped);
   bool threw_event = false;
   while (!threadPool()->allStopped(int_thread::InternalStateID)) {
      if (!threw_event) {
         throwNopEvent();
         threw_event = true;
      }
      bool is_exited = false;
      auto pid_ = getPid();
      int_process::waitAndHandleForProc(true, this, is_exited);
      if (is_exited) {
         // Note, can't even call getPid() anymore, since 'this' is ironically deleted.
         perr_printf("Process %d exited during terminate handling.  Is this irony?\n", pid_);
         return false;
      }
   }
   pthrd_printf("Putting process %d back into previous state\n", getPid());
   threadPool()->initialThread()->getInternalState().restoreStateProc();

#if defined(bug_force_terminate_failure)
    // On some Linux versions (currently only identified on our power platform),
    // a force terminate can fail to actually kill a process due to some OS level
    // race condition. The result is that some threads in a process are stopped
    // instead of exited and for some reason, continues will not continue the
    // process. This can be detected because some OS level structures (such as pipes)
    // still exist for the terminated process

    // It appears that this bug largely results from the pre-LWP destroy and pre-Exit
    // events being delivered to the debugger, so we stop the process and disable these
    // events for all threads in the process

   int_threadPool::iterator i;
   for(i = threadPool()->begin(); i != threadPool()->end(); i++)
   {
      linux_thread *thr = dynamic_cast<linux_thread *>(*i);
      pthrd_printf("Disabling syscall tracing events for thread %d/%d\n",
                   getPid(), thr->getLWP());
      if( !thr->unsetOptions() ) {
         perr_printf("Failed to unset options for thread %d/%d in pre-terminate handling\n",
                     getPid(), thr->getLWP());
         return false;
      }
   }
#endif

   // We don't want to be mixing termination and breakpoint stepping.
   removeAllBreakpoints();

   // And put things back where we found them.
   throwNopEvent();

   pthrd_printf("Waiting for process %d to resynchronize before terminating\n", getPid());
   int_process::waitAndHandleEvents(false);

   return true;
}

OSType linux_process::getOS() const
{
   return Dyninst::Linux;
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

bool linux_process::fork_setTracking(FollowFork::follow_t f)
{
   int_threadPool::iterator i;
   for (i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thrd = *i;
      if (thrd->getUserState().getState() != int_thread::stopped) {
         perr_printf("Could not set fork tracking because thread %d/%d was not stopped\n",
                     getPid(), thrd->getLWP());
         setLastError(err_notstopped, "All threads must be stopped to change fork tracking\n");
         return false;
      }
   }
   if (f == FollowFork::None) {
      perr_printf("Could not set fork tracking on %d to None\n", getPid());
      setLastError(err_badparam, "Cannot set fork tracking to None");
      return false;
   }

   if (f == fork_tracking) {
      pthrd_printf("Leaving fork tracking for %d in state %d\n",
                   getPid(), (int) f);
      return true;
   }

   for (i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thrd = *i;
      linux_thread *lthrd = dynamic_cast<linux_thread *>(thrd);
      pthrd_printf("Changing fork tracking for thread %d/%d to %d\n",
                   getPid(), lthrd->getLWP(), (int) f);
      lthrd->setOptions();
   }
   return true;
}

FollowFork::follow_t linux_process::fork_isTracking() {
   return fork_tracking;
}

bool linux_process::plat_lwpChangeTracking(bool) {
   int_threadPool *pool = threadPool();
   if (!pool->allStopped(int_thread::UserStateID)) {
      perr_printf("Attempted to change lwpTracking, but not all threads stopped in %d", getPid());
      setLastError(err_notstopped, "Process not stopped before changing LWP tracking state");
      return false;
   }

   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      int_thread *thrd = *i;
      linux_thread *lthrd = dynamic_cast<linux_thread *>(thrd);
      assert(lthrd);
      lthrd->setOptions();
   }
   return true;
}

bool linux_process::allowSignal(int signal_no) {
   dyn_sigset_t mask = getSigMask();
   return sigismember(&mask, signal_no);
}

bool linux_process::readStatM(unsigned long &stk, unsigned long &heap, unsigned long &shrd)
{
   char path[64];
   snprintf(path, 64, "/proc/%d/statm", getPid());
   path[63] = '\0';

   unsigned long size, resident, shared, text, lib, data, dt;
   boost::shared_ptr<FILE> f(fopen(path, "r"), fclose);
   if (!f) {
      perr_printf("Could not open %s: %s\n", path, strerror(errno));
      setLastError(err_internal, "Could not access /proc");
      return false;
   }
   if(fscanf(f.get(), "%lu %lu %lu %lu %lu %lu %lu", &size, &resident, &shared,
          &text, &lib, &data, &dt) < 0) {
      perr_printf("Could not read from %s: %s\n", path, strerror(errno));
      setLastError(err_internal, "Could not read from /proc");
      return false;
   }
   unsigned long page_size = getpagesize();

   stk = 0;
   shrd = (shared + text) * page_size;
   heap = data * page_size;
   return true;
}

bool linux_process::plat_getStackUsage(MemUsageResp_t *resp)
{
   unsigned long stk, heap, shrd;
   bool result = readStatM(stk, heap, shrd);
   if (!result)
      return false;
   *resp->get() = stk;
   resp->done();
   return true;
}

bool linux_process::plat_getHeapUsage(MemUsageResp_t *resp)
{
   unsigned long stk, heap, shrd;
   bool result = readStatM(stk, heap, shrd);
   if (!result)
      return false;
   *resp->get() = heap;
   resp->done();
   return true;
}

bool linux_process::plat_getSharedUsage(MemUsageResp_t *resp)
{
   unsigned long stk, heap, shrd;
   bool result = readStatM(stk, heap, shrd);
   if (!result)
      return false;
   *resp->get() = shrd;
   resp->done();
   return true;
}

bool linux_process::plat_residentNeedsMemVals()
{
   return false;
}

bool linux_process::plat_getResidentUsage(unsigned long, unsigned long, unsigned long,
                           MemUsageResp_t *)
{
   return false;
}

#if !defined(OFFSETOF)
#define OFFSETOF(STR, FLD) (unsigned long) (&(((STR *) 0x0)->FLD))
#endif

dynreg_to_user_t dynreg_to_user;
static void init_dynreg_to_user()
{
   static volatile bool initialized = false;
   static Mutex<> init_lock;
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
#if defined(arch_x86) || defined(arch_x86_64)
      cur = OFFSETOF(user, u_debugreg);
#endif
      dynreg_to_user[x86::dr0]   = make_pair(cur, 4);
      dynreg_to_user[x86::dr1]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr2]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr3]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr4]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr5]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr6]   = make_pair(cur+=8, 4);
      dynreg_to_user[x86::dr7]   = make_pair(cur+=8, 4);
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
#if defined(arch_x86) || defined(arch_x86_64)
      cur = OFFSETOF(user, u_debugreg);
#endif
      dynreg_to_user[x86::dr0]   = make_pair(cur, 4);
      dynreg_to_user[x86::dr1]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr2]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr3]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr4]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr5]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr6]   = make_pair(cur+=4, 4);
      dynreg_to_user[x86::dr7]   = make_pair(cur+=4, 4);
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
#if defined(arch_x86) || defined(arch_x86_64)
   cur = OFFSETOF(user, u_debugreg);
#endif
   dynreg_to_user[x86_64::dr0]   = make_pair(cur, 8);
   dynreg_to_user[x86_64::dr1]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr2]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr3]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr4]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr5]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr6]   = make_pair(cur+=8, 8);
   dynreg_to_user[x86_64::dr7]   = make_pair(cur+=8, 8);

   cur = 0;
   if(sizeof(void *) == 8 ) {
       dynreg_to_user[ppc32::r0]        = make_pair(cur, 4);
       dynreg_to_user[ppc32::r1]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r2]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r3]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r4]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r5]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r6]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r7]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r8]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r9]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r10]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r11]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r12]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r13]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r14]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r15]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r16]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r17]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r18]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r19]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r20]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r21]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r22]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r23]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r24]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r25]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r26]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r27]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r28]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r29]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r30]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::r31]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::pc]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::msr]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::or3]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::ctr]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::lr]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::xer]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::cr]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::mq]         = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::trap]       = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::dar]        = make_pair(cur+=8, 4);
       dynreg_to_user[ppc32::dsisr]      = make_pair(cur+=8, 4);
   }else{
       dynreg_to_user[ppc32::r0]        = make_pair(cur, 4);
       dynreg_to_user[ppc32::r1]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r2]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r3]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r4]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r5]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r6]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r7]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r8]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r9]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r10]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r11]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r12]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r13]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r14]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r15]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r16]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r17]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r18]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r19]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r20]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r21]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r22]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r23]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r24]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r25]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r26]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r27]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r28]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r29]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r30]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::r31]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::pc]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::msr]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::or3]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::ctr]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::lr]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::xer]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::cr]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::mq]         = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::trap]       = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::dar]        = make_pair(cur+=4, 4);
       dynreg_to_user[ppc32::dsisr]      = make_pair(cur+=4, 4);
   }
   cur = 0;
   dynreg_to_user[ppc64::r0]        = make_pair(cur, 8);
   dynreg_to_user[ppc64::r1]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r2]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r3]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r4]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r5]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r6]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r7]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r8]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r9]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r10]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r11]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r12]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r13]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r14]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r15]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r16]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r17]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r18]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r19]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r20]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r21]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r22]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r23]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r24]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r25]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r26]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r27]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r28]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r29]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r30]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::r31]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::pc]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::msr]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::or3]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::ctr]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::lr]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::xer]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::cr]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::mq]         = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::trap]       = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::dar]        = make_pair(cur+=8, 8);
   dynreg_to_user[ppc64::dsisr]      = make_pair(cur+=8, 8);


   //according to /sys/user.h
   cur = 0;
   int step = 8;
   dynreg_to_user[aarch64::x0]         = make_pair(cur,    8);
   dynreg_to_user[aarch64::x1]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x2]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x3]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x4]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x5]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x6]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x7]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x8]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x9]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x10]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x11]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x12]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x13]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x14]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x15]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x16]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x17]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x18]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x19]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x20]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x21]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x22]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x23]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x24]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x25]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x26]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x27]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x28]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x29]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::x30]        = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::sp]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::pc]         = make_pair(cur+=step, 8);
   dynreg_to_user[aarch64::pstate]     = make_pair(cur+=step, 8);

   initialized = true;

   init_lock.unlock();
}

#if defined(PT_GETREGS)
#define MY_PTRACE_GETREGS PTRACE_GETREGS
#elif defined(arch_power)
//Kernel value for PPC_PTRACE_SETREGS 0x99
#define MY_PTRACE_GETREGS 12
#elif defined(arch_aarch64)
//leave blank
#endif

#if defined(arch_aarch64)
//31 GPR + SP + PC + PSTATE
#define MAX_USER_REGS 34
#define MAX_USER_SIZE (34*8)
#else
//912 is currently the x86_64 size, 128 bytes for just-because padding
#define MAX_USER_SIZE (912+128)
#endif
bool linux_thread::plat_getAllRegisters(int_registerPool &regpool)
{

#if defined(MY_PTRACE_GETREGS)
   static bool have_getregs = true;
#else
#define MY_PTRACE_GETREGS 0
   static bool have_getregs = false;
#endif
   static bool tested_getregs = false;

#if defined(bug_registers_after_exit)
   /* On some kernels, attempting to read registers from a thread in a pre-Exit
    * state causes an oops
    */
   if( isExiting() ) {
       perr_printf("Cannot reliably retrieve registers from an exited thread\n");
       setLastError(err_exited, "Cannot retrieve registers from an exited thread");
       return false;
   }
#endif

   volatile unsigned int sentinel1 = 0xfeedface;
   unsigned char user_area[MAX_USER_SIZE];
   volatile unsigned int sentinel2 = 0xfeedface;
   memset(user_area, 0, MAX_USER_SIZE);

   Dyninst::Architecture curplat = llproc()->getTargetArch();
   init_dynreg_to_user();
   dynreg_to_user_t::iterator i;

   if (have_getregs)
   {
      long result = do_ptrace((pt_req) MY_PTRACE_GETREGS, lwp, user_area, user_area);
      if (result != 0) {
         int error = errno;
         if (error == EIO && !tested_getregs) {
            pthrd_printf("PTRACE_GETREGS not working.  Trying PTRACE_PEEKUSER\n");
            have_getregs = false;
         }
         else {
            perr_printf("Error reading registers from %d\n", lwp);
            setLastError(err_internal, "Could not read user area from thread");
            return false;
         }
      }
      tested_getregs = true;
   }
   if (!have_getregs)
   {
#if defined(arch_aarch64)
        elf_gregset_t regs;
        struct iovec iovec;
        iovec.iov_base = &regs;
        iovec.iov_len = sizeof(regs);
        long ret = do_ptrace((pt_req)PTRACE_GETREGSET, lwp, (void *)NT_PRSTATUS, &iovec);
        if( ret < 0){
            perr_printf("-AARCH64: Unable to fetch registers!\n");
            return false;
        }
        memcpy(user_area, regs, iovec.iov_len);
#else
      for (i = dynreg_to_user.begin(); i != dynreg_to_user.end(); i++) {
         const MachRegister reg = i->first;
         if (reg.getArchitecture() != curplat)
            continue;
         long result = do_ptrace((pt_req) PTRACE_PEEKUSER, lwp, (void *) (unsigned long) i->second.first, NULL);
         //errno == -1 is not sufficient here for aarch4
         //if (errno == -1) {
         if (errno == -1 || result == -1) {
            int error = errno;
            perr_printf("Error reading registers from %d at %x\n", lwp, i->second.first);
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Could not read user area from thread");
            return false;
         }
         if (Dyninst::getArchAddressWidth(curplat) == 4) {
            uint32_t val = (uint32_t) result;
            write_memory_as(user_area + i->second.first, val);
         }
         else if (Dyninst::getArchAddressWidth(curplat) == 8) {
            uint64_t val = (uint64_t) result;
            write_memory_as(user_area + i->second.first, val);
         }
         else {
            assert(0);
         }
      }
#endif
   }

   //If a sentinel assert fails, then someone forgot to increase MAX_USER_SIZE
   // for a new platform.
   assert(sentinel1 == 0xfeedface);
   assert(sentinel2 == 0xfeedface);
   if(sentinel1 != 0xfeedface || sentinel2 != 0xfeedface) return false;


    regpool.regs.clear();
    for (i = dynreg_to_user.begin(); i != dynreg_to_user.end(); i++)
    {
        const MachRegister reg = i->first;
        MachRegisterVal val = 0;
        if (reg.getArchitecture() != curplat)
           continue;
        const unsigned int offset = i->second.first;
        const unsigned int size = i->second.second;
        if (size == 4) {
           if( sizeof(void *) == 8 ) {
              // Avoid endian issues
              auto tmpVal = Dyninst::read_memory_as<uint64_t>(user_area+offset);
              val = (uint32_t) tmpVal;
           }else{
              val = Dyninst::read_memory_as<uint32_t>(user_area+offset);
           }
        }
        else if (size == 8) {
           val = Dyninst::read_memory_as<uint64_t>(user_area+offset);
        }
        else {
           assert(0);
        }

        pthrd_printf("Register %2s has value %16lx, offset %u\n", reg.name().c_str(), val, offset);
        regpool.regs[reg] = val;
    }
    return true;
}

bool linux_thread::plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
#if defined(bug_registers_after_exit)
   /* On some kernels, attempting to read registers from a thread in a pre-Exit
    * state causes an oops
    */
   if( isExiting() ) {
       perr_printf("Cannot reliably retrieve registers from an exited thread\n");
       setLastError(err_exited, "Cannot retrieve registers from an exited thread");
       return false;
   }
#endif

   if (x86::fsbase == reg || x86::gsbase == reg
       || x86_64::fsbase == reg || x86_64::gsbase == reg) {
      return getSegmentBase(reg, val);
   }

   init_dynreg_to_user();
   dynreg_to_user_t::iterator i = dynreg_to_user.find(reg);
   if (i == dynreg_to_user.end() || reg.getArchitecture() != llproc()->getTargetArch()) {
      perr_printf("Recieved unexpected register %s on thread %d\n", reg.name().c_str(), lwp);
      setLastError(err_badparam, "Invalid register");
      return false;
   }

   const unsigned offset = i->second.first;
   const unsigned size = i->second.second;
   assert(sizeof(val) >= size);
   if(sizeof(val) < size) return false;

   val = 0;

/*
 * Here it is different for aarch64,
 * I have to use GETREGSET instead of PEEKUSER
 */
   long result;
#if defined(arch_aarch64)
   elf_gregset_t regs;
   struct iovec iovec;
   iovec.iov_base = &regs;
   iovec.iov_len = sizeof(regs);
   long ret = do_ptrace((pt_req)PTRACE_GETREGSET, lwp, (void *)NT_PRSTATUS, &iovec);
   //if( ret < 0){
   //    perr_printf("ERROR-ARM: Unable to fetch registers!\n");
   //    return false;
   //}
   result = regs[(int)(offset/8)]; //30, 31(sp), 32(pc), 33(pstate)
#else
   result = do_ptrace((pt_req) PTRACE_PEEKUSER, lwp, (void *) (unsigned long) offset, NULL);
#endif
   //unsigned long result = do_ptrace((pt_req) PTRACE_PEEKUSER, lwp, (void *) (unsigned long) offset, NULL);
#if defined(arch_aarch64)
   if (ret != 0) {
#else
   if (result == -1 && errno != 0) {
#endif
      int error = errno;
      perr_printf("Error reading registers from %d: %s\n", lwp, strerror(errno));
      //pthrd_printf("ARM-Info: offset(%d-%d)\n", (void *)(unsigned long)offset, offset/8);
      if (error == ESRCH)
         setLastError(err_internal, "Could not read register from thread");
      return false;
   }
   val = result;

   pthrd_printf("Register %s has value 0x%lx\n", reg.name().c_str(), val);
   return true;
}

#if defined(PT_SETREGS)
#define MY_PTRACE_SETREGS PT_SETREGS
#elif defined(arch_aarch64)
//leave blank
//#define MY_PTRACE_SETREGS PTRACE_SETREGSET
#else
//Common kernel value for PTRACE_SETREGS
#define MY_PTRACE_SETREGS 13
#endif

bool linux_thread::plat_setAllRegisters(int_registerPool &regpool)
{
#if defined(MY_PTRACE_SETREGS)
   static bool have_setregs = true;
#else
#define MY_PTRACE_SETREGS 0
   static bool have_setregs = false;
#endif
   static bool tested_setregs = false;
#if defined(bug_registers_after_exit)
   /* On some kernels, attempting to read registers from a thread in a pre-Exit
    * state causes an oops
    */
   if( isExiting() ) {
       perr_printf("Cannot reliably retrieve registers from an exited thread\n");
       setLastError(err_exited, "Cannot retrieve registers from an exited thread");
       return false;
   }
#endif


   if (have_setregs)
   {
      unsigned char user_area[MAX_USER_SIZE];
      //Fill in 'user_area' with the contents of regpool.
      if( !plat_convertToSystemRegs(regpool, user_area) ) return false;

      //Double up the user_area parameter because if MY_PTRACE_SETREGS is
      // defined to PPC_PTRACE_SETREGS than the parameters data and addr
      // pointers get swapped (just because linux hates us).  Since the
      // other is ignored, we pass it in twice.
      int result = do_ptrace((pt_req) MY_PTRACE_SETREGS, lwp, user_area, user_area);
      if (result != 0) {
         int error = errno;
         if (error == EIO && !tested_setregs) {
            pthrd_printf("PTRACE_SETREGS not working.  Trying PTRACE_POKEUSER\n");
            have_setregs = false;
         }
         else {
            perr_printf("Error setting registers for %d\n", lwp);
            setLastError(err_internal, "Could not read user area from thread");
            return false;
         }
      }
      tested_setregs = true;
   }
   if (!have_setregs)
   {
#if defined(arch_aarch64)
        //pthrd_printf("ARM-info: setAllregisters.\n");
        elf_gregset_t regs;
        struct iovec iovec;
        long ret;
        iovec.iov_base = &regs;
        iovec.iov_len = sizeof(regs);

        //set regs
        for (int_registerPool::iterator i = regpool.regs.begin(); i != regpool.regs.end(); i++) {
            dynreg_to_user_t::iterator di = dynreg_to_user.find(i->first);
            assert(di != dynreg_to_user.end());
            int regs_pos = (int)(di->second.first / sizeof(unsigned long));
            assert( regs_pos < MAX_USER_REGS);
            regs[regs_pos] = i->second;
        }

        //store them back
        ret = do_ptrace((pt_req)PTRACE_SETREGSET, lwp, (void *)NT_PRSTATUS, &iovec);
        if( ret < 0 ){
            int error = errno;
            //perr_printf("ERROR-ARM: Unable to set registers: %s\n", strerror(error) );
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Could not read user area from thread");
            return false;
        }
#else //not aarch64
      Dyninst::Architecture curplat = llproc()->getTargetArch();
      init_dynreg_to_user();
      for (int_registerPool::iterator i = regpool.regs.begin(); i != regpool.regs.end(); i++) {
         assert(i->first.getArchitecture() == curplat);
         dynreg_to_user_t::iterator di = dynreg_to_user.find(i->first);
         assert(di != dynreg_to_user.end());

         //Don't treat errors on these registers as real errors.
         bool not_present = true;
         if (curplat == Arch_ppc32)
            not_present = (i->first == ppc32::mq || i->first == ppc32::dar ||
                           i->first == ppc32::dsisr || i->first == ppc32::trap ||
                           i->first == ppc32::or3);

         if (not_present)
            continue;

         int result;
         uintptr_t res;
         if (Dyninst::getArchAddressWidth(curplat) == 4) {
            res = (uint32_t) i->second;
         }
         else {
            res = (uint64_t) i->second;
         }
         result = do_ptrace((pt_req) PTRACE_POKEUSER, lwp, (void *) (unsigned long) di->second.first, (void *) res);

         //if (result != 0) {
         if (result != 0) {
            int error = errno;
            perr_printf("Error setting register %s for %d at %d: %s\n", i->first.name().c_str(),
                        lwp, (int) di->second.first, strerror(error));
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Could not read user area from thread");
            return false;
         }
      }
#endif
   }

   pthrd_printf("Successfully set the values of all registers for %d\n", lwp);
   return true;
}

bool linux_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area,
                                            bool gprs_only)
{
   init_dynreg_to_user();

   Architecture curplat = llproc()->getTargetArch();
   unsigned num_found = 0;
   for (dynreg_to_user_t::const_iterator i = dynreg_to_user.begin(); i != dynreg_to_user.end(); i++)
   {
      const MachRegister reg = i->first;
      MachRegisterVal val;
      if (reg.getArchitecture() != curplat)
         continue;

      if (gprs_only) {
         bool is_gpr;
         int rclass = (int) reg.regClass();

         switch (llproc()->getTargetArch()) {
            //In this case our definition of GPR is anything stored in the elf_gregset_t of
            // the user struct.
            case Dyninst::Arch_x86:
               is_gpr = ((rclass == x86::GPR) || (rclass == x86::FLAG) ||
                         (rclass == x86::MISC) || (rclass == x86::SEG) || !rclass);
               break;
            case Dyninst::Arch_x86_64:
               is_gpr = ((rclass == x86_64::GPR) || (rclass == x86_64::FLAG) ||
                         (rclass == x86_64::MISC) || (rclass == x86_64::SEG) || !rclass);
               break;
            case Dyninst::Arch_ppc32:
               is_gpr = true;
               break;
            case Dyninst::Arch_ppc64:
               is_gpr = true;
               break;
            case Dyninst::Arch_aarch64:
               is_gpr = true;
               break;
            default:
               assert(0);
	       return false;
         }

         if (!is_gpr) {
            continue;
         }
      }

      num_found++;
      const unsigned int offset = i->second.first;
      const unsigned int size = i->second.second;
      assert(offset+size < MAX_USER_SIZE);

      if ((offset+size) > sizeof(prgregset_t)) continue;

      int_registerPool::reg_map_t::const_iterator j = regpool.regs.find(reg);
      assert(j != regpool.regs.end());
      val = j->second;

      if (size == 4) {
          if( sizeof(void *) == 8 ) {
              write_memory_as(user_area+offset, uint64_t{val});
          } else {
              write_memory_as(user_area+offset, static_cast<uint32_t>(val));
          }
      }
      else if (size == 8) {
         write_memory_as(user_area+offset, uint64_t{val});
      }
      else {
         assert(0);
      }
      pthrd_printf("Register %s gets value %lx, offset %u\n", reg.name().c_str(), val, offset);
   }

   if (!gprs_only && (num_found != regpool.regs.size()))
   {
      setLastError(err_badparam, "Invalid register set passed to setAllRegisters");
      perr_printf("Couldn't find all registers in the register set %u/%u\n", num_found,
                  (unsigned int) regpool.regs.size());
      return false;
   }

   return true;
}

bool linux_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
#if defined(bug_registers_after_exit)
   /* On some kernels, attempting to read registers from a thread in a pre-Exit
    * state causes an oops
    */
   if( isExiting() ) {
       perr_printf("Cannot reliably retrieve registers from an exited thread\n");
       setLastError(err_exited, "Cannot retrieve registers from an exited thread");
       return false;
   }
#endif

   init_dynreg_to_user();
   dynreg_to_user_t::iterator i = dynreg_to_user.find(reg);
   if (reg.getArchitecture() != llproc()->getTargetArch() ||
       i == dynreg_to_user.end())
   {
      setLastError(err_badparam, "Invalid register passed to setRegister");
      perr_printf("User passed invalid register %s to plat_setRegister, arch is %x\n",
                  reg.name().c_str(), (unsigned int) reg.getArchitecture());
      return false;
   }

   const unsigned int offset = i->second.first;
   const unsigned int size = i->second.second;
   int result;
   uintptr_t value;
   if (size == 4) {
      value = (uint32_t) val;
   }
   else if (size == 8) {
      value = (uint64_t) val;
   }
   else {
      assert(0);
      return false;

   }

#if defined(arch_aarch64)
   elf_gregset_t regs;
   struct iovec iovec;
   long ret;
   iovec.iov_base = &regs;
   iovec.iov_len = sizeof(regs);
   //first get
   ret = do_ptrace((pt_req)PTRACE_GETREGSET, lwp, (void *)NT_PRSTATUS, &iovec);
   if( ret < 0){
       perr_printf("ERROR-ARM: Unable to fetch registers!\n");
       return false;
   }

   //set the corresponding reg
   assert( (int)(offset/8) < 34 );
   regs[(int)(offset/8)] = value;

   //store them back
   ret = do_ptrace((pt_req)PTRACE_SETREGSET, lwp, (void *)NT_PRSTATUS, &iovec);
   if( ret < 0 ){
       perr_printf("ERROR-ARM: Unable to set registers!\n");
       return false;
   }
   result = ret;
#else
   result = do_ptrace((pt_req) PTRACE_POKEUSER, lwp, (void *) (uintptr_t)offset, (void *) value);
#endif
   //result = do_ptrace((pt_req) PTRACE_POKEUSER, lwp, (void *) (uintptr_t)offset, (void *) value);
   pthrd_printf("Set register %s (size %u, offset %u) to value %lx\n", reg.name().c_str(), size, offset, val);
   if (result != 0) {
      int error = errno;
      if (error == ESRCH)
         setLastError(err_exited, "Process exited during operation");
      else
         setLastError(err_internal, "Could not set register value");
      perr_printf("Unable to set value of register %s in thread %d: %s (%d)\n",
                  reg.name().c_str(), lwp, strerror(error), error);
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

bool linux_thread::plat_handle_ghost_thread() {
	std::string loc = "/proc/" + std::to_string(proc()->getPid()) + "/task/" + std::to_string(getLWP());
	struct stat dummy;
	int res = stat(loc.c_str(), &dummy);
	pthrd_printf("GHOST_THREAD: stat=%d, loc=%s\n", res, loc.c_str());

	// If the thread is still alive, do nothing
	if(res != -1) {
		// NB: We got here because ptrace returned ESRCH (no such process) in LinuxPtrace::ptrace_int,
		//     yet the process is alive. This is a valid state for ptrace, but ptrace_int doesn't check
		//     for this. The ptrace man-page indicates that this can happen when a process received a STOP
		//     signal, but it hasn't transitioned to the new state yet.
		return false;
	}

	auto *initial_thread = llproc()->threadPool()->initialThread();

	// Do not create a destroy event for the thread executed from 'main'
	if(initial_thread != thread()->llthrd()) {
		EventLWPDestroy::ptr lwp_ev = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Post));
		lwp_ev->setSyncType(Event::async);
		lwp_ev->setThread(thread());
		lwp_ev->setProcess(proc());
		dynamic_cast<linux_process*>(proc()->llproc())->decodeTdbLWPExit(lwp_ev);
		pthrd_printf("GHOST THREAD: Enqueueing event for %d/%d\n",
				proc()->getPid(), getLWP());
		mbox()->enqueue(lwp_ev, true);
	}
	return true;
  }

bool linux_thread::attach()
{
   if (llproc()->threadPool()->initialThread() == this) {
      return true;
   }

   if (attach_status != as_needs_attach)
   {
      pthrd_printf("thread::attach called on running thread %d/%d, should "
                   "be auto-attached.\n", llproc()->getPid(), lwp);
      return true;
   }

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
// for aarch64
#if !defined(PTRACE_ARM_GET_THREAD_AREA)
#define PTRACE_ARM_GET_THREAD_AREA 22
#endif

bool linux_thread::thrdb_getThreadArea(int val, Dyninst::Address &addr)
{
   Dyninst::Architecture arch = llproc()->getTargetArch();
   switch (arch) {
      case Arch_x86: {
         uint32_t addrv[4];
         int result = do_ptrace((pt_req) PTRACE_GET_THREAD_AREA, lwp, (void *) (intptr_t)val, &addrv);
         if (result != 0) {
            int error = errno;
            perr_printf("Error doing PTRACE_GET_THREAD_AREA on %d/%d: %s\n", llproc()->getPid(), lwp, strerror(error));
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Error doing PTRACE_GET_THREAD_AREA\n");
            return false;
         }
         addr = (Dyninst::Address) addrv[1];
         break;
      }
      case Arch_x86_64: {
         intptr_t op;
         if (val == FS_REG_NUM)
            op = ARCH_GET_FS;
         else if (val == GS_REG_NUM)
            op = ARCH_GET_GS;
         else {
            perr_printf("Bad value (%d) passed to thrdb_getThreadArea\n", val);
            return false;
         }
         uint64_t addrv = 0;
         int result = do_ptrace((pt_req) PTRACE_ARCH_PRCTL, lwp, &addrv, (void *) op);
         if (result != 0) {
            int error = errno;
            perr_printf("Error doing PTRACE_ARCH_PRCTL on %d/%d: %s\n", llproc()->getPid(), lwp, strerror(error));
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Error doing PTRACE_ARCH_PRCTL\n");
            return false;
         }
         addr = (Dyninst::Address) addrv;
         break;
      }
      case Arch_aarch64:{
#if defined(arch_aarch64)
         struct iovec iovec;
         uint64_t reg;

         iovec.iov_base = &reg;
         iovec.iov_len = sizeof (reg);

         int result = do_ptrace((pt_req) PTRACE_GETREGSET, lwp, (void *)NT_ARM_TLS, &iovec);
         if (result != 0) {
            int error = errno;
            perr_printf("Error doing PTRACE_ARM_GET_THREAD_AREA on %d/%d: %s\n", llproc()->getPid(), lwp, strerror(error));
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            else
               setLastError(err_internal, "Error doing PTRACE_ARM_GET_THREAD_AREA\n");
            return false;
         }
         addr = (Dyninst::Address) (reg-val);
#else
         assert(0);
#endif
         break;
      }
      default:
         assert(0); //Should not be needed on non-x86 and non-arm
   }
   return true;
}

//Copied from /usr/include/asm/ldt.h, as it was not available on all machines
struct linux_x86_user_desc {
   unsigned int  entry_number;
   unsigned long base_addr;
   unsigned int  limit;
   unsigned int  seg_32bit:1;
   unsigned int  contents:2;
   unsigned int  read_exec_only:1;
   unsigned int  limit_in_pages:1;
   unsigned int  seg_not_present:1;
   unsigned int  useable:1;
};

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
         struct linux_x86_user_desc entryDesc;

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
           pthrd_printf("Failed to get segment base with selector %s\n", segmentSelectorReg.name().c_str());
           return false;
         }
         entryNumber = segmentSelectorVal / 8;

         pthrd_printf("Get segment base doing PTRACE with entry %lu\n", entryNumber);
         long result = do_ptrace((pt_req) PTRACE_GET_THREAD_AREA,
                                 lwp, (void *) entryNumber, (void *) &entryDesc);
         if (result == -1 && errno != 0) {
            int error = errno;
            pthrd_printf("PTRACE to get segment base failed: %s\n", strerror(errno));
            if (error == ESRCH)
               setLastError(err_exited, "Process exited during operation");
            return false;
         }

         val = entryDesc.base_addr;
         pthrd_printf("Got segment base: 0x%lx\n", val);
         return true;
      }
      default:
         assert(!"This is not implemented on this architecture");
         return false;
   }
 }

bool linux_thread::suppressSanityChecks()
{
   return generator_started_exit_processing;
}

linux_x86_thread::linux_x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   thread_db_thread(p, t, l),
   linux_thread(p, t, l),
   x86_thread(p, t, l)
{
}

linux_x86_thread::~linux_x86_thread()
{
}

linux_ppc_thread::linux_ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   thread_db_thread(p, t, l),
   linux_thread(p, t, l),
   ppc_thread(p, t, l)
{
}

linux_ppc_thread::~linux_ppc_thread()
{
}

linux_arm_thread::linux_arm_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   thread_db_thread(p, t, l),
   linux_thread(p, t, l),
   arm_thread(p, t, l)
{
}

linux_arm_thread::~linux_arm_thread()
{
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
   child_pid(NULL_PID),
   event_ext(0)
{
}

ArchEventLinux::ArchEventLinux(int e) :
   status(0),
   pid(NULL_PID),
   interrupted(false),
   error(e),
   child_pid(NULL_PID),
   event_ext(0)
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
   {
      assert(0);
      return false;
   }

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
      thr = dynamic_cast<linux_thread *>(ev->getThread()->llthrd());
   }
   else if (ev->getEventType().code() == EventType::ThreadCreate) {
      Dyninst::LWP lwp = static_cast<EventNewThread *>(ev.get())->getLWP();
      ProcPool()->condvar()->lock();
      thr = dynamic_cast<linux_thread *>(ProcPool()->findThread(lwp));
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

LinuxHandleLWPDestroy::LinuxHandleLWPDestroy()
    : Handler("Linux LWP Destroy")
{
}

LinuxHandleLWPDestroy::~LinuxHandleLWPDestroy()
{
}

Handler::handler_ret_t LinuxHandleLWPDestroy::handleEvent(Event::ptr ev) {
    int_thread *thrd = ev->getThread()->llthrd();

    // This handler is necessary because SIGSTOPS cannot be sent to pre-destroyed
    // threads -- these stops will never be delivered to the debugger
    //
    // Setting the exiting state in the thread will avoid any waiting for pending stops
    // on this thread

    thrd->setExiting(true);

    // If there is a pending stop, need to handle it here because there is
    // no guarantee that the stop will ever be received
    if( thrd->hasPendingStop() ) {
       thrd->setPendingStop(false);
    }

    return ret_success;
}

int LinuxHandleLWPDestroy::getPriority() const
{
    return PostPlatformPriority;
}

void LinuxHandleLWPDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Pre, EventType::LWPDestroy));
}

LinuxHandleForceTerminate::LinuxHandleForceTerminate() :
   Handler("Linux Force Termination") {}

LinuxHandleForceTerminate::~LinuxHandleForceTerminate() {}

Handler::handler_ret_t LinuxHandleForceTerminate::handleEvent(Event::ptr ev) {
   int_process *proc = ev->getProcess()->llproc();

   for (int_threadPool::iterator iter = proc->threadPool()->begin();
        iter != proc->threadPool()->end(); ++iter) {
      do_ptrace((pt_req) PTRACE_DETACH, (*iter)->getLWP(), NULL, NULL);
   }
   return ret_success;
}

int LinuxHandleForceTerminate::getPriority() const
{
   return PostPlatformPriority;
}

void LinuxHandleForceTerminate::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::ForceTerminate));
}

HandlerPool *linux_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool initialized = false;
   static LinuxHandleNewThr *lbootstrap = NULL;
   static LinuxHandleForceTerminate *lterm = NULL;
   if (!initialized) {
      lbootstrap = new LinuxHandleNewThr();
      lterm = new LinuxHandleForceTerminate();
      initialized = true;
   }
   hpool->addHandler(lbootstrap);
   hpool->addHandler(lterm);
   thread_db_process::addThreadDBHandlers(hpool);
   sysv_process::addSysVHandlers(hpool);
   return hpool;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   return linux_createDefaultHandlerPool(hpool);
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
   proc(NULL),
   remote_addr(0),
   size(0),
   ret(0),
   bret(false),
   err(0)
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
	    errno = 0;
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


void linux_process::plat_adjustSyncType(Event::ptr ev, bool gen_)
{
   if (gen_) return;

   if (ev->getEventType().code() != EventType::LWPDestroy ||
       ev->getEventType().time() != EventType::Pre)
      return;

   int_thread *thrd = ev->getThread()->llthrd();
   if(!thrd) return;
   if (thrd->getGeneratorState().getState() != int_thread::running)
      return;

   // So we have a pre-LWP destroy and a running generator; this means
   // that someone continued the thread during decode and it is now
   // gone. So set the event to async and set the generator state to
   // exited.

   pthrd_printf("plat_adjustSyncType: thread %d raced with exit, setting event to async\n",
                thrd->getLWP());

   //thrd->getGeneratorState().setState(int_thread::exited);
   ev->setSyncType(Event::async);
   //thrd->getHandlerState().setState(int_thread::exited);
}


