/*
 * Copyright (c) 2007-2008 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// Edge class implementation

#include "Graph.h"
#include "Edge.h"
#include "Node.h"
#include <assert.h>

// Edges are quite simple. The only things we need to implement are:
// * Factory method
// * Constructors

using namespace Dyninst;

Edge::Ptr Edge::createEdge(const NodeSharedPtr source,
                           const NodeSharedPtr target) {
    // We take shared_ptrs as inputs and pull out a weak_ptr
    // to use internally.

    NodePtr s(source);
    NodePtr t(target);

    return Edge::Ptr(new Edge(s, t));
}

// Default constructor; DON'T USE THIS

Edge::Edge() {
    // Impossible to use
    assert(0);
}

// Constructor
Edge::Edge(const NodePtr source, const NodePtr target) :
    source_(source), target_(target) {}


