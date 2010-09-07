#include "proccontrol/h/Handler.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "dynutil/h/dyn_regs.h"

using namespace Dyninst;

#include <assert.h>

Handler::Handler(std::string name_) :
   name(name_)
{
}

Handler::~Handler()
{
}

int Handler::getPriority() const
{
   return DefaultPriority;
}

Event::ptr Handler::convertEventForCB(Event::ptr /*orig*/)
{
   return Event::ptr();
}

std::string Handler::getName() const
{
   return name;
}

HandlerPool::HandlerPool()
{
}

HandlerPool::~HandlerPool()
{
   for (HandlerMap_t::iterator i = handlers.begin(); i != handlers.end(); i++) {
      delete (*i).second;
   }
   //Do not delete actual Handler* objects.
   handlers.clear();
}

void HandlerPool::addHandlerInt(EventType etype, Handler *handler)
{
   pthrd_printf("Handler %s will handle event %s\n", handler->getName().c_str(), 
                etype.name().c_str());
   assert(etype.time() != EventType::Any);
   HandlerMap_t::iterator i = handlers.find(etype);
   HandlerSet_t *theset = NULL;
   if (i == handlers.end()) {
      theset = new HandlerSet_t();
      handlers[etype] = theset;
   }
   else {
      theset = (*i).second;
   }   
   theset->insert(handler);

   HandleCallbacks *cb = HandleCallbacks::getCB();
   if (cb != handler) {
      cb->alleventtypes.insert(etype);
      if (!(etype.code() >= EventType::InternalEvents && etype.code() < EventType::MaxProcCtrlEvent))
         addHandlerInt(etype, cb);
   }
}

void HandlerPool::addHandler(Handler *handler)
{
   std::vector<EventType> etypes;
   handler->getEventTypesHandled(etypes);

   for (std::vector<EventType>::iterator i = etypes.begin(); i != etypes.end(); i++)
   {
      if ((*i).time() == EventType::Any) {
         addHandlerInt(EventType(EventType::Pre, (*i).code()), handler);
         addHandlerInt(EventType(EventType::Post, (*i).code()), handler);
         addHandlerInt(EventType(EventType::None, (*i).code()), handler);
      }
      else {
         addHandlerInt(*i, handler);
      }
   }
}

struct eh_cmp_func
{
   bool operator()(const pair<Event::ptr, Handler*> &a,
                   const pair<Event::ptr, Handler*> &b)
   {
      if (a.second->getPriority() != b.second->getPriority())
         return a.second->getPriority() < b.second->getPriority();
      if (a.first->subservientTo().lock() == b.first)
         return false;
      if (b.first->subservientTo().lock() == a.first)
         return true;
      eventtype_cmp cmp;
      return cmp(a.first->getEventType(), b.first->getEventType());
   }
};

bool HandlerPool::handleEvent(Event::ptr ev)
{
   EventType etype = ev->getEventType();
   Event::ptr cb_replacement_ev = Event::ptr();
   
   /**
    * An event and its subservient events are a set of events that
    * are processed at the same time.  As an example, on SysV systems
    * a Library event is triggered by a breakpoint.  We want to do both the
    * breakpoint handling and the library handling at the same time,
    * but don't want to seperate the handling of these events.  In this
    * example the Library event is subservient to the Breakpoint event.
    *
    * We'll take the event and all its subservient set and run handlers
    * for all of them.
    **/
   set<Event::ptr> all_events;
   all_events.insert(ev);      
   for (vector<Event::ptr>::iterator i = ev->subservient_events.begin();
        i != ev->subservient_events.end(); i++)
   {
      all_events.insert(*i);
   }

   typedef set<pair<Event::ptr, Handler *>, eh_cmp_func > ev_hndler_set_t;
   ev_hndler_set_t events_and_handlers;      
   for (set<Event::ptr>::iterator i = all_events.begin(); i != all_events.end(); i++)
   {
      Event::ptr ev = *i;
      EventType etype = ev->getEventType();
      HandlerMap_t::iterator j = handlers.find(etype);
      if (j == handlers.end()) {
         perr_printf("Event %s has no handlers registered\n", etype.name().c_str());
         continue;
      }
      HandlerSet_t *hset = j->second;
      for (HandlerSet_t::iterator k = hset->begin(); k != hset->end(); k++)
      {
         Handler *hnd = *k;
         events_and_handlers.insert(pair<Event::ptr, Handler*>(ev, hnd));
      }
   }

   /**
    * We should have all events and handlers properly sorted into the
    * events_and_handlers set in the order we want to run them in.
    * Now it's finally time to run the handlers.
    **/
   bool handled_something = false;
   ev_hndler_set_t::iterator i;
   for (i = events_and_handlers.begin(); i != events_and_handlers.end(); i++)
   {
      handled_something = true;
      Event::ptr event = i->first;
      Handler *handler = i->second;
      EventType etype = ev->getEventType();

      pthrd_printf("Handling event '%s' with handler '%s'\n", etype.name().c_str(), 
                   handler->getName().c_str());
      
      bool result = handler->handleEvent(event);
      if (!result) {
         pthrd_printf("Error handling event %s with %s\n", etype.name().c_str(), 
                      handler->getName().c_str());
         return false;
      }
   }
   return handled_something;
}

