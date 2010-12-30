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

#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/dyntypes.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/snippets.h"
#include "proccontrol/src/freebsd.h"
#include "proccontrol/src/int_handler.h"
#include "common/h/freebsdKludges.h"
#include "common/h/SymLite-elf.h"

// System includes
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <machine/reg.h>
#include <sched.h>
#include <sys/event.h>

#if defined(arch_x86) || defined(arch_x86_64)
#include <machine/psl.h>
#endif

#include <cassert>
#include <cerrno>

#include <map>
using std::make_pair;

using namespace Dyninst;
using namespace ProcControlAPI;

/*
 * FreeBSD Bug Descriptions
 *
 * --- bug_freebsd_missing_sigstop ---
 *
 * This bug is documented in FreeBSD problem report kern/142757 -- if we yield
 * to the scheduler, we may avoid the race condition described in this bug
 * report.
 *
 * The basic idea of the workaround it is to yield to other processes (and the
 * debuggee) to avoid the race condition described in the bug report. A call
 * to sched_yield attempts to do this.
 * 
 * Specifically, the race condition results from the following sequence of
 * events, where P = parent process, C = child process:
 * 
 * P-SIGSTOP C-STOP P-WAITPID P-CONT P-SIGSTOP C-RUN
 * 
 * Because the child doesn't run before the parent sends the second stop
 * signal, the child doesn't receive the second SIGSTOP.
 * 
 * A workaround for this problem would guarantee that the C-RUN event always
 * occurs before the second P-SIGSTOP.
 *
 * --- bug_freebsd_mt_suspend ---
 *
 * Here is the scenario:
 *
 * 1) The user requests an attach to a multithreaded process
 * 2) ProcControl issues the attach and waits for the stop
 * 3) On receiving the stop, ProcControl suspends each thread
 *    in the process.
 * 4) User continues a single thread but other threads which
 *    should remain stopped continue to run
 *
 * The workaround for this bug is to issue SIGSTOPs to all the
 * threads (except the thread that received the SIGSTOP from the
 * initial attach). Once the stops are handled, the suspend
 * state will be honored when continuing a whole process; that is,
 * when all threads are suspended, a continue of one thread will
 * not continue another thread.
 *
 * See the Stop, Bootstrap event handlers and the post_attach
 * function for more detailed comments.
 *
 * TODO submit a problem report to FreeBSD devs for this
 *
 * --- bug_freebsd_change_pc ---
 *
 * Here is the scenario:
 *
 * 1) A signal is received on a thread in a multithreaded process
 *    Let T = a thread that did not receive the signal and is blocked
 *    in the kernel.
 *
 * 2) This signal generates a ProcControl event
 * 3) In the handler/callback for this event, the PC of T is changed.
 * 4) When T is continued, it remains blocked in the kernel and does
 *    not run at the new PC.
 *
 * This bug manifests itself in the iRPC tests in the testsuite. The
 * debuggee's threads are blocked in the kernel on a mutex and the iRPC
 * tests change the PCs of these threads to run the RPCs. Specifically,
 * this bug is encountered when RPCs are posted from callbacks because
 * the thread hasn't been stopped directly by a signal rather by another
 * thread in the process receiving a signal.
 *
 * The workaround is to send a signal to a thread after it's PC has been
 * changed in a callback or due to prepping an RPC. The handler for this
 * thread only unsets a flag in the thread. This required a special event,
 * ChangePCStop, which is only defined on platforms this bug affects.
 *  
 * See the ChangePCHandler as well for more info.
 *
 * TODO submit a problem report (or possibly a feature request) to 
 * FreeBSD devs for this
 *
 * --- bug_freebsd_lost_signal ---
 *
 * This bug is documented in FreeBSD problem report kern/150138 -- it also
 * includes a patch that fixes the problem.
 *
 * Here is the scenario:
 *
 * 1) A signal (such as a SIGTRAP) is delivered to a specific thread, which
 * stops the whole process.
 *
 * 2) The user (or handler) thread wishes to make a specific thread stop. Not
 * knowing that the process has already been stopped, a SIGSTOP is sent to the
 * thread. The user (or handler) thread then waits for the stop event.
 *
 * 3) The generator observes the SIGTRAP event (via waitpid) and generates the
 * event for it.  
 *
 * 4) The user (or handler) thread handle the SIGTRAP event but continues to
 * wait on the pending stop.
 *
 * At this point, the user (or handler) thread could wait indefinitely for the
 * pending stop, if the debuggee is blocked in the kernel. The problem is that
 * when the process is continued after the SIGSTOP is sent to the stopped 
 * process, the thread blocked in the kernel does not wakeup to handle the
 * new signal that was delivered to it while it was stopped.
 *
 * Unfortunately, I was not able to come up with a solid workaround for this bug
 * because there wasn't a race-free way to determine if the process was stopped
 * before sending it a signal (i.e., after checking whether the process was stopped,
 * the OS could deschedule the ProcControl process and the debuggee could hit a
 * breakpoint or finish an iRPC, resulting in ProcControl's model of the debuggee
 * being inconsistent).
 */

Generator *Generator::getDefaultGenerator() {
    static GeneratorFreeBSD *gen = NULL;
    if( NULL == gen ) {
        gen = new GeneratorFreeBSD();
        assert(gen);
        gen->launch();
    }
    return static_cast<Generator *>(gen);
}

GeneratorFreeBSD::GeneratorFreeBSD() :
    GeneratorMT(std::string("FreeBSD Generator"))
{
    decoders.insert(new DecoderFreeBSD());
}

GeneratorFreeBSD::~GeneratorFreeBSD()
{
}

bool GeneratorFreeBSD::initialize() {
    return true;
}

bool GeneratorFreeBSD::canFastHandle() {
    return false;
}

