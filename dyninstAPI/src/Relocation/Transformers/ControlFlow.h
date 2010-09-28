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

#if !defined(_R_T_CONTROL_FLOW_H_)
#define _R_T_CONTROL_FLOW_H_

#include "Transformer.h"

namespace Dyninst {
namespace Relocation {
// Ensure that each Trace ends with a CFAtom; 
// if the last instruction in the Trace is an explicit
// control transfer we replace it with a CFAtom.
// Otherwise we append a new one.
class CFAtomCreator : public Transformer {
 public:
  virtual bool processTrace(TraceList::iterator &);

  CFAtomCreator() {};

  virtual ~CFAtomCreator() {};

 private:

  struct Succ {
     TargetInt *targ;
     EdgeTypeEnum type;
     Address addr;
     Succ(TargetInt *a, EdgeTypeEnum b, Address c)
     : targ(a), type(b), addr(c) {};
     Succ() 
     : targ(NULL), type(ParseAPI::NOEDGE), addr(0) {};
  };
  typedef std::vector<Succ> SuccVec;

  // Determine who the successors of a block are
  static void getInterproceduralSuccessors(const bblInstance *inst,
					   SuccVec &succ);

  static bool unparsedFallthrough(const bblInstance *inst);
};

};
};
#endif