HandleBootstrap::HandleBootstrap() :
   Handler(std::string("Bootstrap"))
{
}

HandleBootstrap::~HandleBootstrap()
{
}

void HandleBootstrap::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}

bool HandleBootstrap::handleEvent(Event::ptr ev)
{
   int_process *p = ev->getProcess()->llproc();
   assert(p);
   pthrd_printf("Handling bootstrap for %d\n", p->getPid());

   if (p->getState() != int_process::neonatal_intermediate)
      return true;
   
   bool all_bootstrapped = true;
   int_threadPool *tp = ev->getProcess()->llproc()->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      if (thr->getHandlerState() == int_thread::neonatal_intermediate) {
         pthrd_printf("Thread %d is not yet bootstrapped\n", thr->getLWP());
         all_bootstrapped = false;
         break;
      }
   }

   if (all_bootstrapped) {
      pthrd_printf("All threads are bootstrapped, marking process bootstrapped\n");
      p->setState(int_process::running);
   }

   return true;
}

HandleSignal::HandleSignal() :
   Handler(std::string("Signal"))
{
}

HandleSignal::~HandleSignal()
{
}

void HandleSignal::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Signal));
}

bool HandleSignal::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   
   EventSignal *sigev = static_cast<EventSignal *>(ev.get());
   thrd->setContSignal(sigev->getSignal());

   return true;
}

HandlePostExit::HandlePostExit() :
   Handler("Post Exit")
{
}

HandlePostExit::~HandlePostExit()
{
}

void HandlePostExit::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Exit));
}

bool HandlePostExit::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   assert(proc);
   assert(thrd);
   EventExit *event = static_cast<EventExit *>(ev.get());
   pthrd_printf("Handling post-exit for process %d on thread %d\n",
                proc->getPid(), thrd->getLWP());
   proc->setExitCode(event->getExitCode());
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);
   ProcPool()->rmThread(thrd);

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   }else{
      delete proc;
   }

   return true;
}

HandleCrash::HandleCrash() :
   Handler("Crash")
{
}

HandleCrash::~HandleCrash()
{
}

void HandleCrash::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Crash));
}

bool HandleCrash::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   assert(proc);
   assert(thrd);
   pthrd_printf("Handling crash for process %d on thread %d\n",
                proc->getPid(), thrd->getLWP());
   EventCrash *event = static_cast<EventCrash *>(ev.get());
   proc->setCrashSignal(event->getTermSignal());
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   delete proc;

   return true;
}

HandlePreExit::HandlePreExit() :
   Handler("Pre Exit")
{
}

HandlePreExit::~HandlePreExit()
{
}

void HandlePreExit::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Pre, EventType::Exit));
}

bool HandlePreExit::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling pre-exit for process %d on thread %d\n",
                ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());

   return true;
}

HandleThreadCreate::HandleThreadCreate() :
   Handler("Thread Create")
{
}

HandleThreadCreate::~HandleThreadCreate()
{
}

void HandleThreadCreate::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::ThreadCreate));
}

