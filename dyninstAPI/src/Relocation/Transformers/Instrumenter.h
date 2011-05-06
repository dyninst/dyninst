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

#if !defined(_R_T_INSTRUMENTER_H_)
#define _R_T_INSTRUMENTER_H_

#include "Transformer.h"
#include "dyninstAPI/src/instPoint.h"

namespace Dyninst {
namespace Relocation {

class Instrumenter : public Transformer {
 public:

  virtual bool process(Trace *cur, RelocGraph *);
  
  Instrumenter() {};
  
  virtual ~Instrumenter() {};
  
 private:
  typedef enum {
    Before,
    After } When;
    
  typedef std::pair<Trace *, When> InsertPoint;  
  typedef std::map<InsertPoint, std::list<Trace *> > EdgeTraces;
  typedef dyn_detail::boost::shared_ptr<CFAtom> CFAtomPtr;

  // The instrumenters that can add new Traces have the CFG as an
  // argument
  bool funcEntryInstrumentation(Trace *trace, RelocGraph *cfg);
  bool edgeInstrumentation(Trace *trace, RelocGraph *cfg);
  bool postCallInstrumentation(Trace *trace, RelocGraph *cfg);

  bool funcExitInstrumentation(Trace *trace);
  bool blockEntryInstrumentation(Trace *trace);
  bool preCallInstrumentation(Trace *trace);
  bool insnInstrumentation(Trace *trace);

  AtomPtr makeInstrumentation(instPoint *point);

  struct CallFallthroughPredicate {
     bool operator()(RelocEdge *e);
  };

  struct EdgePredicate {
  EdgePredicate(edge_instance *e) : e_(e) {};
     bool operator()(RelocEdge *e);

     edge_instance *e_;
  };

};

};
};


#endif
