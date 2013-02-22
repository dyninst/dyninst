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

#define BPATCH_FILE

#include "util.h"
#include "function.h"
#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"
#include "instPoint.h"
#include "BPatch_process.h"
#include "BPatch_point.h"

#include "Parsing.h"

using namespace Dyninst;

string 
edge_type_string(BPatch_edgeType t)
{ 
    string ts = "Invalid Edge Type";
    switch (t) {
    case CondJumpTaken: { ts = "CondJumpTaken"; break; }
    case CondJumpNottaken: { ts = "CondJumpNottaken"; break; }
    case UncondJump: { ts = "UncondJump"; break; } 
    case NonJump: { ts = "NonJump"; break; }
    }
    return ts;
}


BPatch_edgeType 
BPatch_edge::getType()
{
   switch(edge->type()) {
   case ParseAPI::NOEDGE:
         return NonJump;
   case ParseAPI::COND_TAKEN:
         return CondJumpTaken;
   case ParseAPI::COND_NOT_TAKEN:
         return CondJumpNottaken;
   case ParseAPI::DIRECT:
   case ParseAPI::INDIRECT:
         return UncondJump;
      default:
         return NonJump;
   }
}


BPatch_edge::BPatch_edge(edge_instance *e, 
   BPatch_flowGraph *FG)
{
   assert(e);
   edge = e;
   fg = FG;
   
   // point is set when this edge is instrumented. instAddr is set
   // when either this edge or its conditional buddy is instrumented
   point = NULL;    
}

 
BPatch_edge::~BPatch_edge()
{
    //fprintf(stderr,"~BPatch_edge\n");
}


void BPatch_edge::dump()
{
}

BPatch_basicBlock *BPatch_edge::getSource() {
   return fg->findBlock(edge->src());
}

BPatch_basicBlock *BPatch_edge::getTarget() {
   return fg->findBlock(edge->trg());
}

BPatch_flowGraph *BPatch_edge::getFlowGraph() {
   return fg;
}

BPatch_point *BPatch_edge::getPoint()
{
   if (!point) {
      BPatch_flowGraph *cfg = getFlowGraph();
      instPoint *ip = instPoint::edge(cfg->ll_func(),
                                      edge);
      AddressSpace *as = cfg->getllAddSpace();
      assert(as);
      
      BPatch_point *newPoint = new BPatch_point(cfg->getAddSpace(),
                                                cfg->getFunction(),
                                                this,
                                                ip,
                                                as);
      if (newPoint) {
         point = newPoint;
      }
   }
   return point;
}

PatchAPI::PatchEdge *PatchAPI::convert(const BPatch_edge *e) {
   return e->edge;
}

ParseAPI::Edge *ParseAPI::convert(const BPatch_edge *e) {
   return e->edge->edge();
}
