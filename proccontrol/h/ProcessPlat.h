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

#include "Process.h"
#include "ProcessSet.h"

#include <list>


#if !defined(PROCESSPLAT_H_)
#define PROCESSPLAT_H_

class int_process;

namespace Dyninst {
namespace ProcControlAPI {

class SysVProcess;
class ThreadDBProcess;
class LinuxProcess;
class BlueGeneQProcess;

class PlatformProcess
{
   friend class ::int_process;
  protected:
   Process::ptr proc;
   virtual ~PlatformProcess();
  public:
   SysVProcess *getSysVProcess();
   ThreadDBProcess *getThreadDBProcess();
   LinuxProcess *getLinuxProcess();
   BlueGeneQProcess *getBlueGeneQProcess();

   const SysVProcess *getSysVProcess() const;
   const ThreadDBProcess *getThreadDBProcess() const;
   const LinuxProcess *getLinuxProcess() const;
   const BlueGeneQProcess *getBlueGeneQProcess() const;
};

class SysVProcess : virtual public PlatformProcess
{
  protected:
   SysVProcess();
   virtual ~SysVProcess();
  public:
   static void setDefaultTrackLibraries(bool b);
   static bool getDefaultTrackLibraries();

   bool setTrackLibraries(bool b) const;
   bool getTrackLibraries() const;
   bool refreshLibraries();

   static bool setTrackLibraries(ProcessSet::ptr ps, bool b);
   static bool refreshLibraries(ProcessSet::ptr ps);
};

class ThreadDBProcess : virtual public PlatformProcess
{
  protected:
   ThreadDBProcess();
   virtual ~ThreadDBProcess();
  public:
   static void setDefaultTrackThreads(bool b);
   static bool getDefaultTrackThreads();
   void setTrackThreads(bool b) const;
   bool getTrackThreads() const;
   bool refreshThreads();
};

class LinuxProcess : public SysVProcess, public ThreadDBProcess
{
  protected:
   virtual ~LinuxProcess();
  public:
   LinuxProcess();
};

class BlueGeneQProcess : public SysVProcess, public ThreadDBProcess
{
  protected:
   BlueGeneQProcess();
   virtual ~BlueGeneQProcess();
  public:
   bool walkStack(std::list<Address> &sw_addrs);
};

}
}

#endif
