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

#include <set>
#include <vector>

#include "Absloc.h"
#include "Graph.h"
#include "intraFunctionCDGCreator.h"

// Dyninst
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"

// InstructionAPI
#include "Instruction.h"

// Annotation interface
#include "Annotatable.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DDG;
using namespace Dyninst::InstructionAPI;

// Handle the annotation interface
static AnnotationClass <Graph::Ptr> CDGAnno(std::string("CDGAnno"));

intraFunctionCDGCreator intraFunctionCDGCreator::create(Function* func) {
  intraFunctionCDGCreator creator(func);
  return creator;
}

Graph::Ptr intraFunctionCDGCreator::getCDG() {
    if (func == NULL) return Graph::Ptr();

    // Check to see if we've already analyzed this graph
    // and if so return the annotated version.
    Graph::Ptr *ret;
    func->getAnnotation(ret, CDGAnno);
    if (ret) return *ret;

    // Perform analysis
    analyze();

    // Store the annotation
    // The annotation interface takes raw pointers. Give it a
    // smart pointer pointer.
    Graph::Ptr *ptr = new Graph::Ptr(CDG);
    func->addAnnotation(ptr, CDGAnno);

    return CDG;
}

void intraFunctionCDGCreator::analyze() {
  // Create a graph
  CDG = Graph::createGraph();
  
  // get a set of basic blocks
  BPatch_flowGraph* cfg = func->getCFG();
  BlockSet allBlocks;
  cfg->getAllBasicBlocks(allBlocks);

  // initialize an array
  BlockSet dependencies[allBlocks.size()];
  
  // create the dependencies between blocks
  createInterBlockDeps(allBlocks, dependencies);
  
  // create instruction-level nodes and insert them into the graph
  createNodeDeps(allBlocks, dependencies);
}

/**
 * Creates control flow dependencies between blocks. For more info, see Ferrante et. al.'s
 * "The program dependence graph and its use in optimization".
 */
void intraFunctionCDGCreator::createInterBlockDeps(BlockSet& blocks, BlockSet dependencies[]) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    vector<Block*> out;
    block->getTargets(out);
    for (unsigned i = 0; i < out.size(); i++) {
      if (out[i]->postdominates(block)) {
        continue;
      }

      // Work on this edge right now: mark all parents of 'block'
      // According to Ferrante et al. page 325, marking only this node and its parent is enough
      Block* parent = block->getImmediatePostDominator();

      // traverse from out[i] to one of the parents marked in the previous step
      for (Block* temp = out[i]; (temp != NULL) && (temp != block) && (temp != parent);
          temp = temp->getImmediatePostDominator()) {
        // mark them as control dependent to 'block'
        dependencies[ temp->getBlockNumber() ].insert(block);
      }
    }
  }
}

void intraFunctionCDGCreator::createNodeDeps(BlockSet& blocks, BlockSet dependencies[]) {
  NodePtr lastNodeInBlock[blocks.size()];
  createNodes(blocks, lastNodeInBlock);
  createDependencies(blocks, dependencies, lastNodeInBlock);
}

/**
 * Creates individual nodes for each instruction and insert an edge from a virtual node to all
 * the nodes to avoid garbage collection by boost.
 */
void intraFunctionCDGCreator::createNodes(BlockSet& blocks, NodePtr lastNodeInBlock[]) {
  NodePtr virtNode = CDG->makeVirtualNode();
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    vector<Instruction> instructions;
    (*blockIter)->getInstructions(instructions);

    NodePtr node;
    Address address = (Address) (*blockIter)->getStartAddress();
    for (unsigned i = 0; i < instructions.size(); i++) {
      node = CDG->makeSimpleNode(instructions[i], address);
      CDG->insertPair(virtNode, node);
      address += instructions[i].size();
    }

    // store the last node for later use
    lastNodeInBlock[ (*blockIter)->getBlockNumber() ] = node;
  }
}

/**
 * Creates dependencies at the instruction level using the dependencies at the Block level.
 */
void intraFunctionCDGCreator::createDependencies(BlockSet& blocks, BlockSet dependencies[],
    NodePtr lastNodeInBlock[]) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    int blockNumber = (*blockIter)->getBlockNumber();
    BlockSet& depList = dependencies[ blockNumber ];

    vector<Instruction> instructions;
    (*blockIter)->getInstructions(instructions);

    Address address = (Address) (*blockIter)->getStartAddress();
    for (unsigned i = 0; i < instructions.size(); i++) {
      NodePtr thisNode = CDG->makeSimpleNode(instructions[i], address);
      address += instructions[i].size();

      for (BlockSet::iterator depIter = depList.begin(); depIter != depList.end(); depIter++) {
        CDG->insertPair(thisNode, lastNodeInBlock[ (*depIter)->getBlockNumber() ]);
      }
    }
  }
}
