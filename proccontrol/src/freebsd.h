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
#if !defined(FREEBSD_H_)
#define FREEBSD_H_

#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>
#include "Generator.h"
#include "Event.h"
#include "Decoder.h"
#include "Handler.h"
#include "int_process.h"
#include "int_thread_db.h"
#include "unix.h"
#include "sysv.h"
#include "x86_process.h"
#include "mmapalloc.h"

#include "common/src/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class GeneratorFreeBSD : public GeneratorMT
{
public:
    GeneratorFreeBSD();
    virtual ~GeneratorFreeBSD();

    virtual bool initialize();
    virtual bool canFastHandle();
    virtual ArchEvent *getEvent(bool block);
};

class ArchEventFreeBSD : public ArchEvent
{
public:
    int status;
    pid_t pid;
    lwpid_t lwp;
    bool interrupted;
    int error;

    ArchEventFreeBSD(bool inter_);
    ArchEventFreeBSD(pid_t p, lwpid_t l, int s);
    ArchEventFreeBSD(int e);

    virtual ~ArchEventFreeBSD();
};

class DecoderFreeBSD : public Decoder
{
public:
    DecoderFreeBSD();
    virtual ~DecoderFreeBSD();
    virtual unsigned getPriority() const;
    virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
    Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
};

class freebsd_process : virtual public sysv_process, \
  virtual public unix_process, \
  virtual public x86_process, \
  virtual public thread_db_process, \
  virtual public mmap_alloc_process, \
  virtual public hybrid_lwp_control_process
{
   friend class freebsd_thread;
public:
    freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
            std::vector<std::string> envp, std::map<int, int> f);
    freebsd_process(Dyninst::PID pid_, int_process *p);
    virtual ~freebsd_process();

    virtual bool plat_create();
    virtual bool plat_attach(bool allStopped, bool &);
    virtual bool plat_forked();
    virtual bool plat_execed();
    virtual bool plat_detach(result_response::ptr resp, bool leave_stopped);
    virtual bool plat_terminate(bool &needs_sync);

    virtual bool plat_readMem(int_thread *thr, void *local,
                              Dyninst::Address remote, size_t size);
    virtual bool plat_writeMem(int_thread *thr, const void *local,
                               Dyninst::Address remote, size_t size, bp_write_t bp_write);

    virtual bool needIndividualThreadAttach();
    virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
    virtual Dyninst::Architecture getTargetArch();
    virtual bool plat_individualRegAccess();
    virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);
    virtual OSType getOS() const;

    virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &async_responses);
    virtual async_ret_t post_create(std::set<response::ptr> &async_responses);

    virtual int getEventQueue();
    virtual bool initKQueueEvents();
    virtual SymbolReaderFactory *plat_defaultSymReader();
    virtual bool plat_threadOpsNeedProcStop();

    /* handling forks on FreeBSD */
    virtual bool forked();
    virtual bool isForking() const;
    virtual void setForking(bool b);
    virtual bool post_forked();
    virtual freebsd_process *getParent();

    /* thread_db_process methods */
    virtual const char *getThreadLibName(const char *symName);
    virtual bool isSupportedThreadLib(string libName);
    virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo);

    virtual bool plat_suspendThread(int_thread *thr);
    virtual bool plat_resumeThread(int_thread *thr);
    //virtual bool plat_debuggerSuspended();
    //virtual void noteNewDequeuedEvent(Event::ptr ev);

protected:
    string libThreadName;
    bool forking;
    freebsd_process *parent;
};

class freebsd_thread : public thread_db_thread
{
public:
    freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

    freebsd_thread();
    virtual ~freebsd_thread();

    virtual bool plat_cont();
    virtual bool plat_stop();
    virtual bool plat_getAllRegisters(int_registerPool &reg);
    virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
    virtual bool plat_setAllRegisters(int_registerPool &reg);
    virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
    virtual bool attach();
    virtual bool plat_suspend();
    virtual bool plat_resume();

    /* FreeBSD-specific */
    virtual bool plat_setStep();
    void setBootstrapStop(bool b);
    bool hasBootstrapStop() const;
    void setPCBugCondition(bool b);
    bool hasPCBugCondition() const;
    void setPendingPCBugSignal(bool b);
    bool hasPendingPCBugSignal() const;
    void setSignalStopped(bool b);
    bool isSignalStopped() const;
    bool isAlive();

protected:
    bool bootstrapStop;
    bool pcBugCondition;
    bool pendingPCBugSignal;
    bool signalStopped;
    bool is_pt_setstep;
    bool is_exited;
};

class FreeBSDPollLWPDeathHandler : public Handler
{
public:
    FreeBSDPollLWPDeathHandler();
    virtual ~FreeBSDPollLWPDeathHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

class FreeBSDPostThreadDeathBreakpointHandler : public Handler
{
 public:
  FreeBSDPostThreadDeathBreakpointHandler();
  virtual ~FreeBSDPostThreadDeathBreakpointHandler();
  virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

#if defined(bug_freebsd_mt_suspend)
class FreeBSDPreStopHandler : public Handler
{
public:
    FreeBSDPreStopHandler();
    virtual ~FreeBSDPreStopHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

class FreeBSDBootstrapHandler : public Handler
{
public:
    FreeBSDBootstrapHandler();
    virtual ~FreeBSDBootstrapHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};
#endif

#if defined(bug_freebsd_change_pc)
class FreeBSDChangePCHandler : public Handler
{
public:
    FreeBSDChangePCHandler();
    virtual ~FreeBSDChangePCHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};
#endif

class FreeBSDPreForkHandler : public Handler
{
public:
    FreeBSDPreForkHandler();
    ~FreeBSDPreForkHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

#endif
