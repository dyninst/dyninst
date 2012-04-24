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
#include <vector>
#include <string>

#if !defined(PROCESSPLAT_H_)
#define PROCESSPLAT_H_

class int_process;

namespace Dyninst {
namespace ProcControlAPI {

class LibraryTracking;
class ThreadTracking;
class CallStackUnwinding;
class RemoteIO;
class FollowFork;
class FreeBSDFeatures;
class LinuxFeatures;
class BlueGeneQFeatures;

class PlatformFeatures
{
   friend class ::int_process;
  protected:
   Process::ptr proc;
   virtual ~PlatformFeatures();
  public:
   LibraryTracking *getLibraryTracking();
   ThreadTracking *getThreadTracking();
   CallStackUnwinding *getCallStackUnwinding();
   RemoteIO *getRemoteIO();
   FollowFork *getFollowFork();
   LinuxFeatures *getLinuxFeatures();
   FreeBSDFeatures *getFreeBSDFeatures();
   BlueGeneQFeatures *getBlueGeneQFeatures();

   const LibraryTracking *getLibraryTracking() const;
   const ThreadTracking *getThreadTracking() const;
   const CallStackUnwinding *getCallStackUnwinding() const;
   const RemoteIO *getRemoteIO() const;
   const FollowFork *getFollowFork() const;
   const LinuxFeatures *getLinuxFeatures() const;
   const FreeBSDFeatures *getFreeBSDFeatures() const;
   const BlueGeneQFeatures *getBlueGeneQFeatures() const;
};

class LibraryTracking : virtual public PlatformFeatures
{
  protected:
   LibraryTracking();
   virtual ~LibraryTracking();
  public:
   static void setDefaultTrackLibraries(bool b);
   static bool getDefaultTrackLibraries();

   bool setTrackLibraries(bool b) const;
   bool getTrackLibraries() const;
   bool refreshLibraries();

   static bool setTrackLibraries(ProcessSet::ptr ps, bool b);
   static bool refreshLibraries(ProcessSet::ptr ps);
};

class ThreadTracking : virtual public PlatformFeatures
{
  protected:
   ThreadTracking();
   virtual ~ThreadTracking();
  public:
   static void setDefaultTrackThreads(bool b);
   static bool getDefaultTrackThreads();

   bool setTrackThreads(bool b) const;
   bool getTrackThreads() const;
   bool refreshThreads();

   static bool setTrackThreads(ProcessSet::ptr ps, bool b);
   static bool refreshThreads(ProcessSet::ptr ps);
};

class CallStackCallback : virtual public PlatformFeatures
{
  private:
   static const bool top_first_default_value = false;
  public:
   bool top_first;
   CallStackCallback();
   virtual bool beginStackWalk(Thread::ptr thr) = 0;
   virtual bool addStackFrame(Thread::ptr thr, Dyninst::Address ra, Dyninst::Address sp, Dyninst::Address fp) = 0;
   virtual void endStackWalk(Thread::ptr thr) = 0;
   virtual ~CallStackCallback();
};

class CallStackUnwinding : virtual public PlatformFeatures
{
  public:
   CallStackUnwinding();
   virtual ~CallStackUnwinding();
   bool walkStack(Thread::ptr thr, CallStackCallback *stk_cb);
   static bool walkStack(ThreadSet::ptr thrset, CallStackCallback *stk_cb);
};

class FollowFork : virtual public PlatformFeatures
{
  public:
   FollowFork();
   virtual ~FollowFork();

   static void setDefaultFollowFork(bool b);
   static bool getDefaultFollowFork();

   void setFollowFork(bool b);
   bool getFollowFork();
};

#if !defined(_MSC_VER)
typedef void* stat_ret_t;
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
typedef struct stat stat_ret_t;
#endif

class RemoteIO : virtual public PlatformFeatures
{
  public:

   bool getFileNames(std::vector<std::string> &filenames);
   static bool getFileNames(ProcessSet::ptr pset, std::map<Process::ptr, std::vector<std::string> > &all_filenames);

   bool getFileStatData(std::string filename, stat_ret_t &stat_results);
   static bool getFileStatData(ProcessSet::ptr pset, std::string filename, 
                               std::map<Process::ptr, stat_ret_t> &stat_results);

   //Results of these two calls should be 'free()'d by the user
   bool readFileContents(std::string filename, size_t offset, size_t numbytes, unsigned char* &result);
   
   struct ReadT {
      Process::ptr proc;
      std::string filename;
      size_t offset;
      size_t numbytes;
      void *result; //Output parameter
   };
   static bool readFileContents(std::vector<ReadT> &targets);

   RemoteIO();
   virtual ~RemoteIO();
};

class LinuxFeatures : public LibraryTracking, public ThreadTracking, public FollowFork
{
  public:
   LinuxFeatures();
   virtual ~LinuxFeatures();
};

class FreeBSDFeatures : public LibraryTracking, public ThreadTracking
{
  public:
   FreeBSDFeatures();
   virtual ~FreeBSDFeatures();
};

class BlueGeneQFeatures : public LibraryTracking, public ThreadTracking, public CallStackUnwinding, public RemoteIO
{
  public:
   BlueGeneQFeatures();
   virtual ~BlueGeneQFeatures();

   static void setToolName(std::string str);
   static void setToolPriority(unsigned int priority);
};

}
}

#endif
