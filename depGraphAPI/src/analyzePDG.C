/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "analyzePDG.h"

#include <set>
#include <map>

#include "dynptr.h"

#include "Graph.h"
#include "analyzeDDG.h"
#include "analyzeCDG.h"

#include "BPatch_function.h"
#include "Annotatable.h"

#include "instructionAPI/h/Register.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DepGraphAPI;

// Annotation type for storing PDGs.
AnnotationClass<PDG::Ptr> PDGAnno(std::string("PDGAnno"));

// Constructor. Copies given function into internal structure.
PDGAnalyzer::PDGAnalyzer(Function *f) : func_(f) {}

// Creates a PDG for the given function.
PDG::Ptr PDGAnalyzer::analyze() {
    // No function, no PDG!
    if (func_ == NULL) return PDG::Ptr();

    // Check the annotations. Did we compute the graph before?
    PDG::Ptr *ret;
    func_->getAnnotation(ret, PDGAnno);
    if (ret) {
        // yes, we did. Return it.
        pdg = *ret;
        return pdg;
    }

    // What we really want to hand into this is a CFG... for
    // now, the set of blocks will suffice.
    BlockSet blocks;
    func_->getCFG()->getAllBasicBlocks(blocks);

    // Create an empty graph
    pdg = PDG::createGraph();

    // merge with the DDG of this function.
    mergeWithDDG();
    // and with the CDG of this function.
    mergeWithCDG();

    // Store as an annotation and return
    PDG::Ptr *ptr = new PDG::Ptr(pdg);
    func_->addAnnotation(ptr, PDGAnno);

    return pdg;
}

void PDGAnalyzer::mergeWithDDG() {
    // find the entry node first
    NodePtr virtEntryNode = pdg->virtualEntryNode();

    // node map so that we can find the nodes created in the first pass during the second pass.
    NodeMap nodeMap;

    // get the DDG and its nodes.
    Graph::Ptr ddg = DDG::analyze(func_);
    NodeIterator ddgBegin, ddgEnd;
    ddg->allNodes(ddgBegin, ddgEnd);

    // Pass 1: create all DDG nodes in PDG
    for (NodeIterator ddgIter = ddgBegin; ddgIter != ddgEnd; ddgIter++) {
        Node::Ptr node = *ddgIter;
        // skip the virtual node
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(node);
        if (virtNode) {
            continue;
        }

        // copy the node and add to the graph
        NodePtr nodeCopy = node->copy();
        nodeMap[node.get()] = nodeCopy;
        pdg->addNode(nodeCopy);
        pdg->insertPair(virtEntryNode, nodeCopy);
    }

    // Pass 2: create the edges!
    for (NodeIterator ddgIter = ddgBegin; ddgIter != ddgEnd; ddgIter++) {
        Node::Ptr node = *ddgIter;
        // skip the virtual node
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(node);
        if (virtNode) {
            continue;
        }
        // get the copy of the ddg node
        NodePtr sourceCopy = nodeMap[node.get()];
        assert(sourceCopy);

        // get list of targets of this node
        NodeIterator outsBegin, outsEnd;
        node->outs(outsBegin, outsEnd);
        for (NodeIterator outsIter = outsBegin; outsIter != outsEnd; outsIter++) {
            // get the copy of the ddg node
            Node::Ptr target = *outsIter;
            NodePtr targetCopy = nodeMap[target.get()];
            assert(targetCopy);

            // now add an edge between these two copy nodes
            pdg->insertPair(sourceCopy, targetCopy);
        }
    }
}

/**
 * Helper Method:
 * Creates an edge from source to each target and puts them in PDG.
 */
void PDGAnalyzer::createEdges(Graph::Ptr graph, Node::Ptr source,
        NodeIterator targetsBegin, NodeIterator targetsEnd) {
    for (NodeIterator targetIter = targetsBegin; targetIter != targetsEnd; targetIter++) {
        Node::Ptr target = *targetIter;
        graph->insertPair(source, target);
    }
}

void PDGAnalyzer::mergeWithCDG() {
    typedef vector<Instruction::Ptr>::iterator InsIter;

    // get the CDG and all of its nodes.
    Graph::Ptr cdg = CDG::analyze(func_);
    NodeIterator cdgBegin, cdgEnd;
    cdg->allNodes(cdgBegin, cdgEnd);
    // Iterate over nodes in CDG (Basic Blocks).
    // For each block, find the outgoing edges of this block. If there is an outgoing edge,
    // the last instruction of this block *must* be a branch instruction with more than one
    // targets. Create edges from this branch instruction to *all* instructions in all
    // blocks that are in the outgoing blocks list.
    for (NodeIterator cdgIter = cdgBegin; cdgIter != cdgEnd; cdgIter++) {
        // skip the virtual node in CDG.
        VirtualNode::Ptr virtNode = dynamic_pointer_cast<VirtualNode>(*cdgIter);
        if (virtNode) {
            continue;
        }
        // This node has to be a block node if not virtual
        BlockNode::Ptr node = dynamic_pointer_cast<BlockNode>(*cdgIter);
        assert(node);

        // skip if there are no dependencies!
        NodeIterator targetsBegin, targetsEnd;
        node->outs(targetsBegin, targetsEnd);
        if (targetsBegin == targetsEnd) {
            continue;
        }

        Block* block = node->block();

        // find last instruction in block
        vector<Instruction::Ptr> insns;
        block->getInstructions(insns);
        Address lastInstAddr = (Address) (block->getEndAddress() - insns.back()->size());

        // Find the node that is related to the EIP register.
        NodeIterator sourcesBegin, sourcesEnd;
        pdg->Graph::find(lastInstAddr, sourcesBegin, sourcesEnd);
        NodePtr source;
        assert(sourcesBegin != sourcesEnd);
        for (NodeIterator sIter = sourcesBegin; sIter != sourcesEnd; sIter++) {
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

        // Now create an edge from the source to *all* instructions in all
        // blocks that are in the outgoing blocks list.
        for (NodeIterator targetIter = targetsBegin; targetIter != targetsEnd; targetIter++) {
            BlockNode::Ptr targetNode = dynamic_pointer_cast<BlockNode>(*targetIter);
            assert(targetNode);

            Block* targetBlock = targetNode->block();
            // for each instruction in block
            vector<Instruction::Ptr> targetInsns;
            targetBlock->getInstructions(targetInsns);
            Address targetAddr = (Address) targetBlock->getStartAddress();
            for (InsIter insIter = targetInsns.begin(); insIter != targetInsns.end(); insIter++) {
                // find all versions -> one set of targets
                NodeIterator targetsBegin, targetsEnd;
                pdg->Graph::find(targetAddr, targetsBegin, targetsEnd);

                // create edges from source to targets
                createEdges(pdg, source, targetsBegin, targetsEnd);

                // increment the address by the size of this instruction
                targetAddr += (*insIter)->size();
            }
        }
    }
}