bool HandleThreadCreate::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   EventNewThread *threadev = static_cast<EventNewThread *>(ev.get());

   pthrd_printf("Handle thread create for %d/%d with new thread %d\n",
                proc->getPid(), thrd ? thrd->getLWP() : -1, threadev->getLWP());

   ProcPool()->condvar()->lock();
   
   int_thread *newthr = int_thread::createThread(proc, 0, threadev->getLWP(), false);
   //New threads start stopped, but inherit the user state of the creating
   // thread (which should be 'running').
   newthr->setGeneratorState(int_thread::stopped);
   newthr->setHandlerState(int_thread::stopped);
   newthr->setInternalState(thrd->getUserState());
   newthr->setUserState(thrd->getUserState());
   
   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   return true;
}

HandleThreadDestroy::HandleThreadDestroy() :
   Handler("Thread Destroy")
{
}

HandleThreadDestroy::~HandleThreadDestroy()
{
}

void HandleThreadDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Any, EventType::ThreadDestroy));
}

bool HandleThreadDestroy::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   if (ev->getEventType().time() == EventType::Pre) {
      pthrd_printf("Handling pre-thread destroy for %d\n", thrd->getLWP());
      return true;
   }

   pthrd_printf("Handling post-thread destroy for %d\n", thrd->getLWP());
   ProcPool()->condvar()->lock();

   thrd->setHandlerState(int_thread::exited);
   thrd->setInternalState(int_thread::exited);
   thrd->setUserState(int_thread::exited);
   ProcPool()->rmThread(thrd);
   proc->threadPool()->rmThread(thrd);

   delete thrd;

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();
   return true;
}

HandleThreadStop::HandleThreadStop() :
   Handler(std::string("Thread Stop"))
{
}

HandleThreadStop::~HandleThreadStop()
{
}

void HandleThreadStop::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Stop));
}

bool HandleThreadStop::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   pthrd_printf("Handling thread stop for %d/%d\n", proc->getPid(), thrd->getLWP());

   assert(thrd->hasPendingStop());
   thrd->setPendingStop(false);

   thrd->setInternalState(int_thread::stopped);
   if (thrd->hasPendingUserStop()) {
      thrd->setUserState(int_thread::stopped);
      thrd->setPendingUserStop(false);
   }

   return true;
}

HandlePostFork::HandlePostFork() :
   Handler("Post Fork")
{
}

HandlePostFork::~HandlePostFork()
{
}

void HandlePostFork::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Fork));
}

bool HandlePostFork::handleEvent(Event::ptr ev)
{
   EventFork *efork = static_cast<EventFork *>(ev.get());
   Dyninst::PID child_pid = efork->getPID();
   int_process *parent_proc = ev->getProcess()->llproc();
   pthrd_printf("Handling fork for parent %d to child %d\n",
                parent_proc->getPid(), child_pid);

   int_process *child_proc = int_process::createProcess(child_pid, parent_proc);
   assert(child_proc);
   return child_proc->forked();
}

HandlePostExec::HandlePostExec() :
   Handler("Post Exec")
{
}

HandlePostExec::~HandlePostExec()
{
}

void HandlePostExec::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Exec));
}

bool HandlePostExec::handleEvent(Event::ptr ev)
{
   EventExec *eexec = static_cast<EventExec *>(ev.get());
   int_process *proc = ev->getProcess()->llproc();
   pthrd_printf("Handling exec for process %d\n",
                proc->getPid());

   bool result = proc->execed();
   if (!result) 
      return false;
   
   eexec->setExecPath(proc->getExecutable());
   eexec->setThread(proc->threadPool()->initialThread()->thread());
   return true;
}

HandleSingleStep::HandleSingleStep() :
   Handler("Single Step")
{
}

HandleSingleStep::~HandleSingleStep()
{
}

void HandleSingleStep::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::SingleStep));
}

bool HandleSingleStep::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling event single step on %d/%d\n", 
                ev->getProcess()->llproc()->getPid(),
                ev->getThread()->llthrd()->getLWP());
   ev->getThread()->llthrd()->setUserState(int_thread::stopped);
   ev->getThread()->llthrd()->setInternalState(int_thread::stopped);
   return true;
}

HandleBreakpoint::HandleBreakpoint() :
   Handler("Breakpoint")
{
}

HandleBreakpoint::~HandleBreakpoint()
{
}

void HandleBreakpoint::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
}

