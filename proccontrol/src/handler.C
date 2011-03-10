#include "proccontrol/h/Handler.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/response.h"
#include "proccontrol/src/int_event.h"
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

HandlerPool::HandlerPool(int_process *p) :
   proc(p)
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
      //Async events go first
      if (a.first->getEventType().code() != EventType::Async ||
          a.first->getEventType().code() != EventType::Async)
      {
         if (a.first->getEventType().code() == EventType::Async)
            return true;
         if (b.first->getEventType().code() == EventType::Async)
            return false;
      }

      //Others are run via handler priority
      if (a.second->getPriority() != b.second->getPriority())
         return a.second->getPriority() < b.second->getPriority();

      //Hard-coded rule.  UserThreadDestroy always comes before LWPDestroy
      if (a.first->getEventType().code() == EventType::LWPDestroy &&
          b.first->getEventType().code() == EventType::UserThreadDestroy)
         return false;
      if (b.first->getEventType().code() == EventType::LWPDestroy &&
          a.first->getEventType().code() == EventType::UserThreadDestroy)
         return true;
      
      //Subservient events run latter in handler tie
      if (a.first->subservientTo().lock() == b.first)
         return false;
      if (b.first->subservientTo().lock() == a.first)
         return true;

      //Events are equal in order--just choose a consistent ordering at this point
      eventtype_cmp cmp;
      if (cmp(a.first->getEventType(), b.first->getEventType()))
         return true;
      if (cmp(b.first->getEventType(), a.first->getEventType()))
         return false;
      return a.first < b.first;
   }
};

Event::ptr HandlerPool::curEvent() const
{
   return cur_event;
}

Event::ptr HandlerPool::getRealParent(Event::ptr ev) const
{
   Event::ptr master_ev = ev;
   for (;;) {
      Event::ptr parent = master_ev->subservientTo().lock();
      if (!parent || parent->getEventType().code() == EventType::Async)
         break;
      master_ev = parent;
   }
   return master_ev;
}

void HandlerPool::notifyOfPendingAsyncs(const std::set<response::ptr> &asyncs, Event::ptr ev)
{
   Event::ptr master_ev = getRealParent(ev);
      
   for (set<response::ptr>::iterator i = asyncs.begin(); i != asyncs.end(); i++) {
      (*i)->setEvent(master_ev);
   }
}

void HandlerPool::notifyOfPendingAsyncs(response::ptr async, Event::ptr ev)
{
   async->setEvent(getRealParent(ev));
}

bool HandlerPool::isEventAsyncPostponed(Event::ptr ev) const
{
   return (pending_async_events.find(ev) != pending_async_events.end());
}

bool HandlerPool::hasAsyncEvent() const
{
   return !pending_async_events.empty();
}

void HandlerPool::markEventAsyncPending(Event::ptr ev)
{
   set<Event::ptr>::iterator i = pending_async_events.find(ev);
   if (i != pending_async_events.end()) {
      pthrd_printf("Async event %s on %d/%d has already been marked, leaving alone.\n",
                   ev->name().c_str(), ev->getProcess()->llproc()->getPid(), 
                   ev->getThread()->llthrd()->getLWP());
      return;
   }
   bool was_empty = pending_async_events.empty();  
   pending_async_events.insert(ev);
   pthrd_printf("Event %s async marked.  pending_async_size = %d, (was_empty = %s)\n", 
                ev->name().c_str(),
                (int) pending_async_events.size(),
                was_empty ? "true" : "false");
   for (set<Event::ptr>::iterator j = pending_async_events.begin(); j != pending_async_events.end(); j++) {
      pthrd_printf("\tEvent %s (%p)\n", (*j)->name().c_str(), (*j).get());
   }

   if (was_empty) {
      markProcAsyncPending(this);
   }

   pthrd_printf("Async event %s on %d/%d, moving process/threads to stopped\n",
                ev->name().c_str(), ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());
   switch (ev->getSyncType()) {
      case Event::unset:
      case Event::async:
         break;
      case Event::sync_thread: {
         int_thread *thr = ev->getThread()->llthrd();
         thr->desyncInternalState();
         //Soundn't trigger block--thread should already be handler stopped
         thr->intStop(false);
         break;
      }
      case Event::sync_process: {
         int_threadPool *tp = ev->getProcess()->llproc()->threadPool();
         tp->desyncInternalState();
         //Soundn't trigger block--proc should already be handler stopped
         tp->intStop(false);
         break;
      }
   }
}

