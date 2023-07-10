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

#ifndef x86_swk_h_
#define x86_swk_h_

#include <map>
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/framestepper.h"
#include "common/h/dyntypes.h"

#include "common/src/lru_cache.h"

namespace Dyninst {
namespace Stackwalker {

#define MAX_WANDERER_DEPTH 256

class StepperWandererImpl : public FrameStepper {
 private:
   WandererHelper *whelper;
   FrameFuncHelper *fhelper;
   StepperWanderer *parent;
   bool getWord(Address &words, Address start);
 public:
   StepperWandererImpl(Walker *walker_, StepperWanderer *parent_, WandererHelper *whelper_,
                       FrameFuncHelper *fhelper_);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   void registerStepperGroup(StepperGroup *group);
   virtual const char *getName() const;
   virtual ~StepperWandererImpl();
};

class LookupFuncStart : public FrameFuncHelper
{
private:
   static std::map<Dyninst::PID, LookupFuncStart*> all_func_starts;
   LookupFuncStart(ProcessState *proc_);
   int ref_count;

   void updateCache(Address addr, alloc_frame_t result);
   bool checkCache(Address addr, alloc_frame_t &result);
   //We need some kind of re-entrant safe synhronization before we can
   // globally turn this caching on, but it would sure help things.
   static const unsigned int cache_size = 64;
   LRUCache<Address, alloc_frame_t> cache;
public:
   static LookupFuncStart *getLookupFuncStart(ProcessState *p);
   void releaseMe();
   virtual alloc_frame_t allocatesFrame(Address addr);
   ~LookupFuncStart();
   static void clear_func_mapping(Dyninst::PID);
};

}
}


#endif
