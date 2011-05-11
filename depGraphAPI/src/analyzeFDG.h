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

#ifndef CONTROLFLOWFIXER_H_
#define CONTROLFLOWFIXER_H_

#include <map>

#include "Node.h"
#include "FDG.h"

#include "Operation.h"

class BPatch_basicBlock;
class BPatch_function;

namespace Dyninst {
namespace DepGraphAPI {

/**
 * The tool that creates the Flow Dependence Graph (FDG) associated with a given 
 * function (currently BPatch_function).
 */
class FDGAnalyzer {
    friend class xPDGAnalyzer;
public:
    typedef BPatch_basicBlock Block;
    typedef std::set<Block*> BlockSet;
    typedef Node::Ptr NodePtr;
    typedef std::set<NodePtr> NodeSet;

private:
    typedef BPatch_function Function;
    typedef std::map<Block*, Node::Ptr> NodeMap;
    typedef std::map<Block*, BlockSet> DependenceMap;
  
  /**
   * Analyzed function
   */
  Function *func_;
  
  /**
   * The Flow Dependence Graph
   */
  FDG::Ptr fdg;
  
  /**
   * Maps each block A to a set of Block S which A depends on (for all S, A depends on S). 
   */
  DependenceMap blockToJumps;
  
  /**
   * Set of blocks which ends with a jump/branch/return instruction.
   */
  BlockSet markedBlocks;

public:
  FDGAnalyzer(Function *f) : func_(f) {};

  /**
   * Returns the FDG created by this intraFunctionCDGCreator object.
   */
  FDG::Ptr analyze();

  ~FDGAnalyzer();

protected:
  /**
   * Returns true iff this operation is a return or exit operation.
   */
  static bool isReturnOp(const Dyninst::InstructionAPI::Operation& opType);

  /**
   * Returns true iff this operation is a branch operation (including jumps) such as jnz, jnle, etc.
   */
  static bool isBranchOp(const Dyninst::InstructionAPI::Operation& opType);
  
private:
  void createNodes(BlockSet &blocks);
  Node::Ptr makeNode(NodeMap& nodeMap, Block* block);
  void markBlocksWithJump(BlockSet &blocks);
  void findDependencies(BlockSet &blocks);
  void findBlocksRecursive(BlockSet& needThese, BlockSet& visited, Block* givenBlock);
};

};
};

#endif /* CONTROLFLOWFIXER_H_ */
