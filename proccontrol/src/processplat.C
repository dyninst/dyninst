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

LibraryTracking *PlatformFeatures::getLibraryTracking()
{
   return dynamic_cast<LibraryTracking *>(this);
}

const LibraryTracking *PlatformFeatures::getLibraryTracking() const
{
   return dynamic_cast<const LibraryTracking *>(this);
}

ThreadTracking *PlatformFeatures::getThreadTracking()
{
   return dynamic_cast<ThreadTracking *>(this);
}

const ThreadTracking *PlatformFeatures::getThreadTracking() const
{
   return dynamic_cast<const ThreadTracking *>(this);
}

CallStackUnwinding *PlatformFeatures::getCallStackUnwinding()
{
   return dynamic_cast<CallStackUnwinding *>(this);
}

const CallStackUnwinding *PlatformFeatures::getCallStackUnwinding() const
{
   return dynamic_cast<const CallStackUnwinding *>(this);
}

LinuxFeatures *PlatformFeatures::getLinuxFeatures()
{
   return dynamic_cast<LinuxFeatures *>(this);
}

const LinuxFeatures *PlatformFeatures::getLinuxFeatures() const
{
   return dynamic_cast<const LinuxFeatures *>(this);
}

FreeBSDFeatures *PlatformFeatures::getFreeBSDFeatures()
{
   return dynamic_cast<FreeBSDFeatures *>(this);
}

const FreeBSDFeatures *PlatformFeatures::getFreeBSDFeatures() const
{
   return dynamic_cast<const FreeBSDFeatures *>(this);
}

BlueGeneQFeatures *PlatformFeatures::getBlueGeneQFeatures()
{
   return dynamic_cast<BlueGeneQFeatures *>(this);
}

const BlueGeneQFeatures *PlatformFeatures::getBlueGeneQFeatures() const
{
   return dynamic_cast<const BlueGeneQFeatures *>(this);
}

LibraryTracking::LibraryTracking()
{
}

LibraryTracking::~LibraryTracking()
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

LinuxFeatures::LinuxFeatures()
{
}

LinuxFeatures::~LinuxFeatures()
{
}

FreeBSDFeatures::FreeBSDFeatures()
{
}

FreeBSDFeatures::~FreeBSDFeatures()
{
}

BlueGeneQFeatures::BlueGeneQFeatures()
{
}

BlueGeneQFeatures::~BlueGeneQFeatures()
{
}

CallStackUnwinding::CallStackUnwinding()
{
}

CallStackUnwinding::~CallStackUnwinding()
{
}

bool CallStackUnwinding::walkStack(Thread::ptr thr, CallStackCallback *stk_cb)
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

static bool default_should_follow_fork;
void FollowFork::setDefaultFollowFork(bool b)
{
   default_should_follow_fork = b;
}

bool FollowFork::getDefaultFollowFork()
{
   return default_should_follow_fork;
}

void FollowFork::setFollowFork(bool b)
{
}

bool FollowFork::getFollowFork()
{
}

bool RemoteIO::getFileNames(std::vector<std::string> &filenames)
{
}

bool RemoteIO::getFileNames(ProcessSet::ptr pset, std::map<Process::ptr, std::vector<std::string> > &all_filenames)
{
}

bool RemoteIO::getFileStatData(std::string filename, stat_ret_t &stat_results)
{
}

bool RemoteIO::getFileStatData(ProcessSet::ptr pset, std::string filename, 
                               std::map<Process::ptr, stat_ret_t> &stat_results)
{
}

//Results of these two calls should be 'free()'d by the user
bool RemoteIO::readFileContents(std::string filename, size_t offset, size_t numbytes, unsigned char* &result)
{
}
   
bool RemoteIO::readFileContents(std::vector<ReadT> &targets)
{
}

RemoteIO::RemoteIO()
{
}

RemoteIO::~RemoteIO()
{
}

