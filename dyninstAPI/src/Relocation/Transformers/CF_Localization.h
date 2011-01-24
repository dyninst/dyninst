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

#if !defined(_R_T_CF_LOCALIZATION_H_)
#define _R_T_CF_LOCALIZATION_H_

#include "Transformer.h"
#include "../CodeMover.h"

class int_function;
class int_block; 

namespace Dyninst {
namespace Relocation {


// Rewire a set of Traces so that we do control
// transfers within them rather than back to 
// original code. 
//
// As a side note, track how many edges we rewire
// in this way so we can tell where we'll need
// patch branches.
class LocalizeCF : public Transformer {
  public:
  /// Mimics typedefs in CodeMover.h, but I don't want
  // to include that file.
  typedef std::list<TracePtr> TraceList;
  //typedef std::map<Address, TraceList> TraceMap;
  typedef std::map<int_block *, TracePtr> TraceMap;

  virtual bool processTrace(TraceList::iterator &);
  virtual bool postprocess(TraceList &); 

  LocalizeCF(const TraceMap &bmap, PriorityMap &p) :
  bMap_(bmap), pMap_(p) {};

  virtual ~LocalizeCF() {};

  TracePtr findTrace(Address addr, int_function *func);

 private:
  int getInEdgeCount(const int_block *bbl);
  void recordIncomingEdges(int_block *bbl);

  // Borrowed from the CodeMover, we don't change it
  const TraceMap &bMap_;
  // And the priority list that we modify
  PriorityMap &pMap_;

  // First element in the pair is the number of incoming edges
  // Second is the number of removed edges.
  std::map<int_block *, std::pair<int, int> > counts_;

};
};
};


#endif
