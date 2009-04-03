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

#include "intraFunctionFDGCreator.h"

// std
#include <set>
#include <vector>

// DDG
#include "Absloc.h"
#include "Graph.h"
#include "Node.h"

// Dyninst
#include "BPatch_basicBlock.h"
#include "BPatch_edge.h"
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

static AnnotationClass <Graph::Ptr> FDGAnno(std::string("FDGAnno"));

intraFunctionFDGCreator intraFunctionFDGCreator::create(Function* func) {
  intraFunctionFDGCreator creator(func);
  return creator;
}

Graph::Ptr intraFunctionFDGCreator::getFDG() {
  if (func == NULL) return Graph::Ptr();

  // Check to see if we've already analyzed this graph
  // and if so return the annotated version.
  Graph::Ptr *ret;
  func->getAnnotation(ret, FDGAnno);
  if (ret) return *ret;

  // Perform analysis
  analyze();

  // Store the annotation
  // The annotation interface takes raw pointers. Give it a
  // smart pointer pointer.
  Graph::Ptr *ptr = new Graph::Ptr(FDG);
  func->addAnnotation(ptr, FDGAnno);

  return FDG;
}

void intraFunctionFDGCreator::analyze() {
  // Create a graph
  FDG = Graph::createGraph();
  
  // get a set of basic blocks
  BPatch_flowGraph* cfg = func->getCFG();
  BlockSet blocks;
  cfg->getAllBasicBlocks(blocks);

  // create an array of <instruction, address> pairs indexed by block number.
  lastInstInBlock = new InstWithAddr*[ blocks.size() ];
  
  // mark blocks that end with a jump/return/branch instruction.
  markBlocksWithJump(blocks);
  // find the dependencies between blocks.
  findDependencies(blocks);

  // finally, create instruction level nodes.
  createNodes(blocks);
}

/**
 * Finds all blocks that end with a jump/return/branch instruction and puts them in
 * markedBlocks member list. It also stores the last instruction of marked blocks.
 */
void intraFunctionFDGCreator::markBlocksWithJump(BlockSet& blocks) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    vector<Instruction> instructions;
    block->getInstructions(instructions);

    assert(instructions.size() > 0);

    Instruction& lastInst = instructions[ instructions.size() - 1 ];
    const Operation& opType = lastInst.getOperation();
    if (isReturnOp(opType) || isJumpOp(opType) || isBranchOp(opType)) {
      markedBlocks.insert(block);
      lastInstInBlock[ block->getBlockNumber() ] =
          new InstWithAddr(lastInst, (Address) (block->getEndAddress() - lastInst.size()));
    }
    else {
      lastInstInBlock[ block->getBlockNumber() ] = NULL;
    }
    
    Address address = (Address) block->getStartAddress();
    for (unsigned i = 0; i < instructions.size(); i++) {
      addrToBlock[address] = block;
      address += instructions[i].size();
    }
  }
}

void intraFunctionFDGCreator::findDependencies(BlockSet& blocks) {
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    BlockSet blocksWithJumps;
    BlockSet visited;
    findBlocksOnWay(blocksWithJumps, visited, block);
    blockToJumps[block] = blocksWithJumps;
  }
}

void intraFunctionFDGCreator::findBlocksOnWay(
    BlockSet& needThese, BlockSet& visited, Block* givenBlock) {
  if (markedBlocks.find(givenBlock) != markedBlocks.end()) {
    needThese.insert(givenBlock);
  }
  findBlocksRecursive(needThese, visited, givenBlock);
}

void intraFunctionFDGCreator::findBlocksRecursive(BlockSet& needThese, BlockSet& visited,
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

void intraFunctionFDGCreator::createDepToOtherBlocks(NodeSet& nodes, NodePtr source) {
  if (nodes.find(source) != nodes.end()) {
    return;
  }
  nodes.insert(source);
  Block* curBlock = addrToBlock[ source->addr() ];
  assert(curBlock);
  BlockSet& targets = blockToJumps[curBlock];
  for (BlockSet::iterator blIter = targets.begin(); blIter != targets.end(); blIter++) {
    InstWithAddr* targetInst = lastInstInBlock[ (*blIter)->getBlockNumber() ];
    assert (targetInst);
    if (targetInst->second == source->addr()) {
      continue;
    }
    NodePtr target = FDG->makeSimpleNode(targetInst->first, targetInst->second);
    FDG->insertPair(source, target);
    
    createDepToOtherBlocks(nodes, target);
  }
}

bool intraFunctionFDGCreator::createDepWithinBlock(NodeSet& nodes, NodePtr source) {
  if (nodes.find(source) != nodes.end()) {
    return true;
  }
  Block* curBlock = addrToBlock[ source->addr() ];
  assert(curBlock);
  InstWithAddr* inst = lastInstInBlock[ curBlock->getBlockNumber() ];
  if (inst && inst->second != source->addr()) {
    nodes.insert(source);
    NodePtr target = FDG->makeSimpleNode(inst->first, inst->second);
    FDG->insertPair(source, target);
    
    // since I am doing it for every instruction, I don't need to go any further now.    
    return true;
  }
  return false;
}

void intraFunctionFDGCreator::createNodes(BlockSet& blocks) {
  NodePtr virtNode = FDG->makeVirtualNode();
  NodeSet processedNodes;
  for (BlockSet::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++) {
    Block* block = *blockIter;
    vector<Instruction> instructions;
    block->getInstructions(instructions);
    
    Address address = (Address) block->getStartAddress();
    for (unsigned i = 0; i < instructions.size(); i++) {
      NodePtr node = FDG->makeSimpleNode(instructions[i], address);
      FDG->insertPair(virtNode, node);
      address += instructions[i].size();
      
      bool success = createDepWithinBlock(processedNodes, node);
      if (!success) {
        createDepToOtherBlocks(processedNodes, node);
      }
    }
  }
}
/**************************************************************************************************/
/****************************   static functions   ************************************************/
bool intraFunctionFDGCreator::isReturnOp(const Operation& opType) {
  entryID opId = opType.getID();
  if ((opId == e_iret) ||
      (opId == e_ret_far) ||
      (opId == e_ret_near) ||
      (opId == e_sysret) ||
      (opId == e_sysexit)) {
    return true;
  }
  return false;
}

bool intraFunctionFDGCreator::isJumpOp(const Operation& opType) {
  return (e_jmp == opType.getID());
}

bool intraFunctionFDGCreator::isBranchOp(const Operation& opType) {
  return ((e_jb <= opType.getID()) && (e_jz >= opType.getID()) && (e_jmp != opType.getID()));
}
