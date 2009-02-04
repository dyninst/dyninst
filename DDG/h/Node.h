/*
 * Copyright (c) 1996-2004 Barton P. Miller
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


// DDG Nodes consist of an Instruction:Absloc pair. This is guaranteed to
// be a single definition on all platforms. We don't convert to an abstract
// language because we (assume) we need to keep the instruction information
// around, and this is lost in a conversion process. 

#if !defined(DDG_NODE_H)
#define DDG_NODE_H

#include <boost/shared_ptr.hpp>
#include <set>
#include "Annotatable.h"
#include "Instruction.h"

namespace Dyninst {
namespace DDG {

    class Dyninst::InstructionAPI::Instruction;
    class Edge;
    class Graph;
    class Absloc;

    class Node : public AnnotatableSparse {
        friend class Edge;
        friend class Graph;
        friend class Creator;
        
    public:
        typedef boost::shared_ptr<Node> Ptr;
        typedef boost::shared_ptr<InstructionAPI::Instruction> InsnPtr;
        typedef boost::shared_ptr<Edge> EdgePtr;
        typedef boost::shared_ptr<Absloc> AbslocPtr;
        typedef std::set<EdgePtr> EdgeSet;

        Ptr createNode(const Address addr, const InsnPtr insn, const AbslocPtr absloc);

        bool ins(EdgeSet &edges) const { return returnEdges(ins_, edges); }
        bool outs(EdgeSet &edges) const { return returnEdges(outs_, edges); }

        InsnPtr insn() const { return insn_; }
        AbslocPtr absloc() const { return absloc_; }
        Address addr() const { return addr_; }

        // We may use "virtual" nodes to represent initial definitions. These
        // have no associated instruction, which we represent as a NULL insn().
        bool isVirtual() const { return insn(); }

    private:
        Node(const Address addr, const InsnPtr insn, const AbslocPtr absloc);
        Node();

        // Instructions don't include their address, so we include this for
        // unique info. We could actually remove and recalculate the Insn
        // based on the address, but I'll add that later. 
        Address addr_;

        InsnPtr insn_;

        // We keep a "One True List" of abstract locations, so all instructions
        // that define a particular absloc will have the same pointer. 
        AbslocPtr absloc_;

        EdgeSet ins_;
        EdgeSet outs_;

        // Do the work of ins() and outs() above
        bool returnEdges(const EdgeSet &source,
                         EdgeSet &target) const;

        // For Creator-class methods to use: add a new edge to 
        // a node.
        void addInEdge(const EdgePtr in) { ins_.insert(in); }
        void addOutEdge(const EdgePtr out) { outs_.insert(out); }
    };
};
}
#endif