bool HandleBreakpoint::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling breakpoint\n");
   int_process *proc = ev->getProcess()->llproc();

   EventBreakpoint *ebp = static_cast<EventBreakpoint *>(ev.get());
   std::vector<Breakpoint::ptr> hl_bps;
   ebp->getBreakpoints(hl_bps);
   bool has_user_breakpoints = !hl_bps.empty();

   if (has_user_breakpoints) 
   {
      int_threadPool *pool = proc->threadPool();
      for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
         (*i)->setUserState(int_thread::stopped);
      }
   }

   return true;
}

HandlePostBreakpoint::HandlePostBreakpoint() :
   Handler("Post Breakpoint")
{
}

HandlePostBreakpoint::~HandlePostBreakpoint()
{
}

void HandlePostBreakpoint::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
}

bool HandlePostBreakpoint::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   /**
    * TODO: Ctrl transfer breakpoints
    **/

   /**
    * Stop all other threads in the job while we remove the breakpoint, single step 
    * through the instruction and then resume the other threads
    **/
   pthrd_printf("Marking all threads in %d stopped for internal breakpoint handling\n",
                proc->getPid());
   int_threadPool *pool = proc->threadPool();
   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      if ((*i)->getInternalState() == int_thread::running)
         (*i)->setInternalState(int_thread::stopped);
   }

   EventBreakpoint *evbp = static_cast<EventBreakpoint *>(ev.get());
   installed_breakpoint *bp = evbp->installedbp();

   pthrd_printf("Setting breakpoint thread to single step mode\n");
   thrd->setSingleStepMode(true);
   thrd->markClearingBreakpoint(bp);

   pthrd_printf("Removing breakpoint from memory\n");
   bool result = bp->suspend(proc);
   assert(result);

   pthrd_printf("Restoring PC to original location location at %lx\n", bp->getAddr());
   MachRegister pcreg = MachRegister::getPC(proc->getTargetArch());
   result = thrd->setRegister(pcreg, (MachRegisterVal) bp->getAddr());
   assert(result);

   thrd->setInternalState(int_thread::running);

   return true;
}

HandleBreakpointClear::HandleBreakpointClear() :
   Handler("Breakpoint Clear")
{
}

HandleBreakpointClear::~HandleBreakpointClear()
{
}

void HandleBreakpointClear::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::BreakpointClear));
}

bool HandleBreakpointClear::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   EventBreakpointClear *bpc = static_cast<EventBreakpointClear *>(ev.get());
   installed_breakpoint *bp = bpc->bp();

   pthrd_printf("Resuming breakpoint at %lx\n", bp->getAddr());
   bp->resume(proc);
  
   pthrd_printf("Restoring process state\n");
   thrd->setSingleStepMode(false);
   thrd->markClearingBreakpoint(NULL);
   thrd->setInternalState(int_thread::stopped);

   proc->threadPool()->restoreInternalState(false);

   return true;
}

int HandlePostBreakpoint::getPriority() const
{
   return Handler::PostCallbackPriority;
}

HandleLibrary::HandleLibrary() :
   Handler("SysV Library Handler")
{
}

HandleLibrary::~HandleLibrary()
{
}

bool HandleLibrary::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling library load/unload\n");
   EventLibrary *lev = static_cast<EventLibrary *>(ev.get());

   int_process *proc = ev->getProcess()->llproc();
   set<int_library *> ll_added, ll_rmd;
   bool result = proc->refresh_libraries(ll_added, ll_rmd);
   if (!result) {
      pthrd_printf("Failed to refresh library list\n");
      return false;
   }
   if (ll_added.empty() && ll_rmd.empty()) {
      pthrd_printf("Could not find actual changes in lib state\n");
      return true;
   }

   set<Library::ptr> added, rmd;
   for (set<int_library*>::iterator i = ll_added.begin(); i != ll_added.end(); i++) {
      added.insert((*i)->getUpPtr());
   }
   for (set<int_library*>::iterator i = ll_rmd.begin(); i != ll_rmd.end(); i++) {
      rmd.insert((*i)->getUpPtr());
   }
   lev->setLibs(added, rmd);
   return true;
}

void HandleLibrary::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Library));
}

HandleCallbacks::HandleCallbacks() : 
   Handler("Callback")
{
}

HandleCallbacks::~HandleCallbacks()
{
}

