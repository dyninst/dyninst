#if !defined(WINDOWS_H_)
#define WINDOWS_H_

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/arch_process.h"
#include "common/h/dthread.h"
#include <sys/types.h>
#include <vector>

using namespace Dyninst;
using namespace ProcControlAPI;

typedef enum __ptrace_request pt_req;

class GeneratorWindows : public GeneratorMT
{
 public:
   GeneratorWindows();
   virtual ~GeneratorWindows();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);
};

class ArchEventWindows : public ArchEvent
{
   static std::vector<ArchEventWindows *> pending_events;
public:
   bool findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child);
   void postponePairedEvent();

   ArchEventWindows(bool inter_);
   ArchEventWindows(pid_t p, int s);
   ArchEventWindows(int e);

   virtual ~ArchEventWindows();
};

class DecoderWindows : public Decoder
{
 public:
   DecoderWindows();
   virtual ~DecoderWindows();
   virtual unsigned getPriority() const;
   virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
   Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
};

class windows_process : public arch_process
{
 public:
   windows_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
           std::vector<std::string> envp, std::map<int,int> f);
   windows_process(Dyninst::PID pid_, int_process *p);
   virtual ~windows_process();

   virtual bool plat_create();
   virtual bool plat_create_int();
   virtual bool plat_attach(bool allStopped);
   virtual bool plat_attachWillTriggerStop();
   virtual bool plat_forked();
   virtual bool plat_execed();
   virtual bool plat_detach();
   virtual bool plat_terminate(bool &needs_sync);

   virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size);
   virtual bool plat_writeMem(int_thread *thr, const void *local, 
                              Dyninst::Address remote, size_t size);
   virtual SymbolReaderFactory *plat_defaultSymReader();
   virtual bool needIndividualThreadAttach();
   virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
   virtual Dyninst::Architecture getTargetArch();
   virtual bool plat_individualRegAccess();
   virtual bool plat_contProcess() { return true; }
   virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size);
   virtual bool plat_supportLWPEvents() const;
   virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates);
   virtual bool plat_convertToBreakpointAddress(psaddr_t &);
};

class windows_thread : public int_thread
{
 public:
   windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

   windows_thread();
   virtual ~windows_thread();

   virtual bool plat_cont();
   virtual bool plat_stop();
   virtual bool plat_getAllRegisters(int_registerPool &reg);
   virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   virtual bool plat_setAllRegisters(int_registerPool &reg);
   virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
   virtual bool attach();

   virtual bool plat_getThreadArea(int val, Dyninst::Address &addr);
   virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs);
   virtual bool plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result);
   virtual bool plat_needsPCSaveBeforeSingleStep();

   void setOptions();
   bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
   
};


class WindowsHandleNewThr : public Handler
{
 public:
   WindowsHandleNewThr();
   virtual ~WindowsHandleNewThr();
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
   void getEventTypesHandled(std::vector<EventType> &etypes);
};

class WindowsHandleLWPDestroy : public Handler
{
 public:
     WindowsHandleLWPDestroy();
     virtual ~WindowsHandleLWPDestroy();
     virtual handler_ret_t handleEvent(Event::ptr ev);
     virtual int getPriority() const;
     void getEventTypesHandled(std::vector<EventType> &etypes);
};

#endif // !defined WINDOWS_H_
