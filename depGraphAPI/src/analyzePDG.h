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
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 *
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 *
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef INTRAFUNCTIONPDGCREATOR_H_
#define INTRAFUNCTIONPDGCREATOR_H_

#include <map>
#include <set>
#include "Absloc.h"
#include "Edge.h"
#include "Graph.h"
#include "Node.h"
#include "DDG.h"
#include "DepGraphNode.h"

#include "boost/tuple/tuple.hpp"

class BPatch_function;

using namespace std;

namespace Dyninst {
namespace DepGraphAPI {

#if 0
    // Let's just use the extended version, since the DDG + CDG seems to be incomplete...
/**
 * The tool that creates the Program Dependence Graph (PDG) associated with a given 
 * function (currently BPatch_function). It creates a Data Dependency Graph (DDG)
 * and a Control Dependency Graph (CDG) using a intraFunctionDDGCreator and
 * a intraFunctionCDGCreator. These graphs are merged to create a PDG.
 */
class PDGAnalyzer {
private:
  typedef BPatch_function Function;
  typedef Node::Ptr NodePtr;
  typedef set<NodePtr> NodeSet;
  typedef NodeSet::iterator NodeIter;
  typedef Edge::Ptr EdgePtr;
  typedef set<EdgePtr> EdgeSet;
  typedef Graph::Ptr GraphPtr;
  typedef dyn_detail::boost::shared_ptr<Absloc> AbslocPtr;
  typedef map<AbslocPtr, NodePtr> AbslocMap;

  /**
   * The function whose PDG is being created.
   */
  Function* func;
  
  /**
   * Program Dependence Graph.
   */
  GraphPtr PDG;

public:
  /**
   * Creates an PDGAnalyzer object associated with the given function.
   */
  static PDGAnalyzer create(Function *func);
  
  /**
   * Returns the PDG created by this PDGAnalyzer object.
   */
  Graph::Ptr getPDG();

private:
  PDGAnalyzer(Function *f) : func(f) {};
  void analyze();
  // Creates a CDG and merges it with the DDG that *must* be contained in the graph pointed by PDG.
  void mergeCDG();

  // Inserts all outgoing edges from the given node into the given graph. Since the given node is
  // not part of this graph, first associated node(s) in this graph have to be found (say, N nodes).
  // Similarly, nodes associated with the target node(s) have to be found (say, M nodes). Then
  // N x M edges are created and inserted into this graph.
  static void addCdgEdgesToGraph(GraphPtr graph, NodePtr node);
};

#endif

/**
 * The tool that creates the Extended Program Dependence Graph (xPDG) associated with a given 
 * function (currently BPatch_function). It creates a Program Dependency Graph (PDG)
 * and a Flow Dependency Graph (FDG) using a PDGAnalyzer and
 * a intraFunctionFDGCreator. These graphs are merged to create an xPDG.
 */
class PDGAnalyzer {

private:
  typedef BPatch_function Function;
  typedef Node::Ptr NodePtr;
  typedef set<NodePtr> NodeSet;
  typedef NodeSet::iterator NodeIter;
  typedef Edge::Ptr EdgePtr;
  typedef set<EdgePtr> EdgeSet;
  typedef Graph::Ptr GraphPtr;

  typedef dyn_detail::boost::shared_ptr<Absloc> AbslocPtr;
  typedef map<AbslocPtr, NodePtr> AbslocMap;
  typedef map<Address, AbslocMap> NodeMap;

  /**
   * Inputs to the PDG
   */
  DDG::Ptr ddg_;
  Graph::Ptr cdg_;
  Graph::Ptr fdg_;

  
  /**
   * Extended Program Dependence Graph.
   */
  GraphPtr xPDG;

public:
  /**
   * Creates an intraFunctionXPDGCreator object associated with the given function.
   */
  PDGAnalyzer(DDG::Ptr ddg, 
              Graph::Ptr cdg,
              Graph::Ptr fdg) :
      ddg_(ddg),
      cdg_(cdg),
      fdg_(fdg) {};

  /**
   * Merges FDG with the PDG to create an xPDG.
   */
  DDG::Ptr analyze();

private:
  
  /**
   * Given a PDG that is copied into an xPDG, inserts FDG edges into the xPDG.
   */
  void mergeFDG();
};

};
};

#endif /* INTRAFUNCTIONPDGCREATOR_H_ */
