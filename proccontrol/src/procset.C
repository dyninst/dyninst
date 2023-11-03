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
#include "PlatFeatures.h"
#include "Mailbox.h"
#include "Generator.h"
#include "int_process.h"
#include "procpool.h"
#include "int_handler.h"
#include "irpc.h"
#include "response.h"
#include "processplat.h"
#include "int_event.h"
#include <stdlib.h>
#include <map>
#include <algorithm>

#include <boost/crc.hpp>
#include <iterator>


using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

namespace Dyninst {
namespace ProcControlAPI {

class PSetFeatures {
   friend class ProcessSet;
private:
   PSetFeatures();
   ~PSetFeatures();
   LibraryTrackingSet *libset;
   ThreadTrackingSet *thrdset;
   LWPTrackingSet *lwpset;
   FollowForkSet *forkset;
   RemoteIOSet *ioset;
   MemoryUsageSet *memset;
};

class TSetFeatures {
   friend class ThreadSet;
private:
   TSetFeatures();
   ~TSetFeatures();
   CallStackUnwindingSet *stkset;
};

}
}

PSetFeatures::PSetFeatures() :
   libset(NULL),
   thrdset(NULL),
   lwpset(NULL),
   forkset(NULL),
   ioset(NULL),
   memset(NULL)
{
}

PSetFeatures::~PSetFeatures()
{
   if (libset) {
      delete libset;
      libset = NULL;
   }
   if (thrdset) {
      delete thrdset;
      thrdset = NULL;
   }
   if (forkset) {
      delete forkset;
      forkset = NULL;
   }
   if (ioset) {
      delete ioset;
      ioset = NULL;
   }
   if (memset) {
      delete memset;
      memset = NULL;
   }
}

TSetFeatures::TSetFeatures() :
   stkset(NULL)
{
}

TSetFeatures::~TSetFeatures()
{
   if (stkset) {
      delete stkset;
      stkset = NULL;
   }
}

/**
 * AddressSet implementation follows.  This is essentially a std::multimap of Address -> Process::ptr
 * plus some additional features:
 *  - Ability to create addresses based on ProcControlAPI objects, such as libraries.
 *  - Additional range features to make it easier to group addresses
 *  - No duplicates of Address, Process:ptr pairs are allowed, though there are
 *    duplicate Address keys.
 *
 * I'd recommend your to your favorite multimap documentation for most of this.
 **/
typedef pair<Address, Process::ptr> apair_t;

AddressSet::AddressSet() :
   iaddrs(NULL)
{
}