ArchEvent *GeneratorFreeBSD::getEvent(bool block) {
    int status, options;

    // Block (or not block) in waitpid to receive a OS event
    options = block ? 0 : WNOHANG;
    pthrd_printf("%s in waitpid\n", block ? "blocking" : "polling");
    int pid = waitpid(-1, &status, options);
    lwpid_t lwp = NULL_LWP;

    ArchEventFreeBSD *newevent = NULL;
    if (pid == -1) {
        int errsv = errno;
        if (errsv == EINTR) {
            pthrd_printf("waitpid interrupted\n");
            newevent = new ArchEventFreeBSD(true);
            return newevent;
        }
        perr_printf("Error. waitpid recieved error %s\n", strerror(errsv));
        newevent = new ArchEventFreeBSD(errsv);
        return newevent;
    }

    if (dyninst_debug_proccontrol)
    {
        pthrd_printf("Waitpid return status %x for pid %d:\n", status, pid);
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

    // On FreeBSD, we need to get information about the thread that caused the
    // stop via ptrace -- try for all cases; the ptrace call will fail for some
    struct ptrace_lwpinfo lwpInfo;
    if( 0 == ptrace(PT_LWPINFO, pid, (caddr_t)&lwpInfo, sizeof(struct ptrace_lwpinfo)) ) {
        lwp = lwpInfo.pl_lwpid;
    }else{
        int errsv = errno;
        pthrd_printf("Failed to retrieve lwp for pid %d: %s\n", pid, strerror(errsv));

        // It's an error for a stopped process
        if( WIFSTOPPED(status) ) {
            newevent = new ArchEventFreeBSD(errsv);
            return newevent;
        }
    }

    newevent = new ArchEventFreeBSD(pid, lwp, status);
    return newevent;
}

ArchEventFreeBSD::ArchEventFreeBSD(bool inter_) :
    status(0),
    pid(NULL_PID),
    lwp(NULL_LWP),
    interrupted(inter_),
    error(0)
{
}

ArchEventFreeBSD::ArchEventFreeBSD(pid_t p, lwpid_t l, int s) :
    status(s),
    pid(p),
    lwp(l),
    interrupted(false),
    error(0)
{
}

ArchEventFreeBSD::ArchEventFreeBSD(int e) :
    status(0),
    pid(NULL_PID),
    lwp(NULL_LWP),
    interrupted(false),
    error(e)
{
}
      
ArchEventFreeBSD::~ArchEventFreeBSD()
{
}

DecoderFreeBSD::DecoderFreeBSD()
{
}

DecoderFreeBSD::~DecoderFreeBSD()
{
}

unsigned DecoderFreeBSD::getPriority() const
{
    return Decoder::default_priority;
}

Dyninst::Address DecoderFreeBSD::adjustTrapAddr(Dyninst::Address addr, Dyninst::Architecture arch)
{
    if( arch == Dyninst::Arch_x86 || arch == Dyninst::Arch_x86_64 ) {
        return addr - 1;
    }

    return addr;
}

static
const int PC_BUG_SIGNAL = SIGUSR1;

bool DecoderFreeBSD::decode(ArchEvent *ae, std::vector<Event::ptr> &events) {
    ArchEventFreeBSD *archevent = static_cast<ArchEventFreeBSD *>(ae);

    int_process *proc = NULL;
    int_thread *thread = NULL;
    freebsd_process *lproc = NULL;
    freebsd_thread *lthread = NULL;

    proc = ProcPool()->findProcByPid(archevent->pid);

    /* ignore any events we get for processes we don't know anything about.
     * This occurs for a double exit event as described below near
     * WIFEXITED
     */
    if( !proc ) {
        pthrd_printf("Ignoring ArchEvent for unknown process with PID %d\n",
                archevent->pid);
        return true;
    }

    lproc = dynamic_cast<freebsd_process *>(proc);

    thread = ProcPool()->findThread(archevent->lwp);

    // If the ArchEvent did not set the LWP, assign the event to the initial thread
    if( !thread ) {
        thread = proc->threadPool()->initialThread();
    }
    lthread = static_cast<freebsd_thread *>(thread);

    Event::ptr event;

    pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
            thread ? thread->getLWP() : -1);

    const int status = archevent->status;
    bool result = false;
    bool multipleEvents = false;
    if( WIFSTOPPED(status) ) {
        const int stopsig = WSTOPSIG(status);

        lthread->setSignalStopped(true);
        for(int_threadPool::iterator i = proc->threadPool()->begin();
            i != proc->threadPool()->end(); ++i)
        {
            if( (*i) != lthread ) {
                static_cast<freebsd_thread *>(*i)->setSignalStopped(false);
            }
        }

        pthrd_printf("Decoded to signal %s\n", strsignal(stopsig));
        switch( stopsig ) {
            case SIGUSR2:
            case SIGSTOP: {
                if( lthread->hasPendingStop() || lthread->hasBootstrapStop() ) {
                    pthrd_printf("Received pending stop on %d/%d\n",
                                 lthread->llproc()->getPid(), lthread->getLWP());
                    event = Event::ptr(new EventStop());
                    break;
                }

                // Relying on fall through for bootstrap and other SIGSTOPs
            }
            case SIGTRAP: {
                if( proc->getState() == int_process::neonatal_intermediate) {
                    pthrd_printf("Decoded event to bootstrap on %d/%d\n",
                            proc->getPid(), thread->getLWP());
                    event = Event::ptr(new EventBootstrap());
                    break;
                }

                Dyninst::MachRegisterVal addr;
		reg_response::ptr regvalue = reg_response::createRegResponse();
                result = thread->getRegister(MachRegister::getPC(proc->getTargetArch()), regvalue);
		bool is_ready = regvalue->isReady();
                if (!result || regvalue->hasError()) {
                    perr_printf("Failed to read PC address upon SIGTRAP\n");
                    return false;
                }
		assert(is_ready);	       
		addr = regvalue->getResult();
		regvalue = reg_response::ptr();

                Dyninst::Address adjusted_addr = adjustTrapAddr(addr, proc->getTargetArch());

                // Check if it is a RPC
                if (rpcMgr()->isRPCTrap(thread, adjusted_addr)) {
                   pthrd_printf("Decoded event to rpc completion on %d/%d at %lx\n",
                                proc->getPid(), thread->getLWP(), adjusted_addr);
                   event = Event::ptr(new EventRPC(thread->runningRPC()->getWrapperForDecode()));
                   break;
                }

                // Check if it is a breakpoint
                installed_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);
                if( ibp && ibp != thread->isClearingBreakpoint() ) {
                    pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(),
                            thread->getLWP(), adjusted_addr);
                    event = Event::ptr(new EventBreakpoint(adjusted_addr, ibp));

                    if( adjusted_addr == lproc->getLibBreakpointAddr() ) {
                        pthrd_printf("Breakpoint is library load/unload\n");
                        Event::ptr lib_event = Event::ptr(new EventLibrary());
                        lib_event->setThread(thread->thread());
                        lib_event->setProcess(proc->proc());
                        event->addSubservientEvent(lib_event);
                    }else{
                        // Check for thread events
                        vector<Event::ptr> threadEvents;
                        if( lproc->getEventsAtAddr(adjusted_addr, lthread, threadEvents) ) {
                            vector<Event::ptr>::iterator threadEventIter;
                            for(threadEventIter = threadEvents.begin(); threadEventIter != threadEvents.end();
                                    ++threadEventIter)
                            {
                                // An event is created for the initial thread, but this thread is already
                                // created during bootstrap. Ignore any create events for threads that
                                // are already created.
                                if( (*threadEventIter)->getEventType().code() == EventType::ThreadCreate ) {
                                    Dyninst::LWP createdLWP = (*threadEventIter)->getEventNewThread()->getLWP();
                                    int_thread * newThread = ProcPool()->findThread(createdLWP);
                                    if( NULL != newThread ) {
                                        freebsd_thread *newlThread = static_cast<freebsd_thread *>(newThread);
                                        pthrd_printf("Thread already created for %d/%d\n", proc->getPid(),
                                                createdLWP);

                                        if( !newlThread->plat_resume() ) {
                                            perr_printf("Failed to resume already created thread %d/%d\n", 
                                                    proc->getPid(), createdLWP);
                                            return false;
                                        }

                                        // Still need to enable events for it -- this can't be done during
                                        // bootstrap because the thread_db library isn't guaranteed to 
                                        // be initialized at bootstrap
                                        newlThread->setEventReporting(true);
                                        continue;
                                    }
                                }

                                (*threadEventIter)->setThread(thread->thread());
                                (*threadEventIter)->setProcess(proc->proc());
                                event->addSubservientEvent(*threadEventIter);
                            }
                        }
                    }
                    break;
                }

                if( thread->singleStep() ) {
                    ibp = thread->isClearingBreakpoint();
                    if( ibp ) {
                        pthrd_printf("Decoded event to breakpoint cleanup\n");
                        event = Event::ptr(new EventBreakpointClear(ibp));

                        // For a thread_db_process, post-ThreadDestroy events happen
                        // after a BreakpointClear
                        vector<Event::ptr> destroyEvents;
                        if( lproc->getPostDestroyEvents(destroyEvents) ) {
                            vector<Event::ptr>::iterator eventIter;
                            for(eventIter = destroyEvents.begin(); eventIter != destroyEvents.end(); ++eventIter) {
                                event->addSubservientEvent(*eventIter);
                                (*eventIter)->getThread()->llthrd()->setGeneratorState(int_thread::exited);
                            }
                        }
                    }else{
                        pthrd_printf("Decoded event to single step on %d/%d\n",
                                proc->getPid(), thread->getLWP());
                        event = Event::ptr(new EventSingleStep());
                    }
                    break;
                }else{
                    // Check the kqueue for events. If an exec occurred, a
                    // kevent should have been generated.
                    
                    int eventQueue;
                    if( -1 == (eventQueue = lproc->getEventQueue()) ) {
                        return false;
                    }

                    struct kevent queueEvent;
                    struct timespec nullSpec;
                    memset(&nullSpec, 0, sizeof(struct timespec));
                    int numEvents;
                    if( -1 == (numEvents = kevent(eventQueue, NULL, 0, &queueEvent, 1, &nullSpec)) ) {
                        perr_printf("low level failure: kevent: %s\n",
                                strerror(errno));
                        return false;
                    }

                    if( numEvents > 1 ) {
                        perr_printf("low level failure: kevent returned more than 1 event\n");
                        return false;
                    }

                    if( numEvents == 1 && queueEvent.fflags & NOTE_EXEC ) {
                        pthrd_printf("Decoded event to exec on %d/%d\n",
                                proc->getPid(), thread->getLWP());
                        event = Event::ptr(new EventExec(EventType::Post));
                        event->setSyncType(Event::sync_process);
                        break;
                    }
                }
                /* Relying on fallthrough to handle any other signals */
                pthrd_printf("No special events found for this signal\n");
            }
            default:
                pthrd_printf("Decoded event to signal %d on %d/%d\n",
                        stopsig, proc->getPid(), thread->getLWP());

                if( lthread->hasPCBugCondition() && stopsig == PC_BUG_SIGNAL ) {
                    pthrd_printf("Decoded event to change PC stop\n");
                    event = Event::ptr(new EventChangePCStop());
                    break;
                }
#if 0
                //Debugging code
                if (stopsig == SIGSEGV) {
                   Dyninst::MachRegisterVal addr;
                   result = thread->getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
                   if (!result) {
                      fprintf(stderr, "Failed to read PC address upon crash\n");
                   }
                   fprintf(stderr, "Got crash at %lx\n", addr);

                   unsigned char bytes[10];
                   proc->readMem((void *)bytes, addr, 10);

                   for(int i = 0; i < 10; i++) {
                       fprintf(stderr, "%#hx ", bytes[i]);
                   }
                   fprintf(stderr, "\n");

                   ptrace(PT_CONTINUE, proc->getPid(), (caddr_t)1, SIGABRT);
                   assert(!"Received SIGSEGV");
                }
#endif
                event = Event::ptr(new EventSignal(stopsig));
                break;
        }
    }
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        if( WIFEXITED(status) ) {
            int exitcode = WEXITSTATUS(status);
            /*
             * On FreeBSD, an exit of a traced process is reported to both its original
             * parent and the tracing process. When the traced process is created by a
             * ProcControlAPI process, ProcControlAPI will receive two copies of the exit
             * event. The second event should be ignored.
             */
            if( int_thread::exited == thread->getGeneratorState() ) {
                pthrd_printf("Decoded duplicate exit event of process %d/%d with code %d\n",
                        proc->getPid(), thread->getLWP(), exitcode);
                return true;
            }

            pthrd_printf("Decoded event to exit of process %d/%d with code %d\n",
                    proc->getPid(), thread->getLWP(), exitcode);
            event = Event::ptr(new EventExit(EventType::Post, exitcode));
        }else{
            int termsig = WTERMSIG(status);
            pthrd_printf("Decoded event to crash of %d/%d with signal %d\n",
                    proc->getPid(), thread->getLWP(), termsig);
            event = Event::ptr(new EventCrash(termsig));
        }
        event->setSyncType(Event::sync_process);

        int_threadPool::iterator i = proc->threadPool()->begin();
        for(; i != proc->threadPool()->end(); ++i) {
            (*i)->setGeneratorState(int_thread::exited);
        }
    }

    if( !multipleEvents ) {
        assert(event);
        assert(proc->proc());
        assert(thread->thread());

        // All signals stop a process in FreeBSD
        if( event && event->getSyncType() == Event::unset)
            event->setSyncType(Event::sync_process);

        event->setThread(thread->thread());
        event->setProcess(proc->proc());
        events.push_back(event);
    }

    delete archevent;

    return true;
}

