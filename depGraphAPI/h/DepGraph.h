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

// Analyzer class
//
// This class wraps all of our internal analysis for generating
// a dependence graph. It is intended to be a thin user-visible
// veneer over the inherent ugliness of detailed dependence
// analysis. 

#if !defined(DEP_GRAPH_API_ANALYZER_H)
#define DEP_GRAPH_API_ANALYZER_H

#include "dyn_detail/boost/shared_ptr.hpp"

// Needed for Graph::Ptr
#include "Graph.h"
// And for DDG::Ptr
#include "DDG.h"

class BPatch_function;

namespace Dyninst {

namespace DepGraphAPI {

    // These are internal classes that cannot appear in this 
    // header file
    class DDGAnalyzer;
    class CDGAnalyzer;
    class FDGAnalyzer;
    class PDGAnalyzer;

class Analyzer {
 public:
    typedef BPatch_function Function;

    // Included in case someone wants to make a vector
    // of these things...
    Analyzer() : func_(NULL) {};
    
    // Public interface. This will be extended as we get
    // additional input types.
    static Analyzer createAnalyzer(Function *);

    // Build dependence graphs
    DDG::Ptr createDDG();
    Graph::Ptr createCDG();
    Graph::Ptr createFDG();
    Graph::Ptr createPDG();

 private:
    // Internal constructor; use createAnalyzer() instead.
    Analyzer(Function *func);
    
    Function *func_;
};

};
}
#endif

