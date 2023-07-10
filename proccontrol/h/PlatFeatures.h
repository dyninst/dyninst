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
#include <map>
#include <set>

#if !defined(PLATFEATURES_H_)
#define PLATFEATURES_H_

class int_process;
class sysv_process;
class thread_db_process;
class linux_process;
class int_libraryTracking;
class int_LWPTracking;
class int_threadTracking;
class int_followFork;
class int_multiToolControl;
class int_signalMask;
class int_callStackUnwinding;
class int_remoteIO;
class int_memUsage;
class int_fileInfo;

//For sigset_t
#if !defined(_MSC_VER)
#include <signal.h>
#endif

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT LibraryTracking
{
   friend class ::int_libraryTracking;
   friend class ::int_process;
  protected:
   LibraryTracking(Process::ptr proc_);
   ~LibraryTracking();
   Process::weak_ptr proc;
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
   friend class ::int_process;
  protected:
   LibraryTrackingSet(ProcessSet::ptr ps_);
   ~LibraryTrackingSet();
   ProcessSet::weak_ptr wps;
  public:
   bool setTrackLibraries(bool b) const;
   bool refreshLibraries() const;
};

class PC_EXPORT LWPTracking
{
   friend class ::linux_process;
   friend class ::int_process;
   friend class ::int_LWPTracking;
  protected:
   LWPTracking(Process::ptr proc_);
   ~LWPTracking();
   Process::weak_ptr proc;
   static bool default_track_lwps;
  public:
   static void setDefaultTrackLWPs(bool b);
   static bool getDefaultTrackLWPs();

   void setTrackLWPs(bool b) const;
   bool getTrackLWPs() const;
   bool refreshLWPs();
};

class PC_EXPORT LWPTrackingSet
{
   friend class ProcessSet;
   friend class PSetFeatures;
   friend class ::int_process;
  protected:
   LWPTrackingSet(ProcessSet::ptr ps_);
   ~LWPTrackingSet();
   ProcessSet::weak_ptr wps;
  public:
   bool setTrackLWPs(bool b) const;
   bool refreshLWPs() const;
};

class PC_EXPORT ThreadTracking
{
   friend class ::int_process;
   friend class ::thread_db_process;
   friend class ::int_threadTracking;
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
   friend class ::int_process;
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
   friend class ::int_process;
   friend class ::int_followFork;
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
   friend class ::int_process;
   friend class ::int_thread;
   friend class ::int_callStackUnwinding;
  private:
   Thread::weak_ptr wt;
  public:
   CallStackUnwinding(Thread::ptr t);
   virtual ~CallStackUnwinding();
   bool walkStack(CallStackCallback *stk_cb) const;
};

class PC_EXPORT MemoryUsage
{
   friend class ::int_process;
   friend class ::int_memUsage;
  private:
   MemoryUsage(Process::ptr proc_);
   ~MemoryUsage();
   Process::weak_ptr proc;
  public:
   bool sharedUsed(unsigned long &sused) const;
   bool heapUsed(unsigned long &hused) const;
   bool stackUsed(unsigned long &sused) const;
   bool resident(unsigned long &resident) const;
};

