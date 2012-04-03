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

#include "proccontrol/h/ProcessPlat.h"
#include "proccontrol/src/int_process.h"

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

PlatformProcess::~PlatformProcess()
{
}

SysVProcess *PlatformProcess::getSysVProcess()
{
   return dynamic_cast<SysVProcess *>(this);
}

const SysVProcess *PlatformProcess::getSysVProcess() const
{
   return dynamic_cast<const SysVProcess *>(this);
}

ThreadDBProcess *PlatformProcess::getThreadDBProcess()
{
   return dynamic_cast<ThreadDBProcess *>(this);
}

const ThreadDBProcess *PlatformProcess::getThreadDBProcess() const
{
   return dynamic_cast<const ThreadDBProcess *>(this);
}

LinuxProcess *PlatformProcess::getLinuxProcess()
{
   return dynamic_cast<LinuxProcess *>(this);
}

const LinuxProcess *PlatformProcess::getLinuxProcess() const
{
   return dynamic_cast<const LinuxProcess *>(this);
}

BlueGeneQProcess *PlatformProcess::getBlueGeneQProcess()
{
   return dynamic_cast<BlueGeneQProcess *>(this);
}

const BlueGeneQProcess *PlatformProcess::getBlueGeneQProcess() const
{
   return dynamic_cast<const BlueGeneQProcess *>(this);
}

SysVProcess::SysVProcess()
{
}

SysVProcess::~SysVProcess()
{
}

static bool default_track_libs = true;

void SysVProcess::setDefaultTrackLibraries(bool b)
{
   default_track_libs = b;
}

bool SysVProcess::getDefaultTrackLibraries()
{
   return default_track_libs;
}

bool SysVProcess::setTrackLibraries(bool b) const
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return SysVProcess::setTrackLibraries(pset, b);
}

bool SysVProcess::getTrackLibraries() const
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

bool SysVProcess::refreshLibraries()
{
   ProcessSet::ptr pset = ProcessSet::newProcessSet(proc);
   return SysVProcess::refreshLibraries(pset);
}

ThreadDBProcess::ThreadDBProcess()
{
}

ThreadDBProcess::~ThreadDBProcess()
{
}

static bool default_track_threads = true;

void ThreadDBProcess::setDefaultTrackThreads(bool b) 
{
   default_track_threads = b;
}

bool ThreadDBProcess::getDefaultTrackThreads()
{
   return default_track_threads;
}

void ThreadDBProcess::setTrackThreads(bool b) const
{
}

bool ThreadDBProcess::getTrackThreads() const
{
}

bool ThreadDBProcess::refreshThreads()
{
}

LinuxProcess::LinuxProcess()
{
}

LinuxProcess::~LinuxProcess()
{
}

BlueGeneQProcess::BlueGeneQProcess()
{
}

BlueGeneQProcess::~BlueGeneQProcess()
{
}

bool BlueGeneQProcess::walkStack(Thread::ptr thr, CallStackCallback *stk_cb)
{
   ThreadSet::ptr thrset = ThreadSet::newThreadSet(thr);
   return BlueGeneQProcess::walkStack(thrset, stk_cb);
}

CallStackCallback::CallStackCallback() :
   top_first(top_first_default_value)
{
}

CallStackCallback::~CallStackCallback()
{
}
