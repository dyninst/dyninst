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

#ifndef _BPatch_edge_h_
#define _BPatch_edge_h_

#include "BPatch_dll.h"

class BPatch_flowGraph;
class BPatch_basicBlock;
class BPatch_point;
class edge_instance;
class BPatch_edge; 

// XXX ignores indirect jumps
typedef enum { 
    CondJumpTaken, CondJumpNottaken, UncondJump, NonJump 
} BPatch_edgeType;

namespace Dyninst {
   namespace ParseAPI {
      class Edge;
      BPATCH_DLL_EXPORT Edge *convert(const BPatch_edge *);
   }
   namespace PatchAPI {
      class PatchEdge;
      BPATCH_DLL_EXPORT PatchEdge *convert(const BPatch_edge *);
   }
}



/** An edge between two blocks
 */
class BPATCH_DLL_EXPORT BPatch_edge {
   friend Dyninst::ParseAPI::Edge *Dyninst::ParseAPI::convert(const BPatch_edge *);
   friend Dyninst::PatchAPI::PatchEdge *Dyninst::PatchAPI::convert(const BPatch_edge *);

 public:

    // BPatch_edge::BPatch_edge
    //
    // constructor
   BPatch_edge(edge_instance *e, BPatch_flowGraph *fg);

    // BPatch_edge::~BPatch_edge
    //
    // destructor
    ~BPatch_edge();

    // BPatch_edge::dump
    //
    // print internal data
    void dump();

    BPatch_basicBlock * getSource();
    BPatch_basicBlock * getTarget();
    BPatch_point * getPoint();
    BPatch_edgeType getType();
    BPatch_flowGraph * getFlowGraph();

 private:

    BPatch_point *createInstPointAtEdge();
    BPatch_point *point;
    edge_instance *edge;
    BPatch_flowGraph *fg;

};


#endif /* _BPatch_edge_h_ */