AddressSet::ptr AddressSet::newAddressSet() {
   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(ProcessSet::const_ptr ps, Address addr)
{
   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   for (ProcessSet::const_iterator i = ps->begin(); i != ps->end(); i++) {
      newset->iaddrs->insert(value_type(addr, *i));
   }
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(ProcessSet::const_ptr ps, string library_name, Offset off)
{
   MTLock lock_this_func;

   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   for (ProcessSet::const_iterator i = ps->begin(); i != ps->end(); i++) {
      int_process *p = (*i)->llproc();
      if (!p) 
         continue;
      int_library *lib = p->getLibraryByName(library_name);
      if (!lib)
         continue;
      newset->iaddrs->insert(value_type(lib->getAddr() + off, *i));
   }
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(Process::const_ptr p, Address addr)
{
   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   newset->iaddrs->insert(value_type(addr, p->llproc()->proc()));
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(ProcessSet::ptr ps, Address addr)
{
   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   for (ProcessSet::iterator i = ps->begin(); i != ps->end(); i++) {
      newset->iaddrs->insert(value_type(addr, *i));
   }
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(ProcessSet::ptr ps, string library_name, Offset off)
{
   MTLock lock_this_func;

   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   for (ProcessSet::iterator i = ps->begin(); i != ps->end(); i++) {
      int_process *p = (*i)->llproc();
      if (!p) 
         continue;
      int_library *lib = p->getLibraryByName(library_name);
      if (!lib)
         continue;
      newset->iaddrs->insert(value_type(lib->getAddr() + off, *i));
   }
   return newset;
}

AddressSet::ptr AddressSet::newAddressSet(Process::ptr p, Address addr)
{
   AddressSet::ptr newset = AddressSet::ptr(new AddressSet);
   newset->iaddrs = new int_addressSet();
   newset->iaddrs->insert(value_type(addr, p->llproc()->proc()));
   return newset;
}

AddressSet::~AddressSet()
{
   if (iaddrs) {
      delete iaddrs;
      iaddrs = NULL;
   }
}

AddressSet::iterator AddressSet::begin()
{
   return iaddrs->begin();
}

AddressSet::iterator AddressSet::end()
{
   return iaddrs->end();
}

AddressSet::iterator AddressSet::find(Address a)
{
   return iaddrs->find(a);
}

AddressSet::iterator AddressSet::find(Address a, Process::const_ptr p)
{
   pair<iterator, iterator> range = equal_range(a);
   for (iterator i = range.first; i != range.second; i++) {
      if (i->second == p)
         return i;
   }
   return end();
}

AddressSet::const_iterator AddressSet::begin() const
{
   return iaddrs->begin();
}

AddressSet::const_iterator AddressSet::end() const
{
   return iaddrs->end();
}

AddressSet::const_iterator AddressSet::find(Address a) const
{
   return iaddrs->find(a);
}

AddressSet::const_iterator AddressSet::find(Address a, Process::const_ptr p) const
{
   pair<const_iterator, const_iterator> range = equal_range(a);
   for (const_iterator i = range.first; i != range.second; i++) {
      if (i->second == p)
         return i;
   }
   return end();
}

size_t AddressSet::count(Address a) const
{
   return iaddrs->count(a);
}

size_t AddressSet::size() const
{
   return iaddrs->size();
}

bool AddressSet::empty() const
{
   return iaddrs->empty();
}

pair<AddressSet::iterator, bool> AddressSet::insert(Address a, Process::const_ptr p)
{
   Process::ptr ncp = pc_const_cast<Process>(p);
   pair<iterator, bool> result;
   for (result.first = iaddrs->find(a); result.first != iaddrs->end() && result.first->first == a; result.first++) {
      if (result.first->second == ncp) {
         result.second = false;
         return result;
      }
   }
   result.first = iaddrs->insert(value_type(a, ncp));
   result.second = true;
   return result;
}

size_t AddressSet::insert(Address a, ProcessSet::const_ptr ps)
{
   size_t count_added = 0;
   for (ProcessSet::const_iterator i = ps->begin(); i != ps->end(); i++) {
      Process::ptr proc = *i;
      pair<AddressSet::iterator, bool> result = insert(a, *i);
      if (result.second)
         count_added++;
   }
   return count_added;
}

pair<AddressSet::iterator, bool> AddressSet::insert(Address a, Process::ptr p)
{
   Process::ptr ncp = pc_const_cast<Process>(p);
   pair<iterator, bool> result;
   for (result.first = iaddrs->find(a); result.first != iaddrs->end() && result.first->first == a; result.first++) {
      if (result.first->second == ncp) {
         result.second = false;
         return result;
      }
   }
   result.first = iaddrs->insert(value_type(a, ncp));
   result.second = true;
   return result;
}

size_t AddressSet::insert(Address a, ProcessSet::ptr ps)
{
   size_t count_added = 0;
   for (ProcessSet::iterator i = ps->begin(); i != ps->end(); i++) {
      Process::ptr proc = *i;
      pair<AddressSet::iterator, bool> result = insert(a, *i);
      if (result.second)
         count_added++;
   }
   return count_added;
}

void AddressSet::erase(AddressSet::iterator pos)
{
   iaddrs->erase(pos);
}

size_t AddressSet::erase(Process::const_ptr p)
{
   size_t num_erased = 0;
   Process::ptr ncp = pc_const_cast<Process>(p);
   iterator i = iaddrs->begin();
   while (i != iaddrs->end()) {
      if (i->second == ncp) {
         iterator j = i++;
         iaddrs->erase(j);
         num_erased++;
      }
      else {
         i++;
      }
   }
   return num_erased;
}

size_t AddressSet::erase(Address a, Process::const_ptr p)
{
   Process::ptr ncp = pc_const_cast<Process>(p);
   iterator i = find(a, p);
   if (i == end())
      return 0;
   erase(i);
   return 1;
}

void AddressSet::clear()
{
   iaddrs->clear();
}

AddressSet::iterator AddressSet::lower_bound(Address a)
{
   return iaddrs->lower_bound(a);
}

AddressSet::iterator AddressSet::upper_bound(Address a)
{
   return iaddrs->upper_bound(a);
}

pair<AddressSet::iterator, AddressSet::iterator> AddressSet::equal_range(Address a)
{
   return iaddrs->equal_range(a);
}

AddressSet::const_iterator AddressSet::lower_bound(Address a) const
{
   return iaddrs->lower_bound(a);
}

AddressSet::const_iterator AddressSet::upper_bound(Address a) const
{
   return iaddrs->upper_bound(a);
}

pair<AddressSet::const_iterator, AddressSet::const_iterator> AddressSet::equal_range(Address a) const
{
   return iaddrs->equal_range(a);
}

AddressSet::ptr AddressSet::set_union(AddressSet::const_ptr pp) const
{
   AddressSet::ptr newset = AddressSet::newAddressSet();

   std::set_union(iaddrs->begin(), iaddrs->end(),
                  pp->iaddrs->begin(), pp->iaddrs->end(),
                  inserter(*newset->iaddrs, newset->iaddrs->end()));

   return newset;
}

AddressSet::ptr AddressSet::set_intersection(AddressSet::const_ptr pp) const
{
   AddressSet::ptr newset = AddressSet::newAddressSet();

   std::set_intersection(iaddrs->begin(), iaddrs->end(),
                         pp->iaddrs->begin(), pp->iaddrs->end(),
                         inserter(*newset->iaddrs, newset->iaddrs->end()));

   return newset;
}

AddressSet::ptr AddressSet::set_difference(AddressSet::const_ptr pp) const
{
   AddressSet::ptr newset = AddressSet::newAddressSet();

   std::set_difference(iaddrs->begin(), iaddrs->end(),
                       pp->iaddrs->begin(), pp->iaddrs->end(),
                       inserter(*newset->iaddrs, newset->iaddrs->end()));

   return newset;
}


/**
 * The following bundle of hackery is a wrapper around iterators that
 * checks processes for common errors.  Many of the functions that implement
 * ProcessSet and ThreadSet have do an operation where they iterate over
 * a collection, pull some Process::ptr or Thread::ptr out of that collection,
 * check it for common errors (e.g, operating on a dead process), then do some
 * real work. These classes/templates are an attempt to bring all that error checking
 * and iteration into a common place.
 *
 * Things are complicated by the fact that we use many different types of collections
 * We might be operating over sets of Process::ptrs, or multimaps from Thread::ptr to 
 * register values, etc.  The common iteration and error handling code is in the iter_t
 * template, which takes the type of collection it's iterating over as template parameters.
 *
 * The three big operations iter_t needs to do is extract a process from an iterator and
 * get the begin/end iterators from a collection.  These operations are done by a set
 * of overloaded functions: get_proc, get_begin, and get_end.  We have an instance of these
 * for each type of collection we deal with.
 **/
template<class T>
static Process::const_ptr get_proc(const T &i, err_t *) {
   return i->first;
}

template<class T>
static typename T::iterator get_begin(T *m) {
   return m->begin();
}

template<class T>
static typename T::iterator get_end(T *m) {
   return m->end();
}

template<class T>
static typename T::const_iterator get_begin(const T *m) {
   return m->begin();
}

template<class T>
static typename T::const_iterator get_end(const T *m) {
   return m->end();
}

static Process::const_ptr get_proc(const int_addressSet::iterator &i, err_t *) {
   return i->second;
}

static int_addressSet::iterator get_begin(AddressSet::ptr as) {
   return as->get_iaddrs()->begin();
}

static int_addressSet::iterator get_end(AddressSet::ptr as) {
   return as->get_iaddrs()->end();
}

static void thread_err_check(int_thread *ithr, err_t *thread_error) {
   if (!ithr) {
      *thread_error = err_exited;
      return;
   }
   if (ithr->getUserState().getState() == int_thread::running) {
      *thread_error = err_notrunning;
   }
}

static Process::const_ptr get_proc(const map<Thread::const_ptr, MachRegisterVal>::const_iterator &i, err_t *thread_error)
{
   if (thread_error)
      thread_err_check(i->first->llthrd(), thread_error);
   return i->first->getProcess();
}

static Process::const_ptr get_proc(const map<Thread::const_ptr, RegisterPool>::const_iterator &i, err_t *thread_error)
{
   if (thread_error)
      thread_err_check(i->first->llthrd(), thread_error);
   return i->first->getProcess();
}

static Process::const_ptr get_proc(const multimap<Thread::const_ptr, IRPC::ptr>::const_iterator &i, err_t *thread_error = NULL) 
{
   if (thread_error)
      thread_err_check(i->first->llthrd(), thread_error);
   return i->first->getProcess();
}

static Process::const_ptr get_proc(const int_processSet::iterator &i, err_t *) {
   return *i;
}

static Process::const_ptr get_proc(const int_threadSet::iterator &i, err_t *thread_error = NULL) {
   if (thread_error)
      thread_err_check((*i)->llthrd(), thread_error);
   return (*i)->getProcess();
}

#define ERR_CHCK_EXITED       (1<<0)
#define ERR_CHCK_DETACHED     (1<<1)
#define ERR_CHCK_STOPPED      (1<<2)
#define ERR_CHCK_THRD_STOPPED (1<<3)
#define CLEAR_ERRS            (1<<4)
#define THREAD_CLEAR_ERRS     (1<<5)

#define ERR_CHCK_NORM (ERR_CHCK_EXITED | ERR_CHCK_DETACHED | CLEAR_ERRS)
#define ERR_CHCK_ALL  (ERR_CHCK_NORM | ERR_CHCK_STOPPED)
#define ERR_CHCK_THRD (ERR_CHCK_EXITED | ERR_CHCK_DETACHED | THREAD_CLEAR_ERRS)

template<class cont_t, class iterator_t>
class iter_t {
private:
   const char *msg;
   bool &had_error;
   unsigned int flags;
   cont_t container;
   iterator_t iter;
   bool finished_clear;
   bool did_begin;

   bool proc_check(Process::const_ptr p, err_t thr_error) {
      int_process *proc = p ? p->llproc() : NULL;
      if (!proc) {
         perr_printf("%s attempted on exited process\n", msg);
         if (p) p->setLastError(err_exited, "Operation attempted on exited process");
         had_error = true;
         return false;
      }
      if ((flags & CLEAR_ERRS) && !finished_clear) {
         proc->clearLastError();
      }
      if ((flags & ERR_CHCK_EXITED) && thr_error == err_exited) {
         perr_printf("%s attempted on exited thread in process %d\n", msg, p->getPid());
         p->setLastError(err_exited, "Group operation attempted on exited thread");
         had_error = true;
         return false;
      }
      if ((flags & ERR_CHCK_THRD_STOPPED) && thr_error == err_notrunning) {
         perr_printf("%s attempted on running thread in process %d\n", msg, p->getPid());
         p->setLastError(err_notrunning, "Group operation attempted on running thread");
         had_error = true;
         return false;
      }
      if ((flags & ERR_CHCK_EXITED) && !proc) {
         perr_printf("%s attempted on exited process %d\n", msg, p->getPid());
         p->setLastError(err_exited, "Group operation attempted on exited process");
         had_error = true;
         return false;
      }
      if ((flags & ERR_CHCK_DETACHED) && p->isDetached()) {
         perr_printf("%s attempted on detached process %d\n", msg, proc->getPid());
         p->setLastError(err_detached, "Group operation attempted on detached process");
         had_error = true;
         return false;
      }
      if ((flags & ERR_CHCK_STOPPED) && p->hasRunningThread()) {
         perr_printf("%s attempted on running process %d\n", msg, proc->getPid());
         p->setLastError(err_notstopped, "Group operation attempted on running process");
         had_error = true;
         return false;
      } 
      return true;
   }

public:
   typedef iterator_t i_t;

   iter_t(const char *m, bool &e, unsigned int f) :
      msg(m),
      had_error(e),
      flags(f),
      container(),
      finished_clear(false),
      did_begin(false)
   {
   }
   
   ~iter_t() {
   }

   iterator_t begin(cont_t c) {
      container = c;
      if (!finished_clear && (flags & THREAD_CLEAR_ERRS)) {
         for (iter = get_begin(container); iter != get_end(container); iter++) {
            Process::const_ptr proc = get_proc(iter, NULL);
            proc->clearLastError();
         }
         finished_clear = true;
      }
      else {
         if (did_begin)
            finished_clear = true;
      }
      did_begin = true;

      iter = get_begin(container);
      err_t thr_error = err_none;
      Process::const_ptr proc = get_proc(iter, &thr_error);
      if (!proc_check(proc, thr_error))
         return inc();
      return iter;
   }
   
   iterator_t end() {
      return get_end(container);
   }

   iterator_t inc() {
      iterator_t end = this->end();
      bool result = false;
      do {
         iter++;
         if (iter == end) {
            return iter;
         }
         err_t thr_error = err_none;
         Process::const_ptr proc = get_proc(iter, &thr_error);
         result = proc_check(proc, thr_error);
      } while (!result);
      return iter;
   }
};

typedef iter_t<AddressSet::ptr, int_addressSet::iterator> addrset_iter;
typedef iter_t<int_processSet *, int_processSet::iterator> procset_iter;
typedef iter_t<multimap<Process::const_ptr, ProcessSet::read_t> *, multimap<Process::const_ptr, ProcessSet::read_t>::iterator> readmap_iter;
typedef iter_t<multimap<Process::const_ptr, ProcessSet::write_t> *, multimap<Process::const_ptr, ProcessSet::write_t>::iterator > writemap_iter;
typedef iter_t<const multimap<Process::const_ptr, IRPC::ptr> *, multimap<Process::const_ptr, IRPC::ptr>::const_iterator > rpcmap_iter;
typedef iter_t<const multimap<Thread::const_ptr, IRPC::ptr> *, multimap<Thread::const_ptr, IRPC::ptr>::const_iterator > rpcmap_thr_iter;
typedef iter_t<int_threadSet *, int_threadSet::iterator> thrset_iter;
typedef iter_t<const map<Thread::const_ptr, Dyninst::MachRegisterVal> *, map<Thread::const_ptr, Dyninst::MachRegisterVal>::const_iterator> setreg_iter;
typedef iter_t<const map<Thread::const_ptr, RegisterPool> *, map<Thread::const_ptr, RegisterPool>::const_iterator> setallreg_iter;

ProcessSet::ProcessSet() :
   features(NULL)
{
   procset = new int_processSet;
}

ProcessSet::~ProcessSet()
{
   if (procset) {
      delete procset;
      procset = NULL;
   }
   if (features) {
      delete features;
      features = NULL;
   }
}

ProcessSet::ptr ProcessSet::newProcessSet()
{
   return ProcessSet::ptr(new ProcessSet());
}

ProcessSet::ptr ProcessSet::newProcessSet(Process::const_ptr pp)
{
   ProcessSet::ptr newps = newProcessSet();
   newps->insert(pp);
   return newps;
}

ProcessSet::ptr ProcessSet::newProcessSet(ProcessSet::const_ptr pp)
{
   return newProcessSet(*pp->procset);
}

ProcessSet::ptr ProcessSet::newProcessSet(const set<Process::ptr> &procs)
{
   ProcessSet::ptr newps = newProcessSet();
   copy(procs.begin(), procs.end(), inserter(*newps->procset, newps->procset->end()));
   return newps;
}

ProcessSet::ptr ProcessSet::newProcessSet(Process::ptr p)
{
   ProcessSet::ptr newps = newProcessSet();
   newps->insert(p);
   return newps;
}

ProcessSet::ptr ProcessSet::newProcessSet(ProcessSet::ptr pp)
{
   return newProcessSet(*pp->procset);
}

struct proc_strip_const {
   Process::ptr operator()(Process::const_ptr p) const {
      return pc_const_cast<Process>(p);
   }
};

ProcessSet::ptr ProcessSet::newProcessSet(const set<Process::const_ptr> &procs)
{
   ProcessSet::ptr newps = newProcessSet();
   int_processSet &newset = *newps->procset;
   transform(procs.begin(), procs.end(), inserter(newset, newset.end()), proc_strip_const());
   return newps;
}

ProcessSet::ptr ProcessSet::newProcessSet(AddressSet::const_iterator begin, AddressSet::const_iterator end)
{
   ProcessSet::ptr newps = newProcessSet();
   int_processSet &newset = *newps->procset;
   for (AddressSet::const_iterator i = begin; i != end; i++) {
      pair<Address, Process::ptr> ii = *i;
      Process::ptr p = ii.second;
      newset.insert((*i).second);
   }
   return newps;
}

ProcessSet::ptr ProcessSet::createProcessSet(vector<CreateInfo> &cinfo)
{
   MTLock lock_this_func(MTLock::allow_init, MTLock::deliver_callbacks);

   pthrd_printf("User asked to launch %u executables\n", (unsigned) cinfo.size());

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process create while in CB, erroring.");
      for (vector<CreateInfo>::iterator i = cinfo.begin(); i != cinfo.end(); i++) {
         i->error_ret = err_incallback;
      }
      return ProcessSet::ptr();
   }

   ProcPool()->condvar()->lock();

   map<int_process *, vector<CreateInfo>::iterator> info_map;
   ProcessSet::ptr newps = newProcessSet();
   int_processSet &newset = *newps->procset;

   pthrd_printf("Creating new process objects\n");
   for (vector<CreateInfo>::iterator i = cinfo.begin(); i != cinfo.end(); i++) {
      Process::ptr newproc(new Process());
      int_process *llproc = int_process::createProcess(i->executable, i->argv, i->envp, i->fds);
      llproc->initializeProcess(newproc);
      info_map[llproc] = i;
      newset.insert(newproc);
   }

   pthrd_printf("Triggering create on new process objects\n");
   int_process::create(&newset); //Releases procpool lock

   for (ProcessSet::iterator i = newps->begin(); i != newps->end();) {
      int_process *proc = (*i)->llproc();
      map<int_process *, vector<CreateInfo>::iterator>::iterator j = info_map.find(proc);
      assert(j != info_map.end());
      CreateInfo &ci = *(j->second);

      err_t last_error = proc->getLastError();
      if (last_error == err_none) {
         ci.proc = proc->proc();
         ci.error_ret = err_none;
         i++;
         continue;
      }
      ci.error_ret = last_error;
      ci.proc = Process::ptr();
      newps->erase(i++);
   }

   return newps;
}

ProcessSet::ptr ProcessSet::attachProcessSet(vector<AttachInfo> &ainfo)
{
   MTLock lock_this_func(MTLock::allow_init, MTLock::deliver_callbacks);

   pthrd_printf("User asked to attach to %u processes\n", (unsigned) ainfo.size());

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process create while in CB, erroring.");
      for (vector<AttachInfo>::iterator i = ainfo.begin(); i != ainfo.end(); i++) {
         i->error_ret = err_incallback;
      }
      return ProcessSet::ptr();
   }

   ProcPool()->condvar()->lock();

   map<int_process *, vector<AttachInfo>::iterator> info_map;
   ProcessSet::ptr newps = newProcessSet();
   int_processSet &newset = *newps->procset;

   for (vector<AttachInfo>::iterator i = ainfo.begin(); i != ainfo.end(); i++) {
      Process::ptr newproc(new Process());
      int_process *llproc = int_process::createProcess(i->pid, i->executable);
      llproc->initializeProcess(newproc);
      info_map[llproc] = i;
      newset.insert(newproc);
   }

   int_process::attach(&newset, false); //Releases procpool lock

   for (ProcessSet::iterator i = newps->begin(); i != newps->end(); ) {
      int_process *proc = (*i)->llproc();
      map<int_process *, vector<AttachInfo>::iterator>::iterator j = info_map.find(proc);
      assert(j != info_map.end());
      AttachInfo &ai = *(j->second);
      
      err_t last_error = proc->getLastError();
      if (last_error == err_none && proc->getState() == int_process::errorstate) {
         last_error = err_noproc;
      }
      if (last_error == err_none) {
         ai.proc = proc->proc();
         ai.error_ret = err_none;
         i++;
         continue;
      }
      pthrd_printf("Erasing process %d from attach return set because err = %u\n",
                   proc->getPid(), last_error);
      ai.error_ret = last_error;
      ai.proc = Process::ptr();
      newps->erase(i++);
   }

   return newps;
}

ProcessSet::ptr ProcessSet::set_union(ProcessSet::ptr pp) const
{
   //No MTLock needed, not digging into internals.
   ProcessSet::ptr newps = ProcessSet::ptr(new ProcessSet);
   int_processSet *me = procset;
   int_processSet *you = pp->procset;
   int_processSet *them = newps->procset;

   std::set_union(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newps;
}

ProcessSet::ptr ProcessSet::set_intersection(ProcessSet::ptr pp) const
{
   //No MTLock needed, not digging into internals.
   ProcessSet::ptr newps = ProcessSet::ptr(new ProcessSet);
   int_processSet *me = procset;
   int_processSet *you = pp->procset;
   int_processSet *them = newps->procset;

   std::set_intersection(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newps;
}

ProcessSet::ptr ProcessSet::set_difference(ProcessSet::ptr pp) const
{
   //No MTLock needed, not digging into internals.
   ProcessSet::ptr newps = ProcessSet::ptr(new ProcessSet);
   int_processSet *me = procset;
   int_processSet *you = pp->procset;
   int_processSet *them = newps->procset;

   std::set_difference(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newps;
}

ProcessSet::iterator ProcessSet::begin()
{
   return iterator(procset->begin());
}

ProcessSet::iterator ProcessSet::end()
{
   return iterator(procset->end());
}

ProcessSet::iterator ProcessSet::find(Process::const_ptr p)
{
   return iterator(procset->find(pc_const_cast<Process>(p)));
}

ProcessSet::iterator ProcessSet::find(PID p)
{
   ProcPool()->condvar()->lock();
   int_process *llproc = ProcPool()->findProcByPid(p);
   ProcPool()->condvar()->unlock();
   if (!llproc) return end();
   return iterator(procset->find(llproc->proc()));
}

ProcessSet::const_iterator ProcessSet::begin() const
{
   return const_iterator(procset->begin());
}

ProcessSet::const_iterator ProcessSet::end() const
{
   return const_iterator(procset->end());
}

ProcessSet::const_iterator ProcessSet::find(Process::const_ptr p) const
{
   return const_iterator(procset->find(pc_const_cast<Process>(p)));
}

ProcessSet::const_iterator ProcessSet::find(PID p) const
{
   ProcPool()->condvar()->lock();
   int_process *llproc = ProcPool()->findProcByPid(p);
   ProcPool()->condvar()->unlock();
   if (!llproc) return end();
   return const_iterator(procset->find(llproc->proc()));
}

bool ProcessSet::empty() const
{
   return procset->empty();
}

size_t ProcessSet::size() const
{
   return procset->size();
}

pair<ProcessSet::iterator, bool> ProcessSet::insert(Process::const_ptr p)
{
   pair<int_processSet::iterator, bool> result = procset->insert(pc_const_cast<Process>(p));
   return pair<ProcessSet::iterator, bool>(ProcessSet::iterator(result.first), result.second);
}

void ProcessSet::erase(ProcessSet::iterator pos)
{
   procset->erase(pos.int_iter);
}

size_t ProcessSet::erase(Process::const_ptr p)
{
   ProcessSet::iterator i = find(p);
   if (i == end()) return 0;
   erase(i);
   return 1;
}

void ProcessSet::clear()
{
   procset->clear();
}

LibraryTrackingSet *ProcessSet::getLibraryTracking()
{
   if (features && features->libset)
      return features->libset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getLibraryTracking()) {
         features->libset = new LibraryTrackingSet(shared_from_this());
         return features->libset;
      }
   }

   return NULL;
}

ThreadTrackingSet *ProcessSet::getThreadTracking()
{
   if (features && features->thrdset)
      return features->thrdset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getThreadTracking()) {
         features->thrdset = new ThreadTrackingSet(shared_from_this());
         return features->thrdset;
      }
   }

   return NULL;
}

LWPTrackingSet *ProcessSet::getLWPTracking()
{
   if (features && features->lwpset)
      return features->lwpset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getLWPTracking()) {
         features->lwpset = new LWPTrackingSet(shared_from_this());
         return features->lwpset;
      }
   }

   return NULL;
}

