/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include "proccontrol/src/int_process.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

PlatformFeatures::~PlatformFeatures()
{
}

static bool default_track_libs = true;

void LibraryTracking::setDefaultTrackLibraries(bool b)
{
   default_track_libs = b;
}

bool LibraryTracking::getDefaultTrackLibraries()
{
   return default_track_libs;
}

bool LibraryTracking::setTrackLibraries(bool b) const
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return LibraryTracking::setTrackLibraries(pset, b);
}

bool LibraryTracking::getTrackLibraries() const
{
   MTLock lock_this_func;
   int_process *llproc = proc->llproc();
   if (!llproc) {
      perr_printf("setTrackLibraries attempted on exited process\n");
      llproc->setLastError(err_exited, "Process is exited\n");
      return false;
   }

   return llproc->sysv_isTrackingLibraries();
}

bool LibraryTracking::refreshLibraries()
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return LibraryTracking::refreshLibraries(pset);
}

ThreadTracking::ThreadTracking()
{
}

ThreadTracking::~ThreadTracking()
{
}

static bool default_track_threads = true;

void ThreadTracking::setDefaultTrackThreads(bool b) 
{
   default_track_threads = b;
}

bool ThreadTracking::getDefaultTrackThreads()
{
   return default_track_threads;
}

bool ThreadTracking::setTrackThreads(bool b) const
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return ThreadTracking::setTrackThreads(pset, b);
}

bool ThreadTracking::getTrackThreads() const
{
   return proc->llproc()->threaddb_isTrackingThreads();
}

bool ThreadTracking::refreshThreads()
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return ThreadTracking::refreshThreads(pset);
}

CallStackUnwinding::CallStackUnwinding()
{
}

CallStackUnwinding::~CallStackUnwinding()
{
}

bool CallStackUnwinding::walkStack(Thread::ptr thr, CallStackCallback *stk_cb) const
{
   ThreadSet::ptr thrset = ThreadSet::newThreadSet(thr);
   return CallStackUnwinding::walkStack(thrset, stk_cb);
}

CallStackCallback::CallStackCallback() :
   top_first(top_first_default_value)
{
}

CallStackCallback::~CallStackCallback()
{
}

FollowFork::FollowFork()
{
}

FollowFork::~FollowFork()
{
}

static FollowFork::follow_t default_should_follow_fork = FollowFork::Follow;
void FollowFork::setDefaultFollowFork(FollowFork::follow_t f) {
   default_should_follow_fork = f;
}

FollowFork::follow_t FollowFork::getDefaultFollowFork()
{
   return default_should_follow_fork;
}

bool FollowFork::setFollowFork(FollowFork::follow_t f) const
{
   MTLock lock_this_func;
   int_process *llproc = proc->llproc();
   if (!llproc) {
      perr_printf("setFollowFork attempted on exited process\n");
      llproc->setLastError(err_exited, "Process is exited\n");
      return false;
   }

   return llproc->fork_setTracking(f);
}

FollowFork::follow_t FollowFork::getFollowFork() const
{
   MTLock lock_this_func;
   int_process *llproc = proc->llproc();
   if (!llproc) {
      perr_printf("getFollowFork attempted on exited process\n");
      llproc->setLastError(err_exited, "Process is exited\n");
      return None;
   }

   return llproc->fork_isTracking();
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