void HandlerPool::clearEventAsync(Event::ptr ev)
{
   set<Event::ptr>::iterator i = pending_async_events.find(ev);
   if (i == pending_async_events.end())
      return;
   pthrd_printf("Erasing event %s (%p) from list\n", ev->name().c_str(), ev.get());
   pending_async_events.erase(i);
   pthrd_printf("pending_async_size = %d, (is_empty = %s):\n", (int) pending_async_events.size(),
                pending_async_events.empty() ? "true" : "false");
   for (set<Event::ptr>::iterator j = pending_async_events.begin(); j != pending_async_events.end(); j++) {
      pthrd_printf("\tEvent %s (%p)\n", (*j)->name().c_str(), (*j).get());
   }

   if (pending_async_events.empty()) {
      clearProcAsyncPending(this);
   }

   pthrd_printf("Async event %s on %d/%d is complete, restoring\n",
                ev->name().c_str(), ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());   
   switch (ev->getSyncType()) {
      case Event::unset:
      case Event::async:
         break;
      case Event::sync_thread: {
         int_thread *thr = ev->getThread()->llthrd();
         thr->restoreInternalState(false);
         break;
      }
      case Event::sync_process: {
         int_threadPool *tp = ev->getProcess()->llproc()->threadPool();
         tp->restoreInternalState(false);
         break;
      }
   }
}

void HandlerPool::addEventToSet(Event::ptr ev, set<Event::ptr> &ev_set) const
{
   ev_set.insert(ev);
   for (vector<Event::ptr>::iterator i = ev->subservient_events.begin(); 
        i != ev->subservient_events.end(); i++)
   {
      addEventToSet(*i, ev_set);
   }
}


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
   addEventToSet(ev, all_events);

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
         if (ev->handled_by.find(hnd) != ev->handled_by.end()) {
            pthrd_printf("Event %s has already been handled by %s\n",
                         ev->name().c_str(), hnd->getName().c_str());
            continue;
         }
         events_and_handlers.insert(pair<Event::ptr, Handler*>(ev, hnd));
      }
   }

   /**
    * We should have all events and handlers properly sorted into the
    * events_and_handlers set in the order we want to run them in.
    * Now it's finally time to run the handlers.
    **/
   bool handled_something = false;
   bool had_error = true;

   ev_hndler_set_t::iterator i;
   for (i = events_and_handlers.begin(); i != events_and_handlers.end(); i++)
   {
      handled_something = true;
      Event::ptr event = i->first;
      Handler *handler = i->second;
      EventType etype = event->getEventType();

      if (handler->getPriority() != Handler::CallbackPriority) {
         //We don't want the callback handler getting async events
         // from user operations in the callback.
         cur_event = getRealParent(event);
      }

      pthrd_printf("Handling event '%s' with handler '%s'\n", etype.name().c_str(), 
                   handler->getName().c_str());
      Handler::handler_ret_t result = handler->handleEvent(event);

      cur_event = Event::ptr();
      if (result == Handler::ret_async) {
         pthrd_printf("Handler %s did not complete due to pending asyncs\n",
                      handler->getName().c_str());
         markEventAsyncPending(event);
         return true;
      }
      event->handled_by.insert(handler);
      if (result == Handler::ret_error) {
         pthrd_printf("Error handling event %s with %s\n", etype.name().c_str(), 
                      handler->getName().c_str());
         had_error = true;
      }
   }

   for (set<Event::ptr>::iterator i = all_events.begin(); i != all_events.end(); i++)
   {
      Event::ptr event = *i;
      clearEventAsync(event); //nop if ev wasn't async
   }

   return !had_error && handled_something;
}

std::set<HandlerPool *> HandlerPool::procsAsyncPending;
Mutex HandlerPool::asyncPendingLock;

void HandlerPool::markProcAsyncPending(HandlerPool *p)
{
   asyncPendingLock.lock();
   assert(procsAsyncPending.find(p) == procsAsyncPending.end());
   procsAsyncPending.insert(p);
   asyncPendingLock.unlock();
}

void HandlerPool::clearProcAsyncPending(HandlerPool *p)
{
   asyncPendingLock.lock();
   std::set<HandlerPool *>::iterator i = procsAsyncPending.find(p);
   assert(i != procsAsyncPending.end());
   procsAsyncPending.erase(i);
   asyncPendingLock.unlock();
}

