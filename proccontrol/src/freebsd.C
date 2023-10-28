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

#include "Architecture.h"
#include "common/h/dyntypes.h"
#include "PCErrors.h"
#include "Generator.h"
#include "Event.h"
#include "Handler.h"
#include "Mailbox.h"
#include "procpool.h"
#include "irpc.h"
#include "snippets.h"
#include "freebsd.h"
#include "int_handler.h"
#include "int_event.h"
#include "common/src/freebsdKludges.h"

#if defined(WITH_SYMLITE)
#include "symlite/h/SymLite-elf.h"
#elif defined(WITH_SYMTAB_API)
#include "symtabAPI/h/SymtabReader.h"
#else
#error "No defined symbol reader"
#endif

#include <iostream>

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
using namespace std;
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
 *
 * --- bug_freebsd_attach_stop ---
 *
 * When attaching to a stopped process on FreeBSD, the stopped process is resumed on
 * attach. This isn't the expected behavior. The best we can do is send a signal to
 * the debuggee after attach to allow us to regain control of the debuggee.
 */

static GeneratorFreeBSD *gen = NULL;

Generator *Generator::getDefaultGenerator() {
    if( NULL == gen ) {
        gen = new GeneratorFreeBSD();
        assert(gen);
        gen->launch();
    }
    return gen;
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

    if (dyninst_debug_proccontrol)
    {
      pthrd_printf("Waitpid return status %x for pid %d/%d:\n", status, pid, lwp);
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
    lthread = dynamic_cast<freebsd_thread *>(thread);

    Event::ptr event;

    pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
            thread ? thread->getLWP() : -1);

        for(int_threadPool::iterator i = proc->threadPool()->begin();
            i != proc->threadPool()->end(); ++i)
        {
	  int_thread *thread = *i;
	  reg_response::ptr regvalue = reg_response::createRegResponse();
	  thread->getRegister(MachRegister::getPC(proc->getTargetArch()), regvalue);
	  regvalue->isReady();
	  regvalue->getResult();
        }


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
                dynamic_cast<freebsd_thread *>(*i)->setSignalStopped(false);
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

                if( lproc->isForking() ) {
                    event = Event::ptr(new EventFork(EventType::Post, proc->getPid()));

                    // Need to maintain consistent behavior across platforms
                    freebsd_process *parent = lproc->getParent();
                    assert(parent);
                    event->setProcess(parent->proc());
                    event->setThread(parent->threadPool()->initialThread()->thread());
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
                sw_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);
                if( ibp && ibp != thread->isClearingBreakpoint() ) {
                    pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(),
                            thread->getLWP(), adjusted_addr);
                    int_eventBreakpoint *new_int_bp = new int_eventBreakpoint(adjusted_addr, ibp, thread);
                    EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(new_int_bp));
                    event = event_bp;
                    event->setThread(thread->thread());

                    if( adjusted_addr == lproc->getLibBreakpointAddr() ) {
                        pthrd_printf("Breakpoint is library load/unload\n");
                        EventLibrary::ptr lib_event = EventLibrary::ptr(new EventLibrary());
                        lib_event->setThread(thread->thread());
                        lib_event->setProcess(proc->proc());
                        lib_event->setSyncType(Event::sync_process);
                        event->addSubservientEvent(lib_event);
                        break;
                    }

                    if (lproc->decodeTdbBreakpoint(event_bp)) {
                        pthrd_printf("Breakpoint was thread event\n");
                        break;
                    }
                    break;
                }

                if( thread->singleStep() ) {
		  bp_instance *tbp = thread->isClearingBreakpoint();
		  if (tbp) {
		    ibp = tbp->swBP();
		  }
		  if( ibp ) {
		    pthrd_printf("Decoded event to breakpoint cleanup\n");
		    event = EventBreakpointRestore::ptr(new EventBreakpointRestore(new int_eventBreakpointRestore(ibp)));
		    if (thread->singleStepUserMode()) {
		      Event::ptr subservient_ss = EventSingleStep::ptr(new EventSingleStep());
		      subservient_ss->setProcess(proc->proc());
		      subservient_ss->setThread(thread->thread());
		      subservient_ss->setSyncType(Event::sync_process);
		      event->addSubservientEvent(subservient_ss);
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

                // If we are re-attaching to a process that we created that has been receiving signals,
                // we can get one of these signals before the bootstrap stop -- ignore this event for
                // now
                if( lproc->getState() == int_process::neonatal_intermediate ) {
                    pthrd_printf("Received signal %d before attach stop\n", stopsig);
                    if( !lproc->threadPool()->initialThread()->plat_cont()) {
                        perr_printf("Failed to continue process to flush out attach stop\n");
                    }
                    return true;
                }

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
            if( int_thread::exited == thread->getGeneratorState().getState() ) {
                pthrd_printf("Decoded duplicate exit event of process %d/%d with code %d\n",
                        proc->getPid(), thread->getLWP(), exitcode);
                return true;
            }

            pthrd_printf("Decoded event to exit of process %d/%d with code %d\n",
                    proc->getPid(), thread->getLWP(), exitcode);
            event = Event::ptr(new EventExit(EventType::Post, exitcode));
        }else{
            int termsig = WTERMSIG(status);
            if( int_thread::exited == thread->getGeneratorState().getState() ) {
                pthrd_printf("Decoded duplicate terminate event of process %d/%d with signal %d\n",
                        proc->getPid(), thread->getLWP(), termsig);
                return true;
            }

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
        for(; i != proc->threadPool()->end(); ++i) {
           (*i)->getGeneratorState().setState(int_thread::exited);
        }
    }

    if( !multipleEvents ) {
        assert(event);
        assert(proc->proc());
        assert(thread->thread());

        // All signals stop a process in FreeBSD
        if( event && event->getSyncType() == Event::unset)
            event->setSyncType(Event::sync_process);

        if( event->getThread() == Thread::ptr() ) {
            event->setThread(thread->thread());
        }

        if( event->getProcess() == Process::ptr() ) {
            event->setProcess(proc->proc());
        }
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
    std::vector<std::string> envp;
    freebsd_process *newproc = new freebsd_process(pid_, exec, args, envp, f);
    assert(newproc);
    newproc->plat_debuggerSuspended();
    return newproc;
}

int_process *int_process::createProcess(std::string exec,
        std::vector<std::string> args, std::vector<std::string> envp,
        std::map<int, int> f) 
{
    freebsd_process *newproc = new freebsd_process(0, exec, args, envp, f);
    assert(newproc);
    return newproc;
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *parent) {
    freebsd_process *newproc = new freebsd_process(pid_, parent);
    newproc->plat_debuggerSuspended();
    assert(newproc);
    return newproc;
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
    return lthrd;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool) {
    static bool initialized = false;
    static FreeBSDPollLWPDeathHandler *lpolldeath = NULL;
    static FreeBSDPreForkHandler *luserfork = NULL;
    static FreeBSDPostThreadDeathBreakpointHandler *ldeathbp = NULL;

#if defined(bug_freebsd_mt_suspend)
    static FreeBSDPreStopHandler *lprestop = NULL;
    static FreeBSDBootstrapHandler *lboot = NULL;
#endif

#if defined(bug_freebsd_change_pc)
    static FreeBSDChangePCHandler *lpc = NULL;
#endif

    if( !initialized ) {
        lpolldeath = new FreeBSDPollLWPDeathHandler();
        luserfork = new FreeBSDPreForkHandler();
	ldeathbp = new FreeBSDPostThreadDeathBreakpointHandler();

#if defined(bug_freebsd_mt_suspend)
        lprestop = new FreeBSDPreStopHandler();
        lboot = new FreeBSDBootstrapHandler();
#endif

#if defined(bug_freebsd_change_pc)
        lpc = new FreeBSDChangePCHandler();
#endif

        initialized = true;
    }
    hpool->addHandler(lpolldeath);
    hpool->addHandler(luserfork);
    hpool->addHandler(ldeathbp);

#if defined(bug_freebsd_mt_suspend)
    hpool->addHandler(lprestop);
    hpool->addHandler(lboot);
#endif

#if defined(bug_freebsd_change_pc)
    hpool->addHandler(lpc);
#endif

    thread_db_process::addThreadDBHandlers(hpool);
    sysv_process::addSysVHandlers(hpool);
    return hpool;
}

bool ProcessPool::LWPIDsAreUnique() {
    return true;
}

freebsd_process::freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, 
        std::map<int, int> f) :
  int_process(p, e, a, envp, f),
  sysv_process(p, e, a, envp, f),
  unix_process(p, e, a, envp, f),
  x86_process(p, e, a, envp, f),
  thread_db_process(p, e, a, envp, f),
  mmap_alloc_process(p, e, a, envp, f),
  hybrid_lwp_control_process(p, e, a, envp, f),
  forking(false),
  parent(NULL)
{
}

freebsd_process::freebsd_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p),
  sysv_process(pid_, p),
  unix_process(pid_, p),
  x86_process(pid_, p),
  thread_db_process(pid_, p),
  mmap_alloc_process(pid_, p),
  hybrid_lwp_control_process(pid_, p),
  forking(false),
  parent(dynamic_cast<freebsd_process *>(p))
{
}

