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

#include "analyzeFDG.h"

// std
#include <set>
#include <vector>

// depGraphAPI
#include "Node.h"
#include "DepGraphNode.h" // for BlockNode

// Dyninst
#include "BPatch_basicBlock.h"
#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"

// Annotation interface
#include "Annotatable.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;

static AnnotationClass <FDG::Ptr> FDGAnno(std::string("FDGAnno"));

FDGAnalyzer::~FDGAnalyzer() {}

FDG::Ptr FDGAnalyzer::analyze() {
    if (fdg) return fdg;

    if (func_ == NULL) return FDG::Ptr();

    FDG::Ptr *ret;
    func_->getAnnotation(ret, FDGAnno);
    if (ret) {
        fdg = *ret;
        return fdg;
    }

    // What we really want to hand into this is a CFG... for
    // now, the set of blocks and entry block will suffice.
    BlockSet blocks;
    func_->getCFG()->getAllBasicBlocks(blocks);

    // Create a graph
    fdg = FDG::createGraph();
    
    // mark blocks that end with a jump/return/branch instruction.
    markBlocksWithJump(blocks);
    // find the dependencies between blocks.
    findDependencies(blocks);
    
    // finally, create block level nodes.
    createNodes(blocks);
    
    // Store as an annotation and return
    FDG::Ptr *ptr = new FDG::Ptr(fdg);
    func_->addAnnotation(ptr, FDGAnno);

    return fdg;
}

/**
 * Finds all blocks that end with a jump/return/branch instruction and puts them in
 * markedBlocks member list. It also stores the last instruction of marked blocks.
 */
void FDGAnalyzer::markBlocksWithJump(BlockSet &blocks) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    vector<Instruction> instructions;
    block->getInstructions(instructions);

    assert(instructions.size() > 0);

    Instruction& lastInst = instructions[ instructions.size() - 1 ];
    const Operation& opType = lastInst.getOperation();
    
    if (isReturnOp(opType) || isBranchOp(opType)) {
      markedBlocks.insert(block);
    }
  }
}

void FDGAnalyzer::findDependencies(BlockSet &blocks) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    BlockSet blocksWithJumps;
    BlockSet visited;
    findBlocksRecursive(blocksWithJumps, visited, block);
    blockToJumps[block] = blocksWithJumps;
  }
}

void FDGAnalyzer::findBlocksRecursive(BlockSet& needThese, 
                                      BlockSet& visited,
                                      Block* givenBlock) {
  if ((visited.find(givenBlock) != visited.end())) {
    return;
  }
  visited.insert(givenBlock);
  
  vector<BPatch_edge*> outEdges;
  givenBlock->getOutgoingEdges(outEdges);
  for (unsigned i = 0; i < outEdges.size(); i++) {
    Block* target = outEdges[i]->getTarget();
    // if the target terminates anything other than a call, it is added to the list, and we don't
    // search on this branch anymore. If not, keep looking!
    if (markedBlocks.find(target) != markedBlocks.end()) {
      needThese.insert(target);
    }
    else {
      findBlocksRecursive(needThese, visited, target);
    }
  }
}

Node::Ptr FDGAnalyzer::makeNode(NodeMap& nodeMap, Block* block) {
    if (nodeMap[block]) {
        return nodeMap[block];
    }
    Node::Ptr blockNode = BlockNode::createNode(block);
    nodeMap[block] = blockNode;
    return blockNode;
}

void FDGAnalyzer::createNodes(BlockSet &blocks) {
    NodePtr virtNode = fdg->virtualEntryNode();
    NodeMap nodeMap;
    for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
        Block* block = *blockIter;
        
        Node::Ptr blockNode = makeNode(nodeMap, block);
        fdg->addNode(blockNode);
        fdg->insertPair(virtNode, blockNode);
        
        BlockSet& targets = blockToJumps[block];
        for (BlockSet::iterator blIter = targets.begin(); blIter != targets.end(); blIter++) {
            Node::Ptr source = makeNode(nodeMap, *blIter);
            fdg->addNode(source);
            fdg->insertPair(source, blockNode);
        }
    }
}
/**************************************************************************************************/
/****************************   static functions   ************************************************/
bool FDGAnalyzer::isReturnOp(const Operation& opType) {
    return (entryToCategory(opType.getID()) == c_ReturnInsn);
}

bool FDGAnalyzer::isBranchOp(const Operation& opType) {
    return (entryToCategory(opType.getID()) == c_BranchInsn);
}