FollowForkSet *ProcessSet::getFollowFork()
{
   if (features && features->forkset)
      return features->forkset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getFollowFork()) {
         features->forkset = new FollowForkSet(shared_from_this());
         return features->forkset;
      }
   }

   return NULL;
}

RemoteIOSet *ProcessSet::getRemoteIO()
{
   if (features && features->ioset)
      return features->ioset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getRemoteIO()) {
         features->ioset = new RemoteIOSet(shared_from_this());
         return features->ioset;
      }
   }

   return NULL;
}

MemoryUsageSet *ProcessSet::getMemoryUsage()
{
   if (features && features->memset)
      return features->memset;

   MTLock lock_this_func;
   if (!procset)
      return NULL;
   if (!features) {
      features = new PSetFeatures();
   }
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      if (p->getMemoryUsage()) {
         features->memset = new MemoryUsageSet(shared_from_this());
         return features->memset;
      }
   }

   return NULL;
}

const LibraryTrackingSet *ProcessSet::getLibraryTracking() const
{
   return const_cast<ProcessSet *>(this)->getLibraryTracking();
}

const ThreadTrackingSet *ProcessSet::getThreadTracking() const
{
   return const_cast<ProcessSet *>(this)->getThreadTracking();
}

const LWPTrackingSet *ProcessSet::getLWPTracking() const
{
   return const_cast<ProcessSet *>(this)->getLWPTracking();
}

const FollowForkSet *ProcessSet::getFollowFork() const
{
   return const_cast<ProcessSet *>(this)->getFollowFork();
}

const RemoteIOSet *ProcessSet::getRemoteIO() const
{
   return const_cast<ProcessSet *>(this)->getRemoteIO();
}

const MemoryUsageSet *ProcessSet::getMemoryUsage() const
{
   return const_cast<ProcessSet *>(this)->getMemoryUsage();
}
ProcessSet::ptr ProcessSet::getErrorSubset() const
{
   MTLock lock_this_func;

   ProcessSet::ptr newps = newProcessSet();
   for (ProcessSet::const_iterator i = begin(); i != end(); i++) {
      ProcControlAPI::err_t err = (*i)->getLastError();
      if (err == err_none)
         continue;
      newps->insert(*i);
   }
   return newps;
}

void ProcessSet::getErrorSubsets(map<ProcControlAPI::err_t, ProcessSet::ptr> &err_sets) const
{
   MTLock lock_this_func;

   for (const_iterator i = begin(); i != end(); i++) {
      Process::ptr proc = *i;
      ProcControlAPI::err_t err = proc->getLastError();
      if (err == err_none)
         continue;
      map<err_t, ProcessSet::ptr>::iterator j = err_sets.find(err);
      ProcessSet::ptr ps;
      if (j != err_sets.end()) {
         ps = j->second;
      }
      else {
         ps = ProcessSet::newProcessSet();
         err_sets[err] = ps;
      }
      ps->insert(proc);
   }
}

template<class iter, class pred>
static bool all_match(iter b, iter e, pred p)
{
   bool result = true;
   for (iter i = b; i != e; i++) {
      if (!p(*i))
         result = false;
   }
   return result;
}

template<class iter, class pred>
static bool any_match(iter b, iter e, pred p)
{
   bool result = false;
   for (iter i = b; i != e; i++) {
      if (p(*i))
         result = true;
   }
   return result;
}

template<class iter, class pred>
static ProcessSet::ptr create_subset(iter b, iter e, pred p) {
   ProcessSet::ptr newps = ProcessSet::newProcessSet();
   for (iter i = b; i != e; i++) {
      if (p(*i)) {
         newps->insert(*i);
      }
   }
   return newps;
}

template<class iter, class pred>
static ThreadSet::ptr create_thrsubset(iter b, iter e, pred p) {
   ThreadSet::ptr newts = ThreadSet::newThreadSet();
   for (iter i = b; i != e; i++) {
      if (p(*i)) {
         newts->insert(*i);
      }
   }
   return newts;
}

struct test_terminate {
   bool operator()(Process::ptr p) {
      p->clearLastError();
      int_process *llproc = p->llproc();
      if (!llproc)
         return true;
      return false;
   }


   bool operator()(Thread::ptr t) {
      Process::ptr p = t->getProcess();
      p->clearLastError();
      return (!p->llproc() || !t->llthrd());
   }
};

bool ProcessSet::anyTerminated() const
{
   MTLock lock_this_func;
   return any_match(procset->begin(), procset->end(), test_terminate());
}

bool ProcessSet::allTerminated() const
{
   MTLock lock_this_func;
   return all_match(procset->begin(), procset->end(), test_terminate());
}

ProcessSet::ptr ProcessSet::getTerminatedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_terminate());
}

struct test_exited {
   bool operator()(Process::ptr p) {
      p->clearLastError();
      int_process *llproc = p->llproc();
      if (!llproc)
         return p->exitstate()->exited;
      return false;
   }
};

bool ProcessSet::anyExited() const
{
   MTLock lock_this_func;
   if (procset->empty())
      return true;
   return any_match(procset->begin(), procset->end(), test_exited());
}

bool ProcessSet::allExited() const
{
   MTLock lock_this_func;
   if (procset->empty())
      return true;
   return all_match(procset->begin(), procset->end(), test_exited());
}

ProcessSet::ptr ProcessSet::getExitedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_exited());
}

struct test_crashed {
   bool operator()(Process::ptr p) {
      p->clearLastError();
      int_process *llproc = p->llproc();
      if (!llproc)
         return p->exitstate()->crashed;
      return false;
   }
};

bool ProcessSet::anyCrashed() const
{
   MTLock lock_this_func;
   return any_match(procset->begin(), procset->end(), test_crashed());
}

bool ProcessSet::allCrashed() const
{
   MTLock lock_this_func;
   return all_match(procset->begin(), procset->end(), test_crashed());
}

ProcessSet::ptr ProcessSet::getCrashedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_crashed());
}

struct test_detached {
   bool operator()(Process::ptr p) {
      p->clearLastError();
      int_process *llproc = p->llproc();
      if (!llproc)
         return false;
      return llproc->getState() == int_process::detached;
   }
};

bool ProcessSet::anyDetached() const
{
   MTLock lock_this_func;
   return any_match(procset->begin(), procset->end(), test_detached());
}

bool ProcessSet::allDetached() const
{
   MTLock lock_this_func;
   return all_match(procset->begin(), procset->end(), test_detached());
}

ProcessSet::ptr ProcessSet::getDetachedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_detached());
}

struct test_thr {
   enum test_t {
      any,
      all
   };
   test_t test;
   int_thread::State ts;

   test_thr(test_t t_, int_thread::State ts_) : test(t_), ts(ts_) {}

   bool operator()(Process::ptr p) {
      p->clearLastError();
      int_process *llproc = p->llproc();
      if (!llproc)
         return false;
      int_threadPool *tp = llproc->threadPool();
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         bool match = ((*i)->getUserState().getState() == ts);
         if (match && test == any)
            return true;
         if (!match && test == all)
            return false;
      }
      return (test == all);
   }

   bool operator()(Thread::ptr t) {
      int_thread *llthrd = t->llthrd();
      if (!llthrd)
         return false;
      int_process *llproc = llthrd->llproc();
      llproc->clearLastError();
      bool match = (llthrd->getUserState().getState() == ts);
      if (match && test == any)
         return true;
      if (!match && test == all)
         return false;
      return (test == all);
   }
};

bool ProcessSet::anyThreadStopped() const
{
   MTLock lock_this_func;
   return any_match(procset->begin(), procset->end(), test_thr(test_thr::any, int_thread::stopped));
}

bool ProcessSet::allThreadsStopped() const
{
   MTLock lock_this_func;
   return all_match(procset->begin(), procset->end(), test_thr(test_thr::all, int_thread::stopped));
}

ProcessSet::ptr ProcessSet::getAllThreadStoppedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_thr(test_thr::all, int_thread::stopped));
}

ProcessSet::ptr ProcessSet::getAnyThreadStoppedSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_thr(test_thr::any, int_thread::stopped));
}

bool ProcessSet::anyThreadRunning() const
{
   MTLock lock_this_func;
   return any_match(procset->begin(), procset->end(), test_thr(test_thr::any, int_thread::running));
}

bool ProcessSet::allThreadsRunning() const
{
   MTLock lock_this_func;
   return all_match(procset->begin(), procset->end(), test_thr(test_thr::all, int_thread::running));
}

ProcessSet::ptr ProcessSet::getAllThreadRunningSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_thr(test_thr::all, int_thread::running));
}

ProcessSet::ptr ProcessSet::getAnyThreadRunningSubset() const
{
   MTLock lock_this_func;
   return create_subset(procset->begin(), procset->end(), test_thr(test_thr::any, int_thread::running));
}

bool ProcessSet::continueProcs() const
{
   bool had_error = false;
   MTLock lock_this_func(MTLock::deliver_callbacks);

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot continueProc from callback\n"));
      return false;
   }

   procset_iter iter("continueProc", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();

      pthrd_printf("User continuing entire process %d\n", proc->getPid());
      proc->threadPool()->initialThread()->getUserState().setStateProc(int_thread::running);
      proc->throwNopEvent();
   }
   return !had_error;
}

bool ProcessSet::stopProcs() const
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   bool had_error = false;
   bool had_success = false;
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot continueProc from callback\n"));
      return false;
   }
   int_processSet error_set;
   procset_iter iter("stopProc", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();
      pthrd_printf("User stopping entire process %d\n", proc->getPid());
      proc->threadPool()->initialThread()->getUserState().setStateProc(int_thread::stopped);
      proc->throwNopEvent();
      had_success = true;
   }

   if (!had_success)
      return false;

   bool result = int_process::waitAndHandleEvents(false);
   if (!result) {
      perr_printf("Internal error calling waitAndHandleEvents\n");
      for_each(procset->begin(), procset->end(), 
               setError(err_internal, "Error while calling waitAndHandleForProc from process stop\n"));
      return false;
   }

   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      int_process *proc = (*i)->llproc();
      if (!proc) {
         perr_printf("Process %d exited while waiting for user stop, erroring\n", (*i)->getPid());
         (*i)->setLastError(err_exited, "Process exited while being stopped.\n");
         had_error = true;
         continue;
      }
   }
   return !had_error;
}

static bool do_detach(int_processSet *procset, bool temporary, bool leaveStopped)
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   bool had_error = false;
   bool had_success = false;

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot continueProc from callback\n"));
      return false;
   }

   procset_iter iter("detach", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();

      if (temporary && !proc->plat_supportDOTF()) {
         perr_printf("Temporary detach not supported on this platform\n");
         p->setLastError(err_unsupported, "Temporary detach not supported on this platform\n");
         had_error = true;
         continue;
      }

      int_threadPool *tp = proc->threadPool();
      bool has_rpc = false;
      for (auto thr : *tp) {
         if (!thr->getPostedRPCs()->empty() || thr->runningRPC()) {
            has_rpc = true;
            break;
         }
      }
      if (has_rpc) {
         perr_printf("detach on a process with pending RPCs\n");
         p->setLastError(err_pendingirpcs, "Process has pending iRPCs, cannot detach\n");
         had_error = true;
         continue;
      }
      
      proc->throwDetachEvent(temporary, leaveStopped);
      had_success = true;
   }

   if (had_success) {
      bool result = int_process::waitAndHandleEvents(false);
      if (!result) {
         perr_printf("Internal error calling waitAndHandleEvents\n");
         for_each(procset->begin(), procset->end(), 
                  setError(err_internal, "Error while calling waitAndHandleForProc from detach\n"));
         return false;
      }
   }
   
   for (int_processSet::iterator i = procset->begin(); i != procset->end(); i++) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();
      
      if (!proc && temporary) {
         perr_printf("Process exited during temporary detach\n");
         p->setLastError(err_exited, "Process exited during temporary detach");
         had_error = true;
         continue;
      }
   }

   return !had_error;
}

bool ProcessSet::detach(bool leaveStopped) const
{
   return do_detach(procset, false, leaveStopped);
}

bool ProcessSet::temporaryDetach() const
{
   return do_detach(procset, true, false);
}

bool ProcessSet::reAttach() const
{
   bool had_error = false;
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot reAttach from callback\n"));
      return false;
   }

   procset_iter iter("reAttach", had_error, ERR_CHCK_EXITED);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc());

   ProcPool()->condvar()->lock();
   bool attach_okay = int_process::attach(procset, true);

   return !had_error && attach_okay;
}

bool ProcessSet::terminate() const
{
   bool had_error = false;
   bool run_sync = false;
   set<int_process *> to_clean;
   pthrd_printf("ProcessSet::terminate entry\n");
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot terminate from callback\n"));
      return false;
   }

