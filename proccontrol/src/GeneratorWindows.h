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

   virtual void plat_continue(ArchEvent* evt);
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
   struct Waiters
   {
	   typedef dyn_detail::boost::shared_ptr<Waiters> ptr;
	   HANDLE gen_wait, user_wait;
	   bool unhandled_exception;
	   Waiters() {
		   gen_wait = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		   user_wait = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		   unhandled_exception = false;
	   }
	   ~Waiters() {
		   ::CloseHandle(gen_wait);
		   ::CloseHandle(user_wait);
	   }
   };

	// Wake the generator thread to do a ContinueDebugEvent
   void wake(Dyninst::PID);
   // Wait for the ContinueDebugEvent to go through
   void wait(Dyninst::PID);
   void markUnhandledException(Dyninst::PID p);
   void enqueue_event(start_mode m, int_process* p);
   std::deque<StartInfo> procsToStart;
   std::map<Dyninst::PID, Waiters::ptr> waiters;
   std::map<int, int_process*> processes;
   virtual bool hasLiveProc();
};

#endif // !defined(GENERATOR_WINDOWS_H)