static 
bool tkill(pid_t pid, long lwp, int sig) {
    static bool has_tkill = true;
    int result = 0;

    pthrd_printf("Sending %d to %d/%ld\n", sig, pid, lwp);

    if( has_tkill ) {
        result = syscall(SYS_thr_kill2, pid, lwp, sig);
        if( 0 != result && ENOSYS == errno ) {
            pthrd_printf("Using kill instead of tkill on this system\n");
            has_tkill = false;
        }
    }

    if( !has_tkill ) {
        result = kill(pid, sig);
    }

    return (result == 0);
}

int_process *int_process::createProcess(Dyninst::PID pid_, std::string exec) {
    std::vector<std::string> args;
    std::map<int, int> f;
    freebsd_process *newproc = new freebsd_process(pid_, exec, args, f);
    assert(newproc);

    return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(std::string exec,
        std::vector<std::string> args, std::map<int, int> f) 
{
    freebsd_process *newproc = new freebsd_process(0, exec, args, f);
    assert(newproc);
    return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *parent) {
    freebsd_process *newproc = new freebsd_process(pid_, parent);
    assert(newproc);
    return static_cast<int_process *>(newproc);
}

int_process::ThreadControlMode int_process::getThreadControlMode() {
    return int_process::HybridLWPControl;
}

int_thread *int_thread::createThreadPlat(int_process *proc, Dyninst::THR_ID thr_id,
        Dyninst::LWP lwp_id, bool initial_thrd)
{
    if( initial_thrd ) {
        lwp_id = sysctl_getInitialLWP(proc->getPid());
        if( lwp_id == -1 ) {
            perr_printf("Failed to determine initial LWP for PID %d\n",
                    proc->getPid());
            assert(!"Failed to determine initial LWP");
        }
    }
    freebsd_thread *lthrd = new freebsd_thread(proc, thr_id, lwp_id);
    assert(lthrd);
    return static_cast<int_thread *>(lthrd);
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool) {
    static bool initialized = false;
    static FreeBSDStopHandler *lstop = NULL;

#if defined(bug_freebsd_mt_suspend)
    static FreeBSDPostStopHandler *lpoststop = NULL;
    static FreeBSDBootstrapHandler *lboot = NULL;
#endif

#if defined(bug_freebsd_change_pc)
    static FreeBSDChangePCHandler *lpc = NULL;
#endif

    if( !initialized ) {
        lstop = new FreeBSDStopHandler();

#if defined(bug_freebsd_mt_suspend)
        lpoststop = new FreeBSDPostStopHandler();
        lboot = new FreeBSDBootstrapHandler();
#endif

#if defined(bug_freebsd_change_pc)
        lpc = new FreeBSDChangePCHandler();
#endif

        initialized = true;
    }
    hpool->addHandler(lstop);

#if defined(bug_freebsd_mt_suspend)
    hpool->addHandler(lpoststop);
    hpool->addHandler(lboot);
#endif

#if defined(bug_freebsd_change_pc)
    hpool->addHandler(lpc);
#endif

    thread_db_process::addThreadDBHandlers(hpool);
    return hpool;
}

bool ProcessPool::LWPIDsAreUnique() {
    return true;
}

freebsd_process::freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f) :
  int_process(p, e, a, f),
  thread_db_process(p, e, a, f),
  sysv_process(p, e, a, f),
  unix_process(p, e, a, f),
  x86_process(p, e, a, f)
{
}

