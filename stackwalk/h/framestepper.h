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

#ifndef FRAMESTEPPER_H_
#define FRAMESTEPPER_H_

#include "basetypes.h"
#include "procstate.h"
#include <utility>
#include <vector>

namespace Dyninst {

namespace SymtabAPI {
   class Symtab;
}

namespace Stackwalker {

class Walker;
class Frame;
class ProcessState;
class StepperGroup;

typedef enum { gcf_success, gcf_stackbottom, gcf_not_me, gcf_error } gcframe_ret_t;

class SW_EXPORT FrameStepper {
protected:
  Walker *walker;
public:
  FrameStepper(Walker *w);

  virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out) = 0;
  virtual unsigned getPriority() const = 0;

  virtual ProcessState *getProcessState();
  virtual Walker *getWalker();

  virtual void newLibraryNotification(LibAddrPair *libaddr,
                                      lib_change_t change);
  virtual void registerStepperGroup(StepperGroup *group);
  virtual const char *getName() const = 0;

  virtual ~FrameStepper();

  //Default priorities for built in wanderers.
  static const unsigned stackbottom_priority = 0x10000;
  static const unsigned dyninstr_priority = 0x10010;
  static const unsigned sighandler_priority = 0x10020;
  static const unsigned analysis_priority = 0x10058;
  static const unsigned debugstepper_priority = 0x10040;
  static const unsigned frame_priority = 0x10050;
  static const unsigned wanderer_priority = 0x10060;
};

class SW_EXPORT FrameFuncHelper
{
 protected:
   ProcessState *proc;
 public:
   typedef enum {
      unknown_t=0,
      no_frame,
      standard_frame,
      savefp_only_frame,
   } frame_type;
   typedef enum {
      unknown_s=0,
      unset_frame,
      halfset_frame,
      set_frame
   } frame_state;
   typedef std::pair<frame_type, frame_state> alloc_frame_t;
   FrameFuncHelper(ProcessState *proc_);
   virtual alloc_frame_t allocatesFrame(Address addr) = 0;
   virtual ~FrameFuncHelper();
};

/*
class ARM_FrameHelper : public FrameFuncHelper {
  private:
    //PCProcess *proc_;

  public:
    //ARM_FrameHelper(PCProcess *pc);
    virtual FrameFuncHelper::alloc_frame_t allocatesFrame(Address addr);
    virtual ~ARM_FrameHelper();
};
*/

class FrameFuncStepperImpl;
class SW_EXPORT FrameFuncStepper : public FrameStepper {
private:
   FrameFuncStepperImpl *impl;
public:
  FrameFuncStepper(Walker *w, FrameFuncHelper *helper = NULL);
  virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
  virtual unsigned getPriority() const;
  virtual ~FrameFuncStepper();
  virtual void registerStepperGroup(StepperGroup *group);
  virtual const char *getName() const;
};

class DebugStepperImpl;
class SW_EXPORT DebugStepper : public FrameStepper {
private:
   DebugStepperImpl *impl;
public:
  DebugStepper(Walker *w);
  virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
  virtual unsigned getPriority() const;
  virtual void registerStepperGroup(StepperGroup *group);
  virtual ~DebugStepper();
  virtual const char *getName() const;
};

class CallChecker;
class SW_EXPORT WandererHelper
{
 private:
   ProcessState *proc;
   CallChecker * callchecker;
 public:
   typedef enum {
      unknown_s = 0,
      in_func,
      outside_func
   } pc_state;
   WandererHelper(ProcessState *proc_);
   virtual bool isPrevInstrACall(Address addr, Address &target);
   virtual pc_state isPCInFunc(Address func_entry, Address pc);
   virtual bool requireExactMatch();
   virtual ~WandererHelper();
};

class StepperWandererImpl;
class SW_EXPORT StepperWanderer : public FrameStepper {
 private:
   StepperWandererImpl *impl;
 public:
   StepperWanderer(Walker *w, WandererHelper *whelper = NULL,
                   FrameFuncHelper *fhelper = NULL);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~StepperWanderer();
   virtual const char *getName() const;
};

class SigHandlerStepperImpl;
class SW_EXPORT SigHandlerStepper : public FrameStepper {
 private:
   SigHandlerStepperImpl *impl;
 public:
   SigHandlerStepper(Walker *w);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change);
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~SigHandlerStepper();
   virtual const char *getName() const;
};

class BottomOfStackStepperImpl;
class SW_EXPORT BottomOfStackStepper : public FrameStepper {
 private:
   BottomOfStackStepperImpl *impl;
 public:
   BottomOfStackStepper(Walker *w);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change);
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~BottomOfStackStepper();
   virtual const char *getName() const;
};

class DyninstInstrStepperImpl;
class SW_EXPORT DyninstInstrStepper : public FrameStepper {
 private:
   DyninstInstrStepperImpl *impl;
 public:
   DyninstInstrStepper(Walker *w);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~DyninstInstrStepper();
   virtual const char *getName() const;
};

class AnalysisStepperImpl;
class SW_EXPORT AnalysisStepper : public FrameStepper {
  private:
   AnalysisStepperImpl *impl;
  public:
   AnalysisStepper(Walker *w);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~AnalysisStepper();
   virtual const char *getName() const;
};

class SW_EXPORT DyninstDynamicHelper
{
 public:
   virtual bool isInstrumentation(Address ra, Address *orig_ra,
                                  unsigned *stack_height, bool *aligned, bool *entryExit) = 0;
   virtual ~DyninstDynamicHelper();
};

class DyninstDynamicStepperImpl;
class SW_EXPORT DyninstDynamicStepper : public FrameStepper {
 private:
   DyninstDynamicStepperImpl *impl;
 public:
   DyninstDynamicStepper(Walker *w, DyninstDynamicHelper *dihelper = NULL);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~DyninstDynamicStepper();
   virtual const char *getName() const;
};

class DyninstInstFrameStepperImpl;
class SW_EXPORT DyninstInstFrameStepper : public FrameStepper {
 private:
   DyninstInstFrameStepperImpl *impl;
 public:
   DyninstInstFrameStepper(Walker *w);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual ~DyninstInstFrameStepper();
   virtual const char *getName() const;
};

}
}

#endif
