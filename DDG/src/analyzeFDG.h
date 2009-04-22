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

#ifndef CONTROLFLOWFIXER_H_
#define CONTROLFLOWFIXER_H_

#include <map>
#include "Graph.h"
#include "Node.h"

using namespace std;

class BPatch_basicBlock;
class BPatch_function;

namespace Dyninst {
namespace DepGraphAPI {

using namespace InstructionAPI;

/**
 * The tool that creates the Flow Dependence Graph (FDG) associated with a given 
 * function (currently BPatch_function).
 */
class FDGAnalyzer {
 public:
  typedef BPatch_basicBlock Block;
  typedef BPatch_function Function;
  typedef set<Block*> BlockSet;
  typedef Node::Ptr NodePtr;
  typedef set<NodePtr> NodeSet;
  typedef pair<Instruction, Address> InstWithAddr;
  typedef vector<InstWithAddr*> InstWithAddrList;

private:
  
  /**
   * Analyzed function
   */
  Function *func_;
  
  /**
   * The Flow Dependence Graph
   */
  Graph::Ptr fdg;
  
  /**
   * Maps each block A to a set of Block S which A depends on (for all S, A depends on S). 
   */
  map<Block*, BlockSet> blockToJumps;
  
  /**
   * Set of blocks which ends with a jump/branch/return instruction.
   */
  BlockSet markedBlocks;
  
  /**
   * Maps each instruction in this function to the basic block that encapsulates it. 
   */
  map<Address, Block*> addrToBlock;
  
  /**
   * This is an array of InstWithAddress pointers. Its memory should be claimed.
   */
  InstWithAddrList lastInstInBlock;

public:
  FDGAnalyzer(Function *f) : func_(f) {};

  /**
   * Returns the FDG created by this intraFunctionCDGCreator object.
   */
  Graph::Ptr analyze();

  ~FDGAnalyzer();

 private:

  /**
   * Returns true iff this operation is a return or exit operation.
   */
  static bool isReturnOp(const Operation& opType);

  /**
   * Returns true iff this operation is a jump operation.
   */
  static bool isJumpOp(const Operation& opType);

  /**
   * Returns true iff this operation is a branch operation such as jnz, jnle, etc.
   */
  static bool isBranchOp(const Operation& opType);
  
  /**
   * Destructor.
   */
private:
  
  void createNodes(BlockSet &blocks);
  void markBlocksWithJump(BlockSet &blocks);
  void findDependencies(BlockSet &blocks);
  void findBlocksOnWay(BlockSet& needThese, BlockSet& visited, Block* givenBlock);
  void findBlocksRecursive(BlockSet& needThese, BlockSet& visited, Block* givenBlock);
  
  void createDepToOtherBlocks(NodeSet& nodes, NodePtr source);
  bool createDepWithinBlock(NodeSet& nodes, NodePtr source);
};

};
};

#endif /* CONTROLFLOWFIXER_H_ */