#if defined(os_linux)
   pthrd_printf("Clearing queue pre-force-terminate\n");
   int_process::waitAndHandleEvents(false);
#endif

   // Clean out the event queue before we terminate; otherwise we can race
   set<int_process *> procs;
   procset_iter iter("terminate", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();

      pthrd_printf("User terminating process %d\n", proc->getPid());

      if (!proc->preTerminate()) {
         perr_printf("pre-terminate hook failed\n");
         p->setLastError(err_internal, "Pre-terminate hook failed\n");
         had_error = true;
         continue;
      }
      procs.insert(proc);
   }

   // Handle anything from preTerminate
#if defined(os_linux)
   int_process::waitAndHandleEvents(false);
#endif

   ProcPool()->condvar()->lock();

   for (set<int_process *>::iterator i = procs.begin(); i != procs.end();) {
      int_process *proc = *i;

      bool needsSync = false;
      bool result = proc->terminate(needsSync);
      if (!result) {
         pthrd_printf("Terminating process %d failed\n", proc->getPid());
         had_error = true;
         procs.erase(i++);
         continue;
      }

      if (needsSync) {
         run_sync = true;
         procs.erase(i++);
         continue;
      }
      i++;
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
   pthrd_printf("Processes terminated: sync is %d\n", run_sync);
   if (run_sync) {
     pthrd_printf("Process: waiting on waitAndHandleEvents\n");
      bool result = int_process::waitAndHandleEvents(false);
      pthrd_printf("Process: back from waitAndHandleEvents\n");
      if (!result) {
         perr_printf("Internal error calling waitAndHandleEvents\n");
         for_each(procset->begin(), procset->end(), 
                  setError(err_internal, "Error while calling waitAndHandleEvents from terminate\n"));
         had_error = true;
      }
   }
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
      int_process *proc = *i;
      HandlerPool *hp = proc->handlerPool();
      delete proc;
      delete hp;
   }

   return !had_error;
}

AddressSet::ptr ProcessSet::mallocMemory(size_t size) const
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   bool had_error = false;
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot mallocMemory from callback\n"));
      return AddressSet::newAddressSet();
   }

   AddressSet::ptr aresult = AddressSet::newAddressSet();
   procset_iter iter("mallocMemory", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      aresult->insert(0, *i);
   }
   
   int_process::infMalloc(size, aresult->get_iaddrs(), false);
   return aresult;
}

bool ProcessSet::mallocMemory(size_t size, AddressSet::ptr location) const
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   bool had_error = false;
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot mallocMemory from callback\n"));
      return false;
   }
   
   addrset_iter iter("mallocMemory", had_error, ERR_CHCK_NORM);
   for (int_addressSet::iterator i = location->iaddrs->begin(); i != location->iaddrs->end(); i++);

   return int_process::infMalloc(size, location->iaddrs, true);
}

bool ProcessSet::freeMemory(AddressSet::ptr addrset) const
{
   bool had_error = false;
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      for_each(procset->begin(), procset->end(), setError(err_incallback, "Cannot freeMemory from callback\n"));
      return false;
   }

   addrset_iter iter("free memory", had_error, ERR_CHCK_NORM);
   for (int_addressSet::iterator i = iter.begin(addrset); i != iter.end(); i = iter.inc());

   bool free_result = int_process::infFree(addrset->iaddrs);
   return !had_error && free_result;   
}

bool ProcessSet::readMemory(AddressSet::ptr addrset, multimap<Process::ptr, void *> &mem_result, size_t size) const
{
   //Use the read_t form of readMemory below
   multimap<Process::const_ptr, read_t> all_reads;
   for (int_addressSet::iterator i = addrset->iaddrs->begin(); i != addrset->iaddrs->end(); i++) {
      Process::const_ptr p = i->second;
      read_t r;
      r.addr = i->first;
      r.size = size;
      r.buffer = malloc(size);
      r.err = 0;
      all_reads.insert(make_pair(p, r));
   }

   readMemory(all_reads);

   bool had_error = false;
   for (multimap<Process::const_ptr, read_t>::iterator i = all_reads.begin(); i != all_reads.end(); i++) {
      Process::const_ptr p = i->first;
      const read_t &r = i->second;
      if (r.err) {
         free(r.buffer);
         had_error = true;
         continue;
      }
      mem_result.insert(make_pair(p->llproc()->proc(), r.buffer));
   }
   return !had_error;
}

struct bufferCompare {
   void *buffer;
   size_t size;
   uint32_t checksum;
   bool use_checksum;

   bufferCompare(void *b, size_t s, bool uc) {
      buffer = b;
      size = s;
      use_checksum = uc;
      boost::crc_32_type crcComputer;
      crcComputer.process_bytes(b, s);
      checksum = crcComputer.checksum();
   }
   bool operator<(const bufferCompare &bc) const {
      if (use_checksum)
         return checksum < bc.checksum;
      if (checksum < bc.checksum)
         return true;
      if (checksum > bc.checksum)
         return false;
      for (unsigned i=0; i<size; i++) {
         char c1 = ((char *) buffer)[i];
         char c2 = ((char *) bc.buffer)[i];
         if (c1 != c2) return c1 < c2;
      }
      return false;
   }
};

bool ProcessSet::readMemory(AddressSet::ptr addrset, map<void *, ProcessSet::ptr> &mem_result, size_t size, 
                            bool use_checksum) const
{
   multimap<Process::ptr, void *> initial_result;
   bool had_error = !readMemory(addrset, initial_result, size);

   map<bufferCompare, ProcessSet::ptr> unique_results;
   for (multimap<Process::ptr, void *>::iterator i = initial_result.begin(); i != initial_result.end(); i++) {
      Process::const_ptr proc = i->first;
      void *buffer = i->second;

      bufferCompare bc(buffer, size, use_checksum);
      map<bufferCompare, ProcessSet::ptr>::iterator j = unique_results.find(bc);
      
      if (j != unique_results.end()) {
         ProcessSet::ptr ps = j->second;
         ps->insert(proc);
         free(buffer);
      }
      else {
         ProcessSet::ptr ps = newProcessSet(proc);
         unique_results.insert(make_pair(bc, ps));
      }
   }

   for (map<bufferCompare, ProcessSet::ptr>::iterator i = unique_results.begin(); i != unique_results.end(); i++) {
      mem_result.insert(make_pair(i->first.buffer, i->second));
   }
   return !had_error;
}

bool ProcessSet::readMemory(multimap<Process::const_ptr, read_t> &addrs) const
{
   MTLock lock_this_func;
   bool had_error = false;
   for_each(procset->begin(), procset->end(), clearError());

   set<response::ptr> all_responses;
   map<response::ptr, multimap<Process::const_ptr, read_t>::const_iterator> resps_to_procs;

   readmap_iter iter("read memory", had_error, ERR_CHCK_ALL);
   for (readmap_iter::i_t i = iter.begin(&addrs); i != iter.end(); i = iter.inc()) {
      Process::const_ptr p = i->first;
      int_process *proc = p->llproc();
      const read_t &r = i->second;
      
      Address addr = r.addr;
      void *buffer = r.buffer;
      size_t size = r.size;
      pthrd_printf("User wants to read memory from 0x%lx of size %lu in process %d\n", 
                   addr, (unsigned long) size, proc->getPid());

      mem_response::ptr resp = mem_response::createMemResponse((char *) buffer, size);
      bool result = proc->readMem(addr, resp);
      if (!result) {
         pthrd_printf("Error reading from memory %lx on target process %d\n", addr, proc->getPid());
         (void)resp->isReady();
         free(buffer);
         had_error = true;
         continue;
      }
      all_responses.insert(resp);
      resps_to_procs[resp] = i;
   }

   int_process::waitForAsyncEvent(all_responses);

   map<response::ptr, multimap<Process::const_ptr, read_t>::const_iterator>::iterator i;
   for (i = resps_to_procs.begin(); i != resps_to_procs.end(); i++) {
      mem_response::ptr resp = i->first->getMemResponse();
      Process::const_ptr p = i->second->first;
      int_process *proc = p->llproc();
      read_t &read_result = const_cast<read_t &>(i->second->second);
      if (resp->hasError()) {
         pthrd_printf("Error reading from memory %lx on target process %d\n",
                      resp->lastBase(), p->getPid());
         had_error = true;
         read_result.err = resp->errorCode();
         proc->setLastError(read_result.err, proc->getLastErrorMsg());
      }
      read_result.err = 0;
   }
   return !had_error;
}

bool ProcessSet::writeMemory(AddressSet::ptr addrset, const void *buffer, size_t size) const
{
   MTLock lock_this_func;
   bool had_error = false;

   set<response::ptr> all_responses;

   addrset_iter iter("write memory", had_error, ERR_CHCK_ALL);
   for (int_addressSet::iterator i = iter.begin(addrset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = i->second;
      int_process *proc = p->llproc();
      Address addr = i->first;

      result_response::ptr resp = result_response::createResultResponse();
      bool result = proc->writeMem(buffer, addr, size, resp);
      if (!result) {
         perr_printf("Failed to write memory to %d at %lx\n", proc->getPid(), addr);
         had_error = true;
         continue;
      }
      all_responses.insert(resp);
   }

   int_process::waitForAsyncEvent(all_responses);

   for (set<response::ptr>::iterator i = all_responses.begin(); i != all_responses.end(); i++) {
      response::ptr resp = *i;
      if (resp->hasError())
         had_error = true;
   }

   return !had_error;
}

bool ProcessSet::writeMemory(multimap<Process::const_ptr, write_t> &addrs) const
{
   MTLock lock_this_func;
   bool had_error = false;
   for_each(procset->begin(), procset->end(), clearError());

   set<response::ptr> all_responses;
   map<response::ptr, multimap<Process::const_ptr, write_t>::const_iterator> resps_to_procs;

   writemap_iter iter("read memory", had_error, ERR_CHCK_ALL);
   for (writemap_iter::i_t i = iter.begin(&addrs); i != iter.end(); i = iter.inc()) {
      Process::const_ptr p = i->first;
      int_process *proc = p->llproc();
      const write_t &w = i->second;

      result_response::ptr resp = result_response::createResultResponse();
      bool result = proc->writeMem(w.buffer, w.addr, w.size, resp);
      if (!result) {
         perr_printf("Failed to write memory to %d at %lx", proc->getPid(), w.addr);
         (void)resp->isReady();
         had_error = true;
         continue;
      }
      all_responses.insert(resp);
      resps_to_procs.insert(make_pair(resp, i));
   }

   int_process::waitForAsyncEvent(all_responses);
   
   map<response::ptr, multimap<Process::const_ptr, write_t>::const_iterator>::iterator i;
   for (i = resps_to_procs.begin(); i != resps_to_procs.end(); i++) {
      result_response::ptr resp = i->first->getResultResponse();
      Process::const_ptr p = i->second->first;
      int_process *proc = p->llproc();
      write_t &write_result = const_cast<write_t &>(i->second->second);

      if (resp->hasError()) {
         pthrd_printf("Error reading from memory %lx on target process %d\n",
                      write_result.addr, p->getPid());
         had_error = true;
         write_result.err = resp->errorCode();
         proc->setLastError(write_result.err, proc->getLastErrorMsg());
      }
      write_result.err = 0;
   }
   return !had_error;
}

static bool addBreakpointWorker(set<pair<int_process *, bp_install_state *> > &bp_installs)
{
   bool had_error = false;
   bool result;

   set<response::ptr> all_responses;
   for (set<pair<int_process *, bp_install_state *> >::iterator i = bp_installs.begin(); 
        i != bp_installs.end();) 
   {
      int_process *proc = i->first;
      bp_install_state *is = i->second;
      
      result = proc->addBreakpoint_phase1(is);
      if (!result) {
         had_error = true;
         delete is;
         bp_installs.erase(i++);
         continue;
      }
      all_responses.insert(is->mem_resp);
      i++;
   }

   result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      perr_printf("Error waiting for async results during bp insertion\n");
      had_error = true;
   }
   all_responses.clear();

   for (set<pair<int_process *, bp_install_state *> >::iterator i = bp_installs.begin(); 
        i != bp_installs.end();) 
   {
      int_process *proc = i->first;
      bp_install_state *is = i->second;
      
      result = proc->addBreakpoint_phase2(is);
      if (!result) {
         had_error = true;
         delete is;
         bp_installs.erase(i++);
         continue;
      }
      all_responses.insert(is->res_resp);
      i++;
   }

   result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      perr_printf("Error waiting for async results during bp insertion\n");
      had_error = true;
   }

   for (set<pair<int_process *, bp_install_state *> >::iterator i = bp_installs.begin(); 
        i != bp_installs.end();) {
      int_process *proc = i->first;
      bp_install_state *is = i->second;
      
      result = proc->addBreakpoint_phase3(is);
      if (!result) {
         had_error = true;
      }
      delete is;
      bp_installs.erase(i++);
      continue;
   }

   return !had_error;
}