bool HandlerPool::hasProcAsyncPending()
{
   asyncPendingLock.lock();
   bool result = !procsAsyncPending.empty();
   if (result) {
      pthrd_printf("Pending async proc: %d\n", (*procsAsyncPending.begin())->proc->getPid());
   }
   asyncPendingLock.unlock();
   return result;
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

Handler::handler_ret_t HandleBootstrap::handleEvent(Event::ptr ev)
{
   int_process *p = ev->getProcess()->llproc();

   assert(p);
   pthrd_printf("Handling bootstrap for %d\n", p->getPid());

   if (p->getState() != int_process::neonatal_intermediate)
      return ret_success;
   
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

   return ret_success;
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

Handler::handler_ret_t HandleSignal::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   
   EventSignal *sigev = static_cast<EventSignal *>(ev.get());
   thrd->setContSignal(sigev->getSignal());

   return ret_success;
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

Handler::handler_ret_t HandlePostExit::handleEvent(Event::ptr ev)
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

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   } else {
      delete proc;
   }

   return ret_success;
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

Handler::handler_ret_t HandleCrash::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   assert(proc);
   assert(thrd);
   pthrd_printf("Handling crash for process %d on thread %d\n",
                proc->getPid(), thrd->getLWP());
   EventCrash *event = static_cast<EventCrash *>(ev.get());

   if (proc->wasForcedTerminated()) {
      pthrd_printf("Crash was due to process::terminate, not reporting\n");
      event->setSuppressCB(true);
   }
   else {
      proc->setCrashSignal(event->getTermSignal());
   }
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   } else {
      delete proc;
   }

   return ret_success;
}

HandleForceTerminate::HandleForceTerminate() :
    Handler("ForceTerminate")
{
}

HandleForceTerminate::~HandleForceTerminate()
{
}

void HandleForceTerminate::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Post, EventType::ForceTerminate));
}

Handler::handler_ret_t HandleForceTerminate::handleEvent(Event::ptr ev) {
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   assert(proc);
   assert(thrd);
   pthrd_printf("Handling force terminate for process %d on thread %d\n",
                proc->getPid(), thrd->getLWP());
   EventForceTerminate *event = static_cast<EventForceTerminate *>(ev.get());

   proc->setCrashSignal(event->getTermSignal());
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   } else {
      delete proc;
   }

   return ret_success;
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

Handler::handler_ret_t HandlePreExit::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thread = ev->getThread()->llthrd();
   pthrd_printf("Handling pre-exit for process %d on thread %d\n",
                proc->getPid(), thread->getLWP());

   if (proc->wasForcedTerminated()) {
      //Linux sometimes throws an extraneous exit after
      // calling ptrace(PTRACE_KILL, ...).  It's not a real exit
      pthrd_printf("Proc pre-exit was due to process::terminate, not reporting\n");
      ev->setSuppressCB(true);
      if (thread->getInternalState() == int_thread::stopped)
      {
         thread->desyncInternalState();
         thread->setInternalState(int_thread::running);
      }
   }
   thread->setExiting(true);

   // If there is a pending stop, need to handle it here because there is
   // no guarantee that the stop will ever be received
   if( thread->hasPendingStop() ) {
       thread->setInternalState(int_thread::stopped);
       if (thread->hasPendingUserStop()) {
          thread->setUserState(int_thread::stopped);
       }
   }

   return ret_success;
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
   etypes.push_back(EventType(EventType::None, EventType::UserThreadCreate));
   etypes.push_back(EventType(EventType::None, EventType::LWPCreate));
}

