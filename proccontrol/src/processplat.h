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

#if !defined(PROCESSPLAT_H_)
#define PROCESSPLAT_H_

#include <map>
#include <set>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>
#include "int_process.h"
#include "resp.h"

class int_libraryTracking : virtual public int_process
{
  public:
   static bool default_track_libs;
   LibraryTracking *up_ptr;
   int_libraryTracking(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                              std::vector<std::string> envp, std::map<int,int> f);
   int_libraryTracking(Dyninst::PID pid_, int_process *p);
   virtual ~int_libraryTracking();
   virtual bool setTrackLibraries(bool b, int_breakpoint* &bp, Address &addr, bool &add_bp) = 0;
   virtual bool isTrackingLibraries() = 0;
};

class int_LWPTracking : virtual public int_process
{
  public:
   bool lwp_tracking;
   LWPTracking *up_ptr;
   int_LWPTracking(Dyninst::PID p, std::string e, std::vector<std::string> a,
                   std::vector<std::string> envp, std::map<int,int> f);
   int_LWPTracking(Dyninst::PID pid_, int_process *p);
   virtual ~int_LWPTracking();
   virtual bool lwp_setTracking(bool b);
   virtual bool plat_lwpChangeTracking(bool b);
   virtual bool lwp_getTracking();
   virtual bool lwp_refreshPost(result_response::ptr &resp);
   virtual bool lwp_refreshCheck(bool &change);
   virtual bool lwp_refresh();
   virtual bool plat_lwpRefreshNoteNewThread(int_thread *thr);
   virtual bool plat_lwpRefresh(result_response::ptr resp);
};

class int_threadTracking : virtual public int_process
{
  protected:
  public:
   ThreadTracking *up_ptr;
   int_threadTracking(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                          std::vector<std::string> envp, std::map<int,int> f);
   int_threadTracking(Dyninst::PID pid_, int_process *p);
   virtual ~int_threadTracking();
   virtual bool setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps,
                                         bool &add_bp) = 0;
   virtual bool isTrackingThreads() = 0;
   virtual bool refreshThreads() = 0;
};

class int_followFork : virtual public int_process
{
  protected:
   FollowFork::follow_t fork_tracking;
  public:
   FollowFork *up_ptr;
   int_followFork(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                  std::vector<std::string> envp, std::map<int,int> f);
   int_followFork(Dyninst::PID pid_, int_process *p);
   virtual ~int_followFork();
   virtual bool fork_setTracking(FollowFork::follow_t b) = 0;
   virtual FollowFork::follow_t fork_isTracking() = 0;
};

class int_callStackUnwinding : virtual public int_process
{
  public:
   int_callStackUnwinding(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                                  std::vector<std::string> envp, std::map<int,int> f);
   int_callStackUnwinding(Dyninst::PID pid_, int_process *p);
   virtual ~int_callStackUnwinding();
   virtual bool plat_getStackInfo(int_thread *thr, stack_response::ptr stk_resp) = 0;
   virtual bool plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs) = 0;
};

class int_memUsage : virtual public resp_process
{
  public:
   MemoryUsage *up_ptr;
   int_memUsage(Dyninst::PID p, std::string e, std::vector<std::string> a,
                std::vector<std::string> envp, std::map<int,int> f);
   int_memUsage(Dyninst::PID pid_, int_process *p);
   virtual ~int_memUsage();
   virtual bool plat_getStackUsage(MemUsageResp_t *resp) = 0;
   virtual bool plat_getHeapUsage(MemUsageResp_t *resp) = 0;
   virtual bool plat_getSharedUsage(MemUsageResp_t *resp) = 0;

   virtual bool plat_residentNeedsMemVals() = 0;
   virtual bool plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu,
                                      MemUsageResp_t *resp) = 0;
};

class int_multiToolControl : virtual public int_process
{
  protected:
  public:
   MultiToolControl *up_ptr;
   int_multiToolControl(Dyninst::PID p, std::string e, std::vector<std::string> a,
                        std::vector<std::string> envp, std::map<int,int> f);
   int_multiToolControl(Dyninst::PID pid_, int_process *p);
   virtual ~int_multiToolControl();
   virtual std::string mtool_getName() = 0;
   virtual MultiToolControl::priority_t mtool_getPriority() = 0;
   virtual MultiToolControl *mtool_getMultiToolControl() = 0;
};

class int_signalMask : virtual public int_process
{
  protected:
   dyn_sigset_t sigset;
  public:
   SignalMask *up_ptr;
   int_signalMask(Dyninst::PID p, std::string e, std::vector<std::string> a,
                  std::vector<std::string> envp, std::map<int,int> f);
   int_signalMask(Dyninst::PID pid_, int_process *p);
   virtual ~int_signalMask();
   virtual bool allowSignal(int signal_no) = 0;
   dyn_sigset_t getSigMask() { return sigset; }
   void setSigMask(dyn_sigset_t msk) { sigset = msk; }
};

class int_remoteIO : virtual public resp_process
{
  public:
   RemoteIO *up_ptr;
   int_remoteIO(Dyninst::PID p, std::string e, std::vector<std::string> a,
                std::vector<std::string> envp, std::map<int,int> f);
   int_remoteIO(Dyninst::PID pid_, int_process *p);
   virtual ~int_remoteIO();

   bool getFileNames(FileSet *fset);
   virtual bool plat_getFileNames(FileSetResp_t *resp) = 0;
   
   bool getFileStatData(FileSet &files);
   virtual bool plat_getFileStatData(std::string filename, Dyninst::ProcControlAPI::stat64_ptr *stat_results,
                                     std::set<StatResp_t *> &resps) = 0;

   bool getFileDataAsync(const FileSet &files);
   virtual bool plat_getFileDataAsync(int_eventAsyncFileRead *fileread) = 0;

   virtual int getMaxFileReadSize() = 0;
};

class int_fileInfo
{
  public:
   int_fileInfo();
   ~int_fileInfo();

   std::string filename;
   stat64_ptr stat_results;
   size_t cur_pos;
};

#endif
