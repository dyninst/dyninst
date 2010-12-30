#if !defined(LINUX_H_)
#define LINUX_H_

//#define debug_async_simulate

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "proccontrol/src/unix.h"
#include "proccontrol/src/x86_process.h"
#include "proccontrol/src/int_thread_db.h"
#include "common/h/dthread.h"
#include <sys/types.h>
#include <sys/ptrace.h>
#include <linux/ptrace.h>
#include <asm/ldt.h>

using namespace Dyninst;
using namespace ProcControlAPI;

typedef enum __ptrace_request pt_req;

class GeneratorLinux : public GeneratorMT
{
 public:
   GeneratorLinux();
   virtual ~GeneratorLinux();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
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

class linux_process : public sysv_process, public unix_process, public x86_process, public thread_db_process
{
 public:
   linux_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int,int> f);
   linux_process(Dyninst::PID pid_, int_process *p);
   virtual ~linux_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach();   
   virtual bool plat_forked();
   virtual bool plat_execed();
   virtual bool plat_detach();
   virtual bool plat_terminate(bool &needs_sync);


   //The following async functions are only used if a linux debugging mode,
   // 'debug_async_simulate' is enabled, which tries to get Linux to simulate having
   // async events for testing purposes.
   virtual bool plat_needsAsyncIO() const;
   virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, 
                                  mem_response::ptr result);
   virtual bool plat_writeMemAsync(int_thread *thr, void *local, Dyninst::Address addr,
                                   size_t size, result_response::ptr result);

   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size);
   virtual bool plat_writeMem(int_thread *thr, void *local, 
                              Dyninst::Address remote, size_t size);
   virtual SymbolReaderFactory *plat_defaultSymReader();
   virtual bool needIndividualThreadAttach();
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
   virtual Dyninst::Architecture getTargetArch();
   virtual bool plat_individualRegAccess();
   virtual bool plat_contProcess() { return true; }
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size);
   virtual bool plat_supportLWPEvents() const;
};

class linux_thread : public thread_db_thread
{
 public:
   linux_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

   linux_thread();
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

   // Needed by HybridLWPControl, unused on Linux
   virtual bool plat_resume() { return true; }
   virtual bool plat_suspend() { return true; }

   void setOptions();
   bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);

   static void fake_async_main(void *);
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
   CondVar init;
   CondVar cond;
   CondVar ret_lock;
   Mutex request_lock;

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
   bool ptrace_write(Dyninst::Address inTrace, unsigned size_, void *inSelf, int pid_);

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

#endif
