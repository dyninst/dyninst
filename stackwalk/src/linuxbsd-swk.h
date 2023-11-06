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

#ifndef LINUXBSD_SWK_H
#define LINUXBSD_SWK_H

#include "common/h/dyntypes.h"
#include "common/h/SymReader.h"

#include "stackwalk/h/framestepper.h"

#define MAX_TRAP_LEN 8

namespace Dyninst {
namespace Stackwalker {

class SigHandlerStepperImpl : public FrameStepper {
private:
   SigHandlerStepper *parent_stepper;
   void registerStepperGroupNoSymtab(StepperGroup *group);
   bool init_libc;
   bool init_libthread;
public:
   SigHandlerStepperImpl(Walker *w, SigHandlerStepper *parent);
   virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out);
   virtual unsigned getPriority() const;
   virtual void registerStepperGroup(StepperGroup *group);
   virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change);
   virtual const char *getName() const;
   virtual ~SigHandlerStepperImpl();  
};

}
}

#if defined(os_linux)
#include "stackwalk/src/linux-swk.h"
#elif defined(os_freebsd)
#include "stackwalk/src/freebsd-swk.h"
#else
#error "Invalid OS inclusion"
#endif

#endif
