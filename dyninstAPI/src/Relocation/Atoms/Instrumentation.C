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

#include "Instrumentation.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/debug.h"

#include "../CodeTracker.h"

using namespace Dyninst;
using namespace Relocation;


Inst::Ptr Inst::create() {
  return Ptr(new Inst());
}

void Inst::addBaseTramp(baseTramp *b) {
  // We should make this an on-the-fly operation.
  b->doOptimizations();

  baseTrampInstance *bti = new baseTrampInstance(b, NULL);
  assert(bti->baseT);
  baseTramps_.push_back(bti);
}

bool Inst::empty() const {
  return baseTramps_.empty();
}

TrackerElement *Inst::tracker() const {
  if (baseTramps_.empty()) return NULL;
  
  baseTrampInstance *bt = (*baseTramps_.begin());
  InstTracker *e = new InstTracker(bt->baseT->instP()->addr(), bt);
  return e;
}

bool Inst::generate(GenStack &gens) {
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
    gens.addPatch(patch);
  }

  return true;
}

string Inst::format() const {
  stringstream ret;
  ret << "Inst(" << baseTramps_.size() << ")";
  return ret.str();
}

// Could be a lot smarter here...
bool InstPatch::apply(codeGen &gen, int, int) {
  relocation_cerr << "\t\t InstPatch::apply" << endl;

  return base->generateCode(gen, gen.currAddr(), NULL);
}

bool InstPatch::preapply(codeGen &gen) {
  if (gen.startAddr() == (Address) -1) {
    // baseTramps don't liiiiike this...
    gen.setAddr(0);
  }

  return apply(gen, 0, 0);
}

InstPatch::~InstPatch() {
  // Don't delete the bti because it belongs to our
  // parent
}
