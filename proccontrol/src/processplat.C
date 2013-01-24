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

#include "proccontrol/h/PlatFeatures.h"
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/procpool.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

bool int_libraryTracking::default_track_libs = true;

LibraryTracking::LibraryTracking(Process::ptr proc_) :
   proc(proc_)
{
}

LibraryTracking::~LibraryTracking()
{
   proc = Process::weak_ptr();
}

void LibraryTracking::setDefaultTrackLibraries(bool b)
{
   MTLock lock_this_func(MTLock::allow_init);
   int_libraryTracking::default_track_libs = b;
}

bool LibraryTracking::getDefaultTrackLibraries()
{
   MTLock lock_this_func(MTLock::allow_init);
   return int_libraryTracking::default_track_libs;
}

bool LibraryTracking::setTrackLibraries(bool b) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "setTrackLibraries", false);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(p);
   return pset->getLibraryTracking()->setTrackLibraries(b);
}

bool LibraryTracking::getTrackLibraries() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getTrackLibraries", false);
   int_libraryTracking *llproc = p->llproc()->getLibraryTracking();
   assert(llproc);
   return llproc->isTrackingLibraries();
}

bool LibraryTracking::refreshLibraries()
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "refreshLibraries", false);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(p);
   return pset->getLibraryTracking()->refreshLibraries();
}

bool ThreadTracking::default_track_threads = true;

ThreadTracking::ThreadTracking(Process::ptr proc_) :
   proc(proc_)
{
}

ThreadTracking::~ThreadTracking()
{
   proc = Process::weak_ptr();
}

void ThreadTracking::setDefaultTrackThreads(bool b) 
{
   MTLock lock_this_func(MTLock::allow_init);
   default_track_threads = b;
}

bool ThreadTracking::getDefaultTrackThreads()
{
   MTLock lock_this_func(MTLock::allow_init);
   return default_track_threads;
}

bool ThreadTracking::setTrackThreads(bool b) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   assert(p);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(p);
   return pset->getThreadTracking()->setTrackThreads(b);
}

bool ThreadTracking::getTrackThreads() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   assert(p);
   int_threadTracking *llproc = p->llproc()->getThreadTracking();
   assert(llproc);
   return llproc->isTrackingThreads();
}

bool ThreadTracking::refreshThreads()
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   assert(p);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(p);
   return pset->getThreadTracking()->refreshThreads();
}

bool LWPTracking::default_track_lwps = true;
LWPTracking::LWPTracking(Process::ptr proc_) :
   proc(proc_)
{
}

LWPTracking::~LWPTracking()
{
   proc = Process::weak_ptr();
}

void LWPTracking::setDefaultTrackLWPs(bool b)
{
   MTLock lock_this_func(MTLock::allow_init);
   default_track_lwps = b;
}

bool LWPTracking::getDefaultTrackLWPs()
{
   MTLock lock_this_func(MTLock::allow_init);
   return default_track_lwps;
}

void LWPTracking::setTrackLWPs(bool b) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   if (!p || !p->llproc()) {
      perr_printf("setTrackLWPs attempted on exited process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return;
   }
   int_LWPTracking *llproc = p->llproc()->getLWPTracking();;   
   llproc->lwp_setTracking(b);
}

bool LWPTracking::getTrackLWPs() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getTrackLWPs", false);
   int_LWPTracking *llproc = p->llproc()->getLWPTracking();;   
   return llproc->lwp_getTracking();
}

bool LWPTracking::refreshLWPs()
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "refreshLWPs", false);
   int_LWPTracking *llproc = p->llproc()->getLWPTracking();;   
   return llproc->lwp_refresh();
}

FollowFork::FollowFork(Process::ptr proc_) :
   proc(proc_)
{
}

FollowFork::~FollowFork()
{
   proc = Process::weak_ptr();
}

FollowFork::follow_t FollowFork::default_should_follow_fork = FollowFork::Follow;

void FollowFork::setDefaultFollowFork(FollowFork::follow_t f) {
   MTLock lock_this_func(MTLock::allow_init);
   default_should_follow_fork = f;
}

FollowFork::follow_t FollowFork::getDefaultFollowFork()
{
   MTLock lock_this_func(MTLock::allow_init);
   return default_should_follow_fork;
}

bool FollowFork::setFollowFork(FollowFork::follow_t f) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "setFollowFork", false);
   int_followFork *llproc = p->llproc()->getFollowFork();
   return llproc->fork_setTracking(f);
}

FollowFork::follow_t FollowFork::getFollowFork() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "setFollowFork", None);
   int_followFork *llproc = p->llproc()->getFollowFork();
   return llproc->fork_isTracking();
}

CallStackUnwinding::CallStackUnwinding(Thread::ptr t) :
   wt(t)
{
}

CallStackUnwinding::~CallStackUnwinding()
{
}

