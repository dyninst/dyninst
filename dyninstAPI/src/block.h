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

#if !defined(_DYN_BLOCK_H_)
#define _DYN_BLOCK_H_

#include <set>
#include <string>
#include <vector>
#include "parse-cfg.h"
#include "parseAPI/h/CFG.h"
#include "instPoint.h"
#include "PatchCFG.h"
#include "mapped_object.h"

class block_instance;
class func_instance;
class parse_func;
class BPatch_edge;
class mapped_object;


class edge_instance : public Dyninst::PatchAPI::PatchEdge {
  friend class block_instance;
  friend class func_instance;
  friend class mapped_object;

  public:
    block_instance *src() const;
    block_instance *trg() const;
    AddressSpace *proc();
    edge_instance(ParseAPI::Edge *edge, block_instance *src, block_instance *trg);
    edge_instance(const edge_instance *parent, mapped_object *child);
    ~edge_instance();
};

// This is somewhat mangled, but allows Dyninst to access the
// iteration predicates of Dyninst without having to go back and
// template that code. Just wrap a ParseAPI predicate in a
// EdgePredicateAdapter and *poof* you're using edge_instances
// instead of ParseAPI edges...

class EdgePredicateAdapter 
   : public ParseAPI::iterator_predicate <
  edge_instance *,
  edge_instance * > {
  public:
  EdgePredicateAdapter() : int_(NULL) {}
  EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {}
   virtual ~EdgePredicateAdapter() {}
   virtual bool pred_impl(edge_instance * const e) const { return int_->pred_impl(e->edge()); }

  private:
   ParseAPI::EdgePredicate *int_;
};

class block_instance : public Dyninst::PatchAPI::PatchBlock {
  friend class mapped_object;

  public:
  //typedef std::vector<edge_instance *> edges;
  //typedef std::vector<edge_instance *> edgelist;

    block_instance(ParseAPI::Block *ib, mapped_object *obj);
    block_instance(const block_instance *parent, mapped_object *child);
    ~block_instance();

    // Up-accessors
    mapped_object *obj() const { return SCAST_MO(obj_); }
    AddressSpace *addrSpace() const;
    AddressSpace *proc() const { return addrSpace(); }

    template<class OutputIterator> 
       void getFuncs(OutputIterator result);

    void triggerModified();
    void setNotAbruptEnd();
    parse_block * llb() const { return SCAST_PB(block_); }
    void *getPtrToInstruction(Address addr) const;

    //const edgelist &sources();
    //const edgelist &targets();

    // Shortcuts
    edge_instance *getTarget();
    edge_instance *getFallthrough();
    // NULL if not conclusive
    block_instance *getFallthroughBlock();

    func_instance *callee();
    std::string calleeName();
    bool _ignorePowerPreamble;
    int id() const;

    // Functions to avoid
    // These are convinence wrappers for really expensive
    // lookups, and thus should be avoided. 
    func_instance *entryOfFunc() const;
    bool isFuncExit() const;
    // static void destroy(block_instance *b); // doesn't need to do anything
    Address GetBlockStartingAddress();
    virtual void markModified();

 private:
    void updateCallTarget(func_instance *func);
    func_instance *findFunction(ParseAPI::Function *);
    func_instance* callee(std::string const&);
    // edges srcs_;
    // edges trgs_;

};

template <class OutputIterator>
void block_instance::getFuncs(OutputIterator result) {
   std::vector<ParseAPI::Function *> pFuncs;
   llb()->getFuncs(pFuncs);
   for (unsigned i = 0; i < pFuncs.size(); ++i) {
      func_instance *func = findFunction(pFuncs[i]);
      *result = func;
      ++result;
   }
}

struct BlockInstanceAddrCompare {
   bool operator()(block_instance * const &b1,
                   block_instance * const &b2) const {
      return (b1->start() < b2->start());
   }
};

typedef std::set<block_instance *, BlockInstanceAddrCompare> AddrOrderedBlockSet;



#endif
