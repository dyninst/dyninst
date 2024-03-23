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

#if !defined(PROCESSSET_H_)
#define PROCESSSET_H_

#include <string>
#include <map>
#include <set>
#include <stddef.h>
#include <utility>
#include <vector>

#include "dyntypes.h"
#include "PCProcess.h"
#include "PCErrors.h"

typedef std::multimap<Dyninst::Address, Dyninst::ProcControlAPI::Process::ptr> int_addressSet;
typedef std::set<Dyninst::ProcControlAPI::Thread::ptr> int_threadSet;
typedef std::set<Dyninst::ProcControlAPI::Process::ptr> int_processSet;

namespace Dyninst {
namespace ProcControlAPI {

class ProcessSet;
class ThreadSet;
class LibraryTrackingSet;
class ThreadTrackingSet;
class CallStackUnwindingSet;
class FollowForkSet;
class LWPTrackingSet;
class RemoteIOSet;
class MemoryUsageSet;
class PSetFeatures;
class TSetFeatures;

typedef boost::shared_ptr<ProcessSet> ProcessSet_ptr;
typedef boost::shared_ptr<ThreadSet> ThreadSet_ptr;
typedef boost::shared_ptr<const ProcessSet> ProcessSet_const_ptr;
typedef boost::shared_ptr<const ThreadSet> ThreadSet_const_ptr;

class PC_EXPORT AddressSet
{
  private:
   int_addressSet *iaddrs;
   friend void boost::checked_delete<AddressSet>(AddressSet *) CHECKED_DELETE_NOEXCEPT;
   friend class ProcessSet;
   AddressSet();
   ~AddressSet();
  public:
   int_addressSet *get_iaddrs() { return iaddrs; }

   typedef boost::shared_ptr<AddressSet> ptr;
   typedef boost::shared_ptr<AddressSet> const_ptr;
   

   static AddressSet::ptr newAddressSet();
   static AddressSet::ptr newAddressSet(ProcessSet_const_ptr ps, Dyninst::Address addr);
   static AddressSet::ptr newAddressSet(Process::const_ptr proc, Dyninst::Address addr);
   static AddressSet::ptr newAddressSet(ProcessSet_const_ptr ps, std::string library_name, Dyninst::Offset off = 0);
   //More redundant factories, to work around gcc 4.1 bug
   static AddressSet::ptr newAddressSet(ProcessSet_ptr ps, Dyninst::Address addr);
   static AddressSet::ptr newAddressSet(Process::ptr proc, Dyninst::Address addr);
   static AddressSet::ptr newAddressSet(ProcessSet_ptr ps, std::string library_name, Dyninst::Offset off = 0);
   
   typedef std::pair<Address, Process::ptr> value_type;
   typedef std::multimap<Dyninst::Address, Process::ptr>::iterator iterator;
   typedef std::multimap<Dyninst::Address, Process::ptr>::const_iterator const_iterator;

   iterator begin();
   iterator end();
   iterator find(Dyninst::Address a);
   iterator find(Dyninst::Address a, Process::const_ptr p);
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator find(Dyninst::Address a) const;
   const_iterator find(Dyninst::Address a, Process::const_ptr p) const;

   size_t count(Dyninst::Address a) const;
   size_t size() const;
   bool empty() const;

   std::pair<iterator, bool> insert(Dyninst::Address a, Process::const_ptr p);
   size_t insert(Dyninst::Address a, ProcessSet_const_ptr ps);
   std::pair<iterator, bool> insert(Dyninst::Address a, Process::ptr p);
   size_t insert(Dyninst::Address a, ProcessSet_ptr ps);
   void erase(iterator pos);
   size_t erase(Process::const_ptr p);
   size_t erase(Dyninst::Address a, Process::const_ptr p);
   void clear();

   iterator lower_bound(Dyninst::Address a);
   iterator upper_bound(Dyninst::Address a);
   std::pair<iterator, iterator> equal_range(Dyninst::Address a);
   const_iterator lower_bound(Dyninst::Address a) const;
   const_iterator upper_bound(Dyninst::Address a) const;
   std::pair<const_iterator, const_iterator> equal_range(Dyninst::Address a) const;

