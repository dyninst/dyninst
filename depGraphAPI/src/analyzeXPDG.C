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

#include "analyzeXPDG.h"

#include <map>

#include "Graph.h"
#include "FDG.h"
#include "PDG.h"
#include "analyzeFDG.h"
#include "analyzePDG.h"
#include "DepGraphNode.h" // for BlockNode

#include "BPatch_function.h"
#include "Annotatable.h"

#include "dynptr.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DepGraphAPI;

// Annotation type for storing xPDGs.
AnnotationClass<xPDG::Ptr> xPDGAnno(std::string("xPDGAnno"));

// Constructor. Copies given function into internal structure.
xPDGAnalyzer::xPDGAnalyzer(Function *f) : func_(f) {}

// Creates and returns an xPDG for the given function.
xPDG::Ptr xPDGAnalyzer::analyze() {
    // No function, no xPDG!
    if (func_ == NULL) return xPDG::Ptr();

    // Check the annotations. Did we compute the graph before?
    xPDG::Ptr *ret;
    func_->getAnnotation(ret, xPDGAnno);
    if (ret) {
        // yes, we did. Return it.
        xpdg = *ret;
        return xpdg;
    }

    // Create a graph
    xpdg = xPDG::createGraph();

    // merge with the PDG of this function.
    mergeWithPDG();
    // and with the FDG of this function.
    mergeWithFDG();

    // Store as an annotation and return
    xPDG::Ptr *ptr = new xPDG::Ptr(xpdg);
    func_->addAnnotation(ptr, xPDGAnno);

    return xpdg;
}

void xPDGAnalyzer::mergeWithPDG() {
    // find the entry node first
    NodePtr virtEntryNode = xpdg->virtualEntryNode();

    // node map so that we can find the nodes created in the first pass during the second pass.
    NodeMap nodeMap;

    // Create the PDG and get the nodes.
    PDG::Ptr pdg = PDG::analyze(func_);
    NodeIterator pdgBegin, pdgEnd;
    pdg->allNodes(pdgBegin, pdgEnd);

    // Pass 1: create all PDG nodes in xPDG
    for (NodeIterator pdgIter = pdgBegin; pdgIter != pdgEnd; pdgIter++) {
        Node::Ptr node = *pdgIter;
        // skip the virtual node
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(node);
        if (virtNode) {
            continue;
        }

        // copy the node and add to the graph
        NodePtr nodeCopy = node->copy();
        nodeMap[node.get()] = nodeCopy;
        xpdg->addNode(nodeCopy);
        xpdg->insertPair(virtEntryNode, nodeCopy);
    }

    // Pass 2: create the edges!
    pdg->allNodes(pdgBegin, pdgEnd);
    for (NodeIterator pdgIter = pdgBegin; pdgIter != pdgEnd; pdgIter++) {
        Node::Ptr node = *pdgIter;
        // skip the virtual node
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(node);
        if (virtNode) {
            continue;
        }
        // get the copy of the pdg node
        NodePtr sourceCopy = nodeMap[node.get()];
        assert(sourceCopy);

        // get list of targets of this node
        NodeIterator outsBegin, outsEnd;
        node->outs(outsBegin, outsEnd);
        for (NodeIterator outsIter = outsBegin; outsIter != outsEnd; outsIter++) {
            // get the copy of the pdg node
            Node::Ptr target = *outsIter;
            NodePtr targetCopy = nodeMap[target.get()];
            assert(target.get());
            assert(targetCopy);

            // now add an edge between these two copy nodes
            xpdg->insertPair(sourceCopy, targetCopy);
        }
    }
}

