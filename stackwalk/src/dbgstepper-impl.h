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

#ifndef DBGSTEPPER_IMPL_
#define DBGSTEPPER_IMPL_

#include "stackwalk/h/framestepper.h"
#include "common/h/ProcReader.h"

namespace Dyninst {

namespace DwarfDyninst {
class DwarfFrameParser;
typedef boost::shared_ptr<DwarfFrameParser> DwarfFrameParserPtr;
}

namespace Stackwalker {

class DebugStepperImpl : public FrameStepper, public Dyninst::ProcessReader {
 private:
    struct cache_t {
      unsigned ra_delta;
      unsigned fp_delta;
      unsigned sp_delta;
      // Note: ra and fp are differences in address, sp is difference in value. 

    cache_t() : ra_delta((unsigned) -1), fp_delta((unsigned) -1), sp_delta((unsigned) -1) {}
    cache_t(unsigned a, unsigned b, unsigned c) : ra_delta(a), fp_delta(b), sp_delta(c) {}
    };

    dyn_hash_map<Address, cache_t> cache_;

    void addToCache(const Frame &cur, const Frame &caller);
    bool lookupInCache(const Frame &cur, Frame &caller);

   Dyninst::Address last_addr_read;
   unsigned long last_val_read;
   unsigned addr_width;
      
   location_t getLastComputedLocation(unsigned long val);
   DebugStepper *parent_stepper;
   const Frame *cur_frame; //TODO: Thread safety
   const Frame *depth_frame; // Current position in the stackwalk
 public:
  DebugStepperImpl(Walker *w, DebugStepper *parent);
  virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
  virtual unsigned getPriority() const;
  virtual void registerStepperGroup(StepperGroup *group);
  virtual bool ReadMem(Address addr, void *buffer, unsigned size);
  virtual bool GetReg(MachRegister reg, MachRegisterVal &val);
  virtual ~DebugStepperImpl();  
  virtual bool start() { return true; }
  virtual bool done() { return true; }
  virtual const char *getName() const;
 protected:
  gcframe_ret_t getCallerFrameArch(Address pc, const Frame &in, Frame &out, 
                                   DwarfDyninst::DwarfFrameParserPtr dinfo, bool isVsyscallPage);
  bool isFrameRegister(MachRegister reg);
  bool isStackRegister(MachRegister reg);
};

}
}

#endif