   AddressSet::ptr set_union(AddressSet::const_ptr pp) const;
   AddressSet::ptr set_intersection(AddressSet::const_ptr pp) const;
   AddressSet::ptr set_difference(AddressSet::const_ptr pp) const;
};


class PC_EXPORT ProcessSet : public boost::enable_shared_from_this<ProcessSet>
{
   friend class ThreadSet;
  private:
   int_processSet *procset;
   PSetFeatures *features;

   ProcessSet();
   ~ProcessSet();

   friend void boost::checked_delete<ProcessSet>(ProcessSet *) CHECKED_DELETE_NOEXCEPT;
 public:
   int_processSet *getIntProcessSet();
   typedef boost::shared_ptr<ProcessSet> ptr;
   typedef boost::shared_ptr<const ProcessSet> const_ptr;
   typedef boost::weak_ptr<ProcessSet> weak_ptr;
   typedef boost::weak_ptr<const ProcessSet> const_weak_ptr;

   static ProcessSet::ptr newProcessSet();
   static ProcessSet::ptr newProcessSet(Process::const_ptr p);
   static ProcessSet::ptr newProcessSet(ProcessSet::const_ptr pp);
   static ProcessSet::ptr newProcessSet(const std::set<Process::const_ptr> &procs);
   static ProcessSet::ptr newProcessSet(AddressSet::const_iterator, AddressSet::const_iterator);

   static ProcessSet::ptr newProcessSet(Process::ptr p); 
   static ProcessSet::ptr newProcessSet(ProcessSet::ptr pp);
   static ProcessSet::ptr newProcessSet(const std::set<Process::ptr> &procs);

   struct CreateInfo {
      std::string executable;
      std::vector<std::string> argv;
      std::vector<std::string> envp;
      std::map<int, int> fds;
      ProcControlAPI::err_t error_ret;
      Process::ptr proc;
   };
   static ProcessSet::ptr createProcessSet(std::vector<CreateInfo> &cinfo);

   struct AttachInfo {
      Dyninst::PID pid;
      std::string executable;
      ProcControlAPI::err_t error_ret;
      Process::ptr proc;
   };
   static ProcessSet::ptr attachProcessSet(std::vector<AttachInfo> &ainfo);

   ProcessSet::ptr set_union(ProcessSet::ptr pp) const;
   ProcessSet::ptr set_intersection(ProcessSet::ptr pp) const;
   ProcessSet::ptr set_difference(ProcessSet::ptr pp) const;

   class PC_EXPORT iterator {
      friend class Dyninst::ProcControlAPI::ProcessSet;
     private:
      int_processSet::iterator int_iter;
      iterator(int_processSet::iterator i);
     public:
	  Process::ptr operator*() const;
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      ProcessSet::iterator operator++();
      ProcessSet::iterator operator++(int);

	  typedef Process::ptr value_type;
	  typedef int difference_type;
	  typedef Process::ptr *pointer;
	  typedef Process::ptr &reference;
	  typedef std::forward_iterator_tag iterator_category;
   };

   class PC_EXPORT const_iterator {
      friend class Dyninst::ProcControlAPI::ProcessSet;
     private:
      int_processSet::iterator int_iter;
	  const_iterator(int_processSet::iterator i) : int_iter(i) {}
     public:
      const_iterator();
      ~const_iterator();
      const_iterator(const const_iterator&) = default;
      Process::ptr operator*() const;
      bool operator==(const const_iterator &i) const;
      bool operator!=(const const_iterator &i) const;
      ProcessSet::const_iterator operator++();
      ProcessSet::const_iterator operator++(int);

	  typedef Process::ptr value_type;
	  typedef int difference_type;
	  typedef Process::ptr *pointer;
	  typedef Process::ptr &reference;
	  typedef std::forward_iterator_tag iterator_category;
   };

   iterator begin();
   iterator end();
   iterator find(Process::const_ptr p);
   iterator find(Dyninst::PID p);
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator find(Process::const_ptr p) const;
   const_iterator find(Dyninst::PID p) const;

   bool empty() const;
   size_t size() const;

   std::pair<iterator, bool> insert(Process::const_ptr p);
   void erase(iterator pos);
   size_t erase(Process::const_ptr);
   void clear();

   ProcessSet::ptr getErrorSubset() const;
   void getErrorSubsets(std::map<ProcControlAPI::err_t, ProcessSet::ptr> &err_sets) const;

