#if !defined(GENERATOR_WINDOWS_H)
#define GENERATOR_WINDOWS_H

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "int_process.h"
#include <sys/types.h>
#include <vector>
#include <deque>


using namespace Dyninst;
using namespace ProcControlAPI;


class GeneratorWindows : public GeneratorMT
{
 public:
   GeneratorWindows();
   virtual ~GeneratorWindows();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);

   virtual bool plat_continue(ArchEvent* evt);
   virtual void plat_start();
   enum start_mode {
	   create,
	   attach
   };
   struct StartInfo
   {
	   start_mode mode;
	   int_process* proc;
   };
   struct processData
   {
	   typedef dyn_detail::boost::shared_ptr<processData> ptr;
	   bool unhandled_exception;
	   int_process* proc;
	   state_t state;
	   processData(const processData& o) :
		unhandled_exception(o.unhandled_exception),
			proc(o.proc),
			state(o.state) {}
		processData() : unhandled_exception(false),
			proc(NULL),
			state(none) {}
   };
   class CriticalSection
   {
	   Mutex& myLock;
   public:
	   CriticalSection(Mutex m) : myLock(m) {
		//   myLock.lock();
	   }
	   ~CriticalSection() { /*myLock.unlock();*/ }
   };


   void markUnhandledException(Dyninst::PID p);
   void enqueue_event(start_mode m, int_process* p);
   std::deque<StartInfo> procsToStart;
   std::map<int, processData::ptr> thread_to_proc;
   virtual bool isExitingState();
   virtual void setState(state_t newstate);
   virtual state_t getState();
   virtual bool hasLiveProc();
   void removeProcess(int_process* proc);

   virtual ArchEvent* getCachedEvent();
   virtual void setCachedEvent(ArchEvent* ae);
   std::map<int, ArchEvent*> m_Events;
   std::map<Dyninst::PID, long long> alreadyHandled;
   //Mutex processDataLock;
};

#endif // !defined(GENERATOR_WINDOWS_H)