class PC_EXPORT MemoryUsageSet
{
   friend class ProcessSet;
   friend class PSetFeatures;
  protected:
   MemoryUsageSet(ProcessSet::ptr ps_);
   ~MemoryUsageSet();
   ProcessSet::weak_ptr wps;
   typedef enum {
      mus_shared = 0,
      mus_heap,
      mus_stack,
      mus_resident
   } mem_usage_t;
   bool usedX(std::map<Process::const_ptr, unsigned long> &used, mem_usage_t mu) const;
  public:
   bool sharedUsed(std::map<Process::const_ptr, unsigned long> &used) const;
   bool heapUsed(std::map<Process::const_ptr, unsigned long> &used) const;
   bool stackUsed(std::map<Process::const_ptr, unsigned long> &used) const;
   bool resident(std::map<Process::const_ptr, unsigned long> &res) const;
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
   friend class ::int_process;
   friend class ::int_multiToolControl;
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

#if defined(_MSC_VER)
typedef void* dyn_sigset_t;
#else
typedef sigset_t dyn_sigset_t;
#endif

//On posix system, the sigset referenced below is a pointer to a sigset_t
class PC_EXPORT SignalMask
{
   friend class ::int_process;
   friend class ::int_signalMask;
  protected:
   static dyn_sigset_t default_sigset;
   static bool sigset_initialized;
  private:
   Process::weak_ptr proc;
   SignalMask(Process::ptr proc_);
   ~SignalMask();
  public:
   static dyn_sigset_t getDefaultSigMask();
   static void setDefaultSigMask(dyn_sigset_t s);
   dyn_sigset_t getSigMask() const;
   bool setSigMask(dyn_sigset_t s);
};

/**
 * This struct is copied from the GLIBC sources for 'struct stat64'.  It is 
 * recreated here because this header is supposed to compile without ifdefs 
 * across platforms that may not have 'struct stat64'
 **/
extern "C" struct stat64_ret_t {
   unsigned long long st_dev;
   unsigned long long st_ino;
   unsigned int st_mode;
   unsigned int st_nlink;
   unsigned int st_uid;
   unsigned int st_gid;
   unsigned long long st_rdev;
   unsigned short __pad2;
   long long st_size;
   int st_blksize;
   long long st_blocks;
   int st_atime_;
   unsigned int st_atime_nsec;
   int st_mtime_;
   unsigned int st_mtime_nsec;
   int st_ctime_;
   unsigned int st_ctime_nsec;
   unsigned int __unused4;
   unsigned int __unused5;
};

typedef stat64_ret_t *stat64_ptr;
typedef boost::shared_ptr<int_fileInfo> int_fileInfo_ptr;

class RemoteIO;
class RemoteIOSet;

class FileInfo {
   friend class ::int_remoteIO;
   friend class RemoteIO;
   friend class RemoteIOSet;
  private:
   mutable int_fileInfo_ptr info;
   int_fileInfo_ptr getInfo() const;
  public:
   FileInfo(std::string fname);
   FileInfo();
   FileInfo(const FileInfo &fi);
   ~FileInfo();
   
   std::string getFilename() const;
   stat64_ptr getStatResults() const; //Filled in by getFileStatData
};

typedef std::multimap<Process::const_ptr, FileInfo> FileSet;

class PC_EXPORT RemoteIO
{
  protected:
   Process::weak_ptr proc;
  public:
   RemoteIO(Process::ptr proc);
   virtual ~RemoteIO();

   //Construct filesets based on filenames, without doing a getFileNames
   // User is responsible for 'delete'ing the FileSet when done.
   FileSet *getFileSet(std::string filename) const;
   FileSet *getFileSet(const std::set<std::string> &filenames) const;
   bool addToFileSet(std::string filename, FileSet *fs) const;
  
   bool getFileNames(FileSet *result) const;

   //Get data as per a stat system call, fill in the FileInfo objects
   bool getFileStatData(FileSet *fset) const;

   //These are whole file reads and produce EventAsyncFileRead callbacks
   bool readFileContents(const FileSet *fset);
};

class PC_EXPORT RemoteIOSet
{
  protected:
   ProcessSet::weak_ptr pset;
  public:
   RemoteIOSet(ProcessSet::ptr procs_);
   virtual ~RemoteIOSet();

   FileSet *getFileSet(std::string filename);
   FileSet *getFileSet(const std::set<std::string> &filenames);
   bool addToFileSet(std::string filename, FileSet *fs);

   bool getFileNames(FileSet *result);
   bool getFileStatData(FileSet *fset);
   bool readFileContents(const FileSet *fset);
};

}
}

#endif