bool ProcessSet::addBreakpoint(AddressSet::ptr addrset, Breakpoint::ptr bp) const
{
   MTLock lock_this_func;
   bool had_error = false;

   set<pair<int_process *, bp_install_state *> > bp_installs;
   addrset_iter iter("Breakpoint add", had_error, ERR_CHCK_ALL);
   for (int_addressSet::iterator i = iter.begin(addrset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = i->second;
      int_process *proc = p->llproc();
      Address addr = i->first;
      
      bp_install_state *is = new bp_install_state();
      is->addr = addr;
      is->bp = bp->llbp();
      bp_installs.insert(make_pair(proc, is));
   }

   return addBreakpointWorker(bp_installs) && !had_error;
}

bool ProcessSet::rmBreakpoint(AddressSet::ptr addrset, Breakpoint::ptr bp) const
{
   MTLock lock_this_func;
   bool had_error = false;
   for_each(procset->begin(), procset->end(), clearError());

   set<response::ptr> all_responses;
   map<response::ptr, int_process *> resp_to_proc;

   addrset_iter iter("Breakpoint remove", had_error, ERR_CHCK_ALL);
   for (int_addressSet::iterator i = iter.begin(addrset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = i->second;
      int_process *proc = p->llproc();
      Address addr = i->first;

      set<response::ptr> resps;
      bool result = proc->removeBreakpoint(addr, bp->llbp(), all_responses);
      if (!result) {
         pthrd_printf("Failed to rmBreakpoint on %d\n", proc->getPid());
         had_error = true;
         continue;
      }
      all_responses.insert(resps.begin(), resps.end());
      for (auto resp : resps)
         resp_to_proc.insert(make_pair(resp, proc));
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Failed to wait for async events\n");
      had_error = true;
   }

   for (set<response::ptr>::iterator i = all_responses.begin(); i != all_responses.end(); i++) {
      response::ptr resp = *i;
      if (!resp->hasError())
         continue;
      had_error = true;
      int_process *proc = resp_to_proc[resp];
      assert(proc);
      proc->setLastError(resp->errorCode(), proc->getLastErrorMsg());
   }

   return !had_error;
}

bool ProcessSet::postIRPC(const multimap<Process::const_ptr, IRPC::ptr> &rpcs) const
{
   MTLock lock_this_func;
   bool had_error = false;

   rpcmap_iter iter("Post RPC", had_error, ERR_CHCK_NORM);
   for (
      multimap<Process::const_ptr, IRPC::ptr>::const_iterator i = iter.begin(&rpcs); 
      i != iter.end();
      i = iter.inc()) {
      Process::const_ptr p = i->first;
      int_process *proc = p->llproc();
      IRPC::ptr rpc = i->second;

      bool result = rpcMgr()->postRPCToProc(proc, rpc->llrpc()->rpc);
      if (!result) {
         pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
         had_error = true;
      }
   }
   return !had_error;
}

bool ProcessSet::postIRPC(IRPC::ptr irpc, multimap<Process::ptr, IRPC::ptr> *result) const
{
   MTLock lock_this_func;
   bool had_error = false;

   procset_iter iter("post RPC", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_process *proc = p->llproc();
      IRPC::ptr local_rpc = IRPC::createIRPC(irpc);
      
      bool bresult = rpcMgr()->postRPCToProc(proc, local_rpc->llrpc()->rpc);
      if (!bresult) {
         pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
         had_error = true;
         continue;
      }
      
      if (result) 
         result->insert(make_pair(p, local_rpc));
   }
   return !had_error;
}

bool ProcessSet::postIRPC(IRPC::ptr irpc, AddressSet::ptr addrset, multimap<Process::ptr, IRPC::ptr> *result) const
{
   MTLock lock_this_func;
   bool had_error = false;

   addrset_iter iter("post RPC", had_error, ERR_CHCK_NORM);
   for (int_addressSet::iterator i = iter.begin(addrset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = i->second;
      int_process *proc = p->llproc();
      Address addr = i->first;
      IRPC::ptr local_rpc = IRPC::createIRPC(irpc, addr);
      
      bool bresult = rpcMgr()->postRPCToProc(proc, local_rpc->llrpc()->rpc);
      if (!bresult) {
         pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
         had_error = true;
         continue;
      }
      
      if (result) 
         result->insert(make_pair(p, local_rpc));
   }
   return !had_error;
}

ProcessSet::iterator::iterator(int_processSet::iterator i)
{
   int_iter = i;
}

Process::ptr ProcessSet::iterator::operator*() const
{
   return *int_iter;
}


bool ProcessSet::iterator::operator==(const ProcessSet::iterator &i) const
{
   return int_iter == i.int_iter;
}

bool ProcessSet::iterator::operator!=(const ProcessSet::iterator &i) const
{
   return int_iter != i.int_iter;
}

ProcessSet::iterator ProcessSet::iterator::operator++()
{
   return ProcessSet::iterator(++int_iter);
}

ProcessSet::iterator ProcessSet::iterator::operator++(int)
{
   return ProcessSet::iterator(int_iter++);
}

ProcessSet::const_iterator::const_iterator()
{
}

ProcessSet::const_iterator::~const_iterator()
{
}

Process::ptr ProcessSet::const_iterator::operator*() const
{
   return *int_iter;
}

bool ProcessSet::const_iterator::operator==(const ProcessSet::const_iterator &i) const
{
   return int_iter == i.int_iter;
}

bool ProcessSet::const_iterator::operator!=(const ProcessSet::const_iterator &i) const
{
   return int_iter != i.int_iter;
}

ProcessSet::const_iterator ProcessSet::const_iterator::operator++()
{
   return ProcessSet::const_iterator(++int_iter);
}

ProcessSet::const_iterator ProcessSet::const_iterator::operator++(int)
{
   return ProcessSet::const_iterator(int_iter++);
}

int_processSet *ProcessSet::getIntProcessSet() {
   return procset;
}

struct thread_strip_const {
   Thread::ptr operator()(Thread::const_ptr t) const {
      return pc_const_cast<Thread>(t);
   }
};


ThreadSet::ThreadSet() :
   features(NULL)
{
   ithrset = new int_threadSet;
}

ThreadSet::~ThreadSet()
{
   if (ithrset) {
      delete ithrset;
      ithrset = NULL;
   }
   if (features) {
      delete features;
      features = NULL;
   }
}

ThreadSet::ptr ThreadSet::newThreadSet()
{
   return ThreadSet::ptr(new ThreadSet());
}

ThreadSet::ptr ThreadSet::newThreadSet(const ThreadPool &threadp)
{
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet());
   int_threadSet* &newset = newts->ithrset;
   
   transform(threadp.begin(), threadp.end(), inserter(*newset, newset->end()), thread_strip_const());
   return newts;
}

ThreadSet::ptr ThreadSet::newThreadSet(const set<Thread::const_ptr> &threads)
{
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet());
   int_threadSet* &newset = newts->ithrset;
   transform(threads.begin(), threads.end(), inserter(*newset, newset->end()), thread_strip_const());
   return newts;
}

ThreadSet::ptr ThreadSet::newThreadSet(Thread::ptr thr) {
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet());
   newts->ithrset->insert(thr);
   return newts;
}

ThreadSet::ptr ThreadSet::newThreadSet(ProcessSet::ptr ps, bool initial_only)
{
   MTLock lock_this_func;
   bool had_error = false;

   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet());
   int_threadSet* &newset = newts->ithrset;

   procset_iter iter("New thread group", had_error, ERR_CHCK_NORM);
   for (procset_iter::i_t i = iter.begin(ps->procset); i != iter.end(); i = iter.inc()) {
      if (had_error) {
         pthrd_printf("Failed to create new thread group\n");
         return ThreadSet::ptr();
      }
      Process::ptr proc = *i;
      if (initial_only) {
         newset->insert(proc->threads().getInitialThread());
      }
      else {
         ThreadPool &pool = proc->threads();
         for (ThreadPool::iterator j = pool.begin(); j != pool.end(); j++) {
            newset->insert(*j);
         }
      }
   }
   return newts;
}

ThreadSet::ptr ThreadSet::set_union(ThreadSet::ptr tp) const
{
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet);
   int_threadSet *me = ithrset;
   int_threadSet *you = tp->ithrset;
   int_threadSet *them = newts->ithrset;

   std::set_union(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newts;
}

ThreadSet::ptr ThreadSet::set_intersection(ThreadSet::ptr tp) const
{
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet);
   int_threadSet *me = ithrset;
   int_threadSet *you = tp->ithrset;
   int_threadSet *them = newts->ithrset;

   std::set_intersection(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newts;
}

ThreadSet::ptr ThreadSet::set_difference(ThreadSet::ptr tp) const
{
   ThreadSet::ptr newts = ThreadSet::ptr(new ThreadSet);
   int_threadSet *me = ithrset;
   int_threadSet *you = tp->ithrset;
   int_threadSet *them = newts->ithrset;

   std::set_difference(me->begin(), me->end(), you->begin(), you->end(), inserter(*them, them->end()));
   return newts;
}

ThreadSet::iterator ThreadSet::begin() {
   return iterator(ithrset->begin());
}

ThreadSet::iterator ThreadSet::end() {
   return iterator(ithrset->end());
}

ThreadSet::iterator ThreadSet::find(Thread::const_ptr p) {
   return iterator(ithrset->find(pc_const_cast<Thread>(p)));
}

ThreadSet::const_iterator ThreadSet::begin() const {
   return const_iterator(ithrset->begin());
}

ThreadSet::const_iterator ThreadSet::end() const {
   return const_iterator(ithrset->end());
}

ThreadSet::const_iterator ThreadSet::find(Thread::const_ptr p) const {
   return const_iterator(ithrset->find(pc_const_cast<Thread>(p)));
}

bool ThreadSet::empty() const {
   return ithrset->empty();
}

size_t ThreadSet::size() const {
   return ithrset->size();
}

int_threadSet *ThreadSet::getIntThreadSet() const {
   return ithrset;
}

pair<ThreadSet::iterator, bool> ThreadSet::insert(Thread::const_ptr p) {
   pair<int_threadSet::iterator, bool> result = ithrset->insert(pc_const_cast<Thread>(p));
   return pair<ThreadSet::iterator, bool>(ThreadSet::iterator(result.first), result.second);
}

void ThreadSet::erase(iterator pos)
{
   ithrset->erase(pos.int_iter);
}

size_t ThreadSet::erase(Thread::const_ptr t)
{
   ThreadSet::iterator i = find(t);
   if (i == end()) return 0;
   erase(i);
   return 1;
}

void ThreadSet::clear()
{
   ithrset->clear();
}

ThreadSet::ptr ThreadSet::getErrorSubset() const
{
   MTLock lock_this_func;

   ThreadSet::ptr errts = newThreadSet();
   for (const_iterator i = begin(); i != end(); i++) {
      Thread::ptr thr = *i;
      Process::ptr proc = thr->getProcess();
      if (!proc) {
         errts->insert(thr);
         continue;
      }
      if (proc->getLastError() == err_none) {
         continue;
      }
      errts->insert(thr);
   }
   return errts;
}

void ThreadSet::getErrorSubsets(map<ProcControlAPI::err_t, ThreadSet::ptr> &err_sets) const
{
   MTLock lock_this_func;

   for (const_iterator i = begin(); i != end(); i++) {
      Thread::ptr thr = *i;
      Process::ptr proc = thr->getProcess();
 
     ProcControlAPI::err_t err = proc->getLastError();
      if (err == err_none)
         continue;
      map<err_t, ThreadSet::ptr>::iterator j = err_sets.find(err);
      ThreadSet::ptr ts;
      if (j != err_sets.end()) {
         ts = j->second;
      }
      else {
         ts = ThreadSet::newThreadSet();
         err_sets[err] = ts;
      }
      ts->insert(thr);
   }   
}


bool ThreadSet::anyStopped() const
{
   MTLock lock_this_func;
   return any_match(ithrset->begin(), ithrset->end(), test_thr(test_thr::any, int_thread::stopped));
}

bool ThreadSet::allStopped() const
{
   MTLock lock_this_func;
   return all_match(ithrset->begin(), ithrset->end(), test_thr(test_thr::all, int_thread::stopped));
}

ThreadSet::ptr ThreadSet::getStoppedSubset() const
{
   MTLock lock_this_func;
   return create_thrsubset(ithrset->begin(), ithrset->end(), test_thr(test_thr::any, int_thread::stopped));
}

bool ThreadSet::anyRunning() const
{
   MTLock lock_this_func;
   return any_match(ithrset->begin(), ithrset->end(), test_thr(test_thr::any, int_thread::running));
}

bool ThreadSet::allRunning() const
{
   MTLock lock_this_func;
   return all_match(ithrset->begin(), ithrset->end(), test_thr(test_thr::all, int_thread::running));
}

ThreadSet::ptr ThreadSet::getRunningSubset() const
{
   MTLock lock_this_func;
   return create_thrsubset(ithrset->begin(), ithrset->end(), test_thr(test_thr::any, int_thread::running));
}

bool ThreadSet::anyTerminated() const
{
   MTLock lock_this_func;
   return any_match(ithrset->begin(), ithrset->end(), test_terminate());
}

bool ThreadSet::allTerminated() const
{
   MTLock lock_this_func;
   return all_match(ithrset->begin(), ithrset->end(), test_terminate());
}

ThreadSet::ptr ThreadSet::getTerminatedSubset() const
{
   MTLock lock_this_func;
   return create_thrsubset(ithrset->begin(), ithrset->end(), test_terminate());
}

struct test_singlestep {
   bool operator()(Thread::ptr t) {
      Process::ptr p = t->getProcess();
      p->clearLastError();
      return t->getSingleStepMode();
   }
};

bool ThreadSet::anySingleStepMode() const
{
   MTLock lock_this_func;
   return any_match(ithrset->begin(), ithrset->end(), test_singlestep());
}

bool ThreadSet::allSingleStepMode() const
{
   MTLock lock_this_func;
   return all_match(ithrset->begin(), ithrset->end(), test_singlestep());
}

ThreadSet::ptr ThreadSet::getSingleStepSubset() const
{
   MTLock lock_this_func;
   return create_thrsubset(ithrset->begin(), ithrset->end(), test_singlestep());
}

struct test_userthrinfo {
   bool operator()(Thread::ptr t) {
      Process::ptr p = t->getProcess();
      p->clearLastError();
      return t->haveUserThreadInfo();
   }
};


bool ThreadSet::anyHaveUserThreadInfo() const
{
   MTLock lock_this_func;
   return any_match(ithrset->begin(), ithrset->end(), test_userthrinfo());
}

bool ThreadSet::allHaveUserThreadInfo() const
{
   MTLock lock_this_func;
   return all_match(ithrset->begin(), ithrset->end(), test_userthrinfo());
}