HandleCallbacks *HandleCallbacks::getCB()
{
   static HandleCallbacks *cb = NULL;
   if (!cb) {
      cb = new HandleCallbacks();
      assert(cb);
      HandlerPool *hp = createDefaultHandlerPool();
      delete hp;
   }
   return cb;
}

int HandleCallbacks::getPriority() const
{
   return CallbackPriority;
}

void HandleCallbacks::getEventTypesHandled(std::vector<EventType> & /*etypes*/)
{
   //Callbacks are special cased, they respond to all event types.
}

bool HandleCallbacks::hasCBs(Event::const_ptr ev)
{
   return cbfuncs.find(ev->getEventType()) != cbfuncs.end();
}

bool HandleCallbacks::requiresCB(Event::const_ptr ev)
{
   return hasCBs(ev) && !ev->suppressCB();
}

bool HandleCallbacks::handleEvent(Event::ptr ev)
{
   EventType evtype = ev->getEventType();
   std::map<EventType, std::set<Process::cb_func_t>, eventtype_cmp>::iterator i = cbfuncs.find(evtype);
   if (i == cbfuncs.end()) {
      pthrd_printf("No callback registered for event type '%s'\n", ev->name().c_str());
      return true;
   }
   int_process *proc = ev->getProcess()->llproc();
   if (proc &&
       (proc->getState() == int_process::neonatal ||
        proc->getState() == int_process::neonatal_intermediate))
   {
      pthrd_printf("No callback for neonatal process %d\n", proc->getPid());
      return true;
   }
   const std::set<Process::cb_func_t> &cbs = i->second;

   return deliverCallback(ev, cbs);
}

static const char *action_str(Process::cb_action_t action)
{
   switch (action) {
      case Process::cbThreadContinue:
         return "cbThreadContinue";
      case Process::cbThreadStop:
         return "cbThreadStop";
      case Process::cbProcContinue:
         return "cbProcContinue";
      case Process::cbProcStop:
         return "cbProcStop";
      case Process::cbDefault:
         return "cbDefault";
      default:
         assert(0);
   }

   return NULL;
}

