#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/procpool.h"

#include "common/h/dthread.h"

#include <assert.h>

using namespace std;

std::set<Generator::gen_cb_func_t> Generator::CBs;
Mutex *Generator::cb_lock;

Generator::Generator(std::string name_) :
   name(name_)
{
   if (!cb_lock) cb_lock = new Mutex();
}

Generator::~Generator()
{
   setState(exiting);
}

void Generator::registerNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex();
   Generator::cb_lock->lock();
   CBs.insert(func);
   Generator::cb_lock->unlock();
}

void Generator::removeNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex();
   Generator::cb_lock->lock();
   std::set<gen_cb_func_t>::iterator i = CBs.find(func);
   if (i != CBs.end())
      CBs.erase(i);
   Generator::cb_lock->unlock();
}

bool Generator::isExitingState()
{
   return (state == error || state == exiting);
}

void Generator::setState(Generator::state_t new_state)
{
   if (isExitingState())
      return;
   state = new_state;
}

bool Generator::getAndQueueEventInt(bool block)
{
   bool result = true;
   ArchEvent *arch_event = NULL;
   vector<Event::ptr> events;

   setState(process_blocked);
   result = processWait(block);
   if (isExitingState()) {
      pthrd_printf("Generator exiting after processWait\n");
      result = false;
      goto done;
   }
   if (!result) {
      goto done;
   }
   
   setState(system_blocked);
   arch_event = getEvent(block);
   if (isExitingState()) {
      pthrd_printf("Generator exiting after getEvent\n");
      result = false;
      goto done;
   }
   if (!arch_event) {
      pthrd_printf("Error. Unable to recieve event\n");
      result = false;
      goto done;
   }

   setState(decoding);
   ProcPool()->condvar()->lock();
   for (decoder_set_t::iterator i = decoders.begin(); i != decoders.end(); i++) {
      Decoder *decoder = *i;
      bool result = decoder->decode(arch_event, events);
      if (!result)
         break;
   }
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); i++) {
      Event::ptr event = *i;
      event->getProcess()->llproc()->updateSyncState(event, true);
   }
   ProcPool()->condvar()->unlock();

   setState(queueing);
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); i++) {
      mbox()->enqueue(*i);
      Generator::cb_lock->lock();
      for (set<gen_cb_func_t>::iterator j = CBs.begin(); j != CBs.end(); j++) {
         (*j)();
      }
      Generator::cb_lock->unlock(); 
   }

   result = true;
 done:
   setState(none);
   return result;
}

static bool allStopped(int_process *proc, void *)
{
   bool all_exited = true;
   int_threadPool *tp = proc->threadPool();
   assert(tp);
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      if ((*i)->getGeneratorState() == int_thread::running ||
          (*i)->getGeneratorState() == int_thread::neonatal_intermediate) 
      {
         pthrd_printf("Found running thread: %d/%d is %s\n", 
                      proc->getPid(), (*i)->getLWP(), 
                      int_thread::stateStr((*i)->getGeneratorState()));
         return false;
      }
      if ((*i)->getGeneratorState() != int_thread::exited) {
         all_exited = false;
      }
   }
   if (all_exited) {
      pthrd_printf("All threads are exited for %d, treating as stopped\n",
                   proc->getPid());
      return true;
   }
   if (proc->forceGeneratorBlock()) {
      pthrd_printf("Process %d is forcing generator input\n", proc->getPid());
      return false;
   }
   pthrd_printf("Checking for running process: %d is stopped\n", proc->getPid());
   return true;
}

bool Generator::hasLiveProc()
{
   ProcessPool *procpool = ProcPool();
   return !procpool->for_each(allStopped, NULL);
}

struct GeneratorMTInternals
{
   GeneratorMTInternals() {}

   //Start-up synchronization
   CondVar init_cond;

   DThread thrd;
};

static void start_generator(void *g)
{
   GeneratorMT *gen = (GeneratorMT *) g;
   gen->start();
}

GeneratorMT::GeneratorMT(std::string name_) :
   Generator(name_)
{
   //Make sure these structures exist before any generators run.
   mbox();
   ProcPool();

   sync = new GeneratorMTInternals();
}

void GeneratorMT::launch()
{
   sync->init_cond.lock();
   state = initializing;
   sync->thrd.spawn(start_generator, this);
   while (state == initializing)
      sync->init_cond.wait();
   sync->init_cond.unlock();

   if (state == error) {
      pthrd_printf("Error creating generator\n");
   }
}

GeneratorMT::~GeneratorMT()
{
   setState(exiting);
   sync->thrd.join();
   delete sync;
   sync = NULL;
}

void GeneratorMT::start() 
{
   setGeneratorThread(DThread::self());
   pthrd_printf("Generator started on thread %lx\n", DThread::self());
   bool result;

   sync->init_cond.lock();
   result = initialize();
   if (!result) {
      pthrd_printf("Error initializing Generator\n");
      setState(error);
   }
   else {
      setState(none);
   }
   sync->init_cond.signal();
   sync->init_cond.unlock();

   if (result)
      main();
   pthrd_printf("Generator thread exiting\n");
}

void GeneratorMT::main()
{
   while (!isExitingState()) {
      bool result = getAndQueueEventInt(true);
      if (!result) {
         pthrd_printf("Error return in getAndQueueEventInt\n");
      }
   }
}

bool GeneratorMT::processWait(bool block)
{
   ProcessPool *pp = ProcPool();
   pp->condvar()->lock();
   pthrd_printf("Checking for live processes\n");
   while (!hasLiveProc()) {
      pthrd_printf("Checked and found no live processes\n");
      if (!block) {
         pthrd_printf("Returning from non-blocking processWait\n");
         return false;
      }
      pp->condvar()->wait();
   }
   pp->condvar()->unlock();
   return true;
}

bool GeneratorMT::getAndQueueEvent(bool)
{
   //Doesn't really make sense to be calling this for a MT
   // generator--part of the point is that you don't have
   // to call it.
   return true;
}
