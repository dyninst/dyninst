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

// DDG edges are simple: in node, out node, and a type. We can extend
// this as desired, although my prediction is that we will instead be
// heavily annotating with user data. 

#if !defined(DDG_EDGE_H)
#define DDG_EDGE_H

#include "dyn_detail/boost/shared_ptr.hpp"
#include "dyn_detail/boost/weak_ptr.hpp"
#include <set>
#include "Annotatable.h"

namespace Dyninst {
namespace DepGraphAPI {

    class Graph;
    class Node;
    
    class Edge : public AnnotatableSparse {
        friend class Node;
        friend class Graph;
        friend class Creator;


        // Types of edges. This is basically dummy for now pending
        // some real ideas what edge types we want. My thought is that
        // we will eventually be able to entirely specify the behavior of
        // an iterator by constraining the edge types it will traverse. 

        typedef enum {
            Unknown,
            IntraProcedural,
            InterProcedural,
            Summary
        } type_t;

    public:
        typedef dyn_detail::boost::shared_ptr<Edge> Ptr;
        typedef std::set<Edge::Ptr> Set;
        typedef dyn_detail::boost::shared_ptr<Node> NodeSharedPtr;
        typedef dyn_detail::boost::weak_ptr<Node> NodePtr;
        
        static Ptr createEdge(const NodeSharedPtr source, const NodeSharedPtr target, const type_t type = Unknown);

        NodeSharedPtr source() const { return source_.lock(); }
        NodeSharedPtr target() const { return target_.lock(); }
        type_t type() const { return type_; } 

    private:
        Edge(const NodePtr source, const NodePtr target, const type_t type); 
        Edge();

        NodePtr source_;
        NodePtr target_;
        type_t type_;
    };
};
}
#endif

