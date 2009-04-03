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

#ifndef INTRACDGCREATOR_H_
#define INTRACDGCREATOR_H_

using namespace Dyninst;
using namespace Dyninst::DDG;
using namespace Dyninst::InstructionAPI;

class BPatch_basicBlock;
class BPatch_flowGraph;
class BPatch_function;

/**
 * The tool that creates the Control Dependence Graph (CDG) associated with a given 
 * function (currently BPatch_function). It uses Dominator Analysis provided by
 * Dyninst. The algorithm is borrowed from Ferrante et. al.
 */
class intraFunctionCDGCreator {
private:
  typedef BPatch_basicBlock Block;
  typedef BPatch_function Function;

  typedef set<Block*> BlockSet;
  typedef Node::Ptr NodePtr;
  typedef vector<NodePtr> NodeList;
  typedef vector<BlockSet> BlockSetList;
  
  /**
   * The function whose CDG is being created.
   */
  Function *func;
  
  /**
   * Control Dependence Graph.
   */
  Graph::Ptr CDG;
  
public:
  /**
   * Creates an intraFunctionCDGCreator object associated with the given function.
   */
  static intraFunctionCDGCreator create(Function *func);
  
  /**
   * Returns the CDG created by this intraFunctionCDGCreator object.
   */
  Graph::Ptr getCDG();

private:
  intraFunctionCDGCreator(Function *f) : func(f) {};
  
  void analyze();
  void createInterBlockDeps(BlockSet& blocks, BlockSetList& dependencies);
  void createNodeDeps(BlockSet& blocks, BlockSetList& dependencies);
  void createNodes(BlockSet& blocks, NodeList& lastNodeInBlock);
  void createDependencies(BlockSet& blocks, BlockSetList& dependencies, NodeList& lastNodeInBlock);
};

#endif /* INTRACDGCREATOR_H_ */
