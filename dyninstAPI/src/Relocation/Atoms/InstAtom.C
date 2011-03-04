/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "InstAtom.h"
#include "dyninstAPI/src/baseTramp.h"
#include "patchapi_debug.h"
#include "CFG.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include <string>

using namespace Dyninst;
using namespace PatchAPI;


Inst::Ptr Inst::create() {
  return Ptr(new Inst());
}

void Inst::addBaseTramp(baseTramp *b) {
  if (!b) return;
  if (!b->empty()) { 
    // We should make this an on-the-fly operation.
    b->doOptimizations();
    
    baseTrampInstance *bti = new baseTrampInstance(b);
    assert(bti->baseT);
    baseTramps_.push_back(bti);
  }
  else if (b->wasNonEmpty()) {
    // We want to mark our existence here
    removedTramps_.push_back(b);
  }
}

bool Inst::empty() const {
  return (baseTramps_.empty() && removedTramps_.empty());
}

TrackerElement *Inst::tracker(baseTrampInstance *bti) const {
   assert(bti);

   InstTracker *e = NULL;
   // Addr depends on the tramp type - pre, post, etc.
   // But we can't actually determine that with at-insn
   // instPs; fix when Wenbin fixes the instP data structure.
   
   e = new InstTracker(bti->baseT->instP()->addr(), bti, Block::convert(bti->baseT->instP()->block()));
   
   return e;
}

Inst::~Inst() {
#if 0
  // don't do this - we kinda need these later for stackwalking etc.
  for (std::list<baseTrampInstance *>::iterator iter = baseTramps_.begin();
       iter != baseTramps_.end(); ++iter) {
    delete (*iter);
  }
#endif
}

bool Inst::generate(const codeGen &,
                    const Trace *,
                    CodeBuffer &buffer) {
  // Fun for the whole family!
  // Okay. This (initially) is going to hork
  // up all of our address/structure tracking because
  // I just can't be bothered to care. Instead, 
  // we'll get some code, and that's good enough for me.

  // For each baseTramp...
  // Fake a baseTrampInstance (FIXME, TODO, etc. etc.)
  //   bti->generateCodeInlined
  // ... done
  // TODO: baseTramp combining for those rare occasions that
  // someone is crazy enough to do post-instruction + pre-successor
  // instrumentation.

  for (std::list<baseTrampInstance *>::iterator b_iter = baseTramps_.begin();
       b_iter != baseTramps_.end(); ++b_iter) {
     InstPatch *patch = new InstPatch(*b_iter);
     buffer.addPatch(patch, tracker(*b_iter));
  }

  return true;
}

string Inst::format() const {
  stringstream ret;
  ret << "Inst(" << baseTramps_.size() << ")";
  return ret.str();
}

// Could be a lot smarter here...
bool InstPatch::apply(codeGen &gen, CodeBuffer *) {
  relocation_cerr << "\t\t InstPatch::apply" << endl;

  gen.registerInstrumentation(base->baseT, gen.currAddr());

  return base->generateCode(gen, gen.currAddr());

}

unsigned InstPatch::estimate(codeGen &) {
   return 0;
}

InstPatch::~InstPatch() {
  // Don't delete the bti because it belongs to our
  // parent
}

bool RemovedInstPatch::apply(codeGen &gen, CodeBuffer *) {
  // Just want to leave a marker here for later.
  gen.registerRemovedInstrumentation(base, gen.currAddr());
  return true;
}

unsigned RemovedInstPatch::estimate(codeGen &) {
   return 0;
}