Handler::handler_ret_t HandleThreadCreate::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   EventNewThread *threadev = static_cast<EventNewThread *>(ev.get());

   pthrd_printf("Handle thread create for %d/%d with new thread %d\n",
                proc->getPid(), thrd ? thrd->getLWP() : -1, threadev->getLWP());

   if (ev->getEventType().code() == EventType::UserThreadCreate)  {
      //If we support both user and LWP thread creation, and we're doing a user
      // creation, then the Thread object may already exist.  Do nothing.
      int_thread *thr = proc->threadPool()->findThreadByLWP(threadev->getLWP());
      if (thr) {
         pthrd_printf("Thread object already exists, ThreadCreate handler doing nothing\n");
         return ret_success;
      }
   }
   ProcPool()->condvar()->lock();
   
   int_thread *newthr = int_thread::createThread(proc, NULL_THR_ID, threadev->getLWP(), false);
   newthr->setGeneratorState(int_thread::stopped);
   newthr->setHandlerState(int_thread::stopped);

   if( proc->hasQueuedProcStoppers() ) {
       // The following ordering of problems causes problems: 
       // Breakpoint LWPCreate Stop
       //
       // Breakpoints require a whole process stop. However, an LWPCreate (this
       // event) is interleaved between the Breakpoint and any stops required
       // to handle the Breakpoint. Thus, the new thread isn't put into a
       // stopped state because the parent thread isn't in a stopped state when
       // this event is processed. 
       //
       // Make the necessary changes due to the postponed Breakpoint event
       newthr->desyncInternalState();
       newthr->setInternalState(int_thread::stopped);
   }else{
       //New threads start stopped, but inherit the user state of the creating
       // thread (which should be 'running').
       newthr->setInternalState(thrd->getUserState());
   }
   newthr->setUserState(thrd->getUserState());
   
   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   return ret_success;
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
   etypes.push_back(EventType(EventType::Any, EventType::UserThreadDestroy));
   etypes.push_back(EventType(EventType::Any, EventType::LWPDestroy));
}

Handler::handler_ret_t HandleThreadDestroy::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   if (ev->getEventType().time() == EventType::Pre) {
      pthrd_printf("Handling pre-thread destroy for %d\n", thrd->getLWP());
      return ret_success;
   }

   if (ev->getEventType().code() == EventType::UserThreadCreate &&
       proc->plat_supportLWPEvents()) 
   {
      //This is a user thread delete, but we still have an upcoming LWP 
      // delete.  Don't actually do anything yet.
      pthrd_printf("Handling user thread... postponing clean of %d/%d until LWP delete\n", 
                   proc->getPid(), thrd->getLWP());
      return ret_success;
   }

   pthrd_printf("Handling post-thread destroy for %d\n", thrd->getLWP());
   ProcPool()->condvar()->lock();

   if (proc->wasForcedTerminated()) {
      //Linux sometimes throws an extraneous thread terminate after
      // calling ptrace(PTRACE_KILL, ...).  It's not a real thread terminate.
      pthrd_printf("Thread terminate was due to process::terminate, not reporting\n");
      ev->setSuppressCB(true);
   }

   thrd->setHandlerState(int_thread::exited);
   thrd->setInternalState(int_thread::exited);
   thrd->setUserState(int_thread::exited);
   ProcPool()->rmThread(thrd);
   proc->threadPool()->rmThread(thrd);

   delete thrd;

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();
   return ret_success;
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

Handler::handler_ret_t HandleThreadStop::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();

   switch (proc->plat_getThreadControlMode())
   {
      case int_process::HybridLWPControl:
      case int_process::IndependentLWPControl: 
      {
         int_thread *thrd = ev->getThread()->llthrd();
         pthrd_printf("Handling thread stop for %d/%d\n", proc->getPid(), thrd->getLWP());
         assert(thrd->hasPendingStop());
         thrd->setPendingStop(false);
         
         thrd->setInternalState(int_thread::stopped);
         if (thrd->hasPendingUserStop()) {
            thrd->setUserState(int_thread::stopped);
            thrd->setPendingUserStop(false);
         }
         break;
      }
      case int_process::NoLWPControl: 
      {
         pthrd_printf("Handling process stop for %d\n", proc->getPid());
         for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++) {
            int_thread *thrd = *i;
            pthrd_printf("Handling process stop for %d/%d\n", proc->getPid(), thrd->getLWP());
            assert(thrd->hasPendingStop());
            thrd->setPendingStop(false);

            thrd->setInternalState(int_thread::stopped);
            if (thrd->hasPendingUserStop()) {
               assert(thrd->hasPendingUserStop());
               thrd->setUserState(int_thread::stopped);
               thrd->setPendingUserStop(false);
            }
         }
         break;
      }
   }

   return ret_success;
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