bool HandleCallbacks::handleCBReturn(Process::const_ptr proc, Thread::const_ptr thrd, 
                                     Process::cb_action_t ret)
{
   switch (ret) {
      case Process::cbThreadContinue:
         if (thrd == Thread::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned thread continue\n");
         thrd->llthrd()->setUserState(int_thread::running);
         thrd->llthrd()->setInternalState(int_thread::running);
         break;
      case Process::cbThreadStop:
         if (thrd == Thread::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned thread stop\n");
         thrd->llthrd()->setUserState(int_thread::stopped);
         thrd->llthrd()->setInternalState(int_thread::stopped);
         break;
      case Process::cbProcContinue: {
         if (proc == Process::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned process continue\n");
         int_threadPool *tp = proc->llproc()->threadPool();
         for (int_threadPool::iterator j = tp->begin(); j != tp->end(); j++) {
            (*j)->setUserState(int_thread::running);
            (*j)->setInternalState(int_thread::running);
         }
         break;
      }
      case Process::cbProcStop: {
         if (proc == Process::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned process stop\n");
         int_threadPool *tp = proc->llproc()->threadPool();
         for (int_threadPool::iterator j = tp->begin(); j != tp->end(); j++) {
            (*j)->setUserState(int_thread::stopped);
            (*j)->setInternalState(int_thread::stopped);
         }
         break;
      }
      case Process::cbDefault:
         pthrd_printf("Callbacks returned default\n");
         break;
   }
   return true;
}

bool HandleCallbacks::deliverCallback(Event::ptr ev, const set<Process::cb_func_t> &cbset)
{
   if (ev->suppressCB()) {
      pthrd_printf("Suppressing callbacks for event %s\n", ev->name().c_str());
      if (mt()->getThreadMode() == Process::HandlerThreading && 
          notify()->hasEvents()) 
      {
         notify()->clearEvent();
      }
      return true;
   }

   //The following code loops over each callback registered for this event type
   // and triggers the callback for the user.  Return results are aggregated.
   assert(!(isHandlerThread() && mt()->getThreadMode() != Process::CallbackThreading));
   unsigned k = 0;
   pthrd_printf("Triggering callback for event '%s'\n", ev->name().c_str());
   Process::cb_action_t parent_result = Process::cbDefault;
   Process::cb_action_t child_result = Process::cbDefault;
   std::set<Process::cb_func_t>::iterator j;
   for (j = cbset.begin(); j != cbset.end(); j++, k++) {
      pthrd_printf("Triggering callback #%u for event '%s'\n", k, ev->name().c_str());
      int_process::setInCB(true);
      Process::cb_ret_t ret = (*j)(ev);
      int_process::setInCB(false);

      if (ret.parent != Process::cbDefault)
         parent_result = ret.parent;
      if (ret.child != Process::cbDefault)
         child_result = ret.child;

      pthrd_printf("Callback #%u return %s/%s\n", k, action_str(ret.parent),
                   action_str(ret.child));
   }

   //Given the callback return result, change the user state to the appropriate
   // setting.
   pthrd_printf("Handling return value for main process\n");
   handleCBReturn(ev->getProcess(), ev->getThread(), parent_result);

   pthrd_printf("Handling return value for child process/thread\n");
   Process::const_ptr child_proc = Process::const_ptr();
   Thread::const_ptr child_thread = Thread::const_ptr();
   bool event_has_child = false;
   switch (ev->getEventType().code()) {
      case EventType::Fork:
         event_has_child = true;
         child_proc = static_cast<EventFork *>(ev.get())->getChildProcess();
         break;
      case EventType::ThreadCreate:
         event_has_child = true;
         child_thread = static_cast<EventNewThread *>(ev.get())->getNewThread();
         break;
   }
   if (event_has_child)
      handleCBReturn(child_proc, child_thread, child_result);

   if (mt()->getThreadMode() == Process::HandlerThreading && 
       notify()->hasEvents())
   {
      notify()->clearEvent();
   }

   return true;
}

bool HandleCallbacks::registerCallback_int(EventType ev, Process::cb_func_t func)
{
   pthrd_printf("Registering event %s with callback function %p\n", ev.name().c_str(), func);
   std::set<EventType>::iterator i = alleventtypes.find(ev);
   if (i == alleventtypes.end()) {
      pthrd_printf("Event %s does not have any handler\n", ev.name().c_str());
      return false;
   }
   cbfuncs[ev].insert(func);
   return true;
}

bool HandleCallbacks::registerCallback(EventType ev, Process::cb_func_t func)
{
   switch (ev.time()) {
      case EventType::Pre:
      case EventType::Post:
      case EventType::None: {
         bool result = registerCallback_int(ev, func);
         if (!result) {
            pthrd_printf("Did not register any callbacks for %s\n", ev.name().c_str());
            setLastError(err_noevents, "EventType does not exist");
            return false;
         }
         break;
      }
      case EventType::Any: {
         bool result1 = registerCallback_int(EventType(EventType::Pre, ev.code()), func);
         bool result2 = registerCallback_int(EventType(EventType::Post, ev.code()), func);
         bool result3 = registerCallback_int(EventType(EventType::None, ev.code()), func);
         if (!result1 && !result2 && !result3) {
            pthrd_printf("Did not register any callbacks for %s\n", ev.name().c_str());
            setLastError(err_noevents, "EventType does not exist");
            return false;
         }
         break;
      }
   }
   return true;
}

bool HandleCallbacks::removeCallback_int(EventType et, Process::cb_func_t func)
{
   cbfuncs_t::iterator i = cbfuncs.find(et);
   if (i == cbfuncs.end()) {
      return false;
   }
   set<Process::cb_func_t> &func_set = i->second;
   set<Process::cb_func_t>::iterator j = func_set.find(func);
   if (j == func_set.end()) {
      return false;
   }
   func_set.erase(j);
   return true;
}

bool HandleCallbacks::removeCallback(EventType et, Process::cb_func_t func)
{
   bool result = false;
   switch (et.time()) {
      case EventType::Pre:
      case EventType::Post:
      case EventType::None: {
         result = removeCallback_int(et, func);
      }
      case EventType::Any: {
         bool result1 = removeCallback_int(EventType(EventType::Pre, et.code()), func);
         bool result2 = removeCallback_int(EventType(EventType::Post,et.code()), func);
         bool result3 = removeCallback_int(EventType(EventType::None,et.code()), func);
         result = (result1 || result2 || result3);
      }
   }
   if (!result) {
      perr_printf("Attempted to remove non-existant callback %s\n", 
                  et.name().c_str());
      setLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

bool HandleCallbacks::removeCallback_int(EventType et)
{
   cbfuncs_t::iterator i = cbfuncs.find(et);
   if (i == cbfuncs.end()) {
      return false;
   }
   cbfuncs.erase(i);
   return true;
}

bool HandleCallbacks::removeCallback(EventType et)
{
   bool result = false;
   switch (et.time()) {
      case EventType::Pre:
      case EventType::Post:
      case EventType::None: {
         result = removeCallback_int(et);
      }
      case EventType::Any: {
         bool result1 = removeCallback_int(EventType(EventType::Pre, et.code()));
         bool result2 = removeCallback_int(EventType(EventType::Post,et.code()));
         bool result3 = removeCallback_int(EventType(EventType::None,et.code()));
         result = (result1 || result2 || result3);
      }
   }
   if (!result) {
      perr_printf("Attempted to remove non-existant callback %s\n", 
                  et.name().c_str());
      setLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

bool HandleCallbacks::removeCallback(Process::cb_func_t func)
{
   bool rmd_something = false;
   for (cbfuncs_t::iterator i = cbfuncs.begin(); i != cbfuncs.end(); i++)
   {
      EventType et = i->first;
      bool result = removeCallback_int(et, func);
      if (result) 
         rmd_something = true;
   }
   if (!rmd_something) {
      perr_printf("Attempted to remove non-existant callback %p\n", func);
      setLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

HandlerPool *createDefaultHandlerPool()
{
   static bool initialized = false;
   static HandleBootstrap *hbootstrap = NULL;
   static HandleSignal *hsignal = NULL;
   static HandlePostExit *hpostexit = NULL;
   static HandlePreExit *hpreexit = NULL;
   static HandleThreadCreate *hthreadcreate = NULL;
   static HandleThreadDestroy *hthreaddestroy = NULL;
   static HandleThreadStop *hthreadstop = NULL;
   static HandleSingleStep *hsinglestep = NULL;
   static HandleCrash *hcrash = NULL;
   static HandleBreakpoint *hbpoint = NULL;
   static HandlePostBreakpoint *hpost_bpoint = NULL;
   static HandleBreakpointClear *hbpclear = NULL;
   static HandleLibrary *hlibrary = NULL;
   static HandlePostFork *hpostfork = NULL;
   static HandlePostExec *hpostexec = NULL;
   static iRPCHandler *hrpc = NULL;
   if (!initialized) {
      hbootstrap = new HandleBootstrap();
      hsignal = new HandleSignal();
      hpostexit = new HandlePostExit();
      hpreexit = new HandlePreExit();
      hthreadcreate = new HandleThreadCreate();
      hthreaddestroy = new HandleThreadDestroy();
      hthreadstop = new HandleThreadStop();
      hsinglestep = new HandleSingleStep();
      hcrash = new HandleCrash();
      hbpoint = new HandleBreakpoint();
      hpost_bpoint = new HandlePostBreakpoint();
      hbpclear = new HandleBreakpointClear();
      hrpc = new iRPCHandler();
      hlibrary = new HandleLibrary();
      hpostfork = new HandlePostFork();
      hpostexec = new HandlePostExec();
      initialized = true;
   }
   HandlerPool *hpool = new HandlerPool();
   hpool->addHandler(hbootstrap);
   hpool->addHandler(hsignal);
   hpool->addHandler(hpostexit);
   hpool->addHandler(hpreexit);
   hpool->addHandler(hthreadcreate);
   hpool->addHandler(hthreaddestroy);
   hpool->addHandler(hthreadstop);
   hpool->addHandler(hsinglestep);
   hpool->addHandler(hcrash);
   hpool->addHandler(hbpoint);
   hpool->addHandler(hpost_bpoint);
   hpool->addHandler(hbpclear);
   hpool->addHandler(hrpc);
   hpool->addHandler(hlibrary);
   hpool->addHandler(hpostfork);
   hpool->addHandler(hpostexec);
   plat_createDefaultHandlerPool(hpool);
   return hpool;
}

