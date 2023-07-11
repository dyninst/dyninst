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
#if !defined(LINUX_H_)
#define LINUX_H_

//#define debug_async_simulate

#include "Generator.h"
#include "Event.h"
#include "Decoder.h"
#include "Handler.h"
#include "int_process.h"
#include "int_thread_db.h"

#include "sysv.h"
#include "unix.h"

#include "x86_process.h"
#include "ppc_process.h"
#include "arm_process.h"
#include "mmapalloc.h"
#include "processplat.h"

#include "common/src/dthread.h"
#include <map>
#include <stddef.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/ptrace.h>

typedef enum __ptrace_request pt_req;

class GeneratorLinux : public GeneratorMT
{
  private:
   int generator_lwp;
   int generator_pid;

  public:
   GeneratorLinux();
   virtual ~GeneratorLinux();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
   void evictFromWaitpid();
};

class ArchEventLinux : public ArchEvent
{
   static std::vector<ArchEventLinux *> pending_events;
public:
   int status;
   pid_t pid;
   bool interrupted;
   int error;
   pid_t child_pid;
   int event_ext;
   bool findPairedEvent(ArchEventLinux* &parent, ArchEventLinux* &child);
   void postponePairedEvent();

   ArchEventLinux(bool inter_);
   ArchEventLinux(pid_t p, int s);
   ArchEventLinux(int e);

   virtual ~ArchEventLinux();
};

class DecoderLinux : public Decoder
{
 public:
   DecoderLinux();
   virtual ~DecoderLinux();
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
   Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
};

class linux_process : public sysv_process, public unix_process, public thread_db_process, public indep_lwp_control_process, public mmap_alloc_process, public int_followFork, public int_signalMask, public int_LWPTracking, public int_memUsage
{
 public:
   linux_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   linux_process(Dyninst::PID pid_, int_process *p);
   virtual ~linux_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach(bool allStopped, bool &);
   virtual bool plat_attachThreadsSync();
   virtual bool plat_attachWillTriggerStop();
   virtual bool plat_forked();
   virtual bool plat_execed();
   virtual bool plat_detach(result_response::ptr resp, bool leave_stopped);
   virtual bool plat_terminate(bool &needs_sync);
   virtual bool preTerminate();
   virtual OSType getOS() const;

   //The following async functions are only used if a linux debugging mode,
   // 'debug_async_simulate' is enabled, which tries to get Linux to simulate having
   // async events for testing purposes.
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr,
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result, bp_write_t bp_write);

   virtual bool plat_readMem(int_thread *thr, void *local,
                             Dyninst::Address remote, size_t size);
   virtual bool plat_writeMem(int_thread *thr, const void *local,
                              Dyninst::Address remote, size_t size, bp_write_t bp_write);
   virtual SymbolReaderFactory *plat_defaultSymReader();
   virtual bool needIndividualThreadAttach();
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
   virtual bool plat_individualRegAccess();
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size);
   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);
   virtual bool plat_supportLWPCreate();
   virtual bool plat_supportLWPPreDestroy();
   virtual bool plat_supportLWPPostDestroy();
   virtual void plat_adjustSyncType(Event::ptr ev, bool gen);
   virtual bool fork_setTracking(FollowFork::follow_t b);
   virtual FollowFork::follow_t fork_isTracking();
   virtual bool plat_lwpChangeTracking(bool b);
   virtual bool allowSignal(int signal_no);

   bool readStatM(unsigned long &stk, unsigned long &heap, unsigned long &shrd);
   virtual bool plat_getStackUsage(MemUsageResp_t *resp);
   virtual bool plat_getHeapUsage(MemUsageResp_t *resp);
   virtual bool plat_getSharedUsage(MemUsageResp_t *resp);
   virtual bool plat_residentNeedsMemVals();
   virtual bool plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu,
                                      MemUsageResp_t *resp);

  protected:
   int computeAddrWidth();
};

class linux_x86_process : public linux_process, public x86_process
{
  public:
   linux_x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   linux_x86_process(Dyninst::PID pid_, int_process *p);
   virtual ~linux_x86_process();

   virtual Dyninst::Architecture getTargetArch();
   virtual bool plat_supportHWBreakpoint();
};

class linux_ppc_process : public linux_process, public ppc_process
{
  public:
   linux_ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   linux_ppc_process(Dyninst::PID pid_, int_process *p);
   virtual ~linux_ppc_process();