Handler::handler_ret_t HandlePostFork::handleEvent(Event::ptr ev)
{
   EventFork *efork = static_cast<EventFork *>(ev.get());
   Dyninst::PID child_pid = efork->getPID();
   int_process *parent_proc = ev->getProcess()->llproc();
   pthrd_printf("Handling fork for parent %d to child %d\n",
                parent_proc->getPid(), child_pid);

   int_process *child_proc = int_process::createProcess(child_pid, parent_proc);
   assert(child_proc);
   return child_proc->forked() ? ret_success : ret_error;
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

Handler::handler_ret_t HandlePostExec::handleEvent(Event::ptr ev)
{
   EventExec *eexec = static_cast<EventExec *>(ev.get());
   int_process *proc = ev->getProcess()->llproc();
   pthrd_printf("Handling exec for process %d\n",
                proc->getPid());

   bool result = proc->execed();
   if (!result) 
      return ret_error;
   
   eexec->setExecPath(proc->getExecutable());
   eexec->setThread(proc->threadPool()->initialThread()->thread());
   return ret_success;
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

Handler::handler_ret_t HandleSingleStep::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling event single step on %d/%d\n", 
                ev->getProcess()->llproc()->getPid(),
                ev->getThread()->llthrd()->getLWP());
   return ret_success;
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

Handler::handler_ret_t HandleBreakpoint::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling breakpoint\n");
   int_process *proc = ev->getProcess()->llproc();

   EventBreakpoint *ebp = static_cast<EventBreakpoint *>(ev.get());
   std::vector<Breakpoint::const_ptr> hl_bps;
   ebp->getBreakpoints(hl_bps);
   bool has_user_breakpoints = !hl_bps.empty();

   if (has_user_breakpoints) 
   {
      int_threadPool *pool = proc->threadPool();
      for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
         (*i)->setUserState(int_thread::stopped);
      }
   }

   return ret_success;
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

Handler::handler_ret_t HandlePostBreakpoint::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   EventBreakpoint *evbp = static_cast<EventBreakpoint *>(ev.get());
   installed_breakpoint *bp = evbp->installedbp();
   int_eventBreakpoint *ibp = evbp->getInternal();

   Breakpoint::const_ptr ctrlTransferBrkpt = Breakpoint::ptr();
   std::vector<Breakpoint::const_ptr> hl_bps;
   evbp->getBreakpoints(hl_bps);

   for(std::vector<Breakpoint::const_ptr>::iterator i = hl_bps.begin();
           i != hl_bps.end(); ++i)
   {
       if( (*i)->isCtrlTransfer() ) {
           ctrlTransferBrkpt = *i;
           break;
       }
   }

   /**
    * Control transfer breakpoints
    *
    * Just change the PC of the thread that hit the breakpoint
    **/
   if( ctrlTransferBrkpt != Breakpoint::const_ptr() ) {
       pthrd_printf("Handling control transfer breakpoint on thread %d/%d\n",
               proc->getPid(), thrd->getLWP());

       if( !ibp->pc_regset ) {
           ibp->pc_regset = result_response::createResultResponse();
           pthrd_printf("Setting PC to control transfer target at 0x%lx\n",
                   ctrlTransferBrkpt->getToAddress());
           MachRegister pcreg = MachRegister::getPC(proc->getTargetArch());
           bool result = thrd->setRegister(pcreg, 
                   (MachRegisterVal) ctrlTransferBrkpt->getToAddress(),
                   ibp->pc_regset);
           assert(result);
       }

       assert( ibp->pc_regset );

       if( ibp->pc_regset->isPosted() && !ibp->pc_regset->isReady() ) {
           pthrd_printf("Suspending breakpoint handling for control transfer pc set\n");
           proc->handlerPool()->notifyOfPendingAsyncs(ibp->pc_regset, ev);
           return ret_async;
       }

       if (evbp->procStopper())
          proc->threadPool()->restoreInternalState(false);

       return ret_success;
   }

   /**
    * Normal breakpoints
    *
    * Stop all other threads in the job while we remove the breakpoint, single step 
    * through the instruction and then resume the other threads
    **/
   pthrd_printf("Marking all threads in %d stopped for internal breakpoint handling\n",
                proc->getPid());
   int_threadPool *pool = proc->threadPool();
   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      if ((*i)->getInternalState() == int_thread::running && (*i)->getHandlerState() == int_thread::stopped)
         (*i)->setInternalState(int_thread::stopped);
   }
   
   if (!ibp->set_singlestep) {
      pthrd_printf("Setting breakpoint thread to single step mode\n");
      thrd->setSingleStepMode(true);
      thrd->markClearingBreakpoint(bp);
      ibp->set_singlestep = true;
   }

   if (!ibp->memwrite_bp_suspend) {
      pthrd_printf("Removing breakpoint from memory\n");
      ibp->memwrite_bp_suspend = result_response::createResultResponse();
      bool result = bp->suspend(proc, ibp->memwrite_bp_suspend);
      assert(result);
   }

   if (proc->plat_breakpointAdvancesPC() && !ibp->pc_regset) 
   {
      ibp->pc_regset = result_response::createResultResponse();
      pthrd_printf("Restoring PC to original location at %lx\n",
                   bp->getAddr());
      MachRegister pcreg = MachRegister::getPC(proc->getTargetArch());
      bool result = thrd->setRegister(pcreg, (MachRegisterVal) bp->getAddr(), 
                                      ibp->pc_regset);
      assert(result);
   }

   bool needs_async_ret = false;
   assert(ibp->memwrite_bp_suspend);
   if (ibp->memwrite_bp_suspend->isPosted() && !ibp->memwrite_bp_suspend->isReady()) {
      pthrd_printf("Suspending breakpoint handling for memory write\n");
      proc->handlerPool()->notifyOfPendingAsyncs(ibp->memwrite_bp_suspend, ev);
      needs_async_ret = true;
   }

   if (proc->plat_breakpointAdvancesPC()) {
      assert(ibp->pc_regset);
      if (ibp->pc_regset->isPosted() && !ibp->pc_regset->isReady()) {
         pthrd_printf("Suspending breakpoint handling for pc set\n");
         proc->handlerPool()->notifyOfPendingAsyncs(ibp->pc_regset, ev);
         needs_async_ret = true;      
      }
   }

   if (needs_async_ret) {
      return ret_async;
   }

   if (ibp->memwrite_bp_suspend->hasError()) {
      pthrd_printf("Error suspending breakpoint\n");
      return ret_error;
   }

   if (proc->plat_breakpointAdvancesPC() && ibp->pc_regset->hasError()) {
      pthrd_printf("Error setting PC register\n");
      return ret_error;
   }

   thrd->setInternalState(int_thread::running);

   return ret_success;
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

