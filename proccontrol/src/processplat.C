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

#include "PlatFeatures.h"
#include "Mailbox.h"
#include "int_process.h"
#include "procpool.h"
#include "processplat.h"
#include "int_event.h"

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

MemoryUsage::MemoryUsage(Process::ptr proc_) :
   proc(proc_)
{
}

MemoryUsage::~MemoryUsage()
{
   proc = Process::weak_ptr();
}

bool MemoryUsage::sharedUsed(unsigned long &used) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "sharedUsed", false);
   int_memUsage *llproc = p->llproc()->getMemUsage();
   unsigned long val;

   MemUsageResp_t mem_response(&val, llproc);
   bool result = llproc->plat_getSharedUsage(&mem_response);
   if (!result) {
      perr_printf("Error getting shared usage\n");
      return false;
   }

   llproc->waitForEvent(&mem_response);
   used = *mem_response.get();
   return true;
}

bool MemoryUsage::heapUsed(unsigned long &used) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "heapUsed", false);
   int_memUsage *llproc = p->llproc()->getMemUsage();
   unsigned long val;

   MemUsageResp_t mem_response(&val, llproc);
   bool result = llproc->plat_getHeapUsage(&mem_response);
   if (!result) {
      perr_printf("Error getting heap usage\n");
      return false;
   }

   llproc->waitForEvent(&mem_response);
   used = *mem_response.get();
   return true;
}

bool MemoryUsage::stackUsed(unsigned long &used) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "stackUsed", false);
   int_memUsage *llproc = p->llproc()->getMemUsage();
   unsigned long val;

   MemUsageResp_t mem_response(&val, llproc);
   bool result = llproc->plat_getStackUsage(&mem_response);
   if (!result) {
      perr_printf("Error getting stack usage\n");
      return false;
   }

   llproc->waitForEvent(&mem_response);
   used = *mem_response.get();
   return true;
}

bool MemoryUsage::resident(unsigned long &resident) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "resident", false);
   int_memUsage *llproc = p->llproc()->getMemUsage();

   if (!llproc->plat_residentNeedsMemVals()) {
      unsigned long val;
      MemUsageResp_t mem_response(&val, llproc);
      bool result = llproc->plat_getResidentUsage(0, 0, 0, &mem_response);
      if (!result) {
         llproc->setLastError(err_internal, "Could not get resident usage\n");
         perr_printf("Error getting resident usage\n");
         return false;
      }

      llproc->waitForEvent(&mem_response);
      resident = *mem_response.get();
      return true;
   }

   unsigned long st, he, sh, re;
   MemUsageResp_t st_resp(&st, llproc), he_resp(&he, llproc), sh_resp(&sh, llproc), re_resp(&re, llproc);

   bool result = llproc->plat_getStackUsage(&st_resp);
   if (!result) {
      llproc->setLastError(err_internal, "Could not get resident usage\n");
      perr_printf("Error getting stack usage for resident in proc %d\n", llproc->getPid());
      return false;
   }

   result = llproc->plat_getHeapUsage(&he_resp);
   if (!result) {
      llproc->setLastError(err_internal, "Could not get resident usage\n");
      perr_printf("Error getting heap usage for resident in proc %d\n", llproc->getPid());
      return false;
   }

   result = llproc->plat_getSharedUsage(&sh_resp);
   if (!result) {
      llproc->setLastError(err_internal, "Could not get resident usage\n");
      perr_printf("Error getting shared usage for resident in proc %d\n", llproc->getPid());
      return false;
   }

   llproc->waitForEvent(&st_resp);
   llproc->waitForEvent(&he_resp);
   llproc->waitForEvent(&sh_resp);

   result = llproc->plat_getResidentUsage(st, he, sh, &re_resp);
   if (!result) {
      llproc->setLastError(err_internal, "Could not get resident usage\n");
      perr_printf("Error getting resident usage for resident in proc %d\n", llproc->getPid());
      return false;
   }

   llproc->waitForEvent(&re_resp);

   resident = *re_resp.get();
   return true;
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

FileInfo::FileInfo(std::string f)
{
   info = int_fileInfo_ptr(new int_fileInfo());
   info->filename = f;
}

FileInfo::FileInfo()
{
}

FileInfo::FileInfo(const FileInfo &fi)
{
   info = fi.info;
}

FileInfo::~FileInfo()
{
}

std::string FileInfo::getFilename() const
{
   if (!info)
      return std::string();
   return info->filename;
}

stat64_ptr FileInfo::getStatResults() const
{
   if (!info)
      return NULL;
   return info->stat_results;
}

