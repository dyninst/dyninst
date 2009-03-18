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

#ifndef BLUEGENE_SWK_H
#define BLUEGENE_SWK_H

#include "stackwalk/h/framestepper.h"

#include "dyntypes.h"
#include <set>

namespace Dyninst {
namespace Stackwalker {

class ProcDebugLinux : public ProcDebug {
 protected:
  virtual bool debug_attach(ThreadState *ts);
  virtual bool debug_post_attach(ThreadState *ts);
  virtual bool debug_post_create();
  virtual bool debug_create(const std::string &executable, 
                            const std::vector<std::string> &argv);
  virtual bool debug_pause(ThreadState *thr);
  virtual bool debug_continue(ThreadState *thr);
  virtual bool debug_continue_with(ThreadState *thr, long sig);
  virtual bool debug_handle_signal(DebugEvent *ev);

  virtual bool debug_handle_event(DebugEvent ev);
  void setOptions(Dyninst::THR_ID tid);
 public:
  ProcDebugLinux(Dyninst::PID pid);
  ProcDebugLinux(const std::string &executable, 
                 const std::vector<std::string> &argv);
  virtual ~ProcDebugLinux();

  virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val);
  virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
  virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
  virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
  virtual unsigned getAddressWidth();

  bool pollForNewThreads();
  bool isLibraryTrap(Dyninst::THR_ID thrd);

  static thread_map_t all_threads;
 protected:
  unsigned cached_addr_width;
  void registerLibSpotter();
  Address lib_load_trap;
};

class SigHandlerStepperImpl : public FrameStepper {
private:
   DebugStepper *parent_stepper;
   void registerStepperGroupNoSymtab(StepperGroup *group);
public:
   SigHandlerStepperImpl(Walker *w, DebugStepper *parent);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~SigHandlerStepperImpl();  
};


void getTrapInstruction(char *buffer, unsigned buf_size, unsigned &actual_len, bool include_return);

}
}

#endif
