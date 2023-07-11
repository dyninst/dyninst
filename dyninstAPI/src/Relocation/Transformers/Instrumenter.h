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

#if !defined(_R_T_INSTRUMENTER_H_)
#define _R_T_INSTRUMENTER_H_

#include <list>
#include <map>
#include <utility>
#include "Transformer.h"
#include "dyninstAPI/src/instPoint.h"

class edge_instance;

namespace Dyninst {
namespace Relocation {

class Instrumenter : public Transformer {
 public:

  virtual bool process(RelocBlock *cur, RelocGraph *);
  
     Instrumenter() : skip(NULL) {}
  
  virtual ~Instrumenter() {}
  
 private:
  typedef enum {
    Before,
    After } When;
    
  typedef std::pair<RelocBlock *, When> InsertPoint;  
  typedef std::map<InsertPoint, std::list<RelocBlock *> > EdgeRelocBlocks;
  typedef boost::shared_ptr<CFWidget> CFWidgetPtr;

  // The instrumenters that can add new RelocBlocks have the CFG as an
  // argument
  bool funcEntryInstrumentation(RelocBlock *trace, RelocGraph *cfg);
  bool edgeInstrumentation(RelocBlock *trace, RelocGraph *cfg);
  bool postCallInstrumentation(RelocBlock *trace, RelocGraph *cfg);
  bool funcExitInstrumentation(RelocBlock *trace, RelocGraph *cfg);

  bool blockEntryInstrumentation(RelocBlock *trace);
  bool blockExitInstrumentation(RelocBlock *trace);
  bool preCallInstrumentation(RelocBlock *trace);
  bool insnInstrumentation(RelocBlock *trace);

  bool handleUnconditionalExitInstrumentation(RelocBlock *trace, RelocGraph *cfg, instPoint *exit);
  bool handleCondIndExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit);
  bool handleCondDirExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit);

  WidgetPtr makeInstrumentation(PatchAPI::Point *point);

  struct CallFallthroughPredicate {
     bool operator()(RelocEdge *e);
  };

  struct EdgePredicate {

	EdgePredicate(edge_instance *e) : e_(e) {}
    bool operator()(RelocEdge *e);
    edge_instance *e_;
  };

  RelocBlock *skip;

};

}
}


#endif
