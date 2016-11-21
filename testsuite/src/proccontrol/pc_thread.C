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

#include "proccontrol_comp.h"
#include "communication.h"

#include <set>
#include <utility>

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

class pc_threadMutator : public ProcControlMutator {
public:
   virtual test_results_t executeTest();
   virtual test_results_t pre_init(ParameterDict &param);
   void registerCB(EventType et, Process::cb_func_t f);
};

extern "C" DLLEXPORT TestMutator* pc_thread_factory()
{
   return new pc_threadMutator();
}

static bool has_lwp = false;
static bool has_thr = false;
static bool has_stack_info = true;
static bool has_initial_func_info = true;
static bool is_attach = false;
static bool has_error = false;
static int user_cb_count = 0;
static int lwp_cb_count = 0;
static int user_exit_cb_count = 0;
static int lwp_exit_cb_count = 0;

static set<pair<PID, THR_ID> > all_tids;
static set<pair<PID, LWP> > all_lwps;
static set<pair<PID, THR_ID> > pre_dead_tids;
static set<pair<PID, THR_ID> > post_dead_tids;
static set<pair<PID, LWP> > pre_dead_lwps;
static set<pair<PID, LWP> > post_dead_lwps;
static set<pair<PID, Address> > all_stack_addrs;
static set<pair<PID, Address> > all_tls;
static set<PID> all_initial_threads;
static set<Process::const_ptr> exited_processes;
static set<PID> all_known_processes;

void pc_threadMutator::registerCB(EventType et, Process::cb_func_t f)
{
   bool result = Process::registerEventCallback(et, f);
   if (!result) {
      logerror("Error registering thread callback\n");
      has_error = true;
   }
}

static Process::cb_ret_t proc_exit(Event::const_ptr ev)
{
   if (all_known_processes.find(ev->getProcess()->getPid()) == 
       all_known_processes.end())
      exited_processes.insert(ev->getProcess());
   return Process::cbDefault;
}



