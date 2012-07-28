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

#include "PCProcess.h"
#include "ProcessSet.h"

#include <list>
#include <vector>
#include <string>

#if !defined(PROCESSPLAT_H_)
#define PROCESSPLAT_H_

class int_process;
class sysv_process;
class thread_db_process;
class linux_process;
namespace bgq {
   class bgq_process;
};
namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT LibraryTracking
{
   friend class ::sysv_process;
  protected:
   LibraryTracking(Process::ptr proc_);
   ~LibraryTracking();
   Process::weak_ptr proc;
   static bool default_track_libs;
  public:
   static void setDefaultTrackLibraries(bool b);
   static bool getDefaultTrackLibraries();

   bool setTrackLibraries(bool b) const;
   bool getTrackLibraries() const;
   bool refreshLibraries();
};

class PC_EXPORT LibraryTrackingSet
{
   friend class ProcessSet;
   friend class PSetFeatures;
  protected:
   LibraryTrackingSet(ProcessSet::ptr ps_);
   ~LibraryTrackingSet();
   ProcessSet::weak_ptr wps;
  public:
   bool setTrackLibraries(bool b) const;
   bool refreshLibraries() const;
};

class PC_EXPORT ThreadTracking
{
   friend class ::thread_db_process;
  protected:
   ThreadTracking(Process::ptr proc_);
   ~ThreadTracking();
   Process::weak_ptr proc;
   static bool default_track_threads;
  public:
   static void setDefaultTrackThreads(bool b);
   static bool getDefaultTrackThreads();

   bool setTrackThreads(bool b) const;
   bool getTrackThreads() const;
   bool refreshThreads();
};

class PC_EXPORT ThreadTrackingSet
{
   friend class ProcessSet;
   friend class PSetFeatures;
  protected:
   ThreadTrackingSet(ProcessSet::ptr ps_);
   ~ThreadTrackingSet();
   ProcessSet::weak_ptr wps;
  public:
   bool setTrackThreads(bool b) const;
   bool refreshThreads() const;
};

class PC_EXPORT FollowFork
{
   friend class ::linux_process;
  protected:
   FollowFork(Process::ptr proc_);
   ~FollowFork();
   Process::weak_ptr proc;
  public:

   typedef enum {
      None,                      //Fork tracking not available on this platform.
      ImmediateDetach,           //Do not even attach to forked children.
      DisableBreakpointsDetach,  //Remove inherited breakpoints from children, then detach.
      Follow                     //Default. Attach and full control of forked children.
   } follow_t;

   static void setDefaultFollowFork(follow_t f);
   static follow_t getDefaultFollowFork();

   bool setFollowFork(follow_t b) const;
   follow_t getFollowFork() const;
  protected:
   static follow_t default_should_follow_fork;
};

class PC_EXPORT FollowForkSet
{
   friend class ProcessSet;
   friend class PSetFeatures;
  protected:
   FollowForkSet(ProcessSet::ptr ps_);
   ~FollowForkSet();
   ProcessSet::weak_ptr wps;
  public:
   bool setFollowFork(FollowFork::follow_t f) const;
};

class PC_EXPORT CallStackCallback
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

class PC_EXPORT CallStackUnwinding
{
  private:
   Thread::weak_ptr wt;
  public:
   CallStackUnwinding(Thread::ptr t);
   virtual ~CallStackUnwinding();
   bool walkStack(CallStackCallback *stk_cb) const;
};

class PC_EXPORT CallStackUnwindingSet
{
  private:
   ThreadSet::weak_ptr wts;
  public:
   CallStackUnwindingSet(ThreadSet::ptr ts);
   ~CallStackUnwindingSet();
   bool walkStack(CallStackCallback *stk_cb);
};

class PC_EXPORT MultiToolControl
{
   friend class bgq::bgq_process;
  public:
   typedef unsigned int priority_t;
  private:
   Process::weak_ptr proc;
  protected:
   MultiToolControl(Process::ptr p);
   ~MultiToolControl();
   static std::string default_tool_name;
   static priority_t default_tool_priority;
  public:
   static void setDefaultToolName(std::string name);
   static void setDefaultToolPriority(priority_t p);
   static std::string getDefaultToolName();
   static priority_t getDefaultToolPriority();
   
   //Tool name and priority cannot be changed after process creation.
   //To set these values, use the static methods to set the default,
   // values, then trigger your attach/create operation.
   std::string getToolName() const;
   priority_t getToolPriority() const;
};

#if 0
//TO BE IMPLEMENTED

#if defined(_MSC_VER)
typedef void* stat_ret_t;
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
typedef struct stat stat_ret_t;
#endif

class PC_EXPORT RemoteIO : virtual public PlatformFeatures
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
#endif

}
}

#endif