freebsd_process::freebsd_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p),
  thread_db_process(pid_, p),
  sysv_process(pid_, p),
  unix_process(pid_, p),
  x86_process(pid_, p)
{
}

freebsd_process::~freebsd_process() 
{
    int eventQueue = getEventQueue();
    if( -1 != eventQueue ) {
        // Remove the event for this process
        struct kevent event;
        EV_SET(&event, pid, EVFILT_PROC, EV_DELETE,
                NOTE_EXEC, 0, NULL);

        if( -1 == kevent(eventQueue, &event, 1, NULL, 0, NULL) ) {
            perr_printf("Failed to remove kevent for process %d\n", pid);
        }else{
            pthrd_printf("Successfully removed kevent for process %d\n", pid);
        }
    }
    freeThreadDBAgent();
}

int freebsd_process::getEventQueue() {
    static volatile int eventQueue = -1;
    static Mutex event_queue_init_lock;

    if( -1 == eventQueue ) {
        event_queue_init_lock.lock();
        if( -1 == eventQueue ) {
            if( -1 == (eventQueue = kqueue()) ) {
                perr_printf("low level failure: kqueue: %s\n",
                        strerror(errno));
            }else{
                pthrd_printf("created kqueue = %d\n", eventQueue);
            }
        }
        event_queue_init_lock.unlock();
    }

    return eventQueue;
}

bool freebsd_process::initKQueueEvents() {
    int eventQueue = getEventQueue();
    if( -1 == eventQueue ) return false;

    struct kevent event;
    EV_SET(&event, pid, EVFILT_PROC, EV_ADD,
            NOTE_EXEC, 0, NULL);

    if( -1 == kevent(eventQueue, &event, 1, NULL, 0, NULL) ) {
        perr_printf("low level failure: kevent: %s\n",
                strerror(errno));
        return false;
    }
    pthrd_printf("Enabled kqueue events for process %d\n",
            pid);

    return true;
}

bool freebsd_process::post_create() {
    if( !thread_db_process::post_create() ) return false;

    return initKQueueEvents();
}

bool freebsd_process::plat_create() {
    pid = fork();
    if( -1 == pid ) {
        int errnum = errno;
        pthrd_printf("Could not fork new process for %s: %s\n",
                executable.c_str(), strerror(errnum));
        setLastError(err_internal, "Unable to fork new process");
        return false;
    }

    if( !pid ) {
        // Child
        if( 0 != ptrace(PT_TRACE_ME, 0, 0, 0) ) {
            perr_printf("Faild to execute a PT_TRACE_ME.\n");
            setLastError(err_internal, "Unable to debug trace new process");
            exit(-1);
        }

        plat_execv();

        exit(-1);
    }

    return true;
}

bool freebsd_process::plat_attach() {
    pthrd_printf("Attaching to pid %d\n", pid);
    if( 0 != ptrace(PT_ATTACH, pid, (caddr_t)1, 0) ) {
        int errnum = errno;
        pthrd_printf("Unable to attach to process %d: %s\n", pid, strerror(errnum));
        if( EPERM == errnum ) {
            setLastError(err_prem, "Do not have correct permissions to attach to pid");
        }else if( ESRCH == errnum ) {
            setLastError(err_noproc, "The specified process was not found");
        }else {
            setLastError(err_internal, "Unable to attach to the specified process");
        }
        return false;
    }
    return true;
}

bool freebsd_process::plat_forked() {
    return true;
}

bool freebsd_process::plat_execed() { 
    bool result = sysv_process::plat_execed();
    if( !result ) return false;

    char *pathname = sysctl_getExecPathname(getPid());
    if( NULL == pathname ) {
        perr_printf("Failed to retrieve executable pathname");
        setLastError(err_internal, "Failed to retrieve executable pathname");
        return false;
    }

    executable = pathname;
    free(pathname);
    return true;
}

bool freebsd_process::plat_detach(bool &needs_sync) {
    pthrd_printf("PT_DETACH on %d\n", getPid());
    needs_sync = false;
    if( 0 != ptrace(PT_DETACH, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_DETACH on %d\n", getPid());
        setLastError(err_internal, "PT_DETACH operation failed\n");
    }
    return true;
}

bool freebsd_process::plat_terminate(bool &needs_sync) {
    pthrd_printf("Terminating process %d\n", getPid());
    if( threadPool()->allStopped() ) {
        if( 0 != ptrace(PT_KILL, getPid(), (caddr_t)1, 0) ) {
            perr_printf("Failed to PT_KILL process %d\n", getPid());
            setLastError(err_internal, "PT_KILL operation failed\n");
            return false;
        }
    }else{
        if( kill(getPid(), SIGKILL) ) {
            perr_printf("Failed to send SIGKILL to process %d: %s\n", getPid(),
                    strerror(errno));
            setLastError(err_internal, "Failed to send SIGKILL");
            return false;
        }
    }

    needs_sync = true;
    return true;
}

bool freebsd_process::plat_readMem(int_thread *thr, void *local, 
                         Dyninst::Address remote, size_t size) 
{
    return PtraceBulkRead(remote, size, local, thr->llproc()->getPid());
}

bool freebsd_process::plat_writeMem(int_thread *thr, void *local, 
                          Dyninst::Address remote, size_t size) 
{
    return PtraceBulkWrite(remote, size, local, thr->llproc()->getPid());
}

bool freebsd_process::needIndividualThreadAttach() {
    return true;
}

bool freebsd_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps) {
    return sysctl_findProcLWPs(getPid(), lwps);
}

Dyninst::Architecture freebsd_process::getTargetArch() {
    if( Dyninst::Arch_none != arch ) {
        return arch;
    }
    int addr_width = sysctl_computeAddrWidth(getPid());

#if defined(arch_x86) || defined(arch_x86_64)
    assert(addr_width == 4 || addr_width == 8);
    arch = (addr_width == 4) ? Dyninst::Arch_x86 : Dyninst::Arch_x86_64;
#elif defined(arch_power)
    assert(addr_width == 4 || addr_width == 8);
    arch = (addr_width == 4) ? Dyninst::Arch_ppc32 : Dyninst::Arch_ppc64;
#else
    assert(!"Unknown architecture");
#endif
    return arch;
}

freebsd_thread::freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
    : thread_db_thread(p, t, l), bootstrapStop(false), pcBugCondition(false),
      pendingPCBugSignal(false), signalStopped(false)
{
}

freebsd_thread::~freebsd_thread() 
{
}

