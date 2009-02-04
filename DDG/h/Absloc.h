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

#if !defined(DDG_ABSLOC_H)
#define DDG_ABSLOC_H

#include <boost/shared_ptr.hpp>
#include <set>
#include <string>
#include "Annotatable.h"
#include "Register.h"

// This is a parent class for all abslocs (abstract locations; e.g., registers/memory/etc)
// used by the DDG. We subclass off this class to provide an Absloc with a particular meaning,
// such as a physical register, stack element, heap element, input or output representation, etc.

namespace Dyninst {
namespace DDG {

    class Graph;
    class Node;
    class Edge;

    class Absloc : public AnnotatableSparse {

        friend class Graph;
        friend class Node;
        friend class Edge;

    public:
        // Get a list of all abstract locations currently defined
        // by the graph.
        typedef boost::shared_ptr<Absloc> Ptr;
        typedef std::map<std::string, Ptr> AbslocMap;
        typedef std::set<Ptr> AbslocSet;

        bool getAbslocs(AbslocSet &locs) const;
        std::string name() const { return name_; }

        static Ptr createAbsloc(const std::string name);

        // InstructionAPI -> what we want
        // TODO: move Absloc to the InstructionAPI and use
        // it as a wrapper class. Talk to Bill about this one.
        static Absloc::Ptr getAbsloc(const InstructionAPI::RegisterAST::Ptr reg);

    private:
        Absloc(const std::string name);
        Absloc();

        static AbslocMap allAbslocs_; 
        std::string name_;
    };
};
}

#endif

