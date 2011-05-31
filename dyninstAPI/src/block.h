
#if !defined(_DYN_BLOCK_H_)
#define _DYN_BLOCK_H_

#include "parse-cfg.h"
#include "parseAPI/h/CFG.h"
#include "instPoint.h"

class block_instance;
class func_instance;
class parse_func;
class BPatch_edge;
class mapped_object;

class edge_instance {
   friend class block_instance;
   friend class func_instance;
   friend class mapped_object;

  public:
   ParseAPI::Edge *edge() const { return edge_; }
   block_instance *src() const { return src_; }
   block_instance *trg() const { return trg_; }
   ParseAPI::EdgeTypeEnum type() const { return edge_->type(); }
   
   bool sinkEdge() const { return edge_->sinkEdge(); }
   bool interproc() const { return edge_->interproc() || 
         (edge_->type() == ParseAPI::CALL) || 
         (edge_->type() == ParseAPI::RET); }

   AddressSpace *proc();

   static void destroy(edge_instance *);

  private:
   edge_instance(ParseAPI::Edge *edge, block_instance *src, block_instance *trg);
   edge_instance(const edge_instance *parent, mapped_object *child);
   ~edge_instance();
   
   ParseAPI::Edge *edge_;
   block_instance *src_;
   block_instance *trg_;
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

class block_instance {
   friend class mapped_object;

 public:
	 typedef std::vector<edge_instance *> edges;
	 typedef std::vector<edge_instance *> edgelist;


    block_instance(ParseAPI::Block *ib, mapped_object *obj);
    block_instance(const block_instance *parent, mapped_object *child);
    ~block_instance();

    // "Basic" block stuff
    Address start() const;
    Address end() const;
    Address last() const;
    unsigned size() const;

    // Up-accessors
    mapped_object *obj() const { return obj_; }
    AddressSpace *addrSpace() const;
    AddressSpace *proc() const { return addrSpace(); }

    int containingFuncs() const { return llb()->containingFuncs(); }
    template<class OutputIterator> 
       void getFuncs(OutputIterator result);

    bool isShared() const { return block_->isShared(); }

    void triggerModified();

    parse_block * llb() const { return block_; }
    
    std::string format() const;

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

    // TODO: this should be a map from addr to insn, really
    typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
    void getInsns(Insns &instances) const;
    InstructionAPI::Instruction::Ptr getInsn(Address a) const;

    std::string disassemble() const;

    void *getPtrToInstruction(Address addr) const;

    bool containsCall();
    bool containsDynamicCall();

    int id() const;

    // Functions to avoid
    // These are convinence wrappers for really expensive
    // lookups, and thus should be avoided. 
    func_instance *entryOfFunc() const;
    bool isFuncExit() const;

    static void destroy(block_instance *b);

 private:
    void updateCallTarget(func_instance *func);
    func_instance *findFunction(ParseAPI::Function *);

    mapped_object *obj_;
    parse_block *block_;

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