   bool anyTerminated() const;
   bool anyExited() const;
   bool anyCrashed() const;
   bool anyDetached() const;
   bool anyThreadStopped() const;
   bool anyThreadRunning() const;
   bool allTerminated() const;
   bool allExited() const;
   bool allCrashed() const;
   bool allDetached() const;
   bool allThreadsStopped() const;
   bool allThreadsRunning() const;

   ProcessSet::ptr getTerminatedSubset() const;
   ProcessSet::ptr getExitedSubset() const;
   ProcessSet::ptr getCrashedSubset() const;
   ProcessSet::ptr getDetachedSubset() const;
   ProcessSet::ptr getAllThreadRunningSubset() const;
   ProcessSet::ptr getAnyThreadRunningSubset() const;
   ProcessSet::ptr getAllThreadStoppedSubset() const;
   ProcessSet::ptr getAnyThreadStoppedSubset() const;

   bool continueProcs() const;
   bool stopProcs() const;
   bool detach(bool leaveStopped = false) const;
   bool terminate() const;
   bool temporaryDetach() const;
   bool reAttach() const;

   AddressSet::ptr mallocMemory(size_t size) const;
   bool mallocMemory(size_t size, AddressSet::ptr location) const;
   bool freeMemory(AddressSet::ptr addrs) const;

   struct write_t {
      void *buffer;
      Dyninst::Address addr;
      size_t size;
      err_t err;
      bool operator<(const write_t &w) { return (addr < w.addr) && (size < w.size) && (buffer < w.buffer); }
   };
   struct read_t {
      Dyninst::Address addr;
      void *buffer;
      size_t size;
      err_t err;
      bool operator<(const read_t &w) { return (addr < w.addr) && (size < w.size) && (buffer < w.buffer); }
   };

   bool readMemory(AddressSet::ptr addr, std::multimap<Process::ptr, void *> &result, size_t size) const;
   bool readMemory(AddressSet::ptr addr, std::map<void *, ProcessSet::ptr> &result, size_t size, bool use_checksum = true) const;
   bool readMemory(std::multimap<Process::const_ptr, read_t> &addrs) const;

   bool writeMemory(AddressSet::ptr addr, const void *buffer, size_t size) const;
   bool writeMemory(std::multimap<Process::const_ptr, write_t> &addrs) const;

   bool addBreakpoint(AddressSet::ptr addrs, Breakpoint::ptr bp) const;
   bool rmBreakpoint(AddressSet::ptr addrs, Breakpoint::ptr bp) const;

   bool postIRPC(const std::multimap<Process::const_ptr, IRPC::ptr> &rpcs) const;
   bool postIRPC(IRPC::ptr irpc, std::multimap<Process::ptr, IRPC::ptr> *result = NULL) const;
   bool postIRPC(IRPC::ptr irpc, AddressSet::ptr addrs, std::multimap<Process::ptr, IRPC::ptr> *result = NULL) const;

   LibraryTrackingSet *getLibraryTracking();
   ThreadTrackingSet *getThreadTracking();
   LWPTrackingSet *getLWPTracking();
   FollowForkSet *getFollowFork();
   RemoteIOSet *getRemoteIO();
   MemoryUsageSet *getMemoryUsage();
   const LibraryTrackingSet *getLibraryTracking() const;
   const ThreadTrackingSet *getThreadTracking() const;
   const LWPTrackingSet *getLWPTracking() const;
   const FollowForkSet *getFollowFork() const;
   const RemoteIOSet *getRemoteIO() const;
   const MemoryUsageSet *getMemoryUsage() const;
};

ProcessSet::const_ptr getAllProcs();

class PC_EXPORT ThreadSet : public boost::enable_shared_from_this<ThreadSet> {
  private:
   int_threadSet *ithrset;
   TSetFeatures *features;
   
   ThreadSet();
   ~ThreadSet();
   friend void boost::checked_delete<ThreadSet>(ThreadSet *) CHECKED_DELETE_NOEXCEPT;
  public:
   typedef boost::shared_ptr<ThreadSet> ptr;
   typedef boost::shared_ptr<const ThreadSet> const_ptr;
   typedef boost::weak_ptr<ThreadSet> weak_ptr;
   typedef boost::weak_ptr<const ThreadSet> const_weak_ptr;

