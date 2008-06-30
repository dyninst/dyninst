/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PROCSTATE_H_
#define PROCSTATE_H_

#include "basetypes.h"

#include <vector>
#include <map>
#include <queue>

namespace Dyninst {
namespace Stackwalker {

class ProcessState {
public:
  //Read register in thread
  virtual bool getRegValue(reg_t reg, Dyninst::THR_ID thread, regval_t &val) = 0;
  
  //Read memory in process
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0;

  //Return list of available threads
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0;
  
  //Return the default thread
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0;

  //Return PID
  virtual Dyninst::PID getProcessId() = 0;

  //Return the size of an address in process in bytes
  virtual unsigned getAddressWidth() = 0;

  //Allow initialization/uninitialization
  virtual void preStackwalk();
  virtual void postStackwalk();
  
  virtual ~ProcessState();
};

class ProcSelf : public ProcessState {
 protected:
  Dyninst::PID mypid;
 public:
  ProcSelf();

  virtual bool getRegValue(reg_t reg, Dyninst::THR_ID thread, regval_t &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual Dyninst::PID getProcessId();
  virtual unsigned getAddressWidth();
  virtual ~ProcSelf();
};

typedef enum {
  ps_neonatal,
  ps_attached_intermediate,
  ps_attached,
  ps_running,
  ps_exited,
  ps_errorstate,
} proc_state;

class ProcDebug;

typedef enum {
   dbg_err,
   dbg_exited,
   dbg_crashed,
   dbg_stopped,
   dbg_other,
   dbg_noevent,
   //BGL
   dbg_continued,
   dbg_mem_ack,
   dbg_reg_ack,
   dbg_allregs_ack,
   dbg_attached,
} dbg_t;

struct DebugEvent {
   dbg_t dbg;
   union {
      int idata;
      void *pdata;
   } data;
   unsigned size;
   ProcDebug *proc;

   DebugEvent() : dbg(dbg_noevent), size(0), proc(NULL) {}
};

struct procdebug_ltint
{
   bool operator()(int a, int b) const;
};

class ProcDebug : public ProcessState {
 protected:
  ProcDebug(Dyninst::PID pid);
  ProcDebug(const std::string &executable, 
            const std::vector<std::string> &argv);
  
  /**
   * attach() is the top-level command, and is implemented by debug_attach and
   * debug_waitfor_attach. 
   **/
  virtual bool attach();
  static bool multi_attach(std::vector<ProcDebug *> &pids);


  virtual bool debug_attach() = 0;
  virtual bool debug_waitfor_attach();

  virtual bool debug_use_intermediate_attach();
  virtual bool debug_intermediate_attach();
  virtual bool debug_waitfor_intermediate_attach();

  virtual bool create(const std::string &executable, 
                      const std::vector<std::string> &argv);
  virtual bool debug_create(const std::string &executable, 
                            const std::vector<std::string> &argv) = 0;
  virtual bool debug_waitfor_create();
  
  /**
   * pause() is the top-level command (under the public section), and is 
   * implemented by debug_pause() and debug_waitfor_pause()
   **/
  virtual bool debug_pause() = 0;
  virtual bool debug_waitfor_pause();

  virtual bool debug_continue() = 0;
  virtual bool debug_continue_with(long sig) = 0;
  virtual bool debug_waitfor_continue();

  virtual bool debug_handle_signal(DebugEvent *ev) = 0;
  virtual bool debug_handle_event(DebugEvent ev) = 0;

  static DebugEvent debug_get_event(bool block);
  static bool debug_wait_and_handle(bool block, bool &handled);
 public:
  
  static ProcDebug *newProcDebug(Dyninst::PID pid);
  static bool newProcDebugSet(const std::vector<Dyninst::PID> &pids,
                              std::vector<ProcDebug *> &out_set);
  static ProcDebug *newProcDebug(const std::string &executable, 
                                 const std::vector<std::string> &argv);
  virtual ~ProcDebug();

  virtual bool getRegValue(reg_t reg, Dyninst::THR_ID thread, regval_t &val) = 0;
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0;
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0;
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0;
  virtual unsigned getAddressWidth() = 0;

  virtual Dyninst::PID getProcessId();
  virtual void preStackwalk();
  virtual void postStackwalk();

  
  virtual bool pause();
  virtual bool resume();
  virtual bool isTerminated();

  static int getNotificationFD();
  static bool handleDebugEvent(bool block = false);
 protected:
  proc_state state;
  bool isRunning;
  bool user_isRunning;
  bool should_resume;
  Dyninst::PID pid;
  static std::map<Dyninst::PID, ProcDebug *, procdebug_ltint> proc_map;
 public:
  static int pipe_in;
  static int pipe_out;
};

}
}

#endif