Handler::handler_ret_t HandleBreakpointClear::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   EventBreakpointClear *bpc = static_cast<EventBreakpointClear *>(ev.get());
   installed_breakpoint *bp = bpc->bp();
   int_eventBreakpointClear *int_bpc = bpc->getInternal();

   pthrd_printf("Resuming breakpoint at %lx\n", bp->getAddr());
   bool result;

   if (!int_bpc->memwrite_bp_resume) {
      int_bpc->memwrite_bp_resume = result_response::createResultResponse();
      result = bp->resume(proc, int_bpc->memwrite_bp_resume);
      if (!result) {
         pthrd_printf("Error resuming breakpoint in handler\n");
         return ret_error;
      }
   }

   assert(int_bpc->memwrite_bp_resume);
   if (int_bpc->memwrite_bp_resume->isPosted() && !int_bpc->memwrite_bp_resume->isReady()) {
      pthrd_printf("Postponing breakpoint clear while waiting for memwrite\n");
      proc->handlerPool()->notifyOfPendingAsyncs(int_bpc->memwrite_bp_resume, ev);
      return ret_async;
   }

   if (int_bpc->memwrite_bp_resume->hasError()) {
      pthrd_printf("Error resuming breakpoint\n");
      return ret_error;
   }

   pthrd_printf("Restoring process state\n");
   thrd->setSingleStepMode(false);
   thrd->markClearingBreakpoint(NULL);
   thrd->setInternalState(int_thread::stopped);

   if (ev->getSyncType() != Event::sync_process && bp->hasNonCtrlTransfer()) {
      //Not all breakpoints are proc stoppers.  This test needs to match a
      // similar one in EventBreakpoint::procStopper
       proc->threadPool()->restoreInternalState(false);
   }

   return ret_success;
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