bool CallStackUnwinding::walkStack(CallStackCallback *stk_cb) const
{
   MTLock lock_this_func;
   Thread::ptr thr = wt.lock();
   if (!thr) {
      perr_printf("CallStackUnwinding called on exited thread\n");
      globalSetLastError(err_exited, "Thread is exited\n");
      return false;
   }
   ThreadSet::ptr thrset = ThreadSet::newThreadSet(thr);
   return thrset->getCallStackUnwinding()->walkStack(stk_cb);
}

CallStackCallback::CallStackCallback() :
   top_first(top_first_default_value)
{
}

CallStackCallback::~CallStackCallback()
{
}

string MultiToolControl::default_tool_name("pcontrl");
MultiToolControl::priority_t MultiToolControl::default_tool_priority = 99;

MultiToolControl::MultiToolControl(Process::ptr p) :
   proc(p)
{
}

MultiToolControl::~MultiToolControl()
{
   proc = Process::weak_ptr();
}

void MultiToolControl::setDefaultToolName(string name) 
{
   MTLock lock_this_func(MTLock::allow_init);
   default_tool_name = name;
}

void MultiToolControl::setDefaultToolPriority(MultiToolControl::priority_t p)
{
   MTLock lock_this_func(MTLock::allow_init);
   default_tool_priority = p;
}

string MultiToolControl::getDefaultToolName()
{
   MTLock lock_this_func(MTLock::allow_init);
   return default_tool_name;
}

MultiToolControl::priority_t MultiToolControl::getDefaultToolPriority()
{
   MTLock lock_this_func(MTLock::allow_init);
   return default_tool_priority;
}

std::string MultiToolControl::getToolName() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getToolName", string());
   int_multiToolControl *llproc = p->llproc()->getMultiToolControl();
   return llproc->mtool_getName();
}

MultiToolControl::priority_t MultiToolControl::getToolPriority() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getToolPriority", 0);
   int_multiToolControl *llproc = p->llproc()->getMultiToolControl();
   return llproc->mtool_getPriority();
}

dyn_sigset_t SignalMask::default_sigset;
bool SignalMask::sigset_initialized = false;

SignalMask::SignalMask(Process::ptr proc_) :
   proc(proc_)
{
}

SignalMask::~SignalMask()
{
}

dyn_sigset_t SignalMask::getSigMask() const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getSigMask", SignalMask::default_sigset);
   int_signalMask *llproc = p->llproc()->getSignalMask();
   return llproc->getSigMask();
}

bool SignalMask::setSigMask(dyn_sigset_t s)
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getSigMask", false);
   int_signalMask *llproc = p->llproc()->getSignalMask();
   llproc->setSigMask(s);
   return true;
}

dyn_sigset_t SignalMask::getDefaultSigMask()
{
   if (!sigset_initialized) {
#if !defined(os_windows)
      sigfillset(&default_sigset);
#endif
      sigset_initialized = true;
   }
   return default_sigset;
}

void SignalMask::setDefaultSigMask(dyn_sigset_t s)
{
   sigset_initialized = true;
   default_sigset = s;
}