void freebsd_thread::setBootstrapStop(bool b) {
    bootstrapStop = b;
}

bool freebsd_thread::hasBootstrapStop() const {
    return bootstrapStop;
}

void freebsd_thread::setPCBugCondition(bool b) {
    pcBugCondition = b;
}

bool freebsd_thread::hasPCBugCondition() const {
    return pcBugCondition;
}

void freebsd_thread::setSignalStopped(bool b) {
    signalStopped = b;
}

bool freebsd_thread::isSignalStopped() const {
    return signalStopped;
}

void freebsd_thread::setPendingPCBugSignal(bool b) {
    pendingPCBugSignal = b;
}

bool freebsd_thread::hasPendingPCBugSignal() const {
    return pendingPCBugSignal;
}

bool freebsd_process::plat_getLWPInfo(lwpid_t lwp, void *lwpInfo) {
    pthrd_printf("Calling PT_LWPINFO on %d\n", lwp);
    if( 0 != ptrace(PT_LWPINFO, lwp, (caddr_t)lwpInfo, sizeof(struct ptrace_lwpinfo)) ) {
        perr_printf("Failed to get thread info for lwp %d: %s\n",
                lwp, strerror(errno));
        setLastError(err_internal, "Failed to get thread info");
        return false;
    }

    return true;
}

const char *freebsd_process::getThreadLibName(const char *symName) {
    // This hack is needed because the FreeBSD implementation doesn't
    // set the object name when looking for a symbol -- instead of
    // searching every library for the symbol, make some educated 
    // guesses
    //
    // It also assumes that the first symbols thread_db will lookup
    // are either _libthr_debug or _libkse_debug
    
    if( !strcmp(symName, "_libkse_debug") ) {
        libThreadName = "libkse.so";
    }else if( !strcmp(symName, "_libthr_debug") || 
            libThreadName.empty() ) 
    {
        libThreadName = "libthr.so";
    }

    return libThreadName.c_str();
}

bool freebsd_process::isSupportedThreadLib(string libName) {
    if( libName.find("libthr") != string::npos ) {
        return true;
    }else if( libName.find("libkse") != string::npos ) {
        return true;
    }

    return false;
}

FreeBSDStopHandler::FreeBSDStopHandler() 
    : Handler("FreeBSD Stop Handler")
{}

FreeBSDStopHandler::~FreeBSDStopHandler()
{}

Handler::handler_ret_t FreeBSDStopHandler::handleEvent(Event::ptr ev) {
    freebsd_process *lproc = dynamic_cast<freebsd_process *>(ev->getProcess()->llproc());
    freebsd_thread *lthread = static_cast<freebsd_thread *>(ev->getThread()->llthrd());

    // No extra handling is required for single-threaded debuggees
    if( lproc->threadPool()->size() <= 1 ) return Handler::ret_success;

    if( lthread->hasPendingStop() || lthread->hasBootstrapStop() ) {
        pthrd_printf("Pending stop on %d/%d\n",
                lproc->getPid(), lthread->getLWP());
        if( !lthread->plat_suspend() ) {
            perr_printf("Failed to suspend thread %d/%d\n", 
                    lproc->getPid(), lthread->getLWP());
            return Handler::ret_error;
        }

        // Since the default thread stop handler is being wrapped, set this flag
        // here so the default handler doesn't have problems
        if( lthread->hasBootstrapStop() ) {
            lthread->setPendingStop(true);
        }
    }

    return Handler::ret_success;
}

int FreeBSDStopHandler::getPriority() const {
    return PrePlatformPriority;
}

void FreeBSDStopHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Stop));
}

#if defined(bug_freebsd_mt_suspend)
FreeBSDPostStopHandler::FreeBSDPostStopHandler() 
    : Handler("FreeBSD Post Stop Handler")
{
}

FreeBSDPostStopHandler::~FreeBSDPostStopHandler()
{
}

Handler::handler_ret_t FreeBSDPostStopHandler::handleEvent(Event::ptr ev) {
    int_process *lproc = ev->getProcess()->llproc();

    freebsd_thread *lthread = static_cast<freebsd_thread *>(ev->getThread()->llthrd());
    
    if( lthread->hasBootstrapStop() ) {
        pthrd_printf("Handling bootstrap stop on %d/%d\n",
                lproc->getPid(), lthread->getLWP());
        lthread->setBootstrapStop(false);
    }

    return Handler::ret_success;
}

int FreeBSDPostStopHandler::getPriority() const {
    return PostPlatformPriority;
}

void FreeBSDPostStopHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Stop));
}

FreeBSDBootstrapHandler::FreeBSDBootstrapHandler() 
    : Handler("FreeBSD Bootstrap Handler")
{}

FreeBSDBootstrapHandler::~FreeBSDBootstrapHandler()
{}

Handler::handler_ret_t FreeBSDBootstrapHandler::handleEvent(Event::ptr ev) {
    int_process *lproc = ev->getProcess()->llproc();
    int_thread *lthread = ev->getThread()->llthrd();

    if( lproc->threadPool()->size() <= 1 ) return Handler::ret_success;

    // Issue SIGSTOPs to all threads except the one that received
    // the initial attach. This acts like a barrier and gets around
    // the mt suspend bug.
    for(int_threadPool::iterator i = lproc->threadPool()->begin(); 
        i != lproc->threadPool()->end(); ++i)
    {
        freebsd_thread *bsdThread = static_cast<freebsd_thread *>(*i);

        if( bsdThread->getLWP() != lthread->getLWP() ) {
            pthrd_printf("Issuing bootstrap stop for %d/%d\n",
                    lproc->getPid(), bsdThread->getLWP());
            bsdThread->setBootstrapStop(true);

            if( !bsdThread->plat_stop() ) {
	      return Handler::ret_error;
            }
        }
    }

    return Handler::ret_success;
}

int FreeBSDBootstrapHandler::getPriority() const {
    return PostPlatformPriority;
}

void FreeBSDBootstrapHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}
#endif

/*
 * In the bootstrap handler, SIGSTOPs where issued to all threads. This function
 * flushes out all those pending "bootstrap stops". Due to some weirdness with
 * signals, the SIGSTOP signal needs to be resent after the process has been continued.
 * I haven't been able to figure out why this needs to happen, but it solves the 
 * problem at hand.
 */
bool freebsd_process::post_attach() {
    if( !thread_db_process::post_attach() ) return false;

    if( !initKQueueEvents() ) return false;

#if defined(bug_freebsd_mt_suspend)
    if( threadPool()->size() <= 1 ) return true;

    for(int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); ++i) {
        freebsd_thread *thrd = static_cast<freebsd_thread *>(*i);
        if( thrd->hasBootstrapStop() ) {
            pthrd_printf("attach workaround: resuming %d/%d\n",
                    getPid(), thrd->getLWP());
            if( !thrd->intCont() ) {
                return false;
            }

            pthrd_printf("attach workaround: continuing whole process\n");
            if( !plat_contProcess() ) {
                return false;
            }

#if defined(bug_freebsd_lost_signal)
            pthrd_printf("attach workaround: sending stop to %d/%d\n",
                    getPid(), thrd->getLWP());
            if( !thrd->plat_stop() ) {
                return false;
            }
#endif

            pthrd_printf("attach workaround: handling stop of %d/%d\n",
                    getPid(), thrd->getLWP());
            if( !waitfor_startup() ) {
                return false;
            }
        }
    }
#endif

    return true;
}