freebsd_process::~freebsd_process() 
{
    int eventQueue = freebsd_process::getEventQueue();
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

async_ret_t freebsd_process::post_create(std::set<response::ptr> &async_responses)
{
   if (thread_db_process::post_create(async_responses) != aret_success) 
      return aret_error;
   if (!initKQueueEvents()) 
      return aret_error;
   return aret_success;
}

bool freebsd_process::post_forked() {
    if( !unix_process::post_forked() ) return false;

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

bool freebsd_process::plat_getOSRunningStates(map<Dyninst::LWP, bool> &runningStates) {
    if( !sysctl_getRunningStates(pid, runningStates) ) {
        pthrd_printf("Unable to retrieve process information via sysctl for pid %d\n",
                pid);
        setLastError(err_noproc, "Unable to retrieve process information via sysctl\n");
        return false;
    }
    return true;
}

OSType freebsd_process::getOS() const
{
   return Dyninst::FreeBSD;
}

bool freebsd_process::plat_attach(bool allStopped, bool &) {
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

#if defined(bug_freebsd_attach_stop)
    if(allStopped) {
        if( !tkill(pid, threadPool()->initialThread()->getLWP(), SIGUSR2) ) {
            perr_printf("Failed to send signal to process %d after attach\n",
                    pid);
            return false;
        }
    }
#endif

    return true;
}

bool freebsd_process::plat_forked() {
    return true;
}

bool freebsd_process::forked() {
    setForking(false);

    ProcPool()->condvar()->lock();

    if( !attachThreads() ) {
        pthrd_printf("Failed to attach to threads in %d\n", pid);
        setLastError(err_internal, "Could not attach to process' threads");
        return false;
    }

    ProcPool()->condvar()->broadcast();
    ProcPool()->condvar()->unlock();

    if( !post_forked() ) {
        pthrd_printf("Post-fork failed on %d\n", pid);
        setLastError(err_internal, "Error handling forked process");
        return false;
    }

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

bool freebsd_process::plat_detach(result_response::ptr, bool) {
    pthrd_printf("PT_DETACH on %d\n", getPid());
    if( 0 != ptrace(PT_DETACH, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_DETACH on %d\n", getPid());
        setLastError(err_internal, "PT_DETACH operation failed\n");
    }
    return true;
}

bool freebsd_process::plat_terminate(bool &needs_sync) {
    pthrd_printf("Terminating process %d\n", getPid());
    if (threadPool()->allHandlerStopped()) {
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

bool freebsd_process::plat_writeMem(int_thread *thr, const void *local, 
                                    Dyninst::Address remote, size_t size, bp_write_t) 
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
  : int_thread(p, t, l), 
    thread_db_thread(p, t, l), bootstrapStop(false), pcBugCondition(false),
      pendingPCBugSignal(false), signalStopped(false), is_pt_setstep(false)
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

bool freebsd_thread::isAlive() {
  if (getUserState().getState() != exited) return true;
  vector<Dyninst::LWP> lwps;
  freebsd_process *lproc = dynamic_cast<freebsd_process *>(llproc());
  lproc->getThreadLWPs(lwps);
  if (std::find(lwps.begin(), lwps.end(), getLWP()) != lwps.end()) return false;
  return true;
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

bool freebsd_process::isForking() const {
    return forking;
}

void freebsd_process::setForking(bool b) {
    forking = b;
}

freebsd_process *freebsd_process::getParent() {
    return parent;
}

bool freebsd_process::plat_suspendThread(int_thread *thr)
{
   return dynamic_cast<freebsd_thread *>(thr)->plat_suspend();
}

bool freebsd_process::plat_resumeThread(int_thread *thr)
{
   return dynamic_cast<freebsd_thread *>(thr)->plat_resume();
}



FreeBSDPollLWPDeathHandler::FreeBSDPollLWPDeathHandler() 
    : Handler("FreeBSD Poll LWP Death")
{}

FreeBSDPollLWPDeathHandler::~FreeBSDPollLWPDeathHandler()
{}

Handler::handler_ret_t FreeBSDPollLWPDeathHandler::handleEvent(Event::ptr ev) {
    freebsd_process *lproc = dynamic_cast<freebsd_process *>(ev->getProcess()->llproc());

    int_threadPool *tp = lproc->threadPool();
    int_threadPool::iterator i;

    vector<Dyninst::LWP> lwps;
    vector<Dyninst::LWP>::iterator j;
    bool have_lwps = false;

    std::set<int_thread *> to_clean;

    for (i = tp->begin(); i != tp->end(); i++) {
       int_thread *thr = *i;
       if (thr->getUserState().getState() != int_thread::exited) {
          continue;
       }
       if (!have_lwps) {
          lproc->getThreadLWPs(lwps);
          have_lwps = true;
       }

       pthrd_printf("%d/%d is marked dead.  Checking if really dead.\n", lproc->getPid(), thr->getLWP());
       bool found_match = false;
       for (j = lwps.begin(); j != lwps.end(); j++) {
          if (*j == thr->getLWP()) {
             found_match = true;
             break;
          }
       }
       if (found_match) {
          pthrd_printf("%d/%d is actually still alive, leaving\n", lproc->getPid(), thr->getLWP());
          continue;
       }
       pthrd_printf("%d/%d is finally dead.  Reaping.\n", lproc->getPid(), thr->getLWP());
       to_clean.insert(thr);
    }

    for (set<int_thread *>::iterator k = to_clean.begin(); k != to_clean.end(); k++) {
       int_thread::cleanFromHandler(*k, true);
    }

    return Handler::ret_success;
}

int FreeBSDPollLWPDeathHandler::getPriority() const {
    return PrePlatformPriority;
}

void FreeBSDPollLWPDeathHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Stop));
    etypes.push_back(EventType(EventType::None, EventType::RPC));
    etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
    etypes.push_back(EventType(EventType::None, EventType::SingleStep));
    etypes.push_back(EventType(EventType::None, EventType::Signal));
}

FreeBSDPostThreadDeathBreakpointHandler::FreeBSDPostThreadDeathBreakpointHandler() 
  : Handler("FreeBSD Post-User-Thread-Death-BP-Restore Handler") {
}

FreeBSDPostThreadDeathBreakpointHandler::~FreeBSDPostThreadDeathBreakpointHandler() {
}

Handler::handler_ret_t FreeBSDPostThreadDeathBreakpointHandler::handleEvent(Event::ptr ev) {
  // We have cleared the BP and can mark the thread as dead
  freebsd_thread *ft = dynamic_cast<freebsd_thread *>(ev->getThread()->llthrd());
  if (!ft->isExiting()) return Handler::ret_success;
  ft->getUserState().setState(int_thread::exited);
  // We might be trying to process a stop on this thread; if so, copy it to a different,
  // non-exited thread.
  
  if (ft->getPendingStopState().getState() == int_thread::running) {
    freebsd_process *lproc = dynamic_cast<freebsd_process *>(ev->getProcess()->llproc());
    
    int_threadPool *tp = lproc->threadPool();
    int_threadPool::iterator i;

    for (i = tp->begin(); i != tp->end(); i++) {
       int_thread *thr = *i;
       if (thr->getUserState().getState() == int_thread::exited) {
          continue;
       }
       if (thr->hasPendingStop()) continue;

       thr->setPendingStop(true);
       break;
    }
  }

  return Handler::ret_success;
}

int FreeBSDPostThreadDeathBreakpointHandler::getPriority() const {
  return PrePlatformPriority;
}

void FreeBSDPostThreadDeathBreakpointHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
  etypes.push_back(EventType(EventType::None, EventType::BreakpointRestore));
}


#if defined(bug_freebsd_mt_suspend)
FreeBSDPreStopHandler::FreeBSDPreStopHandler() 
    : Handler("FreeBSD Post Stop Handler")
{
}

FreeBSDPreStopHandler::~FreeBSDPreStopHandler()
{
}

Handler::handler_ret_t FreeBSDPreStopHandler::handleEvent(Event::ptr ev) {
    int_process *lproc = ev->getProcess()->llproc();

    freebsd_thread *lthread = dynamic_cast<freebsd_thread *>(ev->getThread()->llthrd());
    
    if( lthread->hasBootstrapStop() ) {
        pthrd_printf("Handling bootstrap stop on %d/%d\n",
                lproc->getPid(), lthread->getLWP());
        lthread->setBootstrapStop(false);
        // Since the default thread stop handler is being wrapped, set this flag
        // here so the default handler doesn't have problems
        lthread->setPendingStop(true);
        lthread->getStartupState().setState(int_thread::stopped);
    }

    return Handler::ret_success;
}

int FreeBSDPreStopHandler::getPriority() const {
    return PrePlatformPriority+1;
}

void FreeBSDPreStopHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
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
        freebsd_thread *bsdThread = dynamic_cast<freebsd_thread *>(*i);

        if (bsdThread->getLWP() == lthread->getLWP())
           continue;
        if (bsdThread->getDetachState().getState() == int_thread::detached)
           continue;
        pthrd_printf("Issuing bootstrap stop for %d/%d\n",
                     lproc->getPid(), bsdThread->getLWP());
        bsdThread->setBootstrapStop(true);
        
        if( !bsdThread->plat_stop() ) {
           return Handler::ret_error;
        }
    }

    return Handler::ret_success;
}

int FreeBSDBootstrapHandler::getPriority() const {
    return PrePlatformPriority;
}

void FreeBSDBootstrapHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}
#endif