// Merge current xPDG with the FDG of this function.
void xPDGAnalyzer::mergeWithFDG() {
    // Create FDG and get the nodes. These nodes are basic-block-level nodes.
    FDG::Ptr fdg = FDG::analyze(func_);
    NodeIterator fdgBegin, fdgEnd;
    fdg->allNodes(fdgBegin, fdgEnd);
    // Iterate over nodes in FDG (Basic Blocks).
    // For each block, find the outgoing edges of this block. If there is an outgoing edge,
    // the last instruction of this block *must* be a jump/branch/return instruction (writes
    // to PC). Create edges from this branch instruction to:
    // 1) *all* instructions in this block.
    // 2a) the jump/branch instruction (last instruction) of blocks that are in the outgoing
    //     blocks list of the source block.
    // 2b) to *all* instructions in a block that is in the outgoing blocks list of the source
    //     block, provided that the last instruction is not   jump/return instruction.
    for (NodeIterator fdgIter = fdgBegin; fdgIter != fdgEnd; fdgIter++) {
        // skip the virtual node
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(*fdgIter);
        if (virtNode) {
            continue;
        }
        // This node has to be a block node if not virtual
        BlockNode::Ptr node = dynamic_pointer_cast<BlockNode>(*fdgIter);
        assert(node);

        // Get the blocks that are in the out list.
        NodeIterator depBegin, depEnd;
        node->outs(depBegin, depEnd);
        // if no dependencies, then skip this node!
        if (depBegin == depEnd) {
            continue;
        }

        Block* block = node->block();

        // Find the last instruction.
        vector<Instruction::Ptr> insns;
        block->getInstructions(insns);
        Instruction::Ptr lastInst = insns.back();
        Address lastInstAddr = (Address) (block->getEndAddress() - lastInst->size());
        const Operation& opType = lastInst->getOperation();
        NodePtr source;
        // Process the node if the last instruction is a jump/branch/return instruction.
        if (FDGAnalyzer::isReturnOp(opType) || FDGAnalyzer::isBranchOp(opType)) {
            // Find the node that is related to the EIP register.
            NodeIterator sBegin, sEnd;
            xpdg->Graph::find(lastInstAddr, sBegin, sEnd);
            for (NodeIterator sIter = sBegin; sIter != sEnd; sIter++) {
                OperationNode::Ptr opNode = dynamic_pointer_cast<OperationNode>(*sIter);
                if (opNode) {
                    RegisterLoc::Ptr regLoc = dynamic_pointer_cast<RegisterLoc>(opNode->absloc());
                    if (regLoc) {
                        RegisterAST::Ptr regAst = regLoc->getReg();
                        if (regAst->getID() == r_EIP) {
                            source = *sIter;
                        }
                    }
                }
            }
            // The source has to be set. If not, there is a problem!
            assert(source);

            // add intra-block dependencies first!
            // add an edge from source to all nodes in this block.
            Address targetAddr = (Address) block->getStartAddress();
            for (unsigned i = 0; i < (insns.size() - 1); i++) {
                NodeIterator tBegin, tEnd;
                xpdg->Graph::find(targetAddr, tBegin, tEnd);
                PDGAnalyzer::createEdges(xpdg, source, tBegin, tEnd);
                targetAddr += insns[i]->size();
            }

            // now add inter-block dependencies!
            // for each target block:
            for (NodeIterator nodeIter = depBegin; nodeIter != depEnd; nodeIter++) {
                BlockNode::Ptr targetNode = dynamic_pointer_cast<BlockNode>(*nodeIter);
                assert(targetNode);

                Block* targetBlock = targetNode->block();
                vector<Instruction::Ptr> targetIns;
                targetBlock->getInstructions(targetIns);

                // check if the last instruction is jump/branch/return (It can't be return, can it?)
                Instruction::Ptr lastTargetInst = targetIns.back();
                const Operation& targetOpType = lastTargetInst->getOperation();
                if (FDGAnalyzer::isReturnOp(targetOpType) || FDGAnalyzer::isBranchOp(targetOpType)) {
                    // add an edge only to the last instruction of this block.
                    Address lastTargetAddr =
                        (Address) (targetBlock->getEndAddress() - lastTargetInst->size());
                    NodeIterator tBegin, tEnd;
                    xpdg->Graph::find(lastTargetAddr, tBegin, tEnd);
                    PDGAnalyzer::createEdges(xpdg, source, tBegin, tEnd);
                }
                // otherwise
                else {
                    // add an edge to all instructions in this block.
                    Address targetAddr = (Address) targetBlock->getStartAddress();
                    for (unsigned j = 0; j < targetIns.size(); j++) {
                        NodeIterator tBegin, tEnd;
                        xpdg->Graph::find(targetAddr, tBegin, tEnd);
                        PDGAnalyzer::createEdges(xpdg, source, tBegin, tEnd);
                        targetAddr += targetIns[j]->size();
                    }
                }
            }
        }
        else {
            // there can be out edges only if the last instruction is a return, jump or branch.
            NodeIterator depBegin, depEnd;
            node->outs(depBegin, depEnd);
            assert(depBegin == depEnd);
        }
    }
}
