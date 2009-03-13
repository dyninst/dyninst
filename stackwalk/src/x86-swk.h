/*
 * Copyright (c) 1996-2008 Barton P. Miller
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

#ifndef x86_swk_h_
#define x86_swk_h_

#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/framestepper.h"
#include "dynutil/h/dyntypes.h"

namespace Dyninst {
namespace Stackwalker {

#define MAX_WANDERER_DEPTH 4

class StepperWandererImpl : public FrameStepper {
 private:
   WandererHelper *whelper;
   FrameFuncHelper *fhelper;
   StepperWanderer *parent;
   bool getTopWords(Address *words_out, Address start);
 public:
   StepperWandererImpl(Walker *walker_, WandererHelper *whelper_,
                       FrameFuncHelper *fhelper_, StepperWanderer *parent_);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual ~StepperWandererImpl();
};

class UninitFrameStepperImpl : public FrameStepper {
 private:
   FrameFuncHelper *alloc_frame;
   FrameStepper *parent;
 public:
   UninitFrameStepperImpl(Walker *w, FrameFuncHelper *f, 
                          FrameStepper *parent_);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual ~UninitFrameStepperImpl();
};

}
}


#endif
