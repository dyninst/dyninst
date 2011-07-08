#if !defined(WINDOWS_H_)
#define WINDOWS_H_

#include "GeneratorWindows.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/arch_process.h"
#include "common/h/dthread.h"
#include <sys/types.h>
#include <vector>
#include <deque>

using namespace Dyninst;
using namespace ProcControlAPI;


class ArchEventWindows : public ArchEvent
{
   static std::vector<ArchEventWindows *> pending_events;
public:
   bool findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child);
   void postponePairedEvent();

   ArchEventWindows(DEBUG_EVENT e);

   virtual ~ArchEventWindows();
   DEBUG_EVENT evt;
};

class PC_EXPORT WinEventNewThread : public EventNewLWP
{
   friend void dyn_detail::boost::checked_delete<WinEventNewThread>(WinEventNewThread *);
   friend void dyn_detail::boost::checked_delete<const WinEventNewThread>(const WinEventNewThread *);
 public:
   typedef dyn_detail::boost::shared_ptr<WinEventNewThread> ptr;
   typedef dyn_detail::boost::shared_ptr<const WinEventNewThread> const_ptr;
	WinEventNewThread(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts,
		LPVOID base) : EventNewLWP(l), hthread(ht), thread_start(ts), tls_base(base)
	{}
	virtual ~WinEventNewThread() {}

	HANDLE getHandle() const { return hthread; }
	LPTHREAD_START_ROUTINE getThreadStart() const { return thread_start; }
	LPVOID getTLSBase() const { return tls_base; }
private:
	HANDLE hthread;
	LPTHREAD_START_ROUTINE thread_start;
	LPVOID tls_base;
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
   virtual bool plat_attach_int();
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
   virtual unsigned int getTargetPageSize();
   virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr,
												   unsigned long size, void*& buffer, unsigned long& buffer_size,
												   unsigned long& start_offset);
   virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size,
													 void*& buffer, unsigned long& buffer_size,
													 unsigned long& start_offset);
   virtual bool plat_collectAllocationResult(int_thread* thr, reg_response::ptr resp);
   virtual bool refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs,
                                  std::set<response::ptr> &async_responses);
   virtual int_library *getExecutableLib();
   virtual bool initLibraryMechanism();
   virtual bool plat_isStaticBinary();
   HANDLE plat_getHandle();
   virtual bool hasPendingDetach() const { return pendingDetach; }
   virtual void clearPendingDebugBreak() { pendingDebugBreak_ = false; }
   virtual void setPendingDebugBreak() { pendingDebugBreak_ = true; }
   virtual bool pendingDebugBreak() const { return pendingDebugBreak_; }
private:
	HANDLE hproc;
	bool pendingDetach;
	bool pendingDebugBreak_;

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
   bool plat_suspend();
   bool plat_resume();
   bool haveUserThreadInfo();
   bool getTID(Dyninst::THR_ID& tid);
   bool getStartFuncAddress(Dyninst::Address& start_addr);
   bool getStackBase(Dyninst::Address& stack_base);
   bool getStackSize(unsigned long& stack_size);
   bool getTLSPtr(Dyninst::Address& tls_ptr);
   void setHandle(HANDLE h);
private:
	HANDLE hthread;
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