ThreadSet::ptr ThreadSet::getHaveUserThreadInfoSubset() const
{
   MTLock lock_this_func;
   return create_thrsubset(ithrset->begin(), ithrset->end(), test_userthrinfo());
}

bool ThreadSet::getStartFunctions(AddressSet::ptr result) const
{
   MTLock lock_this_func;
   bool had_error = false;
   thrset_iter iter("get start function", had_error, ERR_CHCK_THRD);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      Process::ptr p = t->getProcess();
      Address addr;
      bool bresult = t->llthrd()->getStartFuncAddress(addr);
      if (bresult)
         result->insert(addr, p);
   }
   return !had_error;
}

bool ThreadSet::getStackBases(AddressSet::ptr result) const
{
   MTLock lock_this_func;
   bool had_error = false;
   thrset_iter iter("get stack base", had_error, ERR_CHCK_THRD);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      Process::ptr p = t->getProcess();
      Address addr;
      bool bresult = t->llthrd()->getStackBase(addr);
      if (bresult)
         result->insert(addr, p);
   }
   return !had_error;
}

bool ThreadSet::getTLSs(AddressSet::ptr result) const
{
   MTLock lock_this_func;
   bool had_error = false;
   thrset_iter iter("get TLS", had_error, ERR_CHCK_THRD);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      Process::ptr p = t->getProcess();
      Address addr;
      bool bresult = t->llthrd()->getTLSPtr(addr);
      if (bresult)
         result->insert(addr, p);
   }
   return !had_error;
}

bool ThreadSet::stopThreads() const
{
   MTLock lock_this_func;
   bool had_error = false;
   set<int_process *> all_procs;

   thrset_iter iter("stop thread", had_error, ERR_CHCK_THRD);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thr = t->llthrd();
      int_process *proc = thr->llproc();

      thr->getUserState().setState(int_thread::stopped);
      all_procs.insert(proc);
   }

   for (set<int_process *>::iterator i = all_procs.begin(); i != all_procs.end(); i++) {
      int_process *proc = *i;
      proc->throwNopEvent();
   }
   return !had_error;
}

bool ThreadSet::continueThreads() const
{
   MTLock lock_this_func;
   bool had_error = false;
   set<int_process *> all_procs;

   thrset_iter iter("continue thread", had_error, ERR_CHCK_THRD);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thr = t->llthrd();
      int_process *proc = thr->llproc();

      thr->getUserState().setState(int_thread::running);
      all_procs.insert(proc);
   }

   for (set<int_process *>::iterator i = all_procs.begin(); i != all_procs.end(); i++) {
      int_process *proc = *i;
      proc->throwNopEvent();
   }
   return !had_error;
}

bool ThreadSet::setSingleStepMode(bool v) const
{
   MTLock lock_this_func;
   bool had_error = false;

   thrset_iter iter("set single step", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr thr = *i;
      thr->setSingleStepMode(v);
   }
   return !had_error;
}

static bool getRegisterWorker(Dyninst::MachRegister reg, int_threadSet *ithrset, 
                              set<pair<Thread::ptr, reg_response::ptr> > &thr_to_response)
{
   bool had_error = false;
   
   set<response::ptr> all_responses;
   thrset_iter iter("getRegister", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thr = t->llthrd();
      reg_response::ptr response = reg_response::createRegResponse();
      bool result = thr->getRegister(reg, response);
      if (!result) {
         pthrd_printf("Error reading register response on thread %d/%d\n",
                      thr->llproc()->getPid(), thr->getLWP());
         had_error = true;
         continue;
      }

      thr_to_response.insert(make_pair(t, response));
      all_responses.insert(response);
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for async events to complete\n");
      thr_to_response.clear();
      return false;
   }

   for (set<pair<Thread::ptr, reg_response::ptr > >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end();)
   {
      Thread::ptr thr = i->first;
      reg_response::ptr resp = i->second;

      if (resp->hasError()) {
         thr->getProcess()->setLastError(resp->errorCode(), thr->getProcess()->getLastErrorMsg());
         pthrd_printf("Error in response from %d/%d\n", thr->llthrd()->llproc()->getPid(),
                      thr->llthrd()->getLWP());
         had_error = true;
         thr_to_response.erase(i++);
         continue;
      }
      i++;
   }

   return !had_error;
}

bool ThreadSet::getRegister(Dyninst::MachRegister reg, map<Thread::ptr, Dyninst::MachRegisterVal> &results) const
{
   MTLock lock_this_func;
   bool had_error = false;
   
   set<pair<Thread::ptr, reg_response::ptr> > thr_to_response;
   bool result = getRegisterWorker(reg, ithrset, thr_to_response);
   if (!result) {
      pthrd_printf("Error in getRegisterWorker\n");
      had_error = true;
   }

   for (set<pair<Thread::ptr, reg_response::ptr > >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end(); i++) 
   {
      Thread::ptr thr = i->first;
      reg_response::ptr resp = i->second;

      results.insert(make_pair(thr, resp->getResult()));
   }
   return !had_error;
}

bool ThreadSet::getRegister(Dyninst::MachRegister reg, map<Dyninst::MachRegisterVal, ThreadSet::ptr> &results) const
{
   MTLock lock_this_func;
   bool had_error = false;
   
   set<pair<Thread::ptr, reg_response::ptr> > thr_to_response;
   bool result = getRegisterWorker(reg, ithrset, thr_to_response);
   if (!result) {
      pthrd_printf("Error in getRegisterWorker\n");
      had_error = true;
   }

   for (set<pair<Thread::ptr, reg_response::ptr > >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end(); i++) 
   {
      Thread::ptr thr = i->first;
      reg_response::ptr resp = i->second;
      MachRegisterVal val = resp->getResult();

      ThreadSet::ptr ts;
      map<MachRegisterVal, ThreadSet::ptr>::iterator j = results.find(val);
      if (j == results.end()) {
         ts = ThreadSet::newThreadSet();
         results.insert(make_pair(val, ts));
      }
      else {
         ts = j->second;
      }
      ts->insert(thr);
   }
   return !had_error;
}

bool ThreadSet::setRegister(Dyninst::MachRegister reg, const map<Thread::const_ptr, Dyninst::MachRegisterVal> &vals) const
{
   MTLock lock_this_func;
   bool had_error = false;
   
   set<response::ptr> all_responses;
   set<pair<Thread::ptr, result_response::ptr> > thr_to_response;

   setreg_iter iter("setRegister", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (setreg_iter::i_t i = iter.begin(&vals); i != iter.end(); i = iter.inc()) {
      Thread::const_ptr t = i->first;
      MachRegisterVal val = i->second;
      int_thread *thr = t->llthrd();
      result_response::ptr response = result_response::createResultResponse();

      bool result = thr->setRegister(reg, val, response);
      if (!result) {
         pthrd_printf("Error writing register response on thread %d/%d\n",
                      thr->llproc()->getPid(), thr->getLWP());
         had_error = true;
         continue;
      }

      thr_to_response.insert(make_pair(thr->thread(), response));
      all_responses.insert(response);
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for async events to complete\n");
      thr_to_response.clear();
      return false;
   }

   for (set<pair<Thread::ptr, result_response::ptr > >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end(); i++)
   {
      Thread::ptr thr = i->first;
      result_response::ptr resp = i->second;

      if (resp->hasError() || !resp->getResult()) {
         thr->getProcess()->setLastError(resp->errorCode(), thr->getProcess()->getLastErrorMsg());
         pthrd_printf("Error in response from %d/%d\n", thr->llthrd()->llproc()->getPid(),
                      thr->llthrd()->getLWP());
         had_error = true;
      }
   }

   return !had_error;
}

bool ThreadSet::setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const
{
   MTLock lock_this_func;
   bool had_error = false;
   map<Thread::const_ptr, Dyninst::MachRegisterVal> vals;

   thrset_iter iter("setRegister", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      vals.insert(make_pair(*i, val));
   }
   return setRegister(reg, vals) && !had_error;
}

bool ThreadSet::getAllRegisters(map<Thread::ptr, RegisterPool> &results) const
{
   MTLock lock_this_func;
   bool had_error = false;

   set<response::ptr> all_responses;
   set<pair<Thread::ptr, allreg_response::ptr> > thr_to_response;

   thrset_iter iter("getAllRegisters", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thr = t->llthrd();
      int_registerPool *newpool = new int_registerPool();
      allreg_response::ptr response = allreg_response::createAllRegResponse(newpool);
      bool result = thr->getAllRegisters(response);
      if (!result) {
         pthrd_printf("Error reading registers on thread %d/%d\n",
                      thr->llproc()->getPid(), thr->getLWP());
         had_error = true;
         delete newpool;
         continue;
      }

      thr_to_response.insert(make_pair(t, response));
      all_responses.insert(response);
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for async events to complete\n");
      for (set<pair<Thread::ptr, allreg_response::ptr> >::iterator i = thr_to_response.begin(); 
           i != thr_to_response.end(); i++) {
         delete i->second->getRegPool();
      }
      return false;
   }

   for (set<pair<Thread::ptr, allreg_response::ptr> >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end(); i++)
   {
      Thread::ptr thr = i->first;
      allreg_response::ptr resp = i->second;
      int_registerPool *pool = resp->getRegPool();

      if (resp->hasError()) {
         thr->getProcess()->setLastError(resp->errorCode(), thr->getProcess()->getLastErrorMsg());
         pthrd_printf("Error in response from %d/%d\n", thr->llthrd()->llproc()->getPid(),
                      thr->llthrd()->getLWP());
         had_error = true;
         delete pool;
         continue;
      }
      
      RegisterPool rpool;
      rpool.llregpool = pool;
      results.insert(make_pair(thr, rpool));
   }

   return !had_error;
}

bool ThreadSet::setAllRegisters(const map<Thread::const_ptr, RegisterPool> &reg_vals) const
{
   MTLock lock_this_func;
   bool had_error = false;
   
   set<response::ptr> all_responses;
   set<pair<Thread::ptr, result_response::ptr> > thr_to_response;

   setallreg_iter iter("setAllRegisters", had_error, ERR_CHCK_THRD | ERR_CHCK_THRD_STOPPED);
   for (setallreg_iter::i_t i = iter.begin(&reg_vals); i != iter.end(); i = iter.inc()) {
      Thread::const_ptr t = i->first;
      RegisterPool pool = i->second;
      int_thread *thr = t->llthrd();
      result_response::ptr response = result_response::createResultResponse();

      bool result = thr->setAllRegisters(*pool.llregpool, response);
      if (!result) {
         pthrd_printf("Error setting registers on thread %d/%d\n",
                      thr->llproc()->getPid(), thr->getLWP());
         had_error = true;
         continue;
      }

      thr_to_response.insert(make_pair(thr->thread(), response));
      all_responses.insert(response);      
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for async events to complete\n");
      return false;
   }

   for (set<pair<Thread::ptr, result_response::ptr > >::iterator i = thr_to_response.begin(); 
        i != thr_to_response.end(); i++)
   {
      Thread::ptr thr = i->first;
      result_response::ptr resp = i->second;

      if (resp->hasError() || !resp->getResult()) {
         thr->getProcess()->setLastError(resp->errorCode(), thr->getProcess()->getLastErrorMsg());
         pthrd_printf("Error in response from %d/%d\n", thr->llthrd()->llproc()->getPid(),
                      thr->llthrd()->getLWP());
         had_error = true;
      }
   }
   return !had_error;
}

bool ThreadSet::postIRPC(const multimap<Thread::const_ptr, IRPC::ptr> &rpcs) const
{
   MTLock lock_this_func;
   bool had_error = false;

   rpcmap_thr_iter iter("Post RPC", had_error, ERR_CHCK_NORM);
   for (rpcmap_thr_iter::i_t i = iter.begin(&rpcs); i != iter.end(); i = iter.inc()) {
      IRPC::ptr rpc = i->second;
      Thread::const_ptr t = i->first;
      int_thread *thread = t->llthrd();

      bool result = rpcMgr()->postRPCToThread(thread, rpc->llrpc()->rpc);
      if (!result) {
         pthrd_printf("postRPCToThread failed on %d/%d\n", thread->llproc()->getPid(), thread->getLWP());
         had_error = true;
      }
   }
   return !had_error;
}

bool ThreadSet::postIRPC(IRPC::ptr irpc, multimap<Thread::ptr, IRPC::ptr> *results) const
{
   MTLock lock_this_func;
   bool had_error = false;

   thrset_iter iter("Post RPC", had_error, ERR_CHCK_NORM);
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thread = t->llthrd();
      IRPC::ptr local_rpc = IRPC::createIRPC(irpc);

      bool result = rpcMgr()->postRPCToThread(thread, local_rpc->llrpc()->rpc);
      if (!result) {
         pthrd_printf("postRPCToThread failed on %d/%d\n", thread->llproc()->getPid(), thread->getLWP());
         had_error = true;
         continue;
      }
      if (results)
         results->insert(make_pair(t, local_rpc));
   }
   return !had_error;
}

CallStackUnwindingSet *ThreadSet::getCallStackUnwinding()
{
   if (features && features->stkset)
      return features->stkset;

   MTLock lock_this_func;
   if (!features) {
      features = new TSetFeatures();
   }
   if (!ithrset)
      return NULL;
   for (int_threadSet::iterator i = ithrset->begin(); i != ithrset->end(); i++) {
      Thread::ptr t = *i;
      if (t->getCallStackUnwinding()) {
         features->stkset = new CallStackUnwindingSet(shared_from_this());
         return features->stkset;
      }
   }

   return NULL;
}

const CallStackUnwindingSet *ThreadSet::getCallStackUnwinding() const
{
   return const_cast<ThreadSet *>(this)->getCallStackUnwinding();
}

ThreadSet::iterator::iterator(int_threadSet::iterator i)
{
   int_iter = i;
}