static Process::cb_ret_t handle_new_thread(Thread::const_ptr thr)
{
	if(!thr) {
		return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
	}
   user_cb_count++;

   if (!thr->haveUserThreadInfo()) {
      logerror("Error.  Thread does not have thread info after thread create callback\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }

   PID pid = thr->getProcess()->getPid();
   LWP lwp = thr->getLWP();
   THR_ID tid = thr->getTID();
   if (tid == NULL_THR_ID) {
      logerror("Error.  Thread does not have tid after new event\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }
   
   if (all_tids.find(pair<PID, THR_ID>(pid, tid)) != all_tids.end()) {
      logerror("Error. Recieved duplicate callback, or threads share a tid value\n");
      has_error = true;
   }
   all_tids.insert(pair<PID, THR_ID>(pid, tid));

   // In create mode, we won't be able to simulate an lwp create event for the
   // initial thread before this callback occurs so ignore the initial thread
   // -- this is valid because the initial lwp should never generate a callback
   // and thus, we don't need to check that it occurred
   if (lwp_cb_count && !thr->isInitialThread() && all_lwps.find(pair<PID, LWP>(pid, lwp)) == all_lwps.end()) {
      logerror("Error. LWPs supported, but no LWP callback before UserThread callback\n");
      has_error = true;
   }
   
   Dyninst::Address start_func = thr->getStartFunction();
   if (has_initial_func_info && !start_func && !thr->isInitialThread()) {
      logerror("Error.  Thread has no start function\n");
      has_error = true;
   }

   Dyninst::Address stack_addr = thr->getStackBase();
   if (has_stack_info && !stack_addr && !thr->isInitialThread()) {
      logerror("Error.  Thread has no stack\n");
      has_error = true;
   }
   if (has_stack_info && all_stack_addrs.find(pair<PID, Address>(pid, stack_addr)) != all_stack_addrs.end()) {
      logerror("Error.  Threads have duplicate stack addresses\n");
      has_error = true;
   }
   all_stack_addrs.insert(pair<PID, Address>(pid, stack_addr));
   
   unsigned long stack_size = thr->getStackSize();
   if (has_stack_info && !stack_size && !thr->isInitialThread()) {
      logerror("Error.  Stack has no size\n");
      has_error = true;
   }

   Dyninst::Address tls_addr = thr->getTLS();
   if (!tls_addr) {
      logerror("Error.  Thread has no TLS\n");
      has_error = true;
   }
   if (all_tls.find(pair<PID, Address>(pid, tls_addr)) != all_tls.end()) {
      logerror("Error.  Threads have duplicate TLS\n");
      has_error = true;
   }
   all_tls.insert(pair<PID, Address>(pid, tls_addr));
   
   logstatus("[User Create] %d/%d: TID - 0x%lx, Start Func - 0x%lx, Stack Base - 0x%lx, Stack Size = 0x%lu, TLS = 0x%lx\n",
           (int) pid, (int) lwp, (unsigned long) tid, (unsigned long) start_func, (unsigned long) stack_addr, (unsigned long) stack_size, (unsigned long) tls_addr);
   
   return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
}

static Process::cb_ret_t uthr_create(Event::const_ptr ev)
{
   EventNewUserThread::const_ptr tev = ev->getEventNewUserThread();
   if (!tev) {
      logerror("Error.  Improper event type passed to callback\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }

   Thread::const_ptr thr = tev->getNewThread();
   return handle_new_thread(thr);
}

static Process::cb_ret_t uthr_destroy(Event::const_ptr ev)
{
   if( ev->getEventType().time() == EventType::Pre ) user_exit_cb_count++;

   EventUserThreadDestroy::const_ptr tev = ev->getEventUserThreadDestroy();
   if (!tev) {
      logerror("Error.  Improper event type passed to callback\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }
   Thread::const_ptr thr = tev->getThread();

   PID pid = thr->getProcess()->getPid();
   LWP lwp = thr->getLWP();
   THR_ID tid = thr->getTID();
   
   if (all_tids.find(pair<PID, THR_ID>(pid, tid)) == all_tids.end())
   {
      logerror("Thread destroy on unknown thread\n");
      has_error = true;
   }

   const char *pstr = NULL;
   if (ev->getEventType().time() == EventType::Pre)
   {
      if (pre_dead_tids.find(pair<PID, THR_ID>(pid, tid)) != pre_dead_tids.end()) {
         logerror("User Thread pre-died twice\n");
         has_error = true;
      }
      pre_dead_tids.insert(pair<PID, THR_ID>(pid, tid));
      pstr = "Pre-";
   }
   else if (ev->getEventType().time() == EventType::Post)
   {
      if (post_dead_tids.find(pair<PID, THR_ID>(pid, tid)) != post_dead_tids.end()) {
         logerror("User Thread post-died twice\n");
         has_error = true;
      }
      post_dead_tids.insert(pair<PID, THR_ID>(pid, tid));
      pstr = "Post-";
   }

   logstatus("[%sUser Delete] %d/%d: TID - 0x%lx\n", pstr, pid, lwp, tid);

   return Process::cbDefault;
}

static Process::cb_ret_t handle_lwp_create(Thread::const_ptr thr)
{
   lwp_cb_count++;

   PID pid = thr->getProcess()->getPid();
   LWP lwp = thr->getLWP();
   if (all_lwps.find(pair<PID, LWP>(pid, lwp)) != all_lwps.end()) {
      logerror("Error.  Duplicate LWP values\n");
      has_error = true;
   }
   all_lwps.insert(pair<PID, LWP>(pid, lwp));

   ThreadPool::const_iterator i = thr->getProcess()->threads().find(lwp);
   if (i == thr->getProcess()->threads().end() || *i != thr) {
      logerror("Threadpool does not contain thread\n");
      has_error = true;
   }

   if (!thr->isLive()) {
      logerror("Thread is not live after create\n");
      has_error = true;
   }

   bool prev_initial_thread = (all_initial_threads.find(pid) != all_initial_threads.end());
   bool is_initial_thread = thr->isInitialThread();
   if (prev_initial_thread && is_initial_thread) {
      logerror("Multiple initial threads\n");
      has_error = true;
   }
   if (is_initial_thread) {
      if (thr->getProcess()->threads().getInitialThread() != thr) {
         logerror("Disagreement with threadpool over initial thread value\n");
         has_error = true;
      }
      all_initial_threads.insert(pid);
   }
   
   logstatus("[LWP Create] - %d/%d, initial: %s\n", pid, lwp, is_initial_thread ? "true" : "false");
   return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
}

static Process::cb_ret_t lwp_create(Event::const_ptr ev)
{
   EventNewLWP::const_ptr lev = ev->getEventNewLWP();
   if (!lev) {
      logerror("Error.  Improper event type passed to callback\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }
   Thread::const_ptr thr = lev->getNewThread();
   return handle_lwp_create(thr);
}

static Process::cb_ret_t lwp_destroy(Event::const_ptr ev)
{
   if (ev->getEventType().time() == EventType::Post)
      lwp_exit_cb_count++;

   EventLWPDestroy::const_ptr tev = ev->getEventLWPDestroy();
   if (!tev) {
      logerror("Error.  Improper event type passed to callback\n");
      has_error = true;
      return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
   }
   Thread::const_ptr thr = tev->getThread();

   PID pid = thr->getProcess()->getPid();
   LWP lwp = thr->getLWP();

   const char *pstr = NULL;
   if (ev->getEventType().time() == EventType::Pre)
   {
      if (pre_dead_lwps.find(pair<PID, LWP>(pid, lwp)) != pre_dead_lwps.end()) {
         logerror("LWP pre-died twice\n");
         has_error = true;
      }
      pre_dead_lwps.insert(pair<PID, LWP>(pid, lwp));
      pstr = "Pre-";
   }
   else if (ev->getEventType().time() == EventType::Post)
   {
      if (post_dead_lwps.find(pair<PID, LWP>(pid, lwp)) != post_dead_lwps.end()) {
         logerror("LWP post-died twice\n");
         has_error = true;
      }
      post_dead_lwps.insert(pair<PID, LWP>(pid, lwp));
      pstr = "Post-";
   }

   if (all_lwps.find(pair<PID, LWP>(pid, lwp)) == all_lwps.end())
   {
      logerror("Dead LWP for unknown LWP\n");
      has_error = true;
   }

   logstatus("[%sLWP Delete] %d/%d\n", pstr ? pstr : "", pid, lwp);

   return Process::cbDefault;
}

static void checkThreadMsg(threadinfo tinfo, Process::ptr proc)
{
#if !defined(os_bg_test)
   if (tinfo.pid != proc->getPid()) {
      logerror("Error.  Mismatched pids in checkThreadMsg\n", tinfo.pid, proc->getPid());
      has_error = true;
   }
#endif

   ThreadPool::iterator i = proc->threads().find(tinfo.lwp);
   if (i == proc->threads().end()) {
      logerror("Error.  Could not find LWP in checkThreadMsg\n");
      has_error = true;
   }
   Thread::ptr thr = *i;

   if (has_thr && thr && thr->getTID() != (Dyninst::THR_ID)(-1)) {
      if (thr->getTID() != (Dyninst::THR_ID) tinfo.tid) {
         logerror("Error.  Mismatched TID, %lx != %lx\n", (unsigned long) thr->getTID(), (unsigned long) tinfo.tid);
         has_error = true;
      }
      Dyninst::Address a_stack_addr = (Dyninst::Address) tinfo.a_stack_addr;
      if (has_stack_info && (thr->getStackBase() < a_stack_addr || thr->getStackBase() + thr->getStackSize() > a_stack_addr)) 
      {
         logerror("Error.  Mismatched stack addresses, base = 0x%lx, size = %lx, loc = 0x%lx\n", 
                  (unsigned long) thr->getStackBase(), (unsigned long) thr->getStackSize(), 
                  (unsigned long) a_stack_addr);
         has_error = true;
      }
      if (has_initial_func_info && thr->getStartFunction() != (Dyninst::Address) tinfo.initial_func) {
         logerror("Mismatched initial function (%lx != %lx)\n", (unsigned long)thr->getStartFunction(),
                 (unsigned long)tinfo.initial_func);
         has_error = true;
      }
#if !defined(os_windows_test)
      Dyninst::Address tls_addr = (Dyninst::Address) tinfo.tls_addr;
#define VALID_TLS_RANGE (1024*1024)
      if (tls_addr < thr->getTLS() - VALID_TLS_RANGE || tls_addr > thr->getTLS() + VALID_TLS_RANGE) {
         logerror("Error.  Invalid TLS address, pc: %lx\tmut: %lx\n", (unsigned long) thr->getTLS(), (unsigned long) tls_addr);
         has_error = true;
      }
#endif
   }
}

test_results_t pc_threadMutator::pre_init(ParameterDict &param)
{
   has_error = false;
   user_cb_count = 0;
   lwp_cb_count = 0;
   user_exit_cb_count = 0;
   lwp_exit_cb_count = 0;

   all_tids.clear();
   all_lwps.clear();
   all_stack_addrs.clear();
   all_tls.clear();
   all_initial_threads.clear();
   pre_dead_tids.clear();
   post_dead_tids.clear();
   pre_dead_lwps.clear();
   post_dead_lwps.clear();
   exited_processes.clear();

#if defined(os_linux_test) && !defined(os_bg_test)
   has_lwp = true;
   has_thr = true;
   has_stack_info = false;
#elif defined(os_freebsd_test)
   has_lwp = false;
   has_thr = true;
   has_stack_info = false;
   has_initial_func_info = false;
#elif defined(os_bg_test)
   has_lwp = false;
   has_thr = true;
   has_stack_info = false;
#elif defined(os_windows_test)
	has_lwp = false;
	has_thr = true;
	has_initial_func_info = false;
    has_stack_info = false;
#else
#error Unknown platform
#endif

   registerCB(EventType::UserThreadCreate, uthr_create);
   registerCB(EventType::UserThreadDestroy, uthr_destroy);
   registerCB(EventType::LWPCreate, lwp_create);
   registerCB(EventType::LWPDestroy, lwp_destroy);
   registerCB(EventType::Terminate, proc_exit);

   is_attach = (create_mode_t) param["createmode"]->getInt() == USEATTACH;
   if (has_error) return FAILED;
   return PASSED;
}

test_results_t pc_threadMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      all_known_processes.insert(proc->getPid());
      if (!proc->supportsUserThreadEvents()) {
         logerror("System does not support user thread events\n");
         return SKIPPED;
      }
   }
   
   if (is_attach) {
      //If attaching then we missed all of the thread callbacks.  Run them.
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         ThreadPool::iterator j;
       
         if (has_lwp)
         {
            handle_lwp_create(proc->threads().getInitialThread());
            for (j = proc->threads().begin(); j != proc->threads().end(); j++) {
               if ((*j)->isInitialThread())
                  continue;
               handle_lwp_create(*j);
            }
         }
         if (has_thr)
         {
            handle_new_thread(proc->threads().getInitialThread());
            for (j = proc->threads().begin(); j != proc->threads().end(); j++) {
               if ((*j)->isInitialThread())
                  continue;
               handle_new_thread(*j);
            }
         }
      }
   }
   else {
      //We never get thread create callbacks for the initial thread (by design).  Fake it to setup
      // some of the test data structures.
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         Thread::ptr init_thr = proc->threads().getInitialThread();
         handle_lwp_create(init_thr);
      }
   }

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         has_error = true;
      }
   }   

   int num_thrds = comp->num_processes * comp->num_threads;
   int num_noninit_thrds = comp->num_processes * (comp->num_threads - 1);

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      for (unsigned j=0; j < comp->num_threads+1; j++) {
         threadinfo tinfo;
         bool result = comp->recv_message((unsigned char *) &tinfo, sizeof(threadinfo), *i);
         if (!result) {
            logerror("Failed to recieve threadinfo message\n");
            has_error = true;
         }
         checkThreadMsg(tinfo, *i);
      }
   }

   syncloc loc;
   loc.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      has_error = true;
   }   


   while ((has_thr && user_exit_cb_count < num_noninit_thrds) ||
          (has_lwp && lwp_exit_cb_count < num_noninit_thrds)) 
   {
      if (exited_processes.size() >= comp->num_processes) {
#if !defined(os_bg_test)
         logerror("Process exited while waiting for thread termination events.\n");
         has_error = true;
#endif
         break;
      }
      bool result = comp->block_for_events();
      if (!result) {
         logerror("Failed to wait for thread terminate events\n");
         has_error = true;
         break;
      }
   }
   while (comp->poll_for_events());

   return has_error ? FAILED : PASSED;
}


