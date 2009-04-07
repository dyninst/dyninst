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

#include "intraFunctionPDGCreator.h"

#include "Graph.h"
#include "intraFunctionCreator.h"
#include "intraFunctionCDGCreator.h"
#include "intraFunctionFDGCreator.h"

#include "BPatch_function.h"
#include "Annotatable.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DDG;

intraFunctionPDGCreator
intraFunctionPDGCreator::create(
    Function *func) {
  intraFunctionPDGCreator creator(func);
  return creator;
}

// Handle the annotation interface
AnnotationClass <Graph::Ptr> PDGAnno(string("PDGAnno"));

Graph::Ptr intraFunctionPDGCreator::getPDG() {
  if (func == NULL) return GraphPtr();

  // Check to see if we've already analyzed this graph
  // and if so return the annotated version.
  GraphPtr *ret;
  func->getAnnotation(ret, PDGAnno);
  if (ret) return *ret;

  // Perform analysis
  analyze();
  
  // Store the annotation
  // The annotation interface takes raw pointers. Give it a
  // smart pointer pointer.
  GraphPtr *ptr = new GraphPtr(PDG);
  func->addAnnotation(ptr, PDGAnno);

  return PDG;
}

void intraFunctionPDGCreator::analyze() {
  // Get a handle to the DDG.
  intraFunctionDDGCreator ddgCreator = intraFunctionDDGCreator::create(func);
  GraphPtr ddg = ddgCreator.getDDG();
  
  // Copy DDG and put the result in PDG.
  PDG = ddg->copyGraph();
  
  // merge CDG with DDG which is currently stored in PDG.
  mergeCDG();
}

/**
 * Creates an edge from each source to each target and puts them in PDG.
 */
void createEdges(Graph::Ptr graph, set<Node::Ptr>& sources, set<Node::Ptr>& targets) {
  typedef set<Node::Ptr>::iterator NodeIter;
  for (NodeIter sourceIter = sources.begin(); sourceIter != sources.end(); sourceIter++) {
    Node::Ptr source = *sourceIter;
    for (NodeIter targetIter = targets.begin(); targetIter != targets.end(); targetIter++) {
      Node::Ptr target = *targetIter;
      graph->insertPair(source, target);
    }
  }
}

/**
 * Merges gr2 into gr1. It handles the case where more than one node represents a single
 * instruction.
 */
void merge(Graph::Ptr gr1, Graph::Ptr gr2) {
  typedef set<Node::Ptr> NodeSet;
  typedef set<Edge::Ptr> EdgeSet;
  typedef NodeSet::iterator NodeIter;
  typedef EdgeSet::iterator EdgeIter;

  NodeSet gr2Nodes;
  gr2->allNodes(gr2Nodes);
  for (NodeIter nodeIter = gr2Nodes.begin(); nodeIter != gr2Nodes.end(); nodeIter++) {
    Node::Ptr node = *nodeIter;

    // find set of nodes in first graph which have the same address as 'node'
    NodeSet sources = gr1->getNodesAtAddr(node->addr());

    // get list of targets of this node
    EdgeSet outEdges;
    node->outs(outEdges);
    for (EdgeIter iter = outEdges.begin(); iter != outEdges.end(); iter++) {
      Node::Ptr target = (*iter)->target();
      // find set of nodes in first graph which have the same address as 'target'
      NodeSet targets = gr1->getNodesAtAddr(target->addr());;

      // create edges between sources and targets
      createEdges(gr1, sources, targets);
    }
  }
}

void intraFunctionPDGCreator::mergeCDG() {
  typedef EdgeSet::iterator EdgeIter;
  intraFunctionCDGCreator cdgCreator = intraFunctionCDGCreator::create(func);
  GraphPtr cdg = cdgCreator.getCDG();

  merge(PDG, cdg);
}

/**************************************************************************************************/
/*************************  xPDG: Extended Program Dependence Graph   *****************************/
// Handle the annotation interface
AnnotationClass <Graph::Ptr> xPDGAnno(string("xPDGAnno"));

intraFunctionXPDGCreator intraFunctionXPDGCreator::create(Function *func) {
  intraFunctionXPDGCreator creator(func);
  return creator;
}

Graph::Ptr intraFunctionXPDGCreator::getXPDG() {
  if (func == NULL) return GraphPtr();

  // Check to see if we've already analyzed this graph
  // and if so return the annotated version.
  GraphPtr *ret;
  func->getAnnotation(ret, xPDGAnno);
  if (ret) return *ret;

  // Perform analysis
  analyze();
  
  // Store the annotation
  // The annotation interface takes raw pointers. Give it a
  // smart pointer pointer.
  GraphPtr *ptr = new GraphPtr(xPDG);
  func->addAnnotation(ptr, xPDGAnno);

  return xPDG;
}

void intraFunctionXPDGCreator::analyze() {
  // Get a handle to the PDG.
  intraFunctionPDGCreator pdgCreator = intraFunctionPDGCreator::create(func);
  GraphPtr pdg = pdgCreator.getPDG();

  // Copy it into xPDG.
  xPDG = pdg->copyGraph();
  
  // Merge FDG with PDG that is currently stored in xPDG.
  mergeFDG();
}

/**
 * Merges FDG with PDG that is currently stored in xPDG.
 */
void intraFunctionXPDGCreator::mergeFDG() {
  // Get a handle to the FDG.
  intraFunctionFDGCreator fdgCreator = intraFunctionFDGCreator::create(func);
  GraphPtr fdg = fdgCreator.getFDG();
  
  // Merge it with xPDG.
  merge(xPDG, fdg);
}