#if defined(bug_freebsd_change_pc)
FreeBSDChangePCHandler::FreeBSDChangePCHandler() 
    : Handler("FreeBSD Change PC Handler")
{}

FreeBSDChangePCHandler::~FreeBSDChangePCHandler()
{}


Handler::handler_ret_t FreeBSDChangePCHandler::handleEvent(Event::ptr ev) {
    freebsd_thread *lthread = static_cast<freebsd_thread *>(ev->getThread()->llthrd());

    pthrd_printf("Unsetting change PC bug condition for %d/%d\n",
            ev->getProcess()->getPid(), lthread->getLWP());
    lthread->setPCBugCondition(false);
    lthread->setPendingPCBugSignal(false);

    return Handler::ret_success;
}

int FreeBSDChangePCHandler::getPriority() const {
    return PostPlatformPriority;
}

void FreeBSDChangePCHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::ChangePCStop));
}
#endif

bool freebsd_process::plat_contProcess() {
    ProcPool()->condvar()->lock();
    for(int_threadPool::iterator i = threadPool()->begin();
        i != threadPool()->end(); ++i)
    {
        if( (*i)->isResumed() ) {
            (*i)->setInternalState(int_thread::running);
            (*i)->setHandlerState(int_thread::running);
            (*i)->setGeneratorState(int_thread::running);
            (*i)->setResumed(false);
        }else if( (*i)->getInternalState() == int_thread::stopped ) {
            pthrd_printf("Suspending before continue %d/%d\n",
                    getPid(), (*i)->getLWP());
            if( !(*i)->plat_suspend() ) {
                perr_printf("Failed to suspend thread %d/%d\n",
                        getPid(), (*i)->getLWP());
                setLastError(err_internal, "low-level continue failed");
                return false;
            }
        }
    }
    ProcPool()->condvar()->signal();
    ProcPool()->condvar()->unlock();

#if defined(bug_freebsd_change_pc)
    freebsd_thread *pcBugThrd = NULL;

    for(int_threadPool::iterator i = threadPool()->begin();
        i != threadPool()->end(); ++i)
    {
        freebsd_thread *thrd = static_cast<freebsd_thread *>(*i);
        if( thrd->hasPCBugCondition() && thrd->getInternalState() == int_thread::running ) {
            // Only wait for one PC bug signal at a time
            if( !thrd->hasPendingPCBugSignal() ) {
                pcBugThrd = thrd;
            }else{
                pthrd_printf("Waiting for PC bug signal for thread %d/%d\n",
                        getPid(), thrd->getLWP());
                pcBugThrd = NULL;
                break;
            }
        }
    }

    if( NULL != pcBugThrd ) {
        pthrd_printf("Attempting to handle change PC bug condition for %d/%d\n",
                getPid(), pcBugThrd->getLWP());
        pcBugThrd->setPendingPCBugSignal(true);
        if( !tkill(getPid(), pcBugThrd->getLWP(), PC_BUG_SIGNAL) ) {
            perr_printf("Failed to handle change PC bug condition\n");
            setLastError(err_internal, "sending signal failed");
            return false;
        }
    }
#endif

    /* Single-stepping is enabled/disabled using PT_SETSTEP
     * and PT_CLEARSTEP instead of continuing the process
     * with PT_STEP. See plat_setStep.
     */
    pthrd_printf("Calling PT_CONTINUE on %d with signal %d\n", 
                getPid(), continueSig);
    if (0 != ptrace(PT_CONTINUE, getPid(), (caddr_t)1, continueSig) ) {
        perr_printf("low-level continue failed: %s\n", strerror(errno));
        setLastError(err_internal, "Low-level continue failed");
        return false;
    }

#if defined(bug_freebsd_missing_sigstop)
    // XXX this workaround doesn't always work
    sched_yield();
#endif

    return true;
}

bool freebsd_thread::plat_stop() {
    Dyninst::PID pid = llproc()->getPid();

    if( !tkill(pid, lwp, SIGUSR2) ) {
        int errnum = errno;
        if( ESRCH == errnum ) {
            pthrd_printf("tkill failed for %d/%d, thread/process doesn't exist\n", lwp, pid);
            setLastError(err_noproc, "Thread no longer exists");
            return false;
        }
        pthrd_printf("tkill failed on %d/%d: %s\n", lwp, pid, strerror(errnum));
        setLastError(err_internal, "Could not send signal to process");
        return false;
    }

    return true;
}

bool freebsd_thread::plat_cont() {
    pthrd_printf("Continuing thread %d/%d\n", 
            llproc()->getPid(), lwp);

    if( !plat_setStep() ) return false;

    setSignalStopped(false);

    // Calling resume only makes sense for processes with multiple threads
    if( llproc()->threadPool()->size() > 1 ) {
        if( !plat_resume() ) return false;
    }

    // Because all signals stop the whole process, only one thread should
    // have a non-zero continue signal
    if( continueSig_ ) {
        llproc()->setContSignal(continueSig_);
    }

    return true;
}

bool freebsd_thread::attach() {
    return true;
}

bool freebsd_thread::plat_setStep() {
    int result;
    if( singleStep() ) {
        pthrd_printf("Calling PT_SETSTEP on %d/%d\n", 
                llproc()->getPid(), lwp);
        result = ptrace(PT_SETSTEP, lwp, (caddr_t)1, 0);
    }else{
        pthrd_printf("Calling PT_CLEARSTEP on %d/%d\n", 
                llproc()->getPid(), lwp);
        result = ptrace(PT_CLEARSTEP, lwp, (caddr_t)1, 0);
    }

    if( 0 != result ) {
        perr_printf("low-level single step change failed: %s\n", strerror(errno));
        setLastError(err_internal, "Low-level single step change failed");
        return false;
    }

    return true;
}