FreeBSDPreForkHandler::FreeBSDPreForkHandler() 
    : Handler("FreeBSD Pre-Fork Handler")
{}

FreeBSDPreForkHandler::~FreeBSDPreForkHandler()
{}

Handler::handler_ret_t FreeBSDPreForkHandler::handleEvent(Event::ptr ev) {
    EventFork::ptr evFork = ev->getEventFork();
    int_process *parent = evFork->getProcess()->llproc();

    // Need to create and partially bootstrap the new process
    // -- the rest of the bootstrap needs to occur after the attach
    int_process *child_proc = int_process::createProcess(evFork->getPID(), parent);
    assert(child_proc);

    ProcPool()->condvar()->lock();

    int_thread *initial_thread;
    initial_thread = int_thread::createThread(child_proc, NULL_THR_ID, NULL_LWP, true,
                                              int_thread::as_created_attached);

    ProcPool()->addProcess(child_proc);

    ProcPool()->condvar()->broadcast();
    ProcPool()->condvar()->unlock();

    // Need to attach to the newly created process
    map<Dyninst::LWP, bool> runningStates;
    if( !child_proc->plat_getOSRunningStates(runningStates) ) {
        perr_printf("Failed to determine running state of child process %d\n",
                evFork->getPID());
        return Handler::ret_error;
    }

    bool allStopped = true;
    for(map<Dyninst::LWP, bool>::iterator i = runningStates.begin();
            i != runningStates.end(); ++i)
    {
        if( i->second ) {
            allStopped = false;
            break;
        }
    }

    freebsd_process *child_fproc = dynamic_cast<freebsd_process *>(child_proc);
    child_fproc->setForking(true);
    bool ignored;
    if( !child_fproc->plat_attach(allStopped, ignored) ) {
        perr_printf("Failed to attach to child process %d\n", evFork->getPID());
        return Handler::ret_error;
    }

    return Handler::ret_success;
}

