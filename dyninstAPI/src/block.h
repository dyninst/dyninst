
#if !defined(_DYN_BLOCK_H_)
#define _DYN_BLOCK_H_

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

// Shortcuts for type casting
#define SCAST_MO(o) static_cast<mapped_object*>(o)
#define SCAST_EI(e) static_cast<edge_instance*>(e)
#define SCAST_BI(b) static_cast<block_instance*>(b)
#define SCAST_PB(b) static_cast<parse_block*>(b)
#define SCAST_PF(f) static_cast<parse_func*>(f)
#define SCAST_FI(f) static_cast<func_instance*>(f)

class edge_instance : public Dyninst::PatchAPI::PatchEdge {
  friend class block_instance;
  friend class func_instance;
  friend class mapped_object;

  public:
    block_instance *src() const;
    block_instance *trg() const;
    AddressSpace *proc();
  private:
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
  EdgePredicateAdapter,
  edge_instance *,
  edge_instance * > {
  public:
  EdgePredicateAdapter() : int_(NULL) {};
  EdgePredicateAdapter(ParseAPI::EdgePredicate *intPred) : int_(intPred) {};
   virtual ~EdgePredicateAdapter() {};
   virtual bool pred_impl(edge_instance *e) const { return int_->pred_impl(e->edge()); };

  private:
   ParseAPI::EdgePredicate *int_;
};

class block_instance : public Dyninst::PatchAPI::PatchBlock {
  friend class mapped_object;

  public:
    typedef std::vector<edge_instance *> edges;
    typedef std::vector<edge_instance *> edgelist;

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
    parse_block * llb() const { return SCAST_PB(block_); }
    void *getPtrToInstruction(Address addr) const;

    const edgelist &sources();
    const edgelist &targets();

    // Shortcuts
    edge_instance *getTarget();
    edge_instance *getFallthrough();
    // NULL if not conclusive
    block_instance *getFallthroughBlock();
    block_instance *getTargetBlock();

    func_instance *callee();
    std::string calleeName();

    int id() const;

    // Functions to avoid
    // These are convinence wrappers for really expensive
    // lookups, and thus should be avoided. 
    func_instance *entryOfFunc() const;
    bool isFuncExit() const;

 private:
    void updateCallTarget(func_instance *func);
    func_instance *findFunction(ParseAPI::Function *);

    edges srcs_;
    edges trgs_;

    BlockInstpoints points_;
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