Handler::handler_ret_t HandleLibrary::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling library load/unload\n");
   EventLibrary *lev = static_cast<EventLibrary *>(ev.get());

   int_process *proc = ev->getProcess()->llproc();
   set<int_library *> ll_added, ll_rmd;
   set<response::ptr> async_responses;
   bool result = proc->refresh_libraries(ll_added, ll_rmd, async_responses);
   if (!result && !async_responses.empty()) {
      proc->handlerPool()->notifyOfPendingAsyncs(async_responses, ev);
      return ret_async;
   }
   if (!result) {
      pthrd_printf("Failed to refresh library list\n");
      return ret_error;
   }
   if (ll_added.empty() && ll_rmd.empty()) {
      pthrd_printf("Could not find actual changes in lib state\n");
      return ret_success;
   }

   set<Library::ptr> added, rmd;
   for (set<int_library*>::iterator i = ll_added.begin(); i != ll_added.end(); i++) {
      added.insert((*i)->getUpPtr());
   }
   for (set<int_library*>::iterator i = ll_rmd.begin(); i != ll_rmd.end(); i++) {
      rmd.insert((*i)->getUpPtr());
   }
   for (set<int_library*>::iterator i = ll_rmd.begin(); i != ll_rmd.end(); i++) {
      (*i)->markAsCleanable();
   }
   lev->setLibs(added, rmd);
   return ret_success;
}

void HandleLibrary::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Library));
}

HandleRPCInternal::HandleRPCInternal() :
   Handler("RPC Internal")
{
}

HandleRPCInternal::~HandleRPCInternal()
{
}
   
Handler::handler_ret_t HandleRPCInternal::handleEvent(Event::ptr)
{
   return ret_success;
}

void HandleRPCInternal::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::RPCInternal));
}

HandleDetached::HandleDetached() :
   Handler("Detached")
{
}

HandleDetached::~HandleDetached()
{
}

Handler::handler_ret_t HandleDetached::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   pthrd_printf("Handle process detached\n");

   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();
   return ret_success;
}

void HandleDetached::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Detached));
}


HandleAsync::HandleAsync() :
   Handler("Async")
{
}

HandleAsync::~HandleAsync()
{
}
   
Handler::handler_ret_t HandleAsync::handleEvent(Event::ptr ev)
{
   EventAsync::ptr eAsync = ev->getEventAsync();
   response::ptr resp = eAsync->getInternal()->getResponse();
   pthrd_printf("Handling Async event for %s on %d/%d\n", resp->name().c_str(),
                eAsync->getProcess()->llproc()->getPid(),
                eAsync->getThread()->llthrd()->getLWP());

   assert(eAsync->getProcess()->llproc()->plat_needsAsyncIO());
   resp->markReady();
   resp->setEvent(Event::ptr());
   return ret_success;
}

void HandleAsync::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Async));
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
      HandlerPool *hp = createDefaultHandlerPool(NULL);
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