static dynreg_to_user_t dynreg_to_user;
static void init_dynreg_to_user() {
    static volatile bool initialized = false;
    static Mutex init_lock;
    if( initialized ) return;

    init_lock.lock();
    if( initialized ) {
        init_lock.unlock();
        return;
    }

#if defined(arch_x86_64)
    dynreg_to_user[x86_64::r15] =   make_pair(offsetof(reg, r_r15), 8);
    dynreg_to_user[x86_64::r14] =   make_pair(offsetof(reg, r_r14), 8);
    dynreg_to_user[x86_64::r13] =   make_pair(offsetof(reg, r_r13), 8);
    dynreg_to_user[x86_64::r12] =   make_pair(offsetof(reg, r_r12), 8);
    dynreg_to_user[x86_64::r11] =   make_pair(offsetof(reg, r_r11), 8);
    dynreg_to_user[x86_64::r10] =   make_pair(offsetof(reg, r_r10), 8);
    dynreg_to_user[x86_64::r9] =    make_pair(offsetof(reg, r_r9), 8);
    dynreg_to_user[x86_64::r8] =    make_pair(offsetof(reg, r_r8), 8);
    dynreg_to_user[x86_64::rdi] =   make_pair(offsetof(reg, r_rdi), 8);
    dynreg_to_user[x86_64::rsi] =   make_pair(offsetof(reg, r_rsi), 8);
    dynreg_to_user[x86_64::rbp] =   make_pair(offsetof(reg, r_rbp), 8);
    dynreg_to_user[x86_64::rbx] =   make_pair(offsetof(reg, r_rbx), 8);
    dynreg_to_user[x86_64::rdx] =   make_pair(offsetof(reg, r_rdx), 8);
    dynreg_to_user[x86_64::rcx] =   make_pair(offsetof(reg, r_rcx), 8);
    dynreg_to_user[x86_64::rax] =   make_pair(offsetof(reg, r_rax), 8);
    dynreg_to_user[x86_64::rip] =   make_pair(offsetof(reg, r_rip), 8);
    dynreg_to_user[x86_64::rsp] =   make_pair(offsetof(reg, r_rsp), 8);
    dynreg_to_user[x86_64::flags] = make_pair(offsetof(reg, r_rflags), 8);
    dynreg_to_user[x86_64::cs] =    make_pair(offsetof(reg, r_cs), 8);
    dynreg_to_user[x86_64::ss] =    make_pair(offsetof(reg, r_ss), 8);

    // x86 registers -- it appears that not all the segment registers are available
    dynreg_to_user[x86::edi] = make_pair(offsetof(reg, r_rdi), 4);
    dynreg_to_user[x86::esi] = make_pair(offsetof(reg, r_rsi), 4);
    dynreg_to_user[x86::ebp] = make_pair(offsetof(reg, r_rbp), 4);
    dynreg_to_user[x86::ebx] = make_pair(offsetof(reg, r_rbx), 4);
    dynreg_to_user[x86::edx] = make_pair(offsetof(reg, r_rdx), 4);
    dynreg_to_user[x86::ecx] = make_pair(offsetof(reg, r_rcx), 4);
    dynreg_to_user[x86::eax] = make_pair(offsetof(reg, r_rax), 4);
    dynreg_to_user[x86::eip] = make_pair(offsetof(reg, r_rip), 4);
    dynreg_to_user[x86::flags] = make_pair(offsetof(reg, r_rflags), 4);
    dynreg_to_user[x86::esp] = make_pair(offsetof(reg, r_rsp), 4);
    dynreg_to_user[x86::ss] = make_pair(offsetof(reg, r_ss), 4);
    dynreg_to_user[x86::cs] = make_pair(offsetof(reg, r_cs), 4);
    
#elif defined(arch_x86)
    dynreg_to_user[x86::fs] = make_pair(offsetof(reg, r_fs), 4);
    dynreg_to_user[x86::es] = make_pair(offsetof(reg, r_es), 4);
    dynreg_to_user[x86::ds] = make_pair(offsetof(reg, r_ds), 4);
    dynreg_to_user[x86::edi] = make_pair(offsetof(reg, r_edi), 4);
    dynreg_to_user[x86::esi] = make_pair(offsetof(reg, r_esi), 4);
    dynreg_to_user[x86::ebp] = make_pair(offsetof(reg, r_ebp), 4);
    dynreg_to_user[x86::ebx] = make_pair(offsetof(reg, r_ebx), 4);
    dynreg_to_user[x86::edx] = make_pair(offsetof(reg, r_edx), 4);
    dynreg_to_user[x86::ecx] = make_pair(offsetof(reg, r_ecx), 4);
    dynreg_to_user[x86::eax] = make_pair(offsetof(reg, r_eax), 4);
    dynreg_to_user[x86::eip] = make_pair(offsetof(reg, r_eip), 4);
    dynreg_to_user[x86::cs] = make_pair(offsetof(reg, r_cs), 4);
    dynreg_to_user[x86::flags] = make_pair(offsetof(reg, r_eflags), 4);
    dynreg_to_user[x86::esp] = make_pair(offsetof(reg, r_esp), 4);
    dynreg_to_user[x86::ss] = make_pair(offsetof(reg, r_ss), 4);
    dynreg_to_user[x86::gs] = make_pair(offsetof(reg, r_gs), 4);
#else
    // TODO implement this for other architectures
    assert(!"Register conversion is not implemented for this architecture");
#endif

    initialized = true;

    init_lock.unlock();
}

#if 0
// Debugging
static void dumpRegisters(struct reg *regs) {
#if defined(arch_x86)
    fprintf(stderr, "r_fs = 0x%x\n", regs->r_fs);
    fprintf(stderr, "r_es = 0x%x\n", regs->r_es);
    fprintf(stderr, "r_ds = 0x%x\n", regs->r_ds);
    fprintf(stderr, "r_edi = 0x%x\n", regs->r_edi);
    fprintf(stderr, "r_esi = 0x%x\n", regs->r_esi);
    fprintf(stderr, "r_ebp = 0x%x\n", regs->r_ebp);
    fprintf(stderr, "r_ebx = 0x%x\n", regs->r_ebx);
    fprintf(stderr, "r_ecx = 0x%x\n", regs->r_ecx);
    fprintf(stderr, "r_eax = 0x%x\n", regs->r_eax);
    fprintf(stderr, "r_eip = 0x%x\n", regs->r_eip);
    fprintf(stderr, "r_cs = 0x%x\n", regs->r_cs);
    fprintf(stderr, "r_eflags = 0x%x\n", regs->r_eflags);
    fprintf(stderr, "r_esp = 0x%x\n", regs->r_esp);
    fprintf(stderr, "r_ss = 0x%x\n", regs->r_ss);
    fprintf(stderr, "r_gs = 0x%x\n", regs->r_gs);
#endif
}
#endif

bool freebsd_process::plat_individualRegAccess() 
{
    return false;
}

SymbolReaderFactory *freebsd_process::plat_defaultSymReader()
{
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
  return symreader_factory;
}

bool freebsd_thread::plat_getAllRegisters(int_registerPool &regpool) {
    struct reg registers;
    unsigned char *regPtr = (unsigned char *)&registers;

    if( 0 != ptrace(PT_GETREGS, lwp, (caddr_t)regPtr, 0) ) {
        perr_printf("Error reading registers from %d/%d: %s\n", 
                llproc()->getPid(), lwp, strerror(errno));
        setLastError(err_internal, "Could not read registers from thread");
        return false;
    }

    init_dynreg_to_user();

    Dyninst::Architecture curplat = llproc()->getTargetArch();
    regpool.regs.clear();

    dynreg_to_user_t::iterator i;
    for(i = dynreg_to_user.begin(); i != dynreg_to_user.end(); ++i ) {
        const MachRegister reg = i->first;
        if (reg.getArchitecture() != curplat ) continue;

        MachRegisterVal val = 0;
        const unsigned int offset = i->second.first;
        const unsigned int size = i->second.second;

        if( size == sizeof(uint32_t) ) {
            val = *((uint32_t *)(&regPtr[offset]));
        }else if( size == sizeof(uint64_t) ) {
            val = *((uint64_t *)(&regPtr[offset]));
        }else{
            assert(!"Unknown address width");
        }
        pthrd_printf("Register %s has value 0x%lx, offset 0x%x\n", reg.name(), (unsigned long)val, offset);
        regpool.regs[reg] = val;
    }

    return true;
}

#if defined(arch_x86)
static bool validateRegisters(struct reg *regs, Dyninst::LWP lwp) {
    struct reg old_regs;
    if( 0 != ptrace(PT_GETREGS, lwp, (caddr_t)&old_regs, 0) ) {
        perr_printf("Error reading registers from %d\n", lwp);
        return false;
    }

    // Sometimes the resume flag is set in the saved version of the
    // registers and not set in the current set of registers -- 
    // the OS doesn't allow us to change this flag, change it to the
    // current value
    if( (old_regs.r_eflags & PSL_RF) != (regs->r_eflags & PSL_RF) ) {
        if( old_regs.r_eflags & PSL_RF ) regs->r_eflags |= PSL_RF;
        else regs->r_eflags &= ~PSL_RF;
    }
    
    return true;
}
#else
static bool validateRegisters(struct reg *, Dyninst::LWP) {
    return false;
}
#endif

