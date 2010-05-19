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
#include "common/h/parseauxv.h"
#include "common/h/freebsdKludges.h"

// System includes
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include <cassert>
#include <cerrno>

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

bool DecoderFreeBSD::decode(ArchEvent *ae, std::vector<Event::ptr> &events) {
    ArchEventFreeBSD *archevent = static_cast<ArchEventFreeBSD *>(ae);

    int_process *proc = ProcPool()->findProcByPid(archevent->pid);
    int_thread *thread = ProcPool()->findThread(archevent->pid);
    assert(thread && proc && "Failed to locate process that generated the event");

    freebsd_process *lproc = static_cast<freebsd_process *>(proc);
    //freebsd_thread *lthread = static_cast<freebsd_thread *>(thread);

    Event::ptr event = Event::ptr();

    pthrd_printf("Decoding event for %d/%d\n", proc ? proc->getPid() : -1,
            thread ? thread->getLWP() : -1);

    const int status = archevent->status;
    bool result;
    if( WIFSTOPPED(status) ) {
        const int stopsig = WSTOPSIG(status);

        pthrd_printf("Decoded to signal %d\n", stopsig);
        installed_breakpoint *ibp = NULL;
        switch( stopsig ) {
            case SIGSTOP:
                event = Event::ptr(new EventStop());
                break;
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

                // TODO RPCTrap //

                ibp = proc->getBreakpoint(addr);
                if( ibp && ibp != thread->isClearingBreakpoint() ) {
                    pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(),
                            thread->getLWP(), addr);
                    event = Event::ptr(new EventBreakpoint(addr, ibp));

                    if( addr == lproc->getLibBreakpointAddr() ) {
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
    // TODO thread exit //
    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        if( WIFEXITED(status) ) {
            int exitcode = WIFEXITED(status);
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

bool installed_breakpoint::plat_install(int_process *, bool) {
    // TODO
    assert(!NA_FREEBSD);
    return NULL;
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool) {
    // TODO add any platform-specific handlers

    return hpool;
}

bool ProcessPool::LWPIDsAreUnique() {
    // TODO this appears to be the case, but I don't know for sure
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

/*
 * The man page says that only some requests will return -1 on error.
 * It says to set errno to 0 before the call, and check it afterwards.
 */
static inline long int bsd_ptrace(int request, pid_t pid, caddr_t addr, int data) {
    long int result;
    errno = 0;
    result = ptrace(request, pid, addr, data);
    if( 0 != errno ) {
        result = -1;
    }
    return result;
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
        long int result = bsd_ptrace(PT_TRACE_ME, 0, 0, 0);
        if( -1 == result ) {
            pthrd_printf("Faild to execute a PT_TRACE_ME.\n");
            setLastError(err_internal, "Unable to debug trace new process");
            exit(-1);
        }

        // Never returns
        plat_execv();
    }

    return true;
}

bool freebsd_process::plat_attach() {
    pthrd_printf("Attaching to pid %d\n", pid);
    if( 0 != bsd_ptrace(PT_ATTACH, pid, (caddr_t)1, 0) ) {
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
    // TODO this will change for multiple threads
    pthrd_printf("PT_DETACH on %d\n", getPid());
    if( 0 != bsd_ptrace(PT_DETACH, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_DETACH on %d\n", getPid());
        setLastError(err_internal, "PT_DETACH operation failed\n");
    }
    return true;
}

bool freebsd_process::plat_terminate(bool &needs_sync) {
    pthrd_printf("Terminating process %d\n", getPid());
    if( 0 != bsd_ptrace(PT_KILL, getPid(), (caddr_t)1, 0) ) {
        perr_printf("Failed to PT_KILL process %d\n", getPid());
        setLastError(err_internal, "PT_KILL operation failed\n");
        return false;
    }

    needs_sync = true;
    return true;
}

bool freebsd_process::plat_readMem(int_thread *, void *, 
                         Dyninst::Address, size_t) {
    // TODO
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_writeMem(int_thread *, void *, 
                          Dyninst::Address, size_t) {
    // TODO
    assert(!NA_FREEBSD);
    return false;
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
    assert(!NA_FREEBSD);
    return 0;
}

bool freebsd_process::independentLWPControl() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_process::plat_individualRegAccess() {
    assert(!NA_FREEBSD);
    return false;
}

freebsd_thread::freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
    : int_thread(p, t, l)
{
}

freebsd_thread::~freebsd_thread() 
{
}

bool freebsd_thread::plat_cont() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_stop() {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_getAllRegisters(int_registerPool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_getRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_setAllRegisters(int_registerPool &) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::plat_setRegister(Dyninst::MachRegister, Dyninst::MachRegisterVal) {
    assert(!NA_FREEBSD);
    return false;
}

bool freebsd_thread::attach() {
    assert(!NA_FREEBSD);
    return false;
}
