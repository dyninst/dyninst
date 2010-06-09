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
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/freebsd.h"
#include "proccontrol/src/int_handler.h"
#include "common/h/freebsdKludges.h"

// System includes
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <machine/reg.h>

#include <cassert>
#include <cerrno>

#include <map>
using std::make_pair;

using namespace Dyninst;
using namespace ProcControlAPI;

#define NA_FREEBSD "This function is not implemented on FreeBSD"

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
   // NOTE: while similar to Linux code, it is not the same
   // e.g., __WALL is not defined.
   int status, options;

   //Block (or not block) in waitpid to receive a OS event
   options = block ? 0 : WNOHANG; 
   pthrd_printf("%s in waitpid\n", block ? "blocking" : "polling");
   int pid = waitpid(-1, &status, options);

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

   newevent = new ArchEventFreeBSD(pid, status);
   return newevent;
}

ArchEventFreeBSD::ArchEventFreeBSD(bool inter_) : 
   status(0),
   pid(NULL_PID),
   interrupted(inter_),
   error(0)
{
}

ArchEventFreeBSD::ArchEventFreeBSD(pid_t p, int s) : 
   status(s),
   pid(p), 
   interrupted(false), 
   error(0)
{
}

ArchEventFreeBSD::ArchEventFreeBSD(int e) : 
   status(0),
   pid(NULL_PID),
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