   int_threadSet *getIntThreadSet() const;

   static ThreadSet::ptr newThreadSet();
   static ThreadSet::ptr newThreadSet(Thread::ptr thr);
   static ThreadSet::ptr newThreadSet(const ThreadPool &threadp);
   static ThreadSet::ptr newThreadSet(const std::set<Thread::const_ptr> &threads);
   static ThreadSet::ptr newThreadSet(ProcessSet::ptr ps, bool initial_only = false);

   ThreadSet::ptr set_union(ThreadSet::ptr tp) const;
   ThreadSet::ptr set_intersection(ThreadSet::ptr tp) const;
   ThreadSet::ptr set_difference(ThreadSet::ptr tp) const;

   class PC_EXPORT iterator {
      friend class Dyninst::ProcControlAPI::ThreadSet;
     protected:
      std::set<Thread::ptr>::iterator int_iter;
      iterator(int_threadSet::iterator i);
     public:
      Thread::ptr operator*();
      bool operator==(const iterator &i) const;
      bool operator!=(const iterator &i) const;
      ThreadSet::iterator operator++();
      ThreadSet::iterator operator++(int);
   };

   class PC_EXPORT const_iterator {
      friend class Dyninst::ProcControlAPI::ThreadSet;
     protected:
      std::set<Thread::ptr>::iterator int_iter;
      const_iterator(int_threadSet::iterator i);
     public:
      const_iterator();
      ~const_iterator();
      const_iterator(const const_iterator&) = default;
      Thread::ptr operator*();
      bool operator==(const const_iterator &i) const;
      bool operator!=(const const_iterator &i) const;
      ThreadSet::const_iterator operator++();
      ThreadSet::const_iterator operator++(int);
   };

   iterator begin();
   iterator end();
   iterator find(Thread::const_ptr p);
   const_iterator begin() const;
   const_iterator end() const;
   const_iterator find(Thread::const_ptr p) const;
   bool empty() const;
   size_t size() const;

   std::pair<iterator, bool> insert(Thread::const_ptr p);
   void erase(iterator pos);
   size_t erase(Thread::const_ptr t);
   void clear();

   ThreadSet::ptr getErrorSubset() const;
   void getErrorSubsets(std::map<ProcControlAPI::err_t, ThreadSet::ptr> &err_sets) const;

   bool allStopped() const;
   bool allRunning() const;
   bool allTerminated() const;
   bool allSingleStepMode() const;
   bool allHaveUserThreadInfo() const;
   bool anyStopped() const;
   bool anyRunning() const;
   bool anyTerminated() const;
   bool anySingleStepMode() const;
   bool anyHaveUserThreadInfo() const;
   ThreadSet::ptr getStoppedSubset() const;
   ThreadSet::ptr getRunningSubset() const;
   ThreadSet::ptr getTerminatedSubset() const;
   ThreadSet::ptr getSingleStepSubset() const;
   ThreadSet::ptr getHaveUserThreadInfoSubset() const;

   bool getStartFunctions(AddressSet::ptr result) const;
   bool getStackBases(AddressSet::ptr result) const;
   bool getTLSs(AddressSet::ptr result) const;

   bool stopThreads() const;
   bool continueThreads() const;
   bool setSingleStepMode(bool v) const;

   bool getRegister(Dyninst::MachRegister reg, std::map<Thread::ptr, Dyninst::MachRegisterVal> &results) const;
   bool getRegister(Dyninst::MachRegister reg, std::map<Dyninst::MachRegisterVal, ThreadSet::ptr> &results) const;
   bool setRegister(Dyninst::MachRegister reg, const std::map<Thread::const_ptr, Dyninst::MachRegisterVal> &vals) const;
   bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const;

   bool getAllRegisters(std::map<Thread::ptr, RegisterPool> &results) const;
   bool setAllRegisters(const std::map<Thread::const_ptr, RegisterPool> &reg_vals) const;

   bool postIRPC(const std::multimap<Thread::const_ptr, IRPC::ptr> &rpcs) const;
   bool postIRPC(IRPC::ptr irpc, std::multimap<Thread::ptr, IRPC::ptr> *result = NULL) const;

   CallStackUnwindingSet *getCallStackUnwinding();
   const CallStackUnwindingSet *getCallStackUnwinding() const;
};

}
}

#endif
