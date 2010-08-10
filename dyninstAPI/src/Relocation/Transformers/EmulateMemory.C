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



#include "Relocation/Transformers/Transformer.h"
#include "Relocation/Transformers/EmulateMemory.h"
#include "debug.h"
#include "Relocation/Atoms/Atom.h"
#include "Relocation/Atoms/CopyInsn.h"
#include "Relocation/Atoms/MemoryEmulator.h"
#include "instPoint.h" // Memory insn modelling requirement.

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool MemEmulatorTransformer::preprocess(TraceList &t) {
  // For each Register create a translator object
  for (Register i = REGNUM_EAX; i <= REGNUM_EDI; ++i) {
    createTranslator(i);
    t.push_back(translators_[i]);
  }

  MemEmulator::initTranslators(translators_);

  return true;
}

// Replace all memory accesses to a non-statically-determinable
// location with an emulation sequence
bool MemEmulatorTransformer::processTrace(TraceList::iterator &iter) {
  if (!((*iter)->bbl())) return true;

  // AssignmentConverter is written in terms of image_func,
  // so translate
  int_function *func = (*iter)->bbl()->func();
  

  AtomList &elements = (*iter)->elements();
  
  for (AtomList::iterator e_iter = elements.begin();
       e_iter != elements.end(); ++e_iter) {
    // If we're not an instruction then skip...

    CopyInsn::Ptr reloc = dyn_detail::boost::dynamic_pointer_cast<CopyInsn>(*e_iter);
    if (!reloc) continue;

    if (!isSensitive(reloc, func)) {
      continue;
    }
    
    if (!canRewriteMemInsn(reloc, func)) {
      //cerr << "\t\t Can't rewrite " << reloc->insn()->format() << endl;
      continue;
    }

    Atom::Ptr replacement = createReplacement(reloc, func);
    if (!replacement) return false;
    
    (*e_iter).swap(replacement);
  }
  return true;
}

bool MemEmulatorTransformer::canRewriteMemInsn(CopyInsn::Ptr reloc,
					       int_function *func) {
  // Let's see if this is an instruction we can rewrite;
  // otherwise complain but let it through (for testing purposes)
  
  codeGen tmpGen(1024);
  tmpGen.setAddrSpace(func->proc()); // needed by insn::generateMem
  
  instruction ugly_insn(reloc->insn()->ptr());
  if (!insnCodeGen::generateMem(tmpGen,
				ugly_insn,
				0, 
				0,
				0,
				Null_Register)) {
    // We can't rewrite it
    /*
    cerr << "Warning: skipping possibly memory sensitive insn "
	 << reloc->insn()->format() << " @ "
	 << std::hex << reloc->addr() << std::dec
	 << " as rewriting is unsupported!" << endl;
    */
    return false;
  }
  return true;
}

Atom::Ptr MemEmulatorTransformer::createReplacement(CopyInsn::Ptr reloc,
						       int_function *func) {
  // MemEmulators want instPoints. How unreasonable.
  instPoint *point = func->findInstPByAddr(reloc->addr());
  if (!point) {
    // bleah...
    point = instPoint::createArbitraryInstPoint(reloc->addr(),
						func->proc(),
						func);
  }
  if (!point) return Atom::Ptr();
  
  // Replace this instruction with a MemEmulator
  Atom::Ptr memE = MemEmulator::create(reloc->insn(),
				       reloc->addr(),
				       point);
  
  return memE;
}

bool MemEmulatorTransformer::isSensitive(CopyInsn::Ptr reloc,
					 int_function *func) {

  image_func *ifunc = func->ifunc();  
  Address image_addr = func->addrToOffset(reloc->addr());

  std::vector<Assignment::Ptr> assignments;
  aConverter.convert(reloc->insn(),
		     image_addr,
		     ifunc,
		     assignments);
  
  for (std::vector<Assignment::Ptr>::const_iterator a_iter = assignments.begin();
       a_iter != assignments.end(); ++a_iter) {
    
    const std::vector<AbsRegion> &ins = (*a_iter)->inputs();
    for (std::vector<AbsRegion>::const_iterator i = ins.begin();
	 i != ins.end(); ++i) {
      if (i->contains(Absloc::Heap)) {
	return true;
      }
      
      // Writes too
      if ((*a_iter)->out().contains(Absloc::Heap)) {
	return true;
      }
    }
  }

  return false;
}

void MemEmulatorTransformer::createTranslator(Register r) {
  Atom::Ptr translator = MemEmulatorTranslator::create(r);
  Trace::Ptr newTrace = Trace::create(translator);
  translators_[r] = newTrace;
};