bool DecoderFreeBSD::decode(ArchEvent *ae, std::vector<Event::ptr> &events) {
    ArchEventFreeBSD *archevent = static_cast<ArchEventFreeBSD *>(ae);

    int_process *proc = NULL;
    freebsd_process *lproc = NULL;
    freebsd_thread *lthread = NULL;

    int_thread *thread = ProcPool()->findThread(archevent->pid);

    if( thread ) {
        proc = thread->llproc();
        lthread = static_cast<freebsd_thread *>(thread);
    }

    if( proc ) {
        lproc = static_cast<freebsd_process *>(proc);
    }

    //int_process *proc = ProcPool()->findProcByPid(archevent->pid);

    assert( (thread || proc) && "Couldn't locate process/thread that created the event");

    Event::ptr event = Event::ptr();

    pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
            thread ? thread->getLWP() : -1);

    const int status = archevent->status;
    bool result;
    if( WIFSTOPPED(status) ) {
        const int stopsig = WSTOPSIG(status);

        pthrd_printf("Decoded to signal %s\n", strsignal(stopsig));
        installed_breakpoint *ibp = NULL;
        Dyninst::Address adjusted_addr = 0;
        switch( stopsig ) {
            case SIGSTOP:
                if( lthread->hasPendingStop() ) {
                    pthrd_printf("Received pending SIGSTOP on %d/%d\n",
                            thread->llproc()->getPid(), thread->getLWP());
                    event = Event::ptr(new EventStop());
                    break;
                }
                // Relying on fall through for bootstrap
            case SIGTRAP:
                if( proc->getState() == int_process::neonatal_intermediate) {
                    pthrd_printf("Decoded event to bootstrap on %d/%d\n",
                            proc->getPid(), thread->getLWP());
                    event = Event::ptr(new EventBootstrap());
                    break;
                }

                Dyninst::MachRegisterVal addr;
                result = thread->getRegister(MachRegister::getPC(proc->getTargetArch()), addr);
                if( !result ) {
                    perr_printf("Failed to read PC address upon SIGTRAP\n");
                    return false;
                }

                adjusted_addr = adjustTrapAddr(addr, proc->getTargetArch());

                // TODO RPCTrap //

                ibp = proc->getBreakpoint(adjusted_addr);
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
                    }
                    break;
                }

                if( thread->singleStep() ) {
                    ibp = thread->isClearingBreakpoint();
                    if( ibp ) {
                        pthrd_printf("Decoded event to breakpoint cleanup\n");
                        event = Event::ptr(new EventBreakpointClear(ibp));
                        break;
                    }else{
                        pthrd_printf("Decoded event to single step on %d/%d\n",
                                proc->getPid(), thread->getLWP());
                        event = Event::ptr(new EventSingleStep());
                        break;
                    }
                }else{
                    // TODO for now, just assume that a call to exec happened
                    // In the future, if we need to trace syscall entry/exit, we will
                    // need to differentiate between different syscalls
                    pthrd_printf("Decoded event to exec on %d/%d\n",
                            proc->getPid(), thread->getLWP());
                    event = Event::ptr(new EventExec(EventType::Post));
                    event->setSyncType(Event::sync_process);
                }
                break;
            default:
                pthrd_printf("Decoded event to signal %d on %d/%d\n",
                        stopsig, proc->getPid(), thread->getLWP());
                event = Event::ptr(new EventSignal(stopsig));
                break;
        }
        if( event && event->getSyncType() == Event::unset) 
            event->setSyncType(Event::sync_thread);
    }
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        if( WIFEXITED(status) ) {
            int exitcode = WEXITSTATUS(status);
            /*
             * TODO add a capability for this (plus a configure check for it)
             * XXX This likely a kernel bug.
             *
             * Sometimes, when a traced process exits, an extraneous exit for the process
             * will be returned by waitpid. That is, it will appear as if the
             * process has exited twice.
             *
             * The workaround is to not generate a new exit event for a process if the
             * generator has already seen the exit event
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
    // TODO thread exit

    // Single event decoded
    assert(event);
    assert(proc->proc());
    assert(thread->thread());
    delete archevent;

    event->setThread(thread->thread());
    event->setProcess(proc->proc());
    events.push_back(event);

    return true;
}

int_process *int_process::createProcess(Dyninst::PID pid_, std::string exec) {
    std::vector<std::string> args;
    freebsd_process *newproc = new freebsd_process(pid_, exec, args);
    assert(newproc);

    return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(std::string exec, std::vector<std::string> args) {
    freebsd_process *newproc = new freebsd_process(0, exec, args);
    assert(newproc);
    return static_cast<int_process *>(newproc);
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *parent) {
    freebsd_process *newproc = new freebsd_process(pid_, parent);
    assert(newproc);
    return static_cast<int_process *>(newproc);
}

int_thread *int_thread::createThreadPlat(int_process *proc, Dyninst::THR_ID thr_id,
        Dyninst::LWP lwp_id, bool initial_thrd)
{
    if( initial_thrd ) {
        lwp_id = proc->getPid();
    }
    freebsd_thread *lthrd = new freebsd_thread(proc, thr_id, lwp_id);
    assert(lthrd);
    return static_cast<int_thread *>(lthrd);
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool) {
    // TODO add any platform-specific handlers

    return hpool;
}

bool ProcessPool::LWPIDsAreUnique() {
    return true;
}

bool iRPCMgr::collectAllocationResult(int_thread *, Dyninst::Address &, bool &) {
    // TODO
    assert(!NA_FREEBSD);
    return false;
}

bool iRPCMgr::createDeallocationSnippet(int_process *, Dyninst::Address, unsigned long,
        void * &, unsigned long &, unsigned long &)
{
    // TODO
    assert(!NA_FREEBSD);
    return false;
}

bool iRPCMgr::createAllocationSnippet(int_process *, Dyninst::Address,
        bool, unsigned long, void * &, unsigned long &, unsigned long &)
{
    // TODO
    assert(!NA_FREEBSD);
    return false;
}

freebsd_process::freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a)
    : sysv_process(p, e, a)
{
}

freebsd_process::freebsd_process(Dyninst::PID pid_, int_process *p) 
    : sysv_process(pid_, p)
{
}

freebsd_process::~freebsd_process() 
{
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

        plat_execv(); // Never returns

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
    // TODO
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::post_forked() {
    // TODO
    assert(!NA_FREEBSD);
    return false;
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

bool freebsd_process::plat_detach() {
    pthrd_printf("PT_DETACH on %d\n", getPid());
    if( 0 != ptrace(PT_DETACH, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_DETACH on %d\n", getPid());
        setLastError(err_internal, "PT_DETACH operation failed\n");
    }
    return true;
}

bool freebsd_process::plat_terminate(bool &needs_sync) {
    pthrd_printf("Terminating process %d\n", getPid());
    if( 0 != ptrace(PT_KILL, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_KILL process %d\n", getPid());
        setLastError(err_internal, "PT_KILL operation failed\n");
        return false;
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
    // TODO not explored in depth
    return false;
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

Dyninst::Address freebsd_process::plat_mallocExecMemory(Dyninst::Address, unsigned) {
    // TODO
    assert(!NA_FREEBSD);
    return 0;
}

bool freebsd_process::independentLWPControl() {
    // TODO for now, this will probably need to change to a hybrid
    return true;
}

freebsd_thread::freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
    : int_thread(p, t, l)
{
}

freebsd_thread::~freebsd_thread() 
{
}

bool freebsd_thread::plat_cont() {
    // TODO the whole process is continued
    //
    // On FreeBSD, individual threads are controlled by stopping the whole
    // process and then suspending or resuming individual threads before
    // continuing the process.
    //

    int result;
    if (singleStep()) {
        pthrd_printf("Calling PT_STEP with signal %d\n", continueSig_);
        result = ptrace(PT_STEP, proc_->getPid(), (caddr_t)1, continueSig_);
    } else {
        pthrd_printf("Calling PT_CONTINUE with signal %d\n", continueSig_);
        result = ptrace(PT_CONTINUE, proc_->getPid(), (caddr_t)1, continueSig_);
    }

    if (0 != result) {
        int error = errno;
        perr_printf("low-level continue failed: %s\n", strerror(error));
        setLastError(err_internal, "Low-level continue failed\n");
        return false;
    }

    return true;
}

static bool t_kill(pid_t pid, long lwp, int sig) {
    static bool has_tkill = true;
    int result = 0;

    pthrd_printf("Sending %d to %ld/%d\n", sig, lwp, pid);

    if( has_tkill ) {
        // No threads have been created yet
        if( pid == lwp ) {
            result = kill(pid, sig);
        }else{
            result = syscall(SYS_thr_kill2, pid, lwp, sig);
            if( 0 != result && ENOSYS == errno ) {
                pthrd_printf("Using kill instead of tkill on this system\n");
                has_tkill = false;
            }
        }
    }

    if( !has_tkill ) {
        result = kill(pid, sig);
    }

    return (result == 0);
}

bool freebsd_thread::plat_stop() {
    assert(pending_stop);

    Dyninst::PID pid = proc_->getPid();
    if( !t_kill(pid, lwp, SIGSTOP) ) {
        int errnum = errno;
        if( ESRCH == errnum ) {
            pthrd_printf("t_kill failed for %d/%d, thread/process doesn't exit\n", lwp, pid);
            setLastError(err_noproc, "Thread no longer exists");
            return false;
        }
        pthrd_printf("t_kill failed on %d/%d: %s\n", lwp, pid, strerror(errnum));
        setLastError(err_internal, "Could not send signal to process while stopping");
        return false;
    }

    return true;
}

bool freebsd_thread::attach() {
    // TODO this might be right
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

    // TODO implement this for other architectures
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
    assert(!"Register conversion is not implemented for this architecture");
#endif

    initialized = true;

    init_lock.unlock();
}

bool freebsd_process::plat_individualRegAccess() {
    return false;
}

bool freebsd_thread::plat_getAllRegisters(int_registerPool &regpool) {
    struct reg registers;
    unsigned char *regPtr = (unsigned char *)&registers;

    if( 0 != ptrace(PT_GETREGS, lwp, (caddr_t)regPtr, 0) ) {
        perr_printf("Error reading registers from LWP %d\n", lwp);
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

        MachRegisterVal val;
        const unsigned int offset = i->second.first;
        const unsigned int size = i->second.second;

        if( size == sizeof(uint32_t) ) {
            val = *((uint32_t *)(&regPtr[offset]));
        }else if( size == sizeof(uint64_t) ) {
            val = *((uint64_t *)(&regPtr[offset]));
        }else{
            assert(!"Unknown address width");
        }
        pthrd_printf("Register %s has value 0x%x, offset 0x%x\n", reg.name(), (unsigned int)val, offset);
        regpool.regs[reg] = val;
    }

    return true;
}

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

        pthrd_printf("Register %s gets value %lx, offset %d\n", reg.name(), val, offset);
    }

    if (num_found != regpool.regs.size()) {
        setLastError(err_badparam, "Invalid register set passed to setAllRegisters");
        perr_printf("Couldn't find all registers in the register set %u/%u\n", num_found,
                    (unsigned int) regpool.regs.size());
        return false;
    }

    if( 0 != ptrace(PT_SETREGS, lwp, (caddr_t)&registers, 0) ) {
        perr_printf("Error setting registers for LWP %d\n", lwp);
        setLastError(err_internal, "Could not set registers in thread");
        return false;
    }

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