int FreeBSDPreForkHandler::getPriority() const {
    return DefaultPriority;
}

void FreeBSDPreForkHandler::getEventTypesHandled(std::vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::Pre, EventType::Fork));
}

/*
 * In the bootstrap handler, SIGSTOPs where issued to all threads. This function
 * flushes out all those pending "bootstrap stops". Due to some weirdness with
 * signals, the SIGSTOP signal needs to be resent after the process has been continued.
 * I haven't been able to figure out why this needs to happen, but it solves the 
 * problem at hand.
 */
async_ret_t freebsd_process::post_attach(bool wasDetached, set<response::ptr> &aresps) {
  async_ret_t result = thread_db_process::post_attach(wasDetached, aresps);
  if (result != aret_success) return result;
  
  if( !initKQueueEvents() ) return aret_error;

#if defined(bug_freebsd_mt_suspend)
  if( threadPool()->size() <= 1 ) return aret_success;
  
  threadPool()->initialThread()->getStartupState().desyncStateProc(int_thread::stopped);
  
  for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); ++i) {
    freebsd_thread *thrd = dynamic_cast<freebsd_thread *>(*i);
    if (!thrd->hasBootstrapStop())
      continue;
    if (thrd->getDetachState().getState() == int_thread::detached)
      continue;
    
    pthrd_printf("attach workaround: continuing %d/%d\n", getPid(), thrd->getLWP());
    thrd->getStartupState().setState(int_thread::running);
    
#if defined(bug_freebsd_lost_signal)
    throwNopEvent();
    result = waitAndHandleEvents(true);
    if (!result) { 
      pthrd_printf("attach workaround: error handling events.\n");
      return aret_error;
    }

    pthrd_printf("attach workaround: sending stop to %d/%d\n",
		 getPid(), thrd->getLWP());
    thrd->getStartupState().setState(int_thread::stopped);
#endif
    
    throwNopEvent();
    pthrd_printf("attach workaround: handling stop of %d/%d\n",
		 getPid(), thrd->getLWP());
    while (thrd->hasBootstrapStop()) {
      if (!waitAndHandleEvents(true)) {
	pthrd_printf("Error in waitAndHandleEvents for attach workaround\n");
	return aret_error;
      }
    }
  }
  threadPool()->initialThread()->getStartupState().restoreStateProc();