Thread::ptr ThreadSet::iterator::operator*()
{
   return *int_iter;
}

bool ThreadSet::iterator::operator==(const iterator &i) const
{
   return int_iter == i.int_iter;
}

bool ThreadSet::iterator::operator!=(const iterator &i) const
{
   return int_iter != i.int_iter;
}

ThreadSet::iterator ThreadSet::iterator::operator++()
{
   return ThreadSet::iterator(++int_iter);
}

ThreadSet::iterator ThreadSet::iterator::operator++(int)
{
   return ThreadSet::iterator(int_iter++);
}

ThreadSet::const_iterator::const_iterator(int_threadSet::iterator i)
{
   int_iter = i;
}

ThreadSet::const_iterator::const_iterator()
{
}

ThreadSet::const_iterator::~const_iterator()
{
}

Thread::ptr ThreadSet::const_iterator::operator*()
{
   return *int_iter;
}

bool ThreadSet::const_iterator::operator==(const const_iterator &i) const
{
   return int_iter == i.int_iter;
}

bool ThreadSet::const_iterator::operator!=(const const_iterator &i) const
{
   return int_iter != i.int_iter;
}

ThreadSet::const_iterator ThreadSet::const_iterator::operator++()
{
   return ThreadSet::const_iterator(++int_iter);
}

ThreadSet::const_iterator ThreadSet::const_iterator::operator++(int)
{
   return ThreadSet::const_iterator(int_iter++);
}

LibraryTrackingSet::LibraryTrackingSet(ProcessSet::ptr ps_) :
   wps(ps_)
{
}

LibraryTrackingSet::~LibraryTrackingSet()
{
   wps = ProcessSet::weak_ptr();
}

bool LibraryTrackingSet::setTrackLibraries(bool b) const
{
   MTLock lock_this_func;
   bool had_error = false;

   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("setTrackLibraries on deleted process set\n");
      globalSetLastError(err_badparam, "setTrackLibraries attempted on deleted ProcessSet object");
      return false;
   }
   int_processSet *procset = ps->getIntProcessSet();

   set<pair<int_process *, bp_install_state *> > bps_to_install;
   set<response::ptr> all_responses;

   procset_iter iter("setTrackLibraries", had_error, ERR_CHCK_NORM);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_libraryTracking *proc = p->llproc()->getLibraryTracking();
      if (!proc) {
         perr_printf("Library tracking not supported on process %d\n", p->getPid());
         p->setLastError(err_unsupported, "No library tracking on this platform\n");
         had_error = true;
         continue;
      }
      
      pthrd_printf("Changing sysv track libraries to %s for %d\n",
                   b ? "true" : "false", proc->getPid());

      bool add_bp;
      int_breakpoint *bp;
      Address addr;

      bool result = proc->setTrackLibraries(b, bp, addr, add_bp);
      if (!result) {
         had_error = true;
         continue;
      }
      if (add_bp) {
         bp_install_state *is = new bp_install_state();
         is->addr = addr;
         is->bp = bp;
         is->do_install = true;
         bps_to_install.insert(make_pair(proc, is));
      }
      else {
         result = proc->removeBreakpoint(addr, bp, all_responses);
         if (!result) {
            pthrd_printf("Error removing breakpoint in setTrackLibraries\n");
            had_error = true;
            continue;
         }
      }
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for bp removals in setTrackLibraries\n");
      had_error = true;
   }
   if (bps_to_install.empty())
      return !had_error;
   
   return addBreakpointWorker(bps_to_install) && !had_error;
}

bool LibraryTrackingSet::refreshLibraries() const
{
   MTLock lock_this_scope;
   bool had_error = false;
   set<int_process *> procs;
    
   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("refreshLibraries on deleted process set\n");
      globalSetLastError(err_badparam, "refreshLibraries attempted on deleted ProcessSet object");
      return false;
   }  
   if (int_process::isInCB()) {
      perr_printf("User attempted refreshLibraries in CB, erroring.");
      for_each(ps->begin(), ps->end(), setError(err_incallback, "Cannot refreshLibraries from callback\n"));
      return false;
   }

   procset_iter iter("refreshLibraries", had_error, ERR_CHCK_ALL);
   for (int_processSet::iterator i = iter.begin(ps->getIntProcessSet()); i != iter.end(); i = iter.inc()) {
      procs.insert((*i)->llproc());
   }
      
   while (!procs.empty()) {
      set<response::ptr> all_responses;
      for (set<int_process *>::iterator i = procs.begin(); i != procs.end();) {
         int_process *proc = *i;
         std::set<int_library *> added;
         std::set<int_library *> rmd;
         bool wait_for_async = false;
            
         bool result = proc->refresh_libraries(added, rmd, wait_for_async, all_responses);
         if (!result && !wait_for_async) {
            pthrd_printf("Error refreshing libraries for %d\n", proc->getPid());
            had_error = true;
            procs.erase(i++);
            continue;
         }
         if (!wait_for_async) {
            procs.erase(i++);
            if (added.empty() && rmd.empty()) {
               pthrd_printf("Refresh found no new library events for process %d\n", proc->getPid());
               continue;
            }

            pthrd_printf("Adding new library event for process %d after refresh\n", proc->getPid());
            struct lib_converter {
               static Library::ptr c(int_library* l) { return l->getUpPtr(); }
            };
            set<Library::ptr> libs_added, libs_rmd;
            transform(added.begin(), added.end(), inserter(libs_added, libs_added.end()), lib_converter::c);
            transform(rmd.begin(), rmd.end(), inserter(libs_rmd, libs_rmd.end()), lib_converter::c);
            EventLibrary::ptr evlib = EventLibrary::ptr(new EventLibrary(libs_added, libs_rmd));
            evlib->setProcess(proc->proc());
            evlib->setThread(proc->threadPool()->initialThread()->thread());
            evlib->setSyncType(Event::async);
            mbox()->enqueue(evlib);
            continue;
         }
         i++;
      }
      bool result = int_process::waitForAsyncEvent(all_responses);
      if (!result) {
         pthrd_printf("Error waiting for async events\n");
         had_error = true;
         break;
      }
   }

   int_process::waitAndHandleEvents(false);
   return !had_error;
}

ThreadTrackingSet::ThreadTrackingSet(ProcessSet::ptr ps_) :
   wps(ps_)
{
}

ThreadTrackingSet::~ThreadTrackingSet()
{
}

bool ThreadTrackingSet::setTrackThreads(bool b) const
{
   MTLock lock_this_func;
   bool had_error = false;

   set<pair<int_process *, bp_install_state *> > bps_to_install;
   set<response::ptr> all_responses;

   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("refreshLibraries on deleted process set\n");
      globalSetLastError(err_badparam, "refreshLibraries attempted on deleted ProcessSet object");
      return false;
   }  
   int_processSet *procset = ps->getIntProcessSet();
   procset_iter iter("setTrackThreads", had_error, ERR_CHCK_ALL);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      int_threadTracking *proc = p->llproc()->getThreadTracking();
      if (!proc) {
         perr_printf("Thread tracking not supported on process %d\n", p->getPid());
         p->setLastError(err_unsupported, "No thread tracking on this platform\n");
         had_error = true;
         continue;
      }

      pthrd_printf("Changing sysv track threads to %s for %d\n",
                   b ? "true" : "false", proc->getPid());

      bool add_bp;
      set<pair<int_breakpoint *, Address> > bps;
      bool result = proc->setTrackThreads(b, bps, add_bp);
      if (!result) {
         had_error = true;
         continue;
      }
      if (add_bp) {
         for (set<pair<int_breakpoint *, Address> >::iterator j = bps.begin(); j != bps.end(); j++) {
            bp_install_state *is = new bp_install_state();
            is->addr = j->second;
            is->bp = j->first;
            is->do_install = true;
            bps_to_install.insert(make_pair(proc, is));
         }
      }
      else {
         for (set<pair<int_breakpoint *, Address> >::iterator j = bps.begin(); j != bps.end(); j++) {
            result = proc->removeBreakpoint(j->second, j->first, all_responses);
            if (!result) {
               pthrd_printf("Error removing breakpoint in setTrackLibraries\n");
               had_error = true;
               continue;
            }
         }
      }
   }

   bool result = int_process::waitForAsyncEvent(all_responses);
   if (!result) {
      pthrd_printf("Error waiting for bp removals in setTrackLibraries\n");
      had_error = true;
   }
   if (bps_to_install.empty())
      return !had_error;
   
   return addBreakpointWorker(bps_to_install) && !had_error;
}

bool ThreadTrackingSet::refreshThreads() const
{
   MTLock lock_this_func;
   bool had_error = false;

   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("refreshThreads on deleted process set\n");
      globalSetLastError(err_badparam, "refreshThreads attempted on deleted ProcessSet object");
      return false;
   }
   int_processSet *procset = ps->getIntProcessSet();
   procset_iter iter("refreshThreads", had_error, ERR_CHCK_ALL);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_threadTracking *proc = (*i)->llproc()->getThreadTracking();
      if (!proc) {
         perr_printf("Thread tracking not supported on process\n");
         had_error = true;
         continue;
      }
      if (!proc->refreshThreads()) {
         had_error = true;
         continue;
      }
   }
   
   int_process::waitAndHandleEvents(false);
   return !had_error;
}

LWPTrackingSet::LWPTrackingSet(ProcessSet::ptr ps_) :
   wps(ps_)
{
}

LWPTrackingSet::~LWPTrackingSet()
{
}

bool LWPTrackingSet::setTrackLWPs(bool b) const
{
   MTLock lock_this_func;
   bool had_error = false;
   pthrd_printf("setting LWP tracking in process set to %s\n", b ? "enabled" : "disabled");
   int_processSet *procset = wps.lock()->getIntProcessSet();
   procset_iter iter("setTrackLWPs", had_error, ERR_CHCK_ALL);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      Process::ptr p = *i;
      LWPTracking *lwpt = p->getLWPTracking();
      if (!lwpt) {
         p->setLastError(err_unsupported, "LWP Tracking not supported on this process");
         had_error = true;
         continue;
      }

      lwpt->setTrackLWPs(b);
   }
   return !had_error;
}

bool LWPTrackingSet::refreshLWPs() const
{
   MTLock lock_this_func;
   bool had_error = false;
   pthrd_printf("refreshing LWPs in process set\n");

   int_processSet *procset = wps.lock()->getIntProcessSet();
   procset_iter iter("setTrackLWPs", had_error, ERR_CHCK_ALL);
   set<response::ptr> all_resps;
   set<int_process *> all_procs;
   set<int_process *> change_procs;
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_LWPTracking *proc = (*i)->llproc()->getLWPTracking();
      if (!proc) {
         perr_printf("LWP tracking not supported on process\n");
         had_error = true;
         continue;
      }
      result_response::ptr resp;
      if (!proc->lwp_refreshPost(resp)) {
         pthrd_printf("Error refreshing lwps on %d\n", proc->getPid());
         had_error = true;
      }
      if (resp) {
         all_resps.insert(resp);
      }
      all_procs.insert(proc);
   }

   int_process::waitForAsyncEvent(all_resps);

   for (set<int_process *>::iterator i = all_procs.begin(); i != all_procs.end(); i++) {
      int_LWPTracking *proc = (*i)->getLWPTracking();
      if (!proc)
         continue;
      bool changed;
      bool result = proc->lwp_refreshCheck(changed);
      if (!result) {
         pthrd_printf("Error refreshing lwps while creating events on %d\n", proc->getPid());
         had_error = true;
      }
      if (changed) {
         change_procs.insert(proc);
         proc->setForceGeneratorBlock(true);
      }
   }

   if (change_procs.empty())
      return !had_error;

   pthrd_printf("Found changes to thread in refresh.  Handling events.\n");
   ProcPool()->condvar()->lock();
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   int_process::waitAndHandleEvents(false);

   for (set<int_process *>::iterator i = change_procs.begin(); i != change_procs.end(); i++) {
      int_process *proc = *i;
      proc->setForceGeneratorBlock(false);      
   }
   return !had_error;
}

FollowForkSet::FollowForkSet(ProcessSet::ptr ps_) :
   wps(ps_)
{
}

FollowForkSet::~FollowForkSet()
{
   wps = ProcessSet::weak_ptr();
}

bool FollowForkSet::setFollowFork(FollowFork::follow_t f) const
{
   MTLock lock_this_func;
   bool had_error = false;

   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("setFollowFork on deleted process set\n");
      globalSetLastError(err_badparam, "setFollowFork attempted on deleted ProcessSet object");
      return false;
   }  
   int_processSet *procset = ps->getIntProcessSet();
   procset_iter iter("setFollowFork", had_error, ERR_CHCK_ALL);
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_followFork *proc = (*i)->llproc()->getFollowFork();
      if (!proc) {
         perr_printf("Follow Fork not supported on process\n");
         had_error = true;
         continue;
      }
      proc->fork_setTracking(f);
   }

   return !had_error;
}

CallStackUnwindingSet::CallStackUnwindingSet(ThreadSet::ptr ts) :
   wts(ts)
{
}

CallStackUnwindingSet::~CallStackUnwindingSet()
{
   wts = ThreadSet::weak_ptr();
}