int_fileInfo_ptr FileInfo::getInfo() const
{
   if (!info)
      info = int_fileInfo_ptr(new int_fileInfo());
   return info;
}

RemoteIO::RemoteIO(Process::ptr proc_) :
   proc(proc_)
{
}

RemoteIO::~RemoteIO()
{
}

FileSet *RemoteIO::getFileSet(string filename) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getFileSet", NULL);
   FileSet *new_fs = new FileSet();
   new_fs->insert(make_pair(static_cast<Process::const_ptr>(p), FileInfo(filename)));
   return new_fs;
}

FileSet *RemoteIO::getFileSet(const set<string> &filenames) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getFielSet", NULL);
   FileSet *new_fs = new FileSet();

   for (set<string>::const_iterator i = filenames.begin(); i != filenames.end(); i++) {
      new_fs->insert(make_pair(static_cast<Process::const_ptr>(p), FileInfo(*i)));
   }
   return new_fs;
}

bool RemoteIO::addToFileSet(std::string filename, FileSet *fs) const
{
   MTLock lock_this_func;
   Process::const_ptr p = proc.lock();
   PTR_EXIT_TEST(p, "addToFileSet", false);
   fs->insert(make_pair(p, FileInfo(filename)));
   return true;
}

bool RemoteIO::getFileNames(FileSet *fset) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getFileNames", false);
   int_remoteIO *remoteIO = p->llproc()->getRemoteIO();
   return remoteIO->getFileNames(fset);
}

bool RemoteIO::getFileStatData(FileSet *fset) const
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getStatData", false);
   int_remoteIO *remoteIO = p->llproc()->getRemoteIO();
   return remoteIO->getFileStatData(*fset);
}

bool RemoteIO::readFileContents(const FileSet *fset)
{
   MTLock lock_this_func;
   Process::ptr p = proc.lock();
   PTR_EXIT_TEST(p, "getStatData", false);
   int_remoteIO *remoteIO = p->llproc()->getRemoteIO();
   return remoteIO->getFileDataAsync(*fset);
}

RemoteIOSet::RemoteIOSet(ProcessSet::ptr procs_)
{
   pset = procs_;
}

RemoteIOSet::~RemoteIOSet()
{
}

FileSet *RemoteIOSet::getFileSet(string filename)
{
   ProcessSet::ptr procs = pset.lock();
   if (!procs || procs->empty()) {
      globalSetLastError(err_badparam, "Cannot create fileset from empty process set");
      return NULL;
   }

   FileSet *new_fs = new FileSet();
   for (ProcessSet::iterator i = procs->begin(); i != procs->end(); i++) {
      new_fs->insert(make_pair(*i, FileInfo(filename)));
   }
   return new_fs;
}

FileSet *RemoteIOSet::getFileSet(const set<string> &filenames)
{
   ProcessSet::ptr procs = pset.lock();
   if (!procs || procs->empty()) {
      globalSetLastError(err_badparam, "Cannot create fileset from empty process set");
      return NULL;
   }
   if (filenames.empty()) {
      globalSetLastError(err_badparam, "Cannot create a fileset from empty list of names");
      return NULL;
   }

   FileSet *new_fs = new FileSet();
   for (ProcessSet::iterator i = procs->begin(); i != procs->end(); i++) {
      for (set<string>::iterator j = filenames.begin(); j != filenames.end(); j++) {
         new_fs->insert(make_pair(*i, *j));
      }
   }
   return new_fs;
}

bool RemoteIOSet::addToFileSet(string filename, FileSet *fs)
{
   ProcessSet::ptr procs = pset.lock();
   if (!procs || procs->empty()) {
      globalSetLastError(err_badparam, "Cannot add empty process et to fileset");
      return false;
   }
   if (!fs) {
      globalSetLastError(err_badparam, "NULL FileSet parameter\n");
      return false;
   }
   for (ProcessSet::iterator i = procs->begin(); i != procs->end(); i++) {
      fs->insert(make_pair(*i, filename));
   }
   return true;
}

int_libraryTracking::int_libraryTracking(Dyninst::PID p, string e, vector<string> a,
                                         vector<string> envp, map<int,int> f) :
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

int_LWPTracking::int_LWPTracking(Dyninst::PID p, string e, vector<string> a,
                                 vector<string> envp, map<int,int> f) :
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

int_threadTracking::int_threadTracking(Dyninst::PID p, string e, vector<string> a,
                                       vector<string> envp, map<int,int> f) :
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