Handler::handler_ret_t HandleCallbacks::handleEvent(Event::ptr ev)
{
   EventType evtype = ev->getEventType();
   std::map<EventType, std::set<Process::cb_func_t>, eventtype_cmp>::iterator i = cbfuncs.find(evtype);
   if (i == cbfuncs.end()) {
      pthrd_printf("No callback registered for event type '%s'\n", ev->name().c_str());
      return ret_success;
   }
   int_process *proc = ev->getProcess()->llproc();
   if (proc &&
       (proc->getState() == int_process::neonatal ||
        proc->getState() == int_process::neonatal_intermediate))
   {
      pthrd_printf("No callback for neonatal process %d\n", proc->getPid());
      return ret_success;
   }
   const std::set<Process::cb_func_t> &cbs = i->second;

   return deliverCallback(ev, cbs) ? ret_success : ret_error;
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
         return "cbInvalid";
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

            if( (*j)->getHandlerState() == int_thread::running ) {
                // sync cannot be set here or it will result in recursive event
                // handling -- waitAndHandleEvents will take care of flushing out
                // the stop
                if( !(*j)->intStop(false) ) {
                    // Silent for now
                    perr_printf("Failed to issue stop to handle user CB return\n");
                }
            }else{
                (*j)->setInternalState(int_thread::stopped);
            }
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

   // Make sure the user can operate on the process in the callback
   // But if this is a PostCrash or PostExit, the underlying process and
   // threads are already gone and thus no operations on them really make sense
   int_thread::State savedUserState = int_thread::neonatal;
   if( !ev->getProcess()->isTerminated() && ev->getThread()->llthrd() != NULL ) {
       savedUserState = ev->getThread()->llthrd()->getUserState();
       ev->getThread()->llthrd()->setUserState(int_thread::stopped);
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

   // Now that the callback is over, return the state to what it was before the
   // callback so the return value from the callback can be used to update the state
   if( !ev->getProcess()->isTerminated() && ev->getThread()->llthrd() != NULL ) {
      ev->getThread()->llthrd()->setUserState(savedUserState);

      //Given the callback return result, change the user state to the appropriate
      // setting.
      pthrd_printf("Handling return value for main process\n");
      handleCBReturn(ev->getProcess(), ev->getThread(), parent_result);
   }

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
      case EventType::UserThreadCreate:
      case EventType::LWPCreate:
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

void HandleCallbacks::getRealEvents(EventType ev, std::vector<EventType> &out_evs)
{
   switch (ev.code()) {
      case EventType::Terminate:
         out_evs.push_back(EventType(ev.time(), EventType::Exit));
         out_evs.push_back(EventType(ev.time(), EventType::Crash));
         break;
      case EventType::ThreadCreate:
         out_evs.push_back(EventType(ev.time(), EventType::UserThreadCreate));
         out_evs.push_back(EventType(ev.time(), EventType::LWPCreate));
         break;
      case EventType::ThreadDestroy:
         out_evs.push_back(EventType(ev.time(), EventType::UserThreadDestroy));
         out_evs.push_back(EventType(ev.time(), EventType::LWPDestroy));
         break;
      default:
         out_evs.push_back(ev);
   }
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

bool HandleCallbacks::registerCallback(EventType oev, Process::cb_func_t func)
{
   bool registered_cb = false;
   std::vector<EventType> real_evs;
   getRealEvents(oev, real_evs);
   
   for (std::vector<EventType>::iterator i = real_evs.begin(); i != real_evs.end(); i++)
   {
      EventType ev = *i;
      switch (ev.time()) {
         case EventType::Pre:
         case EventType::Post:
         case EventType::None: {
            bool result = registerCallback_int(ev, func);
            if (result)
               registered_cb = true;
            break;
         }
         case EventType::Any: {
            bool result1 = registerCallback_int(EventType(EventType::Pre, ev.code()), func);
            bool result2 = registerCallback_int(EventType(EventType::Post, ev.code()), func);
            bool result3 = registerCallback_int(EventType(EventType::None, ev.code()), func);
            if (result1 || result2 || result3)
               registered_cb = true;
            break;
         }
      }
   }
   if (!registered_cb) {
      pthrd_printf("Did not register any callbacks for %s\n", oev.name().c_str());
      setLastError(err_noevents, "EventType does not exist");
      return false;
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

bool HandleCallbacks::removeCallback(EventType oet, Process::cb_func_t func)
{
   bool removed_cb = false;
   std::vector<EventType> real_ets;
   getRealEvents(oet, real_ets);
   
   for (std::vector<EventType>::iterator i = real_ets.begin(); i != real_ets.end(); i++)
   {
      EventType et = *i;

      switch (et.time()) {
         case EventType::Pre:
         case EventType::Post:
         case EventType::None: {
            bool result = removeCallback_int(et, func);
            if (result)
               removed_cb = true;
         }
         case EventType::Any: {
            bool result1 = removeCallback_int(EventType(EventType::Pre, et.code()), func);
            bool result2 = removeCallback_int(EventType(EventType::Post,et.code()), func);
            bool result3 = removeCallback_int(EventType(EventType::None,et.code()), func);
            if (result1 || result2 || result3)
               removed_cb = true;
         }
      }
   }

   if (!removed_cb) {
      perr_printf("Attempted to remove non-existant callback %s\n", 
                  oet.name().c_str());
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

HandlerPool *createDefaultHandlerPool(int_process *p)
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
   static HandleRPCInternal *hrpcinternal = NULL;
   static HandleAsync *hasync = NULL;
   static HandleForceTerminate *hforceterm = NULL;
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
      hasync = new HandleAsync();
      hrpcinternal = new HandleRPCInternal();
      hforceterm = new HandleForceTerminate();
      initialized = true;
   }
   HandlerPool *hpool = new HandlerPool(p);
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
   hpool->addHandler(hrpcinternal);
   hpool->addHandler(hasync);
   hpool->addHandler(hforceterm);
   plat_createDefaultHandlerPool(hpool);
   return hpool;
}

