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

//TODO: isRunning(), isStopped()...
//TODO: Bug if trying to stop an already stopped process

#ifndef PROCSTATE_H_
#define PROCSTATE_H_

#include "basetypes.h"
#include "registers/MachRegister.h"
#include "Architecture.h"
#include "PCProcess.h"


#include <vector>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <stddef.h>
#include <utility>

namespace Dyninst {
namespace Stackwalker {

class LibraryState;
class ThreadState;
class Walker;

class SW_EXPORT ProcessState {
   friend class Walker;
protected:
   Dyninst::PID pid;
   std::string exec_path;
   LibraryState *library_tracker;
   Walker *walker;
   static std::map<Dyninst::PID, ProcessState *> proc_map;
   std::string executable_path;

   ProcessState(Dyninst::PID pid_ = 0, std::string executable_path_ = std::string(""));
   void setPid(Dyninst::PID pid_);
public:

  //look-up Process-State by pid
  static ProcessState* getProcessStateByPid(Dyninst::PID pid);

  //Read register in thread
  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val) = 0;
  
  //Read memory in process
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0;

  //Return list of available threads
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0;
  
  //Return the default thread
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0;

  //Return PID
  virtual Dyninst::PID getProcessId();

  //Return the size of an address in process in bytes
  virtual unsigned getAddressWidth() = 0;

  virtual Dyninst::Architecture getArchitecture() = 0;

  virtual ~ProcessState();

  Walker *getWalker() const;

  void setLibraryTracker(LibraryState *);
  void setDefaultLibraryTracker();
  virtual LibraryState *getLibraryTracker();

  //Allow initialization/uninitialization
  virtual bool preStackwalk(Dyninst::THR_ID tid);
  virtual bool postStackwalk(Dyninst::THR_ID tid);

  virtual bool isFirstParty() = 0;

  std::string getExecutablePath();
};

class ProcSelf : public ProcessState {
 public:
  ProcSelf(std::string exe_path = std::string(""));
  void initialize();

  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual unsigned getAddressWidth();
  virtual bool isFirstParty();
  virtual Dyninst::Architecture getArchitecture();
  virtual ~ProcSelf();
};

class SW_EXPORT ProcDebug : public ProcessState {
 protected:
   Dyninst::ProcControlAPI::Process::ptr proc;
   ProcDebug(Dyninst::ProcControlAPI::Process::ptr p);

   std::set<Dyninst::ProcControlAPI::Thread::ptr> needs_resume;
 public:
  
  static ProcDebug *newProcDebug(Dyninst::PID pid, std::string executable="");
  static ProcDebug *newProcDebug(Dyninst::ProcControlAPI::Process::ptr proc);
  static bool newProcDebugSet(const std::vector<Dyninst::PID> &pids,
                              std::vector<ProcDebug *> &out_set);
  static ProcDebug *newProcDebug(std::string executable, 
                                 const std::vector<std::string> &argv);
  virtual ~ProcDebug();

  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &thrds);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual unsigned getAddressWidth();

  virtual bool preStackwalk(Dyninst::THR_ID tid);
  virtual bool postStackwalk(Dyninst::THR_ID tid);

  
  virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID);
  virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID);
  virtual bool isTerminated();

  virtual bool detach(bool leave_stopped = false);

  Dyninst::ProcControlAPI::Process::ptr getProc();

  static int getNotificationFD();
  std::string getExecutablePath();

  static bool handleDebugEvent(bool block = false);
  virtual bool isFirstParty();

  virtual Dyninst::Architecture getArchitecture();
};

//LibAddrPair.first = path to library, LibAddrPair.second = load address
typedef std::pair<std::string, Address> LibAddrPair;
typedef enum { library_load, library_unload } lib_change_t;
class LibraryState {
 protected:
   ProcessState *procstate;
   std::vector<std::pair<LibAddrPair, unsigned int> > arch_libs;
 public:
   LibraryState(ProcessState *parent);
   virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0;
   virtual bool getLibraries(std::vector<LibAddrPair> &libs, bool allow_refresh = true) = 0;
   virtual void notifyOfUpdate() = 0;
   virtual Address getLibTrapAddress() = 0;
   virtual bool getLibc(LibAddrPair &lc);
   virtual bool getLibthread(LibAddrPair &lt);
   virtual bool getAOut(LibAddrPair &ao) = 0;
   virtual ~LibraryState();

   virtual bool updateLibsArch(std::vector<std::pair<LibAddrPair, unsigned int> > &alibs);
};

}
}

#endif
