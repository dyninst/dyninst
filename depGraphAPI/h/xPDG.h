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

#if !defined(xPDG_GRAPH_H)
#define xPDG_GRAPH_H

#include "Node.h"
#include "Graph.h"

#include "dynptr.h"

class BPatch_function;

namespace Dyninst {

namespace DepGraphAPI {

// This class represents an Extended Program Dependence Graph for a given function.
class xPDG : public Graph {
public:
    typedef dyn_shared_ptr<xPDG> Ptr;

private:
    typedef BPatch_function Function;
public:
    // Creates and returns the xPDG for the given function.
    static xPDG::Ptr analyze(Function *func);

    virtual ~xPDG() {};

    // Returns the entry node for the xPDG.
    Node::Ptr virtualEntryNode() { return virtEntryNode_; }

    // Creates an empty xPDG and returns.
    static Ptr createGraph() { return xPDG::Ptr(new xPDG()); }
private:
    xPDG();

    // Node to make sure everyone is reachable...
    Node::Ptr virtEntryNode_;
};
};
}
#endif