bool CallStackUnwindingSet::walkStack(CallStackCallback *stk_cb)
{
   MTLock lock_this_func;
   bool had_error = false;

   ThreadSet::ptr thrset = wts.lock();
   if (!thrset) {
      perr_printf("walkStack was given an exited thread set\n");
      globalSetLastError(err_exited, "Exited threads passed to walkStack\n");
      return false;
   }

   thrset_iter iter("walkStack", had_error, ERR_CHCK_NORM);
   set<response::ptr> all_responses;
   set<int_process *> all_procs;
   int_threadSet *ithrset = thrset->getIntThreadSet();

   pthrd_printf("Sending requests for callstack\n");
   getResponses().lock();
   for (thrset_iter::i_t i = iter.begin(ithrset); i != iter.end(); i = iter.inc()) {
      Thread::ptr t = *i;
      int_thread *thr = t->llthrd();
      int_callStackUnwinding *proc = thr->llproc()->getCallStackUnwinding();
      if (!proc) {
         perr_printf("Stack unwinding not supported on process %d\n", t->getProcess()->getPid());
         t->setLastError(err_unsupported, "No stack unwinding on this platform\n");
         had_error = true;
         continue;
      }
      stack_response::ptr stk_resp = stack_response::createStackResponse(thr);
      stk_resp->markSyncHandled();

      bool result = proc->plat_getStackInfo(thr, stk_resp);
      if (!result) {
         had_error = true;
         pthrd_printf("Could not get stackwalk from %d/%d\n", proc->getPid(), thr->getLWP());
         continue;
      }

      getResponses().addResponse(stk_resp, proc);
      all_procs.insert(proc);
      all_responses.insert(stk_resp);
   }
   getResponses().unlock();
   getResponses().noteResponse();

   for (set<int_process *>::iterator i = all_procs.begin(); i != all_procs.end(); i++) {
      int_process *proc = *i;
      proc->plat_preAsyncWait();
   }
   all_procs.clear();


   pthrd_printf("Processing requests for callstack\n");
   while (!all_responses.empty()) {
      bool did_something = false;
      stack_response::ptr a_resp;
      for (set<response::ptr>::iterator i = all_responses.begin(); i != all_responses.end();) {
         stack_response::ptr stk_resp = (*i)->getStackResponse();
         if (stk_resp->hasError() || stk_resp->isReady()) {
            int_thread *thr = stk_resp->getThread();
            int_callStackUnwinding *proc = thr->llproc()->getCallStackUnwinding();
            pthrd_printf("Handling completed stackwalk for %d/%d\n", proc->getPid(), thr->getLWP());
            bool result = proc->plat_handleStackInfo(stk_resp, stk_cb);
            if (!result) {
               pthrd_printf("Error handling stack info\n");
               had_error = true;
            }
            did_something = true;
            all_responses.erase(i++);
         }
         else {
            a_resp = stk_resp;
            i++;
         }
      }
      if (!did_something) 
         int_process::waitForAsyncEvent(a_resp);
   }

   return !had_error;
}

bool RemoteIOSet::getFileNames(FileSet *fset)
{
   MTLock lock_this_func;
   bool had_error = false;

   if (!fset) {
      perr_printf("NULL FileSet passed to getFileNames\n");
      globalSetLastError(err_badparam, "Unexpected NULL parameter");
      return false;
   }
   ProcessSet::ptr procs = pset.lock();
   if (!procs || procs->empty()) {
      perr_printf("getFileNames attempted on empty proces set\n");
      globalSetLastError(err_badparam, "getFileNames on empty process set");
      return false;
   }

   pthrd_printf("RemoteIOSet::getFileNames called on %lu processes\n", (unsigned long)procs->size());

   set<FileSetResp_t *> all_resps;
   int_processSet *procset = procs->getIntProcessSet();
   procset_iter iter("getFileNames", had_error, ERR_CHCK_NORM);   
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_remoteIO *proc = (*i)->llproc()->getRemoteIO();
      if (!proc) {
         perr_printf("getFileNames attempted on non RemoteIO process\n");
         had_error = true;
         continue;
      }

      FileSetResp_t *new_resp = new FileSetResp_t(fset, proc);
      bool result = proc->plat_getFileNames(new_resp);
      if (!result) {
         pthrd_printf("Error running plat_getFileNames on %d\n", proc->getPid());
         proc->setLastError(err_internal, "Internal error getting filenames");
         had_error = true;
         delete new_resp;
         continue;
      }

      all_resps.insert(new_resp);
   }

   for (set<FileSetResp_t *>::iterator i = all_resps.begin(); i != all_resps.end(); i++) {
      FileSetResp_t *resp = *i;
      resp->getProc()->waitForEvent(resp);
      delete resp;
   }

   return !had_error;
}

bool RemoteIOSet::getFileStatData(FileSet *fset)
{
   MTLock lock_this_func;
   bool had_error = false;

   if (!fset) {
      perr_printf("NULL FileSet passed to getFileStatData\n");
      globalSetLastError(err_badparam, "Unexpected NULL parameter");
      return false;
   }
   ProcessSet::ptr procs = pset.lock();
   if (!procs || procs->empty()) {
      perr_printf("getFileStatData attempted on empty proces set\n");
      globalSetLastError(err_badparam, "getFileStatData on empty process set");
      return false;
   }


   pthrd_printf("RemoteIOSet::getFileStatData called on %lu processes\n", (unsigned long)procs->size());

   set<StatResp_t *> all_resps;

   for (FileSet::iterator i = fset->begin(); i != fset->end(); i++) {
      pthrd_printf("About to access proc %p\n", (void*)i->first->llproc());
      fflush(stderr);
      int_remoteIO *proc = i->first->llproc()->getRemoteIO();
      if (!proc) {
         perr_printf("getFileStatData attempted on non RemoteIO process\n");
         had_error = true;
         continue;
      }
      FileInfo &fi = i->second;
      int_fileInfo_ptr info = fi.getInfo();
      if (info->filename.empty()) {
         perr_printf("Empty filename in stat operation on %d\n", proc->getPid());
         proc->setLastError(err_badparam, "Empty filename specified in stat operation");
         had_error = true;
         continue;
      }
      
      bool result = proc->plat_getFileStatData(info->filename, &info->stat_results, all_resps);
      if (!result) {
         pthrd_printf("Error while requesting file stat data on %d\n", proc->getPid());
         had_error = true;
         continue;
      }
   }

   for (set<StatResp_t *>::iterator i = all_resps.begin(); i != all_resps.end(); i++) {
      StatResp_t *resp = *i;
      resp->getProc()->waitForEvent(resp);
      delete resp;
   }

   return !had_error;
}

bool RemoteIOSet::readFileContents(const FileSet *fset)
{
   MTLock lock_this_func;
   bool had_error = false;

   if (!fset) {
      perr_printf("NULL FileSet passed to getFileStatData\n");
      globalSetLastError(err_badparam, "Unexpected NULL parameter");
      return false;
   }

   set<FileReadResp_t *> resps;

   for (FileSet::const_iterator i = fset->begin(); i != fset->end(); i++) {
      int_remoteIO *proc = i->first->llproc()->getRemoteIO();
      if (!proc) {
         perr_printf("getFileStatData attempted on non RemoteIO\n");
         had_error = true;
         continue;
      }

      int_eventAsyncFileRead *fileread = new int_eventAsyncFileRead();
      fileread->offset = 0;
      fileread->whole_file = true;
      fileread->filename = i->second.getFilename();
      bool result = proc->plat_getFileDataAsync(fileread);
      if (!result) {
         pthrd_printf("Error while requesting file data on %d\n", proc->getPid());
         had_error = true;
         delete fileread;
         continue;
      }
   }

   return !had_error;
}

MemoryUsageSet::MemoryUsageSet(ProcessSet::ptr ps_) :
   wps(ps_)
{
   pthrd_printf("Constructed MemoryUsageSet %p on procset of size %lu\n", (void*)this, (unsigned long) ps_->size());
}

MemoryUsageSet::~MemoryUsageSet()
{
   wps = ProcessSet::weak_ptr();
}

bool MemoryUsageSet::usedX(std::map<Process::const_ptr, unsigned long> &used, MemoryUsageSet::mem_usage_t mu) const
{
   MTLock lock_this_func;
   bool had_error = false;

   const char *mu_str = NULL;
   switch (mu) {
      case mus_shared:
         mu_str = "sharedUsed";
         break;
      case mus_heap:
         mu_str = "heapUsed";
         break;
      case mus_stack:
         mu_str = "stackUsed";
         break;
      case mus_resident:
         mu_str = "resident";
         break;
   }
   ProcessSet::ptr ps = wps.lock();
   if (!ps) {
      perr_printf("%s on deleted process set\n", mu_str);
      globalSetLastError(err_badparam, "memory usage query attempted on deleted ProcessSet object");
      return false;
   }
   set<MemUsageResp_t *> resps;
   const unsigned int max_operations = (mu == mus_resident ? ps->size() * 4 : ps->size());
   unsigned long *result_sizes = new unsigned long[max_operations];
   pthrd_printf("Performing set operation getting %s (may use %u ops)\n", mu_str, max_operations);

   map<int_memUsage *, MemUsageResp_t *> shared_results, stack_results, heap_results, resident_results;

   int_processSet *procset = ps->getIntProcessSet();
   procset_iter iter(mu_str, had_error, ERR_CHCK_ALL);
   unsigned int cur = 0;
   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_memUsage *proc = (*i)->llproc()->getMemUsage();
      if (!proc) {
         perr_printf("GetMemUsage not supported\n");
         had_error = true;
         continue;
      }

      int start, end;
      if (mu == mus_resident && proc->plat_residentNeedsMemVals()) {
         //To get resident we need shared, heap and stack first.  Do each one.
         start = (int) mus_shared;
         end = (int) mus_stack;
      }
      else {
         //Just do the operation that was requested.
         start = mu;
         end = mu;
      }
      for (int j = start; j <= end; j++) {
         assert(cur < max_operations);
         MemUsageResp_t *resp = new MemUsageResp_t(result_sizes + cur++, proc);
         bool result = false;
         switch ((mem_usage_t) j) {
            case mus_shared:
               result = proc->plat_getSharedUsage(resp);
               break;
            case mus_heap:
               result = proc->plat_getHeapUsage(resp);
               break;
            case mus_stack:
               result = proc->plat_getStackUsage(resp);
               break;
            case mus_resident:
               result = proc->plat_getResidentUsage(0, 0, 0, resp);
               break;
         }
      
         if (!result) {
            had_error = true;
            delete resp;
            continue;
         }

         switch ((mem_usage_t) j) {
            case mus_shared:
               shared_results.insert(make_pair(proc, resp));
               break;
            case mus_heap:
               heap_results.insert(make_pair(proc, resp));
               break;
            case mus_stack:
               stack_results.insert(make_pair(proc, resp));
               break;
            case mus_resident:
               resident_results.insert(make_pair(proc, resp));
               break;
         }

         resps.insert(resp);
      }
   }

   for (set<MemUsageResp_t *>::iterator i = resps.begin(); i != resps.end(); i++)
      (*i)->getProc()->waitForEvent((*i));
   resps.clear();

   if (mu == mus_resident) { 
      for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
         int_memUsage *proc = (*i)->llproc()->getMemUsage();
         if (!proc)
            continue;
         if (!proc->plat_residentNeedsMemVals())
            continue;

         map<int_memUsage *, MemUsageResp_t *>::iterator sh, st, he;
         sh = shared_results.find(proc);
         st = stack_results.find(proc);
         he = heap_results.find(proc);
         if (sh == shared_results.end() || st == stack_results.end() || he == heap_results.end() ||
             sh->second->hadError() || st->second->hadError() || he->second->hadError()) {
            perr_printf("Failed to read shared stack or heap on process %d for resident memory\n",
                        proc->getPid());
            continue;
         }
         pthrd_printf("Currently doing operation %u of %u\n", cur, max_operations);
         assert(cur < max_operations);
         MemUsageResp_t *resp = new MemUsageResp_t(result_sizes + cur++, proc);
         bool result = proc->plat_getResidentUsage(*st->second->get(), *he->second->get(), *sh->second->get(), resp);
         if (!result) {
            perr_printf("Error calculating resident usage from stack, heap and shared on %d\n", proc->getPid());
            delete resp;
            continue;
         }
         resident_results.insert(make_pair(proc, resp));
         resps.insert(resp);
      }

      for (set<MemUsageResp_t *>::iterator i = resps.begin(); i != resps.end(); i++)
         (*i)->getProc()->waitForEvent(*i);
   }

   for (int_processSet::iterator i = iter.begin(procset); i != iter.end(); i = iter.inc()) {
      int_memUsage *proc = (*i)->llproc()->getMemUsage();
      if (!proc)
         continue;   
      map<int_memUsage *, MemUsageResp_t *> *the_results = NULL;
      switch (mu) {
         case mus_shared:
            the_results = &shared_results;
            break;
         case mus_heap:
            the_results = &heap_results;
            break;
         case mus_stack:
            the_results = &stack_results;
            break;
         case mus_resident:
            the_results = &resident_results;
            break;
      }
      map<int_memUsage *, MemUsageResp_t *>::iterator j = the_results->find(proc);
      if (j == the_results->end())
         continue;
      MemUsageResp_t *resp = j->second;
      used.insert(make_pair(resp->getProc()->proc(), *resp->get()));         
   }

   map<int_memUsage *, MemUsageResp_t *>::iterator i;
   for (i = shared_results.begin(); i != shared_results.end(); i++)
      delete i->second;
   for (i = stack_results.begin(); i != stack_results.end(); i++)
      delete i->second;
   for (i = heap_results.begin(); i != heap_results.end(); i++)
      delete i->second;
   for (i = resident_results.begin(); i != resident_results.end(); i++)
      delete i->second;

   delete [] result_sizes;

   return !had_error;
}

bool MemoryUsageSet::sharedUsed(std::map<Process::const_ptr, unsigned long> &used) const
{
   return usedX(used, mus_shared);
}

bool MemoryUsageSet::heapUsed(std::map<Process::const_ptr, unsigned long> &used) const
{
   return usedX(used, mus_heap);
}

bool MemoryUsageSet::stackUsed(std::map<Process::const_ptr, unsigned long> &used) const
{
   return usedX(used, mus_stack);
}

bool MemoryUsageSet::resident(std::map<Process::const_ptr, unsigned long> &res) const
{
   return usedX(res, mus_resident);
}
