/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <set>
#include <vector>

#include "analyzeCDG.h"
#include "CDG.h"
#include "DepGraphNode.h" // for BlockNode

// Dyninst
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"

// Annotation interface
#include "Annotatable.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;

// Annotation type for storing CDGs.
AnnotationClass <CDG::Ptr> CDGAnno(std::string("CDGAnno"));

// Constructor. Copies given function into internal structure.
CDGAnalyzer::CDGAnalyzer(Function *f) : func_(f) {};

// Creates and returns a CDG for the given function.
CDG::Ptr CDGAnalyzer::analyze() {
	// No function, no CDG!
    if (func_ == NULL) return CDG::Ptr();

	// Check the annotations. Did we compute the graph before?
    CDG::Ptr *ret;
    func_->getAnnotation(ret, CDGAnno);
    if (ret) {
    	// yes, we did. Return it.
        cdg = *ret;
        return cdg;
    }

    // What we really want to hand into this is a CFG... for
    // now, the set of blocks and entry block will suffice.
    BlockSet blocks;
    func_->getCFG()->getAllBasicBlocks(blocks);

    // Create a graph
    cdg = CDG::createGraph();
    
    // create the dependencies between blocks
    createDependencies(blocks);

    // Store as an annotation and return
    CDG::Ptr *ptr = new CDG::Ptr(cdg);
    func_->addAnnotation(ptr, CDGAnno);
    
    return cdg;
}

/**
 * Creates control flow dependencies between blocks. For more info, see Ferrante et. al.'s
 * "The program dependence graph and its use in optimization".
 */
void CDGAnalyzer::createDependencies(BlockSet &blocks) {
    NodePtr entryNode = cdg->virtualEntryNode();
    for (BlockSet::iterator blockIter = blocks.begin(); 
         blockIter != blocks.end();
         blockIter++) {
        Block* block = *blockIter;
        
        // create the node for this basic block
        NodePtr source = makeNode(block);
        // add an edge from the entry node to this one.
        // note that even if the node was created earlier, this is the first time
        // we are adding an edge from the entry node.
        cdg->insertPair(entryNode, source);
        
        // Find nodes that are dependent on this node.
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
                NodePtr target = makeNode(temp);
                cdg->insertPair(source, target);
            }
        }
    }
}

// Create and return a BlockNode.
Node::Ptr CDGAnalyzer::makeNode(Block *b) {
	// Create the BlockNode only if we haven't done so before.
    if (nodeMap.find(b) == nodeMap.end()) {
        Node::Ptr newNode = BlockNode::createNode(b);
        nodeMap[b] = newNode;
    }
    return nodeMap[b];
}
