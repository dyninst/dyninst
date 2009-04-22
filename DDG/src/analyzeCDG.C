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

#include <set>
#include <vector>

#include "analyzeCDG.h"

#include "Absloc.h"
#include "Graph.h"

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
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;

AnnotationClass <Graph::Ptr> CDGAnno(std::string("CDGAnno"));

CDGAnalyzer::CDGAnalyzer(Function *f) : func_(f) {};

Graph::Ptr CDGAnalyzer::analyze() {
    if (func_ == NULL) return Graph::Ptr();

    Graph::Ptr *ret;
    func_->getAnnotation(ret, CDGAnno);
    if (ret) {
        cdg = *ret;
        return cdg;
    }

    // What we really want to hand into this is a CFG... for
    // now, the set of blocks and entry block will suffice.
    CDGAnalyzer::BlockSet blocks;
    func_->getCFG()->getAllBasicBlocks(blocks);

    // Create a graph
    cdg = Graph::createGraph();
    
    // create the dependencies between blocks
    createInterBlockDeps(blocks);
    
    // create instruction-level nodes and insert them into the graph
    createNodeDeps(blocks);

    // Store as an annotation and return
    Graph::Ptr *ptr = new Graph::Ptr(cdg);
    func_->addAnnotation(ptr, CDGAnno);
    
    return cdg;
}

/**
 * Creates control flow dependencies between blocks. For more info, see Ferrante et. al.'s
 * "The program dependence graph and its use in optimization".
 */
void CDGAnalyzer::createInterBlockDeps(BlockSet &blocks) {
    for (BlockSet::iterator blockIter = blocks.begin(); 
         blockIter != blocks.end(); blockIter++) {
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
      for (Block* temp = out[i]; 
           (temp != NULL) && (temp != block) && (temp != parent);
           temp = temp->getImmediatePostDominator()) {
        // mark them as control dependent to 'block'
        dependencies[ temp ].insert(block);
      }
    }
  }
}

void CDGAnalyzer::createNodeDeps(BlockSet &) {
    //createNodes();
    createDependencies();
}

/**
 * Creates individual nodes for each instruction and insert an edge from a virtual node to all
 * the nodes to avoid garbage collection by boost.
 */
void CDGAnalyzer::createNodes(BlockSet &blocks) {

    for (BlockSet::iterator blockIter = blocks.begin(); 
         blockIter != blocks.end(); blockIter++) {

        NodePtr node = BlockNode::createNode(*blockIter);
        nodeMap[*blockIter] = node;
    }
}

/**
 * Creates dependencies at the instruction level using the dependencies at the Block level.
 */
void CDGAnalyzer::createDependencies() {
    for (BlockMap::iterator iter = dependencies.begin(); 
         iter != dependencies.end(); iter++) {
        // Everything in iter->second depends on iter->first...
        Node::Ptr source = makeNode(iter->first);

        for (BlockSet::iterator blockIter = iter->second.begin();
             blockIter != iter->second.end();
             blockIter++) {
            Node::Ptr target = makeNode(*blockIter);

            cdg->insertPair(source, target);
        }
    }
}

Node::Ptr CDGAnalyzer::makeNode(Block *b) {
    if (nodeMap.find(b) == nodeMap.end()) {
        Node::Ptr newNode = BlockNode::createNode(b);
        nodeMap[b] = newNode;
    }
    
    return nodeMap[b];
}
