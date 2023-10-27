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

#include "InstWidget.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/debug.h"
#include "CFG.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include <string>
#include <iostream>

using namespace Dyninst;
using namespace Relocation;


InstWidget::Ptr InstWidget::create(instPoint *i) {
  return Ptr(new InstWidget(i));
}

TrackerElement *InstWidget::tracker() const {
   // Trackers need a base tramp and a block...
   // The block is "where we're going to" for edge instrumentation 
   // purposes. 

   InstTracker *e = NULL;
   // Addr depends on the tramp type - pre, post, etc.
   // But we can't actually determine that with at-insn
   // instPs; fix when Wenbin fixes the instP data structure.
   if (point_->block_compat() == NULL)
      return NULL;

   e = new InstTracker(point_->addr_compat(), point_->tramp(), point_->block_compat(), point_->func());
   
   return e;
}

InstWidget::~InstWidget() {
}

bool InstWidget::generate(const codeGen &,
                    const RelocBlock *,
                    CodeBuffer &buffer) {
   // We should work baseTramp code generation into the CodeBuffer
   // system, but that's for future work...
   InstWidgetPatch *patch = new InstWidgetPatch(point_->tramp());
   buffer.addPatch(patch, tracker());
   return true;
}

std::string InstWidget::format() const {
   return "InstWidget()";
}

// Could be a lot smarter here...
bool InstWidgetPatch::apply(codeGen &gen, CodeBuffer *) {
   relocation_cerr << "\t\t InstWidgetPatch::apply " << this << " /w/ tramp " << tramp << std::endl;

   gen.registerInstrumentation(tramp, gen.currAddr());
   bool ret = tramp->generateCode(gen, gen.currAddr());
   return ret;
}

unsigned InstWidgetPatch::estimate(codeGen &) {
   return 0;
}

InstWidgetPatch::~InstWidgetPatch() {
   // Don't delete the tramp because it belongs to 
   // an instPoint.
}

bool RemovedInstWidgetPatch::apply(codeGen &gen, CodeBuffer *) {
  // Just want to leave a marker here for later.
  gen.registerRemovedInstrumentation(tramp, gen.currAddr());
  return true;
}

unsigned RemovedInstWidgetPatch::estimate(codeGen &) {
   return 0;
}