bool freebsd_thread::plat_setAllRegisters(int_registerPool &regpool) {
    init_dynreg_to_user();

    // Populate a struct reg using the registerPool
    struct reg registers;
    unsigned char *regPtr = (unsigned char *)&registers;

    dynreg_to_user_t::iterator i;
    unsigned num_found = 0;
    Dyninst::Architecture curplat = llproc()->getTargetArch();

    for (i = dynreg_to_user.begin(); i != dynreg_to_user.end(); ++i) {
        const MachRegister reg = i->first;
        MachRegisterVal val;

        if (reg.getArchitecture() != curplat) continue;

        const unsigned int offset = i->second.first;
        const unsigned int size = i->second.second;

        assert(offset+size <= sizeof(struct reg));

        int_registerPool::reg_map_t::iterator j = regpool.regs.find(reg);

        // A register was not set in the registerPool, error report after loop
        if( j == regpool.regs.end() ) break;

        num_found++;
        val = j->second;

        if (size == sizeof(uint32_t)) {
            *((uint32_t *)&regPtr[offset]) = (uint32_t) val;
        } else if (size == sizeof(uint64_t)) {
            *((uint64_t *)&regPtr[offset]) = (uint64_t) val;
        } else {
            assert(!"Unknown address width");
        }

        pthrd_printf("Register %s gets value 0x%lx, offset 0x%x\n", reg.name(), (unsigned long)val, offset);
    }

    if (num_found != regpool.regs.size()) {
        setLastError(err_badparam, "Invalid register set passed to setAllRegisters");
        perr_printf("Couldn't find all registers in the register set %u/%u\n", num_found,
                    (unsigned int) regpool.regs.size());
        return false;
    }

    if( 0 != ptrace(PT_SETREGS, lwp, (caddr_t)&registers, 0) ) {
        bool success = false;
        if( EINVAL == errno ) {
            // This usually means that the flag register would change some system status bits
            pthrd_printf("Attempting to handle EINVAL caused by PT_SETREGS for %d/%d\n", 
                    llproc()->getPid(), lwp);

            if( validateRegisters(&registers, lwp) ) {
                if( !ptrace(PT_SETREGS, lwp, (caddr_t)&registers, 0) ) {
                    pthrd_printf("Successfully handled EINVAL caused by PT_SETREGS\n");
                    success = true;
                }else{
                    perr_printf("Failed to handle EINVAL caused by PT_SETREGS\n");
                }
            }
        }

        if( !success ) {
            perr_printf("Error setting registers for LWP %d: %s\n", lwp, strerror(errno));
            setLastError(err_internal, "Could not set registers in thread");
            return false;
        }
    }

#if defined(bug_freebsd_change_pc)
    if( !isSignalStopped() ) {
        pthrd_printf("Setting change PC bug condition for %d/%d\n",
                llproc()->getPid(), lwp);
        setPCBugCondition(true);
    }
#endif

    pthrd_printf("Successfully set the values of all registers for LWP %d\n", lwp);

    return true;
}

bool freebsd_thread::plat_getRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal &) {
    assert(!"This platform does not have individual register access");
    return false;
}

bool freebsd_thread::plat_setRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal) {
    assert(!"This platform does not have individual register access");
    return false;
}

// iRPC snippets
const unsigned int x86_64_mmap_flags_position = 21;
const unsigned int x86_64_mmap_size_position = 34;
const unsigned int x86_64_mmap_addr_position = 44;
const unsigned int x86_64_mmap_start_position = 4;
const unsigned char x86_64_call_mmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop sled
0x49, 0xc7, 0xc1, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0,%r9 (offset)
0x49, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff,       //mov    $0xffffffffffffffff,%r8 (fd)
0x49, 0xc7, 0xc2, 0x12, 0x10, 0x00, 0x00,       //mov    $0x1012,%r10 (flags)
0x48, 0xc7, 0xc2, 0x07, 0x00, 0x00, 0x00,       //mov    $0x7,%rdx (perms)
0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rsi (size)
0x00, 0x00, 0x00,                               //
0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rdi (addr)
0x00, 0x00, 0x00,                               //
0xb8, 0xdd, 0x01, 0x00, 0x00,                   //mov    $0x1dd,%eax (SYS_mmap)
0x0f, 0x05,                                     //syscall
0xcc,                                           //trap
0x90                                            //nop
};
const unsigned int x86_64_call_mmap_size = sizeof(x86_64_call_mmap);

const unsigned int x86_64_munmap_size_position = 6;
const unsigned int x86_64_munmap_addr_position = 16;
const unsigned int x86_64_munmap_start_position = 4;
const unsigned char x86_64_call_munmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop sled
0x48, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rsi
0x00, 0x00, 0x00,                               //
0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00,       //mov    $0x0000000000000000,%rdi
0x00, 0x00, 0x00,                               //
0xb8, 0x49, 0x00, 0x00, 0x00,                   //mov    $0x49,%eax
0x0f, 0x05,                                     //syscall
0xcc,                                           //trap
0x90                                            //nop
};
const unsigned int x86_64_call_munmap_size = sizeof(x86_64_call_munmap);

const unsigned int x86_mmap_flags_position = 9;
const unsigned int x86_mmap_size_position = 16;
const unsigned int x86_mmap_addr_position = 21;
const unsigned int x86_mmap_start_position = 4;
const unsigned char x86_call_mmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop sled
0x6a, 0x00,                                     //push   $0x0 (offset)
0x6a, 0xff,                                     //push   $0xffffffff (fd)
0x68, 0x12, 0x10, 0x00, 0x00,                   //push   $0x1012 (flags)
0x6a, 0x07,                                     //push   $0x7 (perms)
0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (size)
0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (addr)
0xb8, 0xdd, 0x01, 0x00, 0x00,                   //mov    $0x1dd,%eax (SYS_mmap)
0x50,                                           //push   %eax (required by calling convention)
0xcd, 0x80,                                     //int    $0x80
0x8d, 0x64, 0x24, 0x1c,                         //lea    0x1c(%esp),%esp
0xcc,                                           //trap
0x90                                            //nop
};
const unsigned int x86_call_mmap_size = sizeof(x86_call_mmap);

const unsigned int x86_munmap_size_position = 5;
const unsigned int x86_munmap_addr_position = 10;
const unsigned int x86_munmap_start_position = 4;
const unsigned char x86_call_munmap[] = {
0x90, 0x90, 0x90, 0x90,                         //nop sled
0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (size)    
0x68, 0x00, 0x00, 0x00, 0x00,                   //push   $0x0 (addr)
0xb8, 0x49, 0x00, 0x00, 0x00,                   //mov    $0x49,%eax (SYS_munmap)
0x50,                                           //push   %eax (required by calling convention)
0xcd, 0x80,                                     //int    $0x80
0x8d, 0x64, 0x24, 0x0c,                         //lea    0xc(%esp),%esp 
0xcc,                                           //trap
0x90                                            //nop
};
const unsigned int x86_call_munmap_size = sizeof(x86_call_munmap);