#endif

  return aret_success;
}

#if defined(bug_freebsd_change_pc)
FreeBSDChangePCHandler::FreeBSDChangePCHandler() 
    : Handler("FreeBSD Change PC Handler")
{}

FreeBSDChangePCHandler::~FreeBSDChangePCHandler()
{}


Handler::handler_ret_t FreeBSDChangePCHandler::handleEvent(Event::ptr ev) {
    freebsd_thread *lthread = dynamic_cast<freebsd_thread *>(ev->getThread()->llthrd());

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

bool freebsd_thread::plat_resume() {
    if( !llproc()->threadPool()->hadMultipleThreads() ) return true;

    pthrd_printf("Calling PT_RESUME on %d\n", lwp);
    if( 0 != ptrace(PT_RESUME, lwp, (caddr_t)1, 0) ) {
       perr_printf("Failed to resume lwp %d: %s\n",
                   lwp, strerror(errno));
       setLastError(err_internal, "Failed to resume lwp");
       return false;
    }

    return true;
}

bool freebsd_thread::plat_suspend() {
    if( !llproc()->threadPool()->hadMultipleThreads() ) return true;

    pthrd_printf("Calling PT_SUSPEND on %d\n", lwp);
    if( 0 != ptrace(PT_SUSPEND, lwp, (caddr_t)1, 0) ) {
       perr_printf("Failed to suspend lwp %d: %s\n",
                   lwp, strerror(errno));
       setLastError(err_internal, "Failed to suspend lwp");
       return false;
    }

    return true;
}

bool freebsd_thread::plat_cont() 
{
   freebsd_process *proc = dynamic_cast<freebsd_process *>(llproc());
   int_threadPool *tp = llproc()->threadPool();
   int_threadPool::iterator i;
   
   freebsd_thread *cont_thread = this;
   freebsd_thread *pcBugThrd = NULL;
   for (i = tp->begin(); i != tp->end(); i++) {
      freebsd_thread *thr = dynamic_cast<freebsd_thread *>(*i);
      if (thr->isSignalStopped()) {
         cont_thread = thr;
      }
      if (!thr->plat_setStep()) {
         return false;
      }
   }


#if defined(bug_freebsd_change_pc)
   for (i = tp->begin(); i != tp->end(); i++) {
      freebsd_thread *thrd = dynamic_cast<freebsd_thread *>(*i);
      if (thrd->hasPCBugCondition() && !thrd->isSuspended()) {
         // Only wait for one PC bug signal at a time
         if (!thrd->hasPendingPCBugSignal()) {
            pcBugThrd = thrd;
         } 
         else {
            pthrd_printf("Waiting for PC bug signal for thread %d/%d\n",
                         proc->getPid(), thrd->getLWP());
            pcBugThrd = NULL;
            break;
         }
      }
   }
   
   if (pcBugThrd) {
      pthrd_printf("Attempting to handle change PC bug condition for %d/%d\n",
                   proc->getPid(), pcBugThrd->getLWP());
      pcBugThrd->setPendingPCBugSignal(true);
      if (!tkill(proc->getPid(), pcBugThrd->getLWP(), PC_BUG_SIGNAL)) {
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
    pthrd_printf("Calling PT_CONTINUE on %d/%d with signal %d\n", 
                 proc->getPid(), getLWP(), cont_thread->continueSig_);
    int result = ptrace(PT_CONTINUE, proc->getPid(), (caddr_t)1, cont_thread->continueSig_);
    //int result = ptrace(PT_CONTINUE, getLWP(), (caddr_t)1, cont_thread->continueSig_);
    if (result) {
        perr_printf("low-level continue failed: %s\n", strerror(errno));
        setLastError(err_internal, "Low-level continue failed");
        return false;
    }
    cont_thread->continueSig_ = 0;

#if defined(bug_freebsd_missing_sigstop)
    // XXX this workaround doesn't always work
    sched_yield();
#endif

    return true;
}

bool freebsd_thread::attach() {
    return true;
}

bool freebsd_thread::plat_setStep() {
    int result = 0;
    if (singleStep()) {
        pthrd_printf("Calling PT_SETSTEP on %d/%d\n", 
                llproc()->getPid(), lwp);
        result = ptrace(PT_SETSTEP, lwp, (caddr_t)1, 0);
        is_pt_setstep = true;
    } 
    if (!singleStep() && is_pt_setstep) {
        pthrd_printf("Calling PT_CLEARSTEP on %d/%d\n", 
                llproc()->getPid(), lwp);
        result = ptrace(PT_CLEARSTEP, lwp, (caddr_t)1, 0);
        is_pt_setstep = false;
    }

    if (0 != result) {
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

bool freebsd_process::plat_threadOpsNeedProcStop()
{
   return true;
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
        pthrd_printf("Register %s has value 0x%lx, offset 0x%x\n", reg.name().c_str(), (unsigned long)val, offset);
        regpool.regs[reg] = val;
    }

    return true;
}

#if defined(arch_x86) || defined(arch_x86_64)
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
#if defined(arch_x86)
    if( (old_regs.r_eflags & PSL_RF) != (regs->r_eflags & PSL_RF) ) {
        if( old_regs.r_eflags & PSL_RF ) regs->r_eflags |= PSL_RF;
        else regs->r_eflags &= ~PSL_RF;
    }
#elif defined(arch_x86_64)
    if( (old_regs.r_rflags & PSL_RF) != (regs->r_rflags & PSL_RF) ) {
        if( old_regs.r_rflags & PSL_RF ) regs->r_rflags |= PSL_RF;
        else regs->r_rflags &= ~PSL_RF;
    }
#endif    
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

        pthrd_printf("Register %s gets value 0x%lx, offset 0x%x\n", reg.name().c_str(), (unsigned long)val, offset);
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