int_libraryTracking::int_libraryTracking(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                                         std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_libraryTracking::int_libraryTracking(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   up_ptr(NULL)
{
}

int_libraryTracking::~int_libraryTracking()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_LWPTracking::int_LWPTracking(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                 std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   lwp_tracking(LWPTracking::default_track_lwps),
   up_ptr(NULL)
{
}

int_LWPTracking::int_LWPTracking(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   lwp_tracking(p->getLWPTracking()->lwp_tracking),
   up_ptr(NULL)
{
}

int_LWPTracking::~int_LWPTracking()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

bool int_LWPTracking::lwp_setTracking(bool b)
{
   pthrd_printf("Changing lwp tracking in %d from %s to %s\n", getPid(),
                lwp_tracking ? "true" : "false", b ? "true" : "false");
   if (b == lwp_tracking)
      return true;
   lwp_tracking = b;
   return plat_lwpChangeTracking(b);
}

bool int_LWPTracking::plat_lwpChangeTracking(bool)
{
   return true;
}

bool int_LWPTracking::lwp_getTracking()
{
   return lwp_tracking;
}

bool int_LWPTracking::lwp_refresh()
{
   pthrd_printf("Refreshing LWPs in process %d\n", getPid());
   result_response::ptr resp;
   bool result = lwp_refreshPost(resp);
   if (!result) {
      pthrd_printf("Error from lwp_refreshPost\n");
      return false;
   }
   if (resp) {
      int_process::waitForAsyncEvent(resp);
   }
   bool change;
   result = lwp_refreshCheck(change);
   if (!result) {
      pthrd_printf("Failed to check for new LWPs");
      return false;
   }
   
   if (!change)
      return true;
   
   setForceGeneratorBlock(true);
   ProcPool()->condvar()->lock();
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
   int_process::waitAndHandleEvents(false);
   setForceGeneratorBlock(false);
   return true;
}

bool int_LWPTracking::plat_lwpRefresh(result_response::ptr)
{
   return false;
}

bool int_LWPTracking::lwp_refreshPost(result_response::ptr &resp)
{
   if (!plat_needsAsyncIO()) {
      resp = result_response::ptr();
      return true;
   }
   
   resp = result_response::createResultResponse();
   resp->setProcess(this);
   resp->markSyncHandled();
   
   getResponses().lock();
   bool result = plat_lwpRefresh(resp);
   if (result) {
      getResponses().addResponse(resp, this);
   }
   if (!result) {
      resp = result_response::ptr();
   }
   getResponses().unlock();
   getResponses().noteResponse();
   
   return true;
}

bool int_LWPTracking::lwp_refreshCheck(bool &change)
{
   vector<Dyninst::LWP> lwps;
   change = false;
   bool result = getThreadLWPs(lwps);
   if (!result) {
      pthrd_printf("Error calling getThreadLWPs during refresh\n");
      return false;
   }
   
   //Look for added LWPs
   int_threadPool *pool = threadPool();
   int new_lwps_found = 0;
   for (vector<Dyninst::LWP>::iterator i = lwps.begin(); i != lwps.end(); i++) {
      Dyninst::LWP lwp = *i;
      int_thread *thr = pool->findThreadByLWP(*i);
      if (thr)
         continue;
      pthrd_printf("Found new thread %d/%d during refresh\n", getPid(), lwp);
      thr = int_thread::createThread(this, NULL_THR_ID, *i, false, int_thread::as_needs_attach);
      new_lwps_found++;
      change = true;
      plat_lwpRefreshNoteNewThread(thr);
   }
   
   //Look for removed LWPs
   if (lwps.size() - new_lwps_found != pool->size()) {     
      for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
         int_thread *thr = *i;
         bool found = false;
         for (vector<Dyninst::LWP>::iterator j = lwps.begin(); j != lwps.end(); j++) {
            if (thr->getLWP() == *j) {
               found = true;
               break;
            }
         }
         if (found)
            continue;
         change = true;
         pthrd_printf("Found thread %d/%d is dead during refresh\n", getPid(), thr->getLWP());
         EventLWPDestroy::ptr newev = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Pre));
         newev->setProcess(proc());
         newev->setThread(thr->thread());
         newev->setSyncType(Event::async);
         mbox()->enqueue(newev);
      }
   }

   return true;
}

bool int_LWPTracking::plat_lwpRefreshNoteNewThread(int_thread *)
{
   return true;
}

int_threadTracking::int_threadTracking(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                       std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_threadTracking::int_threadTracking(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   up_ptr(NULL)
{
}

int_threadTracking::~int_threadTracking()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_followFork::int_followFork(Dyninst::PID p, std::string e, std::vector<std::string> a,
                               std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   fork_tracking(FollowFork::default_should_follow_fork),
   up_ptr(NULL)
{
}

int_followFork::int_followFork(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   fork_tracking(p->getFollowFork()->fork_tracking),
   up_ptr(NULL)
{
}

int_followFork::~int_followFork()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_multiToolControl::int_multiToolControl(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                           std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_multiToolControl::int_multiToolControl(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   up_ptr(NULL)
{
}

int_multiToolControl::~int_multiToolControl()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_signalMask::int_signalMask(Dyninst::PID p, std::string e, std::vector<std::string> a,
                               std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_signalMask::int_signalMask(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   up_ptr(NULL)
{
}

int_signalMask::~int_signalMask()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_callStackUnwinding::int_callStackUnwinding(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                               std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f)
{
}

int_callStackUnwinding::int_callStackUnwinding(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

int_callStackUnwinding::~int_callStackUnwinding()
{
}


#if 0
//TO BE IMPLEMENTED
bool RemoteIO::getFileNames(std::vector<std::string> &/*filenames*/)
{
   assert(0);
   return false;
}

bool RemoteIO::getFileNames(ProcessSet::ptr /*pset*/, std::map<Process::ptr, std::vector<std::string> > &/*all_filenames*/)
{
   assert(0);
   return false;
}

bool RemoteIO::getFileStatData(std::string /*filename*/, stat_ret_t &/*stat_results*/)
{
   assert(0);
   return false;
}

bool RemoteIO::getFileStatData(ProcessSet::ptr /*pset*/, std::string /*filename*/, 
                               std::map<Process::ptr, stat_ret_t> &/*stat_results*/)
{
   assert(0);
   return false;
}

//Results of these two calls should be 'free()'d by the user
bool RemoteIO::readFileContents(std::string /*filename*/, size_t /*offset*/,
                                size_t /*numbytes*/, unsigned char* &/*result*/)
{
   assert(0);
   return false;
}
   
bool RemoteIO::readFileContents(std::vector<ReadT>& /*targets*/)
{
   assert(0);
   return false;
}

RemoteIO::RemoteIO()
{
   assert(0);
   return false;
}

RemoteIO::~RemoteIO()
{
   assert(0);
   return false;
}
#endif
