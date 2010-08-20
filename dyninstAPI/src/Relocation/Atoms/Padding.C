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

#include "Padding.h"
#include "Atom.h"
#include "instructionAPI/h/Instruction.h"
#include "../CodeTracker.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

/////////////////////////

bool Padding::generate(GenStack &gens) {
  gens().fill(size_, codeGen::cgNOP);
  return true;
}

TrackerElement *Padding::tracker() const {
  EmulatorTracker *e = new EmulatorTracker(addr_, size_);
  return e;
}

Padding::Ptr Padding::create(Address addr, unsigned size) {
  return Ptr(new Padding(addr, size));
}

string Padding::format() const {
  stringstream ret;
  ret << "Padding(" << size_ << ")";
  return ret.str();
}