int_followFork::int_followFork(Dyninst::PID p, string e, vector<string> a,
                               vector<string> envp, map<int,int> f) :
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

int_multiToolControl::int_multiToolControl(Dyninst::PID p, string e, vector<string> a,
                                           vector<string> envp, map<int,int> f) :
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

int_signalMask::int_signalMask(Dyninst::PID p, string e, vector<string> a,
                               vector<string> envp, map<int,int> f) :
   int_process(p, e, a, envp, f),
   sigset(SignalMask::getDefaultSigMask()),
   up_ptr(NULL)
{
}

int_signalMask::int_signalMask(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   sigset(SignalMask::getDefaultSigMask()),
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

int_callStackUnwinding::int_callStackUnwinding(Dyninst::PID p, string e, vector<string> a,
                                               vector<string> envp, map<int,int> f) :
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

int_memUsage::int_memUsage(Dyninst::PID p, std::string e, std::vector<std::string> a,
                           std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_memUsage::int_memUsage(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   up_ptr(NULL)
{
}

int_memUsage::~int_memUsage()
{
   if (up_ptr) {
      delete up_ptr;
      up_ptr = NULL;
   }
}

int_fileInfo::int_fileInfo() :
   stat_results(NULL),
   cur_pos(0)
{
}

int_fileInfo::~int_fileInfo()
{
   if (stat_results) {
      delete stat_results;
      stat_results = NULL;
   }
}

int_remoteIO::int_remoteIO(Dyninst::PID p, std::string e, std::vector<std::string> a,
                           std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   resp_process(p, e, a, envp, f),
   up_ptr(NULL)
{
}

int_remoteIO::int_remoteIO(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   resp_process(pid_, p),
   up_ptr(NULL)
{
}

int_remoteIO::~int_remoteIO()
{
}

bool int_remoteIO::getFileNames(FileSet *fset)
{
   if (!fset) {
      perr_printf("Null FileSet passed to getFileNames\n");
      setLastError(err_badparam, "Unexpected NULL parameter");
      return false;
   }

   FileSetResp_t resp(fset, this);

   bool result = plat_getFileNames(&resp);
   if (!result) {
      perr_printf("Error requesting filenames");
      return false;
   }

   waitForEvent(&resp);
   return true;
}

bool int_remoteIO::getFileStatData(FileSet &files)
{
   set<StatResp_t *> resps;
   bool had_error = false;

   for (FileSet::iterator i = files.begin(); i != files.end(); i++) {
      if (static_cast<int_process *>(this) != i->first->llproc()) {
         perr_printf("Non-local process in fileset, %d specified for %d\n",
                     i->first->llproc()->getPid(), getPid());
         setLastError(err_badparam, "Non-local process specified in FileSet");
         had_error = true;
         continue;
      }

      FileInfo &fi = i->second;
      int_fileInfo_ptr info = fi.getInfo();
      if (info->filename.empty()) {
         perr_printf("Empty filename in stat operation on %d\n", getPid());
         setLastError(err_badparam, "Empty filename specified in stat operation");
         had_error = true;
         continue;
      }

      bool result = plat_getFileStatData(info->filename, &info->stat_results, resps);
      if (!result) {
         pthrd_printf("Error while requesting file data stat on %d\n", getPid());
         had_error = true;
         continue;
      }
   }

   for (set<StatResp_t *>::iterator i = resps.begin(); i != resps.end(); i++) {
      waitForEvent(*i);
      delete *i;
   }

   return !had_error;
}

bool int_remoteIO::getFileDataAsync(const FileSet &files)
{
   set<FileReadResp_t *> resps;
   bool had_error = false;

   for (FileSet::const_iterator i = files.begin(); i != files.end(); i++) {
      int_fileInfo_ptr fi = i->second.getInfo();
      if (static_cast<int_process *>(this) != i->first->llproc()) {
         perr_printf("Non-local process in fileset, %d specified for %d\n",
                     i->first->llproc()->getPid(), getPid());
         setLastError(err_badparam, "Non-local process specified in FileSet\n");
         had_error = true;
         continue;
      }

      int_eventAsyncFileRead *fileread = new int_eventAsyncFileRead();
      fileread->offset = 0;
      fileread->whole_file = true;
      fileread->filename = fi->filename;
      fi->cur_pos = 0;
      bool result = plat_getFileDataAsync(fileread);
      if (!result) {
         pthrd_printf("Error while requesting file data on %d\n", getPid());
         had_error = true;
         delete fileread;
         continue;
      }
   }

   return !had_error;
}