   virtual Dyninst::Architecture getTargetArch();
};

//steve: added
class linux_arm_process : public linux_process, public arm_process
{
  public:
   linux_arm_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
           std::vector<std::string> envp, std::map<int,int> f);
   linux_arm_process(Dyninst::PID pid_, int_process *p);
   virtual ~linux_arm_process();

   virtual Dyninst::Architecture getTargetArch();
};


class linux_thread : virtual public thread_db_thread
{
 public:
   linux_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

   virtual ~linux_thread();

   virtual bool plat_cont();
   virtual bool plat_stop();
   virtual bool plat_getAllRegisters(int_registerPool &reg);
   virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   virtual bool plat_setAllRegisters(int_registerPool &reg);
   virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool attach();

   virtual bool plat_getAllRegistersAsync(allreg_response::ptr result);
   virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg,
                                      reg_response::ptr result);
   virtual bool plat_setAllRegistersAsync(int_registerPool &pool,
                                          result_response::ptr result);
   virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg,
                                      Dyninst::MachRegisterVal val,
                                      result_response::ptr result);
   virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr);
   virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gprs_only = false);

   virtual bool plat_handle_ghost_thread();
   void setOptions();
   bool unsetOptions();
   bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);

   void postponeSyscallEvent(ArchEventLinux *event);
   bool hasPostponedSyscallEvent();
   ArchEventLinux *getPostponedSyscallEvent();

   static void fake_async_main(void *);
   virtual bool suppressSanityChecks();

   void setGeneratorExiting() { generator_started_exit_processing = true; }
 private:
   ArchEventLinux *postponed_syscall_event;
   bool generator_started_exit_processing;
};

class linux_x86_thread : virtual public linux_thread, virtual public x86_thread
{
  public:
   linux_x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~linux_x86_thread();
};

class linux_ppc_thread : virtual public linux_thread, virtual public ppc_thread
{
  public:
   linux_ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~linux_ppc_thread();
};

class linux_arm_thread : virtual public linux_thread, virtual public arm_thread
{
  public:
   linux_arm_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
   virtual ~linux_arm_thread();
};

class LinuxPtrace
{
private:
   typedef enum {
      unknown,
      create_req,
      ptrace_req,
      ptrace_bulkread,
      ptrace_bulkwrite
   } req_t;

   req_t ptrace_request;
   pt_req request;
   pid_t pid;
   void *addr;
   void *data;
   linux_process *proc;
   Dyninst::Address remote_addr;
   unsigned size;
   long ret;
   bool bret;
   int err;

   DThread thrd;
   CondVar<> init;
   CondVar<> cond;
   CondVar<> ret_lock;
   Mutex<> request_lock;

   void start_request();
   void waitfor_ret();
   void end_request();
   static LinuxPtrace *linuxptrace;
public:
   static LinuxPtrace *getPtracer();
   LinuxPtrace();
   ~LinuxPtrace();
   void start();
   void main();
   long ptrace_int(pt_req request_, pid_t pid_, void *addr_, void *data_);
   bool ptrace_read(Dyninst::Address inTrace, unsigned size_, void *inSelf, int pid_);
   bool ptrace_write(Dyninst::Address inTrace, unsigned size_, const void *inSelf, int pid_);

   bool plat_create(linux_process *p);
};

long do_ptrace(pt_req request_, pid_t pid_, void *addr_, void *data_);

class LinuxHandleNewThr : public Handler
{
 public:
   LinuxHandleNewThr();
   virtual ~LinuxHandleNewThr();
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
   void getEventTypesHandled(std::vector<EventType> &etypes);
};

class LinuxHandleLWPDestroy : public Handler
{
 public:
     LinuxHandleLWPDestroy();
     virtual ~LinuxHandleLWPDestroy();
     virtual handler_ret_t handleEvent(Event::ptr ev);
     virtual int getPriority() const;
     void getEventTypesHandled(std::vector<EventType> &etypes);
};

class LinuxHandleForceTerminate : public Handler
{
  public:
   LinuxHandleForceTerminate();
   virtual ~LinuxHandleForceTerminate();
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
   void getEventTypesHandled(std::vector<EventType> &etypes);
};

SymbolReaderFactory *getElfReader();

#endif
